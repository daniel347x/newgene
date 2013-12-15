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
#include "../../../NewGeneBackEnd/Project/ProjectManager.h"
#include "eventloopthreadmanager.h"
#include "uiprojectmanagerworkqueue.h"
#include <memory>
#include <vector>
#include <map>
#include <tuple>

class NewGeneMainWindow;

class UIProjectManager : public QObject,
						 public UIManager<UIProjectManager, ProjectManager, MANAGER_DESCRIPTION_NAMESPACE::MANAGER_PROJECT_UI, MANAGER_DESCRIPTION_NAMESPACE::MANAGER_PROJECT>,
						 public EventLoopThreadManager<UI_PROJECT_MANAGER>
{

		Q_OBJECT

		static int const number_worker_threads = 1;

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
		//                        The backend project possesses a shared_ptr to the backend Project Settings instance.
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
		//                        The UIModel's QThread event loop manages *two* Boost thread worker pools
		//                        (all other UI objects that manage a QThread event loop manage 1 corresponding
		//                        Boost worker thread pool).
		//                            (1) Model Action worker thread pool to handle updates to the in-memory
		//                                cache of the model data.
		//                            (2) Model Database worker thread pool to handle reading/writing to the
		//                                database itself (and correspondingly, writing/reading from the
		//                                cache of the model data).
		//
		//                  (list to be maintained by UIProjectManager)
		//                  Backend Model instance:
		//
		//                      The backend model instance processes and updates the model data (in worker thread pool #1).
		//                      The backend model instance reads, writes, and caches the data in the database itself (in worker thread pool #2).
		//
		//                      The UIModel possesses a shared_ptr to the backend model instance.
		//
		//          Backend Project:
		//
		//              Convenience class accessible in the backend library, corresponding to, and owned by, the UIProject instance
		//              (and tied to the lifetime of the UIProject instance; i.e., to the lifetime of the tab in the user interface).
		//
		//              (list to be maintained by UIProjectManager)
		//              Backend Model instance:
		//
		//                  The backend Project possesses a shared_ptr to the Model Settings instance.
		//                  The backend Project possesses a shared_ptr to the backend Project Settings instance.
		//                  The backend Project possesses a shared_ptr to the Model instance.
		//
		//              As noted, the UIProject owns the backend project with a unique_ptr via RAII.
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

		// ************************************************************************************************************************************* //
		// Clang workaround: http://stackoverflow.com/questions/20583591/clang-only-a-pairpath-path-can-be-emplaced-into-a-vector-so-can-a-pairuniq
		// ************************************************************************************************************************************* //
		struct ProjectPaths
		{
			ProjectPaths(boost::filesystem::path const & project_settings_path_, boost::filesystem::path const & model_settings_path_, boost::filesystem::path const& model_database_path_)
				: project_settings_path(project_settings_path_)
				, model_settings_path(model_settings_path_)
				, model_database_path(model_database_path_)
			{}
			boost::filesystem::path project_settings_path;
			boost::filesystem::path model_settings_path;
			boost::filesystem::path model_database_path;
		};
		//typedef std::tuple<boost::filesystem::path, boost::filesystem::path, boost::filesystem::path> ProjectPaths; // project settings, model settings, model database

		// ************************************************************************************************************************************* //
		// Clang workaround: http://stackoverflow.com/questions/20583591/clang-only-a-pairpath-path-can-be-emplaced-into-a-vector-so-can-a-pairuniq
		// ************************************************************************************************************************************* //
		template<typename BACKEND_PROJECT_CLASS, typename UI_PROJECT_SETTINGS_CLASS, typename UI_MODEL_SETTINGS_CLASS, typename UI_MODEL_CLASS, WORK_QUEUE_THREAD_LOOP_CLASS_ENUM UI_THREAD_LOOP_CLASS_ENUM>
		struct ProjectTabContents
		{
			ProjectPaths paths;
			std::unique_ptr<UIProject<BACKEND_PROJECT_CLASS, UI_PROJECT_SETTINGS_CLASS, UI_MODEL_SETTINGS_CLASS, UI_MODEL_CLASS, UI_THREAD_LOOP_CLASS_ENUM>> project;
			ProjectTabContents(ProjectPaths const && paths_, std::unique_ptr<UIProject<BACKEND_PROJECT_CLASS, UI_PROJECT_SETTINGS_CLASS, UI_MODEL_SETTINGS_CLASS, UI_MODEL_CLASS, UI_THREAD_LOOP_CLASS_ENUM>> && project_)
				: paths(std::move(paths_))
				, project(std::move(project_))
			{
			}
			ProjectTabContents(ProjectTabContents && rhs)
				: paths(rhs.paths)
				, project(rhs.project)
			{}
		};

		template<typename BACKEND_PROJECT_CLASS, typename UI_PROJECT_SETTINGS_CLASS, typename UI_MODEL_SETTINGS_CLASS, typename UI_MODEL_CLASS, WORK_QUEUE_THREAD_LOOP_CLASS_ENUM UI_THREAD_LOOP_CLASS_ENUM>
		struct ProjectTab
		{
			//typedef std::pair<ProjectPaths, std::unique_ptr<UIProject<BACKEND_PROJECT_CLASS, UI_PROJECT_SETTINGS_CLASS, UI_MODEL_SETTINGS_CLASS, UI_MODEL_CLASS, UI_THREAD_LOOP_CLASS_ENUM>>> type;
			typedef ProjectTabContents<BACKEND_PROJECT_CLASS, UI_PROJECT_SETTINGS_CLASS, UI_MODEL_SETTINGS_CLASS, UI_MODEL_CLASS, UI_THREAD_LOOP_CLASS_ENUM> type;
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

		void LoadOpenProjects(NewGeneMainWindow*, QObject*);

		UIInputProject * getActiveUIInputProject();
		UIOutputProject * getActiveUIOutputProject();

		void DoRefreshAllInputModelWidgets();
		void DoRefreshAllOutputModelWidgets();

		void EndAllLoops();

	signals:

		void UpdateInputConnections(UIProjectManager::UPDATE_CONNECTIONS_TYPE connection_type, UIInputProject * project);
		void UpdateOutputConnections(UIProjectManager::UPDATE_CONNECTIONS_TYPE connection_type, UIOutputProject * project);
		void LoadFromDatabase(UI_INPUT_MODEL_PTR);
		void LoadFromDatabase(UI_OUTPUT_MODEL_PTR);

	public slots:

		void SignalMessageBox(STD_STRING);
		void DoneLoadingFromDatabase(UI_INPUT_MODEL_PTR);
		void DoneLoadingFromDatabase(UI_OUTPUT_MODEL_PTR);
		void OpenOutputDataset(STD_STRING, QObject *);
		void CloseCurrentOutputDataset();
		void OpenInputDataset(STD_STRING, QObject *);
		void CloseCurrentInputDataset();

	private:

		typedef ProjectTabs<InputProject, UIInputProjectSettings, UIInputModelSettings, UIInputModel, UI_INPUT_PROJECT>::type InputProjectTabs;
		typedef ProjectTabs<OutputProject, UIOutputProjectSettings, UIOutputModelSettings, UIOutputModel, UI_OUTPUT_PROJECT>::type OutputProjectTabs;

		typedef ProjectTab<InputProject, UIInputProjectSettings, UIInputModelSettings, UIInputModel, UI_INPUT_PROJECT>::type InputProjectTab;
		typedef ProjectTab<OutputProject, UIOutputProjectSettings, UIOutputModelSettings, UIOutputModel, UI_OUTPUT_PROJECT>::type OutputProjectTab;

		typedef Tabs<InputProject, UIInputProjectSettings, UIInputModelSettings, UIInputModel, UI_INPUT_PROJECT>::type InputTabs;
		typedef Tabs<OutputProject, UIOutputProjectSettings, UIOutputModelSettings, UIOutputModel, UI_OUTPUT_PROJECT>::type OutputTabs;

		InputTabs input_tabs;
		OutputTabs output_tabs;

	protected:

		WorkQueueManager<UI_PROJECT_MANAGER> * InstantiateWorkQueue(void * ui_object, bool = false)
		{
			UIProjectManagerWorkQueue * work_queue = new UIProjectManagerWorkQueue();
			work_queue->SetUIObject(reinterpret_cast<UIProjectManager*>(ui_object));
			work_queue->SetConnections();
			return work_queue;
		}

	private:

		bool RawOpenInputProject(UIMessager & messager, boost::filesystem::path const & input_project_settings_path, QObject * mainWindowObject);
		bool RawOpenOutputProject(UIMessager & messager, boost::filesystem::path const & output_project_settings_path, QObject * mainWindowObject);

		void RawCloseInputProject(UIInputProject * input_project);
		void RawCloseOutputProject(UIOutputProject * output_project);

};

#endif // UIPROJECTMANAGER_H
