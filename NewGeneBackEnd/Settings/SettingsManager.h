#ifndef SETTINGSMANAGER_H
#define SETTINGSMANAGER_H

#include "../globals.h"
#include "../Manager.h"
#ifndef Q_MOC_RUN
#   include <boost/filesystem.hpp>
#   include <boost/property_tree/ptree.hpp>
#   include <boost/property_tree/xml_parser.hpp>
#endif
#include "Setting.h"
#include "GlobalSettings.h"
#include "ProjectSettings.h"
#include "../Project/InputProject.h"
#include "../Project/OutputProject.h"

class SettingsManager : public Manager<SettingsManager, MANAGER_DESCRIPTION_NAMESPACE::MANAGER_SETTINGS>
{

	public:
		SettingsManager(Messager & messager_) : Manager<SettingsManager, MANAGER_DESCRIPTION_NAMESPACE::MANAGER_SETTINGS>(messager_) {}

	public:

		std::unique_ptr<BackendGlobalSetting> getSetting(Messager & messager, GLOBAL_SETTINGS_BACKEND_NAMESPACE::GLOBAL_SETTINGS_BACKEND const which_setting);

		std::unique_ptr<BackendProjectInputSetting> getSetting(Messager & messager, InputProjectSettings & project_settings, INPUT_PROJECT_SETTINGS_BACKEND_NAMESPACE::INPUT_PROJECT_SETTINGS_BACKEND const which_setting);
		std::unique_ptr<BackendProjectOutputSetting> getSetting(Messager & messager, OutputProjectSettings & project_settings, OUTPUT_PROJECT_SETTINGS_BACKEND_NAMESPACE::OUTPUT_PROJECT_SETTINGS_BACKEND const which_setting);

		std::unique_ptr<InputModelSetting> getSetting(Messager & messager, InputModelSettings & model_settings, INPUT_MODEL_SETTINGS_NAMESPACE::INPUT_MODEL_SETTINGS const which_setting);
		std::unique_ptr<OutputModelSetting> getSetting(Messager & messager, OutputModelSettings & model_settings, OUTPUT_MODEL_SETTINGS_NAMESPACE::OUTPUT_MODEL_SETTINGS const which_setting);

		std::unique_ptr<GlobalSettings> _global_settings;

};

#endif
