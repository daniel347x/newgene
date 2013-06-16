#ifndef INPUTMODELWORKQUEUE_H
#define INPUTMODELWORKQUEUE_H

#include "Base/inputmodelworkqueue_base.h"

class UIInputModel;

class InputModelWorkQueue : public WorkQueueManager<UI_INPUT_MODEL>
{

	public:

		explicit InputModelWorkQueue(bool isPool2_ = false, QObject * parent = NULL);

		void SetUIObject(void * ui_input_object_)
		{
			inp = ui_input_object_;
		}

		void SetConnections();

	private:

		void * inp;

	protected:

		UIInputModel * get();

		void TestSlot();
		void LoadFromDatabase(UI_INPUT_MODEL_PTR);

};

#endif // INPUTMODELWORKQUEUE_H
