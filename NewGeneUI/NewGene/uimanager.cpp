#include "uimanager.h"
#include "newgenemainwindow.h"
#include "uimodelmanager.h"
#include "uisettingsmanager.h"
#include "uidocumentmanager.h"
#include "uistatusmanager.h"
#include "uiloggingmanager.h"

UIManager::UIManager(NewGeneMainWindow *parent) :
	QObject(static_cast<QObject*>(parent))
{
}

NewGeneMainWindow &UIManager::getMainWindow()
{
	QObject * myParent = parent();
	if (myParent == NULL)
	{
		boost::format msg("Parent is not valid in %1%");
		msg % which_descriptor.toStdString();
		throw NewGeneException() << newgene_error_description(msg.str());
	}

	NewGeneMainWindow * myWindow = NULL;
	try
	{
		myWindow = dynamic_cast<NewGeneMainWindow *>(myParent);
	}
	catch (std::bad_cast)
	{
		boost::format msg("Main window not valid in %1%");
		msg % which_descriptor.toStdString();
		throw NewGeneException() << newgene_error_description(msg.str());
	}

	return *myWindow;
}

UIModelManager &UIManager::modelManager(NewGeneMainWindow * parent)
{
	return UIModelManager::getModelManager(parent);
}

UISettingsManager &UIManager::settingsManager(NewGeneMainWindow * parent)
{
	return UISettingsManager::getSettingsManager(parent);
}

UIDocumentManager &UIManager::documentManager(NewGeneMainWindow * parent)
{
	return UIDocumentManager::getDocumentManager(parent);
}

UIStatusManager &UIManager::statusManager(NewGeneMainWindow * parent)
{
	return UIStatusManager::getStatusManager(parent);
}

UILoggingManager &UIManager::loggingManager(NewGeneMainWindow * parent)
{
	return UILoggingManager::getLoggingManager(parent);
}
