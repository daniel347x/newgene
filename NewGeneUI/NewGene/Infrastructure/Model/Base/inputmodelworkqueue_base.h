#ifndef INPUTMODELWORKQUEUE_BASE_H
#define INPUTMODELWORKQUEUE_BASE_H

#include "workqueuemanager.h"
#include "modelworkqueue.h"

template<>
class WorkQueueManager<UI_INPUT_MODEL> : public WorkQueueManagerBase
{

	public:

		WorkQueueManager(bool isPool2_ = false, QObject * parent = 0)
			: WorkQueueManagerBase(isPool2_, parent)
		{

		}

};

#endif // INPUTMODELWORKQUEUE_BASE_H
