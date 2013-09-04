#include "globals.h"
#include <QObject>
#include "uiprojectmanager.h"
#include "uimodelmanager.h"
#include "uisettingsmanager.h"
#include "uidocumentmanager.h"
#include "uistatusmanager.h"
#include "uiloggingmanager.h"
#include "uiallglobalsettings_list.h"
#include "../../../../NewGeneBackEnd/Settings/GlobalSettings_list.h"
#include "../../../../NewGeneBackEnd/Settings/InputProjectSettings_list.h"
#include "../../../../NewGeneBackEnd/Settings/OutputProjectSettings_list.h"
#include "../../../../NewGeneBackEnd/Settings/InputModelSettings_list.h"
#include "../../../../NewGeneBackEnd/Settings/OutputModelSettings_list.h"
#include "../newgenewidget.h"
#include "uimessagersingleshot.h"
#include "newgenemainwindow.h"

UIProjectManager::UIProjectManager( QObject * parent )
	: QObject(parent)
	, UIManager()
	, EventLoopThreadManager<UI_PROJECT_MANAGER>(number_worker_threads)
{

	// *************************************************************************
	// All Managers are instantiated AFTER the application event loop is running
	// *************************************************************************

	InitializeEventLoop(this);

}

UIProjectManager::~UIProjectManager()
{

	for_each(input_tabs.begin(), input_tabs.end(), [this](std::pair<NewGeneMainWindow * const, InputProjectTabs> & windows)
	{

		InputProjectTabs & tabs = windows.second;
		for_each(tabs.begin(), tabs.end(), [this](InputProjectTab & tab)
		{
			UIInputProject * project_ptr = static_cast<UIInputProject*>(tab.second.release());
			project_ptr->deleteLater();
		});

	});

	for_each(output_tabs.begin(), output_tabs.end(), [this](std::pair<NewGeneMainWindow * const, OutputProjectTabs> & windows)
	{

		OutputProjectTabs & tabs = windows.second;
		for_each(tabs.begin(), tabs.end(), [this](OutputProjectTab & tab)
		{
			UIOutputProject * project_ptr = static_cast<UIOutputProject*>(tab.second.release());
			project_ptr->deleteLater();
		});

	});

}

void UIProjectManager::EndAllLoops()
{

	for_each(input_tabs.begin(), input_tabs.end(), [this](std::pair<NewGeneMainWindow * const, InputProjectTabs> & windows)
	{

		InputProjectTabs & tabs = windows.second;
		for_each(tabs.begin(), tabs.end(), [this](InputProjectTab & tab)
		{
			ProjectPaths & paths = tab.first;
			UIInputProject * project_ptr = static_cast<UIInputProject*>(tab.second.release());
			emit UpdateInputConnections(RELEASE_CONNECTIONS_INPUT_PROJECT, project_ptr); // blocks, because all connections are in NewGeneWidget which are all associated with the UI event loop
			project_ptr->model().EndLoopAndBackgroundPool(); // blocks
			project_ptr->modelSettings().EndLoopAndBackgroundPool(); // blocks
			project_ptr->projectSettings().EndLoopAndBackgroundPool(); // blocks
			project_ptr->EndLoopAndBackgroundPool(); // blocks
		});

	});

	for_each(output_tabs.begin(), output_tabs.end(), [this](std::pair<NewGeneMainWindow * const, OutputProjectTabs> & windows)
	{

		OutputProjectTabs & tabs = windows.second;
		for_each(tabs.begin(), tabs.end(), [this](OutputProjectTab & tab)
		{
			ProjectPaths & paths = tab.first;
			UIOutputProject * project_ptr = static_cast<UIOutputProject*>(tab.second.release());
			emit this->UpdateOutputConnections(RELEASE_CONNECTIONS_OUTPUT_PROJECT, project_ptr); // blocks, because all connections are in NewGeneWidget which are all associated with the UI event loop
			project_ptr->model().EndLoopAndBackgroundPool(); // blocks
			project_ptr->modelSettings().EndLoopAndBackgroundPool(); // blocks
			project_ptr->projectSettings().EndLoopAndBackgroundPool(); // blocks
			project_ptr->EndLoopAndBackgroundPool(); // blocks
		});

	});

	EndLoopAndBackgroundPool();

};

void UIProjectManager::LoadOpenProjects(NewGeneMainWindow* mainWindow)
{
	UIMessager messager;
	UIMessagerSingleShot messager_(messager);
	InputProjectFilesList::instance input_project_list = InputProjectFilesList::get(messager);
	OutputProjectFilesList::instance output_project_list = OutputProjectFilesList::get(messager);

	if (input_project_list->files.size() == 1)
	{

		boost::filesystem::path input_project_settings_path = input_project_list->files[0];

		bool create_new_instance = false;

		if (input_tabs.find(mainWindow) == input_tabs.cend())
		{
			create_new_instance = true;
		}
		else
		{
			InputProjectTabs & tabs = input_tabs[mainWindow];
			for_each(tabs.begin(), tabs.end(), [](InputProjectTab & tab)
			{
				ProjectPaths & paths = tab.first;
				UIInputProject * project_ptr = static_cast<UIInputProject*>(tab.second.get());
			});
		}

		if (create_new_instance)
		{

			// Internally creates both an instance of UI-layer project settings, and an instance of backend-layer project settings
			// via SettingsRepositoryFactory
			std::shared_ptr<UIInputProjectSettings> project_settings(new UIInputProjectSettings(messager, input_project_settings_path));
			project_settings->WriteSettingsToFile(messager); // Writes default settings for those settings not already present

			// Internally creates an instance of backend-layer model settings via SettingsRepositoryFactory
			auto path_to_model_settings = InputProjectPathToModel::get(messager, project_settings->getBackendSettings());
			std::shared_ptr<UIInputModelSettings> model_settings(new UIInputModelSettings(messager, path_to_model_settings->getPath()));
			model_settings->WriteSettingsToFile(messager); // Writes default settings for those settings not already present

			// Backend model does not know its settings, because multiple settings might point to the same model.
			auto path_to_model_database = InputModelPathToDatabase::get(messager, model_settings->getBackendSettings());
			std::shared_ptr<InputModel> backend_model(ModelFactory<InputModel>()(messager, path_to_model_database->getPath()));
			std::shared_ptr<UIInputModel> project_model(new UIInputModel(messager, backend_model));

			input_tabs[mainWindow].push_back(std::make_pair(ProjectPaths(input_project_settings_path, path_to_model_settings->getPath(), path_to_model_database->getPath()),
															std::unique_ptr<UIInputProject>(new UIInputProject(project_settings, model_settings, project_model, mainWindow))));

			UIInputProject * project = getActiveUIInputProject();

			if (!project)
			{
				boost::format msg("NULL input project during attempt to instantiate project.");
				messager.AppendMessage(new MessagerWarningMessage(MESSAGER_MESSAGE__PROJECT_IS_NULL, msg.str()));
				return;
			}

			project->InitializeEventLoop(project); // cannot use 'this' in base class with multiple inheritance
			project->model().InitializeEventLoop(&project->model()); // cannot use 'this' in base class with multiple inheritance
			project->modelSettings().InitializeEventLoop(&project->modelSettings()); // cannot use 'this' in base class with multiple inheritance
			project->projectSettings().InitializeEventLoop(&project->projectSettings()); // cannot use 'this' in base class with multiple inheritance

			project_settings->UpdateConnections();
			model_settings->UpdateConnections();
			project_model->UpdateConnections();
			project->UpdateConnections();

			emit UpdateInputConnections(ESTABLISH_CONNECTIONS_INPUT_PROJECT, project); // blocks, because all connections are in NewGeneWidget which are all associated with the UI event loop

			emit LoadFromDatabase(&project->model());

		}

	}

	if (output_project_list->files.size() == 1)
	{

		boost::filesystem::path output_project_settings_path = output_project_list->files[0];

		bool create_new_instance = false;

		if (output_tabs.find(mainWindow) == output_tabs.cend())
		{
			create_new_instance = true;
		}
		else
		{
			OutputProjectTabs & tabs = output_tabs[mainWindow];
			for_each(tabs.begin(), tabs.end(), [](OutputProjectTab & tab)
			{
				ProjectPaths & paths = tab.first;
				UIOutputProject * project_ptr = static_cast<UIOutputProject*>(tab.second.get());
			});
		}

		if (create_new_instance)
		{

			// Internally creates both an instance of UI-layer project settings, and an instance of backend-layer project settings
			// via SettingsRepositoryFactory
			std::shared_ptr<UIOutputProjectSettings> project_settings(new UIOutputProjectSettings(messager, output_project_settings_path));
			project_settings->WriteSettingsToFile(messager); // Writes default settings for those settings not already present

			// Internally creates an instance of backend-layer model settings via SettingsRepositoryFactory
			auto path_to_model_settings = OutputProjectPathToModel::get(messager, project_settings->getBackendSettings());
			std::shared_ptr<UIOutputModelSettings> model_settings(new UIOutputModelSettings(messager, path_to_model_settings->getPath()));
			model_settings->WriteSettingsToFile(messager); // Writes default settings for those settings not already present

			// The input model and settings are necessary in order to instantiate the output model
			UIInputProject * input_project = getActiveUIInputProject();
			if (!input_project)
			{
				boost::format msg("NULL input project during attempt to instantiate output project.");
				messager.AppendMessage(new MessagerWarningMessage(MESSAGER_MESSAGE__PROJECT_IS_NULL, msg.str()));
				return;
			}

			// Backend model does not know its settings, because multiple settings might point to the same model.
			auto path_to_model_database = OutputModelPathToDatabase::get(messager, model_settings->getBackendSettings());
			std::shared_ptr<OutputModel> backend_model(ModelFactory<OutputModel>()(messager, path_to_model_database->getPath(), std::dynamic_pointer_cast<InputModelSettings>(input_project->backend().modelSettingsSharedPtr()), input_project->backend().modelSharedPtr()));
			std::shared_ptr<UIOutputModel> project_model(new UIOutputModel(messager, backend_model, mainWindow));

			output_tabs[mainWindow].push_back(std::make_pair(ProjectPaths(output_project_settings_path, path_to_model_settings->getPath(), path_to_model_database->getPath()),
															std::unique_ptr<UIOutputProject>(new UIOutputProject(project_settings, model_settings, project_model))));

			UIOutputProject * project = getActiveUIOutputProject();

			if (!project)
			{
				boost::format msg("NULL output project during attempt to instantiate project.");
				messager.AppendMessage(new MessagerWarningMessage(MESSAGER_MESSAGE__PROJECT_IS_NULL, msg.str()));
				return;
			}

			project->InitializeEventLoop(project); // cannot use 'this' in base class with multiple inheritance
			project->model().InitializeEventLoop(&project->model()); // cannot use 'this' in base class with multiple inheritance
			project->modelSettings().InitializeEventLoop(&project->modelSettings()); // cannot use 'this' in base class with multiple inheritance
			project->projectSettings().InitializeEventLoop(&project->projectSettings()); // cannot use 'this' in base class with multiple inheritance

			project_settings->UpdateConnections();
			model_settings->UpdateConnections();
			project_model->UpdateConnections();
			project->UpdateConnections();

			emit UpdateOutputConnections(ESTABLISH_CONNECTIONS_OUTPUT_PROJECT, project); // blocks, because all connections are in NewGeneWidget which are all associated with the UI event loop

		}

	}

}

UIInputProject * UIProjectManager::getActiveUIInputProject()
{

	// For now, just get the first main window
	if (input_tabs.size() == 0)
	{
		return nullptr;
	}

	InputProjectTabs & tabs = input_tabs.begin()->second;

	if (tabs.size() == 0)
	{
		return nullptr;
	}

	InputProjectTab & tab = *tabs.begin();

	UIInputProject * project = static_cast<UIInputProject *>(tab.second.get());

	return project;

}

UIOutputProject * UIProjectManager::getActiveUIOutputProject()
{

	// For now, just get the first main window
	if (output_tabs.size() == 0)
	{
		return nullptr;
	}

	OutputProjectTabs & tabs = output_tabs.begin()->second;

	if (tabs.size() == 0)
	{
		return nullptr;
	}

	OutputProjectTab & tab = *tabs.begin();

	UIOutputProject * project = static_cast<UIOutputProject *>(tab.second.get());

	return project;

}

void UIProjectManager::SignalMessageBox(STD_STRING msg)
{

	QMessageBox msgBox;
	msgBox.setText( msg.c_str() );
	msgBox.exec();

}

void UIProjectManager::DoneLoadingFromDatabase(UI_INPUT_MODEL_PTR model_)
{

	UIMessagerSingleShot messager;

	if (!getActiveUIInputProject()->is_model_equivalent(messager.get(), model_))
	{
		messager.get().AppendMessage(new MessagerWarningMessage(MESSAGER_MESSAGE__INPUT_MODELS_DO_NOT_MATCH, "Input model has completed loading from database, but does not match current input model."));
		return;
	}

	if (!model_->loaded())
	{
		messager.get().AppendMessage(new MessagerWarningMessage(MESSAGER_MESSAGE__INPUT_MODEL_NOT_LOADED, "Input model has completed loading from database, but is marked as not loaded."));
		return;
	}

	emit LoadFromDatabase(&getActiveUIOutputProject()->model());

}

void UIProjectManager::DoneLoadingFromDatabase(UI_OUTPUT_MODEL_PTR model_)
{

	UIMessagerSingleShot messager;

	if (!getActiveUIOutputProject()->is_model_equivalent(messager.get(), model_))
	{
		messager.get().AppendMessage(new MessagerWarningMessage(MESSAGER_MESSAGE__OUTPUT_MODELS_DO_NOT_MATCH, "Output model has completed loading from database, but does not match current output model."));
		return;
	}

	if (!model_->loaded())
	{
		messager.get().AppendMessage(new MessagerWarningMessage(MESSAGER_MESSAGE__OUTPUT_MODEL_NOT_LOADED, "Output model has completed loading from database, but is marked as not loaded."));
		return;
	}

	getActiveUIInputProject()->DoRefreshAllWidgets();
	getActiveUIOutputProject()->DoRefreshAllWidgets();

}
