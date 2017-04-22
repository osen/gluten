#ifndef GLUTEN_BUTTON_H
#define GLUTEN_BUTTON_H

#ifndef AMALGAMATION
  #include "Object.h"
#endif

typedef struct GnButton GnButton;

void GnButtonInit(GnWidget *ctx, GnEvent *event);

struct GnButton
{
  int buttonStuff;
};

#endif
