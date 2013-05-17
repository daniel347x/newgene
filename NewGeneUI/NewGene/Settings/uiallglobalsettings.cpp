#include "uiallglobalsettings.h"

UIAllGlobalSettings::UIAllGlobalSettings( QObject * parent ) :
	UIAllSettings( parent )
{
}

UIAllGlobalSettings::UIAllGlobalSettings(boost::filesystem::path const path_to_settings, QObject * parent) :
	UIAllSettings(path_to_settings, parent)
{
}

void UIAllGlobalSettings::ProvideDefaultSettings()
{

}

void UIAllGlobalSettings::_impl::_UIRelatedImpl::CreateDefaultUISettings()
{
}

void UIAllGlobalSettings::_impl::_UIRelatedImpl::CreateUISettings(boost::filesystem::path const path_to_settings)
{
}

void UIAllGlobalSettings::_impl::_BackendRelatedImpl::CreateDefaultBackendSettings()
{
}

void UIAllGlobalSettings::_impl::_BackendRelatedImpl::CreateBackendSettings(boost::filesystem::path const path_to_settings)
{
}

void UIAllGlobalSettings::CreateImplementation()
{
	__impl.reset(new _impl);
}
