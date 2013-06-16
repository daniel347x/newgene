#include "outputmodelworkqueue.h"
#include "uioutputmodel.h"
#include "globals.h"
#include "uiprojectmanager.h"
#include "uimessagersingleshot.h"

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

	if (IsDatabasePool())
	{
		connect(&projectManagerUI(), SIGNAL(LoadFromDatabase(UI_OUTPUT_MODEL_PTR)), this, SLOT(LoadFromDatabase(UI_OUTPUT_MODEL_PTR)));
		connect(this, SIGNAL(DoneLoadingFromDatabase(UI_OUTPUT_MODEL_PTR)), static_cast<QObject *>(&projectManagerUI()), SLOT(DoneLoadingFromDatabase(UI_OUTPUT_MODEL_PTR)));
	}
}

void OutputModelWorkQueue::LoadFromDatabase(UI_OUTPUT_MODEL_PTR model)
{
	UIMessagerSingleShot messager;
	if (!get()->is_model_equivalent(messager.get(), model))
	{
		return;
	}

	emit SignalMessageBox("Output model's \"LoadFromDatabase()\" successfully called and handled.");

	// Lock model and load all from database here
	{

		get()->loaded(true);
	}

	emit DoneLoadingFromDatabase(model);
}
