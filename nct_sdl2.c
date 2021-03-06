#include <SDL2/SDL.h>
#include <sys/time.h>
#include <stdint.h>
#include "nctietue2.h"
#include "colormaps.h"

#define MIN(a,b) ((a)<=(b)? (a): (b))
#define MAX(a,b) ((a)>=(b)? (a): (b))
#define ARRSIZE(a) sizeof(a)/sizeof(*(a))

static SDL_Renderer* rend;
static SDL_Window* window;
static SDL_Texture* base;
static nct_var* var;
static const Uint32 default_sleep=8; // ms
static Uint32 sleeptime;
static int win_w, win_h, xid, yid, zid, draw_w, draw_h, znum;
static size_t stepsize_z;
static int invert_y, invert_c, stop, echo_on=1, has_echoed, fill_on, play_on, play_inv;
static int cmapnum=18, cmappix=30, cmapspace=10, call_resized, call_redraw;
static float space, minshift, maxshift;
static unsigned char color_fg[3] = {255, 255, 255};
static unsigned char color_bg[3] = {0, 0, 0};
static const char* echo_highlight = "\033[1;93m";
static void (*redraw)(nct_var*);

static void redraw_1d(nct_var* var);
static void redraw_2d(nct_var* var);
static void draw_colormap();
static void set_xid_and_yid();
static uint_fast64_t time_now_ms();

SDL_Event event;

typedef union {
    void* v;
    float f;
    int   i;
} Arg;

typedef struct {
    int key;
    int mod;
    void (*fun)(Arg);
    Arg arg;
} Binding;

#include "nct_draw.h"

void draw_colormap() {
    float cspace = 255.0f/win_w;
    float di = 0;
    if(!invert_c)
	for(int i=0; i<win_w; i++, di+=cspace) {
	    char* c = COLORVALUE(cmapnum, (int)di);
	    SDL_SetRenderDrawColor(rend, c[0], c[1], c[2], 255);
	    for(int j=draw_h+cmapspace; j<draw_h+cmapspace+cmappix; j++)
		SDL_RenderDrawPoint(rend, i, j);
	}
    else
	for(int i=win_w-1; i>=0; i--, di+=cspace) {
	    char* c = COLORVALUE(cmapnum, (int)di);
	    SDL_SetRenderDrawColor(rend, c[0], c[1], c[2], 255);
	    for(int j=draw_h+cmapspace; j<draw_h+cmapspace+cmappix; j++)
		SDL_RenderDrawPoint(rend, i, j);
	}	
}

#define ONE_TYPE(nctype, b, ctype)					\
    static void draw1d_##nctype(nct_var* var)				\
    {									\
	SDL_SetRenderDrawColor(rend, color_bg[0], color_bg[1], color_bg[2], 255); \
	SDL_RenderClear(rend);						\
	double di=0;							\
	ctype minmax[2];						\
	nct_varminmax_##nctype(var, minmax);				\
	SDL_SetRenderDrawColor(rend, color_fg[0], color_fg[1], color_fg[2], 255); \
	for(int i=0; i<win_w; i++, di+=space) {				\
	    int y = (((ctype*)var->data)[(int)di] - minmax[0]) * win_h / (minmax[1]-minmax[0]); \
	    SDL_RenderDrawPoint(rend, i, y);				\
	}								\
    }
ALL_TYPES_EXCEPT_STRING
#undef ONE_TYPE

#define ONE_TYPE(nctype,b,ctype) [nctype]=draw2d_##nctype,
static void (*drawfunctions_2d[])(nct_var*) = {ALL_TYPES_EXCEPT_STRING};
#undef ONE_TYPE

#define ONE_TYPE(nctype,b,ctype) [nctype]=draw1d_##nctype,
static void (*drawfunctions_1d[])(nct_var*) = {ALL_TYPES_EXCEPT_STRING};
#undef ONE_TYPE

void set_xid_and_yid() {
     xid = var->ndims-1;
     yid = var->ndims-2;
     zid = var->ndims-3;
    if( yid < 0)
	redraw = redraw_1d;
    else
	redraw = redraw_2d;
}

static uint_fast64_t time_now_ms() {
    struct timeval t;
    gettimeofday(&t, NULL);
    return t.tv_sec*1000 + t.tv_usec/1000;
}

static void redraw_2d(nct_var* var) {
    static uint_fast64_t lasttime;
    uint_fast64_t thistime = time_now_ms();
    if(thistime-lasttime < 10) {
	call_redraw = 1;
	return;
    }
    call_redraw = 0;
    lasttime = thistime;

    SDL_SetRenderTarget(rend, base);
    drawfunctions_2d[var->xtype](var);
    SDL_SetRenderTarget(rend, NULL);
}

static void redraw_1d(nct_var* var) {
    SDL_SetRenderTarget(rend, base);
    drawfunctions_1d[var->xtype](var);
    SDL_SetRenderTarget(rend, NULL);
}

#define GET_SPACE_FILL(xlen,win_w,ylen,win_h)    MIN((float)(ylen)/(win_h), (float)(xlen)/(win_w))
#define GET_SPACE_NONFILL(xlen,win_w,ylen,win_h) MAX((float)(ylen)/(win_h), (float)(xlen)/(win_w))
#define GET_SPACE(a,b,c,d) (fill_on? GET_SPACE_FILL(a,b,c,d): GET_SPACE_NONFILL(a,b,c,d))

static void set_draw_params() {
    int xlen = NCTVARDIM(*var, xid).len, ylen;
    if( yid>=0) {
	ylen  = NCTVARDIM(*var, yid).len;
	space = GET_SPACE(xlen, win_w, ylen, win_h-cmapspace-cmappix);
    } else {
	space = (float)(xlen)/(win_w);
	ylen  = win_h * space;
    }
    draw_w = xlen / space;
    draw_h = ylen / space;
    if(fill_on) {
	draw_w = MIN(win_w, draw_w);
	draw_h = MIN(win_h-cmapspace-cmappix, draw_h);
    }
    if(zid>=0)
	stepsize_z = nct_vardim_steplen(var, zid);
}

static void quit(Arg _) {
    stop = 1;
    SDL_DestroyTexture(base);
    SDL_DestroyRenderer(rend);
    SDL_DestroyWindow(window);
    SDL_Quit();
}

static void resized() {
    static uint_fast64_t lasttime;
    uint_fast64_t thistime = time_now_ms();
    if(thistime-lasttime < 16) {
	call_resized = 1;
	return;
    }
    call_resized = 0;
    call_redraw = 1;
    lasttime = thistime;
    SDL_DestroyTexture(base);
    SDL_GetWindowSize(window, &win_w, &win_h);
    base = SDL_CreateTexture(rend, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, win_w, win_h);
    set_draw_params();
}

void shift_min(Arg shift) {
    minshift += shift.f;
    call_redraw = 1;
}

void shift_max(Arg shift) {
    maxshift += shift.f;
    call_redraw = 1;
}

void toggle_var(Arg intptr) {
    *(int*)intptr.v = !*(int*)intptr.v;
    set_draw_params();
    call_redraw = 1;
}

void negation(Arg intptr) {
    *(int*)intptr.v = -*(int*)intptr.v;
}

void debug_break(Arg _) {
    asm("int $3");
}

void cmap_ichange(Arg jump) {
    int len = ARRSIZE(colormaps) / 2;
    cmapnum = (cmapnum+len+jump.i) % len;
    call_redraw = 1;
}

void var_ichange(Arg jump) {
    nct_var* v;
    if(jump.i > 0) {
	if(!(v = nct_next_truevar(var, jump.i)))
	    v = nct_next_truevar(var->super->vars[0], 0);
    } else {
	if(!(v = nct_last_truevar(var, jump.i)))
	    v = nct_last_truevar(var->super->vars[var->super->nvars-1], 0);
    }
    if(!v) {
	puts("This is impossible. A variable was not found.");
	return;
    }
    var = v;
    if(!var->data)
	nct_load_var(var, -1);
    set_xid_and_yid();
    set_draw_params();
    call_redraw = 1;
}

void inc_znum(Arg intarg) {
    size_t zlen = NCTVARDIM(*var,zid).len;
    znum = (znum + zlen + intarg.i) % zlen;
    call_redraw = 1;
}

void set_sleep(Arg _) {
    printf("\033[sEnter sleeptime in ms (default = %i): ", default_sleep);
    fflush(stdout);
    if(scanf("%i", &sleeptime) != 1)
	sleeptime = default_sleep;
    printf("\033[u\033[K");
    fflush(stdout);
}

Binding keydown_bindings[] = {
    { SDLK_q,     0,          quit,          {0}             },
    { SDLK_PAUSE, KMOD_SHIFT, debug_break,   {0}             },
    { SDLK_1,     0,          shift_min,     {.f=-0.02}      },
    { SDLK_1,     KMOD_SHIFT, shift_max,     {.f=-0.02}      },
    { SDLK_2,     0,          shift_min,     {.f=0.02}       },
    { SDLK_2,     KMOD_SHIFT, shift_max,     {.f=0.02}       },
    { SDLK_e,     0,          toggle_var,    {.v=&echo_on}   },
    { SDLK_f,     0,          toggle_var,    {.v=&fill_on}   },
    { SDLK_i,     0,          toggle_var,    {.v=&invert_y}  },
    { SDLK_SPACE, 0,          toggle_var,    {.v=&play_on}   },
    { SDLK_SPACE, KMOD_SHIFT, toggle_var,    {.v=&play_inv}  },
    { SDLK_c,     0,          cmap_ichange,  {.i=1}          },
    { SDLK_c,     KMOD_SHIFT, cmap_ichange,  {.i=-1}         },
    { SDLK_c,     KMOD_ALT,   toggle_var,    {.v=&invert_c}  },
    { SDLK_v,     0,          var_ichange,   {.i=1}          },
    { SDLK_v,     KMOD_SHIFT, var_ichange,   {.i=-1}         },
    { SDLK_RIGHT, 0,          inc_znum,      {.i=1}          },
    { SDLK_LEFT,  0,          inc_znum,      {.i=-1}         },
    { SDLK_s,     0,          set_sleep,     {0}             },
};

int get_modstate() {
    /* makes modstate side-insensitive and removes other modifiers than [alt,ctrl,gui,shift] */
    int mod = 0;
    int mod0 = SDL_GetModState();
    if(mod0 & KMOD_CTRL)
	mod |= KMOD_CTRL;
    if(mod0 & KMOD_SHIFT)
	mod |= KMOD_SHIFT;
    if(mod0 & KMOD_ALT)
	mod |= KMOD_ALT;
    if(mod0 & KMOD_GUI)
	mod |= KMOD_GUI;
    return mod;
}

void keydown_func() {
    int len = ARRSIZE(keydown_bindings);
#define A keydown_bindings[i]
    for(int i=0; i<len; i++)
	if(event.key.keysym.sym == A.key)
	    if(get_modstate() == A.mod)
		A.fun(A.arg);
#undef A
}

static void mainloop() {
start:
    while(SDL_PollEvent(&event)) {
	if(event.type == SDL_QUIT)
	    quit((Arg){0});
	else if(event.type==SDL_WINDOWEVENT && event.window.event==SDL_WINDOWEVENT_RESIZED)
	    call_resized = 1;
	else if(event.type==SDL_KEYDOWN)
	    keydown_func();
	if(stop) return;
    }

    if(stop)         return;
    if(call_resized) resized();
    if(call_redraw)  redraw(var);
    if(zid < 0)      play_inv = play_on = 0;
    if(play_inv)     {inc_znum((Arg){.i=-1}); play_on=0;}
    if(play_on)      inc_znum((Arg){.i=1});

    SDL_RenderCopy(rend, base, NULL, NULL);
    SDL_RenderPresent(rend);
    SDL_Delay(sleeptime);
    goto start;
}

void nct_plot_var(nct_var* _var) {
    var = _var;
    if(SDL_Init(SDL_INIT_VIDEO)) {
	fprintf(stderr, "%sError in SDL_INIT:%s %s\n", nct_error_color, SDL_GetError(), nct_default_color);
	return;
    }
    SDL_Event event;
    while(SDL_PollEvent(&event));
    SDL_DisplayMode dm;
    if(SDL_GetCurrentDisplayMode(0, &dm)) {
	fprintf(stderr, "%sError in getting monitor size%s: %s\n", nct_error_color, SDL_GetError(), nct_default_color);
	win_w = win_h = 500;
    } else {
	win_w = dm.w;
	win_h = dm.h;
    }
    set_xid_and_yid();
    int xlen = NCTVARDIM(*var,  xid).len, ylen;
    if( yid>=0)
	ylen = NCTVARDIM(*var,  yid).len;
    else
	ylen = 400;
  
    window = SDL_CreateWindow("Figure", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
			      MIN(xlen, win_w), MIN(ylen+cmapspace+cmappix, win_h), SDL_WINDOW_RESIZABLE);
    rend = SDL_CreateRenderer(window, -1, SDL_RENDERER_TARGETTEXTURE);
    SDL_GetWindowSize(window, &win_w, &win_h);
    base = SDL_CreateTexture(rend, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, win_w, win_h);
    set_draw_params();

    sleeptime = default_sleep;
    stop = has_echoed = play_on = play_inv = 0;
    call_redraw = 1;

    mainloop();
}
