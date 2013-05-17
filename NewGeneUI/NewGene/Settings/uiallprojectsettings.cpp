#include "uiallprojectsettings.h"

UIAllProjectSettings::UIAllProjectSettings( QObject * parent ) :
	UIAllSettings( parent )
{
	CreateImplementation();
}

UIAllProjectSettings::UIAllProjectSettings(boost::filesystem::path const path_to_settings, QObject * parent) :
	UIAllSettings(parent)
{
	CreateImplementation(path_to_settings);
}

void UIAllProjectSettings::CreateImplementation()
{
	__impl.reset(new _impl);
}

void UIAllProjectSettings::CreateImplementation(boost::filesystem::path const path_to_settings)
{
	__impl.reset(new _impl(path_to_settings));
}

void UIAllProjectSettings::_impl::CreateInternalImplementations()
{
	__ui_impl.reset(new _UIRelatedImpl);
	__backend_impl.reset(new _BackendRelatedImpl);
}

void UIAllProjectSettings::_impl::CreateInternalImplementations(boost::filesystem::path const path_to_settings)
{
	__ui_impl.reset(new _UIRelatedImpl(path_to_settings));
	__backend_impl.reset(new _BackendRelatedImpl(path_to_settings));
}
