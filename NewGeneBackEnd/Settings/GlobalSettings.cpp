#include "GlobalSettings.h"

GlobalSettings::GlobalSettings()
	: Settings()
{

}

GlobalSettings::GlobalSettings(boost::filesystem::path const global_settings_path)
	: Settings(global_settings_path)
{

}
