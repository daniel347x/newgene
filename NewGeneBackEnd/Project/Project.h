#ifndef PROJECT_H
#define PROJECT_H

#include <memory>
#include "../Settings/ProjectSettings.h"

class Project
{
public:
	Project(Messager & messager)
	{

	}
	std::shared_ptr<ProjectSettings> _settings;
};

#endif
