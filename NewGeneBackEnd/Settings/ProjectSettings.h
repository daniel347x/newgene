#ifndef PROJECTSETTINGS_H
#define PROJECTSETTINGS_H

#include "Settings.h"
#include "Setting.h"
#include <map>

namespace PROJECT_SETTINGS_BACKEND_NAMESPACE
{

	enum PROJECT_SETTINGS_BACKEND
	{
		  SETTING_FIRST = 0
		, SETTING_LAST
	};

}

template<>
SettingInfo GetSettingInfoFromEnum<PROJECT_SETTINGS_BACKEND_NAMESPACE::PROJECT_SETTINGS_BACKEND>(Messager & messager, PROJECT_SETTINGS_BACKEND_NAMESPACE::PROJECT_SETTINGS_BACKEND const value_);

class ProjectSettings : public Settings<PROJECT_SETTINGS_BACKEND_NAMESPACE::PROJECT_SETTINGS_BACKEND, BackendProjectSetting>
{

	public:

		ProjectSettings(Messager & messager);
		ProjectSettings(Messager & messager, boost::filesystem::path const project_settings_path);
		virtual ~ProjectSettings() {}


	protected:

		boost::filesystem::path GetSettingsPath(Messager & messager, SettingInfo & setting_info);
		void SetMapEntry(Messager & messager, SettingInfo & setting_info, boost::property_tree::ptree & pt);
		BackendProjectSetting * CloneSetting(Messager & messager, BackendProjectSetting * current_setting, SettingInfo & setting_info) const;
		BackendProjectSetting * NewSetting(Messager & messager, SettingInfo & setting_info, void const * setting_value_void);

};

#endif
