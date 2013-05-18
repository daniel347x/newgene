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
