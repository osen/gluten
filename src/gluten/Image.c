#ifndef AMALGAMATION
  #include "Image.h"
  #include "util.h"
  #include "gluten.h"
  #include <lodepng/lodepng.h>
  #include <palloc.h>
#endif

#ifdef USE_SDL
  #include <SDL/SDL.h>

void set_pixel(SDL_Surface *surface, int x, int y, Uint32 pixel)
{
  Uint8 *target_pixel = (Uint8 *)surface->pixels + y * surface->pitch + x * 4;
  *(Uint32 *)target_pixel = pixel;
}
#endif

GnImage *GnImageCreateFromString(char *str)
{
  GnImage *rtn = NULL;
  unsigned error = 0;
  unsigned width = 0;
  unsigned height = 0;
  int x = 0;
  int y = 0;
  size_t i = 0;

  rtn = palloc(GnImage);
  rtn->rawData = vector_new(unsigned char);

  GnHexArrayFromString(str, rtn->rawData);

  error = lodepng_decode32(&GnUnsafe.pngData, &width, &height, vector_raw(rtn->rawData), vector_size(rtn->rawData));

  if(error)
  {
    /* printf("error %u: %s\n", error, lodepng_error_text(error)); */
    vector_delete(rtn->rawData);
    pfree(rtn);
    return NULL;
  }

  /* printf("Image: %i %i\n", width, height); */
  rtn->width = width;
  rtn->height = height;

  vector_resize(rtn->rawData, width * height * 4);

  for(i = 0; i < width * height * 4; i++)
  {
    vector_set(rtn->rawData, i, GnUnsafe.pngData[i]);
  }

#ifdef USE_SDL
  rtn->surface = SDL_CreateRGBSurface(0, width, height, 32,
    GnUnsafe.screen->format->Rmask,
    GnUnsafe.screen->format->Gmask,
    GnUnsafe.screen->format->Bmask,
    GnUnsafe.screen->format->Amask);

  i = 0;

  for(y = 0; y < height; y++)
  {
    for(x = 0; x < width; x++)
    {
      set_pixel(rtn->surface, x, y, SDL_MapRGBA(rtn->surface->format, GnUnsafe.pngData[i], GnUnsafe.pngData[i+1], GnUnsafe.pngData[i+2], GnUnsafe.pngData[i+3]));
      i+=4;
    }
  }
#endif

  free(GnUnsafe.pngData); GnUnsafe.pngData = NULL;

  return rtn;
}

void GnImageDestroy(GnImage *ctx)
{
#ifdef USE_SDL
  SDL_FreeSurface(ctx->surface);
#endif
  vector_delete(ctx->rawData);
  pfree(ctx);
}
