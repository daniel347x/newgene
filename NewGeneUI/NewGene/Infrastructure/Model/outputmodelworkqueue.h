#ifndef OUTPUTMODELWORKQUEUE_H
#define OUTPUTMODELWORKQUEUE_H

#include "Base/modelworkqueue.h"

class UIOutputModel;

class OutputModelWorkQueue : public ModelWorkQueue<UI_OUTPUT_MODEL>
{

	public:

		explicit OutputModelWorkQueue(bool isPool2_ = false, QObject * parent = NULL);

		void SetUIObject(void * ui_input_object_)
		{
			inp = ui_input_object_;
		}

		void SetConnections();

	private:

		void * inp;

	protected:

		UIOutputModel * get();

		void TestSlot();
		void LoadFromDatabase(UI_OUTPUT_MODEL_PTR);

};

#endif // OUTPUTMODELWORKQUEUE_H
