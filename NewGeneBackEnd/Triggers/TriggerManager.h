#ifndef TRIGGERMANAGER_H
#define TRIGGERMANAGER_H

#include "../Manager.h"

class TriggerManager : public Manager<TriggerManager, MANAGER_DESCRIPTION_NAMESPACE::MANAGER_TRIGGERS>
{
	public:
		TriggerManager(Messager & messager_) : Manager<TriggerManager, MANAGER_DESCRIPTION_NAMESPACE::MANAGER_TRIGGERS>(messager_) {}
};

#endif
