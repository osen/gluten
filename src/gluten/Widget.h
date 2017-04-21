#ifndef GLUTEN_WIDGET_H
#define GLUTEN_WIDGET_H

#ifndef AMALGAMATION
  #include <vector.h>
#endif

typedef struct GnObject GnWidget;
typedef struct GnObject GnEvent;

struct GnComponentEntry
{
  char *type;
  void *data;
};

struct GnEventEntry
{
  char *name;
  void (*func)(GnWidget *, GnEvent *);
};

struct GnObject
{
  vector(struct GnComponentEntry) *components;
  vector(struct GnEventEntry) *events;
};

GnWidget *_GnWidgetCreate(size_t size, char *type, void (*initFunc)(GnWidget *, GnEvent *));
#define GnWidgetCreate(T) \
  _GnWidgetCreate(sizeof(T), #T, T##Init)

void *_GnWidgetAddComponent(GnWidget *ctx, size_t size, char *type, void (*initFunc)(GnWidget *, GnEvent *));
#define GnWidgetAddComponent(W, T) \
  (T*)_GnWidgetAddComponent(W, sizeof(T), #T, T##Init)

void *_GnWidgetComponent(GnWidget *ctx, char *type);
#define GnWidgetComponent(W, T) \
  (T*)_GnWidgetComponent(W, #T)

void GnWidgetAddEvent(GnWidget *ctx, char *name,
  void (*func)(GnWidget *, GnEvent *));

void GnWidgetEvent(GnWidget *ctx, char *name, GnEvent *event);

void GnWidgetDestroy(GnWidget *ctx);

#endif
