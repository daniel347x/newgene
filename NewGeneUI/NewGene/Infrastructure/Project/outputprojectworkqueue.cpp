#include "outputprojectworkqueue.h"
#include "uioutputproject.h"
#include "Widgets/newgenemainwindow.h"
#include <QTimer>
#include "../UIData/uiwidgetdatarefresh.h"
#include "../UIAction/generaloptions.h"
#include "../UIAction/variablegroupsetmemberselectionchange.h"
#include "../UIAction/KAdCountChange.h"
#include "../UIAction/TimeRangeChange.h"
#include "../UIAction/GenerateOutput.h"
#include "../UIAction/dmumanagement.h"
#include "../UIAction/uoamanagement.h"
#include "../UIAction/vgmanagement.h"
#include "../UIAction/limitdmus.h"
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

// **********************************************************************************************************//
// Called in context of Boost WORK POOL threads - NOT in context of this work queue manager's event loop thread
// **********************************************************************************************************//
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
// VARIABLE_GROUPS_SUMMARY_SCROLL_AREA
/************************************************************************/
void OutputProjectWorkQueue::RefreshWidget(WidgetDataItemRequest_VARIABLE_GROUPS_SUMMARY_SCROLL_AREA widget)
{
	get()->getWorkService().post(DoRefreshOutputWidget<VARIABLE_GROUPS_SUMMARY_SCROLL_AREA>(widget, this));
}

/************************************************************************/
// VARIABLE_GROUPS_SUMMARY_VARIABLE_GROUP_INSTANCE
/************************************************************************/
void OutputProjectWorkQueue::RefreshWidget(WidgetDataItemRequest_VARIABLE_GROUPS_SUMMARY_VARIABLE_GROUP_INSTANCE widget)
{
	get()->getWorkService().post(DoRefreshOutputWidget<VARIABLE_GROUPS_SUMMARY_VARIABLE_GROUP_INSTANCE>(widget, this));
}

/************************************************************************/
// KAD_SPIN_CONTROLS_AREA
/************************************************************************/
void OutputProjectWorkQueue::RefreshWidget(WidgetDataItemRequest_KAD_SPIN_CONTROLS_AREA widget)
{
	get()->getWorkService().post(DoRefreshOutputWidget<KAD_SPIN_CONTROLS_AREA>(widget, this));
}

/************************************************************************/
// KAD_SPIN_CONTROL_WIDGET
/************************************************************************/
void OutputProjectWorkQueue::RefreshWidget(WidgetDataItemRequest_KAD_SPIN_CONTROL_WIDGET widget)
{
	get()->getWorkService().post(DoRefreshOutputWidget<KAD_SPIN_CONTROL_WIDGET>(widget, this));
}

/************************************************************************/
// TIMERANGE_REGION_WIDGET
/************************************************************************/
void OutputProjectWorkQueue::RefreshWidget(WidgetDataItemRequest_TIMERANGE_REGION_WIDGET widget)
{
	get()->getWorkService().post(DoRefreshOutputWidget<TIMERANGE_REGION_WIDGET>(widget, this));
}

/************************************************************************/
// DATETIME_WIDGET
/************************************************************************/
void OutputProjectWorkQueue::RefreshWidget(WidgetDataItemRequest_DATETIME_WIDGET widget)
{
	get()->getWorkService().post(DoRefreshOutputWidget<DATETIME_WIDGET>(widget, this));
}

/************************************************************************/
// GENERATE_OUTPUT_TAB
/************************************************************************/
void OutputProjectWorkQueue::RefreshWidget(WidgetDataItemRequest_GENERATE_OUTPUT_TAB widget)
{
	get()->getWorkService().post(DoRefreshOutputWidget<GENERATE_OUTPUT_TAB>(widget, this));
}

/************************************************************************/
// LIMIT_DMUS_TAB
/************************************************************************/
void OutputProjectWorkQueue::RefreshWidget(WidgetDataItemRequest_LIMIT_DMUS_TAB widget)
{
	get()->getWorkService().post(DoRefreshOutputWidget<LIMIT_DMUS_TAB>(widget, this));
}


// Actions

/************************************************************************/
// ACTION_VARIABLE_GROUP_SET_MEMBER_SELECTION_CHANGED
/************************************************************************/
void OutputProjectWorkQueue::ReceiveVariableItemChanged(WidgetActionItemRequest_ACTION_VARIABLE_GROUP_SET_MEMBER_SELECTION_CHANGED action_request)
{
	get()->getWorkService().post(VariableGroupSetMemberSelectionChange(action_request, this));
}

/************************************************************************/
// ACTION_KAD_COUNT_CHANGE
/************************************************************************/
void OutputProjectWorkQueue::ReceiveVariableItemChanged(WidgetActionItemRequest_ACTION_KAD_COUNT_CHANGE action_request)
{
	get()->getWorkService().post(KAdCountChange(action_request, this));
}

/************************************************************************/
// ACTION_DO_RANDOM_SAMPLING_CHANGE
/************************************************************************/
void OutputProjectWorkQueue::ReceiveVariableItemChanged(WidgetActionItemRequest_ACTION_DO_RANDOM_SAMPLING_CHANGE action_request)
{
	get()->getWorkService().post(DoRandomSamplingChange(action_request, this));
}

/************************************************************************/
// ACTION_RANDOM_SAMPLING_COUNT_PER_STAGE_CHANGE
/************************************************************************/
void OutputProjectWorkQueue::ReceiveVariableItemChanged(WidgetActionItemRequest_ACTION_RANDOM_SAMPLING_COUNT_PER_STAGE_CHANGE action_request)
{
	get()->getWorkService().post(KadSamplerCountPerStageChange(action_request, this));
}

/************************************************************************/
// ACTION_CONSOLIDATE_ROWS_CHANGE
/************************************************************************/
void OutputProjectWorkQueue::ReceiveVariableItemChanged(WidgetActionItemRequest_ACTION_CONSOLIDATE_ROWS_CHANGE action_request)
{
	get()->getWorkService().post(DoConsolidateRowsChange(action_request, this));
}

/************************************************************************/
// ACTION_DISPLAY_ABSOLUTE_TIME_COLUMNS_CHANGE
/************************************************************************/
void OutputProjectWorkQueue::ReceiveVariableItemChanged(WidgetActionItemRequest_ACTION_DISPLAY_ABSOLUTE_TIME_COLUMNS_CHANGE action_request)
{
	get()->getWorkService().post(DoDisplayAbsoluteTimeColumnsChange(action_request, this));
}

/************************************************************************/
// ACTION_DATETIME_RANGE_CHANGE
/************************************************************************/
void OutputProjectWorkQueue::ReceiveVariableItemChanged(WidgetActionItemRequest_ACTION_DATETIME_RANGE_CHANGE action_request)
{
	get()->getWorkService().post(TimeRangeChange(action_request, this));
}

/************************************************************************/
// ACTION_GENERATE_OUTPUT
/************************************************************************/
void OutputProjectWorkQueue::ReceiveVariableItemChanged(WidgetActionItemRequest_ACTION_GENERATE_OUTPUT action_request)
{
	get()->getWorkService().post(GenerateOutput(action_request, this));
}

/************************************************************************/
// ACTION_LIMIT_DMU_MEMBERS_CHANGE
/************************************************************************/
void OutputProjectWorkQueue::LimitDMUsChange(WidgetActionItemRequest_ACTION_LIMIT_DMU_MEMBERS_CHANGE action_request)
{
	get()->getWorkService().post(LimitDMUs(action_request, this));
}

/************************************************************************/
// ACTION_DELETE_DMU
/************************************************************************/
void OutputProjectWorkQueue::DeleteDMU(WidgetActionItemRequest_ACTION_DELETE_DMU action_request)
{
	get()->getWorkService().post(DeleteDMU_Output(action_request, this));
}

/************************************************************************/
// ACTION_DELETE_DMU_MEMBERS
/************************************************************************/
void OutputProjectWorkQueue::DeleteDMUMembers(WidgetActionItemRequest_ACTION_DELETE_DMU_MEMBERS action_request)
{
	get()->getWorkService().post(DeleteDMUMembers_Output(action_request, this));
}

/************************************************************************/
// ACTION_DELETE_UOA
/************************************************************************/
void OutputProjectWorkQueue::DeleteUOA(WidgetActionItemRequest_ACTION_DELETE_UOA action_request)
{
	get()->getWorkService().post(DeleteUOA_Output(action_request, this));
}

/************************************************************************/
// ACTION_DELETE_VG
/************************************************************************/
void OutputProjectWorkQueue::DeleteVG(WidgetActionItemRequest_ACTION_DELETE_VG action_request)
{
	get()->getWorkService().post(DeleteVG_Output(action_request, this));
}
