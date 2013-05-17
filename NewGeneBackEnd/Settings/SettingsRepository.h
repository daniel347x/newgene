#ifndef SETTINGSREPOSITORY_H
#define SETTINGSREPOSITORY_H

#ifndef Q_MOC_RUN
#	include <boost/filesystem.hpp>
#endif
#include <map>

template<typename SETTINGS_ENUM, typename SETTING_CLASS>
class SettingsRepository
{

	protected:

		void LoadSettingsFromFile(boost::filesystem::path const path_to_settings)
		{

		}

		std::map<SETTINGS_ENUM, SETTING_CLASS> _settings_map;

};

#endif
