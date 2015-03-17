#include "outputprojectsettingsworkqueue.h"
#include "uioutputprojectsettings.h"

OutputProjectSettingsWorkQueue::OutputProjectSettingsWorkQueue(QObject * parent)
	: WorkQueueManager<UI_OUTPUT_PROJECT_SETTINGS>(parent)
	, inp(nullptr)
{
}

UIOutputProjectSettings * OutputProjectSettingsWorkQueue::get()
{
	return reinterpret_cast<UIOutputProjectSettings *>(inp);
}

void OutputProjectSettingsWorkQueue::TestSlot()
{
	emit SignalMessageBox("Output project setting's \"TestSlot()\" successfully called and handled.");
}

void OutputProjectSettingsWorkQueue::SetConnections()
{
	connect(this, SIGNAL(SignalMessageBox(STD_STRING)), get(), SLOT(SignalMessageBox(STD_STRING)));
}
