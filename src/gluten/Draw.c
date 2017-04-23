#ifndef AMALGAMATION
  #include "Draw.h"
  #include "Image.h"
  #include "Event.h"
  #include "config.h"
  #include "gluten.h"
#endif

void GnDrawPixel(GnEvent *ctx, int x, int y, int r, int g, int b)
{
#ifdef USE_X11
  if(GnUnsafe.r != r || GnUnsafe.g != g || GnUnsafe.b != b)
  {
    GnUnsafe.r = r;
    GnUnsafe.g = g;
    GnUnsafe.b = b;
    XFreeColors(GnUnsafe.display, GnUnsafe.cmap, &GnUnsafe.color.pixel, 1, 0);
    GnUnsafe.color.flags = DoRed | DoGreen | DoBlue;
    GnUnsafe.color.red = (r / 255.0f) * 65535;
    GnUnsafe.color.green = (g / 255.0f) * 65535;
    GnUnsafe.color.blue = (b / 255.0f) * 65535;
    XAllocColor(GnUnsafe.display, GnUnsafe.cmap, &GnUnsafe.color);
    XSetForeground(GnUnsafe.display, GnUnsafe.gc, GnUnsafe.color.pixel);
    /*printf("Changing color\n");*/
  }

  XDrawPoint(GnUnsafe.display, GnUnsafe.window, GnUnsafe.gc,
    x, y);
#endif
#ifdef USE_SDL
  SDL_Rect rect = {0};
  rect.x = x;
  rect.y = y;
  rect.w = 1;
  rect.h = 1;
  SDL_FillRect(GnUnsafe.buffer, &rect,
    SDL_MapRGB(GnUnsafe.buffer->format, r, g, b));
#endif
}

void GnDrawFillRect(GnEvent *ctx, int x, int y, int width, int height,
  int r, int g, int b)
{
  size_t xi = 0;
  size_t yi = 0;

  for(yi = 0; yi < height; yi++)
  {
    for(xi = 0; xi < width; xi++)
    {
      GnDrawPixel(ctx, x + xi, y + yi, r, g, b);
    }
  }
}

void GnDrawImage(GnEvent *ctx, struct GnImage *img, int x, int y)
{
  size_t xi = 0;
  size_t yi = 0;
  size_t i = 0;

  for(yi = 0; yi < img->height; yi++)
  {
    for(xi = 0; xi < img->width; xi++)
    {
      GnDrawPixel(ctx, x + xi, y + yi, 
        vector_at(img->rawData, i),
        vector_at(img->rawData, i + 1),
        /*vector_at(img->rawData, i + 2));*/
        vector_at(img->rawData, i + 3));

      i+=4;
    }
  }
}
