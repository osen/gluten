#ifndef GLUTEN_LABEL_H
#define GLUTEN_LABEL_H

#ifndef AMALGAMATION
  #include "Object.h"
#endif

typedef struct GnLabel GnLabel;

void GnLabelInit(GnWidget *ctx, GnEvent *event);
void GnLabelSetText(GnWidget *ctx, char *text);

struct GnLabel
{
  char *text;
};

#endif

