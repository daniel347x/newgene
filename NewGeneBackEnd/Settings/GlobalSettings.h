#ifndef GLOBALSETTINGS_H
#define GLOBALSETTINGS_H

#include "Settings.h"
#include "Setting.h"
#include <map>

namespace GLOBAL_SETTINGS_BACKEND_NAMESPACE
{

	enum GLOBAL_SETTINGS_BACKEND
	{
		  SETTING_FIRST = 0

		, TEST_SETTING

		, SETTING_LAST
	};

}

class GlobalSettings : public Settings<GLOBAL_SETTINGS_BACKEND_NAMESPACE::GLOBAL_SETTINGS_BACKEND, BackendGlobalSetting>
{

	public:

		GlobalSettings(Messager & messager, boost::filesystem::path const global_settings_path);
		virtual ~GlobalSettings() {}


	protected:

		boost::filesystem::path GetSettingsPath(Messager & messager, SettingInfo & setting_info);
		void SetMapEntry(Messager & messager, SettingInfo & setting_info, boost::property_tree::ptree & pt);
		BackendGlobalSetting * CloneSetting(Messager & messager, BackendGlobalSetting * current_setting, SettingInfo & setting_info) const;
		BackendGlobalSetting * NewSetting(Messager & messager, SettingInfo & setting_info, void const * setting_value_void = NULL);

};

#endif
