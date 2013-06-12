#include "globals.h"
#include <QObject>
#include "uiprojectmanager.h"
#include "uimodelmanager.h"
#include "uisettingsmanager.h"
#include "uidocumentmanager.h"
#include "uistatusmanager.h"
#include "uiloggingmanager.h"
#include "uiallglobalsettings_list.h"
#include "../../../NewGeneBackEnd/Settings/GlobalSettings_list.h"
#include "../newgenewidget.h"

UIProjectManager::UIProjectManager( QObject * parent )
	: QObject(parent)
	, UIManager()
{

	// *************************************************************************
	// All Managers are instantiated AFTER the application event loop is running
	// *************************************************************************

}

UIProjectManager::~UIProjectManager()
{
//	for_each(input_projects.begin(), input_projects.end(), [](std::pair<NewGeneMainWindow * const, std::unique_ptr<UIInputProject>> & projects)
//	{
//		projects.second->getQueueManagerThread().quit();
//		UIInputProject * project_ptr = projects.second.release();
//		project_ptr->deleteLater();
//	});
//	for_each(output_projects.begin(), output_projects.end(), [](std::pair<NewGeneMainWindow * const, std::unique_ptr<UIOutputProject>> & projects)
//	{
//		projects.second->getQueueManagerThread().quit();
//		UIOutputProject * project_ptr = projects.second.release();
//		project_ptr->deleteLater();
//	});
}

void UIProjectManager::LoadOpenProjects(NewGeneMainWindow* mainWindow)
{
	UIMessager messager;
	InputProjectFilesList::instance input_project_list = InputProjectFilesList::get(messager);
	OutputProjectFilesList::instance output_project_list = OutputProjectFilesList::get(messager);

	if (input_project_list->files.size() == 1)
	{
		boost::filesystem::path input_project_settings_path = input_project_list->files[0].first;
		//boost::filesystem::path input_project_model_path = input_project_list->files[0].second;
		//if (input_projects.find(mainWindow) == input_projects.cend())
		//{
			std::unique_ptr<UIMessager> project_messager(new UIMessager());
			std::shared_ptr<UIInputProjectSettings> project_settings(new UIInputProjectSettings(*project_messager, input_project_settings_path));
			project_settings->WriteSettingsToFile(*project_messager);
			std::shared_ptr<InputModelSettings> model_settings(SettingsRepositoryFactory<InputModelSettings>()(*project_messager, input_project_settings_path));
			model_settings->WriteSettingsToFile(*project_messager);
			std::shared_ptr<InputModel> backend_model(ModelFactory<InputModel>()(*project_messager, input_project_settings_path));
			std::shared_ptr<UIInputModel> project_model(new UIInputModel(*project_messager, backend_model));
			//input_projects[mainWindow] = std::unique_ptr<UIInputProject>(new UIInputProject(project_messager.release(), project_settings.release(), model_settings.release(), project_model.release()));
			input_project.reset(new UIInputProject(project_messager.release(), project_settings, project_model, model_settings));
			//connect(this, SIGNAL(TriggerInputProject()), &input_projects[mainWindow]->getQueueManager(), SLOT(ReceiveTrigger()));
			//emit TriggerInputProject();
		//}

		//messager.AppendMessage(new MessagerErrorMessage(MESSAGER_MESSAGE_ENUM::MESSAGER_MESSAGE__GENERAL_ERROR, input_project_settings_path.filename().string()));
		//messager.AppendMessage(new MessagerErrorMessage(MESSAGER_MESSAGE_ENUM::MESSAGER_MESSAGE__GENERAL_ERROR, input_project_model_path.filename().string()));
	}

}

void UIProjectManager::TriggerActiveInputProject(NewGeneMainWindow * newGeneMainWindow/* to be filled in */)
{
//	if (input_projects.find(newGeneMainWindow) != input_projects.cend())
//	{
//		UIInputProject & project = *input_projects[newGeneMainWindow];

//	}
}

void UIProjectManager::TriggerActiveOutputProject(NewGeneMainWindow * newGeneMainWindow/* to be filled in */)
{

}

void UIProjectManager::ConnectTrigger(QWidget * w)
{
	NewGeneMainWindow * mainWindow = nullptr;
	try
	{
		NewGeneWidget * ngw = dynamic_cast<NewGeneWidget*>(w);
		mainWindow = &ngw->mainWindow();
	}
	catch (std::bad_cast & bc)
	{
		return;
	}

	//if (input_projects.find(mainWindow) != input_projects.cend())
	{
//		UIInputProject & project = *input_projects[mainWindow];
//		connect(&project.getQueueManager(), SIGNAL(SendTrigger()), w, SLOT(ReceiveUpdate()));
	}
}
