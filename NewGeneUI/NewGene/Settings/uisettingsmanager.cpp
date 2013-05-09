
#include "uisettingsmanager.h"
#include "..\..\NewGeneBackEnd\Utilities\NewGeneException.h"
#include "uiprojectsettings.h"
#include "uiglobalsettings.h"
#include <QStandardPaths>
#include <fstream>
#include <QDebug>

QString             UISettingsManager::settingsFileName = "NewGene.Settings.xml";
UISettingsManager * UISettingsManager::settings_        = NULL;

UISettingsManager::UISettingsManager( NewGeneMainWindow * parent ) :
	UIManager( parent )
{
	bool found = ObtainSettingsPath();

	if ( !found )
	{
	}
}

UISettingsManager * UISettingsManager::getSettingsManager( NewGeneMainWindow * parent )
{
	// *****************************************************************
	// TODO: Create std::map<> from parent to manager, and retrieve that
	// ... this will support multiple main windows in the future.
	// *****************************************************************
	if ( settings_ == NULL )
	{
		settings_ = new UISettingsManager( parent );

		if ( settings_ )
		{
			settings_ -> which            = MANAGER_SETTINGS;
			settings_ -> which_descriptor = "UISettingsManager";
		}
	}

	if ( settings_ == NULL )
	{
		boost::format msg( "Settings manager not instantiated." );

		throw NewGeneException() << newgene_error_description( msg.str() );
	}

	if ( settings_ -> parent() == NULL )
	{
		boost::format msg( "Settings manager's main window not set." );

		throw NewGeneException() << newgene_error_description( msg.str() );
	}

	return settings_;
}

bool UISettingsManager::ObtainSettingsPath()
{
	QStringList settingsPathStringList = QStandardPaths::standardLocations( QStandardPaths::DataLocation );
	bool        found                  = false;

	for ( int n = 0; n < settingsPathStringList.size(); ++n )
	{
		QString settingsPathString = settingsPathStringList.at( n );

		boost::filesystem::path settingsPathTest( settingsPathString.toStdString() );

		settingsPathTest /= settingsFileName.toStdString();

		if ( boost::filesystem::exists( settingsPathTest ) && boost::filesystem::is_regular_file( settingsPathTest ) )
		{
			settingsPath = settingsPathTest;
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

			settingsPathTest /= settingsFileName.toStdString();

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

					settingsPath = settingsPathTest;

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

UIProjectSettings * UISettingsManager::LoadDefaultProjectSettings(bool const initializing)
{
	return NULL;
}
