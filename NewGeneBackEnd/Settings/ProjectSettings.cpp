#include "ProjectSettings.h"

ProjectSettings::ProjectSettings()
	: Settings()
{

}

ProjectSettings::ProjectSettings(boost::filesystem::path const project_settings_path)
	: Settings(project_settings_path)
{

}
