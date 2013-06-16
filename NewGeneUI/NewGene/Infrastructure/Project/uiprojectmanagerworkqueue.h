#ifndef UIPROJECTMANAGERWORKQUEUE_H
#define UIPROJECTMANAGERWORKQUEUE_H

#include "Base/uiprojectmanagerworkqueue_base.h"

class UIProjectManager;

class UIProjectManagerWorkQueue : public WorkQueueManager<UI_PROJECT_MANAGER>
{

	public:

		explicit UIProjectManagerWorkQueue(QObject * parent = NULL);

		void SetUIObject(void * ui_input_object_)
		{
			inp = ui_input_object_;
		}

		void SetConnections();

	private:

		void * inp;

	protected:

		UIProjectManager * get();

		void TestSlot();

};

#endif // UIPROJECTMANAGERWORKQUEUE_H
