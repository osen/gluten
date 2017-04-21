#ifndef GLUTEN_FORM_H
#define GLUTEN_FORM_H

typedef struct GnForm GnForm;
typedef struct GnObject GnEvent;
typedef struct GnObject GnWidget;

void GnFormInit(GnWidget *ctx, GnEvent *event);
void GnFormShow(GnWidget *ctx);

struct GnForm
{
  int formStuff;
};

#endif
