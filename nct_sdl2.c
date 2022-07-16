#include <SDL2/SDL.h>
#include <sys/time.h>
#include <stdint.h>
#include "nctietue2.h"

#define MIN(a,b) ((a)<=(b)? (a): (b))
#define MAX(a,b) ((a)>=(b)? (a): (b))
#define GET_XSCALE_1D(xlen, win_w) (float)(xlen)/(win_w)
#define ARRSIZE(a) sizeof(a)/sizeof(*(a))

static SDL_Renderer* rend;
static SDL_Window* window;
static SDL_Texture* base;
static nct_var* var;
static int win_w, win_h, xid, yid, draw_w, draw_h;
static int invert_y, stop, echo_on, has_echoed, fill_on;
static float space, minshift, maxshift;
static unsigned char color_fg[3] = {1, 1, 1};
static unsigned char color_bg[3] = {0, 0, 0};
static void (*redraw)(nct_var*);

SDL_Event event;

typedef union {
    void* v;
    float f;
} Arg;

typedef struct {
    int key;
    int mod;
    void (*fun)(Arg);
    Arg arg;
} Binding;

#define ONE_TYPE(nctype, form, ctype)					\
    static void draw2d_##nctype(nct_var* var)				\
    {									\
	int xlen = NCTVARDIM(*var,xid).len;				\
	double di=0, dj=0;						\
	ctype minmax[2], range;						\
	nct_varminmax_##nctype(var, minmax);				\
	range = minmax[1]-minmax[0];					\
	minmax[0] += range*minshift;					\
	minmax[1] += range*maxshift;					\
	SDL_SetRenderDrawColor(rend, color_bg[0], color_bg[1], color_bg[2], 255); \
	SDL_RenderClear(rend);						\
	if(echo_on)							\
	    printf("%smin %" #form ", max %" #form "\033[K\n"		\
		   "minshift %.4f, maxshift %.4f\033[K\n"		\
		   "space %.4f\033[K\n",				\
		   has_echoed++? "\033[3F": "", minmax[0], minmax[1], minshift, maxshift, space); \
	if(invert_y) {							\
	    for(int j=draw_h-1; j>=0; j--, dj+=space) {			\
		for(int i=0; i<draw_w; i++, di+=space) {		\
		    ctype val = ((ctype*)var->data)[(int)dj*xlen + (int)di]; \
		    int value = ( val <  minmax[0]? 0   :		\
				  val >= minmax[1]? 255 :		\
				  (val - minmax[0]) * 255 / (minmax[1]-minmax[0]) ); \
		    SDL_SetRenderDrawColor(rend, value, value, value, 255); \
		    SDL_RenderDrawPoint(rend, i, j);			\
		}							\
		di = 0;							\
	    }								\
	} else {							\
	    for(int j=0; j<draw_h; j++, dj+=space) {			\
		for(int i=0; i<draw_w; i++, di+=space) {		\
		    ctype val = ((ctype*)var->data)[(int)dj*xlen + (int)di]; \
		    int value = ( val <  minmax[0]? 0   :		\
				  val >= minmax[1]? 255 :		\
				  (val - minmax[0]) * 255 / (minmax[1]-minmax[0]) ); \
		    SDL_SetRenderDrawColor(rend, value, value, value, 255); \
		    SDL_RenderDrawPoint(rend, i, j);			\
		}							\
		di = 0;							\
	    }								\
	}								\
    }
ALL_TYPES_EXCEPT_STRING
#undef ONE_TYPE

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

static void redraw_2d(nct_var* var) {
    SDL_SetRenderTarget(rend, base);
    drawfunctions_2d[var->xtype](var);
    SDL_SetRenderTarget(rend, NULL);
}

static void redraw_1d(nct_var* var) {
    SDL_SetRenderTarget(rend, base);
    drawfunctions_1d[var->xtype](var);
    SDL_SetRenderTarget(rend, NULL);
}

static uint_fast64_t time_now_ms() {
    struct timeval t;
    gettimeofday(&t, NULL);
    return t.tv_sec*1000 + t.tv_usec/1000;
}

#define GET_SPACE_FILL(xlen,win_w,ylen,win_h)    MIN((float)(ylen)/(win_h), (float)(xlen)/(win_w))
#define GET_SPACE_NONFILL(xlen,win_w,ylen,win_h) MAX((float)(ylen)/(win_h), (float)(xlen)/(win_w))
#define GET_SPACE(a,b,c,d) (fill_on? GET_SPACE_FILL(a,b,c,d): GET_SPACE_NONFILL(a,b,c,d))

static void set_draw_params() {
    int xlen = NCTVARDIM(*var,xid).len;
    int ylen = NCTVARDIM(*var,yid).len;
    space    = GET_SPACE(xlen, win_w, ylen, win_h);
    draw_w   = xlen / space;
    draw_h   = ylen / space;
    if(fill_on) {
	draw_w = MIN(win_w, draw_w);
	draw_h = MIN(win_h, draw_h);
    }
}

static void quit(Arg) {
    stop = 1;
    SDL_DestroyTexture(base);
    SDL_DestroyRenderer(rend);
    SDL_DestroyWindow(window);
    SDL_Quit();
}

static void resized() {
    static uint_fast64_t lasttime;
    uint_fast64_t thistime = time_now_ms();
    if(thistime-lasttime < 10)
	return;
    lasttime = thistime;
    SDL_DestroyTexture(base);
    base = SDL_CreateTexture(rend, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, win_w, win_h);
    SDL_GetWindowSize(window, &win_w, &win_h);
    set_draw_params();
    redraw(var);
}

void invert_y_func(Arg) {
    invert_y = !invert_y;
    redraw(var);
}

void shift_min(Arg shift) {
    minshift += shift.f;
    redraw(var);
}

void shift_max(Arg shift) {
    maxshift += shift.f;
    redraw(var);
}

void toggle_echo(Arg) {
    echo_on = !echo_on;
}

void toggle_fill(Arg) {
    fill_on = !fill_on;
    set_draw_params();
    redraw(var);
}

void debug_break(Arg) {
    asm("int $3");
}

Binding keydown_bindings[] = {
    { SDLK_i,     0,          invert_y_func, {0}        },
    { SDLK_q,     0,          quit,          {0}        },
    { SDLK_1,     0,          shift_min,     {.f=-0.02} },
    { SDLK_1,     KMOD_SHIFT, shift_max,     {.f=-0.02} },
    { SDLK_2,     0,          shift_min,     {.f=0.02}  },
    { SDLK_2,     KMOD_SHIFT, shift_max,     {.f=0.02}  },
    { SDLK_e,     0,          toggle_echo,   {0}        },
    { SDLK_f,     0,          toggle_fill,   {0}        },
    { SDLK_PAUSE, KMOD_SHIFT, debug_break,   {0}        },
};

int get_modstate() {
    /*makes modstate side-insensitive*/
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
    while(1) {
	while(SDL_PollEvent(&event)) {
	    if(event.type == SDL_QUIT)
		quit((Arg){0});
	    else if(event.type==SDL_WINDOWEVENT && event.window.event==SDL_WINDOWEVENT_RESIZED)
		resized();
	    else if(event.type==SDL_KEYDOWN)
		keydown_func();
	    if(stop) return;
	}
	if(stop) return;
	SDL_RenderCopy(rend, base, NULL, NULL);
	SDL_RenderPresent(rend);
	SDL_Delay(12);
    }
}

static void _plot_var_1d(nct_var* var) {
    /*come from nct_plot_var*/
    int xlen = NCTVARDIM(*var, xid).len;
    window = SDL_CreateWindow("Figure", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, MIN(xlen,win_w), MIN(800,win_h), SDL_WINDOW_RESIZABLE);
    rend = SDL_CreateRenderer(window, -1, SDL_RENDERER_TARGETTEXTURE);
    SDL_GetWindowSize(window, &win_w, &win_h);
    base = SDL_CreateTexture(rend, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, win_w, win_h);
    space = GET_XSCALE_1D(xlen, win_w);

    redraw_1d(var);

    mainloop();
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
    xid = var->ndims-1;
    yid = var->ndims-2;
    if(yid < 0) {
	redraw = redraw_1d;
	return _plot_var_1d(var);
    }
    redraw = redraw_2d;
    int xlen = NCTVARDIM(*var, xid).len;
    int ylen = NCTVARDIM(*var, yid).len;
  
    window = SDL_CreateWindow("Figure", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, MIN(xlen,win_w), MIN(ylen,win_h), SDL_WINDOW_RESIZABLE);
    rend = SDL_CreateRenderer(window, -1, SDL_RENDERER_TARGETTEXTURE);
    SDL_GetWindowSize(window, &win_w, &win_h);
    base = SDL_CreateTexture(rend, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, win_w, win_h);
    set_draw_params();

    stop = has_echoed = 0;
    redraw(var);

    mainloop();
}
