#ifndef AMALGAMATION
  #include "config.h"
  #include "gluten.h"
  #include "Label.h"
  #include "Widget.h"
  #include "Position.h"
  #include "Anchor.h"
  #include "Draw.h"
  #include <palloc.h>
#endif

void GnLabelDraw(GnWidget *ctx, GnEvent *event)
{
  GnPosition *position = GnWidgetComponent(ctx, GnPosition);

/*
  SDL_Rect r = {0};
  r.x = position->x;
  r.y = position->y;
  r.w = position->width;
  r.h = position->height;

  SDL_BlitSurface(GnInternal.mediumMono->surface, NULL, GnUnsafe.buffer, &r);
*/
  GnDrawFillRect(event, position->x + 5, position->y + 5,
    position->width - 10, position->height - 10, GN_WIDGET_FOREGROUND);

  GnDrawImage(event, GnInternal.mediumMono, position->x, position->y);
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

