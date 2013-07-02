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

		// Called in context of Boost WORK POOL threads - NOT in context of this work queue manager's event loop thread
		void HandleChanges(DataChangeMessage & changes);

	private:

		void * inp;

	protected:

	// ********************************* //
	// Slot Overrides
	// ********************************* //

		void TestSlot();

		//void RefreshWidget(WidgetDataItemRequest_VARIABLE_GROUPS_SCROLL_AREA);

};

#endif // INPUTPROJECTWORKQUEUE_H
