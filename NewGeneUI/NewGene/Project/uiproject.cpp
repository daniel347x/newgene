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

UIProject::UIProject(Messager &, NewGeneMainWindow * parent) :
	QObject(parent)
{
}

UIProject::UIProject(Messager & messager, boost::filesystem::path const path_to_settings, NewGeneMainWindow * parent) :
	QObject(parent),
	_project_settings( new UIAllProjectSettings(messager, path_to_settings) )
{
}

UIModel * UIProject::model()
{
	//return model_;
	return NULL;
}

UIAllProjectSettings * UIProject::settings()
{
	//return projectSettings_;
	return NULL;
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
	return UIProjectManager::getProjectManager();
}
