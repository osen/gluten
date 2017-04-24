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

GnColor GnImagePixel(GnImage *ctx, int x, int y)
{
  GnColor rtn = {0};
  size_t i = 0;

  if(x >= ctx->width || x < 0 || y >= ctx->height || y < 0)
  {
    printf("Error: Out of bounds of image\n");
    return rtn;
  }

  i = y * ctx->width * 4 + x * 4;

  rtn.r = vector_at(ctx->rawData, i);
  rtn.g = vector_at(ctx->rawData, i + 1);
  rtn.b = vector_at(ctx->rawData, i + 2);
  rtn.a = vector_at(ctx->rawData, i + 3);

  return rtn;
}

void GnImageSetPixel(GnImage *ctx, int x, int y, int r, int g, int b, int a)
{
  size_t i = 0;

  if(x >= ctx->width || x < 0 || y >= ctx->height || y < 0)
  {
    printf("Error: Out of bounds of image\n");
    return;
  }

  i = y * ctx->width * 4 + x * 4;

  vector_set(ctx->rawData, i, r);
  vector_set(ctx->rawData, i + 1, g);
  vector_set(ctx->rawData, i + 2, b);
  vector_set(ctx->rawData, i + 3, a);
}
