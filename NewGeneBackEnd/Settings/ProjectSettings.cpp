#include "ProjectSettings.h"

ProjectSettings::ProjectSettings(Messager & messager)
	: Settings(messager)
{

}

ProjectSettings::ProjectSettings(Messager & messager, boost::filesystem::path const project_settings_path)
	: Settings(messager, project_settings_path)
{

}

void ProjectSettings::LoadDefaultSettings(Messager &messager)
{

}

template<>
std::string GetSettingTextFromEnum<PROJECT_SETTINGS_BACKEND_NAMESPACE::PROJECT_SETTINGS_BACKEND>(PROJECT_SETTINGS_BACKEND_NAMESPACE::PROJECT_SETTINGS_BACKEND const value_)
{
	switch (value_)
	{
		//case GLOBAL_SETTINGS_UI_NAMESPACE::MRU_LIST: return "MRU_LIST";
	}

	return "";
}
