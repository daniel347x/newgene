#ifndef UIPROJECTMANAGER_H
#define UIPROJECTMANAGER_H

#include "uiinputproject.h"
#include "uioutputproject.h"
#include "uiinputprojectsettings.h"
#include "uioutputprojectsettings.h"
#include "uiinputmodel.h"
#include "uioutputmodel.h"
#include "Base/uiproject.h"
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

		//  (list to be maintained by UIProjectManager)
		//  UIProject:
		//
		//      These represent a tab in the user interface (Create Output or Manage Data).
		//
		//      The UIProject owns or contains the following components:
		//
		//          Event loop:
		//
		//              As a tab, they own a QThread event loop to allow for communication
		//              between the tab and the rest of the application.
		//
		//          (list to be maintained by UIProjectManager)
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
		//          (list to be maintained by UIProjectManager)
		//          UIModelSettings:
		//
		//              UIModelSettings represents a single model settings file on disk.
		//
		//                  Event loop:
		//                      The UIModelSettings internally owns a QThread event loop to allow for
		//                      a single point of communication in the application
		//                      when model settings in the settings file change.
		//
		//                  (No UI-layer Model Settings exists)
		//                  Backend Model Settings:
		//
		//                        Internally, the UIModelSettings class instantiates an instance of
		//                        a backend Model Settings class in the form of a SettingsRepository instance.
		//
		//                        The backend project possesses a shared_ptr to the backend Model Settings instance.
		//
		//          (list to be maintained by UIProjectManager)
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
		//                  (list to be maintained by UIProjectManager)
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
		//              (list to be maintained by UIProjectManager)
		//              Backend Model instance:
		//
		//                  The backend Project possesses a shared_ptr to the Model Settings instance.
		//                  The backend Project possesses a shared_ptr to the backend Project Settings instance.
		//                  The backend Project possesses a shared_ptr to the Model instance.
		//
		//              The UIProject owns the backend project with a unique_ptr via RAII.
		//

	public:

		enum UPDATE_CONNECTIONS_TYPE
		{
			  RELEASE_CONNECTIONS_INPUT_PROJECT
			, ESTABLISH_CONNECTIONS_INPUT_PROJECT
			, RELEASE_CONNECTIONS_OUTPUT_PROJECT
			, ESTABLISH_CONNECTIONS_OUTPUT_PROJECT
		};

#		if 0 // possibly to be implemented in a later version of NewGene to manage multiple tabs for input or output
		typedef std::vector<std::shared_ptr<UIInputProject>> UIInputProjectsList;
		typedef std::vector<std::shared_ptr<UIOutputProject>> UIOutputProjectsList;

		typedef std::vector<std::shared_ptr<UIInputProjectSettings>> UIInputProjectSettingsList;
		typedef std::vector<std::shared_ptr<UIOutputProjectSettings>> UIOutputProjectSettingsList;

		typedef std::vector<std::shared_ptr<UIInputModelSettings>> UIInputModelSettingsList;
		typedef std::vector<std::shared_ptr<UIOutputModelSettings>> UIOutputModelSettingsList;

		typedef std::vector<std::shared_ptr<UIInputModel>> UIInputModelsList;
		typedef std::vector<std::shared_ptr<UIOutputModel>> UIOutputModelsList;

		typedef std::vector<std::shared_ptr<InputModel>> BackendInputModelsList;
		typedef std::vector<std::shared_ptr<OutputModel>> BackendOutputModelsList;

		typedef std::map<NewGeneMainWindow*, UIInputProjectsList> InputProjectsMap;
		typedef std::map<NewGeneMainWindow*, UIOutputProjectsList> OutputProjectsMap;
#		endif

		typedef std::tuple<boost::filesystem::path, boost::filesystem::path, boost::filesystem::path> ProjectPaths; // project settings, model settings, model database

		template<typename BACKEND_PROJECT_CLASS, typename UI_PROJECT_SETTINGS_CLASS, typename UI_MODEL_SETTINGS_CLASS, typename UI_MODEL_CLASS, WORK_QUEUE_THREAD_LOOP_CLASS_ENUM UI_THREAD_LOOP_CLASS_ENUM>
		struct ProjectTab
		{
			typedef std::pair<ProjectPaths, std::unique_ptr<UIProject<BACKEND_PROJECT_CLASS, UI_PROJECT_SETTINGS_CLASS, UI_MODEL_SETTINGS_CLASS, UI_MODEL_CLASS, UI_THREAD_LOOP_CLASS_ENUM>>> type;
		};

		template<typename BACKEND_PROJECT_CLASS, typename UI_PROJECT_SETTINGS_CLASS, typename UI_MODEL_SETTINGS_CLASS, typename UI_MODEL_CLASS, WORK_QUEUE_THREAD_LOOP_CLASS_ENUM UI_THREAD_LOOP_CLASS_ENUM>
		struct ProjectTabs
		{
			typedef std::vector<typename ProjectTab<BACKEND_PROJECT_CLASS, UI_PROJECT_SETTINGS_CLASS, UI_MODEL_SETTINGS_CLASS, UI_MODEL_CLASS, UI_THREAD_LOOP_CLASS_ENUM>::type> type;
		};

		template<typename BACKEND_PROJECT_CLASS, typename UI_PROJECT_SETTINGS_CLASS, typename UI_MODEL_SETTINGS_CLASS, typename UI_MODEL_CLASS, WORK_QUEUE_THREAD_LOOP_CLASS_ENUM UI_THREAD_LOOP_CLASS_ENUM>
		struct Tabs
		{
			typedef std::map<NewGeneMainWindow *, typename ProjectTabs<BACKEND_PROJECT_CLASS, UI_PROJECT_SETTINGS_CLASS, UI_MODEL_SETTINGS_CLASS, UI_MODEL_CLASS, UI_THREAD_LOOP_CLASS_ENUM>::type> type;
		};

		explicit UIProjectManager( QObject * parent = 0 );
		~UIProjectManager();

		void LoadOpenProjects(NewGeneMainWindow*);

		UIInputProject * getActiveUIInputProject();
		UIOutputProject * getActiveUIOutputProject();

	signals:
		void UpdateInputConnections(UIProjectManager::UPDATE_CONNECTIONS_TYPE connection_type, UIInputProject * project);
		void UpdateOutputConnections(UIProjectManager::UPDATE_CONNECTIONS_TYPE connection_type, UIOutputProject * project);
		void LoadModel(void *);

	public slots:

	private:

	typedef ProjectTabs<InputProject, UIInputProjectSettings, UIInputModelSettings, UIInputModel, UI_INPUT_PROJECT>::type InputProjectTabs;
	typedef ProjectTabs<OutputProject, UIOutputProjectSettings, UIOutputModelSettings, UIOutputModel, UI_OUTPUT_PROJECT>::type OutputProjectTabs;

	typedef ProjectTab<InputProject, UIInputProjectSettings, UIInputModelSettings, UIInputModel, UI_INPUT_PROJECT>::type InputProjectTab;
	typedef ProjectTab<OutputProject, UIOutputProjectSettings, UIOutputModelSettings, UIOutputModel, UI_OUTPUT_PROJECT>::type OutputProjectTab;

	typedef Tabs<InputProject, UIInputProjectSettings, UIInputModelSettings, UIInputModel, UI_INPUT_PROJECT>::type InputTabs;
	typedef Tabs<OutputProject, UIOutputProjectSettings, UIOutputModelSettings, UIOutputModel, UI_OUTPUT_PROJECT>::type OutputTabs;

	InputTabs input_tabs;
	OutputTabs output_tabs;

};

#endif // UIPROJECTMANAGER_H
