#include "globals.h"
#include "Widgets/newgenemainwindow.h"
#include "uisettingsmanager.h"
#include "uiloggingmanager.h"
#include "uiprojectmanager.h"
#include "uimodelmanager.h"
#include "uidocumentmanager.h"
#include "uistatusmanager.h"
#include "uitriggermanager.h"
#include "uithreadmanager.h"
#include "uiuidatamanager.h"
#include "uiuiactionmanager.h"
#include "uimodelactionmanager.h"

NewGeneMainWindow * theMainWindow = NULL;

UIMessager dummy_messager;
Messager * dummy_messager_ptr = &dummy_messager;

template<typename MANAGER_CLASS>
MANAGER_CLASS & get_a_ui_manager(UIMessager * messager)
{
	return static_cast<MANAGER_CLASS&>(MANAGER_CLASS::getManager(messager));
}

UISettingsManager & settingsManagerUI(UIMessager * messager)
{
	return get_a_ui_manager<UISettingsManager>(messager);
}

UILoggingManager & loggingManagerUI(UIMessager * messager)
{
	return get_a_ui_manager<UILoggingManager>(messager);
}

UIProjectManager & projectManagerUI(UIMessager * messager)
{
	return get_a_ui_manager<UIProjectManager>(messager);
}

UIModelManager & modelManagerUI(UIMessager * messager)
{
	return get_a_ui_manager<UIModelManager>(messager);
}

UIDocumentManager & documentManagerUI(UIMessager * messager)
{
	return get_a_ui_manager<UIDocumentManager>(messager);
}

UIStatusManager & statusManagerUI(UIMessager * messager)
{
	return get_a_ui_manager<UIStatusManager>(messager);
}

UITriggerManager & triggerManagerUI(UIMessager * messager)
{
	return get_a_ui_manager<UITriggerManager>(messager);
}

UIThreadManager & threadManagerUI(UIMessager * messager)
{
	return get_a_ui_manager<UIThreadManager>(messager);
}

UIUIDataManager & uidataManagerUI(UIMessager * messager)
{
	return get_a_ui_manager<UIUIDataManager>(messager);
}

UIUIActionManager & uiactionManagerUI(UIMessager * messager)
{
	return get_a_ui_manager<UIUIActionManager>(messager);
}

UIModelActionManager & modelactionManagerUI(UIMessager * messager)
{
	return get_a_ui_manager<UIModelActionManager>(messager);
}
