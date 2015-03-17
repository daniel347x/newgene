#ifndef INPUTPROJECTSETTINGSWORKQUEUE_BASE_H
#define INPUTPROJECTSETTINGSWORKQUEUE_BASE_H

#include "workqueuemanager.h"

template<>
class WorkQueueManager<UI_INPUT_PROJECT_SETTINGS> : public WorkQueueManagerBase
{

	public:

		WorkQueueManager(QObject * parent = 0)
			: WorkQueueManagerBase(parent)
		{

		}

};

#endif // INPUTPROJECTSETTINGSWORKQUEUE_BASE_H
