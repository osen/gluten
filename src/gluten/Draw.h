#ifndef GLUTEN_DRAW_H
#define GLUTEN_DRAW_H

#ifndef AMALGAMATION
  #include "Object.h"
#endif

struct GnImage;

typedef struct GnDraw GnDraw;

struct GnDraw
{
  struct
  {
    int x;
    int y;
    int width;
    int height;
  } bounds;

  struct
  {
    int x;
    int y;
  } offset;
};

void GnDrawPixel(GnEvent *ctx, int x, int y, int r, int g, int b);

void GnDrawImage(GnEvent *ctx, struct GnImage *img, int x, int y);

void GnDrawFillRect(GnEvent *ctx, int x, int y, int width, int height,
  int r, int g, int b);

#endif
