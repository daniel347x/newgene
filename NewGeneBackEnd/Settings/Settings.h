#ifndef SETTINGS_H
#define SETTINGS_H

#include <boost/filesystem.hpp>

class Settings
{

	public:

		Settings(boost::filesystem::path const settings_path);
		virtual ~Settings() {}

};

#endif
