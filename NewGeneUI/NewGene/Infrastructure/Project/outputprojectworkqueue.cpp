#include "outputprojectworkqueue.h"
#include "uioutputproject.h"
#include "Widgets/newgenemainwindow.h"
#include <QTimer>

OutputProjectWorkQueue::OutputProjectWorkQueue(QObject * parent)
	: WorkQueueManager(parent)
	, outp(nullptr)
{
}

UIOutputProject * OutputProjectWorkQueue::get()
{
	return reinterpret_cast<UIOutputProject*>(outp);
}

void OutputProjectWorkQueue::TestSlot()
{
	emit SignalMessageBox("Output project's \"TestSlot()\" successfully called and handled.");
}

void OutputProjectWorkQueue::SetConnections()
{
	connect(this, SIGNAL(SignalMessageBox(QString)), get(), SLOT(SignalMessageBox(QString)));
}
