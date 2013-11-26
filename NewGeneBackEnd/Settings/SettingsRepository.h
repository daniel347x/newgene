#ifndef SETTINGSREPOSITORY_H
#define SETTINGSREPOSITORY_H

#ifndef Q_MOC_RUN
#	include <boost/filesystem.hpp>
#	include <boost/format.hpp>
#endif
#include <map>
#include "../Messager/Messager.h"
#ifndef Q_MOC_RUN
#	include <boost/property_tree/ptree.hpp>
#	include <boost/property_tree/xml_parser.hpp>
#endif
#include <cstdint>

class UIMessager;
extern Messager * dummy_messager_ptr;

class SettingInfo
{

public:

	enum SETTING_CLASS_ENUM
	{

		  SETTING_CLASS_NONE

		, SETTING_CLASS_BACKEND_GLOBAL_SETTING
		, SETTING_CLASS_BACKEND_GLOBAL_SETTING__TEST

		, SETTING_CLASS_BACKEND_PROJECT_INPUT_SETTING
		, SETTING_CLASS_BACKEND_PROJECT_INPUT_SETTING__PATH_TO_MODEL_SETTINGS

		, SETTING_CLASS_BACKEND_PROJECT_OUTPUT_SETTING
		, SETTING_CLASS_BACKEND_PROJECT_OUTPUT_SETTING__PATH_TO_MODEL_SETTINGS
		, SETTING_CLASS_BACKEND_PROJECT_OUTPUT_SETTING__PATH_TO_KAD_OUTPUT_FILE

		, SETTING_CLASS_MODEL_INPUT_SETTING
		, SETTING_CLASS_MODEL_INPUT_SETTING__PATH_TO_MODEL_DATABASE

		, SETTING_CLASS_MODEL_OUTPUT_SETTING
		, SETTING_CLASS_MODEL_OUTPUT_SETTING__PATH_TO_MODEL_DATABASE

		, SETTING_CLASS_UI_GLOBAL_SETTING
		, SETTING_CLASS_UI_GLOBAL_SETTING__MRU_INPUT_PROJECTS_LIST
		, SETTING_CLASS_UI_GLOBAL_SETTING__MRU_OUTPUT_PROJECTS_LIST
		, SETTING_CLASS_UI_GLOBAL_SETTING__OPEN_INPUT_PROJECTS_LIST
		, SETTING_CLASS_UI_GLOBAL_SETTING__OPEN_OUTPUT_PROJECTS_LIST
		, SETTING_CLASS_UI_GLOBAL_SETTING__OPEN_INPUT_DATASET_FOLDER_PATH
		, SETTING_CLASS_UI_GLOBAL_SETTING__OPEN_OUTPUT_DATASET_FOLDER_PATH

		, SETTING_CLASS_UI_PROJECT_INPUT_SETTING

		, SETTING_CLASS_UI_PROJECT_OUTPUT_SETTING

	};

	enum SETTING_CLASS_CATEGORY
	{
		SETTING_CLASS_CATEGORY__UNKNOWN
		, SETTING_CLASS_CATEGORY__STRING
		, SETTING_CLASS_CATEGORY__INT32
	};

	SettingInfo()
		: setting_class(SETTING_CLASS_NONE)
		, setting_category(SETTING_CLASS_CATEGORY__UNKNOWN)
        , text("")
        , default_val_string("")
        , default_val_int32(0)
        , enum_index(0)
    {

	}

	SettingInfo(SETTING_CLASS_ENUM const setting_class_enum_, int const enum_index_, std::string const & text_, std::string default_val_string_)
		: setting_class(setting_class_enum_)
		, setting_category(SETTING_CLASS_CATEGORY__STRING)
        , text(text_)
        , default_val_string(default_val_string_)
        , default_val_int32(0)
        , enum_index(enum_index_)
    {

	}

	SettingInfo(SETTING_CLASS_ENUM const setting_class_enum_, int const enum_index_, std::string const & text_, std::int32_t default_val_int32_)
		: setting_class(setting_class_enum_)
		, setting_category(SETTING_CLASS_CATEGORY__INT32)
        , text(text_)
        , default_val_string("")
        , default_val_int32(default_val_int32_)
        , enum_index(enum_index_)
    {

	}

	virtual void * getDefaultValue() {return NULL;};

	SETTING_CLASS_ENUM setting_class;
	SETTING_CLASS_CATEGORY setting_category;
	std::string text;
	std::string default_val_string;
	std::int32_t default_val_int32;
	int enum_index;

};

template<SettingInfo::SETTING_CLASS_ENUM setting_type>
class SettingClassTypeTraits
{
public:
	typedef void * type;
};

template<SettingInfo::SETTING_CLASS_CATEGORY setting_category>
class SettingCategoryTypeTraits
{
};

template<>
class SettingCategoryTypeTraits<SettingInfo::SETTING_CLASS_CATEGORY__STRING>
{
public:
	typedef std::string type;
};

template<>
class SettingCategoryTypeTraits<SettingInfo::SETTING_CLASS_CATEGORY__INT32>
{
public:
	typedef std::int32_t type;
};

template<typename SETTINGS_ENUM, typename SETTING_CLASS>
class SettingsRepository
{

	public:

		template<typename SETTINGS_REPOSITORY_CLASS>
		friend class SettingsRepositoryFactory;

		typedef std::map<SETTINGS_ENUM, std::unique_ptr<SETTING_CLASS> > SettingsMap;

		std::unique_ptr<SETTING_CLASS> GetSetting(Messager & messager, SETTINGS_ENUM const which_setting)
		{
			SettingInfo setting_info = SettingInfoObject.GetSettingInfoFromEnum(messager, which_setting);
			typename SettingsMap::const_iterator theSetting = _settings_map.find(which_setting);
			if (theSetting == _settings_map.cend())
			{
				SettingInfo setting_info = SettingInfoObject.GetSettingInfoFromEnum(messager, which_setting);
				_settings_map[which_setting] = std::unique_ptr<SETTING_CLASS>(NewSetting(messager, setting_info));
				theSetting = _settings_map.find(which_setting);
				if (theSetting == _settings_map.cend())
				{
					boost::format msg("Setting cannot be created.");
					messager.AppendMessage(new MessagerCatastrophicErrorMessage(MESSAGER_MESSAGE__SETTING_NOT_CREATED, msg.str()));
					return std::unique_ptr<SETTING_CLASS>(new SETTING_CLASS(messager));
				}
			}
			return std::unique_ptr<SETTING_CLASS>(CloneSetting(messager, theSetting->second.get(), setting_info));
		}

	protected:

        virtual void SetMapEntry(Messager &, SettingInfo &, boost::property_tree::ptree &) {}
        virtual SETTING_CLASS * CloneSetting(Messager & messager, SETTING_CLASS *, SettingInfo &) const { return new SETTING_CLASS(messager); };
        virtual SETTING_CLASS * NewSetting(Messager & messager, SettingInfo &, std::string const & setting_value_void = NULL) { Q_UNUSED(setting_value_void); return new SETTING_CLASS(messager); };
        virtual void SetPTreeEntry(Messager &, SETTINGS_ENUM, boost::property_tree::ptree &) {};

		void LoadSettingsFromFile(Messager & messager)
		{

			boost::filesystem::path path_to_settings = _path_to_settings;

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
				SettingInfo setting_info = SettingInfoObject.GetSettingInfoFromEnum(messager, static_cast<SETTINGS_ENUM>(n));
				SetMapEntry(messager, setting_info, pt); // sets default value if not present in property tree at this point; i.e., if no path is present
			}

		}

	public:

		void WriteSettingsToPtree(Messager & messager, boost::property_tree::ptree & pt)
		{

			for ( int n = static_cast<int>(SETTINGS_ENUM::SETTING_FIRST) + 1; n < static_cast<int>(SETTINGS_ENUM::SETTING_LAST); ++n)
			{
				SetPTreeEntry(messager, static_cast<SETTINGS_ENUM>(n), pt);
			}

		}

		void WritePtreeToFile(Messager & messager, boost::property_tree::ptree & pt)
		{

			bool no_file = false;

			if ( !boost::filesystem::exists(_path_to_settings.parent_path()) )
			{
				no_file = true;
			}

			if ( boost::filesystem::is_directory(_path_to_settings) )
			{
				no_file = true;
			}

			//else if ( boost::filesystem::file_size(_path_to_settings) == 0 )
			{
				//no_file = true;
			}

			if (!no_file)
			{
				try
				{
					boost::property_tree::xml_parser::write_xml(_path_to_settings.string(), pt);
				}
				catch (const boost::property_tree::xml_parser_error & e)
				{
					boost::format msg("Settings file %1% cannot be written to disk: %2%");
					msg % _path_to_settings % e.what();
					messager.AppendMessage(new MessagerWarningMessage(MESSAGER_MESSAGE__FILE_INVALID_FORMAT, msg.str()));
				}
			}

		}

	protected:

		SettingsMap _settings_map;

        SettingsRepository(Messager &, boost::filesystem::path const path_to_settings)
			: _path_to_settings(path_to_settings)
		{
		}

		void SetSettingsPath(boost::filesystem::path const path_to_settings)
		{
			_path_to_settings = path_to_settings;
		}

		boost::filesystem::path const GetSettingsPath()
		{
			return _path_to_settings;
		}

		boost::filesystem::path _path_to_settings;

	protected:

		static SETTING_CLASS SettingInfoObject;

};

template<typename SETTINGS_ENUM, typename SETTING_CLASS>
SETTING_CLASS SettingsRepository<SETTINGS_ENUM, SETTING_CLASS>::SettingInfoObject = SETTING_CLASS(*dummy_messager_ptr);

template<typename SETTINGS_REPOSITORY_CLASS>
class SettingsRepositoryFactory
{

	public:

		SETTINGS_REPOSITORY_CLASS * operator()(Messager & messager, boost::filesystem::path const path_to_settings)
		{
			SETTINGS_REPOSITORY_CLASS * new_settings_repository = new SETTINGS_REPOSITORY_CLASS(messager, path_to_settings);
			new_settings_repository->LoadSettingsFromFile(messager);
			return new_settings_repository;
		}

};

#endif
