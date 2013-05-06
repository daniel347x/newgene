#include "uimodelmanager.h"
#include "..\..\NewGeneBackEnd\Utilities\NewGeneException.h"

UIModelManager * UIModelManager::modelManager = NULL;

UIModelManager::UIModelManager(QObject *parent) :
    QObject(parent)
{
}

UIModelManager *UIModelManager::getModelManager()
{
    if (modelManager == NULL)
    {
        modelManager = new UIModelManager;
    }
    if (modelManager == NULL)
    {
        boost::format msg("Model manager not instantiated.");
        throw NewGeneException() << newgene_error_description(msg.str());
    }
    return modelManager;
}

UISettingsManager &UIModelManager::getSettingsManager()
{
    return *UISettingsManager::getSettingsManager();
}

UIModel *UIModelManager::loadDefaultModel()
{
    return NULL;
}
