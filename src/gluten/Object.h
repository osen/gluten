#ifndef GN_OBJECT_H
#define GN_OBJECT_H

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

#endif
