#ifndef MODELSETTINGS_H
#define MODELSETTINGS_H

#include "Settings.h"
#include "Setting.h"
#include <map>

template<typename MODEL_SETTINGS_ENUM, typename MODEL_SETTING_CLASS>
class ModelSettings : public Settings<MODEL_SETTINGS_ENUM, MODEL_SETTING_CLASS>
{

	public:

		ModelSettings(Messager & messager, boost::filesystem::path const model_settings_path)
			: Settings<MODEL_SETTINGS_ENUM, MODEL_SETTING_CLASS>(messager, model_settings_path)
		{

		}

		void WriteSettingsToFile(Messager & messager)
		{
			boost::property_tree::ptree pt;
			SettingsRepository<MODEL_SETTINGS_ENUM, MODEL_SETTING_CLASS>::WriteSettingsToPtree(messager, pt);
			SettingsRepository<MODEL_SETTINGS_ENUM, MODEL_SETTING_CLASS>::WritePtreeToFile(messager, pt);
		}

		virtual ~ModelSettings() {}


	protected:

};

#endif
