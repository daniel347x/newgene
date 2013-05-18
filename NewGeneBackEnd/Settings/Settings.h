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

// Only specializations of this are used
template<typename SETTINGS_ENUM>
SettingInfo GetSettingInfoFromEnum(Messager & messager, SETTINGS_ENUM const value_)
{
	return SettingInfo();
}

#endif
