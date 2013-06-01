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

void UIProjectManager::LoadOpenProjects(UIMessager & messager)
{
	UIGlobalSetting_Projects_Files_List<GLOBAL_SETTINGS_UI_NAMESPACE::OPEN_INPUT_PROJECTS_LIST>::instance input_project_list = UIGlobalSetting_Projects_Files_List<GLOBAL_SETTINGS_UI_NAMESPACE::OPEN_INPUT_PROJECTS_LIST>::get(messager);
	UIGlobalSetting_Projects_Files_List<GLOBAL_SETTINGS_UI_NAMESPACE::OPEN_OUTPUT_PROJECTS_LIST>::instance output_project_list = UIGlobalSetting_Projects_Files_List<GLOBAL_SETTINGS_UI_NAMESPACE::OPEN_OUTPUT_PROJECTS_LIST>::get(messager);

	if (input_project_list->files.size() > 0)
	{
		boost::filesystem::path input_project_settings_path = input_project_list->files[0].first;
		boost::filesystem::path input_project_model_path = input_project_list->files[0].second;

		messager.AppendMessage(new MessagerErrorMessage(MESSAGER_MESSAGE_ENUM::MESSAGER_MESSAGE__GENERAL_ERROR, input_project_settings_path.filename().string()));
		messager.AppendMessage(new MessagerErrorMessage(MESSAGER_MESSAGE_ENUM::MESSAGER_MESSAGE__GENERAL_ERROR, input_project_model_path.filename().string()));
	}

	if (output_project_list->files.size() > 0)
	{
		boost::filesystem::path output_project_settings_path = output_project_list->files[0].first;
		boost::filesystem::path output_project_model_path = output_project_list->files[0].second;

		messager.AppendMessage(new MessagerErrorMessage(MESSAGER_MESSAGE_ENUM::MESSAGER_MESSAGE__GENERAL_ERROR, output_project_settings_path.filename().string()));
		messager.AppendMessage(new MessagerErrorMessage(MESSAGER_MESSAGE_ENUM::MESSAGER_MESSAGE__GENERAL_ERROR, output_project_model_path.filename().string()));
	}



}
