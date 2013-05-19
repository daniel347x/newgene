#include "uistatusmanager.h"
#include "..\..\NewGeneBackEnd\Utilities\NewGeneException.h"
#include <QMessageBox>
#include <QStatusBar>
#include "newgenemainwindow.h"

UIStatusManager::UIStatusManager( QObject * parent )
	: QObject(parent)
	, UIManager()
{

	// *************************************************************************
	// All Managers are instantiated AFTER the application event loop is running
	// *************************************************************************

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
