#include "globals.h"
#include "uiwidgetdatarefresh.h"
#include "../Project/inputprojectworkqueue.h"
#include "../Project/outputprojectworkqueue.h"
#include "../Messager/uimessager.h"
#include "../Messager/uimessagersingleshot.h"
#include "../UIData/uiuidatamanager.h"
#include "uiinputproject.h"
#include "uioutputproject.h"

void DoRefreshInputWidget::operator()()
{

}

void DoRefreshOutputWidget::operator()()
{
	UIMessagerSingleShot messager(queue->get()->messager);
	uidataManagerUI().getBackendManager().DoRefreshOutputWidget(messager.get(), widget, queue->get()->backend());

	//queue->EmitMessage("Successfully posted a message from the DoRefresh() handler.");
}
