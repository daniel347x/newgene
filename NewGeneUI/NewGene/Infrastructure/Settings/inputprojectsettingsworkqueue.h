#ifndef INPUTPROJECTSETTINGSWORKQUEUE_H
#define INPUTPROJECTSETTINGSWORKQUEUE_H

#include "Base/inputprojectsettingsworkqueue_base.h"

class UIInputProjectSettings;

class InputProjectSettingsWorkQueue : public WorkQueueManager<UI_INPUT_PROJECT_SETTINGS>
{

	public:

		explicit InputProjectSettingsWorkQueue(QObject * parent = NULL);

		void SetUIObject(void * ui_input_object_)
		{
			inp = ui_input_object_;
		}

		void SetConnections();

	private:

		void * inp;

	protected:

		UIInputProjectSettings * get();

		void TestSlot();

};

#endif // INPUTPROJECTSETTINGSWORKQUEUE_H
