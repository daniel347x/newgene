#ifndef OUTPUTMODELWORKQUEUE_BASE_H
#define OUTPUTMODELWORKQUEUE_BASE_H

#include "workqueuemanager.h"

template<>
class WorkQueueManager<UI_OUTPUT_MODEL> : public WorkQueueManagerBase
{

	public:

		WorkQueueManager(bool isPool2_ = false, QObject * parent = 0)
			: WorkQueueManagerBase(isPool2_, parent)
		{

		}

};

#endif // OUTPUTMODELWORKQUEUE_BASE_H
