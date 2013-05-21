#include "SettingsManager.h"
#include "..\Project\Project.h"

std::unique_ptr<BackendGlobalSetting> SettingsManager::getSetting(Messager & messager, GLOBAL_SETTINGS_BACKEND_NAMESPACE::GLOBAL_SETTINGS_BACKEND const which_setting)
{
	//std::unique_ptr<BackendGlobalSetting> the_setting_ = _global_settings->getBackendSettings().GetSetting(messager, which_setting);
	//BackendGlobalSetting * the_setting = the_setting_.get();
	//if (the_setting == NULL)
	//{
	//	boost::format msg("Cannot retrieve setting \"%1%\"");
	//	msg % which_setting;
	//	messager.AppendMessage(new MessagerWarningMessage(MESSAGER_MESSAGE__NO_SETTING_FOUND, msg.str()));
	//	return std::unique_ptr<BackendGlobalSetting>(new BackendGlobalSetting());
	//}
	//return the_setting_;
	return std::unique_ptr<BackendGlobalSetting>(new BackendGlobalSetting());
}

std::unique_ptr<BackendProjectInputSetting> SettingsManager::getSetting(Messager & messager, InputProject * project, INPUT_PROJECT_SETTINGS_BACKEND_NAMESPACE::INPUT_PROJECT_SETTINGS_BACKEND const which_setting)
{
	//if (project == NULL)
	//{
	//	boost::format msg("Cannot retrieve setting (\"%1%\") for a NULL project.");
	//	msg % which_setting;
	//	messager.AppendMessage(new MessagerWarningMessage(MESSAGER_MESSAGE__NO_PROJECT_AVAILABLE, msg.str()));
	//	return std::unique_ptr<BackendProjectSetting>(new BackendProjectSetting());
	//}
	//std::unique_ptr<BackendProjectSetting> the_setting_ = project->settings()->getBackendSettings().GetSetting(messager, which_setting);
	//BackendProjectSetting * the_setting = the_setting_.get();
	//if (the_setting == NULL)
	//{
	//	boost::format msg("Cannot retrieve setting \"%1%\"");
	//	msg % which_setting;
	//	messager.AppendMessage(new MessagerWarningMessage(MESSAGER_MESSAGE__NO_SETTING_FOUND, msg.str()));
	//	return std::unique_ptr<BackendProjectSetting>(new BackendProjectSetting());
	//}
	//return the_setting_;
	return std::unique_ptr<BackendProjectInputSetting>(new BackendProjectInputSetting());
}

std::unique_ptr<BackendProjectOutputSetting> SettingsManager::getSetting(Messager & messager, OutputProject * project, OUTPUT_PROJECT_SETTINGS_BACKEND_NAMESPACE::OUTPUT_PROJECT_SETTINGS_BACKEND const which_setting)
{
	//if (project == NULL)
	//{
	//	boost::format msg("Cannot retrieve setting (\"%1%\") for a NULL project.");
	//	msg % which_setting;
	//	messager.AppendMessage(new MessagerWarningMessage(MESSAGER_MESSAGE__NO_PROJECT_AVAILABLE, msg.str()));
	//	return std::unique_ptr<BackendProjectSetting>(new BackendProjectSetting());
	//}
	//std::unique_ptr<BackendProjectSetting> the_setting_ = project->settings()->getBackendSettings().GetSetting(messager, which_setting);
	//BackendProjectSetting * the_setting = the_setting_.get();
	//if (the_setting == NULL)
	//{
	//	boost::format msg("Cannot retrieve setting \"%1%\"");
	//	msg % which_setting;
	//	messager.AppendMessage(new MessagerWarningMessage(MESSAGER_MESSAGE__NO_SETTING_FOUND, msg.str()));
	//	return std::unique_ptr<BackendProjectSetting>(new BackendProjectSetting());
	//}
	//return the_setting_;
	return std::unique_ptr<BackendProjectOutputSetting>(new BackendProjectOutputSetting());
}
