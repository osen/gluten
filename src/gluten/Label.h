#ifndef GLUTEN_LABEL_H
#define GLUTEN_LABEL_H

typedef struct GnObject GnEvent;
typedef struct GnObject GnWidget;
typedef struct GnLabel GnLabel;

void GnLabelInit(GnWidget *ctx, GnEvent *event);
void GnLabelSetText(GnWidget *ctx, char *text);

struct GnLabel
{
  char *text;
};

#endif

