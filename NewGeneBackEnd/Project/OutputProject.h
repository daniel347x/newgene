#ifndef PROJECTOUTPUT_H
#define PROJECTOUTPUT_H

#include <memory>
#include "Project.h"
#include "../Settings/OutputProjectSettings.h"
#include "../Settings/Setting.h"
#include "../Model/OutputModel.h"

class OutputProject : public Project<OUTPUT_PROJECT_SETTINGS_BACKEND_NAMESPACE::OUTPUT_PROJECT_SETTINGS_BACKEND, BackendProjectOutputSetting, OUTPUT_MODEL_SETTINGS_NAMESPACE::OUTPUT_MODEL_SETTINGS, OutputModelSetting, OutputModel>
{
public:
	OutputProject(std::shared_ptr<ProjectSettings<OUTPUT_PROJECT_SETTINGS_BACKEND_NAMESPACE::OUTPUT_PROJECT_SETTINGS_BACKEND, BackendProjectOutputSetting>> const & project_settings,
									   std::shared_ptr<ModelSettings<OUTPUT_MODEL_SETTINGS_NAMESPACE::OUTPUT_MODEL_SETTINGS, OutputModelSetting>> const & model_settings,
									   std::shared_ptr<OutputModel> const & model)
		: Project(project_settings, model_settings, model)
	{

	}

	~OutputProject()
	{

	}
};

#endif
