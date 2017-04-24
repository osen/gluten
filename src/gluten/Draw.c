#ifndef AMALGAMATION
  #include "Draw.h"
  #include "Image.h"
  #include "Event.h"
  #include "config.h"
  #include "gluten.h"
#endif

void GnDrawPixel(GnEvent *ctx, int x, int y, int r, int g, int b, int a)
{
  GnDraw *draw = GnEventComponent(ctx, GnDraw);

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
    GnColor curr = {0};
    float opacity = (float)a / 255.0f;
    float tr = r;
    float tg = g;
    float tb = b;

    curr = GnImagePixel(GnInternal.buffer, x, y);
    tr = (float)curr.r + tr * opacity;
    tg = (float)curr.g + tg * opacity;
    tb = (float)curr.b + tb * opacity;

    tr /= (1.0f + opacity);
    tg /= (1.0f + opacity);
    tb /= (1.0f + opacity);

    if(tr > 255) tr = 255;
    if(tg > 255) tg = 255;
    if(tb > 255) tb = 255;

    r = tr;
    g = tg;
    b = tb;
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
