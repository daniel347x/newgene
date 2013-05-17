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

		Settings()
		{

		}

		Settings(boost::filesystem::path const settings_path)
		{

		}

		virtual ~Settings()
		{

		}

};

#endif
