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
	return reinterpret_cast<UIOutputModel *>(inp);
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
		connect(&projectManagerUI(), SIGNAL(LoadFromDatabase(UI_OUTPUT_MODEL_PTR, QObject *)), this, SLOT(LoadFromDatabase(UI_OUTPUT_MODEL_PTR, QObject *)));
		connect(this, SIGNAL(DoneLoadingFromDatabase(UI_OUTPUT_MODEL_PTR, QObject *)), static_cast<QObject *>(&projectManagerUI()), SLOT(DoneLoadingFromDatabase(UI_OUTPUT_MODEL_PTR,
				QObject *)));
	}
}

void OutputModelWorkQueue::LoadFromDatabase(UI_OUTPUT_MODEL_PTR model, QObject * mainWindowObject)
{
	UIMessagerSingleShot messager;

	if (!get()->is_model_equivalent(messager.get(), model))
	{
		return;
	}

	// Lock model and load all from database here
	{
		model->backend().LoadTables();
		get()->loaded(true);
	}

	emit DoneLoadingFromDatabase(model, mainWindowObject);
}
