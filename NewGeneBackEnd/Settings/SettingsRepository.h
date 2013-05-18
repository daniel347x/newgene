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
#include <cstdint>

class SettingInfo
{

public:

	enum SETTING_CLASS_ENUM
	{
		  SETTING_CLASS_NONE
		, SETTING_CLASS_BACKEND_GLOBAL_SETTING
		, SETTING_CLASS_BACKEND_PROJECT_SETTING
		, SETTING_CLASS_UI_GLOBAL_SETTING
		, SETTING_CLASS_UI_PROJECT_SETTING
		, SETTING_CLASS_UI_GLOBAL_SETTING_MRU_LIST
	};

	SettingInfo()
		: setting_class(SETTING_CLASS_NONE)
		, text("")
		, default_val_string("")
		, default_val_int32(0)
	{

	}

	SettingInfo(SETTING_CLASS_ENUM const setting_class_enum_, std::string const & text_, std::string default_val_string_)
		: setting_class(setting_class_enum_)
		, text(text_)
		, default_val_string(default_val_string_)
		, default_val_int32(0)
	{

	}

	SettingInfo(SETTING_CLASS_ENUM const setting_class_enum_, std::string const & text_, std::int32_t default_val_int32_)
		: setting_class(setting_class_enum_)
		, text(text_)
		, default_val_string("")
		, default_val_int32(default_val_int32_)
	{

	}

	SETTING_CLASS_ENUM setting_class;
	std::string text;
	std::string default_val_string;
	std::int32_t default_val_int32;

};

template<typename SETTINGS_ENUM, typename SETTING_CLASS>
class SettingsRepository
{

	public:

		template<typename SETTINGS_REPOSITORY_CLASS>
		friend class SettingsRepositoryFactory;

		typedef std::map<SETTINGS_ENUM, std::unique_ptr<SETTING_CLASS> > SettingsMap;

		virtual void SetMapEntry(Messager & messager, SettingInfo & setting_info, int const enum_index, boost::property_tree::ptree & pt) = 0;

	protected:

		void LoadSettingsFromFile(Messager & messager, boost::filesystem::path const path_to_settings)
		{

			bool no_file = false;

			if ( !boost::filesystem::exists(path_to_settings) )
			{
				no_file = true; // no file is fine
			}

			else if ( boost::filesystem::file_size(path_to_settings) == 0 )
			{
				no_file = true; // empty file is fine
			}

			else if ( !boost::filesystem::is_regular_file(path_to_settings) )
			{
				boost::format msg("Settings file %1% is not available.  Using default settings.");
				msg % path_to_settings;
				messager.AppendMessage(new MessagerWarningMessage(MESSAGER_MESSAGE__FILE_DOES_NOT_EXIST, msg.str()));
				no_file = true;
			}

			boost::property_tree::ptree pt;

			if (!no_file)
			{
				try
				{
					boost::property_tree::xml_parser::read_xml(path_to_settings.string(), pt);
				}
				catch (const boost::property_tree::xml_parser_error & e)
				{
					boost::format msg("Settings file %1% is not in the correct format: %2%");
					msg % path_to_settings % e.what();
					messager.AppendMessage(new MessagerWarningMessage(MESSAGER_MESSAGE__FILE_INVALID_FORMAT, msg.str()));
				}
			}

			for ( int n = static_cast<int>(SETTINGS_ENUM::SETTING_FIRST) + 1; n < static_cast<int>(SETTINGS_ENUM::SETTING_LAST); ++n)
			{
				SettingInfo setting_info = GetSettingInfoFromEnum<SETTINGS_ENUM>(static_cast<SETTINGS_ENUM>(n));
				SetMapEntry(messager, setting_info, n, pt); // sets default value if not present in property tree at this point
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
		new_settings_repository->LoadSettingsFromFile(messager, boost::filesystem::path());
		return new_settings_repository;
	}

	SETTINGS_REPOSITORY_CLASS * operator()(Messager & messager, boost::filesystem::path const path_to_settings)
	{
		SETTINGS_REPOSITORY_CLASS * new_settings_repository = new SETTINGS_REPOSITORY_CLASS(messager, path_to_settings);
		new_settings_repository->LoadSettingsFromFile(messager, path_to_settings);
		return new_settings_repository;
	}

};

#endif
