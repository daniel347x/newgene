#ifndef GLOBALSETTINGS_H
#define GLOBALSETTINGS_H

#include "Settings.h"
#include "Setting.h"
#include <map>

enum GLOBAL_SETTINGS_BACKEND
{
	GLOBAL_SETTING_BACKEND__LAST
};

class GlobalSettings : public Settings
{

	public:

		GlobalSettings(boost::filesystem::path const global_settings_path);
		virtual ~GlobalSettings() {}

	private:

		std::map<GLOBAL_SETTINGS_BACKEND, BackendGlobalSetting> _ui_settings;

};

#endif
