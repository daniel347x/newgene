#ifndef UITHREADMANAGER_H
#define UITHREADMANAGER_H

#include "globals.h"
#include "uimanager.h"
#include "..\..\..\NewGeneBackEnd\Status\ThreadManager.h"

class UIThreadManager : public QObject, public UIManager<UIThreadManager, ThreadManager, MANAGER_DESCRIPTION_NAMESPACE::MANAGER_THREADS_UI, MANAGER_DESCRIPTION_NAMESPACE::MANAGER_THREADS>
{
		Q_OBJECT

	public:

		explicit UIThreadManager( QObject * parent = 0 );

	signals:

	public slots:

	public:

		std::queue<WorkItem*> WorkQueueUI;

	private:

};

#endif // UITHREADMANAGER_H
