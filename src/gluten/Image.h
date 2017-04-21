#ifndef GLUTEN_IMAGE_H
#define GLUTEN_IMAGE_H

#ifndef AMALGAMATION
  #include "config.h"
  #include <vector.h>
#endif

#ifdef USE_SDL
  #include <SDL/SDL.h>
#endif

typedef struct GnImage GnImage;

struct GnImage
{
  vector(unsigned char) *rawData;
  int width;
  int height;
#ifdef USE_SDL
  SDL_Surface *surface;
#endif
};

GnImage *GnImageCreateFromString(char *str);
void GnImageDestroy(GnImage *ctx);

#endif
