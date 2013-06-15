#ifndef WORKQUEUEMANAGER_H
#define WORKQUEUEMANAGER_H

#include <QObject>

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
};

class WorkQueueManagerBase : public QObject
{

		Q_OBJECT

	public:

		explicit WorkQueueManagerBase(QObject *parent = 0);

	signals:
		// Signals for ALL specializations of WorkQueueManager go here,
		// and are NOT (and don't need to be) defined as virtual
		void SignalMessageBox(QString);

	public slots:
		// Slots for ALL specializations go here,
		// and are all defined as virtual so that specializations of
		// WorkQueueManager can override them
		virtual void TestSlot() {}

	public:
		virtual void SetConnections() {}

};

template<WORK_QUEUE_THREAD_LOOP_CLASS_ENUM UI_THREAD_LOOP_CLASS_ENUM>
class WorkQueueManager : public WorkQueueManagerBase
{

	public:

		explicit WorkQueueManager(QObject *parent = 0)
			: WorkQueueManagerBase(parent)
		{

		}

};

#endif // WORKQUEUEMANAGER_H
