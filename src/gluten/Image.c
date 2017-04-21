#ifndef AMALGAMATION
  #include "Image.h"
  #include "util.h"
  #include <lodepng/lodepng.h>
  #include <palloc.h>
#endif

GnImage *GnImageCreateFromString(char *str)
{
  GnImage *rtn = NULL;
  unsigned error = 0;
  unsigned width = 0;
  unsigned height = 0;
  unsigned char *data = NULL;

  rtn = palloc(GnImage);
  rtn->rawData = vector_new(unsigned char);

  GnHexArrayFromString(str, rtn->rawData);

  error = lodepng_decode32(&data, &width, &height, vector_raw(rtn->rawData), vector_size(rtn->rawData));

  if(error)
  {
    /* printf("error %u: %s\n", error, lodepng_error_text(error)); */
    vector_delete(rtn->rawData);
    pfree(rtn);
    return NULL;
  }

  printf("Image: %i %i\n", width, height);

  return rtn;
}
