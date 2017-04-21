#ifndef GLUTEN_POSITION_H
#define GLUTEN_POSITION_H

typedef struct GnPosition GnPosition;
typedef struct GnObject GnWidget;
typedef struct GnObject GnEvent;

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
