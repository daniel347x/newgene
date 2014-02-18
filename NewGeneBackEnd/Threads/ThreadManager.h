#ifndef THREADMANAGER_H
#define THREADMANAGER_H

#include "../Manager.h"
#include <queue>
#include "WorkItem.h"
#include "WorkerThread.h"

class ThreadManager : public Manager<ThreadManager, MANAGER_DESCRIPTION_NAMESPACE::MANAGER_THREADS>
{
public:
	ThreadManager(Messager & messager_) : Manager<ThreadManager, MANAGER_DESCRIPTION_NAMESPACE::MANAGER_THREADS>(messager_) {}
};

#endif
