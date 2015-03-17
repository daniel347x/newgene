#ifndef UIUIDATAMANAGER_H
#define UIUIDATAMANAGER_H

#include "globals.h"
#include "Infrastructure/uimanager.h"
#include "../../../NewGeneBackEnd/UIData/UIDataManager.h"

class NewGeneMainWindow;

class UIUIDataManager : public QObject, public
	UIManager<UIUIDataManager, UIDataManager, MANAGER_DESCRIPTION_NAMESPACE::MANAGER_UI_DATA_UI, MANAGER_DESCRIPTION_NAMESPACE::MANAGER_UI_DATA>
{
		Q_OBJECT
	public:
		explicit UIUIDataManager(QObject * parent, UIMessager & messager);

	signals:

	public slots:

};

#endif // UIDOCUMENTMANAGER_H
