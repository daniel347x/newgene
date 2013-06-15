#include "uiinputprojectsettings.h"
#include "uiinputprojectsettings_list.h"
#include <QMessageBox>

SettingInfo UIProjectInputSetting::GetSettingInfoFromEnum(Messager & messager_, int const value__)
{

	UIMessager & messager = static_cast<UIMessager &>(messager_);

	INPUT_PROJECT_SETTINGS_UI_NAMESPACE::INPUT_PROJECT_SETTINGS_UI const value_ = static_cast<INPUT_PROJECT_SETTINGS_UI_NAMESPACE::INPUT_PROJECT_SETTINGS_UI const>(value__);

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
				boost::format msg("Settings information is not available for INPUT_PROJECT_SETTINGS_UI_NAMESPACE::INPUT_PROJECT_SETTINGS_UI value %1%.  Using empty setting.");
				msg % value_;
				messager.AppendMessage(new MessagerWarningMessage(MESSAGER_MESSAGE__INVALID_SETTING_ENUM_VALUE, msg.str()));
			}

	}

	return SettingInfo();

}

void UIInputProjectSettings::SetMapEntry(Messager & messager_, SettingInfo & setting_info, boost::property_tree::ptree & /* pt */ )
{

	UIMessager & messager = static_cast<UIMessager &>(messager_);

	switch (setting_info.setting_class)
	{

//		case SettingInfo::SETTING_CLASS_UI_GLOBAL_SETTING__MRU_LIST:
//			{
//				std::string string_setting = pt.get<std::string>(setting_info.text, setting_info.default_val_string);
//				_settings_map[static_cast<GLOBAL_SETTINGS_UI_NAMESPACE::GLOBAL_SETTINGS_UI>(setting_info.enum_index)] = std::unique_ptr<UIGlobalSetting>(SettingFactory<UIGlobalSetting_MRUList, true>()(messager, string_setting));
//			}
//			break;

		default:
			{
				boost::format msg("Unknown UI input project setting \"%1%\" (\"%2%\") being loaded.");
				msg % setting_info.text % setting_info.setting_class;
				messager.AppendMessage(new MessagerWarningMessage(MESSAGER_MESSAGE__INVALID_SETTING_INFO_OBJECT, msg.str()));
			}
			break;

	}

}

UIProjectInputSetting * UIInputProjectSettings::CloneSetting(Messager & messager_, UIProjectInputSetting * /* current_setting */, SettingInfo & setting_info) const
{

	UIMessager & messager = static_cast<UIMessager &>(messager_);

	try
	{

		switch (setting_info.setting_class)
		{

//			case SettingInfo::SETTING_CLASS_UI_GLOBAL_SETTING__MRU_LIST:
//				{
//					UIGlobalSetting_MRUList * setting = dynamic_cast<UIGlobalSetting_MRUList*>(current_setting);
//					return new UIGlobalSetting_MRUList(messager, setting->getString());
//				}
//				break;

			default:
				{
					boost::format msg("Unknown UI input project setting \"%1%\" (\"%2%\") being requested.");
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

UIProjectInputSetting * UIInputProjectSettings::NewSetting(Messager & messager_, SettingInfo & setting_info, void const * /* setting_value_void */ )
{

	UIMessager & messager = static_cast<UIMessager &>(messager_);

	switch (setting_info.setting_class)
	{

		//case SettingInfo::SETTING_CLASS_BACKEND_GLOBAL_SETTING__TEST:
		//	{
		//		std::string string_setting = setting_info.default_val_string;
		//		if (setting_value_void)
		//		{
		//			string_setting = *((std::string *)(setting_value_void));
		//		}
		//		return new GlobalSetting_Test(messager, string_setting);
		//	}
		//	break;

		default:
			{
				boost::format msg("Unknown UI input project setting \"%1%\" (\"%2%\") being updated.");
				msg % setting_info.text % setting_info.setting_class;
				messager.AppendMessage(new MessagerWarningMessage(MESSAGER_MESSAGE__INVALID_SETTING_INFO_OBJECT, msg.str()));
			}
			break;

	}

	return NULL;

}

void UIInputProjectSettings::SetPTreeEntry(Messager & messager, INPUT_PROJECT_SETTINGS_UI_NAMESPACE::INPUT_PROJECT_SETTINGS_UI which_setting, boost::property_tree::ptree & pt)
{

}

void UIInputProjectSettings::SignalMessageBox(QString msg)
{
	QMessageBox msgBox;
	msgBox.setText( msg );
	msgBox.exec();
}
