#ifndef GLUTEN_CONTAINER_H
#define GLUTEN_CONTAINER_H

#ifndef AMALGAMATION
  #include <vector.h>
#endif

typedef struct GnObject GnWidget;
typedef struct GnObject GnEvent;
typedef struct GnContainer GnContainer;

void GnContainerInit(GnWidget *ctx, GnEvent *event);
void GnContainerAdd(GnWidget *ctx, GnWidget *child);

struct GnContainer
{
  vector(GnWidget *) *children;
};

#endif
