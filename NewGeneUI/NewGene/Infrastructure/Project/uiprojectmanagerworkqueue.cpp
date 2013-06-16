#include "uiprojectmanagerworkqueue.h"
#include "uiprojectmanager.h"

UIProjectManagerWorkQueue::UIProjectManagerWorkQueue(QObject * parent)
	: WorkQueueManager<UI_PROJECT_MANAGER>(parent)
	, inp(nullptr)
{
}

UIProjectManager * UIProjectManagerWorkQueue::get()
{
	return reinterpret_cast<UIProjectManager*>(inp);
}

void UIProjectManagerWorkQueue::TestSlot()
{
	emit SignalMessageBox("Project manager's \"TestSlot()\" successfully called and handled.");
}

void UIProjectManagerWorkQueue::SetConnections()
{
	connect(this, SIGNAL(SignalMessageBox(QString)), get(), SLOT(SignalMessageBox(QString)));
}
