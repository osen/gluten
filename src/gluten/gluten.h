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
  #include "Event.h"
  #include "Object.h"
  #include <vector.h>
#endif

#ifdef USE_SDL
  #include <SDL/SDL.h>
#endif
#ifdef USE_X11
  #include <X11/Xlib.h>
#endif

struct GnInternal
{
  int running;
  vector(GnWidget *) *forms;
  GnImage *mediumMono;
  GnWidget *activeForm;

  GnImage *buffer;
  GnImage *lastBuffer;
};
extern struct GnInternal GnInternal;

struct GnUnsafe
{
  int dummy;
  unsigned char *pngData;
#ifdef USE_SDL
  SDL_Surface *screen;
  SDL_Surface *buffer;
#endif
#ifdef USE_X11
  Display *display;
  int screen;
  Window window;
  GC gc;
  Colormap cmap;
  XColor color;
  int r; int g; int b;
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
