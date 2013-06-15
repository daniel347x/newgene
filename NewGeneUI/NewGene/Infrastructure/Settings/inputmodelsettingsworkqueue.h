#ifndef INPUTMODELSETTINGSWORKQUEUE_H
#define INPUTMODELSETTINGSWORKQUEUE_H

#include "Base/inputmodelsettingsworkqueue_base.h"

class UIInputModelSettings;

class InputModelSettingsWorkQueue : public WorkQueueManager<UI_INPUT_MODEL_SETTINGS>
{

	public:

		explicit InputModelSettingsWorkQueue(QObject * parent = NULL);

		void SetUIObject(void * ui_input_object_)
		{
			inp = ui_input_object_;
		}

		void SetConnections();

	private:

		void * inp;

	protected:

		UIInputModelSettings * get();

		void TestSlot();

};

#endif // INPUTMODELSETTINGSWORKQUEUE_H
