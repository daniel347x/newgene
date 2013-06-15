#include "outputmodelworkqueue.h"
#include "uioutputmodel.h"
#include "globals.h"
#include "uiprojectmanager.h"

OutputModelWorkQueue::OutputModelWorkQueue(QObject * parent)
	: WorkQueueManager<UI_OUTPUT_MODEL>(parent)
	, inp(nullptr)
{
}

UIOutputModel * OutputModelWorkQueue::get()
{
	return reinterpret_cast<UIOutputModel*>(inp);
}

void OutputModelWorkQueue::TestSlot()
{
	emit SignalMessageBox("Output model's \"TestSlot()\" successfully called and handled.");
}

void OutputModelWorkQueue::SetConnections()
{
	connect(this, SIGNAL(SignalMessageBox(QString)), get(), SLOT(SignalMessageBox(QString)));
	connect(&projectManagerUI(), SIGNAL(LoadModel(void*)), this, SLOT(LoadModel(void*)));
}

void OutputModelWorkQueue::LoadModel(void * model)
{
	if (model == get())
	{
		emit SignalMessageBox("Output model's \"LoadModel()\" successfully called and handled.");
	}
}
