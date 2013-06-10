#include "globals.h"

template<typename MANAGER_CLASS>
MANAGER_CLASS & get_a_manager()
{
	return static_cast<MANAGER_CLASS&>(MANAGER_CLASS::getManager());
}

SettingsManager & settingsManager()
{
	return get_a_manager<SettingsManager>();
}

LoggingManager & loggingManager()
{
	return get_a_manager<LoggingManager>();
}

ProjectManager & projectManager()
{
	return get_a_manager<ProjectManager>();
}

ModelManager & modelManager()
{
	return get_a_manager<ModelManager>();
}

DocumentManager & documentManager()
{
	return get_a_manager<DocumentManager>();
}

StatusManager & statusManager()
{
	return get_a_manager<StatusManager>();
}

TriggerManager & triggerManager()
{
	return get_a_manager<TriggerManager>();
}

ThreadManager & threadManager()
{
	return get_a_manager<ThreadManager>();
}
