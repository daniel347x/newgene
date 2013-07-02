#include "ActionChanges.h"

#include "../Project/InputProject.h"
#include "../Project/OutputProject.h"

DataChangeMessage::DataChangeMessage(InputProject * project)
	: inp(project)
	, outp(nullptr)
	, data_lock(std::make_shared<std::lock_guard<std::recursive_timed_mutex>>(project->data_mutex))
{

}

DataChangeMessage::DataChangeMessage(OutputProject * project)
	: inp(nullptr)
	, outp(project)
	, data_lock(std::make_shared<std::lock_guard<std::recursive_timed_mutex>>(project->data_mutex))
{

}

DataChangeMessage::DataChangeMessage(DataChangeMessage const & rhs)
	: inp(rhs.inp)
	, outp(rhs.outp)
	, data_lock(rhs.data_lock)
	, changes(rhs.changes)
{

}

DataChangeMessage::DataChangeMessage()
	: inp(nullptr)
	, outp(nullptr)
{

}
