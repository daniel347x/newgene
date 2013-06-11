#ifndef MODELOUTPUTSETTINGS_H
#define MODELOUTPUTSETTINGS_H

#include "ModelSettings.h"

namespace OUTPUT_MODEL_SETTINGS_NAMESPACE
{

	enum OUTPUT_MODEL_SETTINGS
	{
		SETTING_FIRST = 0
		, SETTING_LAST
	};

}

class OutputModelSettings : public ModelSettings<OUTPUT_MODEL_SETTINGS_NAMESPACE::OUTPUT_MODEL_SETTINGS, OutputModelSetting>
{

public:

	OutputModelSettings(Messager & messager, boost::filesystem::path const model_settings_path)
		: ModelSettings(messager, model_settings_path)
	{}

	virtual ~OutputModelSettings() {}

	void SetMapEntry(Messager & messager, SettingInfo & setting_info, boost::property_tree::ptree & pt);
	OutputModelSetting * CloneSetting(Messager & messager, OutputModelSetting * current_setting, SettingInfo & setting_info) const;
	OutputModelSetting * NewSetting(Messager & messager, SettingInfo & setting_info, void const * setting_value_void = NULL);
	void SetPTreeEntry(Messager & messager, OUTPUT_MODEL_SETTINGS_NAMESPACE::OUTPUT_MODEL_SETTINGS which_setting, boost::property_tree::ptree & pt);

};

#endif
