#include "outputprojectworkqueue.h"
#include "uioutputproject.h"
#include "Widgets/newgenemainwindow.h"
#include <QTimer>
#include "../UIData/uiwidgetdatarefresh.h"

OutputProjectWorkQueue::OutputProjectWorkQueue(QObject * parent)
	: WorkQueueManager(parent)
	, outp(nullptr)
{
}

UIOutputProject * OutputProjectWorkQueue::get()
{
	return reinterpret_cast<UIOutputProject*>(outp);
}

void OutputProjectWorkQueue::TestSlot()
{
	emit SignalMessageBox("Output project's \"TestSlot()\" successfully called and handled.");
}

void OutputProjectWorkQueue::SetConnections()
{
	connect(this, SIGNAL(SignalMessageBox(STD_STRING)), get(), SLOT(SignalMessageBox(STD_STRING)));
}

void OutputProjectWorkQueue::EmitMessage(std::string msg)
{
	emit SignalMessageBox(msg.c_str());
}


/************************************************************************/
// VARIABLE_GROUPS_SCROLL_AREA
/************************************************************************/
void OutputProjectWorkQueue::RefreshWidget(WidgetDataItemRequest_VARIABLE_GROUPS_SCROLL_AREA widget)
{
	get()->getWorkService().post(DoRefreshOutputWidget<VARIABLE_GROUPS_SCROLL_AREA>(widget, this));
}

/************************************************************************/
// VARIABLE_GROUPS_TOOLBOX
/************************************************************************/
void OutputProjectWorkQueue::RefreshWidget(WidgetDataItemRequest_VARIABLE_GROUPS_TOOLBOX widget)
{
	get()->getWorkService().post(DoRefreshOutputWidget<VARIABLE_GROUPS_TOOLBOX>(widget, this));
}

