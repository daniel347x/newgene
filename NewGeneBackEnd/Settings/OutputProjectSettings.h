#ifndef PROJECTOUTPUTSETTINGS_H
#define PROJECTOUTPUTSETTINGS_H

#include "ProjectSettings.h"

namespace OUTPUT_PROJECT_SETTINGS_BACKEND_NAMESPACE
{

	enum OUTPUT_PROJECT_SETTINGS_BACKEND
	{
		  SETTING_FIRST = 0

		, PATH_TO_MODEL
		, PATH_TO_KAD_OUTPUT_FILE

		, SETTING_LAST
	};

}

class OutputProjectSettings : public ProjectSettings<OUTPUT_PROJECT_SETTINGS_BACKEND_NAMESPACE::OUTPUT_PROJECT_SETTINGS_BACKEND, BackendProjectOutputSetting>
{

public:

	static int const number_worker_threads = 1; // For now, single thread only in pool

	OutputProjectSettings(Messager & messager, boost::filesystem::path const project_settings_path)
		: ProjectSettings(messager, project_settings_path)
	{}

	virtual ~OutputProjectSettings() {}

	template<typename T>
	void UpdateSetting(Messager & messager, OUTPUT_PROJECT_SETTINGS_BACKEND_NAMESPACE::OUTPUT_PROJECT_SETTINGS_BACKEND const which_setting, T const & setting_value)
	{
		SettingInfo setting_info = SettingInfoObject.GetSettingInfoFromEnum(messager, which_setting);
		_settings_map[which_setting] = std::unique_ptr<BackendProjectOutputSetting>(NewSetting(messager, setting_info, setting_value.ToString()));
		WriteSettingsToFile(messager);
	}

	void WriteSettingsToFile(Messager & messager)
	{
		boost::property_tree::ptree pt;
		WriteSettingsToPtree(messager, pt);
		WritePtreeToFile(messager, pt);
	}

	void SetMapEntry(Messager & messager, SettingInfo & setting_info, boost::property_tree::ptree & pt);
	BackendProjectOutputSetting * CloneSetting(Messager & messager, BackendProjectOutputSetting * current_setting, SettingInfo & setting_info) const;
	BackendProjectOutputSetting * NewSetting(Messager & messager, SettingInfo & setting_info, std::string const & setting_value_string);
	void SetPTreeEntry(Messager & messager, OUTPUT_PROJECT_SETTINGS_BACKEND_NAMESPACE::OUTPUT_PROJECT_SETTINGS_BACKEND which_setting, boost::property_tree::ptree & pt);

};

#endif
