#ifndef GLUTEN_IMAGE_H
#define GLUTEN_IMAGE_H

#ifndef AMALGAMATION
  #include "config.h"
  #include "Object.h"
  #include <vector.h>
#endif

typedef struct GnImage GnImage;
typedef struct GnColor GnColor;

struct GnColor
{
  int r;
  int g;
  int b;
  int a;
};

struct GnImage
{
  vector(unsigned char) *rawData;
  int width;
  int height;
};

GnImage *GnImageCreateFromString(char *str);
GnImage *GnImageCreate(int width, int height);
void GnImageDestroy(GnImage *ctx);
int GnImageWidth(GnImage *ctx);
int GnImageHeight(GnImage *ctx);
GnColor GnImagePixel(GnImage *ctx, int x, int y);
void GnImageSetPixel(GnImage *ctx, int x, int y, int r, int g, int b, int a);

#endif
