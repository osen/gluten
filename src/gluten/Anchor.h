#ifndef GLUTEN_ANCHOR_H
#define GLUTEN_ANCHOR_H

typedef struct GnObject GnWidget;
typedef struct GnObject GnEvent;
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
