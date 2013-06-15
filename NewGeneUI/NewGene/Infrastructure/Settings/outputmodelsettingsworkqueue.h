#ifndef OUTPUTMODELSETTINGSWORKQUEUE_H
#define OUTPUTMODELSETTINGSWORKQUEUE_H

#include "Base/outputmodelsettingsworkqueue_base.h"

class UIOutputModelSettings;

class OutputModelSettingsWorkQueue : public WorkQueueManager<UI_OUTPUT_MODEL_SETTINGS>
{

	public:

		explicit OutputModelSettingsWorkQueue(QObject * parent = NULL);

		void SetUIObject(void * ui_input_object_)
		{
			inp = ui_input_object_;
		}

		void SetConnections();

	private:

		void * inp;

	protected:

		UIOutputModelSettings * get();

		void TestSlot();

};

#endif // OUTPUTMODELSETTINGSWORKQUEUE_H
