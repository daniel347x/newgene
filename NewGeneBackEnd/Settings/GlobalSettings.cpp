#include "GlobalSettings.h"

void GlobalSettings::SetMapEntry(Messager & messager, SettingInfo & setting_info, int const enum_index, boost::property_tree::ptree & pt)
{

}

BackendGlobalSetting * GlobalSettings::CloneSetting(Messager & messager, BackendGlobalSetting * current_setting, SettingInfo & setting_info) const
{
	return NULL;
}

BackendGlobalSetting * GlobalSettings::NewSetting(Messager & messager, SettingInfo & setting_info, void const * setting_value_void)
{
	return NULL;
}

template<>
SettingInfo GetSettingInfoFromEnum<GLOBAL_SETTINGS_BACKEND_NAMESPACE::GLOBAL_SETTINGS_BACKEND>(GLOBAL_SETTINGS_BACKEND_NAMESPACE::GLOBAL_SETTINGS_BACKEND const value_)
{
	switch (value_)
	{
		//case GLOBAL_SETTINGS_UI_NAMESPACE::MRU_LIST: return "MRU_LIST";
	}

	return SettingInfo();
}

GlobalSettings::GlobalSettings(Messager & messager)
	: Settings(messager)
{

}

GlobalSettings::GlobalSettings(Messager & messager, boost::filesystem::path const global_settings_path)
	: Settings(messager, global_settings_path)
{

}
