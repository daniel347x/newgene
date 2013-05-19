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

class SettingsManager : public Manager<SettingsManager, MANAGER_DESCRIPTION_NAMESPACE::MANAGER_SETTINGS>
{

	public:

		BackendGlobalSetting const & get_setting(GLOBAL_SETTINGS_BACKEND_NAMESPACE::GLOBAL_SETTINGS_BACKEND const which_setting);
		BackendProjectSetting const & get_setting(PROJECT_SETTINGS_BACKEND_NAMESPACE::PROJECT_SETTINGS_BACKEND const which_setting);

};

#endif
