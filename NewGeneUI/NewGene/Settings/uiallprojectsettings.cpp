#include "uiallprojectsettings.h"

void UIAllProjectSettings::UIOnlySettings::SetMapEntry(Messager & messager, SettingInfo & setting_info, boost::property_tree::ptree & /* pt */ )
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

UIProjectSetting * UIAllProjectSettings::UIOnlySettings::CloneSetting(Messager & messager, UIProjectSetting * /* current_setting */, SettingInfo & setting_info) const
{

	try
	{

		switch (setting_info.setting_class)
		{

//			case SettingInfo::SETTING_CLASS_UI_GLOBAL_SETTING_MRU_LIST:
//				{
//					UIGlobalSetting_MRUList * setting = dynamic_cast<UIGlobalSetting_MRUList*>(current_setting);
//					return new UIGlobalSetting_MRUList(messager, setting->getString());
//				}
//				break;

			default:
				{
					boost::format msg("Unknown UI project setting \"%1%\" (\"%2%\") being requested.");
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

UIProjectSetting * UIAllProjectSettings::UIOnlySettings::NewSetting(Messager & messager, SettingInfo & setting_info, void const * /* setting_value_void */ )
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
				boost::format msg("Unknown UI project setting \"%1%\" (\"%2%\") being updated.");
				msg % setting_info.text % setting_info.setting_class;
				messager.AppendMessage(new MessagerWarningMessage(MESSAGER_MESSAGE__INVALID_SETTING_INFO_OBJECT, msg.str()));
			}
			break;

	}

	return NULL;

}

boost::filesystem::path UIAllProjectSettings::UIOnlySettings::GetSettingsPath(Messager &, SettingInfo & /* setting_info */ )
{
	return boost::filesystem::path();
}

template<>
SettingInfo GetSettingInfoFromEnum<PROJECT_SETTINGS_UI_NAMESPACE::PROJECT_SETTINGS_UI>(Messager & messager, PROJECT_SETTINGS_UI_NAMESPACE::PROJECT_SETTINGS_UI const value_)
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
				boost::format msg("Settings information is not available for PROJECT_SETTINGS_UI_NAMESPACE::PROJECT_SETTINGS_UI value %1%.  Using empty setting.");
				msg % value_;
				messager.AppendMessage(new MessagerWarningMessage(MESSAGER_MESSAGE__INVALID_SETTING_ENUM_VALUE, msg.str()));
			}

	}

	return SettingInfo();

}

UIAllProjectSettings::UIOnlySettings & UIAllProjectSettings::getUISettings()
{
	if (!__impl)
	{
		boost::format msg( "Internal settings implementation not yet constructed." );
		throw NewGeneException() << newgene_error_description( msg.str() );
	}
	return reinterpret_cast<UIAllProjectSettings::UIOnlySettings &>(getUISettings_base<ProjectSettings, UIOnlySettings, PROJECT_SETTINGS_UI_NAMESPACE::PROJECT_SETTINGS_UI, UIOnlySettings>(*(__impl.get())));
}

ProjectSettings & UIAllProjectSettings::getBackendSettings()
{
	if (!__impl)
	{
		boost::format msg( "Internal settings implementation not yet constructed." );
		throw NewGeneException() << newgene_error_description( msg.str() );
	}
	return reinterpret_cast<ProjectSettings &>(getBackendSettings_base<ProjectSettings, UIOnlySettings, PROJECT_SETTINGS_UI_NAMESPACE::PROJECT_SETTINGS_UI, ProjectSettings>(*(__impl.get())));
}

UIAllProjectSettings::UIAllProjectSettings(Messager & messager, Project & project, boost::filesystem::path const path_to_settings, QObject * parent) :
	UIAllSettings(messager, parent)
{
	CreateImplementation(messager, project, path_to_settings);
}

void UIAllProjectSettings::_impl::CreateInternalImplementations(Messager & messager, Project & project, boost::filesystem::path const path_to_settings)
{
	__ui_impl.reset(new _UIRelatedImpl(messager, project, path_to_settings));
	__backend_impl.reset(new _BackendRelatedImpl(messager, project, path_to_settings));
}

void UIAllProjectSettings::_impl::CreateInternalImplementations(Messager & messager, boost::filesystem::path const path_to_settings)
{
	boost::format msg("UIAllProjectSettings::_impl::CreateInternalImplementations() non-Project overload being called.");
	messager.AppendMessage(new MessagerCatastrophicErrorMessage(MESSAGER_MESSAGE__PROJECT_NOT_AVAILABLE, msg.str()));
}

void UIAllProjectSettings::CreateImplementation(Messager & messager, Project & project, boost::filesystem::path const path_to_settings)
{
	__impl.reset(new _impl(messager, project, path_to_settings));
}

void UIAllProjectSettings::CreateImplementation(Messager & messager, boost::filesystem::path const path_to_settings)
{
	boost::format msg("UIAllProjectSettings::CreateImplementation() non-Project overload being called.");
	messager.AppendMessage(new MessagerCatastrophicErrorMessage(MESSAGER_MESSAGE__PROJECT_NOT_AVAILABLE, msg.str()));
}
