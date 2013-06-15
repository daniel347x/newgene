#include "inputprojectworkqueue.h"
#include "uiinputproject.h"

InputProjectWorkQueue::InputProjectWorkQueue(QObject * parent)
	: WorkQueueManager<UI_INPUT_PROJECT>(parent)
	, inp(nullptr)
{
}

UIInputProject * InputProjectWorkQueue::get()
{
	return reinterpret_cast<UIInputProject*>(inp);
}

void InputProjectWorkQueue::TestSlot()
{
}

void InputProjectWorkQueue::SetConnections()
{
	connect(this, SIGNAL(SignalMessageBox(QString)), get(), SLOT(SignalMessageBox(QString)));
}
