#ifndef STATUSMANAGER_H
#define STATUSMANAGER_H

#include "../Manager.h"

class StatusManager : public Manager<StatusManager, MANAGER_DESCRIPTION_NAMESPACE::MANAGER_STATUS>
{
public:
	StatusManager(Messager & messager_) : Manager<StatusManager, MANAGER_DESCRIPTION_NAMESPACE::MANAGER_STATUS>(messager_) {}
};

#endif
