#include "GlobalSettings.h"
#include "GlobalSettings_list.h"

SettingInfo BackendGlobalSetting::GetSettingInfoFromEnum(Messager & messager, int const value__)
{

	GLOBAL_SETTINGS_BACKEND_NAMESPACE::GLOBAL_SETTINGS_BACKEND const value_ = static_cast<GLOBAL_SETTINGS_BACKEND_NAMESPACE::GLOBAL_SETTINGS_BACKEND const>(value__);

	switch (value_)
	{

	case GLOBAL_SETTINGS_BACKEND_NAMESPACE::TEST_SETTING:
		{
			return SettingInfo(SettingInfo::SETTING_CLASS_BACKEND_GLOBAL_SETTING__TEST,
				static_cast<int>(GLOBAL_SETTINGS_BACKEND_NAMESPACE::TEST_SETTING),
				"GLOBAL_BACKEND_TEST",
				"default test string!  And it works");
		}
		break;

	default:
		{
			boost::format msg("Settings information is not available for GLOBAL_SETTINGS_BACKEND_NAMESPACE::GLOBAL_SETTINGS_BACKEND value %1%.  Using empty setting.");
			msg % value_;
			messager.AppendMessage(new MessagerWarningMessage(MESSAGER_MESSAGE__INVALID_SETTING_ENUM_VALUE, msg.str()));
		}

	}

	return SettingInfo();

}

void GlobalSettings::SetMapEntry(Messager & messager, SettingInfo & setting_info, boost::property_tree::ptree & pt)
{

	switch (setting_info.setting_class)
	{

	case SettingInfo::SETTING_CLASS_BACKEND_GLOBAL_SETTING__TEST:
		{
			std::string string_setting = pt.get<std::string>(setting_info.text, setting_info.default_val_string);
			_settings_map[static_cast<GLOBAL_SETTINGS_BACKEND_NAMESPACE::GLOBAL_SETTINGS_BACKEND>(setting_info.enum_index)] = std::unique_ptr<BackendGlobalSetting>(SettingFactory<GlobalSetting_Test>()(messager, string_setting));
		}
		break;

	default:
		{
			boost::format msg("Unknown UI global setting \"%1%\" (\"%2%\") being loaded.");
			msg % setting_info.text % setting_info.setting_class;
			messager.AppendMessage(new MessagerWarningMessage(MESSAGER_MESSAGE__INVALID_SETTING_INFO_OBJECT, msg.str()));
		}
		break;

	}

}

BackendGlobalSetting * GlobalSettings::CloneSetting(Messager & messager, BackendGlobalSetting * current_setting, SettingInfo & setting_info) const
{

	try
	{

		switch (setting_info.setting_class)
		{

		case SettingInfo::SETTING_CLASS_BACKEND_GLOBAL_SETTING__TEST:
			{
				GlobalSetting_Test * setting = dynamic_cast<GlobalSetting_Test*>(current_setting);
				return new GlobalSetting_Test(messager, setting->getString());
			}
			break;

		default:
			{
				boost::format msg("Unknown backend global setting \"%1%\" (\"%2%\") being requested.");
				msg % setting_info.text % setting_info.setting_class;
				messager.AppendMessage(new MessagerWarningMessage(MESSAGER_MESSAGE__INVALID_SETTING_INFO_OBJECT, msg.str()));
			}
			break;

		}

	}
	catch (std::bad_cast & e)
	{
		boost::format msg("Unable to retrieve setting \"%1%\" (\"%2%\"): %3%.");
		msg % setting_info.text % setting_info.setting_class % e.what();
		messager.AppendMessage(new MessagerWarningMessage(MESSAGER_MESSAGE__INVALID_SETTING_INFO_OBJECT, msg.str()));
	}

	return NULL;

}

BackendGlobalSetting * GlobalSettings::NewSetting(Messager & messager, SettingInfo & setting_info, void const * setting_value_void)
{

	switch (setting_info.setting_class)
	{

	case SettingInfo::SETTING_CLASS_BACKEND_GLOBAL_SETTING__TEST:
		{
			std::string string_setting = setting_info.default_val_string;
			if (setting_value_void)
			{
				string_setting = *((std::string *)(setting_value_void));
			}
			return new GlobalSetting_Test(messager, string_setting);
		}
		break;

	default:
		{
			boost::format msg("Unknown backend global setting \"%1%\" (\"%2%\") being updated.");
			msg % setting_info.text % setting_info.setting_class;
			messager.AppendMessage(new MessagerWarningMessage(MESSAGER_MESSAGE__INVALID_SETTING_INFO_OBJECT, msg.str()));
		}
		break;

	}

	return NULL;

}

GlobalSettings::GlobalSettings(Messager & messager, boost::filesystem::path const global_settings_path)
	: Settings(messager, global_settings_path)
{

}
