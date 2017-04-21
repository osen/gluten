#ifndef AMALGAMATION
  #include "config.h"
  #include "gluten.h"
  #include "Button.h"
  #include "Label.h"
  #include "Widget.h"
  #include "Position.h"
#endif

#ifdef USE_SDL
  #include <SDL/SDL.h>
#endif

void GnButtonDraw(GnWidget *ctx, GnEvent *event)
{
  GnPosition *position = GnWidgetComponent(ctx, GnPosition);

#ifdef USE_SDL
  SDL_Rect r = {0};
  r.x = position->x;
  r.y = position->y;
  r.w = position->width;
  r.h = position->height;

  SDL_FillRect(GnUnsafe.screen, &r,
    SDL_MapRGB(GnUnsafe.screen->format, GN_WIDGET_BORDER));

  r.x++;
  r.y++;
  r.w-=2;
  r.h-=2;

  SDL_FillRect(GnUnsafe.screen, &r,
    SDL_MapRGB(GnUnsafe.screen->format, GN_WIDGET_BACKGROUND));
#endif
}

void GnButtonInit(GnWidget *ctx, GnEvent *event)
{
  printf("GnButtonInit\n");

  GnWidgetAddEvent(ctx, "draw", GnButtonDraw);
  GnWidgetAddComponent(ctx, GnLabel);
}

