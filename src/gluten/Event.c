#ifndef AMALGAMATION
  #include "Event.h"
  #include <palloc.h>
#endif

#include <string.h>

GnEvent *GnEventCreate()
{
  GnEvent *rtn = NULL;

  rtn = palloc(GnEvent);
  rtn->components = vector_new(struct GnComponentEntry);

  return rtn;
}

void GnEventDestroy(GnEvent *ctx)
{
  size_t i = 0;

  for(i = 0; i < vector_size(ctx->components); i++)
  {
    pfree(vector_at(ctx->components, i).data);
    free(vector_at(ctx->components, i).type);
  }

  vector_delete(ctx->components);
  pfree(ctx);
}

void *_GnEventAddComponent(GnWidget *ctx, size_t size, char *type)
{ 
  void *rtn = NULL;
  struct GnComponentEntry entry = {0};

  rtn = _palloc(size, type);
  entry.data = rtn;
  entry.type = strdup(type);

  vector_push_back(ctx->components, entry);

  return rtn;
}
