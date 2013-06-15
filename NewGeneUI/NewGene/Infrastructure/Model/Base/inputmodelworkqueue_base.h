#ifndef INPUTMODELWORKQUEUE_BASE_H
#define INPUTMODELWORKQUEUE_BASE_H

#include "workqueuemanager.h"

template<>
class WorkQueueManager<UI_INPUT_MODEL> : public WorkQueueManagerBase
{

	public:

		WorkQueueManager(QObject *parent = 0)
			: WorkQueueManagerBase(parent)
		{

		}

};

#endif // INPUTMODELWORKQUEUE_BASE_H
