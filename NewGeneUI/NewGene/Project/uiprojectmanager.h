#ifndef UIPROJECTMANAGER_H
#define UIPROJECTMANAGER_H

#include "uimanager.h"
#include "..\..\..\NewGeneBackEnd\Project\ProjectManager.h"
#include <memory>

class NewGeneMainWindow;
class UIInputProject;
class UIOutputProject;

class UIProjectManager : public QObject, public UIManager<UIProjectManager, ProjectManager, MANAGER_DESCRIPTION_NAMESPACE::MANAGER_PROJECT_UI, MANAGER_DESCRIPTION_NAMESPACE::MANAGER_PROJECT>
{
		Q_OBJECT
	public:
		explicit UIProjectManager( QObject * parent = 0 );

		//UIProject * LoadDefaultProject( NewGeneMainWindow * parent = NULL );

	signals:

	public slots:

	private:

};

#endif // UIPROJECTMANAGER_H
