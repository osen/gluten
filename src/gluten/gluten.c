#ifndef AMALGAMATION
  #include "gluten.h"
  #include "data.h"
  #include "Position.h"
  #include <palloc.h>
#endif

#include <string.h>

#define GN_INITIAL_WIDTH 320
#define GN_INITIAL_HEIGHT 240

struct GnInternal GnInternal;
struct GnUnsafe GnUnsafe;

int GnInit(int argc, char **argv, char *layout)
{
#ifdef USE_SDL
  if(SDL_Init(SDL_INIT_EVERYTHING) == -1)
  {
    return 1;
  }

  GnUnsafe.screen = SDL_SetVideoMode(GN_INITIAL_WIDTH, GN_INITIAL_HEIGHT, 32, SDL_SWSURFACE | SDL_RESIZABLE);

  if(!GnUnsafe.screen)
  {
    return 1;
  }
#endif

  GnInternal.forms = vector_new(GnWidget *);

  GnInternal.mediumMono = GnImageCreateFromString(mediumMono);

  /*GnRun();*/

  return 0;
}

void GnPropagateEvent(char *event)
{
  size_t i = 0;

  if(!GnInternal.activeForm)
  {
    return;
  }

  for(i = 0; i < vector_size(GnInternal.forms); i++)
  {
    GnWidgetEvent(vector_at(GnInternal.forms, i), event, NULL);
  }
}

void GnRun()
{
  GnInternal.running = 1;

#ifdef USE_SDL
  while(GnInternal.running)
  {
    SDL_Event event = {0};

    if(!SDL_WaitEvent(&event))
    {
      GnInternal.running = 0;
      break;
    }

    if(event.type == SDL_VIDEORESIZE)
    {
      GnUnsafe.screen = SDL_SetVideoMode(event.resize.w, event.resize.h, 32, SDL_SWSURFACE | SDL_RESIZABLE);

      if(!GnUnsafe.screen)
      {
        GnInternal.running = 0;
        break;
      }

      GnPropagateEvent("size");
    }
    else if(event.type == SDL_QUIT)
    {
      GnInternal.running = 0;
      GnPropagateEvent("quit");
    }

    GnPropagateEvent("draw");
    SDL_Flip(GnUnsafe.screen);
  }
#endif
}

void GnCleanup()
{
  size_t i = 0;

  for(i = 0; i < vector_size(GnInternal.forms); i++)
  {
    GnWidgetDestroy(vector_at(GnInternal.forms, i));
  }

  vector_delete(GnInternal.forms);

#ifdef USE_SDL
  SDL_Quit();
#endif

  memset(&GnInternal, 0, sizeof(GnInternal));
  memset(&GnUnsafe, 0, sizeof(GnUnsafe));
}

