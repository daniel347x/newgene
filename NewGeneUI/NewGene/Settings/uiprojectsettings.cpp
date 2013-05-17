#include "uiprojectsettings.h"

UIProjectSettings::UIProjectSettings( QObject * parent ) :
    UISettings( parent )
{
}

Settings * UIProjectSettings::CreateBackendSettings(boost::filesystem::path const path_to_settings)
{
    return new ProjectSettings(path_to_settings);
}

Settings * UIProjectSettings::CreateDefaultBackendSettings()
{
}
