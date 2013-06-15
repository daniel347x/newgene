#ifndef GLOBALSETTINGSWORKQUEUE_BASE_H
#define GLOBALSETTINGSWORKQUEUE_BASE_H

#include "workqueuemanager.h"

template<>
class WorkQueueManager<UI_GLOBAL_SETTINGS> : public WorkQueueManagerBase
{

	public:

		WorkQueueManager(QObject *parent = 0)
			: WorkQueueManagerBase(parent)
		{

		}

};

#endif // GLOBALSETTINGSWORKQUEUE_BASE_H
