#ifndef PROJECTINPUTSETTINGS_H
#define PROJECTINPUTSETTINGS_H

#include "ProjectSettings.h"

namespace INPUT_PROJECT_SETTINGS_BACKEND_NAMESPACE
{

	enum INPUT_PROJECT_SETTINGS_BACKEND
	{
		  SETTING_FIRST = 0

		, PATH_TO_MODEL

		, SETTING_LAST
	};

}

class InputProjectSettings : public ProjectSettings<INPUT_PROJECT_SETTINGS_BACKEND_NAMESPACE::INPUT_PROJECT_SETTINGS_BACKEND, BackendProjectInputSetting>
{

public:

	static int const number_worker_threads = 1; // For now, single thread only in pool

	InputProjectSettings(Messager & messager, boost::filesystem::path const project_settings_path)
		: ProjectSettings(messager, project_settings_path)
	{}

	virtual ~InputProjectSettings() {}

	template<typename T>
	void UpdateSetting(Messager & messager, INPUT_PROJECT_SETTINGS_BACKEND_NAMESPACE::INPUT_PROJECT_SETTINGS_BACKEND const which_setting, T const & setting_value)
	{
		SettingInfo setting_info = SettingInfoObject.GetSettingInfoFromEnum(messager, which_setting);
		_settings_map[which_setting] = std::unique_ptr<BackendProjectInputSetting>(NewSetting(messager, setting_info, setting_value.ToString()));
		WriteSettingsToFile(messager);
	}

	void WriteSettingsToFile(Messager & messager)
	{
		boost::property_tree::ptree pt;
		WriteSettingsToPtree(messager, pt);
		WritePtreeToFile(messager, pt);
	}

	void SetMapEntry(Messager & messager, SettingInfo & setting_info, boost::property_tree::ptree & pt);
	BackendProjectInputSetting * CloneSetting(Messager & messager, BackendProjectInputSetting * current_setting, SettingInfo & setting_info) const;
	BackendProjectInputSetting * NewSetting(Messager & messager, SettingInfo & setting_info, std::string const & setting_value_string);
	void SetPTreeEntry(Messager & messager, INPUT_PROJECT_SETTINGS_BACKEND_NAMESPACE::INPUT_PROJECT_SETTINGS_BACKEND which_setting, boost::property_tree::ptree & pt);

};

#endif
