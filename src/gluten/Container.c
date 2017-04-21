#ifndef AMALGAMATION
  #include "Container.h"
  #include "Widget.h"
#endif

void GnContainerDraw(GnWidget *ctx, GnEvent *event)
{
  size_t i = 0;
  GnContainer *container = GnWidgetComponent(ctx, GnContainer);

  for(i = 0; i < vector_size(container->children); i++)
  {
    GnWidgetEvent(vector_at(container->children, i), "draw", event);
  }
}

void GnContainerSize(GnWidget *ctx, GnEvent *event)
{
  size_t i = 0;
  GnContainer *container = GnWidgetComponent(ctx, GnContainer);

  for(i = 0; i < vector_size(container->children); i++)
  {
    GnWidgetEvent(vector_at(container->children, i), "size", event);
  }
}

void GnContainerDestroy(GnWidget *ctx, GnEvent *event)
{
  size_t i = 0;
  GnContainer *container = GnWidgetComponent(ctx, GnContainer);

  for(i = 0; i < vector_size(container->children); i++)
  {
    GnWidgetDestroy(vector_at(container->children, i));
  }

  vector_delete(container->children);
}

void GnContainerInit(GnWidget *ctx, GnEvent *event)
{
  GnContainer *container = GnWidgetComponent(ctx, GnContainer);
  container->children = vector_new(GnWidget *);
  GnWidgetAddEvent(ctx, "draw", GnContainerDraw);
  GnWidgetAddEvent(ctx, "size", GnContainerSize);
  GnWidgetAddEvent(ctx, "destroy", GnContainerDestroy);
}

void GnContainerAdd(GnWidget *ctx, GnWidget *child)
{
  GnContainer *container = GnWidgetComponent(ctx, GnContainer);
  vector_push_back(container->children, child);
}
