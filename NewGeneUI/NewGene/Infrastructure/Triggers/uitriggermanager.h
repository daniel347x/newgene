#ifndef UITRIGGERMANAGER_H
#define UITRIGGERMANAGER_H

#include "Infrastructure/uimanager.h"
#include "..\..\..\NewGeneBackEnd\Triggers\TriggerManager.h"
#include <QWidget>

class UITriggerManager : public QObject, public UIManager<UITriggerManager, TriggerManager, MANAGER_DESCRIPTION_NAMESPACE::MANAGER_TRIGGERS_UI, MANAGER_DESCRIPTION_NAMESPACE::MANAGER_TRIGGERS>
{
		Q_OBJECT

	public:

		explicit UITriggerManager( QObject * parent = 0 );

	signals:

	public slots:

	public:

	private:

};

#endif // UITRIGGERMANAGER_H
