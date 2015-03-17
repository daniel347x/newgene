#ifndef INPUTMODELSETTINGSWORKQUEUE_BASE_H
#define INPUTMODELSETTINGSWORKQUEUE_BASE_H

#include "workqueuemanager.h"

template<>
class WorkQueueManager<UI_INPUT_MODEL_SETTINGS> : public WorkQueueManagerBase
{

	public:

		WorkQueueManager(QObject * parent = 0)
			: WorkQueueManagerBase(parent)
		{

		}

};

#endif // INPUTMODELSETTINGSWORKQUEUE_BASE_H
