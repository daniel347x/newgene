#include "uiloggingmanager.h"
#include "uimodelmanager.h"
#include "uisettingsmanager.h"
#include "uidocumentmanager.h"
#include "uistatusmanager.h"
#include <QStandardPaths>
#include <QCoreApplication>
#include <fstream>

UILoggingManager::UILoggingManager( QObject * parent )
	: QObject(parent)
	, UIManager()
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

