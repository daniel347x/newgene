#ifndef SETTINGSMANAGER_H
#define SETTINGSMANAGER_H

#include "../globals.h"
#include "../Manager.h"
#ifndef Q_MOC_RUN
#   include <boost\filesystem.hpp>
#   include <boost/property_tree/ptree.hpp>
#   include <boost/property_tree/xml_parser.hpp>
#endif
#include "Setting.h"
#include "GlobalSettings.h"
#include "ProjectSettings.h"
#include "../Project/Project.h"

class SettingsManager : public Manager<SettingsManager, MANAGER_DESCRIPTION_NAMESPACE::MANAGER_SETTINGS>
{

	public:

		std::unique_ptr<BackendGlobalSetting> getSetting(Messager & messager, GLOBAL_SETTINGS_BACKEND_NAMESPACE::GLOBAL_SETTINGS_BACKEND const which_setting);
		std::unique_ptr<BackendProjectSetting> getSetting(Messager & messager, Project * project, PROJECT_SETTINGS_BACKEND_NAMESPACE::PROJECT_SETTINGS_BACKEND const which_setting);

};

#endif
