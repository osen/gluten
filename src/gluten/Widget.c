#ifndef AMALGAMATION
  #include "Widget.h"
  #include "Position.h"
  #include "palloc.h"
#endif

#include <string.h>

GnWidget *_GnWidgetCreate(size_t size, char *type, void (*initFunc)(GnWidget *, GnEvent *))
{
  GnWidget *rtn = NULL;

  rtn = palloc(GnWidget);
  rtn->components = vector_new(struct GnComponentEntry);
  rtn->events = vector_new(struct GnEventEntry);

  GnWidgetAddComponent(rtn, GnPosition);
  _GnWidgetAddComponent(rtn, size, type, initFunc);

  return rtn;
}

void GnWidgetDestroy(GnWidget *ctx)
{
  size_t i = 0;

  GnWidgetEvent(ctx, "destroy", NULL);

  for(i = 0; i < vector_size(ctx->components); i++)
  {
    pfree(vector_at(ctx->components, i).data);
    free(vector_at(ctx->components, i).type);
  }

  vector_delete(ctx->components);

  for(i = 0; i < vector_size(ctx->events); i++)
  {
    free(vector_at(ctx->events, i).name);
  }

  vector_delete(ctx->events);
  pfree(ctx);
}

void GnWidgetEvent(GnWidget *ctx, char *name, GnEvent *event)
{
  size_t i = 0;

  for(i = 0; i < vector_size(ctx->events); i++)
  {
    if(strcmp(vector_at(ctx->events, i).name, name) != 0)
    {
      continue;
    }

    vector_at(ctx->events, i).func(ctx, event);
  }
}

void *_GnWidgetAddComponent(GnWidget *ctx, size_t size, char *type, void (*initFunc)(GnWidget *, GnEvent *))
{
  void *rtn = NULL;
  struct GnComponentEntry entry = {0};

  rtn = _palloc(size, type);
  entry.data = rtn;
  entry.type = strdup(type);

  vector_push_back(ctx->components, entry);

/*
  GnWidgetAddEvent(ctx, "init", initFunc);
  GnWidgetEvent(ctx, "init", NULL);
*/
  initFunc(ctx, NULL);

  return rtn;
}

void *_GnWidgetComponent(GnWidget *ctx, char *type)
{
  size_t i = 0;

  for(i = 0; i < vector_size(ctx->components); i++)
  {
    if(strcmp(type, vector_at(ctx->components, i).type) == 0)
    {
      return vector_at(ctx->components, i).data;
    }
  }

  return NULL;
}

void GnWidgetAddEvent(GnWidget *ctx, char *name,
  void (*func)(GnWidget *, GnEvent *))
{
  struct GnEventEntry entry = {0};

  entry.name = strdup(name);
  entry.func = func;

  vector_push_back(ctx->events, entry);
}

