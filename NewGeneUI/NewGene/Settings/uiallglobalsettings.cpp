#include "uiallglobalsettings.h"
#include "globals.h"

void UIAllGlobalSettings::UIOnlySettings::SetMapEntry(Messager & messager, SettingInfo & setting_info, boost::property_tree::ptree & pt)
{

	switch (setting_info.setting_class)
	{

		case SettingInfo::SETTING_CLASS_UI_GLOBAL_SETTING_MRU_LIST:
			{
				std::string string_setting = pt.get<std::string>(setting_info.text, setting_info.default_val_string);
				_settings_map[static_cast<GLOBAL_SETTINGS_UI_NAMESPACE::GLOBAL_SETTINGS_UI>(setting_info.enum_index)] = std::unique_ptr<UIGlobalSetting>(SettingFactory<UIGlobalSetting_MRUList>()(messager, string_setting));
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

UIGlobalSetting * UIAllGlobalSettings::UIOnlySettings::CloneSetting(Messager & messager, UIGlobalSetting * current_setting, SettingInfo & setting_info) const
{

	try
	{

		switch (setting_info.setting_class)
		{

			case SettingInfo::SETTING_CLASS_UI_GLOBAL_SETTING_MRU_LIST:
				{
					UIGlobalSetting_MRUList * setting = dynamic_cast<UIGlobalSetting_MRUList*>(current_setting);
					return new UIGlobalSetting_MRUList(messager, setting->getString());
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

UIGlobalSetting * UIAllGlobalSettings::UIOnlySettings::NewSetting(Messager & messager, SettingInfo & setting_info, void const * setting_value_void)
{

	switch (setting_info.setting_class)
	{

		case SettingInfo::SETTING_CLASS_UI_GLOBAL_SETTING_MRU_LIST:
			{
				std::string string_setting = *((std::string *)(setting_value_void));
				return new UIGlobalSetting_MRUList(messager, string_setting);
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

template<>
SettingInfo GetSettingInfoFromEnum<GLOBAL_SETTINGS_UI_NAMESPACE::GLOBAL_SETTINGS_UI>(Messager & messager, GLOBAL_SETTINGS_UI_NAMESPACE::GLOBAL_SETTINGS_UI const value_)
{

	switch (value_)
	{

		case GLOBAL_SETTINGS_UI_NAMESPACE::MRU_LIST:
			{
				return SettingInfo(SettingInfo::SETTING_CLASS_UI_GLOBAL_SETTING_MRU_LIST,
								   static_cast<int>(GLOBAL_SETTINGS_UI_NAMESPACE::MRU_LIST),
								   "MRU_LIST",
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

UIAllGlobalSettings::UIOnlySettings & UIAllGlobalSettings::getUISettings()
{
	if (!__impl)
	{
		boost::format msg( "Internal settings implementation not yet constructed." );
		throw NewGeneException() << newgene_error_description( msg.str() );
	}
	return reinterpret_cast<UIAllGlobalSettings::UIOnlySettings &>(getUISettings_base<GlobalSettings, UIOnlySettings, GLOBAL_SETTINGS_UI_NAMESPACE::GLOBAL_SETTINGS_UI, UIOnlySettings>(*(__impl.get())));
}

GlobalSettings & UIAllGlobalSettings::getBackendSettings()
{
	if (!__impl)
	{
		boost::format msg( "Internal settings implementation not yet constructed." );
		throw NewGeneException() << newgene_error_description( msg.str() );
	}
	return reinterpret_cast<GlobalSettings &>(getBackendSettings_base<GlobalSettings, UIOnlySettings, GLOBAL_SETTINGS_UI_NAMESPACE::GLOBAL_SETTINGS_UI, ProjectSettings>(*(__impl.get())));
}

UIAllGlobalSettings::UIAllGlobalSettings(Messager & messager, QObject * parent)
	: UIAllSettings(messager, parent)
{
	CreateImplementation(messager);
}

UIAllGlobalSettings::UIAllGlobalSettings(Messager & messager, boost::filesystem::path const path_to_settings, QObject * parent) :
	UIAllSettings(messager, parent)
{
	CreateImplementation(messager, path_to_settings);
}

void UIAllGlobalSettings::CreateImplementation(Messager & messager)
{
	__impl.reset(new _impl(messager));
}

void UIAllGlobalSettings::CreateImplementation(Messager & messager, boost::filesystem::path const path_to_settings)
{
	__impl.reset(new _impl(messager, path_to_settings));
}

void UIAllGlobalSettings::_impl::CreateInternalImplementations(Messager & messager)
{
	__ui_impl.reset(new _UIRelatedImpl(messager));
	__backend_impl.reset(new _BackendRelatedImpl(messager));
}

void UIAllGlobalSettings::_impl::CreateInternalImplementations(Messager & messager, boost::filesystem::path const path_to_settings)
{
	__ui_impl.reset(new _UIRelatedImpl(messager, path_to_settings));
	__backend_impl.reset(new _BackendRelatedImpl(messager, path_to_settings));
}
