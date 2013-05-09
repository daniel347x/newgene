#include "uiproject.h"
#include "uiprojectmanager.h"
#include "uimodelmanager.h"
#include "uisettingsmanager.h"
#include "uidocumentmanager.h"
#include "uistatusmanager.h"
#include "uimodel.h"
#include "uiprojectsettings.h"
#include "newgenemainwindow.h"

UIProject::UIProject(NewGeneMainWindow *parent) :
	QObject(parent)
{
	parent_.reset(parent);
}

UIModel * UIProject::model()
{
	return model_.get();
}

UIProjectSettings * UIProject::settings()
{
	return settings_.get();
}

UIModelManager &UIProject::modelManager()
{
	return *UIModelManager::getModelManager(parent_.get());
}

UISettingsManager &UIProject::settingsManager()
{
	return *UISettingsManager::getSettingsManager(parent_.get());
}

UIDocumentManager &UIProject::documentManager()
{
	return *UIDocumentManager::getDocumentManager(parent_.get());
}

UIStatusManager &UIProject::statusManager()
{
	return *UIStatusManager::getStatusManager(parent_.get());
}
