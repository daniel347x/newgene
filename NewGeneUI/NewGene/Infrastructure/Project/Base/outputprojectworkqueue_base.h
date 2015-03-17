#ifndef OUTPUTPROJECTWORKQUEUE_BASE_H
#define OUTPUTPROJECTWORKQUEUE_BASE_H

#include "workqueuemanager.h"

template<>
class WorkQueueManager<UI_OUTPUT_PROJECT> : public WorkQueueManagerBase
{

	public:

		WorkQueueManager(QObject * parent = 0)
			: WorkQueueManagerBase(parent)
		{

		}

};

#endif // OUTPUTPROJECTWORKQUEUE_BASE_H
