#include "uimodelmanager.h"

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
    return modelManager;
}

UIModel *UIModelManager::loadDefaultModel()
{
    return NULL;
}
