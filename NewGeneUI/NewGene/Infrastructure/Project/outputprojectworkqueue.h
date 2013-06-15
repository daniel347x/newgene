#ifndef OUTPUTPROJECTWORKQUEUE_H
#define OUTPUTPROJECTWORKQUEUE_H

#include "workqueuemanager.h"

template<>
class WorkQueueManager<UI_OUTPUT_PROJECT> : public WorkQueueManagerBase
{

	public:

		WorkQueueManager(QObject *parent = 0)
			: WorkQueueManagerBase(parent)
		{

		}

};

#endif // OUTPUTPROJECTWORKQUEUE_H
