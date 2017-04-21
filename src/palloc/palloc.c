#ifndef AMALGAMATION
  #include "palloc.h"
#endif

#include <stdio.h>
#include <string.h>

static int registered;

struct PoolEntry
{
  void *ptr;
  size_t size;
  char *type;
  int used;
  struct PoolEntry *next;
};

struct PoolEntry *poolHead;

void pool_cleanup()
{
  struct PoolEntry *curr = poolHead;

  while(curr)
  {
    struct PoolEntry *tmp = curr;

    if(curr->used)
    {
      printf("Leak\n");
      printf("  Type: %s\n", curr->type);
      printf("  Size: %i\n", (int)curr->size);
      printf("  Used: %i\n", curr->used);
    }

    curr = curr->next;

    free(tmp->ptr);
    free(tmp->type);
    free(tmp);
  }

  poolHead = NULL;
}

void pfree(void *ptr)
{
  struct PoolEntry *entry = poolHead;

  while(entry)
  {
    if(entry->ptr == ptr)
    {
      if(!entry->used)
      {
        printf("Error: Memory already freed\n");
      }

#ifdef PALLOC_DEBUG
      printf("Freeing: %s\n", entry->type);
#endif
      memset(entry->ptr, 0, entry->size);
      entry->used = 0;
      return;
    }

    entry = entry->next;
  }

  printf("Error: Memory not managed by pool\n");
}

void *_palloc(size_t size, char *type)
{
  struct PoolEntry *entry = poolHead;

  if(!registered)
  {
    registered = 1;
    atexit(pool_cleanup);
  }

  while(entry)
  {
    if(!entry->used && strcmp(type, entry->type) == 0)
    {
      char *ptr = (char *)entry->ptr;
      size_t i = 0;

#ifdef PALLOC_DEBUG
      printf("Reusing: %s\n", type);
#endif

      for(i = 0; i < entry->size; i++)
      {
        if(*ptr != 0)
        {
          printf("Error: Memory is dirty\n");
        }

        ptr++;
      }

      entry->used = 1;

      return entry->ptr;
    }

    entry = entry->next;
  }

#ifdef PALLOC_DEBUG
  printf("Allocating: %s\n", type);
#endif
  entry = calloc(1, sizeof(*entry));
  if(!entry) return NULL;

  entry->ptr = calloc(1, size);

  if(!entry->ptr)
  {
    free(entry);
    return NULL;
  }

  entry->size = size;
  entry->type = calloc(strlen(type) + 1, sizeof(char));
  strcpy(entry->type, type);
  entry->used = 1;

  if(!poolHead)
  {
    poolHead = entry;
  }
  else
  {
    struct PoolEntry *curr = poolHead;

    while(curr->next) curr = curr->next;
    curr->next = entry;
  }

  return entry->ptr;
}

