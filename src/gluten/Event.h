#ifndef GLUTEN_EVENT_H
#define GLUTEN_EVENT_H

#ifndef AMALGAMATION
  #include "Object.h"
#endif

void *_GnEventAddComponent(GnWidget *ctx, size_t size, char *type);
#define GnEventAddComponent(W, T) \
  (T*)_GnEventAddComponent(W, sizeof(T), #T)

void *_GnWidgetComponent(GnWidget *ctx, char *type);
#define GnEventComponent(W, T) \
  (T*)_GnWidgetComponent(W, #T)

GnEvent *GnEventCreate();
void GnEventDestroy(GnEvent *ctx);

#endif
