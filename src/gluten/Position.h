#ifndef GLUTEN_POSITION_H
#define GLUTEN_POSITION_H

#ifndef AMALGAMATION
  #include "Object.h"
#endif

typedef struct GnPosition GnPosition;

void GnPositionInit(GnWidget *ctx, GnEvent *event);
void GnWidgetSetPosition(GnWidget *ctx, int x, int y);
void GnWidgetSetSize(GnWidget *ctx, int width, int height);

struct GnPosition
{
  int x;
  int y;
  int width;
  int height;
};

#endif
