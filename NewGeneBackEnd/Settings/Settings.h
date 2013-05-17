#ifndef SETTINGS_H
#define SETTINGS_H

#ifndef Q_MOC_RUN
#	include <boost/filesystem.hpp>
#endif

#include "SettingsParser.h"

class Settings
{

	public:

		Settings(boost::filesystem::path const settings_path);
		virtual ~Settings() {}

};

#endif
