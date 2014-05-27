#ifndef WORKQUEUEMANAGER_H
#define WORKQUEUEMANAGER_H

#include "globals.h"
#include <QObject>
#include "../../../../NewGeneBackEnd/UIData/DataWidgets.h"
#include "uimodelmanager.h"
#include "../../../Widgets/newgenewidget.h"
#include "../../../q_declare_metatype.h"

enum WORK_QUEUE_THREAD_LOOP_CLASS_ENUM
{
	  UI_INPUT_PROJECT
	, UI_OUTPUT_PROJECT
	, UI_INPUT_PROJECT_SETTINGS
	, UI_OUTPUT_PROJECT_SETTINGS
	, UI_INPUT_MODEL_SETTINGS
	, UI_OUTPUT_MODEL_SETTINGS
	, UI_INPUT_MODEL
	, UI_OUTPUT_MODEL
	, UI_GLOBAL_SETTINGS
	, UI_PROJECT_MANAGER
};

class QListWidgetItem;

class WorkQueueManagerBase : public QObject
{

		Q_OBJECT

	public:

		explicit WorkQueueManagerBase(bool isPool2_ = false, QObject *parent = 0);

		// **********************************************************************************************************//
		// Called in context of Boost WORK POOL threads - NOT in context of this work queue manager's event loop thread
		// **********************************************************************************************************//
		virtual void HandleChanges(DataChangeMessage &) {}

		// Output-project related refreshes
		virtual void EmitOutputWidgetDataRefresh(WidgetDataItem_VARIABLE_GROUPS_SCROLL_AREA &) {}
		virtual void EmitOutputWidgetDataRefresh(WidgetDataItem_VARIABLE_GROUPS_TOOLBOX &) {}
		virtual void EmitOutputWidgetDataRefresh(WidgetDataItem_VARIABLE_GROUP_VARIABLE_GROUP_INSTANCE &) {}
		virtual void EmitOutputWidgetDataRefresh(WidgetDataItem_VARIABLE_GROUPS_SUMMARY_SCROLL_AREA &) {}
		virtual void EmitOutputWidgetDataRefresh(WidgetDataItem_VARIABLE_GROUPS_SUMMARY_VARIABLE_GROUP_INSTANCE &) {}
		virtual void EmitOutputWidgetDataRefresh(WidgetDataItem_KAD_SPIN_CONTROLS_AREA &) {}
		virtual void EmitOutputWidgetDataRefresh(WidgetDataItem_KAD_SPIN_CONTROL_WIDGET &) {}
		virtual void EmitOutputWidgetDataRefresh(WidgetDataItem_TIMERANGE_REGION_WIDGET &) {}
		virtual void EmitOutputWidgetDataRefresh(WidgetDataItem_DATETIME_WIDGET &) {}
		virtual void EmitOutputWidgetDataRefresh(WidgetDataItem_GENERATE_OUTPUT_TAB &) {}
		virtual void EmitOutputWidgetDataRefresh(WidgetDataItem_LIMIT_DMUS_TAB &) {}

		// Input-project related refreshes
		virtual void EmitInputWidgetDataRefresh(WidgetDataItem_MANAGE_DMUS_WIDGET &) {}
		virtual void EmitInputWidgetDataRefresh(WidgetDataItem_MANAGE_UOAS_WIDGET &) {}
		virtual void EmitInputWidgetDataRefresh(WidgetDataItem_MANAGE_VGS_WIDGET &) {}

	signals:

		// Signals for ALL specializations of WorkQueueManager go here,
		// and are NOT (and don't need to be) defined as virtual
		void SignalMessageBox(STD_STRING);
		void SignalUpdateVGImportProgressBar(int, int, int, int);
		void SignalUpdateDMUImportProgressBar(int, int, int, int);
		void DoneLoadingFromDatabase(UI_INPUT_MODEL_PTR, QObject *);
		void DoneLoadingFromDatabase(UI_OUTPUT_MODEL_PTR, QObject *);
		void DataChangeMessageSignal(WidgetChangeMessages);

		// Output-project related refreshes
		void WidgetDataRefresh(WidgetDataItem_VARIABLE_GROUPS_SCROLL_AREA);
		void WidgetDataRefresh(WidgetDataItem_VARIABLE_GROUPS_TOOLBOX);
		void WidgetDataRefresh(WidgetDataItem_VARIABLE_GROUP_VARIABLE_GROUP_INSTANCE);
		void WidgetDataRefresh(WidgetDataItem_VARIABLE_GROUPS_SUMMARY_SCROLL_AREA);
		void WidgetDataRefresh(WidgetDataItem_VARIABLE_GROUPS_SUMMARY_VARIABLE_GROUP_INSTANCE);
		void WidgetDataRefresh(WidgetDataItem_KAD_SPIN_CONTROLS_AREA);
		void WidgetDataRefresh(WidgetDataItem_KAD_SPIN_CONTROL_WIDGET);
		void WidgetDataRefresh(WidgetDataItem_TIMERANGE_REGION_WIDGET);
		void WidgetDataRefresh(WidgetDataItem_DATETIME_WIDGET);
		void WidgetDataRefresh(WidgetDataItem_GENERATE_OUTPUT_TAB);
		void WidgetDataRefresh(WidgetDataItem_LIMIT_DMUS_TAB);

		// Input-project related refreshes
		void WidgetDataRefresh(WidgetDataItem_MANAGE_DMUS_WIDGET);
		void WidgetDataRefresh(WidgetDataItem_MANAGE_UOAS_WIDGET);
		void WidgetDataRefresh(WidgetDataItem_MANAGE_VGS_WIDGET);

	public slots:
		// Slots for ALL specializations go here,
		// and are all defined as virtual so that specializations of
		// WorkQueueManager can override them
		//
		virtual void TestSlot() {}
		//
		// Internal helpers
		virtual void LoadFromDatabase(UI_INPUT_MODEL_PTR, QObject *) {}
		virtual void LoadFromDatabase(UI_OUTPUT_MODEL_PTR, QObject *) {}
		//

		// Data refresh requests

		// Output-project related refreshes
		virtual void RefreshWidget(WidgetDataItemRequest_VARIABLE_GROUPS_SCROLL_AREA) {}
		virtual void RefreshWidget(WidgetDataItemRequest_VARIABLE_GROUPS_TOOLBOX) {}
		virtual void RefreshWidget(WidgetDataItemRequest_VARIABLE_GROUP_VARIABLE_GROUP_INSTANCE) {}
		virtual void RefreshWidget(WidgetDataItemRequest_VARIABLE_GROUPS_SUMMARY_SCROLL_AREA) {}
		virtual void RefreshWidget(WidgetDataItemRequest_VARIABLE_GROUPS_SUMMARY_VARIABLE_GROUP_INSTANCE) {}
		virtual void RefreshWidget(WidgetDataItemRequest_KAD_SPIN_CONTROLS_AREA) {}
		virtual void RefreshWidget(WidgetDataItemRequest_KAD_SPIN_CONTROL_WIDGET) {}
		virtual void RefreshWidget(WidgetDataItemRequest_TIMERANGE_REGION_WIDGET) {}
		virtual void RefreshWidget(WidgetDataItemRequest_DATETIME_WIDGET) {}
		virtual void RefreshWidget(WidgetDataItemRequest_GENERATE_OUTPUT_TAB) {}
		virtual void RefreshWidget(WidgetDataItemRequest_LIMIT_DMUS_TAB) {}

		// Input-project related refreshes
		virtual void RefreshWidget(WidgetDataItemRequest_MANAGE_DMUS_WIDGET) {}
		virtual void RefreshWidget(WidgetDataItemRequest_MANAGE_UOAS_WIDGET) {}
		virtual void RefreshWidget(WidgetDataItemRequest_MANAGE_VGS_WIDGET) {}


		// Actions
		virtual void ReceiveVariableItemChanged(WidgetActionItemRequest_ACTION_VARIABLE_GROUP_SET_MEMBER_SELECTION_CHANGED) {}
		virtual void ReceiveVariableItemChanged(WidgetActionItemRequest_ACTION_KAD_COUNT_CHANGE) {}
		virtual void ReceiveVariableItemChanged(WidgetActionItemRequest_ACTION_DO_RANDOM_SAMPLING_CHANGE) {}
		virtual void ReceiveVariableItemChanged(WidgetActionItemRequest_ACTION_RANDOM_SAMPLING_COUNT_PER_STAGE_CHANGE) {}
		virtual void ReceiveVariableItemChanged(WidgetActionItemRequest_ACTION_CONSOLIDATE_ROWS_CHANGE) {}
		virtual void ReceiveVariableItemChanged(WidgetActionItemRequest_ACTION_DATETIME_RANGE_CHANGE) {}
		virtual void ReceiveVariableItemChanged(WidgetActionItemRequest_ACTION_GENERATE_OUTPUT) {}
		virtual void LimitDMUsChange(WidgetActionItemRequest_ACTION_LIMIT_DMU_MEMBERS_CHANGE) {}
		//
		virtual void AddDMU(WidgetActionItemRequest_ACTION_ADD_DMU) {}
		virtual void DeleteDMU(WidgetActionItemRequest_ACTION_DELETE_DMU) {}
		virtual void DeleteDMUMembers(WidgetActionItemRequest_ACTION_DELETE_DMU_MEMBERS) {}
		virtual void AddDMUMembers(WidgetActionItemRequest_ACTION_ADD_DMU_MEMBERS) {}
		virtual void RefreshDMUsFromFile(WidgetActionItemRequest_ACTION_REFRESH_DMUS_FROM_FILE) {}
		virtual void AddUOA(WidgetActionItemRequest_ACTION_ADD_UOA) {}
		virtual void DeleteUOA(WidgetActionItemRequest_ACTION_DELETE_UOA) {}
		virtual void CreateVG(WidgetActionItemRequest_ACTION_CREATE_VG) {}
		virtual void DeleteVG(WidgetActionItemRequest_ACTION_DELETE_VG) {}
		virtual void RefreshVG(WidgetActionItemRequest_ACTION_REFRESH_VG) {}

	public:
		virtual void SetConnections() {}

		bool IsPoolTwo()
		{
			return isPool2;
		}

	protected:

		bool isPool2;

};

template<WORK_QUEUE_THREAD_LOOP_CLASS_ENUM UI_THREAD_LOOP_CLASS_ENUM>
class WorkQueueManager : public WorkQueueManagerBase
{

	public:

		explicit WorkQueueManager(bool isPool2_ = false, QObject *parent = 0)
			: WorkQueueManagerBase(isPool2_, parent)
		{

		}

};

#endif // WORKQUEUEMANAGER_H
