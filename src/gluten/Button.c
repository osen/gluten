#ifndef AMALGAMATION
  #include "config.h"
  #include "gluten.h"
  #include "Button.h"
  #include "Label.h"
  #include "Widget.h"
  #include "Position.h"
  #include "Draw.h"
#endif

void GnButtonDraw(GnWidget *ctx, GnEvent *event)
{
  GnPosition *position = GnWidgetComponent(ctx, GnPosition);

  GnDrawFillRect(event, position->x, position->y,
    position->width, position->height, GN_WIDGET_BACKGROUND);
}

void GnButtonInit(GnWidget *ctx, GnEvent *event)
{
  printf("GnButtonInit\n");

  GnWidgetAddEvent(ctx, "draw", GnButtonDraw);
  GnWidgetAddComponent(ctx, GnLabel);
}

