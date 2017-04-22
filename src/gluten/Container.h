#ifndef GLUTEN_CONTAINER_H
#define GLUTEN_CONTAINER_H

#ifndef AMALGAMATION
  #include "Object.h"
  #include <vector.h>
#endif

typedef struct GnContainer GnContainer;

void GnContainerInit(GnWidget *ctx, GnEvent *event);
void GnContainerAdd(GnWidget *ctx, GnWidget *child);

struct GnContainer
{
  vector(GnWidget *) *children;
};

#endif
