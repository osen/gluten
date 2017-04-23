#ifndef AMALGAMATION
  #include "Form.h"
  #include "gluten.h"
  #include "Widget.h"
  #include "Position.h"
  #include "Draw.h"
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
  GnDraw *draw = GnEventComponent(event, GnDraw);

  GnDrawFillRect(event, draw->bounds.x, draw->bounds.y,
    draw->bounds.width, draw->bounds.height,
    GN_FORM_BACKGROUND);
}

void GnFormInit(GnWidget *ctx, GnEvent *event)
{
  GnWidgetAddEvent(ctx, "draw", GnFormDraw);
  GnWidgetAddEvent(ctx, "size", GnFormSize);
  GnWidgetAddComponent(ctx, GnContainer);
  vector_push_back(GnInternal.forms, ctx);

  /* TODO: Remove anchor component because form is special */
}

