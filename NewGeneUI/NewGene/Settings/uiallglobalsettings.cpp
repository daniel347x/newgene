#include "uiallglobalsettings.h"
#include "uiallglobalsettings_list.h"
#include "globals.h"

SettingInfo UIGlobalSetting::GetSettingInfoFromEnum(Messager & messager_, int const value__)
{

	UIMessager & messager = static_cast<UIMessager &>(messager_);

	GLOBAL_SETTINGS_UI_NAMESPACE::GLOBAL_SETTINGS_UI const value_ = static_cast<GLOBAL_SETTINGS_UI_NAMESPACE::GLOBAL_SETTINGS_UI const>(value__);

	switch (value_)
	{

		case GLOBAL_SETTINGS_UI_NAMESPACE::MRU_INPUT_LIST:
			{
				return SettingInfo(SettingInfo::SETTING_CLASS_UI_GLOBAL_SETTING__MRU_INPUT_LIST,
								   static_cast<int>(GLOBAL_SETTINGS_UI_NAMESPACE::MRU_INPUT_LIST),
								   "MRU_INPUT_LIST",
								   "");
			}
			break;

		case GLOBAL_SETTINGS_UI_NAMESPACE::MRU_OUTPUT_LIST:
			{
				return SettingInfo(SettingInfo::SETTING_CLASS_UI_GLOBAL_SETTING__MRU_OUTPUT_LIST,
								   static_cast<int>(GLOBAL_SETTINGS_UI_NAMESPACE::MRU_OUTPUT_LIST),
								   "MRU_OUTPUT_LIST",
								   "");
			}
			break;

		default:
			{
				boost::format msg("Settings information is not available for GLOBAL_SETTINGS_UI_NAMESPACE::GLOBAL_SETTINGS_UI value %1%.  Using empty setting.");
				msg % value_;
				messager.AppendMessage(new MessagerWarningMessage(MESSAGER_MESSAGE__INVALID_SETTING_ENUM_VALUE, msg.str()));
			}

	}

	return SettingInfo();

}

void UIAllGlobalSettings::UIOnlySettings::SetMapEntry(Messager & messager_, SettingInfo & setting_info, boost::property_tree::ptree & pt)
{

	UIMessager & messager = static_cast<UIMessager &>(messager_);

	switch (setting_info.setting_class)
	{

		case SettingInfo::SETTING_CLASS_UI_GLOBAL_SETTING__MRU_INPUT_LIST:
			{
				std::string string_setting = pt.get<std::string>(setting_info.text, setting_info.default_val_string);
				_settings_map[static_cast<GLOBAL_SETTINGS_UI_NAMESPACE::GLOBAL_SETTINGS_UI>(setting_info.enum_index)] = std::unique_ptr<UIGlobalSetting>(SettingFactory<UIGlobalSetting_MRU_Input_List>()(messager, string_setting));
			}
			break;

		case SettingInfo::SETTING_CLASS_UI_GLOBAL_SETTING__MRU_OUTPUT_LIST:
			{
				std::string string_setting = pt.get<std::string>(setting_info.text, setting_info.default_val_string);
				_settings_map[static_cast<GLOBAL_SETTINGS_UI_NAMESPACE::GLOBAL_SETTINGS_UI>(setting_info.enum_index)] = std::unique_ptr<UIGlobalSetting>(SettingFactory<UIGlobalSetting_MRU_Output_List>()(messager, string_setting));
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

UIGlobalSetting * UIAllGlobalSettings::UIOnlySettings::CloneSetting(Messager & messager_, UIGlobalSetting * current_setting, SettingInfo & setting_info) const
{

	UIMessager & messager = static_cast<UIMessager &>(messager_);

	try
	{

		switch (setting_info.setting_class)
		{

			case SettingInfo::SETTING_CLASS_UI_GLOBAL_SETTING__MRU_INPUT_LIST:
				{
					UIGlobalSetting_MRU_Input_List * setting = dynamic_cast<UIGlobalSetting_MRU_Input_List*>(current_setting);
					return new UIGlobalSetting_MRU_Input_List(messager, setting->getString());
				}
				break;

			case SettingInfo::SETTING_CLASS_UI_GLOBAL_SETTING__MRU_OUTPUT_LIST:
				{
					UIGlobalSetting_MRU_Output_List * setting = dynamic_cast<UIGlobalSetting_MRU_Output_List*>(current_setting);
					return new UIGlobalSetting_MRU_Output_List(messager, setting->getString());
				}
				break;

			default:
				{
					boost::format msg("Unknown UI global setting \"%1%\" (\"%2%\") being requested.");
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

UIGlobalSetting * UIAllGlobalSettings::UIOnlySettings::NewSetting(Messager & messager_, SettingInfo & setting_info, void const * setting_value_void)
{

	UIMessager & messager = static_cast<UIMessager &>(messager_);

	switch (setting_info.setting_class)
	{

		case SettingInfo::SETTING_CLASS_UI_GLOBAL_SETTING__MRU_INPUT_LIST:
			{
				std::string string_setting = setting_info.default_val_string;
				if (setting_value_void)
				{
					string_setting = *((std::string *)(setting_value_void));
				}
				return new UIGlobalSetting_MRU_Input_List(messager, string_setting);
			}
			break;

		case SettingInfo::SETTING_CLASS_UI_GLOBAL_SETTING__MRU_OUTPUT_LIST:
			{
				std::string string_setting = setting_info.default_val_string;
				if (setting_value_void)
				{
					string_setting = *((std::string *)(setting_value_void));
				}
				return new UIGlobalSetting_MRU_Output_List(messager, string_setting);
			}
			break;

		default:
			{
				boost::format msg("Unknown UI global setting \"%1%\" (\"%2%\") being updated.");
				msg % setting_info.text % setting_info.setting_class;
				messager.AppendMessage(new MessagerWarningMessage(MESSAGER_MESSAGE__INVALID_SETTING_INFO_OBJECT, msg.str()));
			}
			break;

	}

	return NULL;

}

boost::filesystem::path UIAllGlobalSettings::UIOnlySettings::GetSettingsPath(Messager &, SettingInfo & /* setting_info */ )
{
	settingsManagerUI().ObtainGlobalSettingsPath();
	return settingsManagerUI().getGlobalSettingsPath();
}

UIAllGlobalSettings::UIOnlySettings & UIAllGlobalSettings::getUISettings()
{
	if (!__impl)
	{
		boost::format msg( "Internal settings implementation not yet constructed." );
		throw NewGeneException() << newgene_error_description( msg.str() );
	}
	return static_cast<UIAllGlobalSettings::UIOnlySettings &>(getUISettings_base<GlobalSettings, UIOnlySettings, GLOBAL_SETTINGS_UI_NAMESPACE::GLOBAL_SETTINGS_UI, UIGlobalSetting>(*__impl));
}

GlobalSettings & UIAllGlobalSettings::getBackendSettings()
{
	if (!__impl)
	{
		boost::format msg( "Internal settings implementation not yet constructed." );
		throw NewGeneException() << newgene_error_description( msg.str() );
	}
	return static_cast<GlobalSettings &>(getBackendSettings_base<GlobalSettings, UIOnlySettings, GLOBAL_SETTINGS_BACKEND_NAMESPACE::GLOBAL_SETTINGS_BACKEND, BackendGlobalSetting>(*__impl));
}

UIAllGlobalSettings::UIAllGlobalSettings(UIMessager & messager, boost::filesystem::path const path_to_settings, QObject * parent) :
	UIAllSettings(messager, parent)
{
	CreateImplementation(messager, path_to_settings);
}

void UIAllGlobalSettings::_impl::CreateInternalImplementations(UIMessager & messager, boost::filesystem::path const path_to_settings)
{
	__ui_impl.reset(new _UIRelatedImpl(messager, path_to_settings));
	__backend_impl.reset(new _BackendRelatedImpl(messager, path_to_settings));
}

void UIAllGlobalSettings::CreateImplementation(UIMessager & messager, boost::filesystem::path const path_to_settings)
{
	__impl.reset(new _impl(messager, path_to_settings));
}
