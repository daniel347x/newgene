#ifndef GLOBALSETTINGSLIST_H
#define GLOBALSETTINGSLIST_H

#include "SettingsManager.h"

class GlobalSetting_Test : public BackendGlobalSetting, public StringSetting, public SimpleAccessSetting<GlobalSetting_Test, GLOBAL_SETTINGS_BACKEND_NAMESPACE::GLOBAL_SETTINGS_BACKEND, GLOBAL_SETTINGS_BACKEND_NAMESPACE::TEST_SETTING, SettingsManager>
{

public:

	GlobalSetting_Test(Messager & messager, std::string const & setting)
		: Setting(messager)
		, BackendSetting(messager)
		, GlobalSetting(messager)
		, BackendGlobalSetting(messager)
		, StringSetting(messager, setting)
	{}

	virtual void DoSpecialParse(Messager &)
	{
		//boost::format msg("Here is a message from TEST!");
		//messager.AppendMessage(new MessagerErrorMessage(MESSAGER_MESSAGE__GENERAL_ERROR, msg.str()));
	}

	std::string ToString() { return ""; }

};


/************************************************************************************************************************/
//
/************************************************************************************************************************/
template<>
class SettingClassTypeTraits<SettingInfo::SETTING_CLASS_BACKEND_GLOBAL_SETTING__TEST>
{
public:
	typedef GlobalSetting_Test type;
};

#endif
