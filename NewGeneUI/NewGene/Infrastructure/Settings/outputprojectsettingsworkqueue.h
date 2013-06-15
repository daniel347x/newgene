#ifndef OUTPUTPROJECTSETTINGSWORKQUEUE_H
#define OUTPUTPROJECTSETTINGSWORKQUEUE_H

#include "Base/outputprojectsettingsworkqueue_base.h"

class UIOutputProjectSettings;

class OutputProjectSettingsWorkQueue : public WorkQueueManager<UI_OUTPUT_PROJECT_SETTINGS>
{

	public:

		explicit OutputProjectSettingsWorkQueue(QObject * parent = NULL);

		void SetUIObject(void * ui_input_object_)
		{
			inp = ui_input_object_;
		}

		void SetConnections();

	private:

		void * inp;

	protected:

		UIOutputProjectSettings * get();

		void TestSlot();

};

#endif // OUTPUTPROJECTSETTINGSWORKQUEUE_H
