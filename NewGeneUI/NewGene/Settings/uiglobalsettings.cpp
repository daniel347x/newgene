#include "uiglobalsettings.h"

UIGlobalSettings::UIGlobalSettings( QObject * parent ) :
    UISettings( parent )
{
}

Settings * UIGlobalSettings::CreateBackendSettings(boost::filesystem::path const path_to_settings)
{
    return new GlobalSettings(path_to_settings);
}

Settings * UIGlobalSettings::CreateDefaultBackendSettings()
{
}
