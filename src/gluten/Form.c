#ifndef AMALGAMATION
  #include "Form.h"
  #include "gluten.h"
  #include "Widget.h"
  #include <vector.h>
#endif

#include <string.h>

void GnFormShow(GnWidget *ctx)
{
  GnInternal.activeForm = ctx;
}

void GnFormSize(GnWidget *ctx, GnEvent *event)
{
  /* TODO: Set rectangle to window size */
}

void GnFormDraw(GnWidget *ctx, GnEvent *event)
{
#ifdef USE_SDL
  SDL_FillRect(GnUnsafe.screen, &GnUnsafe.screen->clip_rect,
    SDL_MapRGB(GnUnsafe.screen->format, GN_FORM_BACKGROUND));
#endif
}

void GnFormInit(GnWidget *ctx, GnEvent *event)
{
  GnWidgetAddEvent(ctx, "draw", GnFormDraw);
  GnWidgetAddEvent(ctx, "size", GnFormSize);
  GnWidgetAddComponent(ctx, GnContainer);
  vector_push_back(GnInternal.forms, ctx);

  /* TODO: Remove anchor component because form is special */
}

