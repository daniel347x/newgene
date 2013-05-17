#include "ProjectSettings.h"

ProjectSettings::ProjectSettings(Messager & messager)
	: Settings(messager)
{

}

ProjectSettings::ProjectSettings(Messager & messager, boost::filesystem::path const project_settings_path)
	: Settings(messager, project_settings_path)
{

}
