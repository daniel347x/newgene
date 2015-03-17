#ifndef PROJECTINPUT_H
#define PROJECTINPUT_H

#include <memory>
#include "Project.h"
#include "../Settings/InputProjectSettings.h"
#include "../Settings/Setting.h"
#include "../Model/InputModel.h"

class InputProject : public
	Project<INPUT_PROJECT_SETTINGS_BACKEND_NAMESPACE::INPUT_PROJECT_SETTINGS_BACKEND, BackendProjectInputSetting, INPUT_MODEL_SETTINGS_NAMESPACE::INPUT_MODEL_SETTINGS, InputModelSetting, InputModel>
{
	public:
		InputProject(std::shared_ptr<ProjectSettings<INPUT_PROJECT_SETTINGS_BACKEND_NAMESPACE::INPUT_PROJECT_SETTINGS_BACKEND, BackendProjectInputSetting>> const & project_settings,
					 std::shared_ptr<ModelSettings<INPUT_MODEL_SETTINGS_NAMESPACE::INPUT_MODEL_SETTINGS, InputModelSetting>> const & model_settings,
					 std::shared_ptr<InputModel> const & model)
			: Project(project_settings, model_settings, model)
		{

		}

		~InputProject()
		{

		}
};

#endif
