#ifndef MODELWORKQUEUE_H
#define MODELWORKQUEUE_H

#include "inputmodelworkqueue_base.h"
#include "outputmodelworkqueue_base.h"

template<WORK_QUEUE_THREAD_LOOP_CLASS_ENUM UI_MODEL_THREAD_LOOP_CLASS_ENUM>
class ModelWorkQueue : public WorkQueueManager<UI_MODEL_THREAD_LOOP_CLASS_ENUM>
{
	public:
		ModelWorkQueue<UI_MODEL_THREAD_LOOP_CLASS_ENUM>(bool isPool2_, QObject * parent)
			: WorkQueueManager<UI_MODEL_THREAD_LOOP_CLASS_ENUM>(isPool2_, parent)
		{
		}

		bool IsDatabasePool()
		{
            return this->IsPoolTwo();
		}

};

#endif // MODELWORKQUEUE_H
