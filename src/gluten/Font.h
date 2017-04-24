#ifndef GLUTEN_FONT_H
#define GLUTEN_FONT_H

#ifndef AMALGAMATION
  #include <vector.h>
#endif

struct GnImage;
typedef struct GnFont GnFont;
typedef struct GnGlyph GnGlyph;

/* Escaped characters " and \ */
static const char *characters = "!\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~";

struct GnGlyph
{
  char c;
  int x;
  int y;
  int width;
  int height;
  struct GnImage *img;
};

struct GnFont
{
  struct GnImage *img;
  vector(struct GnGlyph) *glyphs;
};

GnFont *GnFontCreateFromString(char *data);
void GnFontDestroy(GnFont *ctx);
GnGlyph GnFontGlyph(GnFont *ctx, char c);

#endif
