
#include "uisettingsmanager.h"
#include "..\..\NewGeneBackEnd\Utilities\NewGeneException.h"
#include "uiallprojectsettings.h"
#include "uiallglobalsettings.h"
#include "uimodelmanager.h"
#include "uisettingsmanager.h"
#include "uidocumentmanager.h"
#include "uistatusmanager.h"
#include <QStandardPaths>
#include <fstream>
#include <QDebug>

std::unique_ptr<UISettingsManager> UISettingsManager::settingsManager;

UISettingsManager::UISettingsManager( QObject * parent ) :
	UIManager( parent )
{

	bool found = ObtainGlobalSettingsPath();

	if ( found )
	{
		statusManager().PostStatus( "Cannot load global applicaton settings; using built-in default global settings.", UIStatusManager::IMPORTANCE_HIGH );
		_global_settings.reset( new UIAllGlobalSettings() );
	}
	else
	{
		_global_settings.reset( new UIAllGlobalSettings( getGlobalSettingsPath() ) );
	}

}

UISettingsManager & UISettingsManager::getSettingsManager()
{
	// *****************************************************************
	// TODO: Create std::map<> from parent to manager, and retrieve that
	// ... this will support multiple main windows in the future.
	// *****************************************************************
	if ( settingsManager == NULL )
	{
		settingsManager.reset( new UISettingsManager( NULL ) );

		if ( settingsManager )
		{
			settingsManager -> which            = MANAGER_SETTINGS;
			settingsManager -> which_descriptor = "UISettingsManager";
		}
	}

	if ( settingsManager == NULL )
	{
		boost::format msg( "Settings manager not instantiated." );

		throw NewGeneException() << newgene_error_description( msg.str() );
	}

	return *settingsManager;
}

bool UISettingsManager::ObtainGlobalSettingsPath()
{
	QStringList settingsPathStringList = QStandardPaths::standardLocations( QStandardPaths::DataLocation );
	bool        found                  = false;

	for ( int n = 0; n < settingsPathStringList.size(); ++n )
	{
		QString settingsPathString = settingsPathStringList.at( n );

		boost::filesystem::path settingsPathTest( settingsPathString.toStdString() );

		settingsPathTest /= NewGeneFileNames::settingsFileName.toStdString();

		if ( boost::filesystem::exists( settingsPathTest ) && boost::filesystem::is_regular_file( settingsPathTest ) )
		{
			global_settings_path = settingsPathTest;
			found        = true;

			break;
		}
	}

	if ( !found )
	{
		for ( int n = 0; n < settingsPathStringList.size(); ++n )
		{
			QString settingsPathString = settingsPathStringList.at( n );

			boost::filesystem::path settingsPathTest( settingsPathString.toStdString() );

			if ( !boost::filesystem::exists( settingsPathTest ) )
			{
				// qDebug() << "Attempting to create directory " << settingsPathTest.string().c_str();
				if ( !boost::filesystem::create_directory( settingsPathTest ) )
				{
					continue;
				}
			}

			settingsPathTest /= NewGeneFileNames::settingsFileName.toStdString();

			// qDebug() << "Attempting to create file " << settingsPathTest.string().c_str();
			std::ofstream settingsPathTestFile;
			settingsPathTestFile.exceptions( std::ifstream::failbit | std::ifstream::badbit );

			try
			{
				settingsPathTestFile.open( settingsPathTest.c_str() );

				if ( settingsPathTestFile.is_open() )
				{
					settingsPathTestFile.write( "\n", 1 );  // write 1 character
					settingsPathTestFile.close();

					global_settings_path = settingsPathTest;

					// qDebug() << "Created: " << settingsPath.string().c_str();
					found = true;

					break;
				}
				else
				{
					continue;
				}
			}
			catch ( std::ofstream::failure e )
			{
				continue;
			}
		}
	}

	return found;
}
