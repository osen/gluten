#ifndef GLUTEN_BUTTON_H
#define GLUTEN_BUTTON_H

typedef struct GnObject GnEvent;
typedef struct GnObject GnWidget;
typedef struct GnButton GnButton;

void GnButtonInit(GnWidget *ctx, GnEvent *event);

struct GnButton
{
  int buttonStuff;
};

#endif
