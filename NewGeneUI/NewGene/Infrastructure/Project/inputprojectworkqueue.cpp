#include "inputprojectworkqueue.h"
#include "uiinputproject.h"

InputProjectWorkQueue::InputProjectWorkQueue(QObject * parent)
	: WorkQueueManager<UI_INPUT_PROJECT>(parent)
	, inp(nullptr)
{
}

UIInputProject * InputProjectWorkQueue::get()
{
	return reinterpret_cast<UIInputProject*>(inp);
}

void InputProjectWorkQueue::TestSlot()
{
	emit SignalMessageBox("Input project's \"TestSlot()\" successfully called and handled.");
}

void InputProjectWorkQueue::SetConnections()
{
	connect(this, SIGNAL(SignalMessageBox(STD_STRING)), get(), SLOT(SignalMessageBox(STD_STRING)));
}


///************************************************************************/
//// VARIABLE_GROUPS_SCROLL_AREA
///************************************************************************/
//void InputProjectWorkQueue::RefreshWidget(WidgetDataItemRequest_VARIABLE_GROUPS_SCROLL_AREA)
//{
//		get()->getWorkService().post(DoRefreshOutputWidget(widget, this));
//}

