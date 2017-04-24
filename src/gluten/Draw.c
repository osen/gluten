#ifndef AMALGAMATION
  #include "Draw.h"
  #include "Image.h"
  #include "Event.h"
  #include "Font.h"
  #include "config.h"
  #include "gluten.h"
#endif

#include <string.h>

void GnDrawPixel(GnEvent *ctx, int x, int y, int r, int g, int b, int a)
{
  GnDraw *draw = GnEventComponent(ctx, GnDraw);
  GnColor curr = {0};
  int rdiff = 0;
  int gdiff = 0;
  int bdiff = 0;
  float rstep = 0;
  float gstep = 0;
  float bstep = 0;
  float opacity = 0;

  x += draw->bounds.x;
  y += draw->bounds.y;

  if(x < draw->bounds.x ||
    x > draw->bounds.x + draw->bounds.width ||
    y < draw->bounds.y ||
    y > draw->bounds.y + draw->bounds.height)
  {
    return;
  }

  if(x >= GnImageWidth(GnInternal.buffer) ||
    y >= GnImageHeight(GnInternal.buffer) ||
    x < 0 || y < 0)
  {
    return;
  }

  if(a != 255)
  {
    opacity = ((float)a / 255.0f * 100.0f);
    curr = GnImagePixel(GnInternal.buffer, x, y);
    rdiff = r - curr.r;
    gdiff = g - curr.g;
    bdiff = b - curr.b;

    rstep = (float)rdiff / 100.0f;
    gstep = (float)gdiff / 100.0f;
    bstep = (float)bdiff / 100.0f;

    r = curr.r + rstep * opacity;
    g = curr.g + gstep * opacity;
    b = curr.b + bstep * opacity;
    a = 255;
  }

  GnImageSetPixel(GnInternal.buffer, x, y, r, g, b, a);
}

void GnDrawFillRect(GnEvent *ctx, int x, int y, int width, int height,
  int r, int g, int b, int a)
{
  size_t xi = 0;
  size_t yi = 0;

  for(yi = 0; yi < height; yi++)
  {
    for(xi = 0; xi < width; xi++)
    {
      GnDrawPixel(ctx, x + xi, y + yi, r, g, b, a);
    }
  }
}

void GnDrawText(GnEvent *ctx, struct GnFont *font, int x, int y, char *text)
{
  size_t xi = 0;
  size_t yi = 0;
  size_t i = 0;
  int colMod[4] = {GN_WIDGET_FOREGROUND};
  /*int colMod[4] = {0};*/

  for(i = 0; i < strlen(text); i++)
  {
    GnGlyph g = GnFontGlyph(font, text[i]);
    GnImage *img = g.img;

    if(!img)
    {
      printf("Failed to find glyph\n");
      return;
    }

    for(yi = 0; yi < g.height; yi++)
    {
      for(xi = 0; xi < g.width; xi++)
      {
        GnColor col = GnImagePixel(img, g.x + xi, g.y + yi);

        GnDrawPixel(ctx, x + xi, y + yi,
          col.r + colMod[0], col.g + colMod[1], col.b + colMod[2], col.a);
      }
    }

    x += g.width + 1;
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
        vector_at(img->rawData, i + 2),
        vector_at(img->rawData, i + 3));

      i+=4;
    }
  }
}
