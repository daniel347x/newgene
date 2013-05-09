#ifndef UIPROJECTMANAGER_H
#define UIPROJECTMANAGER_H

#include "uimanager.h"

class NewGeneMainWindow;

class UIProjectManager : public UIManager
{
		Q_OBJECT
	public:
		explicit UIProjectManager(NewGeneMainWindow *parent = 0);

		static UIProjectManager & projectManager(NewGeneMainWindow * parent = NULL);

		void LoadDefaultProject();

	signals:

	public slots:

	private:

		static std::unique_ptr<UIProjectManager> projectManager_;

};

#endif // UIPROJECTMANAGER_H
