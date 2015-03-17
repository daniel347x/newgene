#ifndef MODELINPUTSETTINGS_H
#define MODELINPUTSETTINGS_H

#include "ModelSettings.h"

namespace INPUT_MODEL_SETTINGS_NAMESPACE
{

	enum INPUT_MODEL_SETTINGS
	{
		SETTING_FIRST = 0

		, PATH_TO_MODEL_DATABASE

		, SETTING_LAST
	};

}

class InputModelSettings : public ModelSettings<INPUT_MODEL_SETTINGS_NAMESPACE::INPUT_MODEL_SETTINGS, InputModelSetting>
{

	public:

		static int const number_worker_threads = 1; // For now, single thread only in pool

		InputModelSettings(Messager & messager, boost::filesystem::path const model_settings_path)
			: ModelSettings(messager, model_settings_path)
		{}

		virtual ~InputModelSettings() {}

		template<typename T>
		void UpdateSetting(Messager & messager, INPUT_MODEL_SETTINGS_NAMESPACE::INPUT_MODEL_SETTINGS const which_setting, T const & setting_value)
		{
			SettingInfo setting_info = SettingInfoObject.GetSettingInfoFromEnum(messager, which_setting);
			_settings_map[which_setting] = std::unique_ptr<InputModelSetting>(NewSetting(messager, setting_info, setting_value.ToString()));
			WriteSettingsToFile(messager);
		}

		void WriteSettingsToFile(Messager & messager)
		{
			boost::property_tree::ptree pt;
			WriteSettingsToPtree(messager, pt);
			WritePtreeToFile(messager, pt);
		}

		void SetMapEntry(Messager & messager, SettingInfo & setting_info, boost::property_tree::ptree & pt);
		InputModelSetting * CloneSetting(Messager & messager, InputModelSetting * current_setting, SettingInfo & setting_info) const;
		InputModelSetting * NewSetting(Messager & messager, SettingInfo & setting_info, std::string const & setting_value_string);
		void SetPTreeEntry(Messager & messager, INPUT_MODEL_SETTINGS_NAMESPACE::INPUT_MODEL_SETTINGS which_setting, boost::property_tree::ptree & pt);

};

#endif
