#ifndef PROJECTINPUTSETTINGS_H
#define PROJECTINPUTSETTINGS_H

#include "ProjectSettings.h"

namespace INPUT_PROJECT_SETTINGS_BACKEND_NAMESPACE
{

	enum INPUT_PROJECT_SETTINGS_BACKEND
	{
		SETTING_FIRST = 0
		, SETTING_LAST
	};

}

class InputProjectSettings : public ProjectSettings<INPUT_PROJECT_SETTINGS_BACKEND_NAMESPACE::INPUT_PROJECT_SETTINGS_BACKEND, BackendProjectInputSetting>
{

public:

	InputProjectSettings(Messager & messager, boost::filesystem::path const project_settings_path)
		: ProjectSettings(messager, project_settings_path)
	{}

	virtual ~InputProjectSettings() {}

	//void SetMapEntry(Messager & messager, SettingInfo & setting_info, boost::property_tree::ptree & pt);
	BackendProjectInputSetting * CloneSetting(Messager & messager, BackendProjectInputSetting * current_setting, SettingInfo & setting_info) const;
	BackendProjectInputSetting * NewSetting(Messager & messager, SettingInfo & setting_info, void const * setting_value_void = NULL);

};

#endif
