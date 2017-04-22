#ifndef GLUTEN_FORM_H
#define GLUTEN_FORM_H

#ifndef AMALGAMATION
  #include "Object.h"
#endif

typedef struct GnForm GnForm;

void GnFormInit(GnWidget *ctx, GnEvent *event);
void GnFormShow(GnWidget *ctx);

struct GnForm
{
  int formStuff;
};

#endif
