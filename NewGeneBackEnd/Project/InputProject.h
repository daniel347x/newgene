#ifndef PROJECTINPUT_H
#define PROJECTINPUT_H

#include <memory>
#include "Project.h"
#include "..\Settings\InputProjectSettings.h"
#include "..\Settings\Setting.h"

class InputProject : public Project<INPUT_PROJECT_SETTINGS_BACKEND_NAMESPACE::INPUT_PROJECT_SETTINGS_BACKEND, BackendProjectInputSetting>
{
public:
	InputProject(Messager & messager)
		: Project(messager)
	{

	}
	//std::shared_ptr<InputProjectSettings> _settings;
};

#endif
