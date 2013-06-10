#include "globals.h"
#include "newgenemainwindow.h"

NewGeneMainWindow * theMainWindow = NULL;

UIMessager dummy_messager;
Messager * dummy_messager_ptr = &dummy_messager;

template<typename MANAGER_CLASS>
MANAGER_CLASS & get_a_ui_manager()
{
	return static_cast<MANAGER_CLASS&>(MANAGER_CLASS::getManager());
}

UISettingsManager & settingsManagerUI()
{
	return get_a_ui_manager<UISettingsManager>();
}

UILoggingManager & loggingManagerUI()
{
	return get_a_ui_manager<UILoggingManager>();
}

UIProjectManager & projectManagerUI()
{
	return get_a_ui_manager<UIProjectManager>();
}

UIModelManager & modelManagerUI()
{
	return get_a_ui_manager<UIModelManager>();
}

UIDocumentManager & documentManagerUI()
{
	return get_a_ui_manager<UIDocumentManager>();
}

UIStatusManager & statusManagerUI()
{
	return get_a_ui_manager<UIStatusManager>();
}

UITriggerManager & triggerManagerUI()
{
	return get_a_ui_manager<UITriggerManager>();
}

UIThreadManager & threadManagerUI()
{
	return get_a_ui_manager<UIThreadManager>();
}
