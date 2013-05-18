#include "ProjectSettings.h"

void ProjectSettings::SetMapEntry(Messager & messager, SettingInfo & setting_info, int const enum_index, boost::property_tree::ptree & pt)
{

}

template<>
SettingInfo GetSettingInfoFromEnum<PROJECT_SETTINGS_BACKEND_NAMESPACE::PROJECT_SETTINGS_BACKEND>(PROJECT_SETTINGS_BACKEND_NAMESPACE::PROJECT_SETTINGS_BACKEND const value_)
{
	switch (value_)
	{
		//case GLOBAL_SETTINGS_UI_NAMESPACE::MRU_LIST: return "MRU_LIST";
	}

	return SettingInfo();
}

ProjectSettings::ProjectSettings(Messager & messager)
	: Settings(messager)
{

}

ProjectSettings::ProjectSettings(Messager & messager, boost::filesystem::path const project_settings_path)
	: Settings(messager, project_settings_path)
{

}
