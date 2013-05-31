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
	OutputProject(Messager & messager, std::shared_ptr<ProjectSettings<OUTPUT_PROJECT_SETTINGS_BACKEND_NAMESPACE::OUTPUT_PROJECT_SETTINGS_BACKEND, BackendProjectOutputSetting> > const & settings, std::shared_ptr<OutputModel> const & model)
		: Project(messager, settings, model)
	{

	}
};

#endif
