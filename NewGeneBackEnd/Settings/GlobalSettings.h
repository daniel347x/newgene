#ifndef GLOBALSETTINGS_H
#define GLOBALSETTINGS_H

#include "Settings.h"
#include "Setting.h"
#include <map>

namespace GLOBAL_SETTINGS_BACKEND_NAMESPACE
{

	enum GLOBAL_SETTINGS_BACKEND
	{
		  SETTING_FIRST = 0

		, TEST_SETTING

		, SETTING_LAST
	};

}

class GlobalSetting_Test : public BackendGlobalSetting, public StringSetting, public SimpleAccessSetting<GlobalSetting_Test, GLOBAL_SETTINGS_BACKEND_NAMESPACE::GLOBAL_SETTINGS_BACKEND, GLOBAL_SETTINGS_BACKEND_NAMESPACE::TEST_SETTING>
{

public:

	GlobalSetting_Test(Messager & messager, std::string const & setting)
		: BackendGlobalSetting()
		, StringSetting(messager, setting)
	{}

	virtual void DoSpecialParse(Messager &)
	{
		//boost::format msg("Here is a message from TEST!");
		//messager.AppendMessage(new MessagerErrorMessage(MESSAGER_MESSAGE__GENERAL_ERROR, msg.str()));
	}

};

class GlobalSettings : public Settings<GLOBAL_SETTINGS_BACKEND_NAMESPACE::GLOBAL_SETTINGS_BACKEND, BackendGlobalSetting>
{

	public:

		GlobalSettings(Messager & messager, boost::filesystem::path const global_settings_path);
		virtual ~GlobalSettings() {}


	protected:

		boost::filesystem::path GetSettingsPath(Messager & messager, SettingInfo & setting_info);
		void SetMapEntry(Messager & messager, SettingInfo & setting_info, boost::property_tree::ptree & pt);
		BackendGlobalSetting * CloneSetting(Messager & messager, BackendGlobalSetting * current_setting, SettingInfo & setting_info) const;
		BackendGlobalSetting * NewSetting(Messager & messager, SettingInfo & setting_info, void const * setting_value_void);

};

#endif
