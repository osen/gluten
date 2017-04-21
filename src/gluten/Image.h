#ifndef GLUTEN_IMAGE_H
#define GLUTEN_IMAGE_H

#ifndef AMALGAMATION
  #include <vector.h>
#endif

typedef struct GnImage GnImage;

struct GnImage
{
  vector(unsigned char) *rawData;
};

GnImage *GnImageCreateFromString(char *str);

#endif
