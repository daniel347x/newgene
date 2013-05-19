#include "globals.h"
#include "newgenemainwindow.h"

NewGeneMainWindow * theMainWindow = NULL;

template<typename MANAGER_CLASS>
MANAGER_CLASS & get_a_manager()
{
	return static_cast<MANAGER_CLASS&>(MANAGER_CLASS::getManager());
}

UISettingsManager & settingsManager()
{
	return get_a_manager<UISettingsManager>();
}

UILoggingManager & loggingManager()
{
	return get_a_manager<UILoggingManager>();
}

UIProjectManager & projectManager()
{
	return get_a_manager<UIProjectManager>();
}

UIModelManager & modelManager()
{
	return get_a_manager<UIModelManager>();
}

UIDocumentManager & documentManager()
{
	return get_a_manager<UIDocumentManager>();
}

UIStatusManager & statusManager()
{
	return get_a_manager<UIStatusManager>();
}
