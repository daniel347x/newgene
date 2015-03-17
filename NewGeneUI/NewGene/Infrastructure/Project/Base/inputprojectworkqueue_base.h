#ifndef INPUTPROJECTWORKQUEUE_BASE_H
#define INPUTPROJECTWORKQUEUE_BASE_H

#include "workqueuemanager.h"

template<>
class WorkQueueManager<UI_INPUT_PROJECT> : public WorkQueueManagerBase
{

	public:

		WorkQueueManager(QObject * parent = 0)
			: WorkQueueManagerBase(parent)
		{

		}

};

#endif // INPUTPROJECTWORKQUEUE_BASE_H
