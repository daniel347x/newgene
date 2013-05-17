#ifndef GLOBALSETTINGS_H
#define GLOBALSETTINGS_H

#include "Settings.h"

class GlobalSettings : public Settings
{

	public:

		GlobalSettings(boost::filesystem::path const global_settings_path);
		virtual ~GlobalSettings() {}

};

#endif
