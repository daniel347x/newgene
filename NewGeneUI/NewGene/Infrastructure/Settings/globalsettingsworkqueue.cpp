#include "globalsettingsworkqueue.h"
#include "uiallglobalsettings.h"

GlobalSettingsWorkQueue::GlobalSettingsWorkQueue(QObject * parent)
	: WorkQueueManager<UI_GLOBAL_SETTINGS>(parent)
	, inp(nullptr)
{
}

UIAllGlobalSettings * GlobalSettingsWorkQueue::get()
{
	return reinterpret_cast<UIAllGlobalSettings*>(inp);
}

void GlobalSettingsWorkQueue::TestSlot()
{
	emit SignalMessageBox("Global setting's \"TestSlot()\" successfully called and handled.");
}

void GlobalSettingsWorkQueue::SetConnections()
{
	connect(this, SIGNAL(SignalMessageBox(QString)), get(), SLOT(SignalMessageBox(QString)));
}
