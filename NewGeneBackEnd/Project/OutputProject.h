#ifndef PROJECTOUTPUT_H
#define PROJECTOUTPUT_H

#include <memory>
#include "Project.h"
#include "..\Settings\OutputProjectSettings.h"
#include "..\Settings\Setting.h"

class OutputProject : public Project<OUTPUT_PROJECT_SETTINGS_BACKEND_NAMESPACE::OUTPUT_PROJECT_SETTINGS_BACKEND, BackendProjectOutputSetting>
{
public:
	OutputProject(Messager & messager)
		: Project(messager)
	{

	}
	//std::shared_ptr<OutputProjectSettings> _settings;
};

#endif
