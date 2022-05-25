#include <SDL2/SDL2.h>
#include "nctietue.h"

#define MIN(a,b) (a)<=(b)? (a): (b)
#define GET_SCALE xlen<=mon_w && ylen<=mon_h ? 1 : MIN((float)mon_h/ylen, (float)mon_w/xlen)

void nct_plot_var(nct_var* var) {
  if(SDL_INIT(SDL_INIT_VIDEO)) {
    fprintf(stderr, "%sError in SDL_INIT:%s %s\n", error_color, SDL_GetError(), default_color);
    return;
  }
  SDL_DisplayMode dm;
  int mon_w, mon_h;
  if(SDL_GetCurrentDisplayMode(0, &dm)) {
    fprintf(stderr, "%sError in getting monitor size%s: %s\n", error_color, SDL_GetError(), default_color);
    mon_w = mon_h = 500;
  } else {
    mon_w = dm.w;
    mon_h = dm.h;
  }
  int xlen = var->dimlens[var->ndims-1];
  int ylen = var->dimlens[var->ndims-2];
  float scale = GET_SCALE;
  
  SDL_Window* window = SDL_CreateWindow("Figure", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, xlen*scale, ylen*scale, SDL_WINDOW_RESIZABLE);
  SDL_Renderer* rend = SDL_CreateRenderer(window, -1, SDL_RENDERER_TARGETTEXTURE);
  SDL_GetWindowSize(ikk, &ikk_w, &ikk_h); //in case window manager changed the size from requested
  scale = GET_SCALE;
}
