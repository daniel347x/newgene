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
	UIGlobalSetting_OpenProject_Input_List::instance input_project_list = UIGlobalSetting_OpenProject_Input_List::get(messager);
	statusManagerUI().PostStatus(input_project_list->getString().c_str());

	UIGlobalSetting_OpenProject_Output_List::instance output_project_list = UIGlobalSetting_OpenProject_Output_List::get(messager);
	statusManagerUI().PostStatus(output_project_list->getString().c_str());
}
