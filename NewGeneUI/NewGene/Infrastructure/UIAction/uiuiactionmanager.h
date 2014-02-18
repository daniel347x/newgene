#ifndef UIUIACTIONMANAGER_H
#define UIUIACTIONMANAGER_H

#include "globals.h"
#include "Infrastructure/uimanager.h"
#include "../../../../NewGeneBackEnd/UIAction/UIActionManager.h"

class NewGeneMainWindow;

class UIUIActionManager : public QObject, public UIManager<UIUIActionManager, UIActionManager, MANAGER_DESCRIPTION_NAMESPACE::MANAGER_UI_ACTION_UI, MANAGER_DESCRIPTION_NAMESPACE::MANAGER_UI_ACTION>
{
		Q_OBJECT
	public:
		explicit UIUIActionManager( QObject * parent, UIMessager & messager );

	signals:

	public slots:

};


#endif // UIUIACTIONMANAGER_H
