#ifndef PROJECTINPUT_H
#define PROJECTINPUT_H

#include <memory>
#include "Project.h"
#include "..\Settings\InputProjectSettings.h"
#include "..\Settings\Setting.h"
#include "..\Model\InputModel.h"

class InputProject : public Project<INPUT_PROJECT_SETTINGS_BACKEND_NAMESPACE::INPUT_PROJECT_SETTINGS_BACKEND, BackendProjectInputSetting, InputModel>
{
public:
	InputProject(Messager & messager, std::shared_ptr<ProjectSettings<INPUT_PROJECT_SETTINGS_BACKEND_NAMESPACE::INPUT_PROJECT_SETTINGS_BACKEND, BackendProjectInputSetting> > const & settings, std::shared_ptr<InputModel> const & model)
		: Project(messager, settings, model)
	{

	}
};

#endif
