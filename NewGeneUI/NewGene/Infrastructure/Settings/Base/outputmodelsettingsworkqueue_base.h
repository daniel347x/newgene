#ifndef OUTPUTMODELSETTINGSWORKQUEUE_BASE_H
#define OUTPUTMODELSETTINGSWORKQUEUE_BASE_H

#include "workqueuemanager.h"

template<>
class WorkQueueManager<UI_OUTPUT_MODEL_SETTINGS> : public WorkQueueManagerBase
{

	public:

		WorkQueueManager(QObject *parent = 0)
			: WorkQueueManagerBase(parent)
		{

		}

};

#endif // OUTPUTMODELSETTINGSWORKQUEUE_BASE_H
