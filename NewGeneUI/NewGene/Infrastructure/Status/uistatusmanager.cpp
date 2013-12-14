#include "uistatusmanager.h"
#include "../../NewGeneBackEnd/Utilities/NewGeneException.h"
#include <QMessageBox>
#include <QStatusBar>
#include "Widgets/newgenemainwindow.h"

UIStatusManager::UIStatusManager( QObject * parent )
	: QObject(parent)
	, UIManager()
{

	// *************************************************************************
	// All Managers are instantiated AFTER the application event loop is running
	// *************************************************************************

}

void UIStatusManager::LogStatus( const QString & /* status_ */, const UIStatusManager::IMPORTANCE /* importance_level */ )
{
}

void UIStatusManager::PostStatus( QString const & _status, UIStatusManager::IMPORTANCE const importance_level, bool const /* forbidWritingToLog */ )
{

	switch ( importance_level )
	{
		case IMPORTANCE_DEBUG:
		case IMPORTANCE_HIGH:
		case IMPORTANCE_CRITICAL:
			{
				LogStatus( _status, importance_level );
			}
			break;
        default:
        {
        }
            break;

	if ( importance_level == IMPORTANCE_CRITICAL )
	{
		QMessageBox msgBox;
		msgBox.setText( _status );
		msgBox.exec();
	}

	bool success = false;
	try
	{
		NewGeneMainWindow & mainWindow = getMainWindow();
		mainWindow.statusBar()->showMessage( _status );
		success = true;
	}
	catch (NewGeneException &)
	{
	}

	if (!success)
	{
		QMessageBox msgBox;
		msgBox.setText( _status );
		msgBox.exec();

		if (importance_level == IMPORTANCE_CRITICAL)
		{
			boost::format msg( "Unrecoverable error: NewGene will now exit." );
			msg % which_descriptor.toStdString();
			throw NewGeneException() << newgene_error_description( msg.str() );
		}
	}

}

void UIStatusManager::ReceiveStatus(STD_STRING msg, int importance_, bool forbid_log)
{
	IMPORTANCE importance = (IMPORTANCE)(importance_);
	PostStatus(msg.c_str(), importance, forbid_log);
}
