#ifndef GLUTEN_ANCHOR_H
#define GLUTEN_ANCHOR_H

#ifndef AMALGAMATION
  #include "Object.h"
#endif

typedef struct GnAnchor GnAnchor;

void GnAnchorInit(GnWidget *ctx, GnEvent *event);

struct GnAnchor
{
  int top;
  int bottom;
  int left;
  int right;
};

#endif
