#include "globals.h"
#include "newgenemainwindow.h"

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
