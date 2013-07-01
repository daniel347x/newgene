#include "ActionChanges.h"

#include "../Project/InputProject.h"
#include "../Project/OutputProject.h"

DataChangeMessage::DataChangeMessage(InputProject * project)
	: data_lock(std::make_shared<std::lock_guard<std::recursive_timed_mutex>>(project->data_mutex))
{

}

DataChangeMessage::DataChangeMessage(OutputProject * project)
	: data_lock(std::make_shared<std::lock_guard<std::recursive_timed_mutex>>(project->data_mutex))
{

}

DataChangeMessage::DataChangeMessage(DataChangeMessage const & rhs)
	: data_lock(rhs.data_lock)
	, changes(rhs.changes)
{

}
