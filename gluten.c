#include "gluten.h"
#ifndef AMALGAMATION
  #include "Container.h"
  #include "Widget.h"
  #include "Position.h"
  #include "Draw.h"
  #include "Event.h"
#endif

void GnContainerRequestDraw(GnWidget *ctx, GnEvent *event)
{
  size_t i = 0;
  GnContainer *container = GnWidgetComponent(ctx, GnContainer);
  GnDraw *draw = GnEventComponent(event, GnDraw);
  GnBounds bounds = draw->bounds;

  for(i = 0; i < vector_size(container->children); i++)
  {
    GnWidget *child = vector_at(container->children, i);

    draw->bounds = bounds;
    draw->bounds.x += GnWidgetX(child);
    draw->bounds.y += GnWidgetY(child);
    draw->bounds.width = GnWidgetWidth(child);
    draw->bounds.height = GnWidgetHeight(child);
    GnWidgetEvent(vector_at(container->children, i), "request_draw", event);
  }

  draw->bounds = bounds;
}

void GnContainerSize(GnWidget *ctx, GnEvent *event)
{
  size_t i = 0;
  GnContainer *container = GnWidgetComponent(ctx, GnContainer);

  for(i = 0; i < vector_size(container->children); i++)
  {
    GnWidgetEvent(vector_at(container->children, i), "size", event);
  }
}

void GnContainerDestroy(GnWidget *ctx, GnEvent *event)
{
  size_t i = 0;
  GnContainer *container = GnWidgetComponent(ctx, GnContainer);

  for(i = 0; i < vector_size(container->children); i++)
  {
    GnWidgetDestroy(vector_at(container->children, i));
  }

  vector_delete(container->children);
}

void GnContainerInit(GnWidget *ctx, GnEvent *event)
{
  GnContainer *container = GnWidgetComponent(ctx, GnContainer);
  container->children = vector_new(GnWidget *);
  GnWidgetAddEvent(ctx, "request_draw", GnContainerRequestDraw);
  GnWidgetAddEvent(ctx, "size", GnContainerSize);
  GnWidgetAddEvent(ctx, "destroy", GnContainerDestroy);
}

void GnContainerAdd(GnWidget *ctx, GnWidget *child)
{
  GnContainer *container = GnWidgetComponent(ctx, GnContainer);
  vector_push_back(container->children, child);
}
#ifndef AMALGAMATION
  #include "Widget.h"
  #include "Position.h"
  #include "palloc.h"
#endif

#include <string.h>

GnWidget *_GnWidgetCreate(size_t size, char *type, void (*initFunc)(GnWidget *, GnEvent *))
{
  GnWidget *rtn = NULL;

  rtn = palloc(GnWidget);
  rtn->components = vector_new(struct GnComponentEntry);
  rtn->events = vector_new(struct GnEventEntry);

  GnWidgetAddComponent(rtn, GnPosition);
  _GnWidgetAddComponent(rtn, size, type, initFunc);

  return rtn;
}

void GnWidgetDestroy(GnWidget *ctx)
{
  size_t i = 0;

  GnWidgetEvent(ctx, "destroy", NULL);

  for(i = 0; i < vector_size(ctx->components); i++)
  {
    pfree(vector_at(ctx->components, i).data);
    free(vector_at(ctx->components, i).type);
  }

  vector_delete(ctx->components);

  for(i = 0; i < vector_size(ctx->events); i++)
  {
    free(vector_at(ctx->events, i).name);
  }

  vector_delete(ctx->events);
  pfree(ctx);
}

void GnWidgetEvent(GnWidget *ctx, char *name, GnEvent *event)
{
  size_t i = 0;

  for(i = 0; i < vector_size(ctx->events); i++)
  {
    if(strcmp(vector_at(ctx->events, i).name, name) != 0)
    {
      continue;
    }

    vector_at(ctx->events, i).func(ctx, event);
  }
}

void *_GnWidgetAddComponent(GnWidget *ctx, size_t size, char *type, void (*initFunc)(GnWidget *, GnEvent *))
{
  void *rtn = NULL;
  struct GnComponentEntry entry = {0};

  rtn = _palloc(size, type);
  entry.data = rtn;
  entry.type = strdup(type);

  vector_push_back(ctx->components, entry);

/*
  GnWidgetAddEvent(ctx, "init", initFunc);
  GnWidgetEvent(ctx, "init", NULL);
*/
  initFunc(ctx, NULL);

  return rtn;
}

void *_GnWidgetComponent(GnWidget *ctx, char *type)
{
  size_t i = 0;

  for(i = 0; i < vector_size(ctx->components); i++)
  {
    if(strcmp(type, vector_at(ctx->components, i).type) == 0)
    {
      return vector_at(ctx->components, i).data;
    }
  }

  return NULL;
}

void GnWidgetAddEvent(GnWidget *ctx, char *name,
  void (*func)(GnWidget *, GnEvent *))
{
  struct GnEventEntry entry = {0};

  entry.name = strdup(name);
  entry.func = func;

  vector_push_back(ctx->events, entry);
}

#ifndef AMALGAMATION
  #include "Event.h"
  #include <palloc.h>
#endif

#include <string.h>

GnEvent *GnEventCreate()
{
  GnEvent *rtn = NULL;

  rtn = palloc(GnEvent);
  rtn->components = vector_new(struct GnComponentEntry);

  return rtn;
}

void GnEventDestroy(GnEvent *ctx)
{
  size_t i = 0;

  for(i = 0; i < vector_size(ctx->components); i++)
  {
    pfree(vector_at(ctx->components, i).data);
    free(vector_at(ctx->components, i).type);
  }

  vector_delete(ctx->components);
  pfree(ctx);
}

void *_GnEventAddComponent(GnWidget *ctx, size_t size, char *type)
{ 
  void *rtn = NULL;
  struct GnComponentEntry entry = {0};

  rtn = _palloc(size, type);
  entry.data = rtn;
  entry.type = strdup(type);

  vector_push_back(ctx->components, entry);

  return rtn;
}
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
  draw->dirty = 1;
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
#ifndef AMALGAMATION
  #include "config.h"
  #include "Button.h"
  #include "Label.h"
  #include "Widget.h"
  #include "Position.h"
  #include "Draw.h"
#endif

void GnButtonDraw(GnWidget *ctx, GnEvent *event)
{
  GnDrawFillRect(event, 0, 0,
    GnWidgetWidth(ctx), GnWidgetHeight(ctx), GN_WIDGET_BORDER);

  GnDrawFillRect(event, 2, 2,
    GnWidgetWidth(ctx) - 4, GnWidgetHeight(ctx) - 4, GN_WIDGET_BACKGROUND);
}

void GnButtonInit(GnWidget *ctx, GnEvent *event)
{
  printf("GnButtonInit\n");

  GnWidgetAddEvent(ctx, "draw", GnButtonDraw);
  GnWidgetAddComponent(ctx, GnLabel);
}

#ifndef AMALGAMATION
  #include "config.h"
  #include "gluten.h"
  #include "Label.h"
  #include "Widget.h"
  #include "Position.h"
  #include "Anchor.h"
  #include "Draw.h"
  #include <palloc.h>
#endif

void GnLabelDraw(GnWidget *ctx, GnEvent *event)
{
/*
  GnLabel *label = GnWidgetComponent(ctx, GnLabel);
  GnDrawText(event, GnInternal.mediumMonoFont, 0, 0, label->text);
*/
  GnDrawText(event, GnInternal.mediumMonoFont, 0, 0, "Close");
}

void GnLabelSetText(GnWidget *ctx, char *text)
{
  GnLabel *label = GnWidgetComponent(ctx, GnLabel);
  printf("GnLabelSetText\n");
  pfree(label->text);
  label->text = palloc(char);
}

void GnLabelDestroy(GnWidget *ctx, GnEvent *event)
{
  GnLabel *label = GnWidgetComponent(ctx, GnLabel);
  pfree(label->text);
}

void GnLabelInit(GnWidget *ctx, GnEvent *event)
{
  GnLabel *label = GnWidgetComponent(ctx, GnLabel);
  label->text = palloc(char);
  printf("GnLabelInit\n");
  GnWidgetAddComponent(ctx, GnAnchor);
  GnWidgetAddEvent(ctx, "draw", GnLabelDraw);
  GnWidgetAddEvent(ctx, "destroy", GnLabelDestroy);
}

#ifndef AMALGAMATION
  #include "Anchor.h"
  #include "Widget.h"
#endif

void GnAnchorSize(GnWidget *ctx, GnEvent *event)
{

}

void GnAnchorInit(GnWidget *ctx, GnEvent *event)
{
  GnWidgetAddEvent(ctx, "size", GnAnchorSize);
}

#ifndef AMALGAMATION
  #include "Form.h"
  #include "gluten.h"
  #include "Widget.h"
  #include "Position.h"
  #include "Draw.h"
  #include <vector.h>
#endif

#include <string.h>

void GnFormShow(GnWidget *ctx)
{
  GnInternal.activeForm = ctx;
}

void GnFormSize(GnWidget *ctx, GnEvent *event)
{
  /* TODO: Set rectangle to window size */
}

void GnFormDraw(GnWidget *ctx, GnEvent *event)
{
  GnDraw *draw = GnEventComponent(event, GnDraw);

  GnDrawFillRect(event, draw->bounds.x, draw->bounds.y,
    draw->bounds.width, draw->bounds.height,
    GN_FORM_BACKGROUND);
}

void GnFormInit(GnWidget *ctx, GnEvent *event)
{
  GnWidgetAddEvent(ctx, "draw", GnFormDraw);
  GnWidgetAddEvent(ctx, "size", GnFormSize);
  GnWidgetAddComponent(ctx, GnContainer);
  vector_push_back(GnInternal.forms, ctx);

  /* TODO: Remove anchor component because form is special */
}

#ifndef AMALGAMATION
  #include "Image.h"
  #include "util.h"
  #include "gluten.h"
  #include <lodepng/lodepng.h>
  #include <palloc.h>
#endif

/*
void set_pixel(SDL_Surface *surface, int x, int y, Uint32 pixel)
{
  Uint8 *target_pixel = (Uint8 *)surface->pixels + y * surface->pitch + x * 4;
  *(Uint32 *)target_pixel = pixel;
}
*/

GnImage *GnImageCreate(int width, int height)
{
  GnImage *rtn = NULL;

  rtn = palloc(GnImage);
  rtn->width = width;
  rtn->height = height;
  rtn->rawData = vector_new(unsigned char);
  vector_resize(rtn->rawData, width * height * 4);

  return rtn;
}

GnImage *GnImageCreateFromString(char *str)
{
  GnImage *rtn = NULL;
  unsigned error = 0;
  unsigned width = 0;
  unsigned height = 0;
  size_t i = 0;

  rtn = palloc(GnImage);
  rtn->rawData = vector_new(unsigned char);

  GnHexArrayFromString(str, rtn->rawData);

  error = lodepng_decode32(&GnUnsafe.pngData, &width, &height, vector_raw(rtn->rawData), vector_size(rtn->rawData));

  if(error)
  {
    /* printf("error %u: %s\n", error, lodepng_error_text(error)); */
    vector_delete(rtn->rawData);
    pfree(rtn);
    return NULL;
  }

  /* printf("Image: %i %i\n", width, height); */
  rtn->width = width;
  rtn->height = height;

  vector_resize(rtn->rawData, width * height * 4);

  for(i = 0; i < width * height * 4; i++)
  {
    vector_set(rtn->rawData, i, GnUnsafe.pngData[i]);
  }

  free(GnUnsafe.pngData); GnUnsafe.pngData = NULL;

  return rtn;
}

void GnImageDestroy(GnImage *ctx)
{
  vector_delete(ctx->rawData);
  pfree(ctx);
}

int GnImageWidth(GnImage *ctx)
{
  return ctx->width;
}

int GnImageHeight(GnImage *ctx)
{
  return ctx->height;
}

GnColor GnImagePixel(GnImage *ctx, int x, int y)
{
  GnColor rtn = {0};
  size_t i = 0;

  if(x >= ctx->width || x < 0 || y >= ctx->height || y < 0)
  {
    printf("Error: Out of bounds of image\n");
    return rtn;
  }

  i = y * ctx->width * 4 + x * 4;

  rtn.r = vector_at(ctx->rawData, i);
  rtn.g = vector_at(ctx->rawData, i + 1);
  rtn.b = vector_at(ctx->rawData, i + 2);
  rtn.a = vector_at(ctx->rawData, i + 3);

  return rtn;
}

void GnImageSetPixel(GnImage *ctx, int x, int y, int r, int g, int b, int a)
{
  size_t i = 0;

  if(x >= ctx->width || x < 0 || y >= ctx->height || y < 0)
  {
    printf("Error: Out of bounds of image\n");
    return;
  }

  i = y * ctx->width * 4 + x * 4;

  vector_set(ctx->rawData, i, r);
  vector_set(ctx->rawData, i + 1, g);
  vector_set(ctx->rawData, i + 2, b);
  vector_set(ctx->rawData, i + 3, a);
}
#ifndef AMALGAMATION
  #include "Font.h"
  #include "Image.h"
  #include <palloc.h>
#endif

#include <string.h>

int GnFontDirtyColumn(GnFont *ctx, int x)
{
  int y = 0;

  for(y = 0; y < GnImageHeight(ctx->img); y++)
  {
    GnColor col = {0};

    col = GnImagePixel(ctx->img, x, y);

    if(col.a != 0) return 1;
  }

  return 0;
}

void GnFontGenerateGlyphs(GnFont *ctx)
{
  size_t x = 0;
  size_t i = 0;

  for(i = 0; i < strlen(characters); i++)
  {
    GnGlyph g = {0};
    g.c = characters[i];
    g.img = ctx->img;
    g.y = 0;
    g.height = GnImageHeight(ctx->img);

    while(1)
    {
      if(x >= GnImageWidth(ctx->img))
      {
        printf("Error: End of image before char found\n");
      }

      if(GnFontDirtyColumn(ctx, x) == 1)
      {
        g.x = x;
        break;
      }

      x++;
    }

    while(1)
    {
      if(x >= GnImageWidth(ctx->img))
      {
        printf("Error: End of image before end of char found\n");
      }

      if(GnFontDirtyColumn(ctx, x) == 0)
      {
        g.width = x - g.x;
        break;
      }

      x++;
    }

    vector_push_back(ctx->glyphs, g);

    if(g.c == '_')
    {
      g.c = ' ';
      g.height = 1;
      vector_push_back(ctx->glyphs, g);
    }
  }
}

GnFont *GnFontCreateFromString(char *data)
{
  GnFont *rtn = NULL;

  rtn = palloc(GnFont);
  rtn->img = GnImageCreateFromString(data);
  rtn->glyphs = vector_new(GnGlyph);

  GnFontGenerateGlyphs(rtn);

  return rtn;
}

GnGlyph GnFontGlyph(GnFont *ctx, char c)
{
  size_t i = 0;
  struct GnGlyph rtn = {0};

  for(i = 0; i < vector_size(ctx->glyphs); i++)
  {
    if(vector_at(ctx->glyphs, i).c == c)
    {
      return vector_at(ctx->glyphs, i);
    }
  }

  return rtn;
}

void GnFontDestroy(GnFont *ctx)
{
  GnImageDestroy(ctx->img);
  vector_delete(ctx->glyphs);
  pfree(ctx);
}
#ifndef AMALGAMATION
  #include "Position.h"
  #include "Widget.h"
#endif

void GnPositionRequestDraw(GnWidget *ctx, GnEvent *event)
{
  GnPosition *position = GnWidgetComponent(ctx, GnPosition);

  if(position->dirty)
  {
    GnWidgetEvent(ctx, "draw", event);
    position->dirty = 0;
  }
}

void GnPositionSize(GnWidget *ctx, GnEvent *event)
{
  GnPosition *position = GnWidgetComponent(ctx, GnPosition);

  position->dirty = 1;
}

void GnPositionInit(GnWidget *ctx, GnEvent *event)
{
  GnPosition *position = GnWidgetComponent(ctx, GnPosition);

  position->dirty = 1;
  GnWidgetSetPosition(ctx, 5, 5);
  GnWidgetSetSize(ctx, 80, 30);

  GnWidgetAddEvent(ctx, "request_draw", GnPositionRequestDraw);
  GnWidgetAddEvent(ctx, "size", GnPositionSize);
}

void GnWidgetSetPosition(GnWidget *ctx, int x, int y)
{
  GnPosition *position = GnWidgetComponent(ctx, GnPosition);

  position->x = x;
  position->y = y;
  position->dirty = 1;
}

void GnWidgetSetSize(GnWidget *ctx, int width, int height)
{
  GnPosition *position = GnWidgetComponent(ctx, GnPosition);

  position->width = width;
  position->height = height;
  position->dirty = 1;

  /* TODO: Send size event */
}

int GnWidgetX(GnWidget *ctx)
{
  GnPosition *position = GnWidgetComponent(ctx, GnPosition);

  return position->x;
}

int GnWidgetY(GnWidget *ctx)
{
  GnPosition *position = GnWidgetComponent(ctx, GnPosition);

  return position->y;
}

int GnWidgetWidth(GnWidget *ctx)
{
  GnPosition *position = GnWidgetComponent(ctx, GnPosition);

  return position->width;
}

int GnWidgetHeight(GnWidget *ctx)
{
  GnPosition *position = GnWidgetComponent(ctx, GnPosition);

  return position->height;
}

char mediumMono[9729] = {'8','9','5','0','4','e','4','7','0','d','0','a','1','a','0','a','0','0','0','0','0','0','0','d','4','9','4','8','4','4','5','2','0','0','0','0','0','5','3','2','0','0','0','0','0','0','1','3','0','8','0','6','0','0','0','0','0','0','a','3','9','8','f','2','6','0','0','0','0','0','0','0','0','6','6','2','4','b','4','7','4','4','0','0','f','f','0','0','f','f','0','0','f','f','a','0','b','d','a','7','9','3','0','0','0','0','0','0','0','9','7','0','4','8','5','9','7','3','0','0','0','0','0','b','1','3','0','0','0','0','0','b','1','3','0','1','0','0','9','a','9','c','1','8','0','0','0','0','0','0','0','7','7','4','4','9','4','d','4','5','0','7','e','1','0','4','1','8','1','0','1','7','2','7','c','d','4','e','2','9','b','a','0','0','0','0','0','0','1','d','6','9','5','4','5','8','7','4','4','3','6','f','6','d','6','d','6','5','6','e','7','4','0','0','0','0','0','0','0','0','0','0','4','3','7','2','6','5','6','1','7','4','6','5','6','4','2','0','7','7','6','9','7','4','6','8','2','0','4','7','4','9','4','d','5','0','6','4','2','e','6','5','0','7','0','0','0','0','1','2','6','4','4','9','4','4','4','1','5','4','7','8','d','a','e','d','9','d','7','b','b','4','5','4','e','5','7','5','c','0','7','f','5','c','a','e','1','7','8','a','c','0','b','d','b','c','4','1','1','e','0','5','9','6','4','d','1','a','1','b','9','3','2','6','4','d','4','4','6','3','a','c','f','8','8','a','5','4','4','5','d','4','8','4','4','4','2','a','c','6','4','7','e','3','2','3','3','5','2','6','6','a','3','4','0','f','6','3','d','4','c','4','e','8','0','a','f','5','4','1','b','4','b','6','6','a','8','d','5','e','d','4','2','6','1','a','6','9','d','5','8','8','1','a','4','0','e','5','1','1','1','f','4','5','5','0','b','b','9','2','4','a','d','a','8','8','1','1','8','1','8','b','8','a','2','8','2','f','9','1','4','7','2','2','4','c','f','f','d','8','f','b','a','c','3','9','7','7','b','8','3','8','f','b','f','b','e','6','9','e','7','3','6','7','8','6','f','d','5','b','6','b','d','6','d','c','b','9','e','b','7','c','b','3','c','f','7','c','c','f','b','d','f','7','b','7','b','f','7','d','c','0','7','1','1','c','c','7','7','1','1','c','c','7','7','1','1','c','c','7','7','1','1','c','c','7','7','1','1','c','c','7','7','1','1','c','c','7','7','1','1','c','c','7','7','1','1','c','c','7','7','1','1','c','c','7','7','1','1','c','c','7','7','1','1','c','c','7','7','1','1','c','c','7','7','1','1','c','c','7','7','1','1','c','c','7','7','1','1','c','c','7','7','1','3','a','a','4','5','0','f','2','e','e','7','4','1','e','5','7','0','2','0','f','0','4','5','c','f','f','2','0','7','0','8','5','5','7','9','b','e','3','3','8','8','e','e','3','3','8','8','e','5','3','e','7','f','6','4','7','a','1','0','e','6','5','5','5','c','2','e','5','c','0','f','d','4','0','b','7','1','c','e','d','b','f','a','e','b','6','3','f','b','3','e','6','3','4','6','0','7','d','0','e','f','2','0','a','5','d','5','8','b','e','9','0','8','3','9','c','4','2','0','d','8','d','2','3','c','7','a','9','7','6','c','e','0','3','d','e','0','4','7','6','6','6','3','c','4','f','e','c','5','0','3','9','e','7','d','5','b','2','a','2','6','0','b','d','a','6','5','0','6','5','f','7','9','5','7','0','1','c','d','0','0','6','a','c','0','0','2','6','4','4','c','8','9','a','0','0','b','c','a','c','9','d','a','5','9','0','7','1','8','7','4','9','1','8','0','0','b','c','0','3','0','c','0','f','2','8','3','3','4','2','c','b','f','4','a','f','4','1','8','5','a','4','d','a','1','9','0','f','c','c','3','7','5','c','d','7','0','0','d','c','a','0','4','a','c','f','7','a','e','0','3','a','a','3','4','2','1','9','b','b','c','0','1','f','0','f','3','c','0','3','6','c','0','3','5','6','0','3','f','f','6','6','6','c','f','f','a','3','8','0','c','7','8','1','a','d','5','a','e','e','b','6','8','8','7','e','f','3','a','b','8','0','f','b','2','d','7','4','f','0','b','2','7','2','3','8','f','0','3','4','b','0','4','5','e','f','f','5','c','b','9','1','f','2','3','6','1','9','c','a','8','d','0','5','6','6','a','5','d','a','e','f','6','1','6','0','b','4','a','1','d','c','6','0','e','0','3','f','b','4','c','c','3','b','c','0','8','d','4','0','7','7','1','f','3','6','1','5','3','1','0','5','f','8','9','d','f','6','e','d','f','f','0','6','0','e','c','9','4','1','e','6','b','3','3','a','9','e','6','2','1','4','f','3','b','7','9','1','c','d','a','4','d','1','c','6','b','2','8','7','6','a','b','f','4','e','c','6','e','0','d','9','0','1','f','2','3','6','0','0','4','f','e','a','d','8','8','8','1','d','7','f','8','5','0','c','c','b','0','1','1','c','0','1','2','c','d','2','d','f','b','7','1','2','f','8','4','a','e','4','3','a','5','4','c','8','7','8','7','e','8','9','9','9','7','f','4','b','a','f','3','b','0','7','6','8','3','5','c','c','f','5','9','5','1','8','8','0','6','9','2','5','f','2','6','b','0','1','f','5','3','2','3','3','e','2','1','e','b','a','0','3','f','f','a','c','f','d','6','c','0','3','7','0','9','3','7','1','3','e','4','b','d','7','e','3','5','b','3','a','2','7','8','e','c','a','4','1','d','e','4','e','e','0','0','d','e','0','e','7','4','6','9','d','a','9','1','9','b','8','1','9','5','8','a','5','f','3','c','b','3','3','c','0','c','4','c','8','b','1','5','0','c','8','b','0','5','f','2','f','0','7','f','e','3','2','f','5','7','9','3','f','e','0','d','5','3','2','6','5','3','6','0','1','7','d','4','a','f','e','d','7','1','7','d','8','6','c','9','0','3','7','0','7','3','8','2','b','f','5','f','9','a','b','c','0','a','3','6','5','c','a','c','c','0','2','b','e','5','3','f','2','b','f','4','b','7','5','e','d','c','c','7','2','7','e','8','9','1','9','4','f','0','b','7','4','d','e','c','d','c','b','5','1','d','4','0','c','2','c','d','5','5','7','7','3','8','e','6','b','e','7','f','7','5','4','0','f','a','d','2','6','3','b','a','c','9','6','e','5','a','f','0','0','f','e','7','a','0','f','a','8','c','f','b','c','e','e','d','3','e','d','c','d','d','a','c','6','d','b','a','f','3','a','5','8','0','3','7','c','4','6','f','d','0','e','5','9','b','6','5','f','8','3','a','e','9','b','6','b','e','a','b','9','c','3','c','6','3','a','3','2','f','f','4','a','8','d','3','1','8','0','f','f','0','5','f','a','1','9','c','a','4','c','5','0','e','5','a','e','a','0','c','a','c','1','1','7','0','c','6','5','5','6','6','b','6','3','1','f','a','c','0','6','5','5','2','8','a','b','8','0','6','3','2','3','3','a','4','b','2','5','0','3','f','e','4','2','e','0','c','e','8','8','7','2','7','7','0','2','1','7','0','4','5','c','d','f','8','a','3','8','4','0','0','b','f','a','d','e','e','a','7','3','4','3','4','5','0','a','f','0','f','9','c','0','1','2','7','5','6','6','8','c','0','1','5','e','5','0','6','3','2','0','a','b','f','e','f','2','3','8','7','0','0','c','d','0','1','b','7','1','4','4','5','e','a','f','8','6','4','5','3','9','7','e','8','5','3','8','3','3','7','b','2','1','4','e','f','3','6','b','8','0','4','7','0','2','e','4','4','e','d','6','f','1','5','7','c','8','7','8','1','c','7','c','0','6','d','9','0','9','f','a','a','2','c','e','0','f','c','3','8','1','b','b','2','3','b','e','6','7','3','a','7','0','a','d','e','1','b','a','6','7','8','1','1','f','a','8','d','1','d','1','a','c','7','f','5','b','1','c','5','b','b','f','0','0','7','e','a','a','7','5','3','9','0','4','b','8','1','7','b','8','d','8','8','7','4','d','4','5','c','c','0','4','f','6','5','7','8','7','c','6','6','1','6','a','4','c','5','8','f','b','c','c','6','2','e','0','3','d','3','5','c','c','a','d','0','e','f','a','3','e','c','0','b','b','f','a','e','a','1','3','d','1','a','f','9','b','b','5','c','d','9','f','3','2','9','4','f','b','3','4','b','2','b','9','7','6','b','c','c','a','f','a','0','b','7','6','7','5','3','8','7','d','9','0','b','c','1','6','2','d','b','b','c','2','3','8','2','f','5','5','e','2','4','0','8','b','e','1','a','3','b','a','6','e','2','6','7','3','d','3','b','e','c','0','8','c','4','8','7','9','7','9','4','4','8','a','8','4','9','6','4','f','5','f','f','7','6','5','e','4','e','4','4','4','f','7','8','c','d','b','6','1','5','1','e','a','f','3','a','2','0','0','d','d','6','c','3','9','f','0','a','1','c','0','7','b','f','8','a','6','a','e','7','9','2','3','f','5','b','5','1','0','f','8','4','6','e','0','e','f','1','b','8','2','6','c','e','4','2','d','c','c','4','1','5','e','3','7','6','0','9','8','a','e','7','f','f','3','0','c','e','5','1','e','4','0','3','6','9','b','8','6','0','1','3','d','8','0','8','3','9','0','4','d','8','8','a','c','f','a','5','9','6','c','b','f','f','e','2','9','b','b','3','a','b','0','6','f','2','f','5','3','6','6','2','9','b','2','2','1','d','7','a','2','7','3','6','0','8','b','7','e','5','e','6','e','9','0','b','7','3','f','b','2','6','9','d','f','1','d','6','8','0','2','5','e','2','9','e','9','3','b','1','d','3','1','0','6','d','9','c','4','4','9','1','c','d','6','a','3','f','4','f','3','9','8','8','8','7','e','7','e','4','b','a','0','d','e','1','b','d','a','0','e','e','3','b','1','6','d','4','8','7','7','8','6','b','c','4','6','5','d','7','f','c','e','5','5','1','d','7','0','0','e','f','5','b','7','b','9','b','9','a','7','3','8','3','2','7','7','b','8','8','3','a','8','4','b','d','6','5','9','c','7','c','7','b','d','5','3','1','d','7','a','7','d','b','7','5','a','6','e','f','7','2','c','1','d','9','9','d','f','4','2','1','c','7','6','1','f','3','1','2','a','b','b','0','0','a','f','0','1','9','f','5','3','5','9','1','f','5','2','8','7','4','1','3','9','5','6','0','3','0','7','a','a','8','1','b','b','2','a','f','5','f','f','6','1','4','6','9','9','e','f','2','0','4','e','b','e','3','c','3','b','d','a','1','3','8','8','f','3','3','4','9','4','e','3','1','0','e','7','9','6','9','5','1','1','5','a','8','7','0','5','e','0','3','e','c','2','2','2','4','0','f','7','9','4','0','9','2','d','4','4','f','9','5','d','a','4','f','d','3','3','6','1','0','2','e','2','1','c','c','b','a','b','3','e','7','b','2','3','1','1','5','0','a','1','f','4','c','1','1','6','b','1','9','1','5','c','b','b','0','a','8','9','9','0','c','b','d','a','9','1','3','9','0','b','3','8','b','d','c','2','3','a','e','9','1','7','3','0','8','6','d','f','d','3','3','a','0','c','a','d','9','7','7','7','1','0','2','7','6','6','c','2','6','0','c','4','a','1','5','d','c','d','f','4','4','4','8','e','4','c','2','d','c','e','5','0','c','6','6','2','9','5','d','1','b','3','c','2','e','f','f','9','1','2','e','2','f','8','2','b','c','7','4','8','c','4','f','1','3','d','5','1','d','b','a','e','1','5','f','8','d','7','8','0','b','9','f','3','0','9','7','5','8','4','1','c','1','b','d','9','a','f','7','b','2','3','9','1','c','3','e','5','7','8','1','0','3','8','b','3','1','3','d','6','9','3','1','3','8','1','c','7','a','a','4','c','e','1','0','1','7','1','e','c','9','f','d','6','4','9','f','2','a','a','d','9','9','1','3','9','1','9','9','8','8','b','3','8','c','3','b','2','5','6','3','c','e','7','a','b','a','e','b','4','1','f','3','0','3','b','4','0','3','7','b','b','0','0','7','1','8','8','8','5','d','c','c','3','1','2','6','0','5','c','e','a','f','3','8','1','c','0','7','3','1','1','b','f','a','f','1','9','d','9','1','8','c','8','4','b','9','e','3','5','f','2','b','0','a','3','a','8','c','5','2','c','c','7','4','5','6','c','b','f','f','e','7','b','d','a','a','7','1','7','7','a','1','0','3','8','b','9','4','c','9','9','d','9','5','a','9','7','5','3','5','4','c','6','1','4','e','0','0','0','6','3','7','d','a','2','7','d','e','5','5','4','c','4','f','9','7','6','b','d','b','1','c','c','e','5','c','0','7','f','e','a','d','f','0','f','0','0','d','f','8','f','a','8','d','3','2','3','9','0','8','d','9','9','e','6','8','c','c','7','f','b','0','2','e','2','a','3','3','2','4','3','e','4','d','d','0','c','5','c','9','2','f','a','7','c','8','9','f','e','a','f','1','a','d','9','5','7','d','7','9','1','2','d','b','a','f','e','8','d','0','e','a','8','8','f','c','b','7','4','c','c','c','d','5','3','b','d','c','7','4','2','2','3','7','0','9','5','d','a','7','3','e','b','d','5','0','e','b','4','3','0','1','8','0','9','7','e','d','9','8','c','6','c','b','e','5','b','d','b','6','3','6','f','e','0','d','f','5','5','d','6','a','f','0','3','6','c','b','f','d','8','6','8','e','1','8','1','a','a','4','b','6','c','0','6','7','e','1','8','b','8','a','e','9','c','8','d','6','c','0','4','b','4','0','1','4','7','4','7','d','c','6','7','4','c','9','f','a','e','2','4','1','2','f','a','b','b','c','0','f','3','8','1','f','3','6','9','c','c','f','c','b','9','0','2','4','9','b','7','3','6','5','b','e','b','3','4','c','b','f','a','1','c','a','2','e','b','f','a','2','6','7','d','1','f','1','2','2','0','e','f','1','a','6','d','f','7','2','7','8','1','4','1','0','1','f','d','7','a','9','e','9','6','f','b','5','1','4','4','5','b','5','c','8','c','0','4','6','9','b','d','0','7','5','c','6','4','2','8','7','3','1','8','1','2','6','4','b','3','4','5','c','7','e','0','1','9','1','9','8','f','8','7','4','2','6','4','f','b','f','f','4','1','f','5','1','d','9','0','c','d','f','1','5','7','0','c','6','5','7','e','8','f','0','4','c','9','a','5','6','9','4','2','4','e','8','8','9','4','6','b','c','7','5','9','1','4','4','f','4','6','9','d','8','b','e','d','5','4','0','1','b','4','3','f','3','9','3','1','5','4','f','b','d','b','5','e','1','9','d','6','2','7','c','8','a','6','d','8','4','e','e','2','8','8','d','5','4','3','7','7','5','2','a','3','9','b','7','1','5','9','3','8','a','a','4','e','2','6','f','e','4','8','c','9','2','6','5','d','9','4','4','2','8','f','9','6','a','f','d','6','8','9','6','c','a','2','2','a','8','7','7','3','0','2','0','c','a','d','3','b','9','1','2','3','b','b','7','9','6','2','1','9','7','4','b','b','9','b','8','0','d','f','0','a','b','8','7','e','3','8','e','2','c','0','2','c','2','0','0','e','4','d','7','7','6','4','5','6','a','6','f','8','6','e','5','4','2','5','2','6','6','1','1','0','7','2','5','4','2','e','a','f','f','a','f','c','3','b','2','4','2','2','2','3','8','4','6','6','5','5','0','c','9','e','3','4','5','e','7','f','6','3','c','a','3','0','0','e','5','9','c','8','d','6','e','8','c','2','f','9','9','2','2','a','a','e','9','6','0','9','f','1','6','d','3','5','0','0','5','e','5','7','2','5','f','4','f','e','0','0','e','5','2','0','e','1','5','2','6','c','d','1','6','0','a','8','3','1','f','6','7','d','3','5','8','8','f','b','e','a','d','f','f','7','1','b','c','a','9','5','a','6','7','4','1','8','8','c','e','d','2','8','7','b','5','7','3','8','9','a','c','6','0','2','f','f','8','4','3','8','f','c','1','e','2','2','6','e','c','3','c','4','c','a','b','1','2','a','e','3','4','d','9','5','1','9','1','a','a','5','d','3','4','f','f','b','d','b','1','6','e','0','1','4','c','3','f','5','3','7','a','8','b','2','9','b','f','e','a','d','d','6','2','8','f','c','e','9','d','a','5','7','4','2','8','e','e','5','1','5','4','a','e','e','f','5','0','a','6','c','1','b','1','7','6','b','8','9','d','b','1','c','2','b','6','d','e','f','0','1','3','a','4','6','a','a','a','9','7','f','2','5','8','a','d','d','f','0','4','e','9','2','5','7','a','d','8','e','c','c','e','3','8','0','6','5','1','9','1','b','7','0','a','5','0','6','d','9','8','f','8','1','6','9','2','9','1','d','c','a','5','2','6','e','6','f','e','4','d','8','f','5','3','e','0','1','f','7','b','0','8','9','5','d','3','7','7','4','3','6','0','6','f','e','b','e','2','1','e','a','0','8','5','b','9','8','9','3','b','c','8','1','0','1','e','b','d','8','4','2','e','0','5','6','c','a','4','7','1','b','7','6','a','6','2','1','1','e','f','3','1','d','4','3','8','1','7','5','c','8','6','9','a','0','0','6','f','d','b','b','9','c','5','e','7','8','b','b','c','e','b','b','f','7','a','a','8','3','e','1','3','e','2','4','3','2','7','a','9','6','f','1','5','e','8','7','2','1','8','e','e','5','2','5','d','8','9','d','8','a','3','d','7','4','2','c','d','c','a','0','e','f','3','d','0','2','e','b','a','7','8','f','c','e','1','9','4','7','e','5','3','0','3','f','8','d','2','7','3','e','2','a','b','3','5','e','f','5','d','d','f','f','5','3','2','7','4','f','2','f','d','5','c','d','6','6','0','7','d','4','c','7','7','4','2','4','e','a','7','7','3','a','e','d','3','7','4','c','3','e','8','8','e','f','e','9','3','8','1','d','a','3','f','d','7','9','9','a','b','1','d','c','1','d','2','9','7','9','3','7','0','5','b','4','c','7','7','5','3','a','2','e','f','a','2','2','8','e','d','7','b','b','3','3','6','e','f','7','d','b','9','0','5','4','1','9','2','d','3','a','0','7','8','6','a','c','2','b','d','7','e','b','7','d','7','e','0','5','f','8','4','d','8','e','f','d','3','3','6','6','9','d','3','d','0','3','3','9','4','d','3','9','2','8','e','3','f','b','2','c','2','0','e','9','5','b','9','2','f','7','5','7','3','3','a','e','c','f','5','6','e','0','2','7','a','9','7','e','7','6','6','7','8','0','b','c','9','f','a','4','c','6','c','3','6','d','1','9','f','7','e','b','4','4','e','6','1','c','e','4','5','4','c','f','2','0','c','4','f','1','5','a','8','e','3','6','d','5','5','f','9','a','5','4','4','7','9','e','9','1','4','3','f','b','c','5','9','4','b','f','8','5','e','2','2','6','c','7','4','5','d','8','8','2','1','2','a','e','5','3','3','d','3','e','c','d','0','9','d','8','4','e','6','a','8','c','d','5','f','e','d','c','4','b','e','7','c','4','b','1','0','1','3','e','a','2','e','4','e','4','c','4','7','9','c','4','9','d','6','c','3','d','2','7','a','0','9','f','3','5','0','2','9','f','2','7','e','e','8','4','7','1','2','5','f','3','c','4','4','a','9','5','d','b','5','8','6','b','0','b','d','c','8','6','8','c','b','e','b','7','4','1','0','7','5','3','2','3','7','0','4','f','8','0','0','2','7','3','2','d','f','0','4','b','6','d','8','8','8','f','0','5','c','8','e','b','8','9','e','c','3','0','3','f','8','e','7','8','d','a','a','d','8','a','d','6','8','1','6','a','8','4','2','7','0','3','3','f','2','f','f','e','8','4','c','d','a','3','5','f','4','a','1','3','f','0','c','7','8','0','e','b','6','7','d','0','f','e','6','8','f','9','8','c','9','c','0','6','4','4','a','d','1','0','a','a','f','4','e','e','4','8','b','5','d','b','2','3','f','a','f','7','f','b','3','9','d','5','e','7','4','7','b','4','a','f','4','e','0','8','9','4','5','7','5','0','4','3','7','7','b','4','e','1','f','a','4','f','e','8','b','5','f','d','2','3','e','f','b','7','0','7','f','0','4','9','3','5','2','4','a','f','3','1','d','6','6','7','a','b','2','a','b','b','f','d','5','5','3','1','b','8','3','7','5','0','d','e','e','b','e','c','b','a','4','3','f','7','4','1','8','6','e','3','b','2','5','4','b','d','2','c','d','3','f','f','9','5','e','3','3','e','5','d','6','4','f','b','2','3','4','e','c','c','b','b','7','2','6','a','f','7','d','0','7','2','8','f','e','a','6','f','b','a','1','4','7','b','5','4','4','2','b','a','9','f','c','4','e','e','1','e','0','e','5','3','9','9','c','b','0','8','4','b','6','1','f','0','6','b','5','5','0','c','8','7','1','a','a','f','7','f','0','9','d','9','c','1','3','d','1','5','7','1','2','e','d','f','1','b','d','0','0','e','b','f','4','5','2','2','9','9','0','e','0','c','5','0','7','8','4','b','e','b','e','3','6','d','e','4','7','8','6','6','3','9','d','e','8','f','3','0','f','6','3','b','6','a','e','f','e','e','b','a','5','6','6','4','d','5','7','e','b','1','e','5','b','6','e','9','5','a','5','4','f','a','3','d','5','9','f','7','b','3','3','c','1','d','9','9','8','b','9','1','0','d','d','2','c','f','e','6','3','4','c','e','f','b','a','b','b','3','6','7','a','9','e','a','3','3','2','1','a','7','6','5','7','e','a','4','0','a','b','e','f','5','1','e','7','6','d','0','3','e','d','f','6','7','0','3','b','6','e','3','9','5','e','9','3','6','5','b','8','b','3','d','6','7','6','c','4','7','f','2','d','e','3','7','c','a','d','b','a','1','6','3','a','0','8','0','6','c','3','a','b','6','1','8','c','a','0','d','4','2','a','2','e','3','d','a','7','4','c','c','d','e','4','a','f','b','8','8','f','a','a','c','0','d','b','9','9','0','e','f','7','8','0','9','f','8','1','4','9','2','8','e','e','2','7','f','0','c','d','7','5','f','a','e','4','6','f','b','7','a','2','4','c','a','7','1','a','3','7','e','b','e','2','3','e','0','7','e','5','f','c','0','1','6','e','9','9','d','6','6','8','2','f','e','a','6','9','8','3','c','f','4','b','7','6','b','1','b','e','4','6','5','9','0','2','d','2','0','2','e','2','a','7','3','4','f','d','8','b','8','e','f','8','9','4','4','7','6','5','9','e','b','2','3','d','9','9','0','1','b','8','1','e','d','d','4','0','4','a','a','0','7','e','c','1','7','7','1','6','f','6','d','2','9','7','9','2','3','0','3','d','a','6','3','3','9','c','5','8','d','9','c','6','6','d','5','0','9','b','3','6','c','f','7','5','5','7','a','7','f','a','4','e','c','2','b','a','b','9','c','4','1','a','9','7','6','d','8','9','a','6','3','f','f','0','c','5','d','6','7','2','7','6','a','b','d','8','e','c','a','6','1','1','c','1','5','9','0','8','d','a','e','e','4','f','d','8','f','1','9','d','7','e','7','e','a','9','2','7','e','d','6','1','6','3','9','1','e','5','6','6','5','d','c','a','f','1','3','9','9','a','1','0','1','0','2','2','b','9','1','b','c','c','6','f','b','9','7','e','8','6','9','7','9','a','d','7','f','d','6','f','2','2','7','5','2','d','c','5','4','9','9','8','3','3','8','2','4','c','b','7','1','2','8','7','2','f','a','6','0','8','0','e','a','1','0','2','d','4','8','8','0','c','a','b','9','4','6','9','9','3','f','4','0','9','c','9','e','5','7','0','6','f','c','b','e','a','9','1','4','4','f','4','e','2','c','4','0','0','2','8','a','4','2','3','8','5','8','6','d','e','2','9','0','9','3','3','d','0','5','e','0','1','f','7','2','b','2','1','b','1','3','c','e','2','4','e','c','5','9','3','1','5','7','a','9','1','d','f','6','0','2','9','2','0','a','a','b','3','7','7','0','7','5','5','7','2','c','6','a','4','b','3','3','f','a','d','e','4','f','5','0','d','c','3','9','0','8','8','d','6','a','3','8','5','8','2','b','7','2','2','e','b','6','2','3','2','c','c','7','e','b','b','5','5','3','8','0','4','9','3','a','4','9','a','d','a','0','f','d','1','1','e','0','d','d','f','1','a','2','4','e','d','8','8','7','e','8','e','2','3','7','1','0','5','b','a','4','4','0','a','5','c','4','4','6','6','4','0','e','2','5','2','c','2','2','3','3','7','6','1','1','a','b','d','4','e','0','8','c','1','d','7','c','7','9','3','f','f','1','2','e','5','4','e','1','e','d','a','a','8','8','c','c','0','3','1','0','0','7','6','3','c','c','7','1','d','5','1','6','9','d','5','c','2','c','8','6','4','b','7','2','5','4','b','8','d','2','f','b','1','d','6','9','5','4','2','6','3','7','d','1','7','e','b','7','d','7','1','a','8','1','9','6','7','0','1','6','6','1','8','e','b','3','c','7','2','8','e','6','c','8','6','c','d','1','8','9','d','8','1','2','9','d','3','0','1','8','c','9','7','1','b','8','5','e','7','f','d','7','b','7','d','5','9','0','a','f','3','6','1','e','4','5','7','2','a','0','7','d','1','7','b','b','7','3','b','0','3','3','1','8','a','6','3','2','9','7','0','5','1','a','c','8','3','b','0','9','7','b','f','a','e','a','4','6','5','d','2','c','d','7','e','8','9','8','1','8','6','7','e','c','a','3','8','3','7','5','9','c','7','6','d','7','d','7','0','6','6','c','5','1','0','6','1','d','4','5','6','4','3','e','6','1','2','8','d','7','5','9','1','1','9','9','f','d','a','9','c','e','8','8','c','c','1','5','2','9','c','3','3','1','e','4','b','b','6','a','2','9','2','2','7','3','9','8','2','a','d','6','2','f','0','3','7','f','9','6','f','1','f','d','2','5','e','5','1','e','4','9','3','9','7','c','4','2','e','a','7','3','b','0','e','a','0','5','f','d','8','c','e','5','d','e','a','5','f','2','0','8','c','9','1','0','3','a','8','a','c','8','d','c','1','0','2','8','e','f','c','3','d','a','1','6','5','3','0','2','6','5','e','f','8','3','4','4','c','3','c','c','c','d','b','0','9','f','e','d','3','4','f','e','a','f','2','3','6','e','4','2','d','2','3','0','7','d','1','b','d','9','d','4','2','9','c','7','1','9','4','8','0','e','e','c','a','5','2','9','a','3','7','5','3','6','1','2','c','d','6','b','e','1','2','8','2','d','d','b','0','6','f','c','4','d','0','e','f','3','c','5','d','1','4','8','8','4','5','5','e','f','1','c','e','7','a','7','f','1','c','4','4','5','6','5','d','6','a','b','2','3','f','3','0','8','6','4','f','3','6','e','3','b','e','1','4','7','4','6','9','3','8','d','f','b','2','6','6','4','0','3','c','b','c','2','5','6','e','2','d','2','c','d','6','c','2','f','9','1','6','7','b','d','c','f','6','d','e','a','d','4','d','8','a','1','e','3','6','e','4','7','c','6','e','d','1','e','7','b','9','f','8','5','0','a','e','5','e','6','e','9','c','8','5','c','8','7','9','c','b','2','c','9','4','3','a','f','2','8','e','c','e','6','3','d','a','b','f','a','2','c','6','d','b','f','e','d','1','9','8','f','8','7','d','8','f','e','9','2','c','8','e','c','1','6','5','8','9','f','9','f','5','4','7','b','6','5','1','5','7','2','9','c','f','d','9','c','9','c','d','a','2','f','b','4','7','c','5','f','d','5','0','d','f','a','e','9','7','b','5','f','4','3','9','9','4','6','d','5','8','9','b','f','8','a','6','c','7','0','9','e','8','a','6','c','c','c','5','8','a','3','8','6','3','f','4','c','3','1','7','5','6','1','8','8','0','e','b','1','4','e','d','f','d','7','1','2','1','6','8','4','3','6','4','2','f','d','4','c','2','1','8','1','2','5','4','d','4','8','b','e','e','c','d','7','7','2','d','6','e','b','5','f','5','3','b','9','5','6','e','7','f','7','d','5','a','a','c','b','1','d','a','a','e','b','e','d','3','a','b','5','a','9','e','b','8','2','d','d','3','9','c','1','4','e','0','e','f','c','0','e','a','b','b','2','9','b','e','e','f','c','c','9','8','3','7','f','3','e','8','b','e','d','e','1','1','1','9','b','2','9','3','e','8','4','6','8','1','a','e','2','7','5','7','f','3','4','8','7','3','a','8','a','c','d','9','1','3','9','9','1','b','0','1','c','9','9','9','d','d','1','9','6','f','5','4','a','8','c','b','2','b','b','8','8','f','6','d','1','c','5','7','9','e','4','c','8','9','c','a','4','4','6','6','a','2','5','4','7','8','4','f','b','6','2','d','b','8','5','e','f','2','c','0','7','f','6','3','0','c','4','f','1','5','a','8','e','e','7','8','9','7','7','6','4','7','6','4','3','8','e','0','7','f','c','6','d','c','0','7','d','6','d','a','1','f','d','a','6','4','a','1','f','e','c','d','1','0','9','a','5','c','6','e','4','c','3','5','5','d','a','a','f','c','7','2','2','9','1','e','d','6','f','2','2','1','1','5','3','c','7','6','4','d','8','e','e','c','7','a','8','8','c','3','7','5','5','e','6','d','8','8','c','7','f','d','b','3','6','c','4','0','9','b','d','0','6','8','9','6','2','3','a','c','0','7','8','9','f','1','d','a','5','3','b','f','9','5','2','c','4','7','8','b','5','e','6','c','8','f','c','2','5','9','d','9','3','2','3','f','3','0','4','a','a','3','3','4','7','e','6','4','3','e','c','9','a','0','a','a','0','d','a','1','d','9','9','7','d','5','5','d','1','0','a','9','5','f','3','0','b','6','c','0','f','1','2','e','b','e','c','7','6','0','8','a','d','c','f','5','b','9','1','6','3','9','5','9','6','7','2','c','f','d','3','3','e','6','7','e','5','3','8','e','2','7','2','5','6','5','a','5','9','d','c','8','1','b','c','1','7','2','3','e','4','1','d','8','6','6','c','9','6','8','4','3','e','9','8','3','1','2','4','4','7','7','4','c','c','e','f','d','c','5','4','6','2','d','c','e','c','8','5','7','d','4','3','7','c','1','2','1','2','8','d','3','2','1','7','d','9','9','4','2','f','c','7','9','1','e','a','a','8','4','9','2','2','7','0','5','b','f','5','f','3','4','5','4','6','7','9','4','f','e','9','d','c','f','7','8','f','8','4','4','7','6','5','c','6','3','c','d','5','7','b','8','5','e','a','5','a','7','9','e','b','9','f','0','b','0','8','8','f','c','a','a','c','5','7','7','d','7','7','2','5','1','2','f','d','d','4','a','4','6','3','2','1','c','4','5','1','3','4','3','c','6','5','9','0','5','b','2','3','d','0','9','6','2','1','2','7','8','8','4','2','6','9','4','b','c','9','0','b','8','9','5','c','f','b','0','3','7','1','8','e','f','2','d','8','7','6','4','f','4','7','6','4','8','e','a','4','3','e','1','d','9','9','a','3','7','5','4','d','3','8','2','8','4','2','d','6','9','f','0','8','c','b','b','5','9','7','b','7','2','3','7','3','7','5','4','9','3','f','6','b','8','b','1','c','0','f','6','d','9','1','f','2','f','2','d','c','b','c','f','d','5','c','8','0','d','e','9','3','1','0','1','a','6','0','b','0','8','d','6','2','b','4','e','1','a','0','c','0','7','b','7','e','1','a','4','9','0','f','f','0','5','4','4','0','9','9','9','f','a','9','7','d','f','9','9','0','b','6','4','1','8','8','4','f','e','4','6','e','e','4','9','4','f','0','5','d','8','1','7','5','f','2','7','b','5','d','6','b','4','3','1','e','5','6','d','7','5','3','7','f','d','7','b','8','8','8','7','6','d','b','9','b','a','e','c','9','9','1','b','9','7','7','2','d','2','e','7','0','5','9','2','d','d','c','0','f','2','2','c','e','c','4','d','3','0','8','d','b','d','5','b','9','4','b','8','d','e','1','0','2','7','0','3','8','b','6','e','4','a','f','6','9','5','e','a','7','e','3','0','7','f','f','e','c','8','e','4','7','9','0','6','3','a','3','a','3','7','4','2','0','b','e','8','c','4','4','8','4','6','6','c','d','0','5','c','4','3','d','b','5','7','c','0','6','f','6','c','4','d','9','d','5','d','c','3','f','a','a','e','1','3','e','6','3','1','4','d','d','f','3','7','5','7','1','1','f','a','d','a','f','2','5','6','4','f','b','d','4','f','2','a','f','6','b','9','f','f','e','5','4','c','4','f','8','1','b','a','7','8','a','e','b','5','0','2','4','2','2','2','5','c','b','0','8','8','5','9','9','4','8','1','4','7','6','1','3','9','2','5','8','f','e','6','1','2','4','f','7','6','1','3','9','b','e','8','5','2','4','6','9','1','f','4','c','f','1','6','8','f','9','c','f','8','d','3','2','4','f','a','0','f','d','9','3','7','e','2','d','3','c','8','7','e','c','1','8','b','5','5','0','c','c','1','d','6','a','f','9','8','e','5','6','c','4','3','9','d','b','0','f','7','9','c','2','f','a','1','b','3','a','c','7','6','4','3','d','8','e','2','a','1','9','7','f','3','d','9','1','0','7','2','9','6','5','f','9','5','0','a','2','2','5','2','a','a','3','6','7','4','e','b','f','e','f','6','5','2','4','8','a','2','8','3','9','5','a','f','e','3','3','e','3','7','7','d','c','a','e','6','3','3','7','e','1','1','b','d','8','f','2','2','2','1','7','4','a','9','c','2','1','1','7','1','a','1','d','3','0','e','3','5','4','7','9','9','d','a','8','0','6','d','6','1','8','6','c','2','9','1','3','0','a','2','9','0','7','c','0','7','1','c','8','d','1','a','e','6','a','7','c','6','a','f','9','4','1','b','a','b','e','1','e','8','9','e','4','1','b','f','a','1','8','d','5','e','9','c','8','b','c','0','7','d','9','8','1','e','f','a','9','6','b','e','e','b','c','0','8','3','9','8','3','1','0','4','7','f','d','0','1','3','9','b','7','4','3','6','8','7','d','e','e','a','b','f','3','9','2','a','5','d','c','2','5','5','a','1','7','2','3','f','4','3','5','d','f','a','8','5','7','c','4','f','e','b','e','0','b','3','b','9','0','7','7','6','5','a','4','b','c','6','7','8','1','9','3','0','c','7','a','d','d','7','8','1','d','4','7','0','3','5','5','0','7','5','d','9','8','6','1','3','f','9','b','8','3','4','4','f','a','0','f','a','0','9','8','c','b','7','3','8','e','b','1','6','c','3','3','e','2','0','8','d','d','8','4','2','d','6','7','6','5','1','2','5','1','7','2','a','4','7','e','9','e','a','a','9','f','a','7','1','a','c','a','1','e','8','e','e','4','0','8','6','b','d','0','b','5','7','a','7','9','c','6','e','b','d','8','0','c','2','4','e','2','b','4','2','b','f','4','c','f','f','1','1','1','3','a','4','f','b','d','3','a','3','2','d','7','a','a','3','d','d','6','4','7','c','7','4','2','8','8','a','3','e','8','0','6','e','d','9','7','d','7','6','3','c','f','0','9','7','8','1','9','e','2','9','8','f','f','7','3','9','d','3','f','a','d','c','7','0','d','e','f','5','0','3','9','c','d','4','8','7','4','b','2','f','5','3','e','a','7','0','1','f','f','a','2','f','a','d','9','2','8','b','2','4','f','a','1','7','5','1','b','9','2','b','a','a','2','1','9','d','9','5','0','a','8','4','7','4','7','2','6','6','a','8','b','b','f','8','8','2','d','5','2','a','e','d','4','e','9','7','3','6','8','8','4','b','c','b','c','1','c','9','9','3','3','5','2','f','d','f','a','4','6','e','c','3','9','0','f','4','b','c','7','8','3','3','5','6','7','6','c','6','b','6','4','b','f','8','e','6','d','f','3','9','9','1','4','8','f','9','5','7','f','8','1','f','0','c','8','b','e','3','5','4','8','a','a','1','3','2','b','b','f','4','1','3','6','e','e','f','b','2','0','9','b','b','e','2','1','f','7','7','c','3','9','e','2','0','8','0','d','7','9','8','0','5','c','1','2','9','8','7','0','8','c','b','e','9','f','6','5','2','c','f','7','7','1','e','4','3','4','4','2','a','3','b','e','7','f','3','c','4','0','e','6','2','d','8','8','a','3','6','f','6','2','a','0','3','f','e','b','f','4','2','e','5','8','f','f','b','a','c','2','d','e','a','c','3','9','0','7','5','0','3','9','1','2','0','5','a','9','0','1','7','9','9','0','c','6','f','8','8','0','b','2','a','7','5','0','c','c','6','1','f','7','0','a','b','2','0','3','1','f','c','2','8','9','a','a','c','c','b','7','6','1','c','b','6','3','3','0','4','4','0','7','f','d','0','6','2','4','5','f','d','b','7','4','7','5','0','2','6','5','4','d','7','f','c','2','9','f','9','6','3','e','1','2','d','9','5','9','1','9','5','0','2','3','f','d','a','3','5','0','c','5','f','7','1','9','a','3','e','4','a','2','7','d','7','a','b','a','f','6','9','7','0','d','b','a','3','8','3','5','1','8','e','f','2','f','f','6','6','9','7','0','1','d','b','d','c','a','e','5','0','9','9','b','8','0','3','8','e','8','b','6','a','b','1','1','7','e','0','f','6','1','a','1','e','d','a','1','f','5','7','a','3','a','1','2','1','d','b','9','1','d','3','9','4','6','7','6','b','5','d','1','b','9','d','5','8','8','2','4','4','d','5','f','8','7','1','c','d','b','9','e','8','9','f','d','8','8','c','0','3','3','0','6','8','3','b','6','9','4','e','4','e','9','9','d','4','9','f','b','3','d','a','6','f','f','b','3','f','c','b','e','3','7','9','0','6','8','b','c','a','7','9','1','a','3','5','a','5','9','8','f','8','5','8','3','5','5','5','6','3','d','1','3','5','a','2','f','3','7','a','9','c','1','1','2','c','a','a','b','b','4','8','f','1','0','d','9','0','f','5','b','4','2','f','9','f','4','9','8','5','b','a','f','c','6','9','9','3','5','f','7','d','7','e','1','e','a','7','4','d','9','8','a','3','8','3','5','c','f','0','e','9','0','b','7','1','1','7','9','5','8','c','2','1','1','1','5','c','e','1','3','5','9','c','e','d','1','9','3','5','5','1','9','d','c','0','6','f','c','c','e','a','8','4','c','5','6','e','2','b','0','8','b','9','9','3','f','a','7','2','2','3','9','0','8','3','7','0','3','f','f','8','5','2','d','b','f','6','9','4','7','f','7','7','4','b','2','7','e','4','f','5','3','8','e','e','d','1','0','5','3','6','e','a','6','b','1','5','c','a','3','8','e','a','3','8','d','f','a','b','a','1','9','5','b','0','4','4','d','6','c','7','f','e','9','a','e','f','3','f','c','0','6','f','d','8','e','1','7','b','1','e','d','f','c','7','7','2','4','e','f','2','4','c','a','9','f','8','2','9','8','a','4','e','d','b','d','0','d','3','9','c','d','3','0','9','3','b','0','5','c','6','f','a','1','b','f','7','3','b','8','e','a','b','a','e','b','f','4','7','5','3','f','6','1','0','f','c','3','7','a','0','e','f','b','5','3','c','7','7','b','6','9','7','f','4','e','2','2','6','0','f','6','d','1','f','b','b','5','a','4','5','2','5','a','4','8','f','b','4','8','e','a','3','3','b','1','3','b','f','7','6','3','1','d','2','f','5','d','9','9','1','3','7','7','4','1','a','4','4','3','a','5','d','e','9','8','a','a','c','e','8','9','7','5','c','8','8','6','5','e','8','8','f','e','f','9','6','3','c','2','9','f','d','2','b','c','1','7','9','2','2','3','7','d','a','5','8','e','f','9','8','b','8','d','e','5','8','6','5','0','7','c','b','a','7','3','c','8','5','3','c','b','7','b','2','1','c','e','c','5','b','5','2','a','f','3','f','4','8','8','7','e','1','a','c','2','4','0','8','a','4','f','5','7','b','f','1','2','7','b','9','e','e','e','5','8','c','7','5','b','6','7','9','c','9','2','8','a','1','d','7','7','9','3','0','8','7','f','7','8','d','2','4','9','6','a','d','f','8','6','3','a','2','4','f','3','7','2','6','4','0','e','4','5','4','e','4','5','b','e','a','b','e','f','4','3','0','3','e','4','4','d','d','3','7','6','9','f','8','7','0','4','6','0','5','8','e','5','c','d','d','7','7','2','5','7','e','5','3','0','0','7','7','e','1','1','d','9','b','0','d','a','8','a','9','c','b','e','3','8','2','4','b','0','f','c','8','5','a','9','b','5','d','a','c','2','d','1','1','4','f','3','5','1','9','f','1','d','7','8','c','f','4','9','7','a','a','8','9','0','a','8','c','5','1','6','1','d','e','b','0','d','f','a','6','e','f','5','8','9','c','c','4','d','c','d','0','d','6','7','6','0','d','f','6','c','4','c','f','4','d','d','2','d','d','8','d','3','0','c','7','5','c','6','b','8','7','d','0','b','d','9','6','8','6','c','c','8','7','8','3','e','6','b','d','0','3','e','b','2','0','6','a','7','4','b','0','d','d','c','5','a','e','4','0','a','c','2','9','c','a','6','0','f','6','9','1','9','6','f','b','f','c','a','0','9','5','5','7','0','1','d','2','7','6','f','e','6','1','1','b','e','9','1','5','3','e','f','0','c','d','7','c','5','f','6','3','c','6','a','f','4','0','8','8','4','e','3','e','c','e','1','4','c','4','6','d','2','7','9','e','c','e','f','5','5','e','1','1','8','0','d','c','1','5','8','c','6','6','3','d','f','b','0','c','e','3','b','3','f','b','b','5','3','1','f','0','c','4','4','3','6','2','a','a','b','9','9','b','d','0','8','7','b','4','8','a','c','9','3','a','d','5','d','e','b','f','3','4','c','e','d','7','1','0','a','9','2','3','2','2','a','4','f','b','e','a','e','f','6','c','e','c','e','8','c','f','b','d','7','4','e','9','5','7','3','b','e','3','7','7','3','7','5','4','e','1','8','8','e','b','7','9','f','e','3','3','8','f','5','c','3','4','7','9','1','5','d','f','4','2','d','4','8','9','4','5','c','3','7','a','f','1','2','c','7','a','9','2','9','7','e','0','8','7','c','d','e','a','b','c','1','7','1','9','c','1','a','e','5','5','a','2','4','1','2','e','c','3','2','2','4','3','d','5','2','3','5','3','3','1','9','f','8','a','d','3','7','5','9','d','5','d','8','b','5','6','e','1','f','d','7','1','6','3','d','9','0','e','0','a','7','d','3','f','6','b','0','7','e','e','a','3','8','8','e','e','3','3','8','8','e','e','3','3','8','8','e','e','3','3','8','b','b','3','d','6','e','e','f','c','6','6','7','6','d','7','1','1','1','9','2','9','6','e','3','1','9','8','a','0','f','f','e','a','9','d','6','f','e','d','6','4','6','3','1','a','7','a','e','5','3','5','9','5','d','5','6','d','3','f','7','3','8','f','9','b','5','f','b','7','c','e','c','4','f','f','3','7','6','3','2','a','2','9','b','6','d','1','c','7','9','7','0','4','b','f','c','3','c','e','4','7','5','e','7','e','d','e','0','3','8','b','b','e','b','9','b','d','e','2','f','1','d','c','7','7','1','1','c','c','7','7','1','1','c','c','7','7','1','1','c','c','7','8','a','5','b','d','1','8','e','e','3','3','8','5','d','4','2','c','1','a','7','6','2','c','7','7','1','1','c','c','7','7','1','1','c','c','7','7','1','1','c','c','7','7','1','1','c','c','7','7','1','1','c','c','7','7','1','1','c','c','7','7','1','1','c','c','7','7','1','1','c','c','7','a','9','2','7','f','e','1','f','1','8','d','4','1','3','3','c','0','6','b','b','9','7','3','2','0','0','0','0','0','0','0','0','4','9','4','5','4','e','4','4','a','e','4','2','6','0','8','2',0};
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

#ifndef AMALGAMATION
  #include "util.h"
#endif

#include <stdlib.h>
#include <string.h>

void GnHexArrayFromString(char *input, vector(unsigned char) *output)
{
  size_t len = strlen(input);
  size_t i = 0;

  for(i = 0; i < len - 1; i+=2)
  {
    char num[3] = {0};

    num[0] = input[i];
    num[1] = input[i + 1];

    vector_push_back(output, strtoul(num, NULL, 16));
  }
}
/*
LodePNG version 20161127

Copyright (c) 2005-2016 Lode Vandevenne

This software is provided 'as-is', without any express or implied
warranty. In no event will the authors be held liable for any damages
arising from the use of this software.

Permission is granted to anyone to use this software for any purpose,
including commercial applications, and to alter it and redistribute it
freely, subject to the following restrictions:

    1. The origin of this software must not be misrepresented; you must not
    claim that you wrote the original software. If you use this software
    in a product, an acknowledgment in the product documentation would be
    appreciated but is not required.

    2. Altered source versions must be plainly marked as such, and must not be
    misrepresented as being the original software.

    3. This notice may not be removed or altered from any source
    distribution.
*/

/*
The manual and changelog are in the header file "lodepng.h"
Rename this file to lodepng.cpp to use it for C++, or to lodepng.c to use it for C.
*/

#ifndef AMALGAMATION
  #include "lodepng.h"
#endif

#include <limits.h>
#include <stdio.h>
#include <stdlib.h>

#if defined(_MSC_VER) && (_MSC_VER >= 1310) /*Visual Studio: A few warning types are not desired here.*/
#pragma warning( disable : 4244 ) /*implicit conversions: not warned by gcc -Wall -Wextra and requires too much casts*/
#pragma warning( disable : 4996 ) /*VS does not like fopen, but fopen_s is not standard C so unusable here*/
#endif /*_MSC_VER */

const char* LODEPNG_VERSION_STRING = "20161127";

/*
This source file is built up in the following large parts. The code sections
with the "LODEPNG_COMPILE_" #defines divide this up further in an intermixed way.
-Tools for C and common code for PNG and Zlib
-C Code for Zlib (huffman, deflate, ...)
-C Code for PNG (file format chunks, adam7, PNG filters, color conversions, ...)
-The C++ wrapper around all of the above
*/

/*The malloc, realloc and free functions defined here with "lodepng_" in front
of the name, so that you can easily change them to others related to your
platform if needed. Everything else in the code calls these. Pass
-DLODEPNG_NO_COMPILE_ALLOCATORS to the compiler, or comment out
#define LODEPNG_COMPILE_ALLOCATORS in the header, to disable the ones here and
define them in your own project's source files without needing to change
lodepng source code. Don't forget to remove "static" if you copypaste them
from here.*/

#ifdef LODEPNG_COMPILE_ALLOCATORS
static void* lodepng_malloc(size_t size)
{
  return malloc(size);
}

static void* lodepng_realloc(void* ptr, size_t new_size)
{
  return realloc(ptr, new_size);
}

static void lodepng_free(void* ptr)
{
  free(ptr);
}
#else /*LODEPNG_COMPILE_ALLOCATORS*/
void* lodepng_malloc(size_t size);
void* lodepng_realloc(void* ptr, size_t new_size);
void lodepng_free(void* ptr);
#endif /*LODEPNG_COMPILE_ALLOCATORS*/

/* ////////////////////////////////////////////////////////////////////////// */
/* ////////////////////////////////////////////////////////////////////////// */
/* // Tools for C, and common code for PNG and Zlib.                       // */
/* ////////////////////////////////////////////////////////////////////////// */
/* ////////////////////////////////////////////////////////////////////////// */

/*
Often in case of an error a value is assigned to a variable and then it breaks
out of a loop (to go to the cleanup phase of a function). This macro does that.
It makes the error handling code shorter and more readable.

Example: if(!uivector_resizev(&frequencies_ll, 286, 0)) ERROR_BREAK(83);
*/
#define CERROR_BREAK(errorvar, code)\
{\
  errorvar = code;\
  break;\
}

/*version of CERROR_BREAK that assumes the common case where the error variable is named "error"*/
#define ERROR_BREAK(code) CERROR_BREAK(error, code)

/*Set error var to the error code, and return it.*/
#define CERROR_RETURN_ERROR(errorvar, code)\
{\
  errorvar = code;\
  return code;\
}

/*Try the code, if it returns error, also return the error.*/
#define CERROR_TRY_RETURN(call)\
{\
  unsigned error = call;\
  if(error) return error;\
}

/*Set error var to the error code, and return from the void function.*/
#define CERROR_RETURN(errorvar, code)\
{\
  errorvar = code;\
  return;\
}

/*
About uivector, ucvector and string:
-All of them wrap dynamic arrays or text strings in a similar way.
-LodePNG was originally written in C++. The vectors replace the std::vectors that were used in the C++ version.
-The string tools are made to avoid problems with compilers that declare things like strncat as deprecated.
-They're not used in the interface, only internally in this file as static functions.
-As with many other structs in this file, the init and cleanup functions serve as ctor and dtor.
*/

#ifdef LODEPNG_COMPILE_ZLIB
/*dynamic vector of unsigned ints*/
typedef struct uivector
{
  unsigned* data;
  size_t size; /*size in number of unsigned longs*/
  size_t allocsize; /*allocated size in bytes*/
} uivector;

static void uivector_cleanup(void* p)
{
  ((uivector*)p)->size = ((uivector*)p)->allocsize = 0;
  lodepng_free(((uivector*)p)->data);
  ((uivector*)p)->data = NULL;
}

/*returns 1 if success, 0 if failure ==> nothing done*/
static unsigned uivector_reserve(uivector* p, size_t allocsize)
{
  if(allocsize > p->allocsize)
  {
    size_t newsize = (allocsize > p->allocsize * 2) ? allocsize : (allocsize * 3 / 2);
    void* data = lodepng_realloc(p->data, newsize);
    if(data)
    {
      p->allocsize = newsize;
      p->data = (unsigned*)data;
    }
    else return 0; /*error: not enough memory*/
  }
  return 1;
}

/*returns 1 if success, 0 if failure ==> nothing done*/
static unsigned uivector_resize(uivector* p, size_t size)
{
  if(!uivector_reserve(p, size * sizeof(unsigned))) return 0;
  p->size = size;
  return 1; /*success*/
}

/*resize and give all new elements the value*/
static unsigned uivector_resizev(uivector* p, size_t size, unsigned value)
{
  size_t oldsize = p->size, i;
  if(!uivector_resize(p, size)) return 0;
  for(i = oldsize; i < size; ++i) p->data[i] = value;
  return 1;
}

static void uivector_init(uivector* p)
{
  p->data = NULL;
  p->size = p->allocsize = 0;
}

#ifdef LODEPNG_COMPILE_ENCODER
/*returns 1 if success, 0 if failure ==> nothing done*/
static unsigned uivector_push_back(uivector* p, unsigned c)
{
  if(!uivector_resize(p, p->size + 1)) return 0;
  p->data[p->size - 1] = c;
  return 1;
}
#endif /*LODEPNG_COMPILE_ENCODER*/
#endif /*LODEPNG_COMPILE_ZLIB*/

/* /////////////////////////////////////////////////////////////////////////// */

/*dynamic vector of unsigned chars*/
typedef struct ucvector
{
  unsigned char* data;
  size_t size; /*used size*/
  size_t allocsize; /*allocated size*/
} ucvector;

/*returns 1 if success, 0 if failure ==> nothing done*/
static unsigned ucvector_reserve(ucvector* p, size_t allocsize)
{
  if(allocsize > p->allocsize)
  {
    size_t newsize = (allocsize > p->allocsize * 2) ? allocsize : (allocsize * 3 / 2);
    void* data = lodepng_realloc(p->data, newsize);
    if(data)
    {
      p->allocsize = newsize;
      p->data = (unsigned char*)data;
    }
    else return 0; /*error: not enough memory*/
  }
  return 1;
}

/*returns 1 if success, 0 if failure ==> nothing done*/
static unsigned ucvector_resize(ucvector* p, size_t size)
{
  if(!ucvector_reserve(p, size * sizeof(unsigned char))) return 0;
  p->size = size;
  return 1; /*success*/
}

#ifdef LODEPNG_COMPILE_PNG

static void ucvector_cleanup(void* p)
{
  ((ucvector*)p)->size = ((ucvector*)p)->allocsize = 0;
  lodepng_free(((ucvector*)p)->data);
  ((ucvector*)p)->data = NULL;
}

static void ucvector_init(ucvector* p)
{
  p->data = NULL;
  p->size = p->allocsize = 0;
}
#endif /*LODEPNG_COMPILE_PNG*/

#ifdef LODEPNG_COMPILE_ZLIB
/*you can both convert from vector to buffer&size and vica versa. If you use
init_buffer to take over a buffer and size, it is not needed to use cleanup*/
static void ucvector_init_buffer(ucvector* p, unsigned char* buffer, size_t size)
{
  p->data = buffer;
  p->allocsize = p->size = size;
}
#endif /*LODEPNG_COMPILE_ZLIB*/

#if (defined(LODEPNG_COMPILE_PNG) && defined(LODEPNG_COMPILE_ANCILLARY_CHUNKS)) || defined(LODEPNG_COMPILE_ENCODER)
/*returns 1 if success, 0 if failure ==> nothing done*/
static unsigned ucvector_push_back(ucvector* p, unsigned char c)
{
  if(!ucvector_resize(p, p->size + 1)) return 0;
  p->data[p->size - 1] = c;
  return 1;
}
#endif /*defined(LODEPNG_COMPILE_PNG) || defined(LODEPNG_COMPILE_ENCODER)*/


/* ////////////////////////////////////////////////////////////////////////// */

#ifdef LODEPNG_COMPILE_PNG
#ifdef LODEPNG_COMPILE_ANCILLARY_CHUNKS
/*returns 1 if success, 0 if failure ==> nothing done*/
static unsigned string_resize(char** out, size_t size)
{
  char* data = (char*)lodepng_realloc(*out, size + 1);
  if(data)
  {
    data[size] = 0; /*null termination char*/
    *out = data;
  }
  return data != 0;
}

/*init a {char*, size_t} pair for use as string*/
static void string_init(char** out)
{
  *out = NULL;
  string_resize(out, 0);
}

/*free the above pair again*/
static void string_cleanup(char** out)
{
  lodepng_free(*out);
  *out = NULL;
}

static void string_set(char** out, const char* in)
{
  size_t insize = strlen(in), i;
  if(string_resize(out, insize))
  {
    for(i = 0; i != insize; ++i)
    {
      (*out)[i] = in[i];
    }
  }
}
#endif /*LODEPNG_COMPILE_ANCILLARY_CHUNKS*/
#endif /*LODEPNG_COMPILE_PNG*/

/* ////////////////////////////////////////////////////////////////////////// */

unsigned lodepng_read32bitInt(const unsigned char* buffer)
{
  return (unsigned)((buffer[0] << 24) | (buffer[1] << 16) | (buffer[2] << 8) | buffer[3]);
}

#if defined(LODEPNG_COMPILE_PNG) || defined(LODEPNG_COMPILE_ENCODER)
/*buffer must have at least 4 allocated bytes available*/
static void lodepng_set32bitInt(unsigned char* buffer, unsigned value)
{
  buffer[0] = (unsigned char)((value >> 24) & 0xff);
  buffer[1] = (unsigned char)((value >> 16) & 0xff);
  buffer[2] = (unsigned char)((value >>  8) & 0xff);
  buffer[3] = (unsigned char)((value      ) & 0xff);
}
#endif /*defined(LODEPNG_COMPILE_PNG) || defined(LODEPNG_COMPILE_ENCODER)*/

#ifdef LODEPNG_COMPILE_ENCODER
static void lodepng_add32bitInt(ucvector* buffer, unsigned value)
{
  ucvector_resize(buffer, buffer->size + 4); /*todo: give error if resize failed*/
  lodepng_set32bitInt(&buffer->data[buffer->size - 4], value);
}
#endif /*LODEPNG_COMPILE_ENCODER*/

/* ////////////////////////////////////////////////////////////////////////// */
/* / File IO                                                                / */
/* ////////////////////////////////////////////////////////////////////////// */

#ifdef LODEPNG_COMPILE_DISK

/* returns negative value on error. This should be pure C compatible, so no fstat. */
static long lodepng_filesize(const char* filename)
{
  FILE* file;
  long size;
  file = fopen(filename, "rb");
  if(!file) return -1;

  if(fseek(file, 0, SEEK_END) != 0)
  {
    fclose(file);
    return -1;
  }

  size = ftell(file);
  /* It may give LONG_MAX as directory size, this is invalid for us. */
  if(size == LONG_MAX) size = -1;

  fclose(file);
  return size;
}

/* load file into buffer that already has the correct allocated size. Returns error code.*/
static unsigned lodepng_buffer_file(unsigned char* out, size_t size, const char* filename)
{
  FILE* file;
  size_t readsize;
  file = fopen(filename, "rb");
  if(!file) return 78;

  readsize = fread(out, 1, size, file);
  fclose(file);

  if (readsize != size) return 78;
  return 0;
}

unsigned lodepng_load_file(unsigned char** out, size_t* outsize, const char* filename)
{
  long size = lodepng_filesize(filename);
  if (size < 0) return 78;
  *outsize = (size_t)size;

  *out = (unsigned char*)lodepng_malloc((size_t)size);
  if(!(*out) && size > 0) return 83; /*the above malloc failed*/

  return lodepng_buffer_file(*out, (size_t)size, filename);
}

/*write given buffer to the file, overwriting the file, it doesn't append to it.*/
unsigned lodepng_save_file(const unsigned char* buffer, size_t buffersize, const char* filename)
{
  FILE* file;
  file = fopen(filename, "wb" );
  if(!file) return 79;
  fwrite((char*)buffer , 1 , buffersize, file);
  fclose(file);
  return 0;
}

#endif /*LODEPNG_COMPILE_DISK*/

/* ////////////////////////////////////////////////////////////////////////// */
/* ////////////////////////////////////////////////////////////////////////// */
/* // End of common code and tools. Begin of Zlib related code.            // */
/* ////////////////////////////////////////////////////////////////////////// */
/* ////////////////////////////////////////////////////////////////////////// */

#ifdef LODEPNG_COMPILE_ZLIB
#ifdef LODEPNG_COMPILE_ENCODER
/*TODO: this ignores potential out of memory errors*/
#define addBitToStream(/*size_t**/ bitpointer, /*ucvector**/ bitstream, /*unsigned char*/ bit)\
{\
  /*add a new byte at the end*/\
  if(((*bitpointer) & 7) == 0) ucvector_push_back(bitstream, (unsigned char)0);\
  /*earlier bit of huffman code is in a lesser significant bit of an earlier byte*/\
  (bitstream->data[bitstream->size - 1]) |= (bit << ((*bitpointer) & 0x7));\
  ++(*bitpointer);\
}

static void addBitsToStream(size_t* bitpointer, ucvector* bitstream, unsigned value, size_t nbits)
{
  size_t i;
  for(i = 0; i != nbits; ++i) addBitToStream(bitpointer, bitstream, (unsigned char)((value >> i) & 1));
}

static void addBitsToStreamReversed(size_t* bitpointer, ucvector* bitstream, unsigned value, size_t nbits)
{
  size_t i;
  for(i = 0; i != nbits; ++i) addBitToStream(bitpointer, bitstream, (unsigned char)((value >> (nbits - 1 - i)) & 1));
}
#endif /*LODEPNG_COMPILE_ENCODER*/

#ifdef LODEPNG_COMPILE_DECODER

#define READBIT(bitpointer, bitstream) ((bitstream[bitpointer >> 3] >> (bitpointer & 0x7)) & (unsigned char)1)

static unsigned char readBitFromStream(size_t* bitpointer, const unsigned char* bitstream)
{
  unsigned char result = (unsigned char)(READBIT(*bitpointer, bitstream));
  ++(*bitpointer);
  return result;
}

static unsigned readBitsFromStream(size_t* bitpointer, const unsigned char* bitstream, size_t nbits)
{
  unsigned result = 0, i;
  for(i = 0; i != nbits; ++i)
  {
    result += ((unsigned)READBIT(*bitpointer, bitstream)) << i;
    ++(*bitpointer);
  }
  return result;
}
#endif /*LODEPNG_COMPILE_DECODER*/

/* ////////////////////////////////////////////////////////////////////////// */
/* / Deflate - Huffman                                                      / */
/* ////////////////////////////////////////////////////////////////////////// */

#define FIRST_LENGTH_CODE_INDEX 257
#define LAST_LENGTH_CODE_INDEX 285
/*256 literals, the end code, some length codes, and 2 unused codes*/
#define NUM_DEFLATE_CODE_SYMBOLS 288
/*the distance codes have their own symbols, 30 used, 2 unused*/
#define NUM_DISTANCE_SYMBOLS 32
/*the code length codes. 0-15: code lengths, 16: copy previous 3-6 times, 17: 3-10 zeros, 18: 11-138 zeros*/
#define NUM_CODE_LENGTH_CODES 19

/*the base lengths represented by codes 257-285*/
static const unsigned LENGTHBASE[29]
  = {3, 4, 5, 6, 7, 8, 9, 10, 11, 13, 15, 17, 19, 23, 27, 31, 35, 43, 51, 59,
     67, 83, 99, 115, 131, 163, 195, 227, 258};

/*the extra bits used by codes 257-285 (added to base length)*/
static const unsigned LENGTHEXTRA[29]
  = {0, 0, 0, 0, 0, 0, 0,  0,  1,  1,  1,  1,  2,  2,  2,  2,  3,  3,  3,  3,
      4,  4,  4,   4,   5,   5,   5,   5,   0};

/*the base backwards distances (the bits of distance codes appear after length codes and use their own huffman tree)*/
static const unsigned DISTANCEBASE[30]
  = {1, 2, 3, 4, 5, 7, 9, 13, 17, 25, 33, 49, 65, 97, 129, 193, 257, 385, 513,
     769, 1025, 1537, 2049, 3073, 4097, 6145, 8193, 12289, 16385, 24577};

/*the extra bits of backwards distances (added to base)*/
static const unsigned DISTANCEEXTRA[30]
  = {0, 0, 0, 0, 1, 1, 2,  2,  3,  3,  4,  4,  5,  5,   6,   6,   7,   7,   8,
       8,    9,    9,   10,   10,   11,   11,   12,    12,    13,    13};

/*the order in which "code length alphabet code lengths" are stored, out of this
the huffman tree of the dynamic huffman tree lengths is generated*/
static const unsigned CLCL_ORDER[NUM_CODE_LENGTH_CODES]
  = {16, 17, 18, 0, 8, 7, 9, 6, 10, 5, 11, 4, 12, 3, 13, 2, 14, 1, 15};

/* ////////////////////////////////////////////////////////////////////////// */

/*
Huffman tree struct, containing multiple representations of the tree
*/
typedef struct HuffmanTree
{
  unsigned* tree2d;
  unsigned* tree1d;
  unsigned* lengths; /*the lengths of the codes of the 1d-tree*/
  unsigned maxbitlen; /*maximum number of bits a single code can get*/
  unsigned numcodes; /*number of symbols in the alphabet = number of codes*/
} HuffmanTree;

/*function used for debug purposes to draw the tree in ascii art with C++*/
/*
static void HuffmanTree_draw(HuffmanTree* tree)
{
  std::cout << "tree. length: " << tree->numcodes << " maxbitlen: " << tree->maxbitlen << std::endl;
  for(size_t i = 0; i != tree->tree1d.size; ++i)
  {
    if(tree->lengths.data[i])
      std::cout << i << " " << tree->tree1d.data[i] << " " << tree->lengths.data[i] << std::endl;
  }
  std::cout << std::endl;
}*/

static void HuffmanTree_init(HuffmanTree* tree)
{
  tree->tree2d = 0;
  tree->tree1d = 0;
  tree->lengths = 0;
}

static void HuffmanTree_cleanup(HuffmanTree* tree)
{
  lodepng_free(tree->tree2d);
  lodepng_free(tree->tree1d);
  lodepng_free(tree->lengths);
}

/*the tree representation used by the decoder. return value is error*/
static unsigned HuffmanTree_make2DTree(HuffmanTree* tree)
{
  unsigned nodefilled = 0; /*up to which node it is filled*/
  unsigned treepos = 0; /*position in the tree (1 of the numcodes columns)*/
  unsigned n, i;

  tree->tree2d = (unsigned*)lodepng_malloc(tree->numcodes * 2 * sizeof(unsigned));
  if(!tree->tree2d) return 83; /*alloc fail*/

  /*
  convert tree1d[] to tree2d[][]. In the 2D array, a value of 32767 means
  uninited, a value >= numcodes is an address to another bit, a value < numcodes
  is a code. The 2 rows are the 2 possible bit values (0 or 1), there are as
  many columns as codes - 1.
  A good huffman tree has N * 2 - 1 nodes, of which N - 1 are internal nodes.
  Here, the internal nodes are stored (what their 0 and 1 option point to).
  There is only memory for such good tree currently, if there are more nodes
  (due to too long length codes), error 55 will happen
  */
  for(n = 0; n < tree->numcodes * 2; ++n)
  {
    tree->tree2d[n] = 32767; /*32767 here means the tree2d isn't filled there yet*/
  }

  for(n = 0; n < tree->numcodes; ++n) /*the codes*/
  {
    for(i = 0; i != tree->lengths[n]; ++i) /*the bits for this code*/
    {
      unsigned char bit = (unsigned char)((tree->tree1d[n] >> (tree->lengths[n] - i - 1)) & 1);
      /*oversubscribed, see comment in lodepng_error_text*/
      if(treepos > 2147483647 || treepos + 2 > tree->numcodes) return 55;
      if(tree->tree2d[2 * treepos + bit] == 32767) /*not yet filled in*/
      {
        if(i + 1 == tree->lengths[n]) /*last bit*/
        {
          tree->tree2d[2 * treepos + bit] = n; /*put the current code in it*/
          treepos = 0;
        }
        else
        {
          /*put address of the next step in here, first that address has to be found of course
          (it's just nodefilled + 1)...*/
          ++nodefilled;
          /*addresses encoded with numcodes added to it*/
          tree->tree2d[2 * treepos + bit] = nodefilled + tree->numcodes;
          treepos = nodefilled;
        }
      }
      else treepos = tree->tree2d[2 * treepos + bit] - tree->numcodes;
    }
  }

  for(n = 0; n < tree->numcodes * 2; ++n)
  {
    if(tree->tree2d[n] == 32767) tree->tree2d[n] = 0; /*remove possible remaining 32767's*/
  }

  return 0;
}

/*
Second step for the ...makeFromLengths and ...makeFromFrequencies functions.
numcodes, lengths and maxbitlen must already be filled in correctly. return
value is error.
*/
static unsigned HuffmanTree_makeFromLengths2(HuffmanTree* tree)
{
  uivector blcount;
  uivector nextcode;
  unsigned error = 0;
  unsigned bits, n;

  uivector_init(&blcount);
  uivector_init(&nextcode);

  tree->tree1d = (unsigned*)lodepng_malloc(tree->numcodes * sizeof(unsigned));
  if(!tree->tree1d) error = 83; /*alloc fail*/

  if(!uivector_resizev(&blcount, tree->maxbitlen + 1, 0)
  || !uivector_resizev(&nextcode, tree->maxbitlen + 1, 0))
    error = 83; /*alloc fail*/

  if(!error)
  {
    /*step 1: count number of instances of each code length*/
    for(bits = 0; bits != tree->numcodes; ++bits) ++blcount.data[tree->lengths[bits]];
    /*step 2: generate the nextcode values*/
    for(bits = 1; bits <= tree->maxbitlen; ++bits)
    {
      nextcode.data[bits] = (nextcode.data[bits - 1] + blcount.data[bits - 1]) << 1;
    }
    /*step 3: generate all the codes*/
    for(n = 0; n != tree->numcodes; ++n)
    {
      if(tree->lengths[n] != 0) tree->tree1d[n] = nextcode.data[tree->lengths[n]]++;
    }
  }

  uivector_cleanup(&blcount);
  uivector_cleanup(&nextcode);

  if(!error) return HuffmanTree_make2DTree(tree);
  else return error;
}

/*
given the code lengths (as stored in the PNG file), generate the tree as defined
by Deflate. maxbitlen is the maximum bits that a code in the tree can have.
return value is error.
*/
static unsigned HuffmanTree_makeFromLengths(HuffmanTree* tree, const unsigned* bitlen,
                                            size_t numcodes, unsigned maxbitlen)
{
  unsigned i;
  tree->lengths = (unsigned*)lodepng_malloc(numcodes * sizeof(unsigned));
  if(!tree->lengths) return 83; /*alloc fail*/
  for(i = 0; i != numcodes; ++i) tree->lengths[i] = bitlen[i];
  tree->numcodes = (unsigned)numcodes; /*number of symbols*/
  tree->maxbitlen = maxbitlen;
  return HuffmanTree_makeFromLengths2(tree);
}

#ifdef LODEPNG_COMPILE_ENCODER

/*BPM: Boundary Package Merge, see "A Fast and Space-Economical Algorithm for Length-Limited Coding",
Jyrki Katajainen, Alistair Moffat, Andrew Turpin, 1995.*/

/*chain node for boundary package merge*/
typedef struct BPMNode
{
  int weight; /*the sum of all weights in this chain*/
  unsigned index; /*index of this leaf node (called "count" in the paper)*/
  struct BPMNode* tail; /*the next nodes in this chain (null if last)*/
  int in_use;
} BPMNode;

/*lists of chains*/
typedef struct BPMLists
{
  /*memory pool*/
  unsigned memsize;
  BPMNode* memory;
  unsigned numfree;
  unsigned nextfree;
  BPMNode** freelist;
  /*two heads of lookahead chains per list*/
  unsigned listsize;
  BPMNode** chains0;
  BPMNode** chains1;
} BPMLists;

/*creates a new chain node with the given parameters, from the memory in the lists */
static BPMNode* bpmnode_create(BPMLists* lists, int weight, unsigned index, BPMNode* tail)
{
  unsigned i;
  BPMNode* result;

  /*memory full, so garbage collect*/
  if(lists->nextfree >= lists->numfree)
  {
    /*mark only those that are in use*/
    for(i = 0; i != lists->memsize; ++i) lists->memory[i].in_use = 0;
    for(i = 0; i != lists->listsize; ++i)
    {
      BPMNode* node;
      for(node = lists->chains0[i]; node != 0; node = node->tail) node->in_use = 1;
      for(node = lists->chains1[i]; node != 0; node = node->tail) node->in_use = 1;
    }
    /*collect those that are free*/
    lists->numfree = 0;
    for(i = 0; i != lists->memsize; ++i)
    {
      if(!lists->memory[i].in_use) lists->freelist[lists->numfree++] = &lists->memory[i];
    }
    lists->nextfree = 0;
  }

  result = lists->freelist[lists->nextfree++];
  result->weight = weight;
  result->index = index;
  result->tail = tail;
  return result;
}

/*sort the leaves with stable mergesort*/
static void bpmnode_sort(BPMNode* leaves, size_t num)
{
  BPMNode* mem = (BPMNode*)lodepng_malloc(sizeof(*leaves) * num);
  size_t width, counter = 0;
  for(width = 1; width < num; width *= 2)
  {
    BPMNode* a = (counter & 1) ? mem : leaves;
    BPMNode* b = (counter & 1) ? leaves : mem;
    size_t p;
    for(p = 0; p < num; p += 2 * width)
    {
      size_t q = (p + width > num) ? num : (p + width);
      size_t r = (p + 2 * width > num) ? num : (p + 2 * width);
      size_t i = p, j = q, k;
      for(k = p; k < r; k++)
      {
        if(i < q && (j >= r || a[i].weight <= a[j].weight)) b[k] = a[i++];
        else b[k] = a[j++];
      }
    }
    counter++;
  }
  if(counter & 1) memcpy(leaves, mem, sizeof(*leaves) * num);
  lodepng_free(mem);
}

/*Boundary Package Merge step, numpresent is the amount of leaves, and c is the current chain.*/
static void boundaryPM(BPMLists* lists, BPMNode* leaves, size_t numpresent, int c, int num)
{
  unsigned lastindex = lists->chains1[c]->index;

  if(c == 0)
  {
    if(lastindex >= numpresent) return;
    lists->chains0[c] = lists->chains1[c];
    lists->chains1[c] = bpmnode_create(lists, leaves[lastindex].weight, lastindex + 1, 0);
  }
  else
  {
    /*sum of the weights of the head nodes of the previous lookahead chains.*/
    int sum = lists->chains0[c - 1]->weight + lists->chains1[c - 1]->weight;
    lists->chains0[c] = lists->chains1[c];
    if(lastindex < numpresent && sum > leaves[lastindex].weight)
    {
      lists->chains1[c] = bpmnode_create(lists, leaves[lastindex].weight, lastindex + 1, lists->chains1[c]->tail);
      return;
    }
    lists->chains1[c] = bpmnode_create(lists, sum, lastindex, lists->chains1[c - 1]);
    /*in the end we are only interested in the chain of the last list, so no
    need to recurse if we're at the last one (this gives measurable speedup)*/
    if(num + 1 < (int)(2 * numpresent - 2))
    {
      boundaryPM(lists, leaves, numpresent, c - 1, num);
      boundaryPM(lists, leaves, numpresent, c - 1, num);
    }
  }
}

unsigned lodepng_huffman_code_lengths(unsigned* lengths, const unsigned* frequencies,
                                      size_t numcodes, unsigned maxbitlen)
{
  unsigned error = 0;
  unsigned i;
  size_t numpresent = 0; /*number of symbols with non-zero frequency*/
  BPMNode* leaves; /*the symbols, only those with > 0 frequency*/

  if(numcodes == 0) return 80; /*error: a tree of 0 symbols is not supposed to be made*/
  if((1u << maxbitlen) < numcodes) return 80; /*error: represent all symbols*/

  leaves = (BPMNode*)lodepng_malloc(numcodes * sizeof(*leaves));
  if(!leaves) return 83; /*alloc fail*/

  for(i = 0; i != numcodes; ++i)
  {
    if(frequencies[i] > 0)
    {
      leaves[numpresent].weight = (int)frequencies[i];
      leaves[numpresent].index = i;
      ++numpresent;
    }
  }

  for(i = 0; i != numcodes; ++i) lengths[i] = 0;

  /*ensure at least two present symbols. There should be at least one symbol
  according to RFC 1951 section 3.2.7. Some decoders incorrectly require two. To
  make these work as well ensure there are at least two symbols. The
  Package-Merge code below also doesn't work correctly if there's only one
  symbol, it'd give it the theoritical 0 bits but in practice zlib wants 1 bit*/
  if(numpresent == 0)
  {
    lengths[0] = lengths[1] = 1; /*note that for RFC 1951 section 3.2.7, only lengths[0] = 1 is needed*/
  }
  else if(numpresent == 1)
  {
    lengths[leaves[0].index] = 1;
    lengths[leaves[0].index == 0 ? 1 : 0] = 1;
  }
  else
  {
    BPMLists lists;
    BPMNode* node;

    bpmnode_sort(leaves, numpresent);

    lists.listsize = maxbitlen;
    lists.memsize = 2 * maxbitlen * (maxbitlen + 1);
    lists.nextfree = 0;
    lists.numfree = lists.memsize;
    lists.memory = (BPMNode*)lodepng_malloc(lists.memsize * sizeof(*lists.memory));
    lists.freelist = (BPMNode**)lodepng_malloc(lists.memsize * sizeof(BPMNode*));
    lists.chains0 = (BPMNode**)lodepng_malloc(lists.listsize * sizeof(BPMNode*));
    lists.chains1 = (BPMNode**)lodepng_malloc(lists.listsize * sizeof(BPMNode*));
    if(!lists.memory || !lists.freelist || !lists.chains0 || !lists.chains1) error = 83; /*alloc fail*/

    if(!error)
    {
      for(i = 0; i != lists.memsize; ++i) lists.freelist[i] = &lists.memory[i];

      bpmnode_create(&lists, leaves[0].weight, 1, 0);
      bpmnode_create(&lists, leaves[1].weight, 2, 0);

      for(i = 0; i != lists.listsize; ++i)
      {
        lists.chains0[i] = &lists.memory[0];
        lists.chains1[i] = &lists.memory[1];
      }

      /*each boundaryPM call adds one chain to the last list, and we need 2 * numpresent - 2 chains.*/
      for(i = 2; i != 2 * numpresent - 2; ++i) boundaryPM(&lists, leaves, numpresent, (int)maxbitlen - 1, (int)i);

      for(node = lists.chains1[maxbitlen - 1]; node; node = node->tail)
      {
        for(i = 0; i != node->index; ++i) ++lengths[leaves[i].index];
      }
    }

    lodepng_free(lists.memory);
    lodepng_free(lists.freelist);
    lodepng_free(lists.chains0);
    lodepng_free(lists.chains1);
  }

  lodepng_free(leaves);
  return error;
}

/*Create the Huffman tree given the symbol frequencies*/
static unsigned HuffmanTree_makeFromFrequencies(HuffmanTree* tree, const unsigned* frequencies,
                                                size_t mincodes, size_t numcodes, unsigned maxbitlen)
{
  unsigned error = 0;
  while(!frequencies[numcodes - 1] && numcodes > mincodes) --numcodes; /*trim zeroes*/
  tree->maxbitlen = maxbitlen;
  tree->numcodes = (unsigned)numcodes; /*number of symbols*/
  tree->lengths = (unsigned*)lodepng_realloc(tree->lengths, numcodes * sizeof(unsigned));
  if(!tree->lengths) return 83; /*alloc fail*/
  /*initialize all lengths to 0*/
  memset(tree->lengths, 0, numcodes * sizeof(unsigned));

  error = lodepng_huffman_code_lengths(tree->lengths, frequencies, numcodes, maxbitlen);
  if(!error) error = HuffmanTree_makeFromLengths2(tree);
  return error;
}

static unsigned HuffmanTree_getCode(const HuffmanTree* tree, unsigned index)
{
  return tree->tree1d[index];
}

static unsigned HuffmanTree_getLength(const HuffmanTree* tree, unsigned index)
{
  return tree->lengths[index];
}
#endif /*LODEPNG_COMPILE_ENCODER*/

/*get the literal and length code tree of a deflated block with fixed tree, as per the deflate specification*/
static unsigned generateFixedLitLenTree(HuffmanTree* tree)
{
  unsigned i, error = 0;
  unsigned* bitlen = (unsigned*)lodepng_malloc(NUM_DEFLATE_CODE_SYMBOLS * sizeof(unsigned));
  if(!bitlen) return 83; /*alloc fail*/

  /*288 possible codes: 0-255=literals, 256=endcode, 257-285=lengthcodes, 286-287=unused*/
  for(i =   0; i <= 143; ++i) bitlen[i] = 8;
  for(i = 144; i <= 255; ++i) bitlen[i] = 9;
  for(i = 256; i <= 279; ++i) bitlen[i] = 7;
  for(i = 280; i <= 287; ++i) bitlen[i] = 8;

  error = HuffmanTree_makeFromLengths(tree, bitlen, NUM_DEFLATE_CODE_SYMBOLS, 15);

  lodepng_free(bitlen);
  return error;
}

/*get the distance code tree of a deflated block with fixed tree, as specified in the deflate specification*/
static unsigned generateFixedDistanceTree(HuffmanTree* tree)
{
  unsigned i, error = 0;
  unsigned* bitlen = (unsigned*)lodepng_malloc(NUM_DISTANCE_SYMBOLS * sizeof(unsigned));
  if(!bitlen) return 83; /*alloc fail*/

  /*there are 32 distance codes, but 30-31 are unused*/
  for(i = 0; i != NUM_DISTANCE_SYMBOLS; ++i) bitlen[i] = 5;
  error = HuffmanTree_makeFromLengths(tree, bitlen, NUM_DISTANCE_SYMBOLS, 15);

  lodepng_free(bitlen);
  return error;
}

#ifdef LODEPNG_COMPILE_DECODER

/*
returns the code, or (unsigned)(-1) if error happened
inbitlength is the length of the complete buffer, in bits (so its byte length times 8)
*/
static unsigned huffmanDecodeSymbol(const unsigned char* in, size_t* bp,
                                    const HuffmanTree* codetree, size_t inbitlength)
{
  unsigned treepos = 0, ct;
  for(;;)
  {
    if(*bp >= inbitlength) return (unsigned)(-1); /*error: end of input memory reached without endcode*/
    /*
    decode the symbol from the tree. The "readBitFromStream" code is inlined in
    the expression below because this is the biggest bottleneck while decoding
    */
    ct = codetree->tree2d[(treepos << 1) + READBIT(*bp, in)];
    ++(*bp);
    if(ct < codetree->numcodes) return ct; /*the symbol is decoded, return it*/
    else treepos = ct - codetree->numcodes; /*symbol not yet decoded, instead move tree position*/

    if(treepos >= codetree->numcodes) return (unsigned)(-1); /*error: it appeared outside the codetree*/
  }
}
#endif /*LODEPNG_COMPILE_DECODER*/

#ifdef LODEPNG_COMPILE_DECODER

/* ////////////////////////////////////////////////////////////////////////// */
/* / Inflator (Decompressor)                                                / */
/* ////////////////////////////////////////////////////////////////////////// */

/*get the tree of a deflated block with fixed tree, as specified in the deflate specification*/
static void getTreeInflateFixed(HuffmanTree* tree_ll, HuffmanTree* tree_d)
{
  /*TODO: check for out of memory errors*/
  generateFixedLitLenTree(tree_ll);
  generateFixedDistanceTree(tree_d);
}

/*get the tree of a deflated block with dynamic tree, the tree itself is also Huffman compressed with a known tree*/
static unsigned getTreeInflateDynamic(HuffmanTree* tree_ll, HuffmanTree* tree_d,
                                      const unsigned char* in, size_t* bp, size_t inlength)
{
  /*make sure that length values that aren't filled in will be 0, or a wrong tree will be generated*/
  unsigned error = 0;
  unsigned n, HLIT, HDIST, HCLEN, i;
  size_t inbitlength = inlength * 8;

  /*see comments in deflateDynamic for explanation of the context and these variables, it is analogous*/
  unsigned* bitlen_ll = 0; /*lit,len code lengths*/
  unsigned* bitlen_d = 0; /*dist code lengths*/
  /*code length code lengths ("clcl"), the bit lengths of the huffman tree used to compress bitlen_ll and bitlen_d*/
  unsigned* bitlen_cl = 0;
  HuffmanTree tree_cl; /*the code tree for code length codes (the huffman tree for compressed huffman trees)*/

  if((*bp) + 14 > (inlength << 3)) return 49; /*error: the bit pointer is or will go past the memory*/

  /*number of literal/length codes + 257. Unlike the spec, the value 257 is added to it here already*/
  HLIT =  readBitsFromStream(bp, in, 5) + 257;
  /*number of distance codes. Unlike the spec, the value 1 is added to it here already*/
  HDIST = readBitsFromStream(bp, in, 5) + 1;
  /*number of code length codes. Unlike the spec, the value 4 is added to it here already*/
  HCLEN = readBitsFromStream(bp, in, 4) + 4;

  if((*bp) + HCLEN * 3 > (inlength << 3)) return 50; /*error: the bit pointer is or will go past the memory*/

  HuffmanTree_init(&tree_cl);

  while(!error)
  {
    /*read the code length codes out of 3 * (amount of code length codes) bits*/

    bitlen_cl = (unsigned*)lodepng_malloc(NUM_CODE_LENGTH_CODES * sizeof(unsigned));
    if(!bitlen_cl) ERROR_BREAK(83 /*alloc fail*/);

    for(i = 0; i != NUM_CODE_LENGTH_CODES; ++i)
    {
      if(i < HCLEN) bitlen_cl[CLCL_ORDER[i]] = readBitsFromStream(bp, in, 3);
      else bitlen_cl[CLCL_ORDER[i]] = 0; /*if not, it must stay 0*/
    }

    error = HuffmanTree_makeFromLengths(&tree_cl, bitlen_cl, NUM_CODE_LENGTH_CODES, 7);
    if(error) break;

    /*now we can use this tree to read the lengths for the tree that this function will return*/
    bitlen_ll = (unsigned*)lodepng_malloc(NUM_DEFLATE_CODE_SYMBOLS * sizeof(unsigned));
    bitlen_d = (unsigned*)lodepng_malloc(NUM_DISTANCE_SYMBOLS * sizeof(unsigned));
    if(!bitlen_ll || !bitlen_d) ERROR_BREAK(83 /*alloc fail*/);
    for(i = 0; i != NUM_DEFLATE_CODE_SYMBOLS; ++i) bitlen_ll[i] = 0;
    for(i = 0; i != NUM_DISTANCE_SYMBOLS; ++i) bitlen_d[i] = 0;

    /*i is the current symbol we're reading in the part that contains the code lengths of lit/len and dist codes*/
    i = 0;
    while(i < HLIT + HDIST)
    {
      unsigned code = huffmanDecodeSymbol(in, bp, &tree_cl, inbitlength);
      if(code <= 15) /*a length code*/
      {
        if(i < HLIT) bitlen_ll[i] = code;
        else bitlen_d[i - HLIT] = code;
        ++i;
      }
      else if(code == 16) /*repeat previous*/
      {
        unsigned replength = 3; /*read in the 2 bits that indicate repeat length (3-6)*/
        unsigned value; /*set value to the previous code*/

        if(i == 0) ERROR_BREAK(54); /*can't repeat previous if i is 0*/

        if((*bp + 2) > inbitlength) ERROR_BREAK(50); /*error, bit pointer jumps past memory*/
        replength += readBitsFromStream(bp, in, 2);

        if(i < HLIT + 1) value = bitlen_ll[i - 1];
        else value = bitlen_d[i - HLIT - 1];
        /*repeat this value in the next lengths*/
        for(n = 0; n < replength; ++n)
        {
          if(i >= HLIT + HDIST) ERROR_BREAK(13); /*error: i is larger than the amount of codes*/
          if(i < HLIT) bitlen_ll[i] = value;
          else bitlen_d[i - HLIT] = value;
          ++i;
        }
      }
      else if(code == 17) /*repeat "0" 3-10 times*/
      {
        unsigned replength = 3; /*read in the bits that indicate repeat length*/
        if((*bp + 3) > inbitlength) ERROR_BREAK(50); /*error, bit pointer jumps past memory*/
        replength += readBitsFromStream(bp, in, 3);

        /*repeat this value in the next lengths*/
        for(n = 0; n < replength; ++n)
        {
          if(i >= HLIT + HDIST) ERROR_BREAK(14); /*error: i is larger than the amount of codes*/

          if(i < HLIT) bitlen_ll[i] = 0;
          else bitlen_d[i - HLIT] = 0;
          ++i;
        }
      }
      else if(code == 18) /*repeat "0" 11-138 times*/
      {
        unsigned replength = 11; /*read in the bits that indicate repeat length*/
        if((*bp + 7) > inbitlength) ERROR_BREAK(50); /*error, bit pointer jumps past memory*/
        replength += readBitsFromStream(bp, in, 7);

        /*repeat this value in the next lengths*/
        for(n = 0; n < replength; ++n)
        {
          if(i >= HLIT + HDIST) ERROR_BREAK(15); /*error: i is larger than the amount of codes*/

          if(i < HLIT) bitlen_ll[i] = 0;
          else bitlen_d[i - HLIT] = 0;
          ++i;
        }
      }
      else /*if(code == (unsigned)(-1))*/ /*huffmanDecodeSymbol returns (unsigned)(-1) in case of error*/
      {
        if(code == (unsigned)(-1))
        {
          /*return error code 10 or 11 depending on the situation that happened in huffmanDecodeSymbol
          (10=no endcode, 11=wrong jump outside of tree)*/
          error = (*bp) > inbitlength ? 10 : 11;
        }
        else error = 16; /*unexisting code, this can never happen*/
        break;
      }
    }
    if(error) break;

    if(bitlen_ll[256] == 0) ERROR_BREAK(64); /*the length of the end code 256 must be larger than 0*/

    /*now we've finally got HLIT and HDIST, so generate the code trees, and the function is done*/
    error = HuffmanTree_makeFromLengths(tree_ll, bitlen_ll, NUM_DEFLATE_CODE_SYMBOLS, 15);
    if(error) break;
    error = HuffmanTree_makeFromLengths(tree_d, bitlen_d, NUM_DISTANCE_SYMBOLS, 15);

    break; /*end of error-while*/
  }

  lodepng_free(bitlen_cl);
  lodepng_free(bitlen_ll);
  lodepng_free(bitlen_d);
  HuffmanTree_cleanup(&tree_cl);

  return error;
}

/*inflate a block with dynamic of fixed Huffman tree*/
static unsigned inflateHuffmanBlock(ucvector* out, const unsigned char* in, size_t* bp,
                                    size_t* pos, size_t inlength, unsigned btype)
{
  unsigned error = 0;
  HuffmanTree tree_ll; /*the huffman tree for literal and length codes*/
  HuffmanTree tree_d; /*the huffman tree for distance codes*/
  size_t inbitlength = inlength * 8;

  HuffmanTree_init(&tree_ll);
  HuffmanTree_init(&tree_d);

  if(btype == 1) getTreeInflateFixed(&tree_ll, &tree_d);
  else if(btype == 2) error = getTreeInflateDynamic(&tree_ll, &tree_d, in, bp, inlength);

  while(!error) /*decode all symbols until end reached, breaks at end code*/
  {
    /*code_ll is literal, length or end code*/
    unsigned code_ll = huffmanDecodeSymbol(in, bp, &tree_ll, inbitlength);
    if(code_ll <= 255) /*literal symbol*/
    {
      /*ucvector_push_back would do the same, but for some reason the two lines below run 10% faster*/
      if(!ucvector_resize(out, (*pos) + 1)) ERROR_BREAK(83 /*alloc fail*/);
      out->data[*pos] = (unsigned char)code_ll;
      ++(*pos);
    }
    else if(code_ll >= FIRST_LENGTH_CODE_INDEX && code_ll <= LAST_LENGTH_CODE_INDEX) /*length code*/
    {
      unsigned code_d, distance;
      unsigned numextrabits_l, numextrabits_d; /*extra bits for length and distance*/
      size_t start, forward, backward, length;

      /*part 1: get length base*/
      length = LENGTHBASE[code_ll - FIRST_LENGTH_CODE_INDEX];

      /*part 2: get extra bits and add the value of that to length*/
      numextrabits_l = LENGTHEXTRA[code_ll - FIRST_LENGTH_CODE_INDEX];
      if((*bp + numextrabits_l) > inbitlength) ERROR_BREAK(51); /*error, bit pointer will jump past memory*/
      length += readBitsFromStream(bp, in, numextrabits_l);

      /*part 3: get distance code*/
      code_d = huffmanDecodeSymbol(in, bp, &tree_d, inbitlength);
      if(code_d > 29)
      {
        if(code_ll == (unsigned)(-1)) /*huffmanDecodeSymbol returns (unsigned)(-1) in case of error*/
        {
          /*return error code 10 or 11 depending on the situation that happened in huffmanDecodeSymbol
          (10=no endcode, 11=wrong jump outside of tree)*/
          error = (*bp) > inlength * 8 ? 10 : 11;
        }
        else error = 18; /*error: invalid distance code (30-31 are never used)*/
        break;
      }
      distance = DISTANCEBASE[code_d];

      /*part 4: get extra bits from distance*/
      numextrabits_d = DISTANCEEXTRA[code_d];
      if((*bp + numextrabits_d) > inbitlength) ERROR_BREAK(51); /*error, bit pointer will jump past memory*/
      distance += readBitsFromStream(bp, in, numextrabits_d);

      /*part 5: fill in all the out[n] values based on the length and dist*/
      start = (*pos);
      if(distance > start) ERROR_BREAK(52); /*too long backward distance*/
      backward = start - distance;

      if(!ucvector_resize(out, (*pos) + length)) ERROR_BREAK(83 /*alloc fail*/);
      if (distance < length) {
        for(forward = 0; forward < length; ++forward)
        {
          out->data[(*pos)++] = out->data[backward++];
        }
      } else {
        memcpy(out->data + *pos, out->data + backward, length);
        *pos += length;
      }
    }
    else if(code_ll == 256)
    {
      break; /*end code, break the loop*/
    }
    else /*if(code == (unsigned)(-1))*/ /*huffmanDecodeSymbol returns (unsigned)(-1) in case of error*/
    {
      /*return error code 10 or 11 depending on the situation that happened in huffmanDecodeSymbol
      (10=no endcode, 11=wrong jump outside of tree)*/
      error = ((*bp) > inlength * 8) ? 10 : 11;
      break;
    }
  }

  HuffmanTree_cleanup(&tree_ll);
  HuffmanTree_cleanup(&tree_d);

  return error;
}

static unsigned inflateNoCompression(ucvector* out, const unsigned char* in, size_t* bp, size_t* pos, size_t inlength)
{
  size_t p;
  unsigned LEN, NLEN, n, error = 0;

  /*go to first boundary of byte*/
  while(((*bp) & 0x7) != 0) ++(*bp);
  p = (*bp) / 8; /*byte position*/

  /*read LEN (2 bytes) and NLEN (2 bytes)*/
  if(p + 4 >= inlength) return 52; /*error, bit pointer will jump past memory*/
  LEN = in[p] + 256u * in[p + 1]; p += 2;
  NLEN = in[p] + 256u * in[p + 1]; p += 2;

  /*check if 16-bit NLEN is really the one's complement of LEN*/
  if(LEN + NLEN != 65535) return 21; /*error: NLEN is not one's complement of LEN*/

  if(!ucvector_resize(out, (*pos) + LEN)) return 83; /*alloc fail*/

  /*read the literal data: LEN bytes are now stored in the out buffer*/
  if(p + LEN > inlength) return 23; /*error: reading outside of in buffer*/
  for(n = 0; n < LEN; ++n) out->data[(*pos)++] = in[p++];

  (*bp) = p * 8;

  return error;
}

static unsigned lodepng_inflatev(ucvector* out,
                                 const unsigned char* in, size_t insize,
                                 const LodePNGDecompressSettings* settings)
{
  /*bit pointer in the "in" data, current byte is bp >> 3, current bit is bp & 0x7 (from lsb to msb of the byte)*/
  size_t bp = 0;
  unsigned BFINAL = 0;
  size_t pos = 0; /*byte position in the out buffer*/
  unsigned error = 0;

  (void)settings;

  while(!BFINAL)
  {
    unsigned BTYPE;
    if(bp + 2 >= insize * 8) return 52; /*error, bit pointer will jump past memory*/
    BFINAL = readBitFromStream(&bp, in);
    BTYPE = 1u * readBitFromStream(&bp, in);
    BTYPE += 2u * readBitFromStream(&bp, in);

    if(BTYPE == 3) return 20; /*error: invalid BTYPE*/
    else if(BTYPE == 0) error = inflateNoCompression(out, in, &bp, &pos, insize); /*no compression*/
    else error = inflateHuffmanBlock(out, in, &bp, &pos, insize, BTYPE); /*compression, BTYPE 01 or 10*/

    if(error) return error;
  }

  return error;
}

unsigned lodepng_inflate(unsigned char** out, size_t* outsize,
                         const unsigned char* in, size_t insize,
                         const LodePNGDecompressSettings* settings)
{
  unsigned error;
  ucvector v;
  ucvector_init_buffer(&v, *out, *outsize);
  error = lodepng_inflatev(&v, in, insize, settings);
  *out = v.data;
  *outsize = v.size;
  return error;
}

static unsigned inflate(unsigned char** out, size_t* outsize,
                        const unsigned char* in, size_t insize,
                        const LodePNGDecompressSettings* settings)
{
  if(settings->custom_inflate)
  {
    return settings->custom_inflate(out, outsize, in, insize, settings);
  }
  else
  {
    return lodepng_inflate(out, outsize, in, insize, settings);
  }
}

#endif /*LODEPNG_COMPILE_DECODER*/

#ifdef LODEPNG_COMPILE_ENCODER

/* ////////////////////////////////////////////////////////////////////////// */
/* / Deflator (Compressor)                                                  / */
/* ////////////////////////////////////////////////////////////////////////// */

static const size_t MAX_SUPPORTED_DEFLATE_LENGTH = 258;

/*bitlen is the size in bits of the code*/
static void addHuffmanSymbol(size_t* bp, ucvector* compressed, unsigned code, unsigned bitlen)
{
  addBitsToStreamReversed(bp, compressed, code, bitlen);
}

/*search the index in the array, that has the largest value smaller than or equal to the given value,
given array must be sorted (if no value is smaller, it returns the size of the given array)*/
static size_t searchCodeIndex(const unsigned* array, size_t array_size, size_t value)
{
  /*binary search (only small gain over linear). TODO: use CPU log2 instruction for getting symbols instead*/
  size_t left = 1;
  size_t right = array_size - 1;

  while(left <= right) {
    size_t mid = (left + right) >> 1;
    if (array[mid] >= value) right = mid - 1;
    else left = mid + 1;
  }
  if(left >= array_size || array[left] > value) left--;
  return left;
}

static void addLengthDistance(uivector* values, size_t length, size_t distance)
{
  /*values in encoded vector are those used by deflate:
  0-255: literal bytes
  256: end
  257-285: length/distance pair (length code, followed by extra length bits, distance code, extra distance bits)
  286-287: invalid*/

  unsigned length_code = (unsigned)searchCodeIndex(LENGTHBASE, 29, length);
  unsigned extra_length = (unsigned)(length - LENGTHBASE[length_code]);
  unsigned dist_code = (unsigned)searchCodeIndex(DISTANCEBASE, 30, distance);
  unsigned extra_distance = (unsigned)(distance - DISTANCEBASE[dist_code]);

  uivector_push_back(values, length_code + FIRST_LENGTH_CODE_INDEX);
  uivector_push_back(values, extra_length);
  uivector_push_back(values, dist_code);
  uivector_push_back(values, extra_distance);
}

/*3 bytes of data get encoded into two bytes. The hash cannot use more than 3
bytes as input because 3 is the minimum match length for deflate*/
static const unsigned HASH_NUM_VALUES = 65536;
static const unsigned HASH_BIT_MASK = 65535; /*HASH_NUM_VALUES - 1, but C90 does not like that as initializer*/

typedef struct Hash
{
  int* head; /*hash value to head circular pos - can be outdated if went around window*/
  /*circular pos to prev circular pos*/
  unsigned short* chain;
  int* val; /*circular pos to hash value*/

  /*TODO: do this not only for zeros but for any repeated byte. However for PNG
  it's always going to be the zeros that dominate, so not important for PNG*/
  int* headz; /*similar to head, but for chainz*/
  unsigned short* chainz; /*those with same amount of zeros*/
  unsigned short* zeros; /*length of zeros streak, used as a second hash chain*/
} Hash;

static unsigned hash_init(Hash* hash, unsigned windowsize)
{
  unsigned i;
  hash->head = (int*)lodepng_malloc(sizeof(int) * HASH_NUM_VALUES);
  hash->val = (int*)lodepng_malloc(sizeof(int) * windowsize);
  hash->chain = (unsigned short*)lodepng_malloc(sizeof(unsigned short) * windowsize);

  hash->zeros = (unsigned short*)lodepng_malloc(sizeof(unsigned short) * windowsize);
  hash->headz = (int*)lodepng_malloc(sizeof(int) * (MAX_SUPPORTED_DEFLATE_LENGTH + 1));
  hash->chainz = (unsigned short*)lodepng_malloc(sizeof(unsigned short) * windowsize);

  if(!hash->head || !hash->chain || !hash->val  || !hash->headz|| !hash->chainz || !hash->zeros)
  {
    return 83; /*alloc fail*/
  }

  /*initialize hash table*/
  for(i = 0; i != HASH_NUM_VALUES; ++i) hash->head[i] = -1;
  for(i = 0; i != windowsize; ++i) hash->val[i] = -1;
  for(i = 0; i != windowsize; ++i) hash->chain[i] = i; /*same value as index indicates uninitialized*/

  for(i = 0; i <= MAX_SUPPORTED_DEFLATE_LENGTH; ++i) hash->headz[i] = -1;
  for(i = 0; i != windowsize; ++i) hash->chainz[i] = i; /*same value as index indicates uninitialized*/

  return 0;
}

static void hash_cleanup(Hash* hash)
{
  lodepng_free(hash->head);
  lodepng_free(hash->val);
  lodepng_free(hash->chain);

  lodepng_free(hash->zeros);
  lodepng_free(hash->headz);
  lodepng_free(hash->chainz);
}



static unsigned getHash(const unsigned char* data, size_t size, size_t pos)
{
  unsigned result = 0;
  if(pos + 2 < size)
  {
    /*A simple shift and xor hash is used. Since the data of PNGs is dominated
    by zeroes due to the filters, a better hash does not have a significant
    effect on speed in traversing the chain, and causes more time spend on
    calculating the hash.*/
    result ^= (unsigned)(data[pos + 0] << 0u);
    result ^= (unsigned)(data[pos + 1] << 4u);
    result ^= (unsigned)(data[pos + 2] << 8u);
  } else {
    size_t amount, i;
    if(pos >= size) return 0;
    amount = size - pos;
    for(i = 0; i != amount; ++i) result ^= (unsigned)(data[pos + i] << (i * 8u));
  }
  return result & HASH_BIT_MASK;
}

static unsigned countZeros(const unsigned char* data, size_t size, size_t pos)
{
  const unsigned char* start = data + pos;
  const unsigned char* end = start + MAX_SUPPORTED_DEFLATE_LENGTH;
  if(end > data + size) end = data + size;
  data = start;
  while(data != end && *data == 0) ++data;
  /*subtracting two addresses returned as 32-bit number (max value is MAX_SUPPORTED_DEFLATE_LENGTH)*/
  return (unsigned)(data - start);
}

/*wpos = pos & (windowsize - 1)*/
static void updateHashChain(Hash* hash, size_t wpos, unsigned hashval, unsigned short numzeros)
{
  hash->val[wpos] = (int)hashval;
  if(hash->head[hashval] != -1) hash->chain[wpos] = hash->head[hashval];
  hash->head[hashval] = wpos;

  hash->zeros[wpos] = numzeros;
  if(hash->headz[numzeros] != -1) hash->chainz[wpos] = hash->headz[numzeros];
  hash->headz[numzeros] = wpos;
}

/*
LZ77-encode the data. Return value is error code. The input are raw bytes, the output
is in the form of unsigned integers with codes representing for example literal bytes, or
length/distance pairs.
It uses a hash table technique to let it encode faster. When doing LZ77 encoding, a
sliding window (of windowsize) is used, and all past bytes in that window can be used as
the "dictionary". A brute force search through all possible distances would be slow, and
this hash technique is one out of several ways to speed this up.
*/
static unsigned encodeLZ77(uivector* out, Hash* hash,
                           const unsigned char* in, size_t inpos, size_t insize, unsigned windowsize,
                           unsigned minmatch, unsigned nicematch, unsigned lazymatching)
{
  size_t pos;
  unsigned i, error = 0;
  /*for large window lengths, assume the user wants no compression loss. Otherwise, max hash chain length speedup.*/
  unsigned maxchainlength = windowsize >= 8192 ? windowsize : windowsize / 8;
  unsigned maxlazymatch = windowsize >= 8192 ? MAX_SUPPORTED_DEFLATE_LENGTH : 64;

  unsigned usezeros = 1; /*not sure if setting it to false for windowsize < 8192 is better or worse*/
  unsigned numzeros = 0;

  unsigned offset; /*the offset represents the distance in LZ77 terminology*/
  unsigned length;
  unsigned lazy = 0;
  unsigned lazylength = 0, lazyoffset = 0;
  unsigned hashval;
  unsigned current_offset, current_length;
  unsigned prev_offset;
  const unsigned char *lastptr, *foreptr, *backptr;
  unsigned hashpos;

  if(windowsize == 0 || windowsize > 32768) return 60; /*error: windowsize smaller/larger than allowed*/
  if((windowsize & (windowsize - 1)) != 0) return 90; /*error: must be power of two*/

  if(nicematch > MAX_SUPPORTED_DEFLATE_LENGTH) nicematch = MAX_SUPPORTED_DEFLATE_LENGTH;

  for(pos = inpos; pos < insize; ++pos)
  {
    size_t wpos = pos & (windowsize - 1); /*position for in 'circular' hash buffers*/
    unsigned chainlength = 0;

    hashval = getHash(in, insize, pos);

    if(usezeros && hashval == 0)
    {
      if(numzeros == 0) numzeros = countZeros(in, insize, pos);
      else if(pos + numzeros > insize || in[pos + numzeros - 1] != 0) --numzeros;
    }
    else
    {
      numzeros = 0;
    }

    updateHashChain(hash, wpos, hashval, numzeros);

    /*the length and offset found for the current position*/
    length = 0;
    offset = 0;

    hashpos = hash->chain[wpos];

    lastptr = &in[insize < pos + MAX_SUPPORTED_DEFLATE_LENGTH ? insize : pos + MAX_SUPPORTED_DEFLATE_LENGTH];

    /*search for the longest string*/
    prev_offset = 0;
    for(;;)
    {
      if(chainlength++ >= maxchainlength) break;
      current_offset = hashpos <= wpos ? wpos - hashpos : wpos - hashpos + windowsize;

      if(current_offset < prev_offset) break; /*stop when went completely around the circular buffer*/
      prev_offset = current_offset;
      if(current_offset > 0)
      {
        /*test the next characters*/
        foreptr = &in[pos];
        backptr = &in[pos - current_offset];

        /*common case in PNGs is lots of zeros. Quickly skip over them as a speedup*/
        if(numzeros >= 3)
        {
          unsigned skip = hash->zeros[hashpos];
          if(skip > numzeros) skip = numzeros;
          backptr += skip;
          foreptr += skip;
        }

        while(foreptr != lastptr && *backptr == *foreptr) /*maximum supported length by deflate is max length*/
        {
          ++backptr;
          ++foreptr;
        }
        current_length = (unsigned)(foreptr - &in[pos]);

        if(current_length > length)
        {
          length = current_length; /*the longest length*/
          offset = current_offset; /*the offset that is related to this longest length*/
          /*jump out once a length of max length is found (speed gain). This also jumps
          out if length is MAX_SUPPORTED_DEFLATE_LENGTH*/
          if(current_length >= nicematch) break;
        }
      }

      if(hashpos == hash->chain[hashpos]) break;

      if(numzeros >= 3 && length > numzeros)
      {
        hashpos = hash->chainz[hashpos];
        if(hash->zeros[hashpos] != numzeros) break;
      }
      else
      {
        hashpos = hash->chain[hashpos];
        /*outdated hash value, happens if particular value was not encountered in whole last window*/
        if(hash->val[hashpos] != (int)hashval) break;
      }
    }

    if(lazymatching)
    {
      if(!lazy && length >= 3 && length <= maxlazymatch && length < MAX_SUPPORTED_DEFLATE_LENGTH)
      {
        lazy = 1;
        lazylength = length;
        lazyoffset = offset;
        continue; /*try the next byte*/
      }
      if(lazy)
      {
        lazy = 0;
        if(pos == 0) ERROR_BREAK(81);
        if(length > lazylength + 1)
        {
          /*push the previous character as literal*/
          if(!uivector_push_back(out, in[pos - 1])) ERROR_BREAK(83 /*alloc fail*/);
        }
        else
        {
          length = lazylength;
          offset = lazyoffset;
          hash->head[hashval] = -1; /*the same hashchain update will be done, this ensures no wrong alteration*/
          hash->headz[numzeros] = -1; /*idem*/
          --pos;
        }
      }
    }
    if(length >= 3 && offset > windowsize) ERROR_BREAK(86 /*too big (or overflown negative) offset*/);

    /*encode it as length/distance pair or literal value*/
    if(length < 3) /*only lengths of 3 or higher are supported as length/distance pair*/
    {
      if(!uivector_push_back(out, in[pos])) ERROR_BREAK(83 /*alloc fail*/);
    }
    else if(length < minmatch || (length == 3 && offset > 4096))
    {
      /*compensate for the fact that longer offsets have more extra bits, a
      length of only 3 may be not worth it then*/
      if(!uivector_push_back(out, in[pos])) ERROR_BREAK(83 /*alloc fail*/);
    }
    else
    {
      addLengthDistance(out, length, offset);
      for(i = 1; i < length; ++i)
      {
        ++pos;
        wpos = pos & (windowsize - 1);
        hashval = getHash(in, insize, pos);
        if(usezeros && hashval == 0)
        {
          if(numzeros == 0) numzeros = countZeros(in, insize, pos);
          else if(pos + numzeros > insize || in[pos + numzeros - 1] != 0) --numzeros;
        }
        else
        {
          numzeros = 0;
        }
        updateHashChain(hash, wpos, hashval, numzeros);
      }
    }
  } /*end of the loop through each character of input*/

  return error;
}

/* /////////////////////////////////////////////////////////////////////////// */

static unsigned deflateNoCompression(ucvector* out, const unsigned char* data, size_t datasize)
{
  /*non compressed deflate block data: 1 bit BFINAL,2 bits BTYPE,(5 bits): it jumps to start of next byte,
  2 bytes LEN, 2 bytes NLEN, LEN bytes literal DATA*/

  size_t i, j, numdeflateblocks = (datasize + 65534) / 65535;
  unsigned datapos = 0;
  for(i = 0; i != numdeflateblocks; ++i)
  {
    unsigned BFINAL, BTYPE, LEN, NLEN;
    unsigned char firstbyte;

    BFINAL = (i == numdeflateblocks - 1);
    BTYPE = 0;

    firstbyte = (unsigned char)(BFINAL + ((BTYPE & 1) << 1) + ((BTYPE & 2) << 1));
    ucvector_push_back(out, firstbyte);

    LEN = 65535;
    if(datasize - datapos < 65535) LEN = (unsigned)datasize - datapos;
    NLEN = 65535 - LEN;

    ucvector_push_back(out, (unsigned char)(LEN & 255));
    ucvector_push_back(out, (unsigned char)(LEN >> 8));
    ucvector_push_back(out, (unsigned char)(NLEN & 255));
    ucvector_push_back(out, (unsigned char)(NLEN >> 8));

    /*Decompressed data*/
    for(j = 0; j < 65535 && datapos < datasize; ++j)
    {
      ucvector_push_back(out, data[datapos++]);
    }
  }

  return 0;
}

/*
write the lz77-encoded data, which has lit, len and dist codes, to compressed stream using huffman trees.
tree_ll: the tree for lit and len codes.
tree_d: the tree for distance codes.
*/
static void writeLZ77data(size_t* bp, ucvector* out, const uivector* lz77_encoded,
                          const HuffmanTree* tree_ll, const HuffmanTree* tree_d)
{
  size_t i = 0;
  for(i = 0; i != lz77_encoded->size; ++i)
  {
    unsigned val = lz77_encoded->data[i];
    addHuffmanSymbol(bp, out, HuffmanTree_getCode(tree_ll, val), HuffmanTree_getLength(tree_ll, val));
    if(val > 256) /*for a length code, 3 more things have to be added*/
    {
      unsigned length_index = val - FIRST_LENGTH_CODE_INDEX;
      unsigned n_length_extra_bits = LENGTHEXTRA[length_index];
      unsigned length_extra_bits = lz77_encoded->data[++i];

      unsigned distance_code = lz77_encoded->data[++i];

      unsigned distance_index = distance_code;
      unsigned n_distance_extra_bits = DISTANCEEXTRA[distance_index];
      unsigned distance_extra_bits = lz77_encoded->data[++i];

      addBitsToStream(bp, out, length_extra_bits, n_length_extra_bits);
      addHuffmanSymbol(bp, out, HuffmanTree_getCode(tree_d, distance_code),
                       HuffmanTree_getLength(tree_d, distance_code));
      addBitsToStream(bp, out, distance_extra_bits, n_distance_extra_bits);
    }
  }
}

/*Deflate for a block of type "dynamic", that is, with freely, optimally, created huffman trees*/
static unsigned deflateDynamic(ucvector* out, size_t* bp, Hash* hash,
                               const unsigned char* data, size_t datapos, size_t dataend,
                               const LodePNGCompressSettings* settings, unsigned final)
{
  unsigned error = 0;

  /*
  A block is compressed as follows: The PNG data is lz77 encoded, resulting in
  literal bytes and length/distance pairs. This is then huffman compressed with
  two huffman trees. One huffman tree is used for the lit and len values ("ll"),
  another huffman tree is used for the dist values ("d"). These two trees are
  stored using their code lengths, and to compress even more these code lengths
  are also run-length encoded and huffman compressed. This gives a huffman tree
  of code lengths "cl". The code lenghts used to describe this third tree are
  the code length code lengths ("clcl").
  */

  /*The lz77 encoded data, represented with integers since there will also be length and distance codes in it*/
  uivector lz77_encoded;
  HuffmanTree tree_ll; /*tree for lit,len values*/
  HuffmanTree tree_d; /*tree for distance codes*/
  HuffmanTree tree_cl; /*tree for encoding the code lengths representing tree_ll and tree_d*/
  uivector frequencies_ll; /*frequency of lit,len codes*/
  uivector frequencies_d; /*frequency of dist codes*/
  uivector frequencies_cl; /*frequency of code length codes*/
  uivector bitlen_lld; /*lit,len,dist code lenghts (int bits), literally (without repeat codes).*/
  uivector bitlen_lld_e; /*bitlen_lld encoded with repeat codes (this is a rudemtary run length compression)*/
  /*bitlen_cl is the code length code lengths ("clcl"). The bit lengths of codes to represent tree_cl
  (these are written as is in the file, it would be crazy to compress these using yet another huffman
  tree that needs to be represented by yet another set of code lengths)*/
  uivector bitlen_cl;
  size_t datasize = dataend - datapos;

  /*
  Due to the huffman compression of huffman tree representations ("two levels"), there are some anologies:
  bitlen_lld is to tree_cl what data is to tree_ll and tree_d.
  bitlen_lld_e is to bitlen_lld what lz77_encoded is to data.
  bitlen_cl is to bitlen_lld_e what bitlen_lld is to lz77_encoded.
  */

  unsigned BFINAL = final;
  size_t numcodes_ll, numcodes_d, i;
  unsigned HLIT, HDIST, HCLEN;

  uivector_init(&lz77_encoded);
  HuffmanTree_init(&tree_ll);
  HuffmanTree_init(&tree_d);
  HuffmanTree_init(&tree_cl);
  uivector_init(&frequencies_ll);
  uivector_init(&frequencies_d);
  uivector_init(&frequencies_cl);
  uivector_init(&bitlen_lld);
  uivector_init(&bitlen_lld_e);
  uivector_init(&bitlen_cl);

  /*This while loop never loops due to a break at the end, it is here to
  allow breaking out of it to the cleanup phase on error conditions.*/
  while(!error)
  {
    if(settings->use_lz77)
    {
      error = encodeLZ77(&lz77_encoded, hash, data, datapos, dataend, settings->windowsize,
                         settings->minmatch, settings->nicematch, settings->lazymatching);
      if(error) break;
    }
    else
    {
      if(!uivector_resize(&lz77_encoded, datasize)) ERROR_BREAK(83 /*alloc fail*/);
      for(i = datapos; i < dataend; ++i) lz77_encoded.data[i - datapos] = data[i]; /*no LZ77, but still will be Huffman compressed*/
    }

    if(!uivector_resizev(&frequencies_ll, 286, 0)) ERROR_BREAK(83 /*alloc fail*/);
    if(!uivector_resizev(&frequencies_d, 30, 0)) ERROR_BREAK(83 /*alloc fail*/);

    /*Count the frequencies of lit, len and dist codes*/
    for(i = 0; i != lz77_encoded.size; ++i)
    {
      unsigned symbol = lz77_encoded.data[i];
      ++frequencies_ll.data[symbol];
      if(symbol > 256)
      {
        unsigned dist = lz77_encoded.data[i + 2];
        ++frequencies_d.data[dist];
        i += 3;
      }
    }
    frequencies_ll.data[256] = 1; /*there will be exactly 1 end code, at the end of the block*/

    /*Make both huffman trees, one for the lit and len codes, one for the dist codes*/
    error = HuffmanTree_makeFromFrequencies(&tree_ll, frequencies_ll.data, 257, frequencies_ll.size, 15);
    if(error) break;
    /*2, not 1, is chosen for mincodes: some buggy PNG decoders require at least 2 symbols in the dist tree*/
    error = HuffmanTree_makeFromFrequencies(&tree_d, frequencies_d.data, 2, frequencies_d.size, 15);
    if(error) break;

    numcodes_ll = tree_ll.numcodes; if(numcodes_ll > 286) numcodes_ll = 286;
    numcodes_d = tree_d.numcodes; if(numcodes_d > 30) numcodes_d = 30;
    /*store the code lengths of both generated trees in bitlen_lld*/
    for(i = 0; i != numcodes_ll; ++i) uivector_push_back(&bitlen_lld, HuffmanTree_getLength(&tree_ll, (unsigned)i));
    for(i = 0; i != numcodes_d; ++i) uivector_push_back(&bitlen_lld, HuffmanTree_getLength(&tree_d, (unsigned)i));

    /*run-length compress bitlen_ldd into bitlen_lld_e by using repeat codes 16 (copy length 3-6 times),
    17 (3-10 zeroes), 18 (11-138 zeroes)*/
    for(i = 0; i != (unsigned)bitlen_lld.size; ++i)
    {
      unsigned j = 0; /*amount of repititions*/
      while(i + j + 1 < (unsigned)bitlen_lld.size && bitlen_lld.data[i + j + 1] == bitlen_lld.data[i]) ++j;

      if(bitlen_lld.data[i] == 0 && j >= 2) /*repeat code for zeroes*/
      {
        ++j; /*include the first zero*/
        if(j <= 10) /*repeat code 17 supports max 10 zeroes*/
        {
          uivector_push_back(&bitlen_lld_e, 17);
          uivector_push_back(&bitlen_lld_e, j - 3);
        }
        else /*repeat code 18 supports max 138 zeroes*/
        {
          if(j > 138) j = 138;
          uivector_push_back(&bitlen_lld_e, 18);
          uivector_push_back(&bitlen_lld_e, j - 11);
        }
        i += (j - 1);
      }
      else if(j >= 3) /*repeat code for value other than zero*/
      {
        size_t k;
        unsigned num = j / 6, rest = j % 6;
        uivector_push_back(&bitlen_lld_e, bitlen_lld.data[i]);
        for(k = 0; k < num; ++k)
        {
          uivector_push_back(&bitlen_lld_e, 16);
          uivector_push_back(&bitlen_lld_e, 6 - 3);
        }
        if(rest >= 3)
        {
          uivector_push_back(&bitlen_lld_e, 16);
          uivector_push_back(&bitlen_lld_e, rest - 3);
        }
        else j -= rest;
        i += j;
      }
      else /*too short to benefit from repeat code*/
      {
        uivector_push_back(&bitlen_lld_e, bitlen_lld.data[i]);
      }
    }

    /*generate tree_cl, the huffmantree of huffmantrees*/

    if(!uivector_resizev(&frequencies_cl, NUM_CODE_LENGTH_CODES, 0)) ERROR_BREAK(83 /*alloc fail*/);
    for(i = 0; i != bitlen_lld_e.size; ++i)
    {
      ++frequencies_cl.data[bitlen_lld_e.data[i]];
      /*after a repeat code come the bits that specify the number of repetitions,
      those don't need to be in the frequencies_cl calculation*/
      if(bitlen_lld_e.data[i] >= 16) ++i;
    }

    error = HuffmanTree_makeFromFrequencies(&tree_cl, frequencies_cl.data,
                                            frequencies_cl.size, frequencies_cl.size, 7);
    if(error) break;

    if(!uivector_resize(&bitlen_cl, tree_cl.numcodes)) ERROR_BREAK(83 /*alloc fail*/);
    for(i = 0; i != tree_cl.numcodes; ++i)
    {
      /*lenghts of code length tree is in the order as specified by deflate*/
      bitlen_cl.data[i] = HuffmanTree_getLength(&tree_cl, CLCL_ORDER[i]);
    }
    while(bitlen_cl.data[bitlen_cl.size - 1] == 0 && bitlen_cl.size > 4)
    {
      /*remove zeros at the end, but minimum size must be 4*/
      if(!uivector_resize(&bitlen_cl, bitlen_cl.size - 1)) ERROR_BREAK(83 /*alloc fail*/);
    }
    if(error) break;

    /*
    Write everything into the output

    After the BFINAL and BTYPE, the dynamic block consists out of the following:
    - 5 bits HLIT, 5 bits HDIST, 4 bits HCLEN
    - (HCLEN+4)*3 bits code lengths of code length alphabet
    - HLIT + 257 code lenghts of lit/length alphabet (encoded using the code length
      alphabet, + possible repetition codes 16, 17, 18)
    - HDIST + 1 code lengths of distance alphabet (encoded using the code length
      alphabet, + possible repetition codes 16, 17, 18)
    - compressed data
    - 256 (end code)
    */

    /*Write block type*/
    addBitToStream(bp, out, BFINAL);
    addBitToStream(bp, out, 0); /*first bit of BTYPE "dynamic"*/
    addBitToStream(bp, out, 1); /*second bit of BTYPE "dynamic"*/

    /*write the HLIT, HDIST and HCLEN values*/
    HLIT = (unsigned)(numcodes_ll - 257);
    HDIST = (unsigned)(numcodes_d - 1);
    HCLEN = (unsigned)bitlen_cl.size - 4;
    /*trim zeroes for HCLEN. HLIT and HDIST were already trimmed at tree creation*/
    while(!bitlen_cl.data[HCLEN + 4 - 1] && HCLEN > 0) --HCLEN;
    addBitsToStream(bp, out, HLIT, 5);
    addBitsToStream(bp, out, HDIST, 5);
    addBitsToStream(bp, out, HCLEN, 4);

    /*write the code lenghts of the code length alphabet*/
    for(i = 0; i != HCLEN + 4; ++i) addBitsToStream(bp, out, bitlen_cl.data[i], 3);

    /*write the lenghts of the lit/len AND the dist alphabet*/
    for(i = 0; i != bitlen_lld_e.size; ++i)
    {
      addHuffmanSymbol(bp, out, HuffmanTree_getCode(&tree_cl, bitlen_lld_e.data[i]),
                       HuffmanTree_getLength(&tree_cl, bitlen_lld_e.data[i]));
      /*extra bits of repeat codes*/
      if(bitlen_lld_e.data[i] == 16) addBitsToStream(bp, out, bitlen_lld_e.data[++i], 2);
      else if(bitlen_lld_e.data[i] == 17) addBitsToStream(bp, out, bitlen_lld_e.data[++i], 3);
      else if(bitlen_lld_e.data[i] == 18) addBitsToStream(bp, out, bitlen_lld_e.data[++i], 7);
    }

    /*write the compressed data symbols*/
    writeLZ77data(bp, out, &lz77_encoded, &tree_ll, &tree_d);
    /*error: the length of the end code 256 must be larger than 0*/
    if(HuffmanTree_getLength(&tree_ll, 256) == 0) ERROR_BREAK(64);

    /*write the end code*/
    addHuffmanSymbol(bp, out, HuffmanTree_getCode(&tree_ll, 256), HuffmanTree_getLength(&tree_ll, 256));

    break; /*end of error-while*/
  }

  /*cleanup*/
  uivector_cleanup(&lz77_encoded);
  HuffmanTree_cleanup(&tree_ll);
  HuffmanTree_cleanup(&tree_d);
  HuffmanTree_cleanup(&tree_cl);
  uivector_cleanup(&frequencies_ll);
  uivector_cleanup(&frequencies_d);
  uivector_cleanup(&frequencies_cl);
  uivector_cleanup(&bitlen_lld_e);
  uivector_cleanup(&bitlen_lld);
  uivector_cleanup(&bitlen_cl);

  return error;
}

static unsigned deflateFixed(ucvector* out, size_t* bp, Hash* hash,
                             const unsigned char* data,
                             size_t datapos, size_t dataend,
                             const LodePNGCompressSettings* settings, unsigned final)
{
  HuffmanTree tree_ll; /*tree for literal values and length codes*/
  HuffmanTree tree_d; /*tree for distance codes*/

  unsigned BFINAL = final;
  unsigned error = 0;
  size_t i;

  HuffmanTree_init(&tree_ll);
  HuffmanTree_init(&tree_d);

  generateFixedLitLenTree(&tree_ll);
  generateFixedDistanceTree(&tree_d);

  addBitToStream(bp, out, BFINAL);
  addBitToStream(bp, out, 1); /*first bit of BTYPE*/
  addBitToStream(bp, out, 0); /*second bit of BTYPE*/

  if(settings->use_lz77) /*LZ77 encoded*/
  {
    uivector lz77_encoded;
    uivector_init(&lz77_encoded);
    error = encodeLZ77(&lz77_encoded, hash, data, datapos, dataend, settings->windowsize,
                       settings->minmatch, settings->nicematch, settings->lazymatching);
    if(!error) writeLZ77data(bp, out, &lz77_encoded, &tree_ll, &tree_d);
    uivector_cleanup(&lz77_encoded);
  }
  else /*no LZ77, but still will be Huffman compressed*/
  {
    for(i = datapos; i < dataend; ++i)
    {
      addHuffmanSymbol(bp, out, HuffmanTree_getCode(&tree_ll, data[i]), HuffmanTree_getLength(&tree_ll, data[i]));
    }
  }
  /*add END code*/
  if(!error) addHuffmanSymbol(bp, out, HuffmanTree_getCode(&tree_ll, 256), HuffmanTree_getLength(&tree_ll, 256));

  /*cleanup*/
  HuffmanTree_cleanup(&tree_ll);
  HuffmanTree_cleanup(&tree_d);

  return error;
}

static unsigned lodepng_deflatev(ucvector* out, const unsigned char* in, size_t insize,
                                 const LodePNGCompressSettings* settings)
{
  unsigned error = 0;
  size_t i, blocksize, numdeflateblocks;
  size_t bp = 0; /*the bit pointer*/
  Hash hash;

  if(settings->btype > 2) return 61;
  else if(settings->btype == 0) return deflateNoCompression(out, in, insize);
  else if(settings->btype == 1) blocksize = insize;
  else /*if(settings->btype == 2)*/
  {
    /*on PNGs, deflate blocks of 65-262k seem to give most dense encoding*/
    blocksize = insize / 8 + 8;
    if(blocksize < 65536) blocksize = 65536;
    if(blocksize > 262144) blocksize = 262144;
  }

  numdeflateblocks = (insize + blocksize - 1) / blocksize;
  if(numdeflateblocks == 0) numdeflateblocks = 1;

  error = hash_init(&hash, settings->windowsize);
  if(error) return error;

  for(i = 0; i != numdeflateblocks && !error; ++i)
  {
    unsigned final = (i == numdeflateblocks - 1);
    size_t start = i * blocksize;
    size_t end = start + blocksize;
    if(end > insize) end = insize;

    if(settings->btype == 1) error = deflateFixed(out, &bp, &hash, in, start, end, settings, final);
    else if(settings->btype == 2) error = deflateDynamic(out, &bp, &hash, in, start, end, settings, final);
  }

  hash_cleanup(&hash);

  return error;
}

unsigned lodepng_deflate(unsigned char** out, size_t* outsize,
                         const unsigned char* in, size_t insize,
                         const LodePNGCompressSettings* settings)
{
  unsigned error;
  ucvector v;
  ucvector_init_buffer(&v, *out, *outsize);
  error = lodepng_deflatev(&v, in, insize, settings);
  *out = v.data;
  *outsize = v.size;
  return error;
}

static unsigned deflate(unsigned char** out, size_t* outsize,
                        const unsigned char* in, size_t insize,
                        const LodePNGCompressSettings* settings)
{
  if(settings->custom_deflate)
  {
    return settings->custom_deflate(out, outsize, in, insize, settings);
  }
  else
  {
    return lodepng_deflate(out, outsize, in, insize, settings);
  }
}

#endif /*LODEPNG_COMPILE_DECODER*/

/* ////////////////////////////////////////////////////////////////////////// */
/* / Adler32                                                                  */
/* ////////////////////////////////////////////////////////////////////////// */

static unsigned update_adler32(unsigned adler, const unsigned char* data, unsigned len)
{
   unsigned s1 = adler & 0xffff;
   unsigned s2 = (adler >> 16) & 0xffff;

  while(len > 0)
  {
    /*at least 5550 sums can be done before the sums overflow, saving a lot of module divisions*/
    unsigned amount = len > 5550 ? 5550 : len;
    len -= amount;
    while(amount > 0)
    {
      s1 += (*data++);
      s2 += s1;
      --amount;
    }
    s1 %= 65521;
    s2 %= 65521;
  }

  return (s2 << 16) | s1;
}

/*Return the adler32 of the bytes data[0..len-1]*/
static unsigned adler32(const unsigned char* data, unsigned len)
{
  return update_adler32(1L, data, len);
}

/* ////////////////////////////////////////////////////////////////////////// */
/* / Zlib                                                                   / */
/* ////////////////////////////////////////////////////////////////////////// */

#ifdef LODEPNG_COMPILE_DECODER

unsigned lodepng_zlib_decompress(unsigned char** out, size_t* outsize, const unsigned char* in,
                                 size_t insize, const LodePNGDecompressSettings* settings)
{
  unsigned error = 0;
  unsigned CM, CINFO, FDICT;

  if(insize < 2) return 53; /*error, size of zlib data too small*/
  /*read information from zlib header*/
  if((in[0] * 256 + in[1]) % 31 != 0)
  {
    /*error: 256 * in[0] + in[1] must be a multiple of 31, the FCHECK value is supposed to be made that way*/
    return 24;
  }

  CM = in[0] & 15;
  CINFO = (in[0] >> 4) & 15;
  /*FCHECK = in[1] & 31;*/ /*FCHECK is already tested above*/
  FDICT = (in[1] >> 5) & 1;
  /*FLEVEL = (in[1] >> 6) & 3;*/ /*FLEVEL is not used here*/

  if(CM != 8 || CINFO > 7)
  {
    /*error: only compression method 8: inflate with sliding window of 32k is supported by the PNG spec*/
    return 25;
  }
  if(FDICT != 0)
  {
    /*error: the specification of PNG says about the zlib stream:
      "The additional flags shall not specify a preset dictionary."*/
    return 26;
  }

  error = inflate(out, outsize, in + 2, insize - 2, settings);
  if(error) return error;

  if(!settings->ignore_adler32)
  {
    unsigned ADLER32 = lodepng_read32bitInt(&in[insize - 4]);
    unsigned checksum = adler32(*out, (unsigned)(*outsize));
    if(checksum != ADLER32) return 58; /*error, adler checksum not correct, data must be corrupted*/
  }

  return 0; /*no error*/
}

static unsigned zlib_decompress(unsigned char** out, size_t* outsize, const unsigned char* in,
                                size_t insize, const LodePNGDecompressSettings* settings)
{
  if(settings->custom_zlib)
  {
    return settings->custom_zlib(out, outsize, in, insize, settings);
  }
  else
  {
    return lodepng_zlib_decompress(out, outsize, in, insize, settings);
  }
}

#endif /*LODEPNG_COMPILE_DECODER*/

#ifdef LODEPNG_COMPILE_ENCODER

unsigned lodepng_zlib_compress(unsigned char** out, size_t* outsize, const unsigned char* in,
                               size_t insize, const LodePNGCompressSettings* settings)
{
  /*initially, *out must be NULL and outsize 0, if you just give some random *out
  that's pointing to a non allocated buffer, this'll crash*/
  ucvector outv;
  size_t i;
  unsigned error;
  unsigned char* deflatedata = 0;
  size_t deflatesize = 0;

  /*zlib data: 1 byte CMF (CM+CINFO), 1 byte FLG, deflate data, 4 byte ADLER32 checksum of the Decompressed data*/
  unsigned CMF = 120; /*0b01111000: CM 8, CINFO 7. With CINFO 7, any window size up to 32768 can be used.*/
  unsigned FLEVEL = 0;
  unsigned FDICT = 0;
  unsigned CMFFLG = 256 * CMF + FDICT * 32 + FLEVEL * 64;
  unsigned FCHECK = 31 - CMFFLG % 31;
  CMFFLG += FCHECK;

  /*ucvector-controlled version of the output buffer, for dynamic array*/
  ucvector_init_buffer(&outv, *out, *outsize);

  ucvector_push_back(&outv, (unsigned char)(CMFFLG >> 8));
  ucvector_push_back(&outv, (unsigned char)(CMFFLG & 255));

  error = deflate(&deflatedata, &deflatesize, in, insize, settings);

  if(!error)
  {
    unsigned ADLER32 = adler32(in, (unsigned)insize);
    for(i = 0; i != deflatesize; ++i) ucvector_push_back(&outv, deflatedata[i]);
    lodepng_free(deflatedata);
    lodepng_add32bitInt(&outv, ADLER32);
  }

  *out = outv.data;
  *outsize = outv.size;

  return error;
}

/* compress using the default or custom zlib function */
static unsigned zlib_compress(unsigned char** out, size_t* outsize, const unsigned char* in,
                              size_t insize, const LodePNGCompressSettings* settings)
{
  if(settings->custom_zlib)
  {
    return settings->custom_zlib(out, outsize, in, insize, settings);
  }
  else
  {
    return lodepng_zlib_compress(out, outsize, in, insize, settings);
  }
}

#endif /*LODEPNG_COMPILE_ENCODER*/

#else /*no LODEPNG_COMPILE_ZLIB*/

#ifdef LODEPNG_COMPILE_DECODER
static unsigned zlib_decompress(unsigned char** out, size_t* outsize, const unsigned char* in,
                                size_t insize, const LodePNGDecompressSettings* settings)
{
  if(!settings->custom_zlib) return 87; /*no custom zlib function provided */
  return settings->custom_zlib(out, outsize, in, insize, settings);
}
#endif /*LODEPNG_COMPILE_DECODER*/
#ifdef LODEPNG_COMPILE_ENCODER
static unsigned zlib_compress(unsigned char** out, size_t* outsize, const unsigned char* in,
                              size_t insize, const LodePNGCompressSettings* settings)
{
  if(!settings->custom_zlib) return 87; /*no custom zlib function provided */
  return settings->custom_zlib(out, outsize, in, insize, settings);
}
#endif /*LODEPNG_COMPILE_ENCODER*/

#endif /*LODEPNG_COMPILE_ZLIB*/

/* ////////////////////////////////////////////////////////////////////////// */

#ifdef LODEPNG_COMPILE_ENCODER

/*this is a good tradeoff between speed and compression ratio*/
#define DEFAULT_WINDOWSIZE 2048

void lodepng_compress_settings_init(LodePNGCompressSettings* settings)
{
  /*compress with dynamic huffman tree (not in the mathematical sense, just not the predefined one)*/
  settings->btype = 2;
  settings->use_lz77 = 1;
  settings->windowsize = DEFAULT_WINDOWSIZE;
  settings->minmatch = 3;
  settings->nicematch = 128;
  settings->lazymatching = 1;

  settings->custom_zlib = 0;
  settings->custom_deflate = 0;
  settings->custom_context = 0;
}

const LodePNGCompressSettings lodepng_default_compress_settings = {2, 1, DEFAULT_WINDOWSIZE, 3, 128, 1, 0, 0, 0};


#endif /*LODEPNG_COMPILE_ENCODER*/

#ifdef LODEPNG_COMPILE_DECODER

void lodepng_decompress_settings_init(LodePNGDecompressSettings* settings)
{
  settings->ignore_adler32 = 0;

  settings->custom_zlib = 0;
  settings->custom_inflate = 0;
  settings->custom_context = 0;
}

const LodePNGDecompressSettings lodepng_default_decompress_settings = {0, 0, 0, 0};

#endif /*LODEPNG_COMPILE_DECODER*/

/* ////////////////////////////////////////////////////////////////////////// */
/* ////////////////////////////////////////////////////////////////////////// */
/* // End of Zlib related code. Begin of PNG related code.                 // */
/* ////////////////////////////////////////////////////////////////////////// */
/* ////////////////////////////////////////////////////////////////////////// */

#ifdef LODEPNG_COMPILE_PNG

/* ////////////////////////////////////////////////////////////////////////// */
/* / CRC32                                                                  / */
/* ////////////////////////////////////////////////////////////////////////// */


#ifndef LODEPNG_NO_COMPILE_CRC
/* CRC polynomial: 0xedb88320 */
static unsigned lodepng_crc32_table[256] = {
           0u, 1996959894u, 3993919788u, 2567524794u,  124634137u, 1886057615u, 3915621685u, 2657392035u,
   249268274u, 2044508324u, 3772115230u, 2547177864u,  162941995u, 2125561021u, 3887607047u, 2428444049u,
   498536548u, 1789927666u, 4089016648u, 2227061214u,  450548861u, 1843258603u, 4107580753u, 2211677639u,
   325883990u, 1684777152u, 4251122042u, 2321926636u,  335633487u, 1661365465u, 4195302755u, 2366115317u,
   997073096u, 1281953886u, 3579855332u, 2724688242u, 1006888145u, 1258607687u, 3524101629u, 2768942443u,
   901097722u, 1119000684u, 3686517206u, 2898065728u,  853044451u, 1172266101u, 3705015759u, 2882616665u,
   651767980u, 1373503546u, 3369554304u, 3218104598u,  565507253u, 1454621731u, 3485111705u, 3099436303u,
   671266974u, 1594198024u, 3322730930u, 2970347812u,  795835527u, 1483230225u, 3244367275u, 3060149565u,
  1994146192u,   31158534u, 2563907772u, 4023717930u, 1907459465u,  112637215u, 2680153253u, 3904427059u,
  2013776290u,  251722036u, 2517215374u, 3775830040u, 2137656763u,  141376813u, 2439277719u, 3865271297u,
  1802195444u,  476864866u, 2238001368u, 4066508878u, 1812370925u,  453092731u, 2181625025u, 4111451223u,
  1706088902u,  314042704u, 2344532202u, 4240017532u, 1658658271u,  366619977u, 2362670323u, 4224994405u,
  1303535960u,  984961486u, 2747007092u, 3569037538u, 1256170817u, 1037604311u, 2765210733u, 3554079995u,
  1131014506u,  879679996u, 2909243462u, 3663771856u, 1141124467u,  855842277u, 2852801631u, 3708648649u,
  1342533948u,  654459306u, 3188396048u, 3373015174u, 1466479909u,  544179635u, 3110523913u, 3462522015u,
  1591671054u,  702138776u, 2966460450u, 3352799412u, 1504918807u,  783551873u, 3082640443u, 3233442989u,
  3988292384u, 2596254646u,   62317068u, 1957810842u, 3939845945u, 2647816111u,   81470997u, 1943803523u,
  3814918930u, 2489596804u,  225274430u, 2053790376u, 3826175755u, 2466906013u,  167816743u, 2097651377u,
  4027552580u, 2265490386u,  503444072u, 1762050814u, 4150417245u, 2154129355u,  426522225u, 1852507879u,
  4275313526u, 2312317920u,  282753626u, 1742555852u, 4189708143u, 2394877945u,  397917763u, 1622183637u,
  3604390888u, 2714866558u,  953729732u, 1340076626u, 3518719985u, 2797360999u, 1068828381u, 1219638859u,
  3624741850u, 2936675148u,  906185462u, 1090812512u, 3747672003u, 2825379669u,  829329135u, 1181335161u,
  3412177804u, 3160834842u,  628085408u, 1382605366u, 3423369109u, 3138078467u,  570562233u, 1426400815u,
  3317316542u, 2998733608u,  733239954u, 1555261956u, 3268935591u, 3050360625u,  752459403u, 1541320221u,
  2607071920u, 3965973030u, 1969922972u,   40735498u, 2617837225u, 3943577151u, 1913087877u,   83908371u,
  2512341634u, 3803740692u, 2075208622u,  213261112u, 2463272603u, 3855990285u, 2094854071u,  198958881u,
  2262029012u, 4057260610u, 1759359992u,  534414190u, 2176718541u, 4139329115u, 1873836001u,  414664567u,
  2282248934u, 4279200368u, 1711684554u,  285281116u, 2405801727u, 4167216745u, 1634467795u,  376229701u,
  2685067896u, 3608007406u, 1308918612u,  956543938u, 2808555105u, 3495958263u, 1231636301u, 1047427035u,
  2932959818u, 3654703836u, 1088359270u,  936918000u, 2847714899u, 3736837829u, 1202900863u,  817233897u,
  3183342108u, 3401237130u, 1404277552u,  615818150u, 3134207493u, 3453421203u, 1423857449u,  601450431u,
  3009837614u, 3294710456u, 1567103746u,  711928724u, 3020668471u, 3272380065u, 1510334235u,  755167117u
};

/*Return the CRC of the bytes buf[0..len-1].*/
unsigned lodepng_crc32(const unsigned char* data, size_t length)
{
  unsigned r = 0xffffffffu;
  size_t i;
  for(i = 0; i < length; ++i)
  {
    r = lodepng_crc32_table[(r ^ data[i]) & 0xff] ^ (r >> 8);
  }
  return r ^ 0xffffffffu;
}
#else /* !LODEPNG_NO_COMPILE_CRC */
unsigned lodepng_crc32(const unsigned char* data, size_t length);
#endif /* !LODEPNG_NO_COMPILE_CRC */

/* ////////////////////////////////////////////////////////////////////////// */
/* / Reading and writing single bits and bytes from/to stream for LodePNG   / */
/* ////////////////////////////////////////////////////////////////////////// */

static unsigned char readBitFromReversedStream(size_t* bitpointer, const unsigned char* bitstream)
{
  unsigned char result = (unsigned char)((bitstream[(*bitpointer) >> 3] >> (7 - ((*bitpointer) & 0x7))) & 1);
  ++(*bitpointer);
  return result;
}

static unsigned readBitsFromReversedStream(size_t* bitpointer, const unsigned char* bitstream, size_t nbits)
{
  unsigned result = 0;
  size_t i;
  for(i = 0 ; i < nbits; ++i)
  {
    result <<= 1;
    result |= (unsigned)readBitFromReversedStream(bitpointer, bitstream);
  }
  return result;
}

#ifdef LODEPNG_COMPILE_DECODER
static void setBitOfReversedStream0(size_t* bitpointer, unsigned char* bitstream, unsigned char bit)
{
  /*the current bit in bitstream must be 0 for this to work*/
  if(bit)
  {
    /*earlier bit of huffman code is in a lesser significant bit of an earlier byte*/
    bitstream[(*bitpointer) >> 3] |= (bit << (7 - ((*bitpointer) & 0x7)));
  }
  ++(*bitpointer);
}
#endif /*LODEPNG_COMPILE_DECODER*/

static void setBitOfReversedStream(size_t* bitpointer, unsigned char* bitstream, unsigned char bit)
{
  /*the current bit in bitstream may be 0 or 1 for this to work*/
  if(bit == 0) bitstream[(*bitpointer) >> 3] &=  (unsigned char)(~(1 << (7 - ((*bitpointer) & 0x7))));
  else         bitstream[(*bitpointer) >> 3] |=  (1 << (7 - ((*bitpointer) & 0x7)));
  ++(*bitpointer);
}

/* ////////////////////////////////////////////////////////////////////////// */
/* / PNG chunks                                                             / */
/* ////////////////////////////////////////////////////////////////////////// */

unsigned lodepng_chunk_length(const unsigned char* chunk)
{
  return lodepng_read32bitInt(&chunk[0]);
}

void lodepng_chunk_type(char type[5], const unsigned char* chunk)
{
  unsigned i;
  for(i = 0; i != 4; ++i) type[i] = (char)chunk[4 + i];
  type[4] = 0; /*null termination char*/
}

unsigned char lodepng_chunk_type_equals(const unsigned char* chunk, const char* type)
{
  if(strlen(type) != 4) return 0;
  return (chunk[4] == type[0] && chunk[5] == type[1] && chunk[6] == type[2] && chunk[7] == type[3]);
}

unsigned char lodepng_chunk_ancillary(const unsigned char* chunk)
{
  return((chunk[4] & 32) != 0);
}

unsigned char lodepng_chunk_private(const unsigned char* chunk)
{
  return((chunk[6] & 32) != 0);
}

unsigned char lodepng_chunk_safetocopy(const unsigned char* chunk)
{
  return((chunk[7] & 32) != 0);
}

unsigned char* lodepng_chunk_data(unsigned char* chunk)
{
  return &chunk[8];
}

const unsigned char* lodepng_chunk_data_const(const unsigned char* chunk)
{
  return &chunk[8];
}

unsigned lodepng_chunk_check_crc(const unsigned char* chunk)
{
  unsigned length = lodepng_chunk_length(chunk);
  unsigned CRC = lodepng_read32bitInt(&chunk[length + 8]);
  /*the CRC is taken of the data and the 4 chunk type letters, not the length*/
  unsigned checksum = lodepng_crc32(&chunk[4], length + 4);
  if(CRC != checksum) return 1;
  else return 0;
}

void lodepng_chunk_generate_crc(unsigned char* chunk)
{
  unsigned length = lodepng_chunk_length(chunk);
  unsigned CRC = lodepng_crc32(&chunk[4], length + 4);
  lodepng_set32bitInt(chunk + 8 + length, CRC);
}

unsigned char* lodepng_chunk_next(unsigned char* chunk)
{
  unsigned total_chunk_length = lodepng_chunk_length(chunk) + 12;
  return &chunk[total_chunk_length];
}

const unsigned char* lodepng_chunk_next_const(const unsigned char* chunk)
{
  unsigned total_chunk_length = lodepng_chunk_length(chunk) + 12;
  return &chunk[total_chunk_length];
}

unsigned lodepng_chunk_append(unsigned char** out, size_t* outlength, const unsigned char* chunk)
{
  unsigned i;
  unsigned total_chunk_length = lodepng_chunk_length(chunk) + 12;
  unsigned char *chunk_start, *new_buffer;
  size_t new_length = (*outlength) + total_chunk_length;
  if(new_length < total_chunk_length || new_length < (*outlength)) return 77; /*integer overflow happened*/

  new_buffer = (unsigned char*)lodepng_realloc(*out, new_length);
  if(!new_buffer) return 83; /*alloc fail*/
  (*out) = new_buffer;
  (*outlength) = new_length;
  chunk_start = &(*out)[new_length - total_chunk_length];

  for(i = 0; i != total_chunk_length; ++i) chunk_start[i] = chunk[i];

  return 0;
}

unsigned lodepng_chunk_create(unsigned char** out, size_t* outlength, unsigned length,
                              const char* type, const unsigned char* data)
{
  unsigned i;
  unsigned char *chunk, *new_buffer;
  size_t new_length = (*outlength) + length + 12;
  if(new_length < length + 12 || new_length < (*outlength)) return 77; /*integer overflow happened*/
  new_buffer = (unsigned char*)lodepng_realloc(*out, new_length);
  if(!new_buffer) return 83; /*alloc fail*/
  (*out) = new_buffer;
  (*outlength) = new_length;
  chunk = &(*out)[(*outlength) - length - 12];

  /*1: length*/
  lodepng_set32bitInt(chunk, (unsigned)length);

  /*2: chunk name (4 letters)*/
  chunk[4] = (unsigned char)type[0];
  chunk[5] = (unsigned char)type[1];
  chunk[6] = (unsigned char)type[2];
  chunk[7] = (unsigned char)type[3];

  /*3: the data*/
  for(i = 0; i != length; ++i) chunk[8 + i] = data[i];

  /*4: CRC (of the chunkname characters and the data)*/
  lodepng_chunk_generate_crc(chunk);

  return 0;
}

/* ////////////////////////////////////////////////////////////////////////// */
/* / Color types and such                                                   / */
/* ////////////////////////////////////////////////////////////////////////// */

/*return type is a LodePNG error code*/
static unsigned checkColorValidity(LodePNGColorType colortype, unsigned bd) /*bd = bitdepth*/
{
  switch(colortype)
  {
    case 0: if(!(bd == 1 || bd == 2 || bd == 4 || bd == 8 || bd == 16)) return 37; break; /*grey*/
    case 2: if(!(                                 bd == 8 || bd == 16)) return 37; break; /*RGB*/
    case 3: if(!(bd == 1 || bd == 2 || bd == 4 || bd == 8            )) return 37; break; /*palette*/
    case 4: if(!(                                 bd == 8 || bd == 16)) return 37; break; /*grey + alpha*/
    case 6: if(!(                                 bd == 8 || bd == 16)) return 37; break; /*RGBA*/
    default: return 31;
  }
  return 0; /*allowed color type / bits combination*/
}

static unsigned getNumColorChannels(LodePNGColorType colortype)
{
  switch(colortype)
  {
    case 0: return 1; /*grey*/
    case 2: return 3; /*RGB*/
    case 3: return 1; /*palette*/
    case 4: return 2; /*grey + alpha*/
    case 6: return 4; /*RGBA*/
  }
  return 0; /*unexisting color type*/
}

static unsigned lodepng_get_bpp_lct(LodePNGColorType colortype, unsigned bitdepth)
{
  /*bits per pixel is amount of channels * bits per channel*/
  return getNumColorChannels(colortype) * bitdepth;
}

/* ////////////////////////////////////////////////////////////////////////// */

void lodepng_color_mode_init(LodePNGColorMode* info)
{
  info->key_defined = 0;
  info->key_r = info->key_g = info->key_b = 0;
  info->colortype = LCT_RGBA;
  info->bitdepth = 8;
  info->palette = 0;
  info->palettesize = 0;
}

void lodepng_color_mode_cleanup(LodePNGColorMode* info)
{
  lodepng_palette_clear(info);
}

unsigned lodepng_color_mode_copy(LodePNGColorMode* dest, const LodePNGColorMode* source)
{
  size_t i;
  lodepng_color_mode_cleanup(dest);
  *dest = *source;
  if(source->palette)
  {
    dest->palette = (unsigned char*)lodepng_malloc(1024);
    if(!dest->palette && source->palettesize) return 83; /*alloc fail*/
    for(i = 0; i != source->palettesize * 4; ++i) dest->palette[i] = source->palette[i];
  }
  return 0;
}

static int lodepng_color_mode_equal(const LodePNGColorMode* a, const LodePNGColorMode* b)
{
  size_t i;
  if(a->colortype != b->colortype) return 0;
  if(a->bitdepth != b->bitdepth) return 0;
  if(a->key_defined != b->key_defined) return 0;
  if(a->key_defined)
  {
    if(a->key_r != b->key_r) return 0;
    if(a->key_g != b->key_g) return 0;
    if(a->key_b != b->key_b) return 0;
  }
  /*if one of the palette sizes is 0, then we consider it to be the same as the
  other: it means that e.g. the palette was not given by the user and should be
  considered the same as the palette inside the PNG.*/
  if(1/*a->palettesize != 0 && b->palettesize != 0*/) {
    if(a->palettesize != b->palettesize) return 0;
    for(i = 0; i != a->palettesize * 4; ++i)
    {
      if(a->palette[i] != b->palette[i]) return 0;
    }
  }
  return 1;
}

void lodepng_palette_clear(LodePNGColorMode* info)
{
  if(info->palette) lodepng_free(info->palette);
  info->palette = 0;
  info->palettesize = 0;
}

unsigned lodepng_palette_add(LodePNGColorMode* info,
                             unsigned char r, unsigned char g, unsigned char b, unsigned char a)
{
  unsigned char* data;
  /*the same resize technique as C++ std::vectors is used, and here it's made so that for a palette with
  the max of 256 colors, it'll have the exact alloc size*/
  if(!info->palette) /*allocate palette if empty*/
  {
    /*room for 256 colors with 4 bytes each*/
    data = (unsigned char*)lodepng_realloc(info->palette, 1024);
    if(!data) return 83; /*alloc fail*/
    else info->palette = data;
  }
  info->palette[4 * info->palettesize + 0] = r;
  info->palette[4 * info->palettesize + 1] = g;
  info->palette[4 * info->palettesize + 2] = b;
  info->palette[4 * info->palettesize + 3] = a;
  ++info->palettesize;
  return 0;
}

unsigned lodepng_get_bpp(const LodePNGColorMode* info)
{
  /*calculate bits per pixel out of colortype and bitdepth*/
  return lodepng_get_bpp_lct(info->colortype, info->bitdepth);
}

unsigned lodepng_get_channels(const LodePNGColorMode* info)
{
  return getNumColorChannels(info->colortype);
}

unsigned lodepng_is_greyscale_type(const LodePNGColorMode* info)
{
  return info->colortype == LCT_GREY || info->colortype == LCT_GREY_ALPHA;
}

unsigned lodepng_is_alpha_type(const LodePNGColorMode* info)
{
  return (info->colortype & 4) != 0; /*4 or 6*/
}

unsigned lodepng_is_palette_type(const LodePNGColorMode* info)
{
  return info->colortype == LCT_PALETTE;
}

unsigned lodepng_has_palette_alpha(const LodePNGColorMode* info)
{
  size_t i;
  for(i = 0; i != info->palettesize; ++i)
  {
    if(info->palette[i * 4 + 3] < 255) return 1;
  }
  return 0;
}

unsigned lodepng_can_have_alpha(const LodePNGColorMode* info)
{
  return info->key_defined
      || lodepng_is_alpha_type(info)
      || lodepng_has_palette_alpha(info);
}

size_t lodepng_get_raw_size(unsigned w, unsigned h, const LodePNGColorMode* color)
{
  /*will not overflow for any color type if roughly w * h < 268435455*/
  size_t bpp = lodepng_get_bpp(color);
  size_t n = w * h;
  return ((n / 8) * bpp) + ((n & 7) * bpp + 7) / 8;
}

size_t lodepng_get_raw_size_lct(unsigned w, unsigned h, LodePNGColorType colortype, unsigned bitdepth)
{
  /*will not overflow for any color type if roughly w * h < 268435455*/
  size_t bpp = lodepng_get_bpp_lct(colortype, bitdepth);
  size_t n = w * h;
  return ((n / 8) * bpp) + ((n & 7) * bpp + 7) / 8;
}


#ifdef LODEPNG_COMPILE_PNG
#ifdef LODEPNG_COMPILE_DECODER
/*in an idat chunk, each scanline is a multiple of 8 bits, unlike the lodepng output buffer*/
static size_t lodepng_get_raw_size_idat(unsigned w, unsigned h, const LodePNGColorMode* color)
{
  /*will not overflow for any color type if roughly w * h < 268435455*/
  size_t bpp = lodepng_get_bpp(color);
  size_t line = ((w / 8) * bpp) + ((w & 7) * bpp + 7) / 8;
  return h * line;
}
#endif /*LODEPNG_COMPILE_DECODER*/
#endif /*LODEPNG_COMPILE_PNG*/

#ifdef LODEPNG_COMPILE_ANCILLARY_CHUNKS

static void LodePNGUnknownChunks_init(LodePNGInfo* info)
{
  unsigned i;
  for(i = 0; i != 3; ++i) info->unknown_chunks_data[i] = 0;
  for(i = 0; i != 3; ++i) info->unknown_chunks_size[i] = 0;
}

static void LodePNGUnknownChunks_cleanup(LodePNGInfo* info)
{
  unsigned i;
  for(i = 0; i != 3; ++i) lodepng_free(info->unknown_chunks_data[i]);
}

static unsigned LodePNGUnknownChunks_copy(LodePNGInfo* dest, const LodePNGInfo* src)
{
  unsigned i;

  LodePNGUnknownChunks_cleanup(dest);

  for(i = 0; i != 3; ++i)
  {
    size_t j;
    dest->unknown_chunks_size[i] = src->unknown_chunks_size[i];
    dest->unknown_chunks_data[i] = (unsigned char*)lodepng_malloc(src->unknown_chunks_size[i]);
    if(!dest->unknown_chunks_data[i] && dest->unknown_chunks_size[i]) return 83; /*alloc fail*/
    for(j = 0; j < src->unknown_chunks_size[i]; ++j)
    {
      dest->unknown_chunks_data[i][j] = src->unknown_chunks_data[i][j];
    }
  }

  return 0;
}

/******************************************************************************/

static void LodePNGText_init(LodePNGInfo* info)
{
  info->text_num = 0;
  info->text_keys = NULL;
  info->text_strings = NULL;
}

static void LodePNGText_cleanup(LodePNGInfo* info)
{
  size_t i;
  for(i = 0; i != info->text_num; ++i)
  {
    string_cleanup(&info->text_keys[i]);
    string_cleanup(&info->text_strings[i]);
  }
  lodepng_free(info->text_keys);
  lodepng_free(info->text_strings);
}

static unsigned LodePNGText_copy(LodePNGInfo* dest, const LodePNGInfo* source)
{
  size_t i = 0;
  dest->text_keys = 0;
  dest->text_strings = 0;
  dest->text_num = 0;
  for(i = 0; i != source->text_num; ++i)
  {
    CERROR_TRY_RETURN(lodepng_add_text(dest, source->text_keys[i], source->text_strings[i]));
  }
  return 0;
}

void lodepng_clear_text(LodePNGInfo* info)
{
  LodePNGText_cleanup(info);
}

unsigned lodepng_add_text(LodePNGInfo* info, const char* key, const char* str)
{
  char** new_keys = (char**)(lodepng_realloc(info->text_keys, sizeof(char*) * (info->text_num + 1)));
  char** new_strings = (char**)(lodepng_realloc(info->text_strings, sizeof(char*) * (info->text_num + 1)));
  if(!new_keys || !new_strings)
  {
    lodepng_free(new_keys);
    lodepng_free(new_strings);
    return 83; /*alloc fail*/
  }

  ++info->text_num;
  info->text_keys = new_keys;
  info->text_strings = new_strings;

  string_init(&info->text_keys[info->text_num - 1]);
  string_set(&info->text_keys[info->text_num - 1], key);

  string_init(&info->text_strings[info->text_num - 1]);
  string_set(&info->text_strings[info->text_num - 1], str);

  return 0;
}

/******************************************************************************/

static void LodePNGIText_init(LodePNGInfo* info)
{
  info->itext_num = 0;
  info->itext_keys = NULL;
  info->itext_langtags = NULL;
  info->itext_transkeys = NULL;
  info->itext_strings = NULL;
}

static void LodePNGIText_cleanup(LodePNGInfo* info)
{
  size_t i;
  for(i = 0; i != info->itext_num; ++i)
  {
    string_cleanup(&info->itext_keys[i]);
    string_cleanup(&info->itext_langtags[i]);
    string_cleanup(&info->itext_transkeys[i]);
    string_cleanup(&info->itext_strings[i]);
  }
  lodepng_free(info->itext_keys);
  lodepng_free(info->itext_langtags);
  lodepng_free(info->itext_transkeys);
  lodepng_free(info->itext_strings);
}

static unsigned LodePNGIText_copy(LodePNGInfo* dest, const LodePNGInfo* source)
{
  size_t i = 0;
  dest->itext_keys = 0;
  dest->itext_langtags = 0;
  dest->itext_transkeys = 0;
  dest->itext_strings = 0;
  dest->itext_num = 0;
  for(i = 0; i != source->itext_num; ++i)
  {
    CERROR_TRY_RETURN(lodepng_add_itext(dest, source->itext_keys[i], source->itext_langtags[i],
                                        source->itext_transkeys[i], source->itext_strings[i]));
  }
  return 0;
}

void lodepng_clear_itext(LodePNGInfo* info)
{
  LodePNGIText_cleanup(info);
}

unsigned lodepng_add_itext(LodePNGInfo* info, const char* key, const char* langtag,
                           const char* transkey, const char* str)
{
  char** new_keys = (char**)(lodepng_realloc(info->itext_keys, sizeof(char*) * (info->itext_num + 1)));
  char** new_langtags = (char**)(lodepng_realloc(info->itext_langtags, sizeof(char*) * (info->itext_num + 1)));
  char** new_transkeys = (char**)(lodepng_realloc(info->itext_transkeys, sizeof(char*) * (info->itext_num + 1)));
  char** new_strings = (char**)(lodepng_realloc(info->itext_strings, sizeof(char*) * (info->itext_num + 1)));
  if(!new_keys || !new_langtags || !new_transkeys || !new_strings)
  {
    lodepng_free(new_keys);
    lodepng_free(new_langtags);
    lodepng_free(new_transkeys);
    lodepng_free(new_strings);
    return 83; /*alloc fail*/
  }

  ++info->itext_num;
  info->itext_keys = new_keys;
  info->itext_langtags = new_langtags;
  info->itext_transkeys = new_transkeys;
  info->itext_strings = new_strings;

  string_init(&info->itext_keys[info->itext_num - 1]);
  string_set(&info->itext_keys[info->itext_num - 1], key);

  string_init(&info->itext_langtags[info->itext_num - 1]);
  string_set(&info->itext_langtags[info->itext_num - 1], langtag);

  string_init(&info->itext_transkeys[info->itext_num - 1]);
  string_set(&info->itext_transkeys[info->itext_num - 1], transkey);

  string_init(&info->itext_strings[info->itext_num - 1]);
  string_set(&info->itext_strings[info->itext_num - 1], str);

  return 0;
}
#endif /*LODEPNG_COMPILE_ANCILLARY_CHUNKS*/

void lodepng_info_init(LodePNGInfo* info)
{
  lodepng_color_mode_init(&info->color);
  info->interlace_method = 0;
  info->compression_method = 0;
  info->filter_method = 0;
#ifdef LODEPNG_COMPILE_ANCILLARY_CHUNKS
  info->background_defined = 0;
  info->background_r = info->background_g = info->background_b = 0;

  LodePNGText_init(info);
  LodePNGIText_init(info);

  info->time_defined = 0;
  info->phys_defined = 0;

  LodePNGUnknownChunks_init(info);
#endif /*LODEPNG_COMPILE_ANCILLARY_CHUNKS*/
}

void lodepng_info_cleanup(LodePNGInfo* info)
{
  lodepng_color_mode_cleanup(&info->color);
#ifdef LODEPNG_COMPILE_ANCILLARY_CHUNKS
  LodePNGText_cleanup(info);
  LodePNGIText_cleanup(info);

  LodePNGUnknownChunks_cleanup(info);
#endif /*LODEPNG_COMPILE_ANCILLARY_CHUNKS*/
}

unsigned lodepng_info_copy(LodePNGInfo* dest, const LodePNGInfo* source)
{
  lodepng_info_cleanup(dest);
  *dest = *source;
  lodepng_color_mode_init(&dest->color);
  CERROR_TRY_RETURN(lodepng_color_mode_copy(&dest->color, &source->color));

#ifdef LODEPNG_COMPILE_ANCILLARY_CHUNKS
  CERROR_TRY_RETURN(LodePNGText_copy(dest, source));
  CERROR_TRY_RETURN(LodePNGIText_copy(dest, source));

  LodePNGUnknownChunks_init(dest);
  CERROR_TRY_RETURN(LodePNGUnknownChunks_copy(dest, source));
#endif /*LODEPNG_COMPILE_ANCILLARY_CHUNKS*/
  return 0;
}

void lodepng_info_swap(LodePNGInfo* a, LodePNGInfo* b)
{
  LodePNGInfo temp = *a;
  *a = *b;
  *b = temp;
}

/* ////////////////////////////////////////////////////////////////////////// */

/*index: bitgroup index, bits: bitgroup size(1, 2 or 4), in: bitgroup value, out: octet array to add bits to*/
static void addColorBits(unsigned char* out, size_t index, unsigned bits, unsigned in)
{
  unsigned m = bits == 1 ? 7 : bits == 2 ? 3 : 1; /*8 / bits - 1*/
  /*p = the partial index in the byte, e.g. with 4 palettebits it is 0 for first half or 1 for second half*/
  unsigned p = index & m;
  in &= (1u << bits) - 1u; /*filter out any other bits of the input value*/
  in = in << (bits * (m - p));
  if(p == 0) out[index * bits / 8] = in;
  else out[index * bits / 8] |= in;
}

typedef struct ColorTree ColorTree;

/*
One node of a color tree
This is the data structure used to count the number of unique colors and to get a palette
index for a color. It's like an octree, but because the alpha channel is used too, each
node has 16 instead of 8 children.
*/
struct ColorTree
{
  ColorTree* children[16]; /*up to 16 pointers to ColorTree of next level*/
  int index; /*the payload. Only has a meaningful value if this is in the last level*/
};

static void color_tree_init(ColorTree* tree)
{
  int i;
  for(i = 0; i != 16; ++i) tree->children[i] = 0;
  tree->index = -1;
}

static void color_tree_cleanup(ColorTree* tree)
{
  int i;
  for(i = 0; i != 16; ++i)
  {
    if(tree->children[i])
    {
      color_tree_cleanup(tree->children[i]);
      lodepng_free(tree->children[i]);
    }
  }
}

/*returns -1 if color not present, its index otherwise*/
static int color_tree_get(ColorTree* tree, unsigned char r, unsigned char g, unsigned char b, unsigned char a)
{
  int bit = 0;
  for(bit = 0; bit < 8; ++bit)
  {
    int i = 8 * ((r >> bit) & 1) + 4 * ((g >> bit) & 1) + 2 * ((b >> bit) & 1) + 1 * ((a >> bit) & 1);
    if(!tree->children[i]) return -1;
    else tree = tree->children[i];
  }
  return tree ? tree->index : -1;
}

#ifdef LODEPNG_COMPILE_ENCODER
static int color_tree_has(ColorTree* tree, unsigned char r, unsigned char g, unsigned char b, unsigned char a)
{
  return color_tree_get(tree, r, g, b, a) >= 0;
}
#endif /*LODEPNG_COMPILE_ENCODER*/

/*color is not allowed to already exist.
Index should be >= 0 (it's signed to be compatible with using -1 for "doesn't exist")*/
static void color_tree_add(ColorTree* tree,
                           unsigned char r, unsigned char g, unsigned char b, unsigned char a, unsigned index)
{
  int bit;
  for(bit = 0; bit < 8; ++bit)
  {
    int i = 8 * ((r >> bit) & 1) + 4 * ((g >> bit) & 1) + 2 * ((b >> bit) & 1) + 1 * ((a >> bit) & 1);
    if(!tree->children[i])
    {
      tree->children[i] = (ColorTree*)lodepng_malloc(sizeof(ColorTree));
      color_tree_init(tree->children[i]);
    }
    tree = tree->children[i];
  }
  tree->index = (int)index;
}

/*put a pixel, given its RGBA color, into image of any color type*/
static unsigned rgba8ToPixel(unsigned char* out, size_t i,
                             const LodePNGColorMode* mode, ColorTree* tree /*for palette*/,
                             unsigned char r, unsigned char g, unsigned char b, unsigned char a)
{
  if(mode->colortype == LCT_GREY)
  {
    unsigned char grey = r; /*((unsigned short)r + g + b) / 3*/;
    if(mode->bitdepth == 8) out[i] = grey;
    else if(mode->bitdepth == 16) out[i * 2 + 0] = out[i * 2 + 1] = grey;
    else
    {
      /*take the most significant bits of grey*/
      grey = (grey >> (8 - mode->bitdepth)) & ((1 << mode->bitdepth) - 1);
      addColorBits(out, i, mode->bitdepth, grey);
    }
  }
  else if(mode->colortype == LCT_RGB)
  {
    if(mode->bitdepth == 8)
    {
      out[i * 3 + 0] = r;
      out[i * 3 + 1] = g;
      out[i * 3 + 2] = b;
    }
    else
    {
      out[i * 6 + 0] = out[i * 6 + 1] = r;
      out[i * 6 + 2] = out[i * 6 + 3] = g;
      out[i * 6 + 4] = out[i * 6 + 5] = b;
    }
  }
  else if(mode->colortype == LCT_PALETTE)
  {
    int index = color_tree_get(tree, r, g, b, a);
    if(index < 0) return 82; /*color not in palette*/
    if(mode->bitdepth == 8) out[i] = index;
    else addColorBits(out, i, mode->bitdepth, (unsigned)index);
  }
  else if(mode->colortype == LCT_GREY_ALPHA)
  {
    unsigned char grey = r; /*((unsigned short)r + g + b) / 3*/;
    if(mode->bitdepth == 8)
    {
      out[i * 2 + 0] = grey;
      out[i * 2 + 1] = a;
    }
    else if(mode->bitdepth == 16)
    {
      out[i * 4 + 0] = out[i * 4 + 1] = grey;
      out[i * 4 + 2] = out[i * 4 + 3] = a;
    }
  }
  else if(mode->colortype == LCT_RGBA)
  {
    if(mode->bitdepth == 8)
    {
      out[i * 4 + 0] = r;
      out[i * 4 + 1] = g;
      out[i * 4 + 2] = b;
      out[i * 4 + 3] = a;
    }
    else
    {
      out[i * 8 + 0] = out[i * 8 + 1] = r;
      out[i * 8 + 2] = out[i * 8 + 3] = g;
      out[i * 8 + 4] = out[i * 8 + 5] = b;
      out[i * 8 + 6] = out[i * 8 + 7] = a;
    }
  }

  return 0; /*no error*/
}

/*put a pixel, given its RGBA16 color, into image of any color 16-bitdepth type*/
static void rgba16ToPixel(unsigned char* out, size_t i,
                         const LodePNGColorMode* mode,
                         unsigned short r, unsigned short g, unsigned short b, unsigned short a)
{
  if(mode->colortype == LCT_GREY)
  {
    unsigned short grey = r; /*((unsigned)r + g + b) / 3*/;
    out[i * 2 + 0] = (grey >> 8) & 255;
    out[i * 2 + 1] = grey & 255;
  }
  else if(mode->colortype == LCT_RGB)
  {
    out[i * 6 + 0] = (r >> 8) & 255;
    out[i * 6 + 1] = r & 255;
    out[i * 6 + 2] = (g >> 8) & 255;
    out[i * 6 + 3] = g & 255;
    out[i * 6 + 4] = (b >> 8) & 255;
    out[i * 6 + 5] = b & 255;
  }
  else if(mode->colortype == LCT_GREY_ALPHA)
  {
    unsigned short grey = r; /*((unsigned)r + g + b) / 3*/;
    out[i * 4 + 0] = (grey >> 8) & 255;
    out[i * 4 + 1] = grey & 255;
    out[i * 4 + 2] = (a >> 8) & 255;
    out[i * 4 + 3] = a & 255;
  }
  else if(mode->colortype == LCT_RGBA)
  {
    out[i * 8 + 0] = (r >> 8) & 255;
    out[i * 8 + 1] = r & 255;
    out[i * 8 + 2] = (g >> 8) & 255;
    out[i * 8 + 3] = g & 255;
    out[i * 8 + 4] = (b >> 8) & 255;
    out[i * 8 + 5] = b & 255;
    out[i * 8 + 6] = (a >> 8) & 255;
    out[i * 8 + 7] = a & 255;
  }
}

/*Get RGBA8 color of pixel with index i (y * width + x) from the raw image with given color type.*/
static void getPixelColorRGBA8(unsigned char* r, unsigned char* g,
                               unsigned char* b, unsigned char* a,
                               const unsigned char* in, size_t i,
                               const LodePNGColorMode* mode)
{
  if(mode->colortype == LCT_GREY)
  {
    if(mode->bitdepth == 8)
    {
      *r = *g = *b = in[i];
      if(mode->key_defined && *r == mode->key_r) *a = 0;
      else *a = 255;
    }
    else if(mode->bitdepth == 16)
    {
      *r = *g = *b = in[i * 2 + 0];
      if(mode->key_defined && 256U * in[i * 2 + 0] + in[i * 2 + 1] == mode->key_r) *a = 0;
      else *a = 255;
    }
    else
    {
      unsigned highest = ((1U << mode->bitdepth) - 1U); /*highest possible value for this bit depth*/
      size_t j = i * mode->bitdepth;
      unsigned value = readBitsFromReversedStream(&j, in, mode->bitdepth);
      *r = *g = *b = (value * 255) / highest;
      if(mode->key_defined && value == mode->key_r) *a = 0;
      else *a = 255;
    }
  }
  else if(mode->colortype == LCT_RGB)
  {
    if(mode->bitdepth == 8)
    {
      *r = in[i * 3 + 0]; *g = in[i * 3 + 1]; *b = in[i * 3 + 2];
      if(mode->key_defined && *r == mode->key_r && *g == mode->key_g && *b == mode->key_b) *a = 0;
      else *a = 255;
    }
    else
    {
      *r = in[i * 6 + 0];
      *g = in[i * 6 + 2];
      *b = in[i * 6 + 4];
      if(mode->key_defined && 256U * in[i * 6 + 0] + in[i * 6 + 1] == mode->key_r
         && 256U * in[i * 6 + 2] + in[i * 6 + 3] == mode->key_g
         && 256U * in[i * 6 + 4] + in[i * 6 + 5] == mode->key_b) *a = 0;
      else *a = 255;
    }
  }
  else if(mode->colortype == LCT_PALETTE)
  {
    unsigned index;
    if(mode->bitdepth == 8) index = in[i];
    else
    {
      size_t j = i * mode->bitdepth;
      index = readBitsFromReversedStream(&j, in, mode->bitdepth);
    }

    if(index >= mode->palettesize)
    {
      /*This is an error according to the PNG spec, but common PNG decoders make it black instead.
      Done here too, slightly faster due to no error handling needed.*/
      *r = *g = *b = 0;
      *a = 255;
    }
    else
    {
      *r = mode->palette[index * 4 + 0];
      *g = mode->palette[index * 4 + 1];
      *b = mode->palette[index * 4 + 2];
      *a = mode->palette[index * 4 + 3];
    }
  }
  else if(mode->colortype == LCT_GREY_ALPHA)
  {
    if(mode->bitdepth == 8)
    {
      *r = *g = *b = in[i * 2 + 0];
      *a = in[i * 2 + 1];
    }
    else
    {
      *r = *g = *b = in[i * 4 + 0];
      *a = in[i * 4 + 2];
    }
  }
  else if(mode->colortype == LCT_RGBA)
  {
    if(mode->bitdepth == 8)
    {
      *r = in[i * 4 + 0];
      *g = in[i * 4 + 1];
      *b = in[i * 4 + 2];
      *a = in[i * 4 + 3];
    }
    else
    {
      *r = in[i * 8 + 0];
      *g = in[i * 8 + 2];
      *b = in[i * 8 + 4];
      *a = in[i * 8 + 6];
    }
  }
}

/*Similar to getPixelColorRGBA8, but with all the for loops inside of the color
mode test cases, optimized to convert the colors much faster, when converting
to RGBA or RGB with 8 bit per cannel. buffer must be RGBA or RGB output with
enough memory, if has_alpha is true the output is RGBA. mode has the color mode
of the input buffer.*/
static void getPixelColorsRGBA8(unsigned char* buffer, size_t numpixels,
                                unsigned has_alpha, const unsigned char* in,
                                const LodePNGColorMode* mode)
{
  unsigned num_channels = has_alpha ? 4 : 3;
  size_t i;
  if(mode->colortype == LCT_GREY)
  {
    if(mode->bitdepth == 8)
    {
      for(i = 0; i != numpixels; ++i, buffer += num_channels)
      {
        buffer[0] = buffer[1] = buffer[2] = in[i];
        if(has_alpha) buffer[3] = mode->key_defined && in[i] == mode->key_r ? 0 : 255;
      }
    }
    else if(mode->bitdepth == 16)
    {
      for(i = 0; i != numpixels; ++i, buffer += num_channels)
      {
        buffer[0] = buffer[1] = buffer[2] = in[i * 2];
        if(has_alpha) buffer[3] = mode->key_defined && 256U * in[i * 2 + 0] + in[i * 2 + 1] == mode->key_r ? 0 : 255;
      }
    }
    else
    {
      unsigned highest = ((1U << mode->bitdepth) - 1U); /*highest possible value for this bit depth*/
      size_t j = 0;
      for(i = 0; i != numpixels; ++i, buffer += num_channels)
      {
        unsigned value = readBitsFromReversedStream(&j, in, mode->bitdepth);
        buffer[0] = buffer[1] = buffer[2] = (value * 255) / highest;
        if(has_alpha) buffer[3] = mode->key_defined && value == mode->key_r ? 0 : 255;
      }
    }
  }
  else if(mode->colortype == LCT_RGB)
  {
    if(mode->bitdepth == 8)
    {
      for(i = 0; i != numpixels; ++i, buffer += num_channels)
      {
        buffer[0] = in[i * 3 + 0];
        buffer[1] = in[i * 3 + 1];
        buffer[2] = in[i * 3 + 2];
        if(has_alpha) buffer[3] = mode->key_defined && buffer[0] == mode->key_r
           && buffer[1]== mode->key_g && buffer[2] == mode->key_b ? 0 : 255;
      }
    }
    else
    {
      for(i = 0; i != numpixels; ++i, buffer += num_channels)
      {
        buffer[0] = in[i * 6 + 0];
        buffer[1] = in[i * 6 + 2];
        buffer[2] = in[i * 6 + 4];
        if(has_alpha) buffer[3] = mode->key_defined
           && 256U * in[i * 6 + 0] + in[i * 6 + 1] == mode->key_r
           && 256U * in[i * 6 + 2] + in[i * 6 + 3] == mode->key_g
           && 256U * in[i * 6 + 4] + in[i * 6 + 5] == mode->key_b ? 0 : 255;
      }
    }
  }
  else if(mode->colortype == LCT_PALETTE)
  {
    unsigned index;
    size_t j = 0;
    for(i = 0; i != numpixels; ++i, buffer += num_channels)
    {
      if(mode->bitdepth == 8) index = in[i];
      else index = readBitsFromReversedStream(&j, in, mode->bitdepth);

      if(index >= mode->palettesize)
      {
        /*This is an error according to the PNG spec, but most PNG decoders make it black instead.
        Done here too, slightly faster due to no error handling needed.*/
        buffer[0] = buffer[1] = buffer[2] = 0;
        if(has_alpha) buffer[3] = 255;
      }
      else
      {
        buffer[0] = mode->palette[index * 4 + 0];
        buffer[1] = mode->palette[index * 4 + 1];
        buffer[2] = mode->palette[index * 4 + 2];
        if(has_alpha) buffer[3] = mode->palette[index * 4 + 3];
      }
    }
  }
  else if(mode->colortype == LCT_GREY_ALPHA)
  {
    if(mode->bitdepth == 8)
    {
      for(i = 0; i != numpixels; ++i, buffer += num_channels)
      {
        buffer[0] = buffer[1] = buffer[2] = in[i * 2 + 0];
        if(has_alpha) buffer[3] = in[i * 2 + 1];
      }
    }
    else
    {
      for(i = 0; i != numpixels; ++i, buffer += num_channels)
      {
        buffer[0] = buffer[1] = buffer[2] = in[i * 4 + 0];
        if(has_alpha) buffer[3] = in[i * 4 + 2];
      }
    }
  }
  else if(mode->colortype == LCT_RGBA)
  {
    if(mode->bitdepth == 8)
    {
      for(i = 0; i != numpixels; ++i, buffer += num_channels)
      {
        buffer[0] = in[i * 4 + 0];
        buffer[1] = in[i * 4 + 1];
        buffer[2] = in[i * 4 + 2];
        if(has_alpha) buffer[3] = in[i * 4 + 3];
      }
    }
    else
    {
      for(i = 0; i != numpixels; ++i, buffer += num_channels)
      {
        buffer[0] = in[i * 8 + 0];
        buffer[1] = in[i * 8 + 2];
        buffer[2] = in[i * 8 + 4];
        if(has_alpha) buffer[3] = in[i * 8 + 6];
      }
    }
  }
}

/*Get RGBA16 color of pixel with index i (y * width + x) from the raw image with
given color type, but the given color type must be 16-bit itself.*/
static void getPixelColorRGBA16(unsigned short* r, unsigned short* g, unsigned short* b, unsigned short* a,
                                const unsigned char* in, size_t i, const LodePNGColorMode* mode)
{
  if(mode->colortype == LCT_GREY)
  {
    *r = *g = *b = 256 * in[i * 2 + 0] + in[i * 2 + 1];
    if(mode->key_defined && 256U * in[i * 2 + 0] + in[i * 2 + 1] == mode->key_r) *a = 0;
    else *a = 65535;
  }
  else if(mode->colortype == LCT_RGB)
  {
    *r = 256u * in[i * 6 + 0] + in[i * 6 + 1];
    *g = 256u * in[i * 6 + 2] + in[i * 6 + 3];
    *b = 256u * in[i * 6 + 4] + in[i * 6 + 5];
    if(mode->key_defined
       && 256u * in[i * 6 + 0] + in[i * 6 + 1] == mode->key_r
       && 256u * in[i * 6 + 2] + in[i * 6 + 3] == mode->key_g
       && 256u * in[i * 6 + 4] + in[i * 6 + 5] == mode->key_b) *a = 0;
    else *a = 65535;
  }
  else if(mode->colortype == LCT_GREY_ALPHA)
  {
    *r = *g = *b = 256u * in[i * 4 + 0] + in[i * 4 + 1];
    *a = 256u * in[i * 4 + 2] + in[i * 4 + 3];
  }
  else if(mode->colortype == LCT_RGBA)
  {
    *r = 256u * in[i * 8 + 0] + in[i * 8 + 1];
    *g = 256u * in[i * 8 + 2] + in[i * 8 + 3];
    *b = 256u * in[i * 8 + 4] + in[i * 8 + 5];
    *a = 256u * in[i * 8 + 6] + in[i * 8 + 7];
  }
}

unsigned lodepng_convert(unsigned char* out, const unsigned char* in,
                         const LodePNGColorMode* mode_out, const LodePNGColorMode* mode_in,
                         unsigned w, unsigned h)
{
  size_t i;
  ColorTree tree;
  size_t numpixels = w * h;

  if(lodepng_color_mode_equal(mode_out, mode_in))
  {
    size_t numbytes = lodepng_get_raw_size(w, h, mode_in);
    for(i = 0; i != numbytes; ++i) out[i] = in[i];
    return 0;
  }

  if(mode_out->colortype == LCT_PALETTE)
  {
    size_t palettesize = mode_out->palettesize;
    const unsigned char* palette = mode_out->palette;
    size_t palsize = 1u << mode_out->bitdepth;
    /*if the user specified output palette but did not give the values, assume
    they want the values of the input color type (assuming that one is palette).
    Note that we never create a new palette ourselves.*/
    if(palettesize == 0)
    {
      palettesize = mode_in->palettesize;
      palette = mode_in->palette;
    }
    if(palettesize < palsize) palsize = palettesize;
    color_tree_init(&tree);
    for(i = 0; i != palsize; ++i)
    {
      const unsigned char* p = &palette[i * 4];
      color_tree_add(&tree, p[0], p[1], p[2], p[3], i);
    }
  }

  if(mode_in->bitdepth == 16 && mode_out->bitdepth == 16)
  {
    for(i = 0; i != numpixels; ++i)
    {
      unsigned short r = 0, g = 0, b = 0, a = 0;
      getPixelColorRGBA16(&r, &g, &b, &a, in, i, mode_in);
      rgba16ToPixel(out, i, mode_out, r, g, b, a);
    }
  }
  else if(mode_out->bitdepth == 8 && mode_out->colortype == LCT_RGBA)
  {
    getPixelColorsRGBA8(out, numpixels, 1, in, mode_in);
  }
  else if(mode_out->bitdepth == 8 && mode_out->colortype == LCT_RGB)
  {
    getPixelColorsRGBA8(out, numpixels, 0, in, mode_in);
  }
  else
  {
    unsigned char r = 0, g = 0, b = 0, a = 0;
    for(i = 0; i != numpixels; ++i)
    {
      getPixelColorRGBA8(&r, &g, &b, &a, in, i, mode_in);
      CERROR_TRY_RETURN(rgba8ToPixel(out, i, mode_out, &tree, r, g, b, a));
    }
  }

  if(mode_out->colortype == LCT_PALETTE)
  {
    color_tree_cleanup(&tree);
  }

  return 0; /*no error*/
}

#ifdef LODEPNG_COMPILE_ENCODER

void lodepng_color_profile_init(LodePNGColorProfile* profile)
{
  profile->colored = 0;
  profile->key = 0;
  profile->key_r = profile->key_g = profile->key_b = 0;
  profile->alpha = 0;
  profile->numcolors = 0;
  profile->bits = 1;
}

/*function used for debug purposes with C++*/
/*void printColorProfile(LodePNGColorProfile* p)
{
  std::cout << "colored: " << (int)p->colored << ", ";
  std::cout << "key: " << (int)p->key << ", ";
  std::cout << "key_r: " << (int)p->key_r << ", ";
  std::cout << "key_g: " << (int)p->key_g << ", ";
  std::cout << "key_b: " << (int)p->key_b << ", ";
  std::cout << "alpha: " << (int)p->alpha << ", ";
  std::cout << "numcolors: " << (int)p->numcolors << ", ";
  std::cout << "bits: " << (int)p->bits << std::endl;
}*/

/*Returns how many bits needed to represent given value (max 8 bit)*/
static unsigned getValueRequiredBits(unsigned char value)
{
  if(value == 0 || value == 255) return 1;
  /*The scaling of 2-bit and 4-bit values uses multiples of 85 and 17*/
  if(value % 17 == 0) return value % 85 == 0 ? 2 : 4;
  return 8;
}

/*profile must already have been inited with mode.
It's ok to set some parameters of profile to done already.*/
unsigned lodepng_get_color_profile(LodePNGColorProfile* profile,
                                   const unsigned char* in, unsigned w, unsigned h,
                                   const LodePNGColorMode* mode)
{
  unsigned error = 0;
  size_t i;
  ColorTree tree;
  size_t numpixels = w * h;

  unsigned colored_done = lodepng_is_greyscale_type(mode) ? 1 : 0;
  unsigned alpha_done = lodepng_can_have_alpha(mode) ? 0 : 1;
  unsigned numcolors_done = 0;
  unsigned bpp = lodepng_get_bpp(mode);
  unsigned bits_done = bpp == 1 ? 1 : 0;
  unsigned maxnumcolors = 257;
  unsigned sixteen = 0;
  if(bpp <= 8) maxnumcolors = bpp == 1 ? 2 : (bpp == 2 ? 4 : (bpp == 4 ? 16 : 256));

  color_tree_init(&tree);

  /*Check if the 16-bit input is truly 16-bit*/
  if(mode->bitdepth == 16)
  {
    unsigned short r, g, b, a;
    for(i = 0; i != numpixels; ++i)
    {
      getPixelColorRGBA16(&r, &g, &b, &a, in, i, mode);
      if((r & 255) != ((r >> 8) & 255) || (g & 255) != ((g >> 8) & 255) ||
         (b & 255) != ((b >> 8) & 255) || (a & 255) != ((a >> 8) & 255)) /*first and second byte differ*/
      {
        sixteen = 1;
        break;
      }
    }
  }

  if(sixteen)
  {
    unsigned short r = 0, g = 0, b = 0, a = 0;
    profile->bits = 16;
    bits_done = numcolors_done = 1; /*counting colors no longer useful, palette doesn't support 16-bit*/

    for(i = 0; i != numpixels; ++i)
    {
      getPixelColorRGBA16(&r, &g, &b, &a, in, i, mode);

      if(!colored_done && (r != g || r != b))
      {
        profile->colored = 1;
        colored_done = 1;
      }

      if(!alpha_done)
      {
        unsigned matchkey = (r == profile->key_r && g == profile->key_g && b == profile->key_b);
        if(a != 65535 && (a != 0 || (profile->key && !matchkey)))
        {
          profile->alpha = 1;
          profile->key = 0;
          alpha_done = 1;
        }
        else if(a == 0 && !profile->alpha && !profile->key)
        {
          profile->key = 1;
          profile->key_r = r;
          profile->key_g = g;
          profile->key_b = b;
        }
        else if(a == 65535 && profile->key && matchkey)
        {
          /* Color key cannot be used if an opaque pixel also has that RGB color. */
          profile->alpha = 1;
          profile->key = 0;
          alpha_done = 1;
        }
      }
      if(alpha_done && numcolors_done && colored_done && bits_done) break;
    }

    if(profile->key && !profile->alpha)
    {
      for(i = 0; i != numpixels; ++i)
      {
        getPixelColorRGBA16(&r, &g, &b, &a, in, i, mode);
        if(a != 0 && r == profile->key_r && g == profile->key_g && b == profile->key_b)
        {
          /* Color key cannot be used if an opaque pixel also has that RGB color. */
          profile->alpha = 1;
          profile->key = 0;
          alpha_done = 1;
        }
      }
    }
  }
  else /* < 16-bit */
  {
    unsigned char r = 0, g = 0, b = 0, a = 0;
    for(i = 0; i != numpixels; ++i)
    {
      getPixelColorRGBA8(&r, &g, &b, &a, in, i, mode);

      if(!bits_done && profile->bits < 8)
      {
        /*only r is checked, < 8 bits is only relevant for greyscale*/
        unsigned bits = getValueRequiredBits(r);
        if(bits > profile->bits) profile->bits = bits;
      }
      bits_done = (profile->bits >= bpp);

      if(!colored_done && (r != g || r != b))
      {
        profile->colored = 1;
        colored_done = 1;
        if(profile->bits < 8) profile->bits = 8; /*PNG has no colored modes with less than 8-bit per channel*/
      }

      if(!alpha_done)
      {
        unsigned matchkey = (r == profile->key_r && g == profile->key_g && b == profile->key_b);
        if(a != 255 && (a != 0 || (profile->key && !matchkey)))
        {
          profile->alpha = 1;
          profile->key = 0;
          alpha_done = 1;
          if(profile->bits < 8) profile->bits = 8; /*PNG has no alphachannel modes with less than 8-bit per channel*/
        }
        else if(a == 0 && !profile->alpha && !profile->key)
        {
          profile->key = 1;
          profile->key_r = r;
          profile->key_g = g;
          profile->key_b = b;
        }
        else if(a == 255 && profile->key && matchkey)
        {
          /* Color key cannot be used if an opaque pixel also has that RGB color. */
          profile->alpha = 1;
          profile->key = 0;
          alpha_done = 1;
          if(profile->bits < 8) profile->bits = 8; /*PNG has no alphachannel modes with less than 8-bit per channel*/
        }
      }

      if(!numcolors_done)
      {
        if(!color_tree_has(&tree, r, g, b, a))
        {
          color_tree_add(&tree, r, g, b, a, profile->numcolors);
          if(profile->numcolors < 256)
          {
            unsigned char* p = profile->palette;
            unsigned n = profile->numcolors;
            p[n * 4 + 0] = r;
            p[n * 4 + 1] = g;
            p[n * 4 + 2] = b;
            p[n * 4 + 3] = a;
          }
          ++profile->numcolors;
          numcolors_done = profile->numcolors >= maxnumcolors;
        }
      }

      if(alpha_done && numcolors_done && colored_done && bits_done) break;
    }

    if(profile->key && !profile->alpha)
    {
      for(i = 0; i != numpixels; ++i)
      {
        getPixelColorRGBA8(&r, &g, &b, &a, in, i, mode);
        if(a != 0 && r == profile->key_r && g == profile->key_g && b == profile->key_b)
        {
          /* Color key cannot be used if an opaque pixel also has that RGB color. */
          profile->alpha = 1;
          profile->key = 0;
          alpha_done = 1;
          if(profile->bits < 8) profile->bits = 8; /*PNG has no alphachannel modes with less than 8-bit per channel*/
        }
      }
    }

    /*make the profile's key always 16-bit for consistency - repeat each byte twice*/
    profile->key_r += (profile->key_r << 8);
    profile->key_g += (profile->key_g << 8);
    profile->key_b += (profile->key_b << 8);
  }

  color_tree_cleanup(&tree);
  return error;
}

/*Automatically chooses color type that gives smallest amount of bits in the
output image, e.g. grey if there are only greyscale pixels, palette if there
are less than 256 colors, ...
Updates values of mode with a potentially smaller color model. mode_out should
contain the user chosen color model, but will be overwritten with the new chosen one.*/
unsigned lodepng_auto_choose_color(LodePNGColorMode* mode_out,
                                   const unsigned char* image, unsigned w, unsigned h,
                                   const LodePNGColorMode* mode_in)
{
  LodePNGColorProfile prof;
  unsigned error = 0;
  unsigned i, n, palettebits, palette_ok;

  lodepng_color_profile_init(&prof);
  error = lodepng_get_color_profile(&prof, image, w, h, mode_in);
  if(error) return error;
  mode_out->key_defined = 0;

  if(prof.key && w * h <= 16)
  {
    prof.alpha = 1; /*too few pixels to justify tRNS chunk overhead*/
    prof.key = 0;
    if(prof.bits < 8) prof.bits = 8; /*PNG has no alphachannel modes with less than 8-bit per channel*/
  }
  n = prof.numcolors;
  palettebits = n <= 2 ? 1 : (n <= 4 ? 2 : (n <= 16 ? 4 : 8));
  palette_ok = n <= 256 && prof.bits <= 8;
  if(w * h < n * 2) palette_ok = 0; /*don't add palette overhead if image has only a few pixels*/
  if(!prof.colored && prof.bits <= palettebits) palette_ok = 0; /*grey is less overhead*/

  if(palette_ok)
  {
    unsigned char* p = prof.palette;
    lodepng_palette_clear(mode_out); /*remove potential earlier palette*/
    for(i = 0; i != prof.numcolors; ++i)
    {
      error = lodepng_palette_add(mode_out, p[i * 4 + 0], p[i * 4 + 1], p[i * 4 + 2], p[i * 4 + 3]);
      if(error) break;
    }

    mode_out->colortype = LCT_PALETTE;
    mode_out->bitdepth = palettebits;

    if(mode_in->colortype == LCT_PALETTE && mode_in->palettesize >= mode_out->palettesize
        && mode_in->bitdepth == mode_out->bitdepth)
    {
      /*If input should have same palette colors, keep original to preserve its order and prevent conversion*/
      lodepng_color_mode_cleanup(mode_out);
      lodepng_color_mode_copy(mode_out, mode_in);
    }
  }
  else /*8-bit or 16-bit per channel*/
  {
    mode_out->bitdepth = prof.bits;
    mode_out->colortype = prof.alpha ? (prof.colored ? LCT_RGBA : LCT_GREY_ALPHA)
                                     : (prof.colored ? LCT_RGB : LCT_GREY);

    if(prof.key)
    {
      unsigned mask = (1u << mode_out->bitdepth) - 1u; /*profile always uses 16-bit, mask converts it*/
      mode_out->key_r = prof.key_r & mask;
      mode_out->key_g = prof.key_g & mask;
      mode_out->key_b = prof.key_b & mask;
      mode_out->key_defined = 1;
    }
  }

  return error;
}

#endif /* #ifdef LODEPNG_COMPILE_ENCODER */

/*
Paeth predicter, used by PNG filter type 4
The parameters are of type short, but should come from unsigned chars, the shorts
are only needed to make the paeth calculation correct.
*/
static unsigned char paethPredictor(short a, short b, short c)
{
  short pa = abs(b - c);
  short pb = abs(a - c);
  short pc = abs(a + b - c - c);

  if(pc < pa && pc < pb) return (unsigned char)c;
  else if(pb < pa) return (unsigned char)b;
  else return (unsigned char)a;
}

/*shared values used by multiple Adam7 related functions*/

static const unsigned ADAM7_IX[7] = { 0, 4, 0, 2, 0, 1, 0 }; /*x start values*/
static const unsigned ADAM7_IY[7] = { 0, 0, 4, 0, 2, 0, 1 }; /*y start values*/
static const unsigned ADAM7_DX[7] = { 8, 8, 4, 4, 2, 2, 1 }; /*x delta values*/
static const unsigned ADAM7_DY[7] = { 8, 8, 8, 4, 4, 2, 2 }; /*y delta values*/

/*
Outputs various dimensions and positions in the image related to the Adam7 reduced images.
passw: output containing the width of the 7 passes
passh: output containing the height of the 7 passes
filter_passstart: output containing the index of the start and end of each
 reduced image with filter bytes
padded_passstart output containing the index of the start and end of each
 reduced image when without filter bytes but with padded scanlines
passstart: output containing the index of the start and end of each reduced
 image without padding between scanlines, but still padding between the images
w, h: width and height of non-interlaced image
bpp: bits per pixel
"padded" is only relevant if bpp is less than 8 and a scanline or image does not
 end at a full byte
*/
static void Adam7_getpassvalues(unsigned passw[7], unsigned passh[7], size_t filter_passstart[8],
                                size_t padded_passstart[8], size_t passstart[8], unsigned w, unsigned h, unsigned bpp)
{
  /*the passstart values have 8 values: the 8th one indicates the byte after the end of the 7th (= last) pass*/
  unsigned i;

  /*calculate width and height in pixels of each pass*/
  for(i = 0; i != 7; ++i)
  {
    passw[i] = (w + ADAM7_DX[i] - ADAM7_IX[i] - 1) / ADAM7_DX[i];
    passh[i] = (h + ADAM7_DY[i] - ADAM7_IY[i] - 1) / ADAM7_DY[i];
    if(passw[i] == 0) passh[i] = 0;
    if(passh[i] == 0) passw[i] = 0;
  }

  filter_passstart[0] = padded_passstart[0] = passstart[0] = 0;
  for(i = 0; i != 7; ++i)
  {
    /*if passw[i] is 0, it's 0 bytes, not 1 (no filtertype-byte)*/
    filter_passstart[i + 1] = filter_passstart[i]
                            + ((passw[i] && passh[i]) ? passh[i] * (1 + (passw[i] * bpp + 7) / 8) : 0);
    /*bits padded if needed to fill full byte at end of each scanline*/
    padded_passstart[i + 1] = padded_passstart[i] + passh[i] * ((passw[i] * bpp + 7) / 8);
    /*only padded at end of reduced image*/
    passstart[i + 1] = passstart[i] + (passh[i] * passw[i] * bpp + 7) / 8;
  }
}

#ifdef LODEPNG_COMPILE_DECODER

/* ////////////////////////////////////////////////////////////////////////// */
/* / PNG Decoder                                                            / */
/* ////////////////////////////////////////////////////////////////////////// */

/*read the information from the header and store it in the LodePNGInfo. return value is error*/
unsigned lodepng_inspect(unsigned* w, unsigned* h, LodePNGState* state,
                         const unsigned char* in, size_t insize)
{
  LodePNGInfo* info = &state->info_png;
  if(insize == 0 || in == 0)
  {
    CERROR_RETURN_ERROR(state->error, 48); /*error: the given data is empty*/
  }
  if(insize < 33)
  {
    CERROR_RETURN_ERROR(state->error, 27); /*error: the data length is smaller than the length of a PNG header*/
  }

  /*when decoding a new PNG image, make sure all parameters created after previous decoding are reset*/
  lodepng_info_cleanup(info);
  lodepng_info_init(info);

  if(in[0] != 137 || in[1] != 80 || in[2] != 78 || in[3] != 71
     || in[4] != 13 || in[5] != 10 || in[6] != 26 || in[7] != 10)
  {
    CERROR_RETURN_ERROR(state->error, 28); /*error: the first 8 bytes are not the correct PNG signature*/
  }
  if(lodepng_chunk_length(in + 8) != 13)
  {
    CERROR_RETURN_ERROR(state->error, 94); /*error: header size must be 13 bytes*/
  }
  if(!lodepng_chunk_type_equals(in + 8, "IHDR"))
  {
    CERROR_RETURN_ERROR(state->error, 29); /*error: it doesn't start with a IHDR chunk!*/
  }

  /*read the values given in the header*/
  *w = lodepng_read32bitInt(&in[16]);
  *h = lodepng_read32bitInt(&in[20]);
  info->color.bitdepth = in[24];
  info->color.colortype = (LodePNGColorType)in[25];
  info->compression_method = in[26];
  info->filter_method = in[27];
  info->interlace_method = in[28];

  if(*w == 0 || *h == 0)
  {
    CERROR_RETURN_ERROR(state->error, 93);
  }

  if(!state->decoder.ignore_crc)
  {
    unsigned CRC = lodepng_read32bitInt(&in[29]);
    unsigned checksum = lodepng_crc32(&in[12], 17);
    if(CRC != checksum)
    {
      CERROR_RETURN_ERROR(state->error, 57); /*invalid CRC*/
    }
  }

  /*error: only compression method 0 is allowed in the specification*/
  if(info->compression_method != 0) CERROR_RETURN_ERROR(state->error, 32);
  /*error: only filter method 0 is allowed in the specification*/
  if(info->filter_method != 0) CERROR_RETURN_ERROR(state->error, 33);
  /*error: only interlace methods 0 and 1 exist in the specification*/
  if(info->interlace_method > 1) CERROR_RETURN_ERROR(state->error, 34);

  state->error = checkColorValidity(info->color.colortype, info->color.bitdepth);
  return state->error;
}

static unsigned unfilterScanline(unsigned char* recon, const unsigned char* scanline, const unsigned char* precon,
                                 size_t bytewidth, unsigned char filterType, size_t length)
{
  /*
  For PNG filter method 0
  unfilter a PNG image scanline by scanline. when the pixels are smaller than 1 byte,
  the filter works byte per byte (bytewidth = 1)
  precon is the previous unfiltered scanline, recon the result, scanline the current one
  the incoming scanlines do NOT include the filtertype byte, that one is given in the parameter filterType instead
  recon and scanline MAY be the same memory address! precon must be disjoint.
  */

  size_t i;
  switch(filterType)
  {
    case 0:
      for(i = 0; i != length; ++i) recon[i] = scanline[i];
      break;
    case 1:
      for(i = 0; i != bytewidth; ++i) recon[i] = scanline[i];
      for(i = bytewidth; i < length; ++i) recon[i] = scanline[i] + recon[i - bytewidth];
      break;
    case 2:
      if(precon)
      {
        for(i = 0; i != length; ++i) recon[i] = scanline[i] + precon[i];
      }
      else
      {
        for(i = 0; i != length; ++i) recon[i] = scanline[i];
      }
      break;
    case 3:
      if(precon)
      {
        for(i = 0; i != bytewidth; ++i) recon[i] = scanline[i] + (precon[i] >> 1);
        for(i = bytewidth; i < length; ++i) recon[i] = scanline[i] + ((recon[i - bytewidth] + precon[i]) >> 1);
      }
      else
      {
        for(i = 0; i != bytewidth; ++i) recon[i] = scanline[i];
        for(i = bytewidth; i < length; ++i) recon[i] = scanline[i] + (recon[i - bytewidth] >> 1);
      }
      break;
    case 4:
      if(precon)
      {
        for(i = 0; i != bytewidth; ++i)
        {
          recon[i] = (scanline[i] + precon[i]); /*paethPredictor(0, precon[i], 0) is always precon[i]*/
        }
        for(i = bytewidth; i < length; ++i)
        {
          recon[i] = (scanline[i] + paethPredictor(recon[i - bytewidth], precon[i], precon[i - bytewidth]));
        }
      }
      else
      {
        for(i = 0; i != bytewidth; ++i)
        {
          recon[i] = scanline[i];
        }
        for(i = bytewidth; i < length; ++i)
        {
          /*paethPredictor(recon[i - bytewidth], 0, 0) is always recon[i - bytewidth]*/
          recon[i] = (scanline[i] + recon[i - bytewidth]);
        }
      }
      break;
    default: return 36; /*error: unexisting filter type given*/
  }
  return 0;
}

static unsigned unfilter(unsigned char* out, const unsigned char* in, unsigned w, unsigned h, unsigned bpp)
{
  /*
  For PNG filter method 0
  this function unfilters a single image (e.g. without interlacing this is called once, with Adam7 seven times)
  out must have enough bytes allocated already, in must have the scanlines + 1 filtertype byte per scanline
  w and h are image dimensions or dimensions of reduced image, bpp is bits per pixel
  in and out are allowed to be the same memory address (but aren't the same size since in has the extra filter bytes)
  */

  unsigned y;
  unsigned char* prevline = 0;

  /*bytewidth is used for filtering, is 1 when bpp < 8, number of bytes per pixel otherwise*/
  size_t bytewidth = (bpp + 7) / 8;
  size_t linebytes = (w * bpp + 7) / 8;

  for(y = 0; y < h; ++y)
  {
    size_t outindex = linebytes * y;
    size_t inindex = (1 + linebytes) * y; /*the extra filterbyte added to each row*/
    unsigned char filterType = in[inindex];

    CERROR_TRY_RETURN(unfilterScanline(&out[outindex], &in[inindex + 1], prevline, bytewidth, filterType, linebytes));

    prevline = &out[outindex];
  }

  return 0;
}

/*
in: Adam7 interlaced image, with no padding bits between scanlines, but between
 reduced images so that each reduced image starts at a byte.
out: the same pixels, but re-ordered so that they're now a non-interlaced image with size w*h
bpp: bits per pixel
out has the following size in bits: w * h * bpp.
in is possibly bigger due to padding bits between reduced images.
out must be big enough AND must be 0 everywhere if bpp < 8 in the current implementation
(because that's likely a little bit faster)
NOTE: comments about padding bits are only relevant if bpp < 8
*/
static void Adam7_deinterlace(unsigned char* out, const unsigned char* in, unsigned w, unsigned h, unsigned bpp)
{
  unsigned passw[7], passh[7];
  size_t filter_passstart[8], padded_passstart[8], passstart[8];
  unsigned i;

  Adam7_getpassvalues(passw, passh, filter_passstart, padded_passstart, passstart, w, h, bpp);

  if(bpp >= 8)
  {
    for(i = 0; i != 7; ++i)
    {
      unsigned x, y, b;
      size_t bytewidth = bpp / 8;
      for(y = 0; y < passh[i]; ++y)
      for(x = 0; x < passw[i]; ++x)
      {
        size_t pixelinstart = passstart[i] + (y * passw[i] + x) * bytewidth;
        size_t pixeloutstart = ((ADAM7_IY[i] + y * ADAM7_DY[i]) * w + ADAM7_IX[i] + x * ADAM7_DX[i]) * bytewidth;
        for(b = 0; b < bytewidth; ++b)
        {
          out[pixeloutstart + b] = in[pixelinstart + b];
        }
      }
    }
  }
  else /*bpp < 8: Adam7 with pixels < 8 bit is a bit trickier: with bit pointers*/
  {
    for(i = 0; i != 7; ++i)
    {
      unsigned x, y, b;
      unsigned ilinebits = bpp * passw[i];
      unsigned olinebits = bpp * w;
      size_t obp, ibp; /*bit pointers (for out and in buffer)*/
      for(y = 0; y < passh[i]; ++y)
      for(x = 0; x < passw[i]; ++x)
      {
        ibp = (8 * passstart[i]) + (y * ilinebits + x * bpp);
        obp = (ADAM7_IY[i] + y * ADAM7_DY[i]) * olinebits + (ADAM7_IX[i] + x * ADAM7_DX[i]) * bpp;
        for(b = 0; b < bpp; ++b)
        {
          unsigned char bit = readBitFromReversedStream(&ibp, in);
          /*note that this function assumes the out buffer is completely 0, use setBitOfReversedStream otherwise*/
          setBitOfReversedStream0(&obp, out, bit);
        }
      }
    }
  }
}

static void removePaddingBits(unsigned char* out, const unsigned char* in,
                              size_t olinebits, size_t ilinebits, unsigned h)
{
  /*
  After filtering there are still padding bits if scanlines have non multiple of 8 bit amounts. They need
  to be removed (except at last scanline of (Adam7-reduced) image) before working with pure image buffers
  for the Adam7 code, the color convert code and the output to the user.
  in and out are allowed to be the same buffer, in may also be higher but still overlapping; in must
  have >= ilinebits*h bits, out must have >= olinebits*h bits, olinebits must be <= ilinebits
  also used to move bits after earlier such operations happened, e.g. in a sequence of reduced images from Adam7
  only useful if (ilinebits - olinebits) is a value in the range 1..7
  */
  unsigned y;
  size_t diff = ilinebits - olinebits;
  size_t ibp = 0, obp = 0; /*input and output bit pointers*/
  for(y = 0; y < h; ++y)
  {
    size_t x;
    for(x = 0; x < olinebits; ++x)
    {
      unsigned char bit = readBitFromReversedStream(&ibp, in);
      setBitOfReversedStream(&obp, out, bit);
    }
    ibp += diff;
  }
}

/*out must be buffer big enough to contain full image, and in must contain the full decompressed data from
the IDAT chunks (with filter index bytes and possible padding bits)
return value is error*/
static unsigned postProcessScanlines(unsigned char* out, unsigned char* in,
                                     unsigned w, unsigned h, const LodePNGInfo* info_png)
{
  /*
  This function converts the filtered-padded-interlaced data into pure 2D image buffer with the PNG's colortype.
  Steps:
  *) if no Adam7: 1) unfilter 2) remove padding bits (= posible extra bits per scanline if bpp < 8)
  *) if adam7: 1) 7x unfilter 2) 7x remove padding bits 3) Adam7_deinterlace
  NOTE: the in buffer will be overwritten with intermediate data!
  */
  unsigned bpp = lodepng_get_bpp(&info_png->color);
  if(bpp == 0) return 31; /*error: invalid colortype*/

  if(info_png->interlace_method == 0)
  {
    if(bpp < 8 && w * bpp != ((w * bpp + 7) / 8) * 8)
    {
      CERROR_TRY_RETURN(unfilter(in, in, w, h, bpp));
      removePaddingBits(out, in, w * bpp, ((w * bpp + 7) / 8) * 8, h);
    }
    /*we can immediately filter into the out buffer, no other steps needed*/
    else CERROR_TRY_RETURN(unfilter(out, in, w, h, bpp));
  }
  else /*interlace_method is 1 (Adam7)*/
  {
    unsigned passw[7], passh[7]; size_t filter_passstart[8], padded_passstart[8], passstart[8];
    unsigned i;

    Adam7_getpassvalues(passw, passh, filter_passstart, padded_passstart, passstart, w, h, bpp);

    for(i = 0; i != 7; ++i)
    {
      CERROR_TRY_RETURN(unfilter(&in[padded_passstart[i]], &in[filter_passstart[i]], passw[i], passh[i], bpp));
      /*TODO: possible efficiency improvement: if in this reduced image the bits fit nicely in 1 scanline,
      move bytes instead of bits or move not at all*/
      if(bpp < 8)
      {
        /*remove padding bits in scanlines; after this there still may be padding
        bits between the different reduced images: each reduced image still starts nicely at a byte*/
        removePaddingBits(&in[passstart[i]], &in[padded_passstart[i]], passw[i] * bpp,
                          ((passw[i] * bpp + 7) / 8) * 8, passh[i]);
      }
    }

    Adam7_deinterlace(out, in, w, h, bpp);
  }

  return 0;
}

static unsigned readChunk_PLTE(LodePNGColorMode* color, const unsigned char* data, size_t chunkLength)
{
  unsigned pos = 0, i;
  if(color->palette) lodepng_free(color->palette);
  color->palettesize = chunkLength / 3;
  color->palette = (unsigned char*)lodepng_malloc(4 * color->palettesize);
  if(!color->palette && color->palettesize)
  {
    color->palettesize = 0;
    return 83; /*alloc fail*/
  }
  if(color->palettesize > 256) return 38; /*error: palette too big*/

  for(i = 0; i != color->palettesize; ++i)
  {
    color->palette[4 * i + 0] = data[pos++]; /*R*/
    color->palette[4 * i + 1] = data[pos++]; /*G*/
    color->palette[4 * i + 2] = data[pos++]; /*B*/
    color->palette[4 * i + 3] = 255; /*alpha*/
  }

  return 0; /* OK */
}

static unsigned readChunk_tRNS(LodePNGColorMode* color, const unsigned char* data, size_t chunkLength)
{
  unsigned i;
  if(color->colortype == LCT_PALETTE)
  {
    /*error: more alpha values given than there are palette entries*/
    if(chunkLength > color->palettesize) return 38;

    for(i = 0; i != chunkLength; ++i) color->palette[4 * i + 3] = data[i];
  }
  else if(color->colortype == LCT_GREY)
  {
    /*error: this chunk must be 2 bytes for greyscale image*/
    if(chunkLength != 2) return 30;

    color->key_defined = 1;
    color->key_r = color->key_g = color->key_b = 256u * data[0] + data[1];
  }
  else if(color->colortype == LCT_RGB)
  {
    /*error: this chunk must be 6 bytes for RGB image*/
    if(chunkLength != 6) return 41;

    color->key_defined = 1;
    color->key_r = 256u * data[0] + data[1];
    color->key_g = 256u * data[2] + data[3];
    color->key_b = 256u * data[4] + data[5];
  }
  else return 42; /*error: tRNS chunk not allowed for other color models*/

  return 0; /* OK */
}


#ifdef LODEPNG_COMPILE_ANCILLARY_CHUNKS
/*background color chunk (bKGD)*/
static unsigned readChunk_bKGD(LodePNGInfo* info, const unsigned char* data, size_t chunkLength)
{
  if(info->color.colortype == LCT_PALETTE)
  {
    /*error: this chunk must be 1 byte for indexed color image*/
    if(chunkLength != 1) return 43;

    info->background_defined = 1;
    info->background_r = info->background_g = info->background_b = data[0];
  }
  else if(info->color.colortype == LCT_GREY || info->color.colortype == LCT_GREY_ALPHA)
  {
    /*error: this chunk must be 2 bytes for greyscale image*/
    if(chunkLength != 2) return 44;

    info->background_defined = 1;
    info->background_r = info->background_g = info->background_b = 256u * data[0] + data[1];
  }
  else if(info->color.colortype == LCT_RGB || info->color.colortype == LCT_RGBA)
  {
    /*error: this chunk must be 6 bytes for greyscale image*/
    if(chunkLength != 6) return 45;

    info->background_defined = 1;
    info->background_r = 256u * data[0] + data[1];
    info->background_g = 256u * data[2] + data[3];
    info->background_b = 256u * data[4] + data[5];
  }

  return 0; /* OK */
}

/*text chunk (tEXt)*/
static unsigned readChunk_tEXt(LodePNGInfo* info, const unsigned char* data, size_t chunkLength)
{
  unsigned error = 0;
  char *key = 0, *str = 0;
  unsigned i;

  while(!error) /*not really a while loop, only used to break on error*/
  {
    unsigned length, string2_begin;

    length = 0;
    while(length < chunkLength && data[length] != 0) ++length;
    /*even though it's not allowed by the standard, no error is thrown if
    there's no null termination char, if the text is empty*/
    if(length < 1 || length > 79) CERROR_BREAK(error, 89); /*keyword too short or long*/

    key = (char*)lodepng_malloc(length + 1);
    if(!key) CERROR_BREAK(error, 83); /*alloc fail*/

    key[length] = 0;
    for(i = 0; i != length; ++i) key[i] = (char)data[i];

    string2_begin = length + 1; /*skip keyword null terminator*/

    length = chunkLength < string2_begin ? 0 : chunkLength - string2_begin;
    str = (char*)lodepng_malloc(length + 1);
    if(!str) CERROR_BREAK(error, 83); /*alloc fail*/

    str[length] = 0;
    for(i = 0; i != length; ++i) str[i] = (char)data[string2_begin + i];

    error = lodepng_add_text(info, key, str);

    break;
  }

  lodepng_free(key);
  lodepng_free(str);

  return error;
}

/*compressed text chunk (zTXt)*/
static unsigned readChunk_zTXt(LodePNGInfo* info, const LodePNGDecompressSettings* zlibsettings,
                               const unsigned char* data, size_t chunkLength)
{
  unsigned error = 0;
  unsigned i;

  unsigned length, string2_begin;
  char *key = 0;
  ucvector decoded;

  ucvector_init(&decoded);

  while(!error) /*not really a while loop, only used to break on error*/
  {
    for(length = 0; length < chunkLength && data[length] != 0; ++length) ;
    if(length + 2 >= chunkLength) CERROR_BREAK(error, 75); /*no null termination, corrupt?*/
    if(length < 1 || length > 79) CERROR_BREAK(error, 89); /*keyword too short or long*/

    key = (char*)lodepng_malloc(length + 1);
    if(!key) CERROR_BREAK(error, 83); /*alloc fail*/

    key[length] = 0;
    for(i = 0; i != length; ++i) key[i] = (char)data[i];

    if(data[length + 1] != 0) CERROR_BREAK(error, 72); /*the 0 byte indicating compression must be 0*/

    string2_begin = length + 2;
    if(string2_begin > chunkLength) CERROR_BREAK(error, 75); /*no null termination, corrupt?*/

    length = chunkLength - string2_begin;
    /*will fail if zlib error, e.g. if length is too small*/
    error = zlib_decompress(&decoded.data, &decoded.size,
                            (unsigned char*)(&data[string2_begin]),
                            length, zlibsettings);
    if(error) break;
    ucvector_push_back(&decoded, 0);

    error = lodepng_add_text(info, key, (char*)decoded.data);

    break;
  }

  lodepng_free(key);
  ucvector_cleanup(&decoded);

  return error;
}

/*international text chunk (iTXt)*/
static unsigned readChunk_iTXt(LodePNGInfo* info, const LodePNGDecompressSettings* zlibsettings,
                               const unsigned char* data, size_t chunkLength)
{
  unsigned error = 0;
  unsigned i;

  unsigned length, begin, compressed;
  char *key = 0, *langtag = 0, *transkey = 0;
  ucvector decoded;
  ucvector_init(&decoded);

  while(!error) /*not really a while loop, only used to break on error*/
  {
    /*Quick check if the chunk length isn't too small. Even without check
    it'd still fail with other error checks below if it's too short. This just gives a different error code.*/
    if(chunkLength < 5) CERROR_BREAK(error, 30); /*iTXt chunk too short*/

    /*read the key*/
    for(length = 0; length < chunkLength && data[length] != 0; ++length) ;
    if(length + 3 >= chunkLength) CERROR_BREAK(error, 75); /*no null termination char, corrupt?*/
    if(length < 1 || length > 79) CERROR_BREAK(error, 89); /*keyword too short or long*/

    key = (char*)lodepng_malloc(length + 1);
    if(!key) CERROR_BREAK(error, 83); /*alloc fail*/

    key[length] = 0;
    for(i = 0; i != length; ++i) key[i] = (char)data[i];

    /*read the compression method*/
    compressed = data[length + 1];
    if(data[length + 2] != 0) CERROR_BREAK(error, 72); /*the 0 byte indicating compression must be 0*/

    /*even though it's not allowed by the standard, no error is thrown if
    there's no null termination char, if the text is empty for the next 3 texts*/

    /*read the langtag*/
    begin = length + 3;
    length = 0;
    for(i = begin; i < chunkLength && data[i] != 0; ++i) ++length;

    langtag = (char*)lodepng_malloc(length + 1);
    if(!langtag) CERROR_BREAK(error, 83); /*alloc fail*/

    langtag[length] = 0;
    for(i = 0; i != length; ++i) langtag[i] = (char)data[begin + i];

    /*read the transkey*/
    begin += length + 1;
    length = 0;
    for(i = begin; i < chunkLength && data[i] != 0; ++i) ++length;

    transkey = (char*)lodepng_malloc(length + 1);
    if(!transkey) CERROR_BREAK(error, 83); /*alloc fail*/

    transkey[length] = 0;
    for(i = 0; i != length; ++i) transkey[i] = (char)data[begin + i];

    /*read the actual text*/
    begin += length + 1;

    length = chunkLength < begin ? 0 : chunkLength - begin;

    if(compressed)
    {
      /*will fail if zlib error, e.g. if length is too small*/
      error = zlib_decompress(&decoded.data, &decoded.size,
                              (unsigned char*)(&data[begin]),
                              length, zlibsettings);
      if(error) break;
      if(decoded.allocsize < decoded.size) decoded.allocsize = decoded.size;
      ucvector_push_back(&decoded, 0);
    }
    else
    {
      if(!ucvector_resize(&decoded, length + 1)) CERROR_BREAK(error, 83 /*alloc fail*/);

      decoded.data[length] = 0;
      for(i = 0; i != length; ++i) decoded.data[i] = data[begin + i];
    }

    error = lodepng_add_itext(info, key, langtag, transkey, (char*)decoded.data);

    break;
  }

  lodepng_free(key);
  lodepng_free(langtag);
  lodepng_free(transkey);
  ucvector_cleanup(&decoded);

  return error;
}

static unsigned readChunk_tIME(LodePNGInfo* info, const unsigned char* data, size_t chunkLength)
{
  if(chunkLength != 7) return 73; /*invalid tIME chunk size*/

  info->time_defined = 1;
  info->time.year = 256u * data[0] + data[1];
  info->time.month = data[2];
  info->time.day = data[3];
  info->time.hour = data[4];
  info->time.minute = data[5];
  info->time.second = data[6];

  return 0; /* OK */
}

static unsigned readChunk_pHYs(LodePNGInfo* info, const unsigned char* data, size_t chunkLength)
{
  if(chunkLength != 9) return 74; /*invalid pHYs chunk size*/

  info->phys_defined = 1;
  info->phys_x = 16777216u * data[0] + 65536u * data[1] + 256u * data[2] + data[3];
  info->phys_y = 16777216u * data[4] + 65536u * data[5] + 256u * data[6] + data[7];
  info->phys_unit = data[8];

  return 0; /* OK */
}
#endif /*LODEPNG_COMPILE_ANCILLARY_CHUNKS*/

/*read a PNG, the result will be in the same color type as the PNG (hence "generic")*/
static void decodeGeneric(unsigned char** out, unsigned* w, unsigned* h,
                          LodePNGState* state,
                          const unsigned char* in, size_t insize)
{
  unsigned char IEND = 0;
  const unsigned char* chunk;
  size_t i;
  ucvector idat; /*the data from idat chunks*/
  ucvector scanlines;
  size_t predict;
  size_t numpixels;
  size_t outsize = 0;

  /*for unknown chunk order*/
  unsigned unknown = 0;
#ifdef LODEPNG_COMPILE_ANCILLARY_CHUNKS
  unsigned critical_pos = 1; /*1 = after IHDR, 2 = after PLTE, 3 = after IDAT*/
#endif /*LODEPNG_COMPILE_ANCILLARY_CHUNKS*/

  /*provide some proper output values if error will happen*/
  *out = 0;

  state->error = lodepng_inspect(w, h, state, in, insize); /*reads header and resets other parameters in state->info_png*/
  if(state->error) return;

  numpixels = *w * *h;

  /*multiplication overflow*/
  if(*h != 0 && numpixels / *h != *w) CERROR_RETURN(state->error, 92);
  /*multiplication overflow possible further below. Allows up to 2^31-1 pixel
  bytes with 16-bit RGBA, the rest is room for filter bytes.*/
  if(numpixels > 268435455) CERROR_RETURN(state->error, 92);

  ucvector_init(&idat);
  chunk = &in[33]; /*first byte of the first chunk after the header*/

  /*loop through the chunks, ignoring unknown chunks and stopping at IEND chunk.
  IDAT data is put at the start of the in buffer*/
  while(!IEND && !state->error)
  {
    unsigned chunkLength;
    const unsigned char* data; /*the data in the chunk*/

    /*error: size of the in buffer too small to contain next chunk*/
    if((size_t)((chunk - in) + 12) > insize || chunk < in) CERROR_BREAK(state->error, 30);

    /*length of the data of the chunk, excluding the length bytes, chunk type and CRC bytes*/
    chunkLength = lodepng_chunk_length(chunk);
    /*error: chunk length larger than the max PNG chunk size*/
    if(chunkLength > 2147483647) CERROR_BREAK(state->error, 63);

    if((size_t)((chunk - in) + chunkLength + 12) > insize || (chunk + chunkLength + 12) < in)
    {
      CERROR_BREAK(state->error, 64); /*error: size of the in buffer too small to contain next chunk*/
    }

    data = lodepng_chunk_data_const(chunk);

    /*IDAT chunk, containing compressed image data*/
    if(lodepng_chunk_type_equals(chunk, "IDAT"))
    {
      size_t oldsize = idat.size;
      if(!ucvector_resize(&idat, oldsize + chunkLength)) CERROR_BREAK(state->error, 83 /*alloc fail*/);
      for(i = 0; i != chunkLength; ++i) idat.data[oldsize + i] = data[i];
#ifdef LODEPNG_COMPILE_ANCILLARY_CHUNKS
      critical_pos = 3;
#endif /*LODEPNG_COMPILE_ANCILLARY_CHUNKS*/
    }
    /*IEND chunk*/
    else if(lodepng_chunk_type_equals(chunk, "IEND"))
    {
      IEND = 1;
    }
    /*palette chunk (PLTE)*/
    else if(lodepng_chunk_type_equals(chunk, "PLTE"))
    {
      state->error = readChunk_PLTE(&state->info_png.color, data, chunkLength);
      if(state->error) break;
#ifdef LODEPNG_COMPILE_ANCILLARY_CHUNKS
      critical_pos = 2;
#endif /*LODEPNG_COMPILE_ANCILLARY_CHUNKS*/
    }
    /*palette transparency chunk (tRNS)*/
    else if(lodepng_chunk_type_equals(chunk, "tRNS"))
    {
      state->error = readChunk_tRNS(&state->info_png.color, data, chunkLength);
      if(state->error) break;
    }
#ifdef LODEPNG_COMPILE_ANCILLARY_CHUNKS
    /*background color chunk (bKGD)*/
    else if(lodepng_chunk_type_equals(chunk, "bKGD"))
    {
      state->error = readChunk_bKGD(&state->info_png, data, chunkLength);
      if(state->error) break;
    }
    /*text chunk (tEXt)*/
    else if(lodepng_chunk_type_equals(chunk, "tEXt"))
    {
      if(state->decoder.read_text_chunks)
      {
        state->error = readChunk_tEXt(&state->info_png, data, chunkLength);
        if(state->error) break;
      }
    }
    /*compressed text chunk (zTXt)*/
    else if(lodepng_chunk_type_equals(chunk, "zTXt"))
    {
      if(state->decoder.read_text_chunks)
      {
        state->error = readChunk_zTXt(&state->info_png, &state->decoder.zlibsettings, data, chunkLength);
        if(state->error) break;
      }
    }
    /*international text chunk (iTXt)*/
    else if(lodepng_chunk_type_equals(chunk, "iTXt"))
    {
      if(state->decoder.read_text_chunks)
      {
        state->error = readChunk_iTXt(&state->info_png, &state->decoder.zlibsettings, data, chunkLength);
        if(state->error) break;
      }
    }
    else if(lodepng_chunk_type_equals(chunk, "tIME"))
    {
      state->error = readChunk_tIME(&state->info_png, data, chunkLength);
      if(state->error) break;
    }
    else if(lodepng_chunk_type_equals(chunk, "pHYs"))
    {
      state->error = readChunk_pHYs(&state->info_png, data, chunkLength);
      if(state->error) break;
    }
#endif /*LODEPNG_COMPILE_ANCILLARY_CHUNKS*/
    else /*it's not an implemented chunk type, so ignore it: skip over the data*/
    {
      /*error: unknown critical chunk (5th bit of first byte of chunk type is 0)*/
      if(!lodepng_chunk_ancillary(chunk)) CERROR_BREAK(state->error, 69);

      unknown = 1;
#ifdef LODEPNG_COMPILE_ANCILLARY_CHUNKS
      if(state->decoder.remember_unknown_chunks)
      {
        state->error = lodepng_chunk_append(&state->info_png.unknown_chunks_data[critical_pos - 1],
                                            &state->info_png.unknown_chunks_size[critical_pos - 1], chunk);
        if(state->error) break;
      }
#endif /*LODEPNG_COMPILE_ANCILLARY_CHUNKS*/
    }

    if(!state->decoder.ignore_crc && !unknown) /*check CRC if wanted, only on known chunk types*/
    {
      if(lodepng_chunk_check_crc(chunk)) CERROR_BREAK(state->error, 57); /*invalid CRC*/
    }

    if(!IEND) chunk = lodepng_chunk_next_const(chunk);
  }

  ucvector_init(&scanlines);
  /*predict output size, to allocate exact size for output buffer to avoid more dynamic allocation.
  If the decompressed size does not match the prediction, the image must be corrupt.*/
  if(state->info_png.interlace_method == 0)
  {
    /*The extra *h is added because this are the filter bytes every scanline starts with*/
    predict = lodepng_get_raw_size_idat(*w, *h, &state->info_png.color) + *h;
  }
  else
  {
    /*Adam-7 interlaced: predicted size is the sum of the 7 sub-images sizes*/
    const LodePNGColorMode* color = &state->info_png.color;
    predict = 0;
    predict += lodepng_get_raw_size_idat((*w + 7) >> 3, (*h + 7) >> 3, color) + ((*h + 7) >> 3);
    if(*w > 4) predict += lodepng_get_raw_size_idat((*w + 3) >> 3, (*h + 7) >> 3, color) + ((*h + 7) >> 3);
    predict += lodepng_get_raw_size_idat((*w + 3) >> 2, (*h + 3) >> 3, color) + ((*h + 3) >> 3);
    if(*w > 2) predict += lodepng_get_raw_size_idat((*w + 1) >> 2, (*h + 3) >> 2, color) + ((*h + 3) >> 2);
    predict += lodepng_get_raw_size_idat((*w + 1) >> 1, (*h + 1) >> 2, color) + ((*h + 1) >> 2);
    if(*w > 1) predict += lodepng_get_raw_size_idat((*w + 0) >> 1, (*h + 1) >> 1, color) + ((*h + 1) >> 1);
    predict += lodepng_get_raw_size_idat((*w + 0), (*h + 0) >> 1, color) + ((*h + 0) >> 1);
  }
  if(!state->error && !ucvector_reserve(&scanlines, predict)) state->error = 83; /*alloc fail*/
  if(!state->error)
  {
    state->error = zlib_decompress(&scanlines.data, &scanlines.size, idat.data,
                                   idat.size, &state->decoder.zlibsettings);
    if(!state->error && scanlines.size != predict) state->error = 91; /*decompressed size doesn't match prediction*/
  }
  ucvector_cleanup(&idat);

  if(!state->error)
  {
    outsize = lodepng_get_raw_size(*w, *h, &state->info_png.color);
    *out = (unsigned char*)lodepng_malloc(outsize);
    if(!*out) state->error = 83; /*alloc fail*/
  }
  if(!state->error)
  {
    for(i = 0; i < outsize; i++) (*out)[i] = 0;
    state->error = postProcessScanlines(*out, scanlines.data, *w, *h, &state->info_png);
  }
  ucvector_cleanup(&scanlines);
}

unsigned lodepng_decode(unsigned char** out, unsigned* w, unsigned* h,
                        LodePNGState* state,
                        const unsigned char* in, size_t insize)
{
  *out = 0;
  decodeGeneric(out, w, h, state, in, insize);
  if(state->error) return state->error;
  if(!state->decoder.color_convert || lodepng_color_mode_equal(&state->info_raw, &state->info_png.color))
  {
    /*same color type, no copying or converting of data needed*/
    /*store the info_png color settings on the info_raw so that the info_raw still reflects what colortype
    the raw image has to the end user*/
    if(!state->decoder.color_convert)
    {
      state->error = lodepng_color_mode_copy(&state->info_raw, &state->info_png.color);
      if(state->error) return state->error;
    }
  }
  else
  {
    /*color conversion needed; sort of copy of the data*/
    unsigned char* data = *out;
    size_t outsize;

    /*TODO: check if this works according to the statement in the documentation: "The converter can convert
    from greyscale input color type, to 8-bit greyscale or greyscale with alpha"*/
    if(!(state->info_raw.colortype == LCT_RGB || state->info_raw.colortype == LCT_RGBA)
       && !(state->info_raw.bitdepth == 8))
    {
      return 56; /*unsupported color mode conversion*/
    }

    outsize = lodepng_get_raw_size(*w, *h, &state->info_raw);
    *out = (unsigned char*)lodepng_malloc(outsize);
    if(!(*out))
    {
      state->error = 83; /*alloc fail*/
    }
    else state->error = lodepng_convert(*out, data, &state->info_raw,
                                        &state->info_png.color, *w, *h);
    lodepng_free(data);
  }
  return state->error;
}

unsigned lodepng_decode_memory(unsigned char** out, unsigned* w, unsigned* h, const unsigned char* in,
                               size_t insize, LodePNGColorType colortype, unsigned bitdepth)
{
  unsigned error;
  LodePNGState state;
  lodepng_state_init(&state);
  state.info_raw.colortype = colortype;
  state.info_raw.bitdepth = bitdepth;
  error = lodepng_decode(out, w, h, &state, in, insize);
  lodepng_state_cleanup(&state);
  return error;
}

unsigned lodepng_decode32(unsigned char** out, unsigned* w, unsigned* h, const unsigned char* in, size_t insize)
{
  return lodepng_decode_memory(out, w, h, in, insize, LCT_RGBA, 8);
}

unsigned lodepng_decode24(unsigned char** out, unsigned* w, unsigned* h, const unsigned char* in, size_t insize)
{
  return lodepng_decode_memory(out, w, h, in, insize, LCT_RGB, 8);
}

#ifdef LODEPNG_COMPILE_DISK
unsigned lodepng_decode_file(unsigned char** out, unsigned* w, unsigned* h, const char* filename,
                             LodePNGColorType colortype, unsigned bitdepth)
{
  unsigned char* buffer = 0;
  size_t buffersize;
  unsigned error;
  error = lodepng_load_file(&buffer, &buffersize, filename);
  if(!error) error = lodepng_decode_memory(out, w, h, buffer, buffersize, colortype, bitdepth);
  lodepng_free(buffer);
  return error;
}

unsigned lodepng_decode32_file(unsigned char** out, unsigned* w, unsigned* h, const char* filename)
{
  return lodepng_decode_file(out, w, h, filename, LCT_RGBA, 8);
}

unsigned lodepng_decode24_file(unsigned char** out, unsigned* w, unsigned* h, const char* filename)
{
  return lodepng_decode_file(out, w, h, filename, LCT_RGB, 8);
}
#endif /*LODEPNG_COMPILE_DISK*/

void lodepng_decoder_settings_init(LodePNGDecoderSettings* settings)
{
  settings->color_convert = 1;
#ifdef LODEPNG_COMPILE_ANCILLARY_CHUNKS
  settings->read_text_chunks = 1;
  settings->remember_unknown_chunks = 0;
#endif /*LODEPNG_COMPILE_ANCILLARY_CHUNKS*/
  settings->ignore_crc = 0;
  lodepng_decompress_settings_init(&settings->zlibsettings);
}

#endif /*LODEPNG_COMPILE_DECODER*/

#if defined(LODEPNG_COMPILE_DECODER) || defined(LODEPNG_COMPILE_ENCODER)

void lodepng_state_init(LodePNGState* state)
{
#ifdef LODEPNG_COMPILE_DECODER
  lodepng_decoder_settings_init(&state->decoder);
#endif /*LODEPNG_COMPILE_DECODER*/
#ifdef LODEPNG_COMPILE_ENCODER
  lodepng_encoder_settings_init(&state->encoder);
#endif /*LODEPNG_COMPILE_ENCODER*/
  lodepng_color_mode_init(&state->info_raw);
  lodepng_info_init(&state->info_png);
  state->error = 1;
}

void lodepng_state_cleanup(LodePNGState* state)
{
  lodepng_color_mode_cleanup(&state->info_raw);
  lodepng_info_cleanup(&state->info_png);
}

void lodepng_state_copy(LodePNGState* dest, const LodePNGState* source)
{
  lodepng_state_cleanup(dest);
  *dest = *source;
  lodepng_color_mode_init(&dest->info_raw);
  lodepng_info_init(&dest->info_png);
  dest->error = lodepng_color_mode_copy(&dest->info_raw, &source->info_raw); if(dest->error) return;
  dest->error = lodepng_info_copy(&dest->info_png, &source->info_png); if(dest->error) return;
}

#endif /* defined(LODEPNG_COMPILE_DECODER) || defined(LODEPNG_COMPILE_ENCODER) */

#ifdef LODEPNG_COMPILE_ENCODER

/* ////////////////////////////////////////////////////////////////////////// */
/* / PNG Encoder                                                            / */
/* ////////////////////////////////////////////////////////////////////////// */

/*chunkName must be string of 4 characters*/
static unsigned addChunk(ucvector* out, const char* chunkName, const unsigned char* data, size_t length)
{
  CERROR_TRY_RETURN(lodepng_chunk_create(&out->data, &out->size, (unsigned)length, chunkName, data));
  out->allocsize = out->size; /*fix the allocsize again*/
  return 0;
}

static void writeSignature(ucvector* out)
{
  /*8 bytes PNG signature, aka the magic bytes*/
  ucvector_push_back(out, 137);
  ucvector_push_back(out, 80);
  ucvector_push_back(out, 78);
  ucvector_push_back(out, 71);
  ucvector_push_back(out, 13);
  ucvector_push_back(out, 10);
  ucvector_push_back(out, 26);
  ucvector_push_back(out, 10);
}

static unsigned addChunk_IHDR(ucvector* out, unsigned w, unsigned h,
                              LodePNGColorType colortype, unsigned bitdepth, unsigned interlace_method)
{
  unsigned error = 0;
  ucvector header;
  ucvector_init(&header);

  lodepng_add32bitInt(&header, w); /*width*/
  lodepng_add32bitInt(&header, h); /*height*/
  ucvector_push_back(&header, (unsigned char)bitdepth); /*bit depth*/
  ucvector_push_back(&header, (unsigned char)colortype); /*color type*/
  ucvector_push_back(&header, 0); /*compression method*/
  ucvector_push_back(&header, 0); /*filter method*/
  ucvector_push_back(&header, interlace_method); /*interlace method*/

  error = addChunk(out, "IHDR", header.data, header.size);
  ucvector_cleanup(&header);

  return error;
}

static unsigned addChunk_PLTE(ucvector* out, const LodePNGColorMode* info)
{
  unsigned error = 0;
  size_t i;
  ucvector PLTE;
  ucvector_init(&PLTE);
  for(i = 0; i != info->palettesize * 4; ++i)
  {
    /*add all channels except alpha channel*/
    if(i % 4 != 3) ucvector_push_back(&PLTE, info->palette[i]);
  }
  error = addChunk(out, "PLTE", PLTE.data, PLTE.size);
  ucvector_cleanup(&PLTE);

  return error;
}

static unsigned addChunk_tRNS(ucvector* out, const LodePNGColorMode* info)
{
  unsigned error = 0;
  size_t i;
  ucvector tRNS;
  ucvector_init(&tRNS);
  if(info->colortype == LCT_PALETTE)
  {
    size_t amount = info->palettesize;
    /*the tail of palette values that all have 255 as alpha, does not have to be encoded*/
    for(i = info->palettesize; i != 0; --i)
    {
      if(info->palette[4 * (i - 1) + 3] == 255) --amount;
      else break;
    }
    /*add only alpha channel*/
    for(i = 0; i != amount; ++i) ucvector_push_back(&tRNS, info->palette[4 * i + 3]);
  }
  else if(info->colortype == LCT_GREY)
  {
    if(info->key_defined)
    {
      ucvector_push_back(&tRNS, (unsigned char)(info->key_r >> 8));
      ucvector_push_back(&tRNS, (unsigned char)(info->key_r & 255));
    }
  }
  else if(info->colortype == LCT_RGB)
  {
    if(info->key_defined)
    {
      ucvector_push_back(&tRNS, (unsigned char)(info->key_r >> 8));
      ucvector_push_back(&tRNS, (unsigned char)(info->key_r & 255));
      ucvector_push_back(&tRNS, (unsigned char)(info->key_g >> 8));
      ucvector_push_back(&tRNS, (unsigned char)(info->key_g & 255));
      ucvector_push_back(&tRNS, (unsigned char)(info->key_b >> 8));
      ucvector_push_back(&tRNS, (unsigned char)(info->key_b & 255));
    }
  }

  error = addChunk(out, "tRNS", tRNS.data, tRNS.size);
  ucvector_cleanup(&tRNS);

  return error;
}

static unsigned addChunk_IDAT(ucvector* out, const unsigned char* data, size_t datasize,
                              LodePNGCompressSettings* zlibsettings)
{
  ucvector zlibdata;
  unsigned error = 0;

  /*compress with the Zlib compressor*/
  ucvector_init(&zlibdata);
  error = zlib_compress(&zlibdata.data, &zlibdata.size, data, datasize, zlibsettings);
  if(!error) error = addChunk(out, "IDAT", zlibdata.data, zlibdata.size);
  ucvector_cleanup(&zlibdata);

  return error;
}

static unsigned addChunk_IEND(ucvector* out)
{
  unsigned error = 0;
  error = addChunk(out, "IEND", 0, 0);
  return error;
}

#ifdef LODEPNG_COMPILE_ANCILLARY_CHUNKS

static unsigned addChunk_tEXt(ucvector* out, const char* keyword, const char* textstring)
{
  unsigned error = 0;
  size_t i;
  ucvector text;
  ucvector_init(&text);
  for(i = 0; keyword[i] != 0; ++i) ucvector_push_back(&text, (unsigned char)keyword[i]);
  if(i < 1 || i > 79) return 89; /*error: invalid keyword size*/
  ucvector_push_back(&text, 0); /*0 termination char*/
  for(i = 0; textstring[i] != 0; ++i) ucvector_push_back(&text, (unsigned char)textstring[i]);
  error = addChunk(out, "tEXt", text.data, text.size);
  ucvector_cleanup(&text);

  return error;
}

static unsigned addChunk_zTXt(ucvector* out, const char* keyword, const char* textstring,
                              LodePNGCompressSettings* zlibsettings)
{
  unsigned error = 0;
  ucvector data, compressed;
  size_t i, textsize = strlen(textstring);

  ucvector_init(&data);
  ucvector_init(&compressed);
  for(i = 0; keyword[i] != 0; ++i) ucvector_push_back(&data, (unsigned char)keyword[i]);
  if(i < 1 || i > 79) return 89; /*error: invalid keyword size*/
  ucvector_push_back(&data, 0); /*0 termination char*/
  ucvector_push_back(&data, 0); /*compression method: 0*/

  error = zlib_compress(&compressed.data, &compressed.size,
                        (unsigned char*)textstring, textsize, zlibsettings);
  if(!error)
  {
    for(i = 0; i != compressed.size; ++i) ucvector_push_back(&data, compressed.data[i]);
    error = addChunk(out, "zTXt", data.data, data.size);
  }

  ucvector_cleanup(&compressed);
  ucvector_cleanup(&data);
  return error;
}

static unsigned addChunk_iTXt(ucvector* out, unsigned compressed, const char* keyword, const char* langtag,
                              const char* transkey, const char* textstring, LodePNGCompressSettings* zlibsettings)
{
  unsigned error = 0;
  ucvector data;
  size_t i, textsize = strlen(textstring);

  ucvector_init(&data);

  for(i = 0; keyword[i] != 0; ++i) ucvector_push_back(&data, (unsigned char)keyword[i]);
  if(i < 1 || i > 79) return 89; /*error: invalid keyword size*/
  ucvector_push_back(&data, 0); /*null termination char*/
  ucvector_push_back(&data, compressed ? 1 : 0); /*compression flag*/
  ucvector_push_back(&data, 0); /*compression method*/
  for(i = 0; langtag[i] != 0; ++i) ucvector_push_back(&data, (unsigned char)langtag[i]);
  ucvector_push_back(&data, 0); /*null termination char*/
  for(i = 0; transkey[i] != 0; ++i) ucvector_push_back(&data, (unsigned char)transkey[i]);
  ucvector_push_back(&data, 0); /*null termination char*/

  if(compressed)
  {
    ucvector compressed_data;
    ucvector_init(&compressed_data);
    error = zlib_compress(&compressed_data.data, &compressed_data.size,
                          (unsigned char*)textstring, textsize, zlibsettings);
    if(!error)
    {
      for(i = 0; i != compressed_data.size; ++i) ucvector_push_back(&data, compressed_data.data[i]);
    }
    ucvector_cleanup(&compressed_data);
  }
  else /*not compressed*/
  {
    for(i = 0; textstring[i] != 0; ++i) ucvector_push_back(&data, (unsigned char)textstring[i]);
  }

  if(!error) error = addChunk(out, "iTXt", data.data, data.size);
  ucvector_cleanup(&data);
  return error;
}

static unsigned addChunk_bKGD(ucvector* out, const LodePNGInfo* info)
{
  unsigned error = 0;
  ucvector bKGD;
  ucvector_init(&bKGD);
  if(info->color.colortype == LCT_GREY || info->color.colortype == LCT_GREY_ALPHA)
  {
    ucvector_push_back(&bKGD, (unsigned char)(info->background_r >> 8));
    ucvector_push_back(&bKGD, (unsigned char)(info->background_r & 255));
  }
  else if(info->color.colortype == LCT_RGB || info->color.colortype == LCT_RGBA)
  {
    ucvector_push_back(&bKGD, (unsigned char)(info->background_r >> 8));
    ucvector_push_back(&bKGD, (unsigned char)(info->background_r & 255));
    ucvector_push_back(&bKGD, (unsigned char)(info->background_g >> 8));
    ucvector_push_back(&bKGD, (unsigned char)(info->background_g & 255));
    ucvector_push_back(&bKGD, (unsigned char)(info->background_b >> 8));
    ucvector_push_back(&bKGD, (unsigned char)(info->background_b & 255));
  }
  else if(info->color.colortype == LCT_PALETTE)
  {
    ucvector_push_back(&bKGD, (unsigned char)(info->background_r & 255)); /*palette index*/
  }

  error = addChunk(out, "bKGD", bKGD.data, bKGD.size);
  ucvector_cleanup(&bKGD);

  return error;
}

static unsigned addChunk_tIME(ucvector* out, const LodePNGTime* time)
{
  unsigned error = 0;
  unsigned char* data = (unsigned char*)lodepng_malloc(7);
  if(!data) return 83; /*alloc fail*/
  data[0] = (unsigned char)(time->year >> 8);
  data[1] = (unsigned char)(time->year & 255);
  data[2] = (unsigned char)time->month;
  data[3] = (unsigned char)time->day;
  data[4] = (unsigned char)time->hour;
  data[5] = (unsigned char)time->minute;
  data[6] = (unsigned char)time->second;
  error = addChunk(out, "tIME", data, 7);
  lodepng_free(data);
  return error;
}

static unsigned addChunk_pHYs(ucvector* out, const LodePNGInfo* info)
{
  unsigned error = 0;
  ucvector data;
  ucvector_init(&data);

  lodepng_add32bitInt(&data, info->phys_x);
  lodepng_add32bitInt(&data, info->phys_y);
  ucvector_push_back(&data, info->phys_unit);

  error = addChunk(out, "pHYs", data.data, data.size);
  ucvector_cleanup(&data);

  return error;
}

#endif /*LODEPNG_COMPILE_ANCILLARY_CHUNKS*/

static void filterScanline(unsigned char* out, const unsigned char* scanline, const unsigned char* prevline,
                           size_t length, size_t bytewidth, unsigned char filterType)
{
  size_t i;
  switch(filterType)
  {
    case 0: /*None*/
      for(i = 0; i != length; ++i) out[i] = scanline[i];
      break;
    case 1: /*Sub*/
      for(i = 0; i != bytewidth; ++i) out[i] = scanline[i];
      for(i = bytewidth; i < length; ++i) out[i] = scanline[i] - scanline[i - bytewidth];
      break;
    case 2: /*Up*/
      if(prevline)
      {
        for(i = 0; i != length; ++i) out[i] = scanline[i] - prevline[i];
      }
      else
      {
        for(i = 0; i != length; ++i) out[i] = scanline[i];
      }
      break;
    case 3: /*Average*/
      if(prevline)
      {
        for(i = 0; i != bytewidth; ++i) out[i] = scanline[i] - (prevline[i] >> 1);
        for(i = bytewidth; i < length; ++i) out[i] = scanline[i] - ((scanline[i - bytewidth] + prevline[i]) >> 1);
      }
      else
      {
        for(i = 0; i != bytewidth; ++i) out[i] = scanline[i];
        for(i = bytewidth; i < length; ++i) out[i] = scanline[i] - (scanline[i - bytewidth] >> 1);
      }
      break;
    case 4: /*Paeth*/
      if(prevline)
      {
        /*paethPredictor(0, prevline[i], 0) is always prevline[i]*/
        for(i = 0; i != bytewidth; ++i) out[i] = (scanline[i] - prevline[i]);
        for(i = bytewidth; i < length; ++i)
        {
          out[i] = (scanline[i] - paethPredictor(scanline[i - bytewidth], prevline[i], prevline[i - bytewidth]));
        }
      }
      else
      {
        for(i = 0; i != bytewidth; ++i) out[i] = scanline[i];
        /*paethPredictor(scanline[i - bytewidth], 0, 0) is always scanline[i - bytewidth]*/
        for(i = bytewidth; i < length; ++i) out[i] = (scanline[i] - scanline[i - bytewidth]);
      }
      break;
    default: return; /*unexisting filter type given*/
  }
}

/* log2 approximation. A slight bit faster than std::log. */
static float flog2(float f)
{
  float result = 0;
  while(f > 32) { result += 4; f /= 16; }
  while(f > 2) { ++result; f /= 2; }
  return result + 1.442695f * (f * f * f / 3 - 3 * f * f / 2 + 3 * f - 1.83333f);
}

static unsigned filter(unsigned char* out, const unsigned char* in, unsigned w, unsigned h,
                       const LodePNGColorMode* info, const LodePNGEncoderSettings* settings)
{
  /*
  For PNG filter method 0
  out must be a buffer with as size: h + (w * h * bpp + 7) / 8, because there are
  the scanlines with 1 extra byte per scanline
  */

  unsigned bpp = lodepng_get_bpp(info);
  /*the width of a scanline in bytes, not including the filter type*/
  size_t linebytes = (w * bpp + 7) / 8;
  /*bytewidth is used for filtering, is 1 when bpp < 8, number of bytes per pixel otherwise*/
  size_t bytewidth = (bpp + 7) / 8;
  const unsigned char* prevline = 0;
  unsigned x, y;
  unsigned error = 0;
  LodePNGFilterStrategy strategy = settings->filter_strategy;

  /*
  There is a heuristic called the minimum sum of absolute differences heuristic, suggested by the PNG standard:
   *  If the image type is Palette, or the bit depth is smaller than 8, then do not filter the image (i.e.
      use fixed filtering, with the filter None).
   * (The other case) If the image type is Grayscale or RGB (with or without Alpha), and the bit depth is
     not smaller than 8, then use adaptive filtering heuristic as follows: independently for each row, apply
     all five filters and select the filter that produces the smallest sum of absolute values per row.
  This heuristic is used if filter strategy is LFS_MINSUM and filter_palette_zero is true.

  If filter_palette_zero is true and filter_strategy is not LFS_MINSUM, the above heuristic is followed,
  but for "the other case", whatever strategy filter_strategy is set to instead of the minimum sum
  heuristic is used.
  */
  if(settings->filter_palette_zero &&
     (info->colortype == LCT_PALETTE || info->bitdepth < 8)) strategy = LFS_ZERO;

  if(bpp == 0) return 31; /*error: invalid color type*/

  if(strategy == LFS_ZERO)
  {
    for(y = 0; y != h; ++y)
    {
      size_t outindex = (1 + linebytes) * y; /*the extra filterbyte added to each row*/
      size_t inindex = linebytes * y;
      out[outindex] = 0; /*filter type byte*/
      filterScanline(&out[outindex + 1], &in[inindex], prevline, linebytes, bytewidth, 0);
      prevline = &in[inindex];
    }
  }
  else if(strategy == LFS_MINSUM)
  {
    /*adaptive filtering*/
    size_t sum[5];
    unsigned char* attempt[5]; /*five filtering attempts, one for each filter type*/
    size_t smallest = 0;
    unsigned char type, bestType = 0;

    for(type = 0; type != 5; ++type)
    {
      attempt[type] = (unsigned char*)lodepng_malloc(linebytes);
      if(!attempt[type]) return 83; /*alloc fail*/
    }

    if(!error)
    {
      for(y = 0; y != h; ++y)
      {
        /*try the 5 filter types*/
        for(type = 0; type != 5; ++type)
        {
          filterScanline(attempt[type], &in[y * linebytes], prevline, linebytes, bytewidth, type);

          /*calculate the sum of the result*/
          sum[type] = 0;
          if(type == 0)
          {
            for(x = 0; x != linebytes; ++x) sum[type] += (unsigned char)(attempt[type][x]);
          }
          else
          {
            for(x = 0; x != linebytes; ++x)
            {
              /*For differences, each byte should be treated as signed, values above 127 are negative
              (converted to signed char). Filtertype 0 isn't a difference though, so use unsigned there.
              This means filtertype 0 is almost never chosen, but that is justified.*/
              unsigned char s = attempt[type][x];
              sum[type] += s < 128 ? s : (255U - s);
            }
          }

          /*check if this is smallest sum (or if type == 0 it's the first case so always store the values)*/
          if(type == 0 || sum[type] < smallest)
          {
            bestType = type;
            smallest = sum[type];
          }
        }

        prevline = &in[y * linebytes];

        /*now fill the out values*/
        out[y * (linebytes + 1)] = bestType; /*the first byte of a scanline will be the filter type*/
        for(x = 0; x != linebytes; ++x) out[y * (linebytes + 1) + 1 + x] = attempt[bestType][x];
      }
    }

    for(type = 0; type != 5; ++type) lodepng_free(attempt[type]);
  }
  else if(strategy == LFS_ENTROPY)
  {
    float sum[5];
    unsigned char* attempt[5]; /*five filtering attempts, one for each filter type*/
    float smallest = 0;
    unsigned type, bestType = 0;
    unsigned count[256];

    for(type = 0; type != 5; ++type)
    {
      attempt[type] = (unsigned char*)lodepng_malloc(linebytes);
      if(!attempt[type]) return 83; /*alloc fail*/
    }

    for(y = 0; y != h; ++y)
    {
      /*try the 5 filter types*/
      for(type = 0; type != 5; ++type)
      {
        filterScanline(attempt[type], &in[y * linebytes], prevline, linebytes, bytewidth, type);
        for(x = 0; x != 256; ++x) count[x] = 0;
        for(x = 0; x != linebytes; ++x) ++count[attempt[type][x]];
        ++count[type]; /*the filter type itself is part of the scanline*/
        sum[type] = 0;
        for(x = 0; x != 256; ++x)
        {
          float p = count[x] / (float)(linebytes + 1);
          sum[type] += count[x] == 0 ? 0 : flog2(1 / p) * p;
        }
        /*check if this is smallest sum (or if type == 0 it's the first case so always store the values)*/
        if(type == 0 || sum[type] < smallest)
        {
          bestType = type;
          smallest = sum[type];
        }
      }

      prevline = &in[y * linebytes];

      /*now fill the out values*/
      out[y * (linebytes + 1)] = bestType; /*the first byte of a scanline will be the filter type*/
      for(x = 0; x != linebytes; ++x) out[y * (linebytes + 1) + 1 + x] = attempt[bestType][x];
    }

    for(type = 0; type != 5; ++type) lodepng_free(attempt[type]);
  }
  else if(strategy == LFS_PREDEFINED)
  {
    for(y = 0; y != h; ++y)
    {
      size_t outindex = (1 + linebytes) * y; /*the extra filterbyte added to each row*/
      size_t inindex = linebytes * y;
      unsigned char type = settings->predefined_filters[y];
      out[outindex] = type; /*filter type byte*/
      filterScanline(&out[outindex + 1], &in[inindex], prevline, linebytes, bytewidth, type);
      prevline = &in[inindex];
    }
  }
  else if(strategy == LFS_BRUTE_FORCE)
  {
    /*brute force filter chooser.
    deflate the scanline after every filter attempt to see which one deflates best.
    This is very slow and gives only slightly smaller, sometimes even larger, result*/
    size_t size[5];
    unsigned char* attempt[5]; /*five filtering attempts, one for each filter type*/
    size_t smallest = 0;
    unsigned type = 0, bestType = 0;
    unsigned char* dummy;
    LodePNGCompressSettings zlibsettings = settings->zlibsettings;
    /*use fixed tree on the attempts so that the tree is not adapted to the filtertype on purpose,
    to simulate the true case where the tree is the same for the whole image. Sometimes it gives
    better result with dynamic tree anyway. Using the fixed tree sometimes gives worse, but in rare
    cases better compression. It does make this a bit less slow, so it's worth doing this.*/
    zlibsettings.btype = 1;
    /*a custom encoder likely doesn't read the btype setting and is optimized for complete PNG
    images only, so disable it*/
    zlibsettings.custom_zlib = 0;
    zlibsettings.custom_deflate = 0;
    for(type = 0; type != 5; ++type)
    {
      attempt[type] = (unsigned char*)lodepng_malloc(linebytes);
      if(!attempt[type]) return 83; /*alloc fail*/
    }
    for(y = 0; y != h; ++y) /*try the 5 filter types*/
    {
      for(type = 0; type != 5; ++type)
      {
        unsigned testsize = linebytes;
        /*if(testsize > 8) testsize /= 8;*/ /*it already works good enough by testing a part of the row*/

        filterScanline(attempt[type], &in[y * linebytes], prevline, linebytes, bytewidth, type);
        size[type] = 0;
        dummy = 0;
        zlib_compress(&dummy, &size[type], attempt[type], testsize, &zlibsettings);
        lodepng_free(dummy);
        /*check if this is smallest size (or if type == 0 it's the first case so always store the values)*/
        if(type == 0 || size[type] < smallest)
        {
          bestType = type;
          smallest = size[type];
        }
      }
      prevline = &in[y * linebytes];
      out[y * (linebytes + 1)] = bestType; /*the first byte of a scanline will be the filter type*/
      for(x = 0; x != linebytes; ++x) out[y * (linebytes + 1) + 1 + x] = attempt[bestType][x];
    }
    for(type = 0; type != 5; ++type) lodepng_free(attempt[type]);
  }
  else return 88; /* unknown filter strategy */

  return error;
}

static void addPaddingBits(unsigned char* out, const unsigned char* in,
                           size_t olinebits, size_t ilinebits, unsigned h)
{
  /*The opposite of the removePaddingBits function
  olinebits must be >= ilinebits*/
  unsigned y;
  size_t diff = olinebits - ilinebits;
  size_t obp = 0, ibp = 0; /*bit pointers*/
  for(y = 0; y != h; ++y)
  {
    size_t x;
    for(x = 0; x < ilinebits; ++x)
    {
      unsigned char bit = readBitFromReversedStream(&ibp, in);
      setBitOfReversedStream(&obp, out, bit);
    }
    /*obp += diff; --> no, fill in some value in the padding bits too, to avoid
    "Use of uninitialised value of size ###" warning from valgrind*/
    for(x = 0; x != diff; ++x) setBitOfReversedStream(&obp, out, 0);
  }
}

/*
in: non-interlaced image with size w*h
out: the same pixels, but re-ordered according to PNG's Adam7 interlacing, with
 no padding bits between scanlines, but between reduced images so that each
 reduced image starts at a byte.
bpp: bits per pixel
there are no padding bits, not between scanlines, not between reduced images
in has the following size in bits: w * h * bpp.
out is possibly bigger due to padding bits between reduced images
NOTE: comments about padding bits are only relevant if bpp < 8
*/
static void Adam7_interlace(unsigned char* out, const unsigned char* in, unsigned w, unsigned h, unsigned bpp)
{
  unsigned passw[7], passh[7];
  size_t filter_passstart[8], padded_passstart[8], passstart[8];
  unsigned i;

  Adam7_getpassvalues(passw, passh, filter_passstart, padded_passstart, passstart, w, h, bpp);

  if(bpp >= 8)
  {
    for(i = 0; i != 7; ++i)
    {
      unsigned x, y, b;
      size_t bytewidth = bpp / 8;
      for(y = 0; y < passh[i]; ++y)
      for(x = 0; x < passw[i]; ++x)
      {
        size_t pixelinstart = ((ADAM7_IY[i] + y * ADAM7_DY[i]) * w + ADAM7_IX[i] + x * ADAM7_DX[i]) * bytewidth;
        size_t pixeloutstart = passstart[i] + (y * passw[i] + x) * bytewidth;
        for(b = 0; b < bytewidth; ++b)
        {
          out[pixeloutstart + b] = in[pixelinstart + b];
        }
      }
    }
  }
  else /*bpp < 8: Adam7 with pixels < 8 bit is a bit trickier: with bit pointers*/
  {
    for(i = 0; i != 7; ++i)
    {
      unsigned x, y, b;
      unsigned ilinebits = bpp * passw[i];
      unsigned olinebits = bpp * w;
      size_t obp, ibp; /*bit pointers (for out and in buffer)*/
      for(y = 0; y < passh[i]; ++y)
      for(x = 0; x < passw[i]; ++x)
      {
        ibp = (ADAM7_IY[i] + y * ADAM7_DY[i]) * olinebits + (ADAM7_IX[i] + x * ADAM7_DX[i]) * bpp;
        obp = (8 * passstart[i]) + (y * ilinebits + x * bpp);
        for(b = 0; b < bpp; ++b)
        {
          unsigned char bit = readBitFromReversedStream(&ibp, in);
          setBitOfReversedStream(&obp, out, bit);
        }
      }
    }
  }
}

/*out must be buffer big enough to contain uncompressed IDAT chunk data, and in must contain the full image.
return value is error**/
static unsigned preProcessScanlines(unsigned char** out, size_t* outsize, const unsigned char* in,
                                    unsigned w, unsigned h,
                                    const LodePNGInfo* info_png, const LodePNGEncoderSettings* settings)
{
  /*
  This function converts the pure 2D image with the PNG's colortype, into filtered-padded-interlaced data. Steps:
  *) if no Adam7: 1) add padding bits (= posible extra bits per scanline if bpp < 8) 2) filter
  *) if adam7: 1) Adam7_interlace 2) 7x add padding bits 3) 7x filter
  */
  unsigned bpp = lodepng_get_bpp(&info_png->color);
  unsigned error = 0;

  if(info_png->interlace_method == 0)
  {
    *outsize = h + (h * ((w * bpp + 7) / 8)); /*image size plus an extra byte per scanline + possible padding bits*/
    *out = (unsigned char*)lodepng_malloc(*outsize);
    if(!(*out) && (*outsize)) error = 83; /*alloc fail*/

    if(!error)
    {
      /*non multiple of 8 bits per scanline, padding bits needed per scanline*/
      if(bpp < 8 && w * bpp != ((w * bpp + 7) / 8) * 8)
      {
        unsigned char* padded = (unsigned char*)lodepng_malloc(h * ((w * bpp + 7) / 8));
        if(!padded) error = 83; /*alloc fail*/
        if(!error)
        {
          addPaddingBits(padded, in, ((w * bpp + 7) / 8) * 8, w * bpp, h);
          error = filter(*out, padded, w, h, &info_png->color, settings);
        }
        lodepng_free(padded);
      }
      else
      {
        /*we can immediately filter into the out buffer, no other steps needed*/
        error = filter(*out, in, w, h, &info_png->color, settings);
      }
    }
  }
  else /*interlace_method is 1 (Adam7)*/
  {
    unsigned passw[7], passh[7];
    size_t filter_passstart[8], padded_passstart[8], passstart[8];
    unsigned char* adam7;

    Adam7_getpassvalues(passw, passh, filter_passstart, padded_passstart, passstart, w, h, bpp);

    *outsize = filter_passstart[7]; /*image size plus an extra byte per scanline + possible padding bits*/
    *out = (unsigned char*)lodepng_malloc(*outsize);
    if(!(*out)) error = 83; /*alloc fail*/

    adam7 = (unsigned char*)lodepng_malloc(passstart[7]);
    if(!adam7 && passstart[7]) error = 83; /*alloc fail*/

    if(!error)
    {
      unsigned i;

      Adam7_interlace(adam7, in, w, h, bpp);
      for(i = 0; i != 7; ++i)
      {
        if(bpp < 8)
        {
          unsigned char* padded = (unsigned char*)lodepng_malloc(padded_passstart[i + 1] - padded_passstart[i]);
          if(!padded) ERROR_BREAK(83); /*alloc fail*/
          addPaddingBits(padded, &adam7[passstart[i]],
                         ((passw[i] * bpp + 7) / 8) * 8, passw[i] * bpp, passh[i]);
          error = filter(&(*out)[filter_passstart[i]], padded,
                         passw[i], passh[i], &info_png->color, settings);
          lodepng_free(padded);
        }
        else
        {
          error = filter(&(*out)[filter_passstart[i]], &adam7[padded_passstart[i]],
                         passw[i], passh[i], &info_png->color, settings);
        }

        if(error) break;
      }
    }

    lodepng_free(adam7);
  }

  return error;
}

/*
palette must have 4 * palettesize bytes allocated, and given in format RGBARGBARGBARGBA...
returns 0 if the palette is opaque,
returns 1 if the palette has a single color with alpha 0 ==> color key
returns 2 if the palette is semi-translucent.
*/
static unsigned getPaletteTranslucency(const unsigned char* palette, size_t palettesize)
{
  size_t i;
  unsigned key = 0;
  unsigned r = 0, g = 0, b = 0; /*the value of the color with alpha 0, so long as color keying is possible*/
  for(i = 0; i != palettesize; ++i)
  {
    if(!key && palette[4 * i + 3] == 0)
    {
      r = palette[4 * i + 0]; g = palette[4 * i + 1]; b = palette[4 * i + 2];
      key = 1;
      i = (size_t)(-1); /*restart from beginning, to detect earlier opaque colors with key's value*/
    }
    else if(palette[4 * i + 3] != 255) return 2;
    /*when key, no opaque RGB may have key's RGB*/
    else if(key && r == palette[i * 4 + 0] && g == palette[i * 4 + 1] && b == palette[i * 4 + 2]) return 2;
  }
  return key;
}

#ifdef LODEPNG_COMPILE_ANCILLARY_CHUNKS
static unsigned addUnknownChunks(ucvector* out, unsigned char* data, size_t datasize)
{
  unsigned char* inchunk = data;
  while((size_t)(inchunk - data) < datasize)
  {
    CERROR_TRY_RETURN(lodepng_chunk_append(&out->data, &out->size, inchunk));
    out->allocsize = out->size; /*fix the allocsize again*/
    inchunk = lodepng_chunk_next(inchunk);
  }
  return 0;
}
#endif /*LODEPNG_COMPILE_ANCILLARY_CHUNKS*/

unsigned lodepng_encode(unsigned char** out, size_t* outsize,
                        const unsigned char* image, unsigned w, unsigned h,
                        LodePNGState* state)
{
  LodePNGInfo info;
  ucvector outv;
  unsigned char* data = 0; /*uncompressed version of the IDAT chunk data*/
  size_t datasize = 0;

  /*provide some proper output values if error will happen*/
  *out = 0;
  *outsize = 0;
  state->error = 0;

  lodepng_info_init(&info);
  lodepng_info_copy(&info, &state->info_png);

  if((info.color.colortype == LCT_PALETTE || state->encoder.force_palette)
      && (info.color.palettesize == 0 || info.color.palettesize > 256))
  {
    state->error = 68; /*invalid palette size, it is only allowed to be 1-256*/
    return state->error;
  }

  if(state->encoder.auto_convert)
  {
    state->error = lodepng_auto_choose_color(&info.color, image, w, h, &state->info_raw);
  }
  if(state->error) return state->error;

  if(state->encoder.zlibsettings.btype > 2)
  {
    CERROR_RETURN_ERROR(state->error, 61); /*error: unexisting btype*/
  }
  if(state->info_png.interlace_method > 1)
  {
    CERROR_RETURN_ERROR(state->error, 71); /*error: unexisting interlace mode*/
  }

  state->error = checkColorValidity(info.color.colortype, info.color.bitdepth);
  if(state->error) return state->error; /*error: unexisting color type given*/
  state->error = checkColorValidity(state->info_raw.colortype, state->info_raw.bitdepth);
  if(state->error) return state->error; /*error: unexisting color type given*/

  if(!lodepng_color_mode_equal(&state->info_raw, &info.color))
  {
    unsigned char* converted;
    size_t size = (w * h * (size_t)lodepng_get_bpp(&info.color) + 7) / 8;

    converted = (unsigned char*)lodepng_malloc(size);
    if(!converted && size) state->error = 83; /*alloc fail*/
    if(!state->error)
    {
      state->error = lodepng_convert(converted, image, &info.color, &state->info_raw, w, h);
    }
    if(!state->error) preProcessScanlines(&data, &datasize, converted, w, h, &info, &state->encoder);
    lodepng_free(converted);
  }
  else preProcessScanlines(&data, &datasize, image, w, h, &info, &state->encoder);

  ucvector_init(&outv);
  while(!state->error) /*while only executed once, to break on error*/
  {
#ifdef LODEPNG_COMPILE_ANCILLARY_CHUNKS
    size_t i;
#endif /*LODEPNG_COMPILE_ANCILLARY_CHUNKS*/
    /*write signature and chunks*/
    writeSignature(&outv);
    /*IHDR*/
    addChunk_IHDR(&outv, w, h, info.color.colortype, info.color.bitdepth, info.interlace_method);
#ifdef LODEPNG_COMPILE_ANCILLARY_CHUNKS
    /*unknown chunks between IHDR and PLTE*/
    if(info.unknown_chunks_data[0])
    {
      state->error = addUnknownChunks(&outv, info.unknown_chunks_data[0], info.unknown_chunks_size[0]);
      if(state->error) break;
    }
#endif /*LODEPNG_COMPILE_ANCILLARY_CHUNKS*/
    /*PLTE*/
    if(info.color.colortype == LCT_PALETTE)
    {
      addChunk_PLTE(&outv, &info.color);
    }
    if(state->encoder.force_palette && (info.color.colortype == LCT_RGB || info.color.colortype == LCT_RGBA))
    {
      addChunk_PLTE(&outv, &info.color);
    }
    /*tRNS*/
    if(info.color.colortype == LCT_PALETTE && getPaletteTranslucency(info.color.palette, info.color.palettesize) != 0)
    {
      addChunk_tRNS(&outv, &info.color);
    }
    if((info.color.colortype == LCT_GREY || info.color.colortype == LCT_RGB) && info.color.key_defined)
    {
      addChunk_tRNS(&outv, &info.color);
    }
#ifdef LODEPNG_COMPILE_ANCILLARY_CHUNKS
    /*bKGD (must come between PLTE and the IDAt chunks*/
    if(info.background_defined) addChunk_bKGD(&outv, &info);
    /*pHYs (must come before the IDAT chunks)*/
    if(info.phys_defined) addChunk_pHYs(&outv, &info);

    /*unknown chunks between PLTE and IDAT*/
    if(info.unknown_chunks_data[1])
    {
      state->error = addUnknownChunks(&outv, info.unknown_chunks_data[1], info.unknown_chunks_size[1]);
      if(state->error) break;
    }
#endif /*LODEPNG_COMPILE_ANCILLARY_CHUNKS*/
    /*IDAT (multiple IDAT chunks must be consecutive)*/
    state->error = addChunk_IDAT(&outv, data, datasize, &state->encoder.zlibsettings);
    if(state->error) break;
#ifdef LODEPNG_COMPILE_ANCILLARY_CHUNKS
    /*tIME*/
    if(info.time_defined) addChunk_tIME(&outv, &info.time);
    /*tEXt and/or zTXt*/
    for(i = 0; i != info.text_num; ++i)
    {
      if(strlen(info.text_keys[i]) > 79)
      {
        state->error = 66; /*text chunk too large*/
        break;
      }
      if(strlen(info.text_keys[i]) < 1)
      {
        state->error = 67; /*text chunk too small*/
        break;
      }
      if(state->encoder.text_compression)
      {
        addChunk_zTXt(&outv, info.text_keys[i], info.text_strings[i], &state->encoder.zlibsettings);
      }
      else
      {
        addChunk_tEXt(&outv, info.text_keys[i], info.text_strings[i]);
      }
    }
    /*LodePNG version id in text chunk*/
    if(state->encoder.add_id)
    {
      unsigned alread_added_id_text = 0;
      for(i = 0; i != info.text_num; ++i)
      {
        if(!strcmp(info.text_keys[i], "LodePNG"))
        {
          alread_added_id_text = 1;
          break;
        }
      }
      if(alread_added_id_text == 0)
      {
        addChunk_tEXt(&outv, "LodePNG", LODEPNG_VERSION_STRING); /*it's shorter as tEXt than as zTXt chunk*/
      }
    }
    /*iTXt*/
    for(i = 0; i != info.itext_num; ++i)
    {
      if(strlen(info.itext_keys[i]) > 79)
      {
        state->error = 66; /*text chunk too large*/
        break;
      }
      if(strlen(info.itext_keys[i]) < 1)
      {
        state->error = 67; /*text chunk too small*/
        break;
      }
      addChunk_iTXt(&outv, state->encoder.text_compression,
                    info.itext_keys[i], info.itext_langtags[i], info.itext_transkeys[i], info.itext_strings[i],
                    &state->encoder.zlibsettings);
    }

    /*unknown chunks between IDAT and IEND*/
    if(info.unknown_chunks_data[2])
    {
      state->error = addUnknownChunks(&outv, info.unknown_chunks_data[2], info.unknown_chunks_size[2]);
      if(state->error) break;
    }
#endif /*LODEPNG_COMPILE_ANCILLARY_CHUNKS*/
    addChunk_IEND(&outv);

    break; /*this isn't really a while loop; no error happened so break out now!*/
  }

  lodepng_info_cleanup(&info);
  lodepng_free(data);
  /*instead of cleaning the vector up, give it to the output*/
  *out = outv.data;
  *outsize = outv.size;

  return state->error;
}

unsigned lodepng_encode_memory(unsigned char** out, size_t* outsize, const unsigned char* image,
                               unsigned w, unsigned h, LodePNGColorType colortype, unsigned bitdepth)
{
  unsigned error;
  LodePNGState state;
  lodepng_state_init(&state);
  state.info_raw.colortype = colortype;
  state.info_raw.bitdepth = bitdepth;
  state.info_png.color.colortype = colortype;
  state.info_png.color.bitdepth = bitdepth;
  lodepng_encode(out, outsize, image, w, h, &state);
  error = state.error;
  lodepng_state_cleanup(&state);
  return error;
}

unsigned lodepng_encode32(unsigned char** out, size_t* outsize, const unsigned char* image, unsigned w, unsigned h)
{
  return lodepng_encode_memory(out, outsize, image, w, h, LCT_RGBA, 8);
}

unsigned lodepng_encode24(unsigned char** out, size_t* outsize, const unsigned char* image, unsigned w, unsigned h)
{
  return lodepng_encode_memory(out, outsize, image, w, h, LCT_RGB, 8);
}

#ifdef LODEPNG_COMPILE_DISK
unsigned lodepng_encode_file(const char* filename, const unsigned char* image, unsigned w, unsigned h,
                             LodePNGColorType colortype, unsigned bitdepth)
{
  unsigned char* buffer;
  size_t buffersize;
  unsigned error = lodepng_encode_memory(&buffer, &buffersize, image, w, h, colortype, bitdepth);
  if(!error) error = lodepng_save_file(buffer, buffersize, filename);
  lodepng_free(buffer);
  return error;
}

unsigned lodepng_encode32_file(const char* filename, const unsigned char* image, unsigned w, unsigned h)
{
  return lodepng_encode_file(filename, image, w, h, LCT_RGBA, 8);
}

unsigned lodepng_encode24_file(const char* filename, const unsigned char* image, unsigned w, unsigned h)
{
  return lodepng_encode_file(filename, image, w, h, LCT_RGB, 8);
}
#endif /*LODEPNG_COMPILE_DISK*/

void lodepng_encoder_settings_init(LodePNGEncoderSettings* settings)
{
  lodepng_compress_settings_init(&settings->zlibsettings);
  settings->filter_palette_zero = 1;
  settings->filter_strategy = LFS_MINSUM;
  settings->auto_convert = 1;
  settings->force_palette = 0;
  settings->predefined_filters = 0;
#ifdef LODEPNG_COMPILE_ANCILLARY_CHUNKS
  settings->add_id = 0;
  settings->text_compression = 1;
#endif /*LODEPNG_COMPILE_ANCILLARY_CHUNKS*/
}

#endif /*LODEPNG_COMPILE_ENCODER*/
#endif /*LODEPNG_COMPILE_PNG*/

#ifdef LODEPNG_COMPILE_ERROR_TEXT
/*
This returns the description of a numerical error code in English. This is also
the documentation of all the error codes.
*/
const char* lodepng_error_text(unsigned code)
{
  switch(code)
  {
    case 0: return "no error, everything went ok";
    case 1: return "nothing done yet"; /*the Encoder/Decoder has done nothing yet, error checking makes no sense yet*/
    case 10: return "end of input memory reached without huffman end code"; /*while huffman decoding*/
    case 11: return "error in code tree made it jump outside of huffman tree"; /*while huffman decoding*/
    case 13: return "problem while processing dynamic deflate block";
    case 14: return "problem while processing dynamic deflate block";
    case 15: return "problem while processing dynamic deflate block";
    case 16: return "unexisting code while processing dynamic deflate block";
    case 17: return "end of out buffer memory reached while inflating";
    case 18: return "invalid distance code while inflating";
    case 19: return "end of out buffer memory reached while inflating";
    case 20: return "invalid deflate block BTYPE encountered while decoding";
    case 21: return "NLEN is not ones complement of LEN in a deflate block";
     /*end of out buffer memory reached while inflating:
     This can happen if the inflated deflate data is longer than the amount of bytes required to fill up
     all the pixels of the image, given the color depth and image dimensions. Something that doesn't
     happen in a normal, well encoded, PNG image.*/
    case 22: return "end of out buffer memory reached while inflating";
    case 23: return "end of in buffer memory reached while inflating";
    case 24: return "invalid FCHECK in zlib header";
    case 25: return "invalid compression method in zlib header";
    case 26: return "FDICT encountered in zlib header while it's not used for PNG";
    case 27: return "PNG file is smaller than a PNG header";
    /*Checks the magic file header, the first 8 bytes of the PNG file*/
    case 28: return "incorrect PNG signature, it's no PNG or corrupted";
    case 29: return "first chunk is not the header chunk";
    case 30: return "chunk length too large, chunk broken off at end of file";
    case 31: return "illegal PNG color type or bpp";
    case 32: return "illegal PNG compression method";
    case 33: return "illegal PNG filter method";
    case 34: return "illegal PNG interlace method";
    case 35: return "chunk length of a chunk is too large or the chunk too small";
    case 36: return "illegal PNG filter type encountered";
    case 37: return "illegal bit depth for this color type given";
    case 38: return "the palette is too big"; /*more than 256 colors*/
    case 39: return "more palette alpha values given in tRNS chunk than there are colors in the palette";
    case 40: return "tRNS chunk has wrong size for greyscale image";
    case 41: return "tRNS chunk has wrong size for RGB image";
    case 42: return "tRNS chunk appeared while it was not allowed for this color type";
    case 43: return "bKGD chunk has wrong size for palette image";
    case 44: return "bKGD chunk has wrong size for greyscale image";
    case 45: return "bKGD chunk has wrong size for RGB image";
    case 48: return "empty input buffer given to decoder. Maybe caused by non-existing file?";
    case 49: return "jumped past memory while generating dynamic huffman tree";
    case 50: return "jumped past memory while generating dynamic huffman tree";
    case 51: return "jumped past memory while inflating huffman block";
    case 52: return "jumped past memory while inflating";
    case 53: return "size of zlib data too small";
    case 54: return "repeat symbol in tree while there was no value symbol yet";
    /*jumped past tree while generating huffman tree, this could be when the
    tree will have more leaves than symbols after generating it out of the
    given lenghts. They call this an oversubscribed dynamic bit lengths tree in zlib.*/
    case 55: return "jumped past tree while generating huffman tree";
    case 56: return "given output image colortype or bitdepth not supported for color conversion";
    case 57: return "invalid CRC encountered (checking CRC can be disabled)";
    case 58: return "invalid ADLER32 encountered (checking ADLER32 can be disabled)";
    case 59: return "requested color conversion not supported";
    case 60: return "invalid window size given in the settings of the encoder (must be 0-32768)";
    case 61: return "invalid BTYPE given in the settings of the encoder (only 0, 1 and 2 are allowed)";
    /*LodePNG leaves the choice of RGB to greyscale conversion formula to the user.*/
    case 62: return "conversion from color to greyscale not supported";
    case 63: return "length of a chunk too long, max allowed for PNG is 2147483647 bytes per chunk"; /*(2^31-1)*/
    /*this would result in the inability of a deflated block to ever contain an end code. It must be at least 1.*/
    case 64: return "the length of the END symbol 256 in the Huffman tree is 0";
    case 66: return "the length of a text chunk keyword given to the encoder is longer than the maximum of 79 bytes";
    case 67: return "the length of a text chunk keyword given to the encoder is smaller than the minimum of 1 byte";
    case 68: return "tried to encode a PLTE chunk with a palette that has less than 1 or more than 256 colors";
    case 69: return "unknown chunk type with 'critical' flag encountered by the decoder";
    case 71: return "unexisting interlace mode given to encoder (must be 0 or 1)";
    case 72: return "while decoding, unexisting compression method encountering in zTXt or iTXt chunk (it must be 0)";
    case 73: return "invalid tIME chunk size";
    case 74: return "invalid pHYs chunk size";
    /*length could be wrong, or data chopped off*/
    case 75: return "no null termination char found while decoding text chunk";
    case 76: return "iTXt chunk too short to contain required bytes";
    case 77: return "integer overflow in buffer size";
    case 78: return "failed to open file for reading"; /*file doesn't exist or couldn't be opened for reading*/
    case 79: return "failed to open file for writing";
    case 80: return "tried creating a tree of 0 symbols";
    case 81: return "lazy matching at pos 0 is impossible";
    case 82: return "color conversion to palette requested while a color isn't in palette";
    case 83: return "memory allocation failed";
    case 84: return "given image too small to contain all pixels to be encoded";
    case 86: return "impossible offset in lz77 encoding (internal bug)";
    case 87: return "must provide custom zlib function pointer if LODEPNG_COMPILE_ZLIB is not defined";
    case 88: return "invalid filter strategy given for LodePNGEncoderSettings.filter_strategy";
    case 89: return "text chunk keyword too short or long: must have size 1-79";
    /*the windowsize in the LodePNGCompressSettings. Requiring POT(==> & instead of %) makes encoding 12% faster.*/
    case 90: return "windowsize must be a power of two";
    case 91: return "invalid decompressed idat size";
    case 92: return "too many pixels, not supported";
    case 93: return "zero width or height is invalid";
    case 94: return "header chunk must have a size of 13 bytes";
  }
  return "unknown error code";
}
#endif /*LODEPNG_COMPILE_ERROR_TEXT*/

/* ////////////////////////////////////////////////////////////////////////// */
/* ////////////////////////////////////////////////////////////////////////// */
/* // C++ Wrapper                                                          // */
/* ////////////////////////////////////////////////////////////////////////// */
/* ////////////////////////////////////////////////////////////////////////// */

#ifdef LODEPNG_COMPILE_CPP
namespace lodepng
{

#ifdef LODEPNG_COMPILE_DISK
unsigned load_file(std::vector<unsigned char>& buffer, const std::string& filename)
{
  long size = lodepng_filesize(filename.c_str());
  if(size < 0) return 78;
  buffer.resize((size_t)size);
  return size == 0 ? 0 : lodepng_buffer_file(&buffer[0], (size_t)size, filename.c_str());
}

/*write given buffer to the file, overwriting the file, it doesn't append to it.*/
unsigned save_file(const std::vector<unsigned char>& buffer, const std::string& filename)
{
  return lodepng_save_file(buffer.empty() ? 0 : &buffer[0], buffer.size(), filename.c_str());
}
#endif /* LODEPNG_COMPILE_DISK */

#ifdef LODEPNG_COMPILE_ZLIB
#ifdef LODEPNG_COMPILE_DECODER
unsigned decompress(std::vector<unsigned char>& out, const unsigned char* in, size_t insize,
                    const LodePNGDecompressSettings& settings)
{
  unsigned char* buffer = 0;
  size_t buffersize = 0;
  unsigned error = zlib_decompress(&buffer, &buffersize, in, insize, &settings);
  if(buffer)
  {
    out.insert(out.end(), &buffer[0], &buffer[buffersize]);
    lodepng_free(buffer);
  }
  return error;
}

unsigned decompress(std::vector<unsigned char>& out, const std::vector<unsigned char>& in,
                    const LodePNGDecompressSettings& settings)
{
  return decompress(out, in.empty() ? 0 : &in[0], in.size(), settings);
}
#endif /* LODEPNG_COMPILE_DECODER */

#ifdef LODEPNG_COMPILE_ENCODER
unsigned compress(std::vector<unsigned char>& out, const unsigned char* in, size_t insize,
                  const LodePNGCompressSettings& settings)
{
  unsigned char* buffer = 0;
  size_t buffersize = 0;
  unsigned error = zlib_compress(&buffer, &buffersize, in, insize, &settings);
  if(buffer)
  {
    out.insert(out.end(), &buffer[0], &buffer[buffersize]);
    lodepng_free(buffer);
  }
  return error;
}

unsigned compress(std::vector<unsigned char>& out, const std::vector<unsigned char>& in,
                  const LodePNGCompressSettings& settings)
{
  return compress(out, in.empty() ? 0 : &in[0], in.size(), settings);
}
#endif /* LODEPNG_COMPILE_ENCODER */
#endif /* LODEPNG_COMPILE_ZLIB */


#ifdef LODEPNG_COMPILE_PNG

State::State()
{
  lodepng_state_init(this);
}

State::State(const State& other)
{
  lodepng_state_init(this);
  lodepng_state_copy(this, &other);
}

State::~State()
{
  lodepng_state_cleanup(this);
}

State& State::operator=(const State& other)
{
  lodepng_state_copy(this, &other);
  return *this;
}

#ifdef LODEPNG_COMPILE_DECODER

unsigned decode(std::vector<unsigned char>& out, unsigned& w, unsigned& h, const unsigned char* in,
                size_t insize, LodePNGColorType colortype, unsigned bitdepth)
{
  unsigned char* buffer;
  unsigned error = lodepng_decode_memory(&buffer, &w, &h, in, insize, colortype, bitdepth);
  if(buffer && !error)
  {
    State state;
    state.info_raw.colortype = colortype;
    state.info_raw.bitdepth = bitdepth;
    size_t buffersize = lodepng_get_raw_size(w, h, &state.info_raw);
    out.insert(out.end(), &buffer[0], &buffer[buffersize]);
    lodepng_free(buffer);
  }
  return error;
}

unsigned decode(std::vector<unsigned char>& out, unsigned& w, unsigned& h,
                const std::vector<unsigned char>& in, LodePNGColorType colortype, unsigned bitdepth)
{
  return decode(out, w, h, in.empty() ? 0 : &in[0], (unsigned)in.size(), colortype, bitdepth);
}

unsigned decode(std::vector<unsigned char>& out, unsigned& w, unsigned& h,
                State& state,
                const unsigned char* in, size_t insize)
{
  unsigned char* buffer = NULL;
  unsigned error = lodepng_decode(&buffer, &w, &h, &state, in, insize);
  if(buffer && !error)
  {
    size_t buffersize = lodepng_get_raw_size(w, h, &state.info_raw);
    out.insert(out.end(), &buffer[0], &buffer[buffersize]);
  }
  lodepng_free(buffer);
  return error;
}

unsigned decode(std::vector<unsigned char>& out, unsigned& w, unsigned& h,
                State& state,
                const std::vector<unsigned char>& in)
{
  return decode(out, w, h, state, in.empty() ? 0 : &in[0], in.size());
}

#ifdef LODEPNG_COMPILE_DISK
unsigned decode(std::vector<unsigned char>& out, unsigned& w, unsigned& h, const std::string& filename,
                LodePNGColorType colortype, unsigned bitdepth)
{
  std::vector<unsigned char> buffer;
  unsigned error = load_file(buffer, filename);
  if(error) return error;
  return decode(out, w, h, buffer, colortype, bitdepth);
}
#endif /* LODEPNG_COMPILE_DECODER */
#endif /* LODEPNG_COMPILE_DISK */

#ifdef LODEPNG_COMPILE_ENCODER
unsigned encode(std::vector<unsigned char>& out, const unsigned char* in, unsigned w, unsigned h,
                LodePNGColorType colortype, unsigned bitdepth)
{
  unsigned char* buffer;
  size_t buffersize;
  unsigned error = lodepng_encode_memory(&buffer, &buffersize, in, w, h, colortype, bitdepth);
  if(buffer)
  {
    out.insert(out.end(), &buffer[0], &buffer[buffersize]);
    lodepng_free(buffer);
  }
  return error;
}

unsigned encode(std::vector<unsigned char>& out,
                const std::vector<unsigned char>& in, unsigned w, unsigned h,
                LodePNGColorType colortype, unsigned bitdepth)
{
  if(lodepng_get_raw_size_lct(w, h, colortype, bitdepth) > in.size()) return 84;
  return encode(out, in.empty() ? 0 : &in[0], w, h, colortype, bitdepth);
}

unsigned encode(std::vector<unsigned char>& out,
                const unsigned char* in, unsigned w, unsigned h,
                State& state)
{
  unsigned char* buffer;
  size_t buffersize;
  unsigned error = lodepng_encode(&buffer, &buffersize, in, w, h, &state);
  if(buffer)
  {
    out.insert(out.end(), &buffer[0], &buffer[buffersize]);
    lodepng_free(buffer);
  }
  return error;
}

unsigned encode(std::vector<unsigned char>& out,
                const std::vector<unsigned char>& in, unsigned w, unsigned h,
                State& state)
{
  if(lodepng_get_raw_size(w, h, &state.info_raw) > in.size()) return 84;
  return encode(out, in.empty() ? 0 : &in[0], w, h, state);
}

#ifdef LODEPNG_COMPILE_DISK
unsigned encode(const std::string& filename,
                const unsigned char* in, unsigned w, unsigned h,
                LodePNGColorType colortype, unsigned bitdepth)
{
  std::vector<unsigned char> buffer;
  unsigned error = encode(buffer, in, w, h, colortype, bitdepth);
  if(!error) error = save_file(buffer, filename);
  return error;
}

unsigned encode(const std::string& filename,
                const std::vector<unsigned char>& in, unsigned w, unsigned h,
                LodePNGColorType colortype, unsigned bitdepth)
{
  if(lodepng_get_raw_size_lct(w, h, colortype, bitdepth) > in.size()) return 84;
  return encode(filename, in.empty() ? 0 : &in[0], w, h, colortype, bitdepth);
}
#endif /* LODEPNG_COMPILE_DISK */
#endif /* LODEPNG_COMPILE_ENCODER */
#endif /* LODEPNG_COMPILE_PNG */
} /* namespace lodepng */
#endif /*LODEPNG_COMPILE_CPP*/
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

#ifndef AMALGAMATION
  #include "vector.h"
  #include "palloc.h"
#endif

#include <string.h>

void *_VectorNew(size_t size, char *type)
{
  struct _Vector *rtn = NULL;
  char typeStr[256] = {0};

  strcpy(typeStr, "vector(");
  strcat(typeStr, type);
  strcat(typeStr, ")");
  rtn = _palloc(sizeof(*rtn), typeStr);

  strcpy(typeStr, "vector_header(");
  strcat(typeStr, type);
  strcat(typeStr, ")");
  rtn->vh = _palloc(sizeof(*rtn->vh), typeStr);
  rtn->vh->entrySize = size;

  return rtn;
}

int _VectorOobAssert(void *_vh, size_t idx)
{
  struct _VectorHeader *vh = _vh;

  if(vh->size <= idx)
  {
    printf("Error: Index out of bounds\n");
    return 1;
  }

  return 0;
}

void _VectorResize(void *_vh, void *_v, size_t size)
{
  struct _Vector *v = _v;
  struct _VectorHeader *vh = _vh;

  if(vh->size == size)
  {
    return;
  }
  else if(size == 0)
  {
    if(v->data)
    {
      free(v->data);
      v->data = NULL;
    }
  }
  else if(vh->size > size)
  {
    v->data = realloc(v->data, size * vh->entrySize);

    if(!v->data)
    {
      /* TODO: Should cache and revert */
      printf("Error: Failed to reallocate\n");
    }
  }
  else if(vh->size < size)
  {
    char *cur = NULL;
    char *last = NULL;

    v->data = realloc(v->data, size * vh->entrySize);

    if(!v->data)
    {
      /* TODO: Should cache and revert */
      printf("Error: Failed to reallocate\n");
    }

    cur = v->data;
    cur += vh->size * vh->entrySize;
    last = v->data;
    last += size * vh->entrySize;

    while(cur != last)
    {
      *cur = 0;
      cur++;
    }
  }

  vh->size = size;
}

size_t _VectorSize(void *_vh)
{
  struct _VectorHeader *vh = _vh;

  return vh->size;
}

void _VectorDelete(void *_vh, void *_v)
{
  struct _Vector *v = _v;
  struct _VectorHeader *vh = _vh;

  if(v->vh != vh)
  {
    printf("Error: Invalid vector\n");
  }

  if(v->data)
  {
    free(v->data);
  }

  pfree(vh);
  pfree(v);
}

