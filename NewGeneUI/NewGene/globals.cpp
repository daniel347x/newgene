#include "globals.h"
#include "newgenemainwindow.h"

#include "uiprojectmanager.h"
#include "uimodelmanager.h"
#include "uisettingsmanager.h"
#include "uidocumentmanager.h"
#include "uistatusmanager.h"
#include "uiloggingmanager.h"

NewGeneMainWindow * theMainWindow = NULL;

UISettingsManager & settingsManager()
{
	return UISettingsManager::getSettingsManager();
}

UILoggingManager & loggingManager()
{
	return UILoggingManager::getLoggingManager();
}

UIProjectManager & projectManager()
{
	return UIProjectManager::getProjectManager();
}

UIModelManager & modelManager()
{
	return UIModelManager::getModelManager();
}

UIDocumentManager & documentManager()
{
	return UIDocumentManager::getDocumentManager();
}

UIStatusManager & statusManager()
{
	return UIStatusManager::getStatusManager();
}
