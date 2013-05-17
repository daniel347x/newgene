#ifndef SETTINGSREPOSITORY_H
#define SETTINGSREPOSITORY_H

#ifndef Q_MOC_RUN
#	include <boost/filesystem.hpp>
#	include <boost/format.hpp>
#endif
#include <map>
#include "..\Messager\Messager.h"

template<typename SETTINGS_ENUM, typename SETTING_CLASS>
class SettingsRepository
{

public:

	SettingsRepository(Messager &)
	{
	}

	SettingsRepository(Messager & messager, boost::filesystem::path const path_to_settings)
	{
		LoadSettingsFromFile(messager, path_to_settings);
	}

	protected:

		void LoadSettingsFromFile(Messager & messager, boost::filesystem::path const path_to_settings)
		{

			if ( !boost::filesystem::exists(path_to_settings) || !boost::filesystem::is_regular_file(path_to_settings) )
			{
				boost::format msg("Settings file %1% does not exist.");
				msg % path_to_settings;
				messager.AppendMessage(new MessagerWarningMessage(MESSAGER_MESSAGE__FILE_DOES_NOT_EXIST, msg.str()));
				return;
			}

		}

		std::map<SETTINGS_ENUM, SETTING_CLASS> _settings_map;

};

#endif
