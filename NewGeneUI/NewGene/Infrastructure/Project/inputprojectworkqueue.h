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
		void EmitInputWidgetDataRefresh(WidgetDataItem_MANAGE_UOAS_WIDGET & widgetData)
		{
			emit WidgetDataRefresh(widgetData);
		}
		void EmitInputWidgetDataRefresh(WidgetDataItem_MANAGE_VGS_WIDGET & widgetData)
		{
			emit WidgetDataRefresh(widgetData);
		}
		void EmitSignalUpdateVGImportProgressBar(int mode_, int min_, int max_, int val_)
		{
			emit SignalUpdateVGImportProgressBar(mode_, min_, max_, val_);
		}
		void EmitSignalUpdateDMUImportProgressBar(int mode_, int min_, int max_, int val_)
		{
			emit SignalUpdateDMUImportProgressBar(mode_, min_, max_, val_);
		}

	private:

		void * inp;

	protected:

		// ********************************* //
		// Slot Overrides
		// ********************************* //

		void TestSlot();

		void RefreshWidget(WidgetDataItemRequest_MANAGE_DMUS_WIDGET widget);
		void RefreshWidget(WidgetDataItemRequest_MANAGE_UOAS_WIDGET widget);
		void RefreshWidget(WidgetDataItemRequest_MANAGE_VGS_WIDGET widget);

		// Actions
		void AddDMU(WidgetActionItemRequest_ACTION_ADD_DMU);
		void DeleteDMU(WidgetActionItemRequest_ACTION_DELETE_DMU);
		void AddDMUMembers(WidgetActionItemRequest_ACTION_ADD_DMU_MEMBERS);
		void DeleteDMUMembers(WidgetActionItemRequest_ACTION_DELETE_DMU_MEMBERS);
		void RefreshDMUsFromFile(WidgetActionItemRequest_ACTION_REFRESH_DMUS_FROM_FILE);
		void AddUOA(WidgetActionItemRequest_ACTION_ADD_UOA);
		void DeleteUOA(WidgetActionItemRequest_ACTION_DELETE_UOA);
		void CreateVG(WidgetActionItemRequest_ACTION_CREATE_VG);
		void DeleteVG(WidgetActionItemRequest_ACTION_DELETE_VG);
		void RefreshVG(WidgetActionItemRequest_ACTION_REFRESH_VG);

};

#endif // INPUTPROJECTWORKQUEUE_H
