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
  GnDrawImage(event, GnInternal.mediumMono, 0, 0);
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

