#ifndef AMALGAMATION
  #include "Position.h"
  #include "Widget.h"
#endif

void GnPositionRequestDraw(GnWidget *ctx, GnEvent *event)
{
  GnPosition *position = GnWidgetComponent(ctx, GnPosition);

  if(position->dirty)
  {
    GnWidgetEvent(ctx, "draw", event);
    position->dirty = 0;
  }
}

void GnPositionSize(GnWidget *ctx, GnEvent *event)
{
  GnPosition *position = GnWidgetComponent(ctx, GnPosition);

  position->dirty = 1;
}

void GnPositionInit(GnWidget *ctx, GnEvent *event)
{
  GnPosition *position = GnWidgetComponent(ctx, GnPosition);

  position->dirty = 1;
  GnWidgetSetPosition(ctx, 5, 5);
  GnWidgetSetSize(ctx, 80, 30);

  GnWidgetAddEvent(ctx, "request_draw", GnPositionRequestDraw);
  GnWidgetAddEvent(ctx, "size", GnPositionSize);
}

void GnWidgetSetPosition(GnWidget *ctx, int x, int y)
{
  GnPosition *position = GnWidgetComponent(ctx, GnPosition);

  position->x = x;
  position->y = y;
  position->dirty = 1;
}

void GnWidgetSetSize(GnWidget *ctx, int width, int height)
{
  GnPosition *position = GnWidgetComponent(ctx, GnPosition);

  position->width = width;
  position->height = height;
  position->dirty = 1;

  /* TODO: Send size event */
}

int GnWidgetX(GnWidget *ctx)
{
  GnPosition *position = GnWidgetComponent(ctx, GnPosition);

  return position->x;
}

int GnWidgetY(GnWidget *ctx)
{
  GnPosition *position = GnWidgetComponent(ctx, GnPosition);

  return position->y;
}

int GnWidgetWidth(GnWidget *ctx)
{
  GnPosition *position = GnWidgetComponent(ctx, GnPosition);

  return position->width;
}

int GnWidgetHeight(GnWidget *ctx)
{
  GnPosition *position = GnWidgetComponent(ctx, GnPosition);

  return position->height;
}

