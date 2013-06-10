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

UIProjectManager::UIProjectManager( QObject * parent )
	: QObject(parent)
	, UIManager()
{

	// *************************************************************************
	// All Managers are instantiated AFTER the application event loop is running
	// *************************************************************************

}

void UIProjectManager::LoadOpenProjects(UIMessager & messager, NewGeneMainWindow* mainWindow)
{
	InputProjectFilesList::instance input_project_list = InputProjectFilesList::get(messager);
	OutputProjectFilesList::instance output_project_list = OutputProjectFilesList::get(messager);

	if (input_project_list->files.size() > 0)
	{
		boost::filesystem::path input_project_settings_path = input_project_list->files[0].first;
		boost::filesystem::path input_project_model_path = input_project_list->files[0].second;
		if (input_projects.find(mainWindow) == input_projects.cend())
		{
			std::unique_ptr<UIMessager> project_messager(new UIMessager());
			std::unique_ptr<UIInputProjectSettings> project_settings(new UIInputProjectSettings(*project_messager, input_project_settings_path));
			std::unique_ptr<UIInputModel> project_model(new UIInputModel(*project_messager, input_project_model_path));
			input_projects[mainWindow] = std::unique_ptr<UIInputProject>(new UIInputProject(project_messager.release(), project_settings.release(), project_model.release()));
		}

		//messager.AppendMessage(new MessagerErrorMessage(MESSAGER_MESSAGE_ENUM::MESSAGER_MESSAGE__GENERAL_ERROR, input_project_settings_path.filename().string()));
		//messager.AppendMessage(new MessagerErrorMessage(MESSAGER_MESSAGE_ENUM::MESSAGER_MESSAGE__GENERAL_ERROR, input_project_model_path.filename().string()));
	}

	if (output_project_list->files.size() > 0)
	{
		boost::filesystem::path output_project_settings_path = output_project_list->files[0].first;
		boost::filesystem::path output_project_model_path = output_project_list->files[0].second;
		if (output_projects.find(mainWindow) == output_projects.cend())
		{
			std::unique_ptr<UIMessager> project_messager(new UIMessager());
			std::unique_ptr<UIOutputProjectSettings> project_settings(new UIOutputProjectSettings(*project_messager, output_project_settings_path));
			std::unique_ptr<UIOutputModel> project_model(new UIOutputModel(*project_messager, output_project_model_path));
			output_projects[mainWindow] = std::unique_ptr<UIOutputProject>(new UIOutputProject(project_messager.release(), project_settings.release(), project_model.release()));
		}

		//messager.AppendMessage(new MessagerErrorMessage(MESSAGER_MESSAGE_ENUM::MESSAGER_MESSAGE__GENERAL_ERROR, output_project_settings_path.filename().string()));
		//messager.AppendMessage(new MessagerErrorMessage(MESSAGER_MESSAGE_ENUM::MESSAGER_MESSAGE__GENERAL_ERROR, output_project_model_path.filename().string()));
	}



}
