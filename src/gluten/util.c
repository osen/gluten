#ifndef AMALGAMATION
  #include "util.h"
#endif

#include <stdlib.h>
#include <string.h>

void GnHexArrayFromString(char *input, vector(unsigned char) *output)
{
  size_t len = strlen(input);
  size_t i = 0;

  for(i = 0; i < len - 1; i+=2)
  {
    char num[3] = {0};

    num[0] = input[i];
    num[1] = input[i + 1];

    vector_push_back(output, strtoul(num, NULL, 16));
  }
}
