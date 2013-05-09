#include "uiproject.h"
#include "uiprojectmanager.h"
#include "uimodelmanager.h"
#include "uisettingsmanager.h"
#include "uidocumentmanager.h"
#include "uistatusmanager.h"
#include "uimodel.h"

UIProject::UIProject(QObject *parent) :
	QObject(parent)
{
}

UIModel * UIProject::model()
{
	return model_.get();
}

UIModelManager &UIProject::modelManager(NewGeneMainWindow * parent)
{
	return *UIModelManager::getModelManager(parent);
}

UISettingsManager &UIProject::settingsManager(NewGeneMainWindow * parent)
{
	return *UISettingsManager::getSettingsManager(parent);
}

UIDocumentManager &UIProject::documentManager(NewGeneMainWindow * parent)
{
	return *UIDocumentManager::getDocumentManager(parent);
}

UIStatusManager &UIProject::statusManager(NewGeneMainWindow * parent)
{
	return *UIStatusManager::getStatusManager(parent);
}
