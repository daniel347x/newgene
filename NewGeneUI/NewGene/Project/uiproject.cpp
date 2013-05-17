#include "uiproject.h"
#include "uiprojectmanager.h"
#include "uimodelmanager.h"
#include "uisettingsmanager.h"
#include "uidocumentmanager.h"
#include "uistatusmanager.h"
#include "uiloggingmanager.h"
#include "uiprojectmanager.h"
#include "uimodel.h"
#include "uiallprojectsettings.h"
#include "newgenemainwindow.h"

UIProject::UIProject( NewGeneMainWindow * parent ) :
	QObject( parent ),
	model_( NULL ),
	projectSettings_( NULL )
{
}

UIModel * UIProject::model()
{
	return model_;
}

UIAllProjectSettings * UIProject::settings()
{
	return projectSettings_;
}

UIModelManager & UIProject::modelManager()
{
	return UIModelManager::getModelManager();
}

UISettingsManager & UIProject::settingsManager()
{
	return UISettingsManager::getSettingsManager();
}

UIDocumentManager & UIProject::documentManager()
{
	return UIDocumentManager::getDocumentManager();
}

UIStatusManager & UIProject::statusManager()
{
	return UIStatusManager::getStatusManager();
}

UILoggingManager & UIProject::loggingManager()
{
	return UILoggingManager::getLoggingManager();
}

UIProjectManager & UIProject::projectManager()
{
	return UIProjectManager::projectManager();
}
