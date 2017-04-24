#ifndef GLUTEN_IMAGE_H
#define GLUTEN_IMAGE_H

#ifndef AMALGAMATION
  #include "config.h"
  #include "Object.h"
  #include <vector.h>
#endif

#ifdef USE_SDL
  #include <SDL/SDL.h>
#endif
#ifdef USE_X11
  #include <X11/Xlib.h>
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
#ifdef USE_SDL
  SDL_Surface *surface;
#endif
#ifdef USE_X11
  XImage *img;
  Pixmap p;
#endif
};

GnImage *GnImageCreateFromString(char *str);
GnImage *GnImageCreate(int width, int height);
void GnImageDestroy(GnImage *ctx);
int GnImageWidth(GnImage *ctx);
int GnImageHeight(GnImage *ctx);
GnColor GnImagePixel(GnImage *ctx, int x, int y);
void GnImageSetPixel(GnImage *ctx, int x, int y, int r, int g, int b, int a);

#endif
