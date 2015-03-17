#ifndef UIMODELMANAGER_H
#define UIMODELMANAGER_H

#include "globals.h"
#include "Infrastructure/uimanager.h"
#include "../../../NewGeneBackEnd/Model/ModelManager.h"

class NewGeneMainWindow;

class UIInputModel;
class UIOutputModel;

typedef UIInputModel * UI_INPUT_MODEL_PTR;
typedef UIOutputModel * UI_OUTPUT_MODEL_PTR;

class UIModelManager : public QObject, public UIManager<UIModelManager, ModelManager, MANAGER_DESCRIPTION_NAMESPACE::MANAGER_MODEL_UI, MANAGER_DESCRIPTION_NAMESPACE::MANAGER_MODEL>
{
		Q_OBJECT
	public:
		explicit UIModelManager(QObject * parent, UIMessager & messager);

		//UIInputModel * loadDefaultModel();

	signals:

	public slots:

	private:

};

#endif // UIMODELMANAGER_H
