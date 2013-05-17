#include "uiallprojectsettings.h"

UIAllProjectSettings::UIAllProjectSettings( QObject * parent ) :
	UIAllSettings( parent )
{
}

UIAllProjectSettings::UIAllProjectSettings(boost::filesystem::path const path_to_settings, QObject * parent) :
	UIAllSettings(path_to_settings, parent)
{
}

void UIAllProjectSettings::ProvideDefaultSettings()
{

}

void UIAllProjectSettings::_impl::_UIRelatedImpl::CreateDefaultUISettings()
{
}

void UIAllProjectSettings::_impl::_UIRelatedImpl::CreateUISettings(boost::filesystem::path const path_to_settings)
{
}

void UIAllProjectSettings::_impl::_BackendRelatedImpl::CreateDefaultBackendSettings()
{
}

void UIAllProjectSettings::_impl::_BackendRelatedImpl::CreateBackendSettings(boost::filesystem::path const path_to_settings)
{
}
