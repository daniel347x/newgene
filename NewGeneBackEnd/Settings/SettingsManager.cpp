#include "SettingsManager.h"
#include "../Project/Project.h"

std::unique_ptr<BackendGlobalSetting> SettingsManager::getSetting(Messager & messager, GLOBAL_SETTINGS_BACKEND_NAMESPACE::GLOBAL_SETTINGS_BACKEND const which_setting)
{
	return _global_settings->GetSetting(messager, which_setting);
}

std::unique_ptr<BackendProjectInputSetting> SettingsManager::getSetting(Messager & messager, InputProjectSettings & project_settings, INPUT_PROJECT_SETTINGS_BACKEND_NAMESPACE::INPUT_PROJECT_SETTINGS_BACKEND const which_setting)
{
	return project_settings.GetSetting(messager, which_setting);
}

std::unique_ptr<BackendProjectOutputSetting> SettingsManager::getSetting(Messager & messager, OutputProjectSettings & project_settings, OUTPUT_PROJECT_SETTINGS_BACKEND_NAMESPACE::OUTPUT_PROJECT_SETTINGS_BACKEND const which_setting)
{
	return project_settings.GetSetting(messager, which_setting);
}

std::unique_ptr<InputModelSetting> SettingsManager::getSetting(Messager & messager, InputModelSettings & model_settings, INPUT_MODEL_SETTINGS_NAMESPACE::INPUT_MODEL_SETTINGS const which_setting)
{
	return model_settings.GetSetting(messager, which_setting);
}

std::unique_ptr<OutputModelSetting> SettingsManager::getSetting(Messager & messager, OutputModelSettings & model_settings, OUTPUT_MODEL_SETTINGS_NAMESPACE::OUTPUT_MODEL_SETTINGS const which_setting)
{
	return model_settings.GetSetting(messager, which_setting);
}
