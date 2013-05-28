#include "SettingsManager.h"
#include "..\Project\Project.h"

std::unique_ptr<BackendGlobalSetting> SettingsManager::getSetting(Messager & messager, GLOBAL_SETTINGS_BACKEND_NAMESPACE::GLOBAL_SETTINGS_BACKEND const which_setting)
{
	return _global_settings->GetSetting(messager, which_setting);
}

std::unique_ptr<BackendProjectInputSetting> SettingsManager::getSetting(Messager & messager, InputProject & project, INPUT_PROJECT_SETTINGS_BACKEND_NAMESPACE::INPUT_PROJECT_SETTINGS_BACKEND const which_setting)
{
	return project.settings().GetSetting(messager, which_setting);
}

std::unique_ptr<BackendProjectOutputSetting> SettingsManager::getSetting(Messager & messager, OutputProject & project, OUTPUT_PROJECT_SETTINGS_BACKEND_NAMESPACE::OUTPUT_PROJECT_SETTINGS_BACKEND const which_setting)
{
	return project.settings().GetSetting(messager, which_setting);
}
