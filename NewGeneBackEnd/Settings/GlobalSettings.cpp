#include "GlobalSettings.h"

GlobalSettings::GlobalSettings(Messager & messager)
	: Settings(messager)
{

}

GlobalSettings::GlobalSettings(Messager & messager, boost::filesystem::path const global_settings_path)
	: Settings(messager, global_settings_path)
{

}

void GlobalSettings::LoadDefaultSettings(Messager &messager)
{
}

template<>
std::string GetSettingTextFromEnum<GLOBAL_SETTINGS_BACKEND_NAMESPACE::GLOBAL_SETTINGS_BACKEND>(GLOBAL_SETTINGS_BACKEND_NAMESPACE::GLOBAL_SETTINGS_BACKEND const value_)
{
	switch (value_)
	{
		//case GLOBAL_SETTINGS_UI_NAMESPACE::MRU_LIST: return "MRU_LIST";
	}

	return "";
}
