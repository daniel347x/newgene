#ifndef SETTINGSPARSER_H
#define SETTINGSPARSER_H

#include <map>

template<typename SETTINGS_ENUM, typename SETTING_CLASS>
class SettingsRepository
{

	protected:

		std::map<SETTINGS_ENUM, SETTING_CLASS> _settings_map;

};

#endif
