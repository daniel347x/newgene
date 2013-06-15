#include "outputmodelworkqueue.h"
#include "uioutputmodel.h"

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
}
