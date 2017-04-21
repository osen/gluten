#ifndef AMALGAMATION
  #include "Position.h"
  #include "Widget.h"
#endif

void GnPositionInit(GnWidget *ctx, GnEvent *event)
{
  GnWidgetSetPosition(ctx, 5, 5);
  GnWidgetSetSize(ctx, 80, 30);
}

void GnWidgetSetPosition(GnWidget *ctx, int x, int y)
{
  GnPosition *position = GnWidgetComponent(ctx, GnPosition);

  position->x = x;
  position->y = y;
}

void GnWidgetSetSize(GnWidget *ctx, int width, int height)
{
  GnPosition *position = GnWidgetComponent(ctx, GnPosition);

  position->width = width;
  position->height = height;
}

