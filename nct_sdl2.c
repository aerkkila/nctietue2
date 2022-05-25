#include <SDL2/SDL.h>
#include <netcdf.h>
#include "nctietue.h"

#define MIN(a,b) (a)<=(b)? (a): (b)
#define GET_SCALE(xlen,win_w,ylen,win_h) (xlen)<=(win_w) && (ylen)<=(win_h) ? 1 : MIN((float)(win_h)/(ylen), (float)(win_w)/(xlen))

static SDL_Renderer* rend;
static SDL_Window* window;
static SDL_Texture* base;
static int loop = 1;

#define ONE_TYPE(a, b, ctype)						\
  static void draw_##ctype(nct_var* var, int xid, int yid, double scale) \
  {									\
  double dxlen = var->dimlens[xid], dylen = var->dimlens[yid];		\
  int xlen = var->dimlens[xid];						\
  for(double dj=0; dj<dylen; dj+=scale) 				\
    for(double di=0; di<dxlen; di+=scale) {				\
      int i = di, j=dj;							\
      int value = ((ctype*)var->data)[j*xlen + i];			\
      SDL_SetRenderDrawColor(rend, value, value, value, 255);		\
      SDL_RenderDrawPoint(rend, i, j);					\
    }									\
  }
ALL_TYPES_EXCEPT_STRING
#undef ONE_TYPE

#define ONE_TYPE(nctype,b,ctype) [nctype]=draw_##ctype,
void (*drawfunctions[])(nct_var*, int, int, double) = {
  ALL_TYPES_EXCEPT_STRING
};
#undef ONE_TYPE

void quit() {
  SDL_DestroyTexture(base);
  SDL_DestroyRenderer(rend);
  SDL_DestroyWindow(window);
  SDL_Quit();
  loop = 0;
}

void mainloop() {
  SDL_Event event;
  while(loop) {
    while(SDL_GetEvent(&event))
      if(event.type == SDL_QUIT)
	quit();
    SDL_RenderCopy(rend, base);
    SDL_RenderPresent(rend);
    SDL_Delay(12);
  }
}

void nct_plot_var(nct_var* var) {
  if(SDL_INIT(SDL_INIT_VIDEO)) {
    fprintf(stderr, "%sError in SDL_INIT:%s %s\n", error_color, SDL_GetError(), default_color);
    return;
  }
  SDL_DisplayMode dm;
  int win_w, win_h;
  if(SDL_GetCurrentDisplayMode(0, &dm)) {
    fprintf(stderr, "%sError in getting monitor size%s: %s\n", error_color, SDL_GetError(), default_color);
    win_w = win_h = 500;
  } else {
    win_w = dm.w;
    win_h = dm.h;
  }
  int xid = var->ndims-1, yid = var->ndims-2;
  int xlen = var->dimlens[xid];
  int ylen = var->dimlens[yid];
  double scale = GET_SCALE(xlen,win_w,ylen,win_h);
  
  window = SDL_CreateWindow("Figure", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, xlen*scale, ylen*scale, SDL_WINDOW_RESIZABLE);
  rend = SDL_CreateRenderer(window, -1, SDL_RENDERER_TARGETTEXTURE);
  SDL_GetWindowSize(ikk, &ikk_w, &ikk_h); //in case window manager changed the size from requested
  base = SDL_CreateTexture( rend, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, win_w, win_h );
  scale = GET_SCALE(xlen,win_w,ylen,win_h);

  SDL_SetRenderTarget(rend, base);
  drawfunctions[var->nctype](var, xid, yid);
  SDL_SetRenderTarget(rend, NULL);

  mainloop();
}
