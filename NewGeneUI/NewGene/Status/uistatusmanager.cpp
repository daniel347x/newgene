#include "uistatusmanager.h"
#include "..\..\NewGeneBackEnd\Utilities\NewGeneException.h"
#include <QMessageBox>
#include <QStatusBar>
#include "newgenemainwindow.h"

std::unique_ptr<UIStatusManager> UIStatusManager::_statusManager;

UIStatusManager::UIStatusManager( QObject * parent ) :
	UIManager( parent )
{
}

UIStatusManager & UIStatusManager::getStatusManager()
{

	// *****************************************************************
	// TODO: Create std::map<> from parent to manager, and retrieve that
	// ... this will support multiple main windows in the future.
	// *****************************************************************

	if ( _statusManager == NULL )
	{
		_statusManager.reset( new UIStatusManager( NULL ) );

		if ( _statusManager )
		{
			_statusManager->which = MANAGER_STATUS;
			_statusManager->which_descriptor = "UIStatusManager";
		}
	}

	if ( _statusManager == NULL )
	{
		boost::format msg( "Status manager not instantiated." );
		throw NewGeneException() << newgene_error_description( msg.str() );
	}

	return *_statusManager;

}

void UIStatusManager::LogStatus( const QString & status_, const UIStatusManager::IMPORTANCE importance_level )
{
}

void UIStatusManager::PostStatus( QString const & status_, UIStatusManager::IMPORTANCE const  importance_level, bool const forbidWritingToLog )
{

	switch ( importance_level )
	{
		case IMPORTANCE_DEBUG:
		case IMPORTANCE_HIGH:
		case IMPORTANCE_CRITICAL:
			{
				LogStatus( status_, importance_level );
			}
			break;
	}

	if ( importance_level == IMPORTANCE_CRITICAL )
	{
		QMessageBox msgBox;
		msgBox.setText( status_ );
		msgBox.exec();
	}

	NewGeneMainWindow & mainWindow = getMainWindow();
	mainWindow.statusBar()->showMessage( status_ );

}
