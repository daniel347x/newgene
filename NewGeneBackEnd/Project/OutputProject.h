#ifndef PROJECTOUTPUT_H
#define PROJECTOUTPUT_H

#include <memory>
#include "Project.h"
#include "..\Settings\OutputProjectSettings.h"
#include "..\Settings\Setting.h"
#include "..\Model\OutputModel.h"

class OutputProject : public Project<OUTPUT_PROJECT_SETTINGS_BACKEND_NAMESPACE::OUTPUT_PROJECT_SETTINGS_BACKEND, BackendProjectOutputSetting, OutputModel>
{
public:
	OutputProject(Messager & messager)
		: Project(messager)
	{

	}
};

#endif
