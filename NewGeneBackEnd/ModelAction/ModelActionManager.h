#ifndef MODELACTIONMANAGER_H
#define MODELACTIONMANAGER_H

#include "../Manager.h"

class ModelActionManager : public Manager<ModelActionManager, MANAGER_DESCRIPTION_NAMESPACE::MANAGER_MODEL_ACTION>
{
public:
	ModelActionManager(Messager & messager_) : Manager<ModelActionManager, MANAGER_DESCRIPTION_NAMESPACE::MANAGER_MODEL_ACTION>(messager_) {}
};

#endif
