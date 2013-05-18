#include "uiallglobalsettings.h"

class UIGlobalSetting_MRUList : public UIGlobalSetting, public StringSetting
{

	public:

		UIGlobalSetting_MRUList(Messager & messager, std::string const & setting)
			: UIGlobalSetting()
			, StringSetting(messager, setting)
		{}

		virtual void DoSpecialParse(Messager & messager)
		{
//			boost::format msg("Here is a message!");
//			messager.AppendMessage(new UIMessagerErrorMessage(MESSAGER_MESSAGE__GENERAL_ERROR, msg.str()));
		}

};

template<>
SettingInfo GetSettingTextFromEnum<GLOBAL_SETTINGS_UI_NAMESPACE::GLOBAL_SETTINGS_UI>(GLOBAL_SETTINGS_UI_NAMESPACE::GLOBAL_SETTINGS_UI const value_)
{
	switch (value_)
	{
		case GLOBAL_SETTINGS_UI_NAMESPACE::MRU_LIST: return SettingInfo(SettingInfo::SETTING_TYPE_STRING, "MRU_LIST", "");
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

void UIAllGlobalSettings::UIOnlySettings::LoadDefaultSettings(Messager & messager)
{
	_settings_map[GLOBAL_SETTINGS_UI_NAMESPACE::MRU_LIST] = std::unique_ptr<UIGlobalSetting>(SettingFactory<UIGlobalSetting_MRUList>()(messager, ""));
}
