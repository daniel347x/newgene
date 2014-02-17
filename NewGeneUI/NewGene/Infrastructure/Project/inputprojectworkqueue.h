#ifndef INPUTPROJECTWORKQUEUE_H
#define INPUTPROJECTWORKQUEUE_H

#include "Base/inputprojectworkqueue_base.h"

class UIInputProject;

class InputProjectWorkQueue : public WorkQueueManager<UI_INPUT_PROJECT>
{

	public:

		explicit InputProjectWorkQueue(QObject * parent = NULL);

		void SetUIObject(void * ui_input_object_)
		{
			inp = ui_input_object_;
		}

		void SetConnections();

		UIInputProject * get();

		// Called in context of Boost WORK POOL threads - NOT in context of this work queue manager's event loop thread
		void HandleChanges(DataChangeMessage & changes);

		void EmitInputWidgetDataRefresh(WidgetDataItem_MANAGE_DMUS_WIDGET & widgetData)
		{
			emit WidgetDataRefresh(widgetData);
		}

	private:

		void * inp;

	protected:

		// ********************************* //
		// Slot Overrides
		// ********************************* //

		void TestSlot();

		void RefreshWidget(WidgetDataItemRequest_MANAGE_DMUS_WIDGET widget);

		// Actions
		void AddDMU(WidgetActionItemRequest_ACTION_ADD_DMU);
		void DeleteDMU(WidgetActionItemRequest_ACTION_DELETE_DMU);
		void AddDMUMembers(WidgetActionItemRequest_ACTION_ADD_DMU_MEMBERS);
		void DeleteDMUMembers(WidgetActionItemRequest_ACTION_DELETE_DMU_MEMBERS);
		void RefreshDMUsFromFile(WidgetActionItemRequest_ACTION_REFRESH_DMUS_FROM_FILE);

};

#endif // INPUTPROJECTWORKQUEUE_H
