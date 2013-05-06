#include "uisettings.h"

UISettingsManager * UISettingsManager::settings_ = NULL;

UISettingsManager::UISettingsManager(QObject *parent) :
    QObject(parent)
{
}

UISettingsManager * UISettingsManager::getSettingsManager()
{
    if (settings_ == NULL)
    {
        settings_ = new UISettingsManager;
    }
    return settings_;
}
