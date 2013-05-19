#include "uiproject.h"
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
