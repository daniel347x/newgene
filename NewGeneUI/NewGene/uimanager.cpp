#include "uimanager.h"
#include "newgenemainwindow.h"
#include "uimodelmanager.h"
#include "uisettingsmanager.h"
#include "uidocumentmanager.h"
#include "uistatusmanager.h"
#include "uiloggingmanager.h"
#include <QApplication>

UIManager::UIManager( QObject * parent ) :
	QObject( parent )
{
}

NewGeneMainWindow & UIManager::getMainWindow()
{
	if ( theMainWindow == NULL )
	{
		boost::format msg( "Main window does not exist in %1%" );
		msg % which_descriptor.toStdString();
		throw NewGeneException() << newgene_error_description( msg.str() );
	}

	return *theMainWindow;
}

UIModelManager & UIManager::modelManager()
{
	return UIModelManager::getModelManager();
}

UISettingsManager & UIManager::settingsManager()
{
	return UISettingsManager::getSettingsManager();
}

UIDocumentManager & UIManager::documentManager()
{
	return UIDocumentManager::getDocumentManager();
}

UIStatusManager & UIManager::statusManager()
{
	return UIStatusManager::getStatusManager();
}

UILoggingManager & UIManager::loggingManager()
{
	return UILoggingManager::getLoggingManager();
}
