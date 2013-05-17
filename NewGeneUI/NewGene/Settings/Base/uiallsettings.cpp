#include "uiallsettings.h"

UIAllSettings::UIAllSettings( QObject * parent ) :
	QObject( parent )
{
	init();
}

UIAllSettings::UIAllSettings(boost::filesystem::path const path_to_settings, QObject *)
{
	init();
}

void UIAllSettings::init()
{
	ProvideDefaultSettings();
}
