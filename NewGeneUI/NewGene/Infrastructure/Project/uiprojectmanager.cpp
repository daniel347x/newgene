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

#include <QFileDialog>
#include <QCoreApplication>

UIProjectManager::UIProjectManager( QObject * parent, UIMessager & messager )
	: QObject(parent)
	, loading(false)
	, UIManager(messager)
	, EventLoopThreadManager<UI_PROJECT_MANAGER>(messager, number_worker_threads)
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
			// Release the pointer so that when the "tabs" map is destroyed, its destructor will do the job
			UIInputProject * project_ptr = static_cast<UIInputProject*>(tab.project.release());
			RawCloseInputProject(project_ptr);
		});

	});

	for_each(output_tabs.begin(), output_tabs.end(), [this](std::pair<NewGeneMainWindow * const, OutputProjectTabs> & windows)
	{

		OutputProjectTabs & tabs = windows.second;
		for_each(tabs.begin(), tabs.end(), [this](OutputProjectTab & tab)
		{
			UIOutputProject * project_ptr = static_cast<UIOutputProject*>(tab.project.release());
			RawCloseOutputProject(project_ptr);
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

			//ProjectPaths & paths = tab.first;
			UIInputProject * project_ptr = static_cast<UIInputProject*>(tab.project.release());
			RawCloseInputProject(project_ptr);

		});

	});

	for_each(output_tabs.begin(), output_tabs.end(), [this](std::pair<NewGeneMainWindow * const, OutputProjectTabs> & windows)
	{

		OutputProjectTabs & tabs = windows.second;
		for_each(tabs.begin(), tabs.end(), [this](OutputProjectTab & tab)
		{

			//ProjectPaths & paths = tab.first;
			UIOutputProject * project_ptr = static_cast<UIOutputProject*>(tab.project.release());
			RawCloseOutputProject(project_ptr);

		});

	});

	EndLoopAndBackgroundPool();

};

void UIProjectManager::LoadOpenProjects(NewGeneMainWindow* mainWindow, QObject * mainWindowObject)
{

	loading = true;

	UIMessager messager;
	UIMessagerSingleShot messager_(messager);
	InputProjectFilesList::instance input_project_list = InputProjectFilesList::get(messager);

	connect(mainWindowObject, SIGNAL(SignalCloseCurrentInputDataset()), this, SLOT(CloseCurrentInputDataset()));
	connect(mainWindowObject, SIGNAL(SignalCloseCurrentOutputDataset()), this, SLOT(CloseCurrentOutputDataset()));
	connect(mainWindowObject, SIGNAL(SignalOpenInputDataset(STD_STRING, QObject *)), this, SLOT(OpenInputDataset(STD_STRING, QObject *)));
	connect(mainWindowObject, SIGNAL(SignalOpenOutputDataset(STD_STRING, QObject *)), this, SLOT(OpenOutputDataset(STD_STRING, QObject *)));
	connect(mainWindowObject, SIGNAL(SignalSaveCurrentInputDatasetAs(STD_STRING, QObject *)), this, SLOT(SaveCurrentInputDatasetAs(STD_STRING, QObject *)));
	connect(mainWindowObject, SIGNAL(SignalSaveCurrentOutputDatasetAs(STD_STRING, QObject *)), this, SLOT(SaveCurrentOutputDatasetAs(STD_STRING, QObject *)));

	bool success = false;

	if (input_project_list->files.size() == 0)
	{
		boost::format msg_title("Open input project at default location?");
		boost::format msg_text("You have no input project open.  Would you like to open the project at the default location?");
		QMessageBox::StandardButton reply;
		reply = QMessageBox::question(nullptr, QString(msg_title.str().c_str()), QString(msg_text.str().c_str()), QMessageBox::StandardButtons(QMessageBox::Yes | QMessageBox::No));
		if (reply == QMessageBox::No)
		{
			return;
		}

		boost::filesystem::path input_project_path = settingsManagerUI().ObtainGlobalPath(QStandardPaths::DocumentsLocation, "NewGene/Input", NewGeneFileNames::defaultInputProjectFileName);
		if (input_project_path != boost::filesystem::path())
		{
			settingsManagerUI().globalSettings().getUISettings().UpdateSetting(messager, GLOBAL_SETTINGS_UI_NAMESPACE::OPEN_INPUT_PROJECTS_LIST, InputProjectFilesList(messager, input_project_path.string().c_str()));
			settingsManagerUI().globalSettings().getUISettings().UpdateSetting(messager, GLOBAL_SETTINGS_UI_NAMESPACE::OPEN_INPUT_DATASET_FOLDER_PATH, OpenInputFilePath(messager, input_project_path.parent_path()));
			input_project_list = InputProjectFilesList::get(messager);
		}
	}

	if (input_project_list->files.size() == 1)
	{

		boost::filesystem::path input_project_settings_path = input_project_list->files[0];

		bool create_new_instance = false;

		if (input_tabs.find(mainWindow) == input_tabs.cend())
		{
			create_new_instance = true;
		}

		if (create_new_instance)
		{

			RawOpenInputProject(messager, input_project_settings_path, mainWindowObject);

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

	UIInputProject * project = static_cast<UIInputProject *>(tab.project.get());

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

	UIOutputProject * project = static_cast<UIOutputProject *>(tab.project.get());

	return project;

}

void UIProjectManager::SignalMessageBox(STD_STRING msg)
{

	QMessageBox msgBox;
	msgBox.setText( msg.c_str() );
	msgBox.exec();

}

void UIProjectManager::DoneLoadingFromDatabase(UI_INPUT_MODEL_PTR model_, QObject * mainWindowObject)
{

	bool was_loading = loading;
	loading = false;

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

	projectManager().input_project_is_open = true;

	if (was_loading)
	{

		OutputProjectFilesList::instance output_project_list = OutputProjectFilesList::get(messager.get());

		if (output_project_list->files.size() == 0)
		{
			boost::format msg_title("Open output project at default location?");
			boost::format msg_text("You have no output project open.  Would you like to open the project at the default location?");
			QMessageBox::StandardButton reply;
			reply = QMessageBox::question(nullptr, QString(msg_title.str().c_str()), QString(msg_text.str().c_str()), QMessageBox::StandardButtons(QMessageBox::Yes | QMessageBox::No));
			if (reply == QMessageBox::No)
			{
				return;
			}

			boost::filesystem::path output_project_path = settingsManagerUI().ObtainGlobalPath(QStandardPaths::DocumentsLocation, "NewGene/Output", NewGeneFileNames::defaultOutputProjectFileName);
			if (output_project_path != boost::filesystem::path())
			{
				settingsManagerUI().globalSettings().getUISettings().UpdateSetting(messager.get(), GLOBAL_SETTINGS_UI_NAMESPACE::OPEN_OUTPUT_PROJECTS_LIST, OutputProjectFilesList(messager.get(), output_project_path.string().c_str()));
				settingsManagerUI().globalSettings().getUISettings().UpdateSetting(messager.get(), GLOBAL_SETTINGS_UI_NAMESPACE::OPEN_OUTPUT_DATASET_FOLDER_PATH, OpenOutputFilePath(messager.get(), output_project_path.parent_path()));
				output_project_list = OutputProjectFilesList::get(messager.get());
			}
		}

		if (output_project_list->files.size() == 1)
		{

			boost::filesystem::path output_project_settings_path = output_project_list->files[0];

			bool create_new_instance = false;

			NewGeneMainWindow * mainWindow = nullptr;
			try
			{
				mainWindow = dynamic_cast<NewGeneMainWindow *>(mainWindowObject);
			}
			catch (std::bad_cast &)
			{
				return;
			}

			if (output_tabs.find(mainWindow) == output_tabs.cend())
			{
				create_new_instance = true;
			}

			if (create_new_instance)
			{

				RawOpenOutputProject(messager.get(), output_project_settings_path, mainWindowObject);

			}

		}

	}
	else
	{

		UIInputProject * input_project = getActiveUIInputProject();
		if (!input_project)
		{
			return;
		}

		settingsManagerUI().globalSettings().getUISettings().UpdateSetting(messager.get(), GLOBAL_SETTINGS_UI_NAMESPACE::OPEN_INPUT_PROJECTS_LIST, InputProjectFilesList(messager.get(), input_project->projectSettings().getUISettings().GetSettingsPath().string()));

		QMessageBox::StandardButton reply;
		reply = QMessageBox::question(nullptr, QString("Open output project?"), QString("Would you also like to open an associated output project?"), QMessageBox::StandardButtons(QMessageBox::Yes | QMessageBox::No));
		if (reply == QMessageBox::Yes)
		{
			OpenOutputFilePath::instance folder_path = OpenOutputFilePath::get(messager.get());
			QWidget * mainWindow = nullptr;
			try
			{
				mainWindow = dynamic_cast<QWidget*>(mainWindowObject);
			}
			catch (std::bad_cast &)
			{

			}
			if (mainWindow)
			{
				QString the_file = QFileDialog::getOpenFileName(mainWindow, "Choose output dataset", folder_path ? folder_path->getPath().string().c_str() : "", "NewGene output settings file (*.newgene.out.xml)");
				if (the_file.size())
				{
					if (boost::filesystem::exists(the_file.toStdString()) && !boost::filesystem::is_directory(the_file.toStdString()))
					{
						boost::filesystem::path file_path(the_file.toStdString());
						settingsManagerUI().globalSettings().getUISettings().UpdateSetting(messager.get(), GLOBAL_SETTINGS_UI_NAMESPACE::OPEN_OUTPUT_DATASET_FOLDER_PATH, OpenOutputFilePath(messager.get(), file_path.parent_path()));
						OpenOutputDataset(file_path.string(), mainWindowObject);
					}
				}
			}
		}

	}

}

void UIProjectManager::DoneLoadingFromDatabase(UI_OUTPUT_MODEL_PTR model_, QObject * mainWindowObject)
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

	UIOutputProject * output_project = getActiveUIOutputProject();
	if (!output_project)
	{
		return;
	}

	projectManager().output_project_is_open = true;

	settingsManagerUI().globalSettings().getUISettings().UpdateSetting(messager.get(), GLOBAL_SETTINGS_UI_NAMESPACE::OPEN_OUTPUT_PROJECTS_LIST, OutputProjectFilesList(messager.get(), output_project->projectSettings().getUISettings().GetSettingsPath().string()));

	getActiveUIInputProject()->DoRefreshAllWidgets();
	getActiveUIOutputProject()->DoRefreshAllWidgets();

}

void UIProjectManager::OpenOutputDataset(STD_STRING the_output_dataset, QObject * mainWindowObject)
{

	// blocks - all widgets that respond to the UIProjectManager
	CloseCurrentOutputDataset();

	UIMessager messager;

	UIInputProject * input_project = getActiveUIInputProject();
	if (!input_project)
	{
		boost::format msg("Please open or create an input dataset before attempting to open or create an output dataset.");
		QMessageBox msgBox;
		msgBox.setText( msg.str().c_str() );
		msgBox.exec();
		return;
	}

	if (!boost::filesystem::is_directory(the_output_dataset))
	{
		RawOpenOutputProject(messager, boost::filesystem::path(the_output_dataset), mainWindowObject);
	}

}

void UIProjectManager::CloseCurrentOutputDataset()
{

	// One per main window (currently only 1 supported per application)
	if (output_tabs.size() > 1 || output_tabs.size() == 0)
	{
		return;
	}
	OutputProjectTabs & tabs = (*output_tabs.begin()).second;

	// One per project (corresponding to an actual physical tab; currently only 1 per main window supported)
	if (tabs.size() > 1 || tabs.size() == 0)
	{
		return;
	}
	OutputProjectTab & tab = *tabs.begin();

	if (!tab.project)
	{
		tabs.clear();
		return;
	}

	UIOutputProject * project_ptr = static_cast<UIOutputProject*>(tab.project.release());
	RawCloseOutputProject(project_ptr);

	tabs.clear();

	UIMessager messager;
	settingsManagerUI().globalSettings().getUISettings().UpdateSetting(messager, GLOBAL_SETTINGS_UI_NAMESPACE::OPEN_OUTPUT_PROJECTS_LIST, InputProjectFilesList(messager, ""));

}

void UIProjectManager::OpenInputDataset(STD_STRING the_input_dataset, QObject * mainWindowObject)
{

	// blocks - all widgets that respond to the UIProjectManager
	CloseCurrentOutputDataset();

	// blocks - all widgets that respond to the UIProjectManager
	CloseCurrentInputDataset();

	bool success = false;

	UIMessager messager;
	if (!boost::filesystem::is_directory(the_input_dataset))
	{
		RawOpenInputProject(messager, boost::filesystem::path(the_input_dataset), mainWindowObject);
	}

}

void UIProjectManager::CloseCurrentInputDataset()
{

	// Blocks - All happens in main thread
	CloseCurrentOutputDataset();

	// One per main window (currently only 1 supported per application)
	if (input_tabs.size() > 1 || input_tabs.size() == 0)
	{
		return;
	}
	InputProjectTabs & tabs = (*input_tabs.begin()).second;

	// One per project (corresponding to an actual physical tab; currently only 1 per main window supported)
	if (tabs.size() > 1 || tabs.size() == 0)
	{
		return;
	}
	InputProjectTab & tab = *tabs.begin();

	if (!tab.project)
	{
		tabs.clear();
		return;
	}

	UIInputProject * project_ptr = static_cast<UIInputProject*>(tab.project.release());
	RawCloseInputProject(project_ptr);

	tabs.clear();

	UIMessager messager;
	settingsManagerUI().globalSettings().getUISettings().UpdateSetting(messager, GLOBAL_SETTINGS_UI_NAMESPACE::OPEN_INPUT_PROJECTS_LIST, InputProjectFilesList(messager, ""));

}

void UIProjectManager::RawOpenInputProject(UIMessager & messager, boost::filesystem::path const & input_project_settings_path, QObject * mainWindowObject)
{

	if (boost::filesystem::is_directory(input_project_settings_path))
	{
		QMessageBox msgBox;
		boost::format msg("%1%, the input project settings filename, is not a valid file.");
		msg % input_project_settings_path.string();
		msgBox.setText(msg.str().c_str());
		msgBox.exec();
		return;
	}

	// Internally creates both an instance of UI-layer project settings, and an instance of backend-layer project settings
	// via SettingsRepositoryFactory
	std::shared_ptr<UIInputProjectSettings> project_settings(new UIInputProjectSettings(messager, input_project_settings_path));
	project_settings->WriteSettingsToFile(messager); // Writes default settings for those settings not already present

	// When the UIInputModelSettings instance is created below, it internally creates
	// an instance of backend-layer model settings
	auto path_to_model_settings_ = InputProjectPathToModel::get(messager, project_settings->getBackendSettings());
	boost::filesystem::path path_to_model_settings = path_to_model_settings_->getPath();
	if (path_to_model_settings.is_relative())
	{
		boost::filesystem::path new_path = input_project_settings_path.parent_path();
		new_path /= path_to_model_settings;
		path_to_model_settings = new_path;
	}
	if (boost::filesystem::is_directory(path_to_model_settings))
	{
		path_to_model_settings /= (input_project_settings_path.stem().string() + ".model.xml");
	}
	std::shared_ptr<UIInputModelSettings> model_settings(new UIInputModelSettings(messager, path_to_model_settings));
	model_settings->WriteSettingsToFile(messager); // Writes default settings for those settings not already present

	// Backend model does not know about the current project's settings, because multiple settings might point to the same model.
	auto path_to_model_database_ = InputModelPathToDatabase::get(messager, model_settings->getBackendSettings());
	boost::filesystem::path path_to_model_database = path_to_model_database_->getPath();
	if (path_to_model_database.is_relative())
	{
		boost::filesystem::path new_path = path_to_model_settings.parent_path();
		new_path /= path_to_model_database;
		path_to_model_database = new_path;
	}
	if (boost::filesystem::is_directory(path_to_model_database))
	{
		path_to_model_database /= (input_project_settings_path.stem().string() + ".db");
	}
	std::shared_ptr<InputModel> backend_model;
	try
	{
		backend_model.reset(ModelFactory<InputModel>()(messager, path_to_model_database));
	}
	catch (boost::exception & e)
	{
		if ( std::string const * error_desc = boost::get_error_info<newgene_error_description>( e ) )
		{
			boost::format msg( error_desc->c_str() );
			QMessageBox msgBox;
			msgBox.setText( msg.str().c_str() );
			msgBox.exec();
		}
		boost::format msg( "Unable to create input project database." );
		messager.AppendMessage(new MessagerWarningMessage(MESSAGER_MESSAGE__INPUT_MODEL_DATABASE_CANNOT_BE_CREATED, msg.str()));
		return;
	}
	std::shared_ptr<UIInputModel> project_model(new UIInputModel(messager, backend_model));

	NewGeneMainWindow * mainWindow = nullptr;
	try
	{
		mainWindow = dynamic_cast<NewGeneMainWindow *>(mainWindowObject);
	}
	catch (std::bad_cast &)
	{
		return;
	}

	if (mainWindow == nullptr)
	{
		return;
	}

	// ************************************************************************************************************************************* //
	// Clang workaround: http://stackoverflow.com/questions/20583591/clang-only-a-pairpath-path-can-be-emplaced-into-a-vector-so-can-a-pairuniq
	// ... cannot pass const filesystem::path, so must create temp from the const that can act as rvalue
	// ************************************************************************************************************************************* //
	std::unique_ptr<UIMessagerInputProject> messager_ptr(new UIMessagerInputProject(nullptr));
	std::unique_ptr<UIInputProject> project_ptr(new UIInputProject(project_settings, model_settings, project_model, mainWindowObject, nullptr, *messager_ptr));
	input_tabs[mainWindow].emplace_back(ProjectPaths(input_project_settings_path, path_to_model_settings, path_to_model_database),
		project_ptr.release(), // can't use move() in the initialization list, I think, because we might have a custom deleter
		messager_ptr.release());

	UIInputProject * project = getActiveUIInputProject();

	if (!project)
	{
		boost::format msg("No input dataset is open.");
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

	// blocks, because all connections are in NewGeneWidget which are all associated with the UI event loop
	emit UpdateInputConnections(NewGeneWidget::ESTABLISH_CONNECTIONS_INPUT_PROJECT, project);

	emit LoadFromDatabase(&project->model(), mainWindowObject);

}

void UIProjectManager::RawOpenOutputProject(UIMessager & messager, boost::filesystem::path const & output_project_settings_path, QObject * mainWindowObject)
{

	if (boost::filesystem::is_directory(output_project_settings_path))
	{
		QMessageBox msgBox;
		boost::format msg("%1%, the input project settings filename, is not a valid file.");
		msg % output_project_settings_path.string();
		msgBox.setText(msg.str().c_str());
		msgBox.exec();
		return;
	}

	// Internally creates both an instance of UI-layer project settings, and an instance of backend-layer project settings
	// via SettingsRepositoryFactory
	std::shared_ptr<UIOutputProjectSettings> project_settings(new UIOutputProjectSettings(messager, output_project_settings_path));
	project_settings->WriteSettingsToFile(messager); // Writes default settings for those settings not already present

	// When the UIOutputModelSettings instance is created below, it internally creates
	// an instance of backend-layer model settings
	auto path_to_model_settings_ = OutputProjectPathToModel::get(messager, project_settings->getBackendSettings());
	boost::filesystem::path path_to_model_settings = path_to_model_settings_->getPath();
	if (path_to_model_settings.is_relative())
	{
		boost::filesystem::path new_path = output_project_settings_path.parent_path();
		new_path /= path_to_model_settings;
		path_to_model_settings = new_path;
	}
	if (boost::filesystem::is_directory(path_to_model_settings))
	{
		path_to_model_settings /= (output_project_settings_path.stem().string() + ".model.xml");
	}
	std::shared_ptr<UIOutputModelSettings> model_settings(new UIOutputModelSettings(messager, path_to_model_settings));
	model_settings->WriteSettingsToFile(messager); // Writes default settings for those settings not already present

	// The input model and settings are necessary in order to instantiate the output model
	UIInputProject * input_project = getActiveUIInputProject();
	if (!input_project)
	{
		boost::format msg("NULL input project during attempt to instantiate output project.");
		messager.AppendMessage(new MessagerWarningMessage(MESSAGER_MESSAGE__PROJECT_IS_NULL, msg.str()));
		return;
	}

	// Backend model does not know about the current project's settings, because multiple settings might point to the same model.
	auto path_to_model_database_ = OutputModelPathToDatabase::get(messager, model_settings->getBackendSettings());
	boost::filesystem::path path_to_model_database = path_to_model_database_->getPath();
	if (path_to_model_database.is_relative())
	{
		boost::filesystem::path new_path = path_to_model_settings.parent_path();
		new_path /= path_to_model_database;
		path_to_model_database = new_path;
	}
	if (boost::filesystem::is_directory(path_to_model_database))
	{
		path_to_model_database /= (output_project_settings_path.stem().string() + ".db");
	}
	std::shared_ptr<OutputModel> backend_model;
	try
	{
		backend_model.reset(ModelFactory<OutputModel>()(messager, path_to_model_database, std::dynamic_pointer_cast<InputModelSettings>(input_project->backend().modelSettingsSharedPtr()), input_project->backend().modelSharedPtr()));
	}
	catch (boost::exception & e)
	{
		if (std::string const * error_desc = boost::get_error_info<newgene_error_description>(e))
		{
			boost::format msg(error_desc->c_str());
			QMessageBox msgBox;
			msgBox.setText(msg.str().c_str());
			msgBox.exec();
		}
		boost::format msg("Unable to create output project database.");
		messager.AppendMessage(new MessagerWarningMessage(MESSAGER_MESSAGE__OUTPUT_MODEL_DATABASE_CANNOT_BE_CREATED, msg.str()));
		return;
	}
	std::shared_ptr<UIOutputModel> project_model(new UIOutputModel(messager, backend_model));

	NewGeneMainWindow * mainWindow = nullptr;
	try
	{
		mainWindow = dynamic_cast<NewGeneMainWindow *>(mainWindowObject);
	}
	catch (std::bad_cast &)
	{
		return;
	}

	if (mainWindow == nullptr)
	{
		return;
	}

	std::unique_ptr<UIMessagerOutputProject> messager_ptr(new UIMessagerOutputProject(nullptr));
	std::unique_ptr<UIOutputProject> project_ptr(new UIOutputProject(project_settings, model_settings, project_model, mainWindowObject, nullptr, *messager_ptr, input_project));
	output_tabs[mainWindow].emplace_back(ProjectPaths(output_project_settings_path, path_to_model_settings, path_to_model_database),
		project_ptr.release(), // can't use move() in the initialization list, I think, because we might have a custom deleter
		messager_ptr.release());

	UIOutputProject * project = getActiveUIOutputProject();

	if (!project)

	{
		boost::format msg("NULL output project during attempt to instantiate project.");
		messager.AppendMessage(new MessagerWarningMessage(MESSAGER_MESSAGE__PROJECT_IS_NULL, msg.str()));
		return;
	}

	project->InitializeEventLoop(project, 32000000); // cannot use 'this' in base class with multiple inheritance
	project->model().InitializeEventLoop(&project->model()); // cannot use 'this' in base class with multiple inheritance
	project->modelSettings().InitializeEventLoop(&project->modelSettings()); // cannot use 'this' in base class with multiple inheritance
	project->projectSettings().InitializeEventLoop(&project->projectSettings()); // cannot use 'this' in base class with multiple inheritance

	project_settings->UpdateConnections();
	model_settings->UpdateConnections();
	project_model->UpdateConnections();
	project->UpdateConnections();

	// blocks, because all connections are in NewGeneWidget which are all associated with the UI event loop
	emit UpdateOutputConnections(NewGeneWidget::ESTABLISH_CONNECTIONS_OUTPUT_PROJECT, project);

	emit LoadFromDatabase(&getActiveUIOutputProject()->model(), mainWindowObject);

}

void UIProjectManager::RawCloseInputProject(UIInputProject * input_project)
{

	if (!input_project)
	{
		return;
	}

	projectManager().input_project_is_open = false;

	// blocks, because all connections are in NewGeneWidget which are all associated with the UI event loop
	emit UpdateInputConnections(NewGeneWidget::RELEASE_CONNECTIONS_INPUT_PROJECT, input_project);

	input_project->model().EndLoopAndBackgroundPool(); // blocks
	input_project->modelSettings().EndLoopAndBackgroundPool(); // blocks
	input_project->projectSettings().EndLoopAndBackgroundPool(); // blocks
	input_project->EndLoopAndBackgroundPool(); // blocks

	input_project->deleteLater();

}

void UIProjectManager::RawCloseOutputProject(UIOutputProject * output_project)
{

	if (!output_project)
	{
		return;
	}

	projectManager().output_project_is_open = false;

	output_project->setUIInputProject(nullptr);

	// blocks, because all connections are in NewGeneWidget which are all associated with the UI event loop
	emit this->UpdateOutputConnections(NewGeneWidget::RELEASE_CONNECTIONS_OUTPUT_PROJECT, output_project);

	output_project->model().EndLoopAndBackgroundPool(); // blocks
	output_project->modelSettings().EndLoopAndBackgroundPool(); // blocks
	output_project->projectSettings().EndLoopAndBackgroundPool(); // blocks
	output_project->EndLoopAndBackgroundPool(); // blocks

	output_project->deleteLater();

}

void UIProjectManager::SaveCurrentInputDatasetAs(STD_STRING the_input_dataset, QObject * mainWindowObject)
{

	UIMessager messager;
	UIMessagerSingleShot messager_(messager);

	UIInputProject * active_input_project = getActiveUIInputProject();
	if (active_input_project != nullptr)
	{

		// Create a path object for the path to the project settings
		boost::filesystem::path input_project_settings_path(the_input_dataset.c_str());

		// Create a path object for the path to the model settings
		boost::filesystem::path path_to_model_settings(input_project_settings_path.parent_path() / (input_project_settings_path.stem().string() + ".model.xml"));

		// Create a path object for the path to the model database file
		boost::filesystem::path path_to_model_database(input_project_settings_path.parent_path() / (input_project_settings_path.stem().string() + ".db"));

		if (boost::filesystem::exists(input_project_settings_path))
		{
			QMessageBox::StandardButton reply;
			boost::format msg("The input project file \"%1%\" already exists.  Would you like to overwrite it?");
			msg % input_project_settings_path.string();
			reply = QMessageBox::question(nullptr, QString("Overwrite input project?"), QString(msg.str().c_str()), QMessageBox::StandardButtons(QMessageBox::Yes | QMessageBox::No));
			if (reply == QMessageBox::No)
			{
				return;
			}
		}

		if (boost::filesystem::exists(path_to_model_settings))
		{
			QMessageBox::StandardButton reply;
			boost::format msg("The input model file \"%1%\" already exists.  Would you like to overwrite it?");
			msg % path_to_model_settings.string();
			reply = QMessageBox::question(nullptr, QString("Overwrite input model?"), QString(msg.str().c_str()), QMessageBox::StandardButtons(QMessageBox::Yes | QMessageBox::No));
			if (reply == QMessageBox::No)
			{
				return;
			}
		}

		if (boost::filesystem::exists(path_to_model_database))
		{
			QMessageBox::StandardButton reply;
			boost::format msg("The input model database file \"%1%\" already exists.  Would you like to overwrite it?");
			msg % path_to_model_database.string();
			reply = QMessageBox::question(nullptr, QString("Overwrite input model database?"), QString(msg.str().c_str()), QMessageBox::StandardButtons(QMessageBox::Yes | QMessageBox::No));
			if (reply == QMessageBox::No)
			{
				return;
			}
		}

		// Set the new path for the project settings in the currently open project
		active_input_project->SetProjectPaths(input_project_settings_path, path_to_model_settings);

		// Write the project settings to file
		active_input_project->projectSettings().WriteSettingsToFile(messager);

		// Write the model settings to file
		active_input_project->modelSettings().WriteSettingsToFile(messager);

		// Copy the database
		active_input_project->backend().model().SaveDatabaseAs(messager, path_to_model_database);

	}

}

void UIProjectManager::SaveCurrentOutputDatasetAs(STD_STRING the_output_dataset, QObject * mainWindowObject)
{

	UIMessager messager;
	UIMessagerSingleShot messager_(messager);

	UIOutputProject * active_output_project = getActiveUIOutputProject();
	if (active_output_project != nullptr)
	{

		// Create a path object for the path to the project settings
		boost::filesystem::path output_project_settings_path(the_output_dataset.c_str());

		// Create a path object for the path to the model settings
		boost::filesystem::path path_to_model_settings(output_project_settings_path.parent_path() / (output_project_settings_path.stem().string() + ".model.xml"));

		// Create a path object for the path to the model database file
		boost::filesystem::path path_to_model_database(output_project_settings_path.parent_path() / (output_project_settings_path.stem().string() + ".db"));

		if (boost::filesystem::exists(output_project_settings_path))
		{
			QMessageBox::StandardButton reply;
			boost::format msg("The output project file \"%1%\" already exists.  Would you like to overwrite it?");
			msg % output_project_settings_path.string();
			reply = QMessageBox::question(nullptr, QString("Overwrite output project?"), QString(msg.str().c_str()), QMessageBox::StandardButtons(QMessageBox::Yes | QMessageBox::No));
			if (reply == QMessageBox::No)
			{
				return;
			}
		}

		if (boost::filesystem::exists(path_to_model_settings))
		{
			QMessageBox::StandardButton reply;
			boost::format msg("The output model file \"%1%\" already exists.  Would you like to overwrite it?");
			msg % path_to_model_settings.string();
			reply = QMessageBox::question(nullptr, QString("Overwrite output model?"), QString(msg.str().c_str()), QMessageBox::StandardButtons(QMessageBox::Yes | QMessageBox::No));
			if (reply == QMessageBox::No)
			{
				return;
			}
		}

		if (boost::filesystem::exists(path_to_model_database))
		{
			QMessageBox::StandardButton reply;
			boost::format msg("The output model database file \"%1%\" already exists.  Would you like to overwrite it?");
			msg % path_to_model_database.string();
			reply = QMessageBox::question(nullptr, QString("Overwrite output model database?"), QString(msg.str().c_str()), QMessageBox::StandardButtons(QMessageBox::Yes | QMessageBox::No));
			if (reply == QMessageBox::No)
			{
				return;
			}
		}

		// Set the new path for the project settings in the currently open project
		active_output_project->SetProjectPaths(output_project_settings_path, path_to_model_settings);

		// Write the project settings to file
		active_output_project->projectSettings().WriteSettingsToFile(messager);

		// Write the model settings to file
		active_output_project->modelSettings().WriteSettingsToFile(messager);

		// Copy the database
		active_output_project->backend().model().SaveDatabaseAs(messager, path_to_model_database);

	}

}
