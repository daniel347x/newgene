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
		, SETTING_LAST
	};

}

template<>
SettingInfo GetSettingInfoFromEnum<GLOBAL_SETTINGS_BACKEND_NAMESPACE::GLOBAL_SETTINGS_BACKEND>(GLOBAL_SETTINGS_BACKEND_NAMESPACE::GLOBAL_SETTINGS_BACKEND const value_);

class GlobalSettings : public Settings<GLOBAL_SETTINGS_BACKEND_NAMESPACE::GLOBAL_SETTINGS_BACKEND, BackendGlobalSetting>
{

	public:

		GlobalSettings(Messager & messager);
		GlobalSettings(Messager & messager, boost::filesystem::path const global_settings_path);
		virtual ~GlobalSettings() {}

		void LoadDefaultSettings(Messager & messager);
		void SetMapEntry(Messager & messager, SettingInfo & setting_info, int const enum_index, boost::property_tree::ptree & pt);


	private:

		std::map<GLOBAL_SETTINGS_BACKEND_NAMESPACE::GLOBAL_SETTINGS_BACKEND, BackendGlobalSetting> _ui_settings;

};

#endif
