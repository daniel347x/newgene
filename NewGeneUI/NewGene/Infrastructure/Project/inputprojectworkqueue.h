#ifndef INPUTPROJECTWORKQUEUE_H
#define INPUTPROJECTWORKQUEUE_H

#include "Base/inputprojectworkqueue_base.h"

class UIInputProject;

class InputProjectWorkQueue : public WorkQueueManager<UI_INPUT_PROJECT>
{

	public:

		explicit InputProjectWorkQueue(QObject * parent = NULL);

		void SetUIObject(void * ui_input_object_)
		{
			inp = ui_input_object_;
		}

		void SetConnections();

		UIInputProject * get();

	private:

		void * inp;

	protected:

	// ********************************* //
	// Slot Overrides
	// ********************************* //

		void TestSlot();
		void RefreshWidget(DATA_WIDGETS);

};

#endif // INPUTPROJECTWORKQUEUE_H
