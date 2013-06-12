#ifndef UIPROJECTMANAGER_H
#define UIPROJECTMANAGER_H

#include "uiinputproject.h"
#include "uioutputproject.h"
#include "uiinputprojectsettings.h"
#include "uioutputprojectsettings.h"
#include "uiinputmodel.h"
#include "uioutputmodel.h"
#include "Infrastructure/uimanager.h"
#include "Infrastructure/Messager/uimessager.h"
#include "..\..\..\NewGeneBackEnd\Project\ProjectManager.h"
#include <memory>
#include <vector>
#include <map>
#include <tuple>

class NewGeneMainWindow;

class UIProjectManager : public QObject, public UIManager<UIProjectManager, ProjectManager, MANAGER_DESCRIPTION_NAMESPACE::MANAGER_PROJECT_UI, MANAGER_DESCRIPTION_NAMESPACE::MANAGER_PROJECT>
{

		Q_OBJECT

	public:

		//  (list maintained by UIProjectManager)
		//  UIProject:
		//
		//      These represent a tab in the user interface (Create Output or Manage Data).
		//
		//          Event loop:
		//
		//              As a tab, they own a QThread event loop to allow for communication
		//              between the tab and the rest of the application.
		//
		//          (list maintained by UIProjectManager)
		//          UIProjectSettings:
		//
		//              UIProjectSettings represents a single project settings file on disk.
		//
		//                  Event loop:
		//                      The UIProjectSettings internally owns a QThread event loop to allow for
		//                      a single point of communication in the application
		//                      when project settings in the settings file change.
		//
		//                  UI-layer Project Settings:
		//                  Backend Project Settings:
		//
		//                        Internally, the UIProjectSettings decomposes the settings in the file
		//                        into UI-layer settings, and backend settings, instantiating
		//                        a separate class instance for each in the form of a SettingsRepository instance.
		//
		//                        The UIProjectSettings possesses a shared_ptr to the UI-layer Project Settings instance.
		//
		//                        The UIProjectSettings possesses a shared_ptr to the backend Project Settings instance.
		//
		//          (list maintained by UIProjectManager)
		//          ModelSettings:
		//
		//              ModelSettings represents a single model settings file on disk.
		//
		//                        The backend Model instance, below, possesses a shared_ptr to the backend Model Settings instance.
		//
		//          (list maintained by UIProjectManager)
		//          UIModel:
		//
		//              The purpose of the UIModel is to represent the actual database for the model.
		//
		//                  Event loop:
		//
		//                      The UIModel owns (yet another) QThread event loop to manage
		//                      a single point of communication in the application when
		//                      any data in the database changes.
		//
		//                      The UIModel owns the QThread event loop via RAII.
		//
		//                  (list maintained by UIProjectManager)
		//                  Backend Model instance:
		//
		//                      The backend model instance reads, writes, and caches the data in the database itself.
		//
		//                      The UIModel possesses a shared_ptr to the backend model instance.
		//
		//          Backend Project:
		//
		//              Convenience class accessible in the backend library.
		//
		//              (list maintained by UIProjectManager)
		//              Backend Model instance:
		//
		//                  The backend Project possesses a shared_ptr to the Model Settings instance.
		//                  The backend Project possesses a shared_ptr to the backend Project Settings instance.
		//                  The backend Project possesses a shared_ptr to the Model instance.
		//
		//              The UIProject owns the backend project with a unique_ptr via RAII.
		//
		typedef std::vector<std::shared_ptr<UIInputProject>> UIInputProjectsList;
		typedef std::vector<std::shared_ptr<UIOutputProject>> UIOutputProjectsList;

		typedef std::vector<std::shared_ptr<UIInputProjectSettings>> UIInputProjectSettingsList;
		typedef std::vector<std::shared_ptr<UIOutputProjectSettings>> UIOutputProjectSettingsList;

		typedef std::vector<std::shared_ptr<InputModelSettings>> InputModelSettingsList;
		typedef std::vector<std::shared_ptr<OutputModelSettings>> OutputModelSettingsList;

		typedef std::vector<std::shared_ptr<UIInputModel>> UIInputModelsList;
		typedef std::vector<std::shared_ptr<UIOutputModel>> UIOutputModelsList;

		typedef std::vector<std::shared_ptr<InputModel>> BackendInputModelsList;
		typedef std::vector<std::shared_ptr<OutputModel>> BackendOutputModelsList;

		typedef std::map<NewGeneMainWindow*, UIInputProjectsList> InputProjectsMap;
		typedef std::map<NewGeneMainWindow*, UIOutputProjectsList> OutputProjectsMap;

		//typedef std::tuple<UIProjectSettings*

		explicit UIProjectManager( QObject * parent = 0 );
		~UIProjectManager();

		void LoadOpenProjects(NewGeneMainWindow*);
		void TriggerActiveInputProject(NewGeneMainWindow* /* to be filled in */);
		void TriggerActiveOutputProject(NewGeneMainWindow* /* to be filled in */);
		void ConnectTrigger(QWidget * w);

	signals:
		void TriggerInputProject();
		void TriggerOutputProject();

	public slots:

	private:

		std::unique_ptr<UIInputProject> input_project;
		//InputProjects input_projects;
		//OutputProjects output_projects;

};

#endif // UIPROJECTMANAGER_H
