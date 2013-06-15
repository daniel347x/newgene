#ifndef INPUTPROJECTWORKQUEUE_H
#define INPUTPROJECTWORKQUEUE_H

#include "Base/inputprojectworkqueue_base.h"

class InputProjectWorkQueue : public WorkQueueManager<UI_INPUT_PROJECT>
{

	public:

		explicit InputProjectWorkQueue(QObject * parent = NULL);

	protected:

		virtual WorkQueueManager<UI_INPUT_PROJECT> * InstantiateWorkQueue()
		{
			return nullptr;
		}

};

#endif // INPUTPROJECTWORKQUEUE_H
