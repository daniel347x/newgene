#ifndef UITHREADMANAGER_H
#define UITHREADMANAGER_H

#include "Infrastructure/uimanager.h"
#include "../../../NewGeneBackEnd/Threads/ThreadManager.h"

class UIThreadManager : public QObject, public UIManager<UIThreadManager, ThreadManager, MANAGER_DESCRIPTION_NAMESPACE::MANAGER_THREADS_UI, MANAGER_DESCRIPTION_NAMESPACE::MANAGER_THREADS>
{
		Q_OBJECT

	public:

		explicit UIThreadManager( QObject * parent, UIMessager & messager );

	signals:

	public slots:

	private:

};

#endif // UITHREADMANAGER_H
