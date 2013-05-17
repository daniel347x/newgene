#include "uiloggingmanager.h"
#include "uimodelmanager.h"
#include "uisettingsmanager.h"
#include "uidocumentmanager.h"
#include "uistatusmanager.h"
#include <QStandardPaths>
#include <QCoreApplication>
#include <fstream>

std::unique_ptr<UILoggingManager> UILoggingManager::_loggingManager;

UILoggingManager::UILoggingManager( QObject * parent ) :
	UIManager( parent )
{

	// *************************************************************************
	// All Managers are instantiated AFTER the application event loop is running
	// *************************************************************************

	bool found = ObtainLogfilePath();

	if ( !found )
	{
		statusManager().PostStatus( "Unable to open NewGene logfile for writing.  No logging will occur.", UIStatusManager::IMPORTANCE_STANDARD, true );
	}
}

UILoggingManager & UILoggingManager::getLoggingManager()
{

	// *****************************************************************
	// TODO: Create std::map<> from parent to manager, and retrieve that
	// ... this will support multiple main windows in the future.
	// *****************************************************************
	if ( _loggingManager == NULL )
	{
		_loggingManager.reset( new UILoggingManager( NULL ) );

		if ( _loggingManager )
		{
			_loggingManager -> which            = MANAGER_LOGGING;
			_loggingManager -> which_descriptor = "UILoggingManager";
		}
	}

	if ( _loggingManager == NULL )
	{
		boost::format msg( "Logging manager not instantiated." );

		throw NewGeneException() << newgene_error_description( msg.str() );
	}

	return *_loggingManager;

}

bool UILoggingManager::ObtainLogfilePath()
{
	// TODO: Make the location of the logfile path be a user setting
	// TODO: Make the default locatio nof the logfile path switch the the user data directory if the application directory is not writable

	QString logfilePathString = QCoreApplication::applicationDirPath();
	boost::filesystem::path logfilePathTest( logfilePathString.toStdString() );
	logfilePathTest /= NewGeneFileNames::logFileName.toStdString();
	std::ofstream logfilePathTestFile;
	logfilePathTestFile.exceptions( std::ifstream::failbit | std::ifstream::badbit );
	bool found = false;

	try
	{
		logfilePathTestFile.open( logfilePathTest.c_str() );

		if ( logfilePathTestFile.is_open() )
		{
			logfilePathTestFile.write( "\n", 1 );  // write 1 character
			logfilePathTestFile.close();

			loggingPath = logfilePathTest;

			// qDebug() << "Created: " << settingsPath.string().c_str();
			found = true;
		}
		else
		{
		}
	}
	catch ( std::ofstream::failure e )
	{
	}

	return found;
}

