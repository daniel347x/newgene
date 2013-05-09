#ifndef UIPROJECTMANAGER_H
#define UIPROJECTMANAGER_H

#include "uimanager.h"

class NewGeneMainWindow;
class UIModelManager;
class UISettingsManager;
class UIDocumentManager;
class UIStatusManager;

class UIProjectManager : public UIManager
{
		Q_OBJECT
	public:
		explicit UIProjectManager(NewGeneMainWindow *parent = 0);

		static UIProjectManager & projectManager(NewGeneMainWindow * parent = NULL);

		void LoadDefaultProject();

		UIModelManager & modelManager(NewGeneMainWindow * parent = NULL);
		UISettingsManager & settingsManager(NewGeneMainWindow * parent = NULL);
		UIDocumentManager & documentManager(NewGeneMainWindow * parent = NULL);
		UIStatusManager & statusManager(NewGeneMainWindow * parent = NULL);

	signals:

	public slots:

	private:

		static std::unique_ptr<UIProjectManager> projectManager_;

};

#endif // UIPROJECTMANAGER_H
