#ifndef PROJECTINPUT_H
#define PROJECTINPUT_H

#include <memory>
#include "Project.h"

class ProjectInput : public Project
{
public:
	ProjectInput(Messager & messager)
		: Project(messager)
	{

	}
	//std::shared_ptr<ProjectSettings> _settings;
};

#endif
