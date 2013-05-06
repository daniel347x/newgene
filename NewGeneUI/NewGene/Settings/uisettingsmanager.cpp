#include "uisettingsmanager.h"
#include "..\..\NewGeneBackEnd\Utilities\NewGeneException.h"

UISettingsManager * UISettingsManager::settings_ = NULL;

UISettingsManager::UISettingsManager(QObject *parent) :
    QObject(parent),
    dirty(false)
{

}

UISettingsManager * UISettingsManager::getSettingsManager()
{
    if (settings_ == NULL)
    {
        settings_ = new UISettingsManager;
    }
    if (settings_ == NULL)
    {
        boost::format msg("Settings manager not instantiated.");
        throw NewGeneException() << newgene_error_description(msg.str());
    }
    return settings_;
}
