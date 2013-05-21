#ifndef PROJECTOUTPUTSETTINGS_H
#define PROJECTOUTPUTSETTINGS_H

#include "ProjectSettings.h"

namespace OUTPUT_PROJECT_SETTINGS_BACKEND_NAMESPACE
{

	enum OUTPUT_PROJECT_SETTINGS_BACKEND
	{
		SETTING_FIRST = 0
		, SETTING_LAST
	};

}

class OutputProjectSettings : public ProjectSettings<OUTPUT_PROJECT_SETTINGS_BACKEND_NAMESPACE::OUTPUT_PROJECT_SETTINGS_BACKEND, BackendProjectOutputSetting>
{

public:

	OutputProjectSettings(Messager & messager, boost::filesystem::path const project_settings_path)
		: ProjectSettings(messager, project_settings_path)
	{}

	virtual ~OutputProjectSettings() {}

	void SetMapEntry(Messager & messager, SettingInfo & setting_info, boost::property_tree::ptree & pt);
	BackendProjectOutputSetting * CloneSetting(Messager & messager, BackendProjectOutputSetting * current_setting, SettingInfo & setting_info) const;
	BackendProjectOutputSetting * NewSetting(Messager & messager, SettingInfo & setting_info, void const * setting_value_void);

};

#endif
