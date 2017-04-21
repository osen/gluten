#ifndef GLUTEN_GLUTEN_H
#define GLUTEN_GLUTEN_H

#ifndef AMALGAMATION
  #include "config.h"
  #include "Widget.h"
  #include "Container.h"
  #include "Anchor.h"
  #include "Position.h"
  #include "Form.h"
  #include "Button.h"
  #include "Label.h"
  #include "Image.h"
  #include <vector.h>
#endif

#ifdef USE_SDL
  #include <SDL/SDL.h>
#endif

typedef struct GnObject GnEvent;

struct GnInternal
{
  int running;
  vector(GnWidget *) *forms;
  GnImage *mediumMono;
  GnWidget *activeForm;
};
extern struct GnInternal GnInternal;

struct GnUnsafe
{
  int dummy;
  unsigned char *pngData;
#ifdef USE_SDL
  SDL_Surface *screen;
#endif
};
extern struct GnUnsafe GnUnsafe;

int GnInit(int argc, char **argv, char *layout);
void GnRun();
void GnCleanup();

struct GnClick
{
  int clickEventStuff;
};

#endif
