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
