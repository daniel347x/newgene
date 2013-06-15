#ifndef GLOBALSETTINGSWORKQUEUE_H
#define GLOBALSETTINGSWORKQUEUE_H

#include "Base/globalsettingsworkqueue_base.h"

class UIAllGlobalSettings;

class GlobalSettingsWorkQueue : public WorkQueueManager<UI_GLOBAL_SETTINGS>
{

	public:

		explicit GlobalSettingsWorkQueue(QObject * parent = NULL);

		void SetUIObject(void * ui_input_object_)
		{
			inp = ui_input_object_;
		}

		void SetConnections();

	private:

		void * inp;

	protected:

		UIAllGlobalSettings * get();

		void TestSlot();

};

#endif // GLOBALSETTINGSWORKQUEUE_H
