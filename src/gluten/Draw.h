#ifndef GLUTEN_DRAW_H
#define GLUTEN_DRAW_H

#ifndef AMALGAMATION
  #include "Object.h"
#endif

struct GnImage;
struct GnFont;

typedef struct GnDraw GnDraw;
typedef struct GnBounds GnBounds;

struct GnBounds
{
  int x;
  int y;
  int width;
  int height;
};

struct GnDraw
{
  GnBounds bounds;

  struct
  {
    int x;
    int y;
  } offset;
};

void GnDrawPixel(GnEvent *ctx, int x, int y, int r, int g, int b, int a);

void GnDrawImage(GnEvent *ctx, struct GnImage *img, int x, int y);

void GnDrawFillRect(GnEvent *ctx, int x, int y, int width, int height,
  int r, int g, int b, int a);

void GnDrawText(GnEvent *ctx, struct GnFont *font, int x, int y, char *text);

#endif
