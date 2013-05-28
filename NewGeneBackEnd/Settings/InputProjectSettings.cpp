#include "InputProjectSettings.h"

SettingInfo BackendProjectInputSetting::GetSettingInfoFromEnum(Messager & messager, int const value__)
{

	INPUT_PROJECT_SETTINGS_BACKEND_NAMESPACE::INPUT_PROJECT_SETTINGS_BACKEND const value_ = static_cast<INPUT_PROJECT_SETTINGS_BACKEND_NAMESPACE::INPUT_PROJECT_SETTINGS_BACKEND const>(value__);

	switch (value_)
	{

		//		case GLOBAL_SETTINGS_UI_NAMESPACE::MRU_LIST:
		//			{
		//				return SettingInfo(SettingInfo::SETTING_CLASS_UI_GLOBAL_SETTING__MRU_LIST,
		//								   static_cast<int>(GLOBAL_SETTINGS_UI_NAMESPACE::MRU_LIST),
		//								   "MRU_LIST",
		//								   "");
		//			}
		//			break;

	default:
		{
			boost::format msg("Settings information is not available for INPUT_PROJECT_SETTINGS_BACKEND_NAMESPACE::INPUT_PROJECT_SETTINGS_BACKEND value %1%.  Using empty setting.");
			msg % value_;
			messager.AppendMessage(new MessagerWarningMessage(MESSAGER_MESSAGE__INVALID_SETTING_ENUM_VALUE, msg.str()));
		}

	}

	return SettingInfo();

}

void InputProjectSettings::SetMapEntry(Messager & messager, SettingInfo & setting_info, boost::property_tree::ptree & pt)
{

	switch (setting_info.setting_class)
	{

		//		case SettingInfo::SETTING_CLASS_UI_GLOBAL_SETTING__MRU_LIST:
		//			{
		//				std::string string_setting = pt.get<std::string>(setting_info.text, setting_info.default_val_string);
		//				_settings_map[static_cast<GLOBAL_SETTINGS_UI_NAMESPACE::GLOBAL_SETTINGS_UI>(setting_info.enum_index)] = std::unique_ptr<UIGlobalSetting>(SettingFactory<UIGlobalSetting_MRUList, false>()(messager, string_setting));
		//			}
		//			break;

	default:
		{
			boost::format msg("Unknown backend input project setting \"%1%\" (\"%2%\") being loaded.");
			msg % setting_info.text % setting_info.setting_class;
			messager.AppendMessage(new MessagerWarningMessage(MESSAGER_MESSAGE__INVALID_SETTING_INFO_OBJECT, msg.str()));
		}
		break;

	}

}

BackendProjectInputSetting * InputProjectSettings::CloneSetting(Messager & messager, BackendProjectInputSetting * current_setting, SettingInfo & setting_info) const
{

	try
	{

		switch (setting_info.setting_class)
		{

			//case SettingInfo::SETTING_CLASS_UI_GLOBAL_SETTING__MRU_LIST:
			//	{
			//		UIGlobalSetting_MRUList * setting = dynamic_cast<UIGlobalSetting_MRUList*>(current_setting);
			//		return new UIGlobalSetting_MRUList(messager, setting->getString());
			//	}
			//	break;

		default:
			{
				boost::format msg("Unknown backend input project setting \"%1%\" (\"%2%\") being requested.");
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

BackendProjectInputSetting * InputProjectSettings::NewSetting(Messager & messager, SettingInfo & setting_info, void const * setting_value_void)
{

	switch (setting_info.setting_class)
	{

		//		case SettingInfo::SETTING_CLASS_UI_GLOBAL_SETTING__MRU_LIST:
		//			{
		//				std::string string_setting = *((std::string *)(setting_value_void));
		//				return new UIGlobalSetting_MRUList(messager, string_setting);
		//			}
		//			break;

	default:
		{
			boost::format msg("Unknown backend input project setting \"%1%\" (\"%2%\") being updated.");
			msg % setting_info.text % setting_info.setting_class;
			messager.AppendMessage(new MessagerWarningMessage(MESSAGER_MESSAGE__INVALID_SETTING_INFO_OBJECT, msg.str()));
		}
		break;

	}

	return NULL;

}
