#ifndef AMALGAMATION
  #include "config.h"
  #include "gluten.h"
  #include "Label.h"
  #include "Widget.h"
  #include "Position.h"
  #include "Anchor.h"
  #include <palloc.h>
#endif

#ifdef USE_SDL
  #include <SDL/SDL.h>
#endif

void GnLabelDraw(GnWidget *ctx, GnEvent *event)
{
  GnPosition *position = GnWidgetComponent(ctx, GnPosition);

#ifdef USE_SDL
  SDL_Rect r = {0};
  r.x = position->x + 5;
  r.y = position->y + 5;
  r.w = position->width - 10;
  r.h = position->height - 10;

  SDL_FillRect(GnUnsafe.screen, &r,
    SDL_MapRGB(GnUnsafe.screen->format, GN_WIDGET_FOREGROUND));
#endif
}

void GnLabelSetText(GnWidget *ctx, char *text)
{
  GnLabel *label = GnWidgetComponent(ctx, GnLabel);
  printf("GnLabelSetText\n");
  pfree(label->text);
  label->text = palloc(char);
}

void GnLabelDestroy(GnWidget *ctx, GnEvent *event)
{
  GnLabel *label = GnWidgetComponent(ctx, GnLabel);
  pfree(label->text);
}

void GnLabelInit(GnWidget *ctx, GnEvent *event)
{
  GnLabel *label = GnWidgetComponent(ctx, GnLabel);
  label->text = palloc(char);
  printf("GnLabelInit\n");
  GnWidgetAddComponent(ctx, GnAnchor);
  GnWidgetAddEvent(ctx, "draw", GnLabelDraw);
  GnWidgetAddEvent(ctx, "destroy", GnLabelDestroy);
}

