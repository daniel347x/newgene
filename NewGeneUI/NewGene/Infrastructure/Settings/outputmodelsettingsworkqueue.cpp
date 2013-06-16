#include "outputmodelsettingsworkqueue.h"
#include "uioutputmodelsettings.h"

OutputModelSettingsWorkQueue::OutputModelSettingsWorkQueue(QObject * parent)
	: WorkQueueManager<UI_OUTPUT_MODEL_SETTINGS>(parent)
	, inp(nullptr)
{
}

UIOutputModelSettings * OutputModelSettingsWorkQueue::get()
{
	return reinterpret_cast<UIOutputModelSettings*>(inp);
}

void OutputModelSettingsWorkQueue::TestSlot()
{
	emit SignalMessageBox("Output model setting's \"TestSlot()\" successfully called and handled.");
}

void OutputModelSettingsWorkQueue::SetConnections()
{
	connect(this, SIGNAL(SignalMessageBox(STD_STRING)), get(), SLOT(SignalMessageBox(STD_STRING)));
}
