#include "outputmodelworkqueue.h"
#include "uioutputmodel.h"
#include "globals.h"
#include "uiprojectmanager.h"

OutputModelWorkQueue::OutputModelWorkQueue(bool isPool2_, QObject * parent)
	: ModelWorkQueue<UI_OUTPUT_MODEL>(isPool2_, parent)
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
	connect(this, SIGNAL(SignalMessageBox(STD_STRING)), get(), SLOT(SignalMessageBox(STD_STRING)));
	connect(&projectManagerUI(), SIGNAL(LoadFromDatabase(UI_OUTPUT_MODEL_PTR)), this, SLOT(LoadFromDatabase(UI_OUTPUT_MODEL_PTR)));
}

void OutputModelWorkQueue::LoadFromDatabase(UI_OUTPUT_MODEL_PTR model)
{
	if (model == get())
	{
		//emit SignalMessageBox("Output model's \"LoadFromDatabase()\" successfully called and handled.");
	}
}
