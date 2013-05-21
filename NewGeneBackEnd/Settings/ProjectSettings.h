#ifndef PROJECTSETTINGS_H
#define PROJECTSETTINGS_H

#include "Settings.h"
#include "Setting.h"
#include <map>

template<typename PROJECT_SETTINGS_ENUM, typename BACKEND_PROJECT_SETTING_CLASS>
class ProjectSettings : public Settings<PROJECT_SETTINGS_ENUM, BACKEND_PROJECT_SETTING_CLASS>
{

	public:

		ProjectSettings(Messager & messager, boost::filesystem::path const project_settings_path)
			: Settings(messager, project_settings_path)
		{

		}

		virtual ~ProjectSettings() {}


	protected:

		boost::filesystem::path GetSettingsPath(Messager & messager, SettingInfo & setting_info)
		{
			return boost::filesystem::path();
		}

};

#endif
