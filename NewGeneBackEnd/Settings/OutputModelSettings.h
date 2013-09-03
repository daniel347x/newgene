#ifndef MODELOUTPUTSETTINGS_H
#define MODELOUTPUTSETTINGS_H

#include "ModelSettings.h"

namespace OUTPUT_MODEL_SETTINGS_NAMESPACE
{

	enum OUTPUT_MODEL_SETTINGS
	{
		  SETTING_FIRST = 0

		, PATH_TO_MODEL_DATABASE

		, SETTING_LAST
	};

}

class OutputModelSettings : public ModelSettings<OUTPUT_MODEL_SETTINGS_NAMESPACE::OUTPUT_MODEL_SETTINGS, OutputModelSetting>
{

public:

	static int const number_worker_threads = 1; // For now, single thread only in pool

	OutputModelSettings(Messager & messager, boost::filesystem::path const model_settings_path)
		: ModelSettings(messager, model_settings_path)
	{}

	virtual ~OutputModelSettings() {}

	template<typename T>
	void UpdateSetting(Messager & messager, OUTPUT_MODEL_SETTINGS_NAMESPACE::OUTPUT_MODEL_SETTINGS const which_setting, T const & setting_value)
	{
		SettingInfo setting_info = SettingInfoObject.GetSettingInfoFromEnum(messager, which_setting);
		_settings_map[which_setting] = std::unique_ptr<OutputModelSetting>(NewSetting(messager, setting_info, (void const *)(&setting_value)));
		WriteSettingsToFile(messager);
	}

	void WriteSettingsToFile(Messager & messager)
	{
		boost::property_tree::ptree pt;
		WriteSettingsToPtree(messager, pt);
		WritePtreeToFile(messager, pt);
	}

	void SetMapEntry(Messager & messager, SettingInfo & setting_info, boost::property_tree::ptree & pt);
	OutputModelSetting * CloneSetting(Messager & messager, OutputModelSetting * current_setting, SettingInfo & setting_info) const;
	OutputModelSetting * NewSetting(Messager & messager, SettingInfo & setting_info, void const * setting_value_void = NULL);
	void SetPTreeEntry(Messager & messager, OUTPUT_MODEL_SETTINGS_NAMESPACE::OUTPUT_MODEL_SETTINGS which_setting, boost::property_tree::ptree & pt);

};

#endif
