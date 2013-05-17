#ifndef UIPROJECTMANAGER_H
#define UIPROJECTMANAGER_H

#include "uimanager.h"

class NewGeneMainWindow;
class UIProject;

class UIProjectManager : public UIManager
{
		Q_OBJECT
	public:
		explicit UIProjectManager( QObject * parent = 0 );

		static UIProjectManager & projectManager();

		UIProject * LoadDefaultProject( NewGeneMainWindow * parent = NULL );

	signals:

	public slots:

	private:

		static std::unique_ptr<UIProjectManager> projectManager_;

};

#endif // UIPROJECTMANAGER_H
