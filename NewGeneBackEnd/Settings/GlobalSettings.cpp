#include "GlobalSettings.h"

void GlobalSettings::SetMapEntry(Messager & messager, SettingInfo & setting_info, boost::property_tree::ptree & pt)
{

	switch (setting_info.setting_class)
	{

		//		case SettingInfo::SETTING_CLASS_UI_GLOBAL_SETTING_MRU_LIST:
		//			{
		//				std::string string_setting = pt.get<std::string>(setting_info.text, setting_info.default_val_string);
		//				_settings_map[static_cast<GLOBAL_SETTINGS_UI_NAMESPACE::GLOBAL_SETTINGS_UI>(setting_info.enum_index)] = std::unique_ptr<UIGlobalSetting>(SettingFactory<UIGlobalSetting_MRUList>()(messager, string_setting));
		//			}
		//			break;

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

		//case SettingInfo::SETTING_CLASS_UI_GLOBAL_SETTING_MRU_LIST:
		//	{
		//		UIGlobalSetting_MRUList * setting = dynamic_cast<UIGlobalSetting_MRUList*>(current_setting);
		//		return new UIGlobalSetting_MRUList(messager, setting->getString());
		//	}
		//	break;

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

		//		case SettingInfo::SETTING_CLASS_UI_GLOBAL_SETTING_MRU_LIST:
		//			{
		//				std::string string_setting = *((std::string *)(setting_value_void));
		//				return new UIGlobalSetting_MRUList(messager, string_setting);
		//			}
		//			break;

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

boost::filesystem::path GlobalSettings::GetSettingsPath(Messager & messager, SettingInfo & setting_info)
{
	return boost::filesystem::path();
}

template<>
SettingInfo GetSettingInfoFromEnum<GLOBAL_SETTINGS_BACKEND_NAMESPACE::GLOBAL_SETTINGS_BACKEND>(Messager & messager, GLOBAL_SETTINGS_BACKEND_NAMESPACE::GLOBAL_SETTINGS_BACKEND const value_)
{

	switch (value_)
	{

		//		case GLOBAL_SETTINGS_UI_NAMESPACE::MRU_LIST:
		//			{
		//				return SettingInfo(SettingInfo::SETTING_CLASS_UI_GLOBAL_SETTING_MRU_LIST,
		//								   static_cast<int>(GLOBAL_SETTINGS_UI_NAMESPACE::MRU_LIST),
		//								   "MRU_LIST",
		//								   "");
		//			}
		//			break;

	default:
		{
			boost::format msg("Settings information is not available for GLOBAL_SETTINGS_BACKEND_NAMESPACE::GLOBAL_SETTINGS_BACKEND value %1%.  Using empty setting.");
			msg % value_;
			messager.AppendMessage(new MessagerWarningMessage(MESSAGER_MESSAGE__FILE_INVALID_SETTING_ENUM_VALUE, msg.str()));
		}

	}

	return SettingInfo();

}

GlobalSettings::GlobalSettings(Messager & messager)
	: Settings(messager)
{

}

GlobalSettings::GlobalSettings(Messager & messager, boost::filesystem::path const global_settings_path)
	: Settings(messager, global_settings_path)
{

}
