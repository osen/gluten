#include <gluten.h>

#include <stdlib.h>

void OnCloseButtonClicked(GnWidget *sender, GnEvent *event)
{
  printf("Exit...\n");
/*
  GnButton *button = GnWidgetDerived(sender, GnButton);
  GnClickEvent *clickEvent = GnEventDerived(event, GnClickEvent);

  GnExit();
*/
}

int main(int argc, char *argv[])
{
  GnWidget *loginForm = NULL;
  GnWidget *closeButton = NULL;
  GnWidget *testLabel = NULL;

  GnInit(argc, argv, "example.gluten");

  loginForm = GnWidgetCreate(GnForm);
  GnFormShow(loginForm);

  closeButton = GnWidgetCreate(GnButton);
  GnLabelSetText(closeButton, "Close");
  GnWidgetSetPosition(closeButton, 100, 100);
  GnContainerAdd(loginForm, closeButton);

  testLabel = GnWidgetCreate(GnLabel);
  GnLabelSetText(testLabel, "This is a test label");
  GnWidgetSetPosition(testLabel, 200, 10);
  GnWidgetSetSize(testLabel, 50, 200);
  GnContainerAdd(loginForm, testLabel);

  GnWidgetAddEvent(closeButton, "click", OnCloseButtonClicked);
  GnWidgetEvent(closeButton, "click", NULL);

/*
  closeButton = GnFindWidget("loginForm.closeButton");
  loginForm = GnFindWidget("loginForm");

  GnWidgetAddEvent(closeButton, "click", OnCloseButtonClicked);

  GnWidgetSetVisible(loginForm, true);
*/

  GnRun();

  /* GnWidgetDestroy(closeButton); */

  GnCleanup();

  return 0;
}

