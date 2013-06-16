#include "outputprojectworkqueue.h"
#include "uioutputproject.h"
#include "Widgets/newgenemainwindow.h"
#include <QTimer>
#include "../UIData/widgetdatarefresh.h"

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

void OutputProjectWorkQueue::EmitMessage(std::string msg)
{
	emit SignalMessageBox(msg.c_str());
}

void OutputProjectWorkQueue::RefreshWidget(DATA_WIDGETS widget)
{
	get()->getWorkService().post(DoRefreshOutputWidget(widget, this));
}
