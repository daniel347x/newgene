#ifndef UIPROJECTMANAGER_H
#define UIPROJECTMANAGER_H

#include "uiinputproject.h"
#include "uioutputproject.h"
#include "uimanager.h"
#include "uimessager.h"
#include "..\..\..\NewGeneBackEnd\Project\ProjectManager.h"
#include <memory>
#include <map>

class NewGeneMainWindow;

class UIProjectManager : public QObject, public UIManager<UIProjectManager, ProjectManager, MANAGER_DESCRIPTION_NAMESPACE::MANAGER_PROJECT_UI, MANAGER_DESCRIPTION_NAMESPACE::MANAGER_PROJECT>
{

		Q_OBJECT

	public:

		typedef std::map<NewGeneMainWindow*, std::unique_ptr<UIInputProject> > InputProjects;
		typedef std::map<NewGeneMainWindow*, std::unique_ptr<UIOutputProject> > OutputProjects;

		explicit UIProjectManager( QObject * parent = 0 );

		void LoadOpenProjects(UIMessager & messager, NewGeneMainWindow*);

	signals:

	public slots:

	private:

		InputProjects input_projects;
		OutputProjects output_projects;

};

#endif // UIPROJECTMANAGER_H
