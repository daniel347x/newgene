#include "inputprojectworkqueue.h"
#include "uiinputproject.h"
#include "Widgets/newgenemainwindow.h"
#include <QTimer>
#include "../UIData/uiwidgetdatarefresh.h"
#include <QStandardItem>

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

// **********************************************************************************************************//
// Called in context of Boost WORK POOL threads - NOT in context of this work queue manager's event loop thread
// **********************************************************************************************************//
void InputProjectWorkQueue::HandleChanges(DataChangeMessage & changes)
{
	 WidgetChangeMessages widget_change_messages = get()->HandleChanges(changes);
	 if (!widget_change_messages.empty())
	 {
		 emit DataChangeMessageSignal(widget_change_messages);
	 }
}


/************************************************************************/
//
/************************************************************************/
void InputProjectWorkQueue::RefreshWidget(WidgetDataItemRequest_MANAGE_DMUS_WIDGET widget)
{
	get()->getWorkService().post(DoRefreshInputWidget<MANAGE_DMUS_WIDGET>(widget, this));
}




void InputProjectWorkQueue::AddDMU(WidgetActionItemRequest_ACTION_ADD_DMU action_request)
{
	get()->getWorkService().post(AddDMU_(action_request, this));
}

void InputProjectWorkQueue::DeleteDMU(WidgetActionItemRequest_ACTION_DELETE_DMU action_request)
{
	get()->getWorkService().post(DeleteDMU_(action_request, this));
}

void InputProjectWorkQueue::AddDMUMembers(WidgetActionItemRequest_ACTION_ADD_DMU_MEMBERS action_request)
{
	get()->getWorkService().post(AddDMUMembers_(action_request, this));
}

void InputProjectWorkQueue::DeleteDMUMembers(WidgetActionItemRequest_ACTION_DELETE_DMU_MEMBERS action_request)
{
	get()->getWorkService().post(DeleteDMUMembers_(action_request, this));
}

void InputProjectWorkQueue::RefreshDMUsFromFile(WidgetActionItemRequest_ACTION_REFRESH_DMUS_FROM_FILE action_request)
{
	get()->getWorkService().post(RefreshDMUsFromFile_(action_request, this));
}
