#ifndef OUTPUTPROJECTSETTINGSWORKQUEUE_BASE_H
#define OUTPUTPROJECTSETTINGSWORKQUEUE_BASE_H

#include "workqueuemanager.h"

template<>
class WorkQueueManager<UI_OUTPUT_PROJECT_SETTINGS> : public WorkQueueManagerBase
{

	public:

		WorkQueueManager(QObject *parent = 0)
			: WorkQueueManagerBase(parent)
		{

		}

};

#endif // OUTPUTPROJECTSETTINGSWORKQUEUE_BASE_H
