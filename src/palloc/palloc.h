#ifndef PALLOC_H
#define PALLOC_H

#include <stdlib.h>

//#define PALLOC_DEBUG

void pool_cleanup();
void pfree(void *ptr);
void *_palloc(size_t size, char *type);

#define palloc(T) \
  (T*)_palloc(sizeof(T), #T)

#endif
