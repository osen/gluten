#ifndef AMALGAMATION
  #include "gluten.h"
  #include "data.h"
  #include "Position.h"
  #include "Event.h"
  #include "Draw.h"
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

  GnUnsafe.buffer = SDL_DisplayFormatAlpha(GnUnsafe.screen);
#endif
#ifdef USE_X11
  GnUnsafe.display = XOpenDisplay(NULL);

  if(!GnUnsafe.display)
  {
    return 1;
  }

  GnUnsafe.screen = DefaultScreen(GnUnsafe.display);

  GnUnsafe.window = XCreateSimpleWindow(GnUnsafe.display,
    RootWindow(GnUnsafe.display, GnUnsafe.screen),
    0, 0, GN_INITIAL_WIDTH, GN_INITIAL_HEIGHT, 1,
    BlackPixel(GnUnsafe.display, GnUnsafe.screen),
    WhitePixel(GnUnsafe.display, GnUnsafe.screen));

  XSelectInput(GnUnsafe.display, GnUnsafe.window, ExposureMask | KeyPressMask);
  XMapWindow(GnUnsafe.display, GnUnsafe.window);
  GnUnsafe.gc = DefaultGC(GnUnsafe.display, GnUnsafe.screen);
/*
  XGCValues gcvalues = {0};
  GnUnsafe.gc = XCreateGC(GnUnsafe.display, GnUnsafe.window, 0, &gcvalues);
*/
  GnUnsafe.cmap = XDefaultColormap(GnUnsafe.display, GnUnsafe.screen);
  GnUnsafe.color.flags = DoRed | DoGreen | DoBlue;
  GnUnsafe.color.red = 00000;
  GnUnsafe.color.green = 00000;
  GnUnsafe.color.blue = 00000;
  XAllocColor(GnUnsafe.display, GnUnsafe.cmap, &GnUnsafe.color);
  XSetForeground(GnUnsafe.display, GnUnsafe.gc, GnUnsafe.color.pixel);
#endif

  GnInternal.forms = vector_new(GnWidget *);

  GnInternal.mediumMono = GnImageCreateFromString(mediumMono);

  /*GnRun();*/

  return 0;
}

void GnPropagateEvent(char *eventName)
{
  size_t i = 0;
  GnEvent *event = NULL;

  if(!GnInternal.activeForm)
  {
    return;
  }

  event = GnEventCreate();

  if(strcmp(eventName, "draw") == 0)
  {
    GnDraw * draw = GnEventAddComponent(event, GnDraw);
    draw->bounds.width = 320;
    draw->bounds.height = 240;
  }

  for(i = 0; i < vector_size(GnInternal.forms); i++)
  {
    GnWidgetEvent(vector_at(GnInternal.forms, i), eventName, event);
  }

  GnEventDestroy(event);
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

      SDL_FreeSurface(GnUnsafe.buffer);
      GnUnsafe.buffer = SDL_DisplayFormatAlpha(GnUnsafe.screen);
      GnPropagateEvent("size");
    }
    else if(event.type == SDL_QUIT)
    {
      GnInternal.running = 0;
      GnPropagateEvent("quit");
    }

    GnPropagateEvent("draw");

    SDL_BlitSurface(GnUnsafe.buffer, NULL, GnUnsafe.screen, NULL);
    SDL_Flip(GnUnsafe.screen);
  }
#endif
#ifdef USE_X11
  do {
  Atom wmDeleteMessage = XInternAtom(GnUnsafe.display,
    "WM_DELETE_WINDOW", False);
  XSetWMProtocols(GnUnsafe.display, GnUnsafe.window, &wmDeleteMessage, 1);

  while(GnInternal.running)
  {
    XEvent e = {0};

    XNextEvent(GnUnsafe.display, &e);

    if(e.type == Expose)
    {
      GnPropagateEvent("draw");
    }
    else if(e.type == KeyPress)
    {
      GnInternal.running = 0;
    }
    else if(e.type == ClientMessage)
    {
      if(e.xclient.data.l[0] == wmDeleteMessage)
      {
        GnInternal.running = 0;
      }
    }
  }
  } while(0);
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

  GnImageDestroy(GnInternal.mediumMono);

#ifdef USE_SDL
  SDL_FreeSurface(GnUnsafe.buffer);
  SDL_Quit();
#endif
#ifdef USE_X11
  XCloseDisplay(GnUnsafe.display);
#endif

  memset(&GnInternal, 0, sizeof(GnInternal));
  memset(&GnUnsafe, 0, sizeof(GnUnsafe));
}

