#ifndef PROJECTMANAGER_H
#define PROJECTMANAGER_H

#include "../Manager.h"

class ProjectManager : public Manager<ProjectManager, MANAGER_DESCRIPTION_NAMESPACE::MANAGER_PROJECT>
{
public:
	ProjectManager(Messager & messager_) : Manager<ProjectManager, MANAGER_DESCRIPTION_NAMESPACE::MANAGER_PROJECT>(messager_) {}
};

#endif
