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
	//GlobalSetting_Test::instance test = GlobalSetting_Test::get(messager);
	//statusManagerUI().PostStatus(test->getString().c_str());

	UIGlobalSetting_MRU_Input_List::instance mru = UIGlobalSetting_MRU_Input_List::get(messager);
	statusManagerUI().PostStatus(mru->getString().c_str());
}
