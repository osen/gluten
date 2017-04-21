#ifndef AMALGAMATION
  #include "Anchor.h"
  #include "Widget.h"
#endif

void GnAnchorSize(GnWidget *ctx, GnEvent *event)
{

}

void GnAnchorInit(GnWidget *ctx, GnEvent *event)
{
  GnWidgetAddEvent(ctx, "size", GnAnchorSize);
}

