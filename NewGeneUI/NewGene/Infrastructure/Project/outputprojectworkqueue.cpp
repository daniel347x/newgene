#include "outputprojectworkqueue.h"
#include "uioutputproject.h"
#include "Widgets/newgenemainwindow.h"
#include <QTimer>
#include "../UIData/uiwidgetdatarefresh.h"
#include "../UIAction/variablegroupsetmemberselectionchange.h"
#include <QStandardItem>

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

// Called in context of Boost WORK POOL threads - NOT in context of this work queue manager's event loop thread
void OutputProjectWorkQueue::HandleChanges(DataChangeMessage & changes)
{
	WidgetChangeMessages widget_change_messages = get()->HandleChanges(changes);
	if (!widget_change_messages.empty())
	{
		emit DataChangeMessageSignal(widget_change_messages);
	}
}



// Refreshes

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

/************************************************************************/
// VARIABLE_GROUP_VARIABLE_GROUP_INSTANCE
/************************************************************************/
void OutputProjectWorkQueue::RefreshWidget(WidgetDataItemRequest_VARIABLE_GROUP_VARIABLE_GROUP_INSTANCE widget)
{
	get()->getWorkService().post(DoRefreshOutputWidget<VARIABLE_GROUP_VARIABLE_GROUP_INSTANCE>(widget, this));
}

/************************************************************************/
// VARIABLE_GROUPS_SUMMARY
/************************************************************************/
void OutputProjectWorkQueue::RefreshWidget(WidgetDataItemRequest_VARIABLE_GROUPS_SUMMARY_SCROLL_AREA widget)
{
	get()->getWorkService().post(DoRefreshOutputWidget<VARIABLE_GROUPS_SUMMARY>(widget, this));
}

/************************************************************************/
// VARIABLE_GROUPS_SUMMARY_VARIABLE_GROUP_INSTANCE
/************************************************************************/
void OutputProjectWorkQueue::RefreshWidget(WidgetDataItemRequest_VARIABLE_GROUPS_SUMMARY_VARIABLE_GROUP_INSTANCE widget)
{
	get()->getWorkService().post(DoRefreshOutputWidget<VARIABLE_GROUPS_SUMMARY_VARIABLE_GROUP_INSTANCE>(widget, this));
}



// Actions

/************************************************************************/
// ACTION_VARIABLE_GROUP_SET_MEMBER_SELECTION_CHANGED
/************************************************************************/
void OutputProjectWorkQueue::ReceiveVariableItemChanged(WidgetActionItemRequest_ACTION_VARIABLE_GROUP_SET_MEMBER_SELECTION_CHANGED action_request)
{
	get()->getWorkService().post(VariableGroupSetMemberSelectionChange(action_request, this));
}
