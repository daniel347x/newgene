#ifndef PROJECTOUTPUT_H
#define PROJECTOUTPUT_H

#include <memory>
#include "Project.h"

class ProjectOutput : public Project
{
public:
	ProjectOutput(Messager & messager)
		: Project(messager)
	{

	}
	//std::shared_ptr<ProjectSettings> _settings;
};

#endif
