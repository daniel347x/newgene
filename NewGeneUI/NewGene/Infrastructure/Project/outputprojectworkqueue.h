#ifndef OUTPUTPROJECTWORKQUEUE_H
#define OUTPUTPROJECTWORKQUEUE_H

#include <QListWidgetItem>

#include "Base/outputprojectworkqueue_base.h"

class UIOutputProject;

class OutputProjectWorkQueue : public WorkQueueManager<UI_OUTPUT_PROJECT>
{

	public:

		explicit OutputProjectWorkQueue(QObject * parent = NULL);

		void SetUIObject(void * ui_output_object_)
		{
			outp = ui_output_object_;
		}

		void SetConnections();

		UIOutputProject * get();

		// Called in context of Boost WORK POOL threads - NOT in context of this work queue manager's event loop thread
		void HandleChanges(DataChangeMessage & changes);

		void EmitMessage(std::string msg);

		void EmitOutputWidgetDataRefresh(WidgetDataItem_VARIABLE_GROUPS_SCROLL_AREA & widgetData)
		{
			emit WidgetDataRefresh(widgetData);
		}
		void EmitOutputWidgetDataRefresh(WidgetDataItem_VARIABLE_GROUPS_TOOLBOX & widgetData)
		{
			emit WidgetDataRefresh(widgetData);
		}
		void EmitOutputWidgetDataRefresh(WidgetDataItem_VARIABLE_GROUP_VARIABLE_GROUP_INSTANCE & widgetData)
		{
			emit WidgetDataRefresh(widgetData);
		}
		void EmitOutputWidgetDataRefresh(WidgetDataItem_VARIABLE_GROUPS_SUMMARY_SCROLL_AREA & widgetData)
		{
			emit WidgetDataRefresh(widgetData);
		}
		void EmitOutputWidgetDataRefresh(WidgetDataItem_VARIABLE_GROUPS_SUMMARY_VARIABLE_GROUP_INSTANCE & widgetData)
		{
			emit WidgetDataRefresh(widgetData);
		}
		void EmitOutputWidgetDataRefresh(WidgetDataItem_KAD_SPIN_CONTROLS_AREA & widgetData)
		{
			emit WidgetDataRefresh(widgetData);
		}
		void EmitOutputWidgetDataRefresh(WidgetDataItem_KAD_SPIN_CONTROL_WIDGET & widgetData)
		{
			emit WidgetDataRefresh(widgetData);
		}

	private:

		void * outp;

	protected:

	// ********************************* //
	// Slot Overrides
	// ********************************* //

		void TestSlot();

		// Data
		void RefreshWidget(WidgetDataItemRequest_VARIABLE_GROUPS_SCROLL_AREA widget);
		void RefreshWidget(WidgetDataItemRequest_VARIABLE_GROUPS_TOOLBOX widget);
		void RefreshWidget(WidgetDataItemRequest_VARIABLE_GROUP_VARIABLE_GROUP_INSTANCE widget);
		void RefreshWidget(WidgetDataItemRequest_VARIABLE_GROUPS_SUMMARY_SCROLL_AREA widget);
		void RefreshWidget(WidgetDataItemRequest_VARIABLE_GROUPS_SUMMARY_VARIABLE_GROUP_INSTANCE widget);
		void RefreshWidget(WidgetDataItemRequest_KAD_SPIN_CONTROLS_AREA widget);
		void RefreshWidget(WidgetDataItemRequest_KAD_SPIN_CONTROL_WIDGET widget);

		// Actions
		void ReceiveVariableItemChanged(WidgetActionItemRequest_ACTION_VARIABLE_GROUP_SET_MEMBER_SELECTION_CHANGED);

};

#endif // OUTPUTPROJECTWORKQUEUE_H
