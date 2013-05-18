#include "uiallglobalsettings.h"

void UIAllGlobalSettings::UIOnlySettings::SetMapEntry(Messager & messager, SettingInfo & setting_info, int const enum_index, boost::property_tree::ptree & pt)
{

	switch (setting_info.setting_class)
	{

		case SettingInfo::SETTING_CLASS_UI_GLOBAL_SETTING_MRU_LIST:
			{
				std::string string_setting = pt.get<std::string>(setting_info.text, setting_info.default_val_string);
				_settings_map[static_cast<GLOBAL_SETTINGS_UI_NAMESPACE::GLOBAL_SETTINGS_UI>(enum_index)] = std::unique_ptr<UIGlobalSetting>(SettingFactory<UIGlobalSetting_MRUList>()(messager, string_setting));
			}
			break;

		default:
			{

			}
			break;

	}

}

UIGlobalSetting * UIAllGlobalSettings::UIOnlySettings::CloneSetting(Messager & messager, UIGlobalSetting * current_setting, SettingInfo & setting_info) const
{
	return NULL;
}

UIGlobalSetting * UIAllGlobalSettings::UIOnlySettings::NewSetting(Messager & messager, SettingInfo & setting_info, void const * setting_value_void)
{
	return NULL;
}

template<>
SettingInfo GetSettingInfoFromEnum<GLOBAL_SETTINGS_UI_NAMESPACE::GLOBAL_SETTINGS_UI>(GLOBAL_SETTINGS_UI_NAMESPACE::GLOBAL_SETTINGS_UI const value_)
{
	switch (value_)
	{
		case GLOBAL_SETTINGS_UI_NAMESPACE::MRU_LIST: return SettingInfo(SettingInfo::SETTING_CLASS_UI_GLOBAL_SETTING_MRU_LIST, "MRU_LIST", "");
	}

	return SettingInfo();
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
