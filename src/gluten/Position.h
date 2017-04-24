#ifndef GLUTEN_POSITION_H
#define GLUTEN_POSITION_H

#ifndef AMALGAMATION
  #include "Object.h"
#endif

typedef struct GnPosition GnPosition;

void GnPositionInit(GnWidget *ctx, GnEvent *event);
void GnWidgetSetPosition(GnWidget *ctx, int x, int y);
void GnWidgetSetSize(GnWidget *ctx, int width, int height);
int GnWidgetX(GnWidget *ctx);
int GnWidgetY(GnWidget *ctx);
int GnWidgetWidth(GnWidget *ctx);
int GnWidgetHeight(GnWidget *ctx);

struct GnPosition
{
  int x;
  int y;
  int width;
  int height;
};

#endif
