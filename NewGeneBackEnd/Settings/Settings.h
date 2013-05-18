#ifndef SETTINGS_H
#define SETTINGS_H

#ifndef Q_MOC_RUN
#	include <boost/filesystem.hpp>
#endif

#include "SettingsRepository.h"

template<typename SETTINGS_ENUM, typename SETTING_CLASS>
class Settings : public SettingsRepository<SETTINGS_ENUM, SETTING_CLASS>
{

	public:

		Settings(Messager & messager)
			: SettingsRepository(messager)
		{

		}

		Settings(Messager & messager, boost::filesystem::path const path_to_settings)
			: SettingsRepository(messager, path_to_settings)
		{

		}

		virtual ~Settings()
		{

		}

};

class SettingInfo
{

	public:

		enum SETTING_TYPE
		{
			  SETTING_TYPE_NONE
			, SETTING_TYPE_STRING
			, SETTING_TYPE_INT32
		};

		SettingInfo()
			: type(SETTING_TYPE_NONE)
			, text("")
			, default_val_string("")
			, default_val_int32(0)
		{

		}

		SettingInfo(SETTING_TYPE const type_, std::string const & text_, std::string default_val_string_)
			: type(type_)
			, text(text_)
			, default_val_string(default_val_string_)
			, default_val_int32(0)
		{

		}

		SettingInfo(SETTING_TYPE const type_, std::string const & text_, std::int32_t default_val_int32_)
			: type(type_)
			, text(text_)
			, default_val_string("")
			, default_val_int32(default_val_int32_)
		{

		}

		SETTING_TYPE type;
		std::string text;
		std::string default_val_string;
		std::int32_t default_val_int32;

};

template<typename SETTINGS_ENUM>
SettingInfo GetSettingTextFromEnum(SETTINGS_ENUM const value_)
{
	return "";
}

#endif
