#include "inputmodelworkqueue.h"
#include "uiinputmodel.h"
#include "globals.h"
#include "uiprojectmanager.h"
#include "uimessagersingleshot.h"

InputModelWorkQueue::InputModelWorkQueue(bool isPool2_, QObject * parent)
	: ModelWorkQueue<UI_INPUT_MODEL>(isPool2_, parent)
	, inp(nullptr)
{
}

UIInputModel * InputModelWorkQueue::get()
{
	return reinterpret_cast<UIInputModel *>(inp);
}

void InputModelWorkQueue::TestSlot()
{
	emit SignalMessageBox("Input model's \"TestSlot()\" successfully called and handled.");
}

void InputModelWorkQueue::SetConnections()
{
	connect(this, SIGNAL(SignalMessageBox(STD_STRING)), get(), SLOT(SignalMessageBox(STD_STRING)));

	if (IsDatabasePool())
	{
		connect(&projectManagerUI(), SIGNAL(LoadFromDatabase(UI_INPUT_MODEL_PTR, QObject *)), this, SLOT(LoadFromDatabase(UI_INPUT_MODEL_PTR, QObject *)));
		connect(this, SIGNAL(DoneLoadingFromDatabase(UI_INPUT_MODEL_PTR, QObject *)), static_cast<QObject *>(&projectManagerUI()), SLOT(DoneLoadingFromDatabase(UI_INPUT_MODEL_PTR,
				QObject *)));
	}
}

void InputModelWorkQueue::LoadFromDatabase(UI_INPUT_MODEL_PTR model, QObject * mainWindowObject)
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

