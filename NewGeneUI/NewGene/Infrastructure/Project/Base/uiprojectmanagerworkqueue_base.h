#ifndef UIPROJECTMANAGERWORKQUEUE_BASE_H
#define UIPROJECTMANAGERWORKQUEUE_BASE_H

#include "workqueuemanager.h"

template<>
class WorkQueueManager<UI_PROJECT_MANAGER> : public WorkQueueManagerBase
{

	public:

		WorkQueueManager(QObject * parent = 0)
			: WorkQueueManagerBase(parent)
		{

		}

};

#endif // UIPROJECTMANAGERWORKQUEUE_BASE_H
