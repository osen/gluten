#ifndef AMALGAMATION
  #include "Image.h"
  #include "util.h"
  #include "gluten.h"
  #include <lodepng/lodepng.h>
  #include <palloc.h>
#endif

/*
void set_pixel(SDL_Surface *surface, int x, int y, Uint32 pixel)
{
  Uint8 *target_pixel = (Uint8 *)surface->pixels + y * surface->pitch + x * 4;
  *(Uint32 *)target_pixel = pixel;
}
*/

GnImage *GnImageCreate(int width, int height)
{
  GnImage *rtn = NULL;

  rtn = palloc(GnImage);
  rtn->width = width;
  rtn->height = height;
  rtn->rawData = vector_new(unsigned char);
  vector_resize(rtn->rawData, width * height * 4);

  return rtn;
}

GnImage *GnImageCreateFromString(char *str)
{
  GnImage *rtn = NULL;
  unsigned error = 0;
  unsigned width = 0;
  unsigned height = 0;
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

  free(GnUnsafe.pngData); GnUnsafe.pngData = NULL;

  return rtn;
}

void GnImageDestroy(GnImage *ctx)
{
  vector_delete(ctx->rawData);
  pfree(ctx);
}

int GnImageWidth(GnImage *ctx)
{
  return ctx->width;
}

int GnImageHeight(GnImage *ctx)
{
  return ctx->height;
}

