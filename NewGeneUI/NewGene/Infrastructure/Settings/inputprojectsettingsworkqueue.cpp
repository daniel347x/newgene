#include "inputprojectsettingsworkqueue.h"
#include "uiinputprojectsettings.h"

InputProjectSettingsWorkQueue::InputProjectSettingsWorkQueue(QObject * parent)
	: WorkQueueManager<UI_INPUT_PROJECT_SETTINGS>(parent)
	, inp(nullptr)
{
}

UIInputProjectSettings * InputProjectSettingsWorkQueue::get()
{
	return reinterpret_cast<UIInputProjectSettings*>(inp);
}

void InputProjectSettingsWorkQueue::TestSlot()
{
	emit SignalMessageBox("Input project setting's \"TestSlot()\" successfully called and handled.");
}

void InputProjectSettingsWorkQueue::SetConnections()
{
	connect(this, SIGNAL(SignalMessageBox(STD_STRING)), get(), SLOT(SignalMessageBox(STD_STRING)));
}
