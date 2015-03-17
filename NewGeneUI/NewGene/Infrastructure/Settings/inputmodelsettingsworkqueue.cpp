#include "inputmodelsettingsworkqueue.h"
#include "uiinputmodelsettings.h"

InputModelSettingsWorkQueue::InputModelSettingsWorkQueue(QObject * parent)
	: WorkQueueManager<UI_INPUT_MODEL_SETTINGS>(parent)
	, inp(nullptr)
{
}

UIInputModelSettings * InputModelSettingsWorkQueue::get()
{
	return reinterpret_cast<UIInputModelSettings *>(inp);
}

void InputModelSettingsWorkQueue::TestSlot()
{
	emit SignalMessageBox("Input model setting's \"TestSlot()\" successfully called and handled.");
}

void InputModelSettingsWorkQueue::SetConnections()
{
	connect(this, SIGNAL(SignalMessageBox(STD_STRING)), get(), SLOT(SignalMessageBox(STD_STRING)));
}
