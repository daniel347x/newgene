#ifndef INPUTPROJECTWORKQUEUE_H
#define INPUTPROJECTWORKQUEUE_H

#include "workqueuemanager.h"

template<>
class WorkQueueManager<UI_INPUT_PROJECT> : public WorkQueueManagerBase
{

	public:

		WorkQueueManager(QObject *parent = 0)
			: WorkQueueManagerBase(parent)
		{

		}

};

#endif // INPUTPROJECTWORKQUEUE_H
