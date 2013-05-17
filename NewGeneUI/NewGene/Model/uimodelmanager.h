#ifndef UIMODELMANAGER_H
#define UIMODELMANAGER_H

#include "globals.h"
#include "uimanager.h"

class NewGeneMainWindow;
class UIModel;

class UIModelManager : public UIManager
{
		Q_OBJECT
	public:
		explicit UIModelManager( QObject * parent = 0 );

		static UIModelManager & getModelManager();

		UIModel * loadDefaultModel();

	signals:

	public slots:

	private:
		static std::unique_ptr<UIModelManager> modelManager;
};

#endif // UIMODELMANAGER_H
