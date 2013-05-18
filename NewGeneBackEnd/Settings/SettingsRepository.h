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

		template<typename SETTINGS_REPOSITORY_CLASS>
		friend class SettingsRepositoryFactory;

		typedef std::map<SETTINGS_ENUM, std::unique_ptr<SETTING_CLASS> > SettingsMap;

		virtual void LoadDefaultSettings(Messager & messager) = 0;

	protected:

		void LoadSettingsFromFile(Messager & messager, boost::filesystem::path const path_to_settings)
		{

			if ( !boost::filesystem::exists(path_to_settings) )
			{
				return; // no file is fine
			}

			else if ( boost::filesystem::file_size(path_to_settings) == 0 )
			{
				return; // empty file is fine
			}

			else if ( !boost::filesystem::is_regular_file(path_to_settings) )
			{
				boost::format msg("Settings file %1% is not available.");
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

			for ( int n = static_cast<int>(SETTINGS_ENUM::SETTING_FIRST) + 1; n < static_cast<int>(SETTINGS_ENUM::SETTING_LAST); ++n)
			{
				std::string setting_text = GetSettingTextFromEnum<SETTINGS_ENUM>(static_cast<SETTINGS_ENUM>(n));

				boost::format msg("The setting name is \"%1%\"");
				msg % setting_text;
				messager.AppendMessage(new MessagerWarningMessage(MESSAGER_MESSAGE__GENERAL_ERROR, msg.str()));
				return;
			}

		}

		SettingsMap _settings_map;

		SettingsRepository(Messager & messager)
		{
		}

		SettingsRepository(Messager & messager, boost::filesystem::path const path_to_settings)
		{
		}

};

template<typename SETTINGS_REPOSITORY_CLASS>
class SettingsRepositoryFactory
{

public:

	SETTINGS_REPOSITORY_CLASS * operator()(Messager & messager)
	{
		SETTINGS_REPOSITORY_CLASS * new_settings_repository = new SETTINGS_REPOSITORY_CLASS(messager);
		new_settings_repository->LoadDefaultSettings(messager);
		return new_settings_repository;
	}

	SETTINGS_REPOSITORY_CLASS * operator()(Messager & messager, boost::filesystem::path const path_to_settings)
	{
		SETTINGS_REPOSITORY_CLASS * new_settings_repository = new SETTINGS_REPOSITORY_CLASS(messager, path_to_settings);
		new_settings_repository->LoadDefaultSettings(messager);
		new_settings_repository->LoadSettingsFromFile(messager, path_to_settings);
		return new_settings_repository;
	}

};

#endif
