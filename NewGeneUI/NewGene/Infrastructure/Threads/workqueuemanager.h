#ifndef WORKQUEUEMANAGER_H
#define WORKQUEUEMANAGER_H

#include "globals.h"
#include <QObject>
#include "../../../../NewGeneBackEnd/UIData/DataWidgets.h"
#include "uimodelmanager.h"

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

		virtual void EmitOutputWidgetDataRefresh(WidgetDataItem_VARIABLE_GROUPS_SCROLL_AREA & widgetData) {}
		virtual void EmitOutputWidgetDataRefresh(WidgetDataItem_VARIABLE_GROUPS_TOOLBOX & widgetData) {}
		virtual void EmitOutputWidgetDataRefresh(WidgetDataItem_VARIABLE_GROUP_VARIABLE_GROUP_INSTANCE & widgetData) {}

	signals:
		// Signals for ALL specializations of WorkQueueManager go here,
		// and are NOT (and don't need to be) defined as virtual
		void SignalMessageBox(STD_STRING);
		void DoneLoadingFromDatabase(UI_INPUT_MODEL_PTR);
		void DoneLoadingFromDatabase(UI_OUTPUT_MODEL_PTR);
		void WidgetDataRefresh(WidgetDataItem_VARIABLE_GROUPS_SCROLL_AREA);
		void WidgetDataRefresh(WidgetDataItem_VARIABLE_GROUPS_TOOLBOX);
		void WidgetDataRefresh(WidgetDataItem_VARIABLE_GROUP_VARIABLE_GROUP_INSTANCE);

	public slots:
		// Slots for ALL specializations go here,
		// and are all defined as virtual so that specializations of
		// WorkQueueManager can override them
		virtual void TestSlot() {}
		virtual void LoadFromDatabase(UI_INPUT_MODEL_PTR) {}
		virtual void LoadFromDatabase(UI_OUTPUT_MODEL_PTR) {}
		virtual void RefreshWidget(WidgetDataItemRequest_VARIABLE_GROUPS_SCROLL_AREA) {}
		virtual void RefreshWidget(WidgetDataItemRequest_VARIABLE_GROUPS_TOOLBOX) {}
		virtual void RefreshWidget(WidgetDataItemRequest_VARIABLE_GROUP_VARIABLE_GROUP_INSTANCE) {}
		virtual void ReceiveVariableItemChanged(const QModelIndex &, const QModelIndex &, const QVector<int>) {}

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
