#include "inputmodelworkqueue.h"
#include "uiinputmodel.h"
#include "globals.h"
#include "uiprojectmanager.h"

InputModelWorkQueue::InputModelWorkQueue(QObject * parent)
	: WorkQueueManager<UI_INPUT_MODEL>(parent)
	, inp(nullptr)
{
}

UIInputModel * InputModelWorkQueue::get()
{
	return reinterpret_cast<UIInputModel*>(inp);
}

void InputModelWorkQueue::TestSlot()
{
	emit SignalMessageBox("Input model's \"TestSlot()\" successfully called and handled.");
}

void InputModelWorkQueue::SetConnections()
{
	connect(this, SIGNAL(SignalMessageBox(QString)), get(), SLOT(SignalMessageBox(QString)));
	connect(&projectManagerUI(), SIGNAL(LoadModel(void*)), this, SLOT(LoadModel(void*)));
}

void InputModelWorkQueue::LoadModel(void * model)
{
	if (model == get())
	{
		emit SignalMessageBox("Input model's \"LoadModel()\" successfully called and handled.");
	}
}

