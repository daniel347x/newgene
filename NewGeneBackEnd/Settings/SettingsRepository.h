#ifndef SETTINGSREPOSITORY_H
#define SETTINGSREPOSITORY_H

#ifndef Q_MOC_RUN
#	include <boost/filesystem.hpp>
#	include <boost/format.hpp>
#endif
#include <map>
#include "..\Messager\Messager.h"
#ifndef Q_MOC_RUN
#	include <boost/property_tree/ptree.hpp>
#	include <boost/property_tree/xml_parser.hpp>
#endif

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

		virtual void LoadDefaultSettings(Messager & messager) = 0;

		void LoadSettingsFromFile(Messager & messager, boost::filesystem::path const path_to_settings)
		{

			if ( !boost::filesystem::exists(path_to_settings) || !boost::filesystem::is_regular_file(path_to_settings) )
			{
				boost::format msg("Settings file %1% does not exist.");
				msg % path_to_settings;
				messager.AppendMessage(new MessagerWarningMessage(MESSAGER_MESSAGE__FILE_DOES_NOT_EXIST, msg.str()));
				return;
			}

			boost::property_tree::ptree pt;

			try
			{
				boost::property_tree::xml_parser::read_xml(path_to_settings.string(), pt);
			}
			catch (const boost::property_tree::xml_parser_error & e)
			{
				boost::format msg("Settings file %1% is not in the correct format: %2%");
				msg % path_to_settings % e.what();
				messager.AppendMessage(new MessagerWarningMessage(MESSAGER_MESSAGE__FILE_INVALID_FORMAT, msg.str()));
				return;
			}

		}

		std::map<SETTINGS_ENUM, SETTING_CLASS> _settings_map;

};

#endif
