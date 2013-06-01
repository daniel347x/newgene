#ifndef UITRIGGERMANAGER_H
#define UITRIGGERMANAGER_H

#include "globals.h"
#include "uimanager.h"
#include "..\..\..\NewGeneBackEnd\Status\ThreadManager.h"

class UITriggerManager : public QObject, public UIManager<UITriggerManager, TriggerManager, MANAGER_DESCRIPTION_NAMESPACE::MANAGER_TRIGGERS_UI, MANAGER_DESCRIPTION_NAMESPACE::MANAGER_TRIGGERS>
{
		Q_OBJECT

	public:

		explicit UIThreadManager( QObject * parent = 0 );

	signals:

	public slots:

	private:

};

#endif // UITRIGGERMANAGER_H
