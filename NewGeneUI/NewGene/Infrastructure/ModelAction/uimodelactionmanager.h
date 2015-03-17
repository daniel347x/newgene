#ifndef UIMODELACTIONMANAGER_H
#define UIMODELACTIONMANAGER_H

#include "globals.h"
#include "Infrastructure/uimanager.h"
#include "../../../NewGeneBackEnd/ModelAction/ModelActionManager.h"

class NewGeneMainWindow;

class UIModelActionManager : public QObject, public
	UIManager<UIModelActionManager, ModelActionManager, MANAGER_DESCRIPTION_NAMESPACE::MANAGER_MODEL_ACTION_UI, MANAGER_DESCRIPTION_NAMESPACE::MANAGER_MODEL_ACTION>
{
		Q_OBJECT
	public:
		explicit UIModelActionManager(QObject * parent, UIMessager & messager);

	signals:

	public slots:

};

#endif // UIMODELACTIONMANAGER_H
