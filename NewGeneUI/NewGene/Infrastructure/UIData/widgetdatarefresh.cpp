#include "widgetdatarefresh.h"
#include "../Project/inputprojectworkqueue.h"
#include "../Project/outputprojectworkqueue.h"

void DoRefreshInputWidget::operator()()
{

}

void DoRefreshOutputWidget::operator()()
{
    queue->EmitMessage("Successfully posted a message from the DoRefresh() handler.");
}
