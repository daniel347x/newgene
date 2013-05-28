#include "globals.h"
#include <QObject>
#include "uiprojectmanager.h"
#include "uimodelmanager.h"
#include "uisettingsmanager.h"
#include "uidocumentmanager.h"
#include "uistatusmanager.h"
#include "uiloggingmanager.h"

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
	//std::unique_ptr<UIGlobalSetting> setting_mru = settingsManagerUI().getSetting(messager, GLOBAL_SETTINGS_UI_NAMESPACE::MRU_LIST);
	GlobalSetting_Test::instance test = GlobalSetting_Test::get(messager);
	statusManagerUI().PostStatus(test->getString().c_str());
}
