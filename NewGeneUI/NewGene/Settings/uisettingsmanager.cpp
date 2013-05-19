
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

UISettingsManager::UISettingsManager( QObject * parent )
	: QObject(parent)
	, UIManager()
{

	// *************************************************************************
	// All Managers are instantiated AFTER the application event loop is running
	// *************************************************************************

	bool found = ObtainGlobalSettingsPath();

	{
		UIMessager messager;
		if ( !found )
		{
			statusManagerUI().PostStatus( "Cannot load global applicaton settings; using built-in default global settings.", UIStatusManager::IMPORTANCE_HIGH );
			_global_settings.reset( new UIAllGlobalSettings(messager) );
		}
		else
		{
			_global_settings.reset( new UIAllGlobalSettings(messager, getGlobalSettingsPath()) );
		}
	}

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

BackendGlobalSetting const & UISettingsManager::get_setting(GLOBAL_SETTINGS_BACKEND_NAMESPACE::GLOBAL_SETTINGS_BACKEND const which_setting)
{
	//_global_settings->__impl
	return BackendGlobalSetting();
}

BackendProjectSetting const & UISettingsManager::get_setting(PROJECT_SETTINGS_BACKEND_NAMESPACE::PROJECT_SETTINGS_BACKEND const which_setting)
{
	return BackendProjectSetting();
}

UIGlobalSetting const & UISettingsManager::get_setting(GLOBAL_SETTINGS_UI_NAMESPACE::GLOBAL_SETTINGS_UI const which_setting)
{
	return UIGlobalSetting();
}

UIProjectSetting const & UISettingsManager::get_setting(PROJECT_SETTINGS_UI_NAMESPACE::PROJECT_SETTINGS_UI const which_setting)
{
	return UIProjectSetting();
}

