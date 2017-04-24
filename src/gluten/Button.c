#ifndef AMALGAMATION
  #include "config.h"
  #include "Button.h"
  #include "Label.h"
  #include "Widget.h"
  #include "Position.h"
  #include "Draw.h"
#endif

void GnButtonDraw(GnWidget *ctx, GnEvent *event)
{
  GnDrawFillRect(event, 0, 0,
    GnWidgetWidth(ctx), GnWidgetHeight(ctx), GN_WIDGET_BORDER);

  GnDrawFillRect(event, 2, 2,
    GnWidgetWidth(ctx) - 4, GnWidgetHeight(ctx) - 4, GN_WIDGET_BACKGROUND);
}

void GnButtonInit(GnWidget *ctx, GnEvent *event)
{
  printf("GnButtonInit\n");

  GnWidgetAddEvent(ctx, "draw", GnButtonDraw);
  GnWidgetAddComponent(ctx, GnLabel);
}

