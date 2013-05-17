#include "uiallglobalsettings.h"

UIAllGlobalSettings::UIAllGlobalSettings( QObject * parent ) :
	UIAllSettings( parent )
{
	CreateImplementation();
}

UIAllGlobalSettings::UIAllGlobalSettings(boost::filesystem::path const path_to_settings, QObject * parent) :
	UIAllSettings(parent)
{
	CreateImplementation(path_to_settings);
}

void UIAllGlobalSettings::CreateImplementation()
{
	__impl.reset(new _impl);
}

void UIAllGlobalSettings::CreateImplementation(boost::filesystem::path const path_to_settings)
{
	__impl.reset(new _impl(path_to_settings));
}

void UIAllGlobalSettings::_impl::CreateInternalImplementations()
{
	__ui_impl.reset(new _UIRelatedImpl);
	__backend_impl.reset(new _BackendRelatedImpl);
}

void UIAllGlobalSettings::_impl::CreateInternalImplementations(boost::filesystem::path const path_to_settings)
{
	__ui_impl.reset(new _UIRelatedImpl(path_to_settings));
	__backend_impl.reset(new _BackendRelatedImpl(path_to_settings));
}
