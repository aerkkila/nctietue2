#include <SDL2/SDL.h>
#include <netcdf.h>
#include <sys/time.h>
#include "nctietue.h"

#define MIN(a,b) (a)<=(b)? (a): (b)
#define GET_SCALE(xlen,win_w,ylen,win_h) MIN((float)(ylen)/(win_h), (float)(xlen)/(win_w))

static SDL_Renderer* rend;
static SDL_Window* window;
static SDL_Texture* base;
static int loop = 1;
static int win_w, win_h, xid, yid;
static double scale;

#define ONE_TYPE(nctype, b, ctype)					\
  static void draw_##nctype(nct_var* var, double scale)			\
  {									\
    int xlen = var->dimlens[xid];					\
    double di=0, dj=0;							\
    for(int j=0; j<win_h; j++, dj+=scale) {				\
      for(int i=0; i<win_w; i++, di+=scale) {				\
	int value = ((ctype*)var->data)[(int)dj*xlen + (int)di];	\
	SDL_SetRenderDrawColor(rend, value, value, value, 255);		\
	SDL_RenderDrawPoint(rend, i, j);				\
      }									\
      di = 0;								\
    }									\
  }

ALL_TYPES_EXCEPT_STRING
#undef ONE_TYPE

#define ONE_TYPE(nctype,b,ctype) [nctype]=draw_##nctype,
void (*drawfunctions[])(nct_var*, double) = {
  ALL_TYPES_EXCEPT_STRING
};
#undef ONE_TYPE

static uint_fast64_t time_now_ms() {
  struct timeval t;
  gettimeofday(&t, NULL);
  return t.tv_sec*1000 + t.tv_usec/1000;
}

static void redraw(nct_var* var, double scale) {
  SDL_SetRenderTarget(rend, base);
  drawfunctions[var->xtype](var, scale);
  SDL_SetRenderTarget(rend, NULL);
}

static void quit() {
  SDL_DestroyTexture(base);
  SDL_DestroyRenderer(rend);
  SDL_DestroyWindow(window);
  SDL_Quit();
  loop = 0;
}

static nct_var* var;

static void resized() {
  static uint_fast64_t lasttime;
  uint_fast64_t thistime = time_now_ms();
  if(thistime-lasttime < 20)
    return;
  lasttime = thistime;
  SDL_DestroyTexture(base);
  base = SDL_CreateTexture(rend, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, win_w, win_h);
  SDL_GetWindowSize(window, &win_w, &win_h);
  scale = GET_SCALE(var->dimlens[xid], win_w, var->dimlens[yid], win_h);
  redraw(var, scale);
}

static void mainloop() {
  SDL_Event event;
  while(1) {
    while(SDL_PollEvent(&event)) {
      if(event.type == SDL_QUIT)
	quit();
      else if(event.type==SDL_WINDOWEVENT && event.window.event==SDL_WINDOWEVENT_RESIZED)
	resized();
      if(!loop)
	return;
    }
    SDL_RenderCopy(rend, base, NULL, NULL);
    SDL_RenderPresent(rend);
    SDL_Delay(12);
  }
}

void nct_plot_var(nct_var* var0) {
  if(SDL_Init(SDL_INIT_VIDEO)) {
    fprintf(stderr, "%sError in SDL_INIT:%s %s\n", error_color, SDL_GetError(), default_color);
    return;
  }
  var = var0;
  SDL_DisplayMode dm;
  if(SDL_GetCurrentDisplayMode(0, &dm)) {
    fprintf(stderr, "%sError in getting monitor size%s: %s\n", error_color, SDL_GetError(), default_color);
    win_w = win_h = 500;
  } else {
    win_w = dm.w;
    win_h = dm.h;
  }
  xid = var->ndims-1;
  yid = var->ndims-2;
  scale = GET_SCALE(var->dimlens[xid], win_w, var->dimlens[yid], win_h);
  int xlen = var->dimlens[xid];
  int ylen = var->dimlens[yid];
  
  window = SDL_CreateWindow("Figure", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, MIN(xlen,win_w), MIN(ylen,win_h), SDL_WINDOW_RESIZABLE);
  rend = SDL_CreateRenderer(window, -1, SDL_RENDERER_TARGETTEXTURE);
  SDL_GetWindowSize(window, &win_w, &win_h);
  base = SDL_CreateTexture(rend, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, win_w, win_h);
  scale = GET_SCALE(xlen, win_w, ylen, win_h);

  redraw(var, scale);

  mainloop();
}
