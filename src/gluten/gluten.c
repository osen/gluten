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

  XSelectInput(GnUnsafe.display, GnUnsafe.window, ExposureMask | KeyPressMask | StructureNotifyMask);
  XMapWindow(GnUnsafe.display, GnUnsafe.window);
  GnUnsafe.gc = DefaultGC(GnUnsafe.display, GnUnsafe.screen);
#endif

  GnInternal.forms = vector_new(GnWidget *);

  GnInternal.mediumMono = GnImageCreateFromString(mediumMono);
  GnInternal.mediumMonoFont = GnFontCreateFromString(mediumMono);

  GnInternal.buffer = GnImageCreate(GN_INITIAL_WIDTH, GN_INITIAL_HEIGHT);
  GnInternal.lastBuffer = GnImageCreate(GN_INITIAL_WIDTH, GN_INITIAL_HEIGHT);

  /*GnRun();*/

  return 0;
}

void GnSubmitModifiedPixels(int force)
{
  size_t x = 0;
  size_t y = 0;

#ifdef USE_X11
  int xr = 0;
  int xg = 0;
  int xb = 0;
  GnUnsafe.cmap = XDefaultColormap(GnUnsafe.display, GnUnsafe.screen);
  GnUnsafe.color.flags = DoRed | DoGreen | DoBlue;
  GnUnsafe.color.red = 00000;
  GnUnsafe.color.green = 00000;
  GnUnsafe.color.blue = 00000;
  XAllocColor(GnUnsafe.display, GnUnsafe.cmap, &GnUnsafe.color);
  XSetForeground(GnUnsafe.display, GnUnsafe.gc, GnUnsafe.color.pixel);
#endif

#ifdef USE_SDL
  if(SDL_MUSTLOCK(GnUnsafe.buffer))
  {
    SDL_LockSurface(GnUnsafe.buffer);
  }
#endif

  for(y = 0; y < GnImageHeight(GnInternal.buffer); y++)
  {
    for(x = 0; x < GnImageWidth(GnInternal.buffer); x++)
    {
      GnColor a = GnImagePixel(GnInternal.buffer, x, y);
      GnColor b = GnImagePixel(GnInternal.lastBuffer, x, y);

      if(force != 0 || a.r != b.r || a.g != b.g || a.b != b.b || a.a != b.a)
      {
#ifdef USE_X11
        if(xr != a.r || xg != a.g || xb != a.b)
        {
          xr = a.r;
          xg = a.g;
          xb = a.b;
          XFreeColors(GnUnsafe.display, GnUnsafe.cmap, &GnUnsafe.color.pixel, 1, 0);
          GnUnsafe.color.flags = DoRed | DoGreen | DoBlue;
          GnUnsafe.color.red = (a.r / 255.0f) * 65535;
          GnUnsafe.color.green = (a.g / 255.0f) * 65535;
          GnUnsafe.color.blue = (a.b / 255.0f) * 65535;
          XAllocColor(GnUnsafe.display, GnUnsafe.cmap, &GnUnsafe.color);
          XSetForeground(GnUnsafe.display, GnUnsafe.gc, GnUnsafe.color.pixel);
        }

        XDrawPoint(GnUnsafe.display, GnUnsafe.window, GnUnsafe.gc, x, y);
#endif
#ifdef USE_SDL
/*
        SDL_Rect rect = {0};
        rect.x = x;
        rect.y = y;
        rect.w = 1;
        rect.h = 1;
        SDL_FillRect(GnUnsafe.buffer, &rect,
          SDL_MapRGB(GnUnsafe.buffer->format, a.r, a.g, a.b));
*/
        Uint32 color = SDL_MapRGB(GnUnsafe.buffer->format, a.r, a.g, a.b);
        Uint8 *pixel = (Uint8*)GnUnsafe.buffer->pixels;
        pixel += (y * GnUnsafe.buffer->pitch) + (x * sizeof(Uint32));
        *((Uint32*)pixel) = color;
#endif
        GnImageSetPixel(GnInternal.lastBuffer, x, y, a.r, a.g, a.b, a.a);
      }
    }
  }

#ifdef USE_SDL
  if(SDL_MUSTLOCK(GnUnsafe.buffer))
  {
    SDL_UnlockSurface(GnUnsafe.buffer);
  }

  SDL_BlitSurface(GnUnsafe.buffer, NULL, GnUnsafe.screen, NULL);
  SDL_Flip(GnUnsafe.screen);
#endif
#ifdef USE_X11
  XFreeColors(GnUnsafe.display, GnUnsafe.cmap, &GnUnsafe.color.pixel, 1, 0);
#endif
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

  if(strcmp(eventName, "request_draw") == 0)
  {
    GnDraw *draw = GnEventAddComponent(event, GnDraw);
    draw->bounds.width = GnImageWidth(GnInternal.buffer);
    draw->bounds.height = GnImageHeight(GnInternal.buffer);
  }
  else if(strcmp(eventName, "size") == 0)
  {

  }

  for(i = 0; i < vector_size(GnInternal.forms); i++)
  {
    GnWidgetEvent(vector_at(GnInternal.forms, i), eventName, event);
  }

  if(strcmp(eventName, "request_draw") == 0)
  {
    GnDraw *draw = GnEventComponent(event, GnDraw);

    if(draw->dirty)
    {
      GnSubmitModifiedPixels(0);
      draw->dirty = 0;
    }
  }

  GnEventDestroy(event);
}

void GnRun()
{
  GnInternal.running = 1;
  GnPropagateEvent("request_draw");

#ifdef USE_SDL
  while(GnInternal.running)
  {
    SDL_Event event = {0};

    if(!SDL_WaitEvent(&event))
    {
      GnInternal.running = 0;
      break;
    }

    if(event.type == SDL_MOUSEMOTION)
    {

    }
    else if(event.type == SDL_VIDEORESIZE)
    {
      GnUnsafe.screen = SDL_SetVideoMode(event.resize.w, event.resize.h, 32, SDL_SWSURFACE | SDL_RESIZABLE);

      if(!GnUnsafe.screen)
      {
        GnInternal.running = 0;
        break;
      }

      SDL_FreeSurface(GnUnsafe.buffer);
      GnUnsafe.buffer = SDL_DisplayFormatAlpha(GnUnsafe.screen);
      GnImageDestroy(GnInternal.buffer);
      GnImageDestroy(GnInternal.lastBuffer);
      GnInternal.buffer = GnImageCreate(event.resize.w, event.resize.h);
      GnInternal.lastBuffer = GnImageCreate(event.resize.w, event.resize.h);
      GnPropagateEvent("size");
      GnPropagateEvent("request_draw");
    }
    else if(event.type == SDL_VIDEOEXPOSE)
    {
      GnSubmitModifiedPixels(1);
    }
    else if(event.type == SDL_QUIT)
    {
      GnInternal.running = 0;
      GnPropagateEvent("quit");
    }
    else
    {
      /*GnPropagateEvent("request_draw");*/
    }
  }
#endif
#ifdef USE_X11
  do
  {
    Atom wmDeleteMessage = XInternAtom(GnUnsafe.display,
      "WM_DELETE_WINDOW", False);

    XSetWMProtocols(GnUnsafe.display, GnUnsafe.window, &wmDeleteMessage, 1);

    while(GnInternal.running)
    {
      XEvent e = {0};

      XNextEvent(GnUnsafe.display, &e);

      if(e.type == Expose)
      {
        /* TODO: Use e.xexpose.x, with, height, y to invalidate lastBuffer
           section rather than clearing entire thing */
        GnSubmitModifiedPixels(1);
      }
      else if(e.type == ConfigureNotify)
      {
        if(GnImageWidth(GnInternal.buffer) != e.xconfigure.width ||
          GnImageHeight(GnInternal.buffer) != e.xconfigure.height)
        {
          GnImageDestroy(GnInternal.buffer);
          GnImageDestroy(GnInternal.lastBuffer);
          GnInternal.buffer = GnImageCreate(e.xconfigure.width, e.xconfigure.height);
          GnInternal.lastBuffer = GnImageCreate(e.xconfigure.width, e.xconfigure.height);
          GnPropagateEvent("size");
          GnPropagateEvent("request_draw");
        }
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

  GnFontDestroy(GnInternal.mediumMonoFont);
  GnImageDestroy(GnInternal.mediumMono);
  GnImageDestroy(GnInternal.buffer);
  GnImageDestroy(GnInternal.lastBuffer);

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

