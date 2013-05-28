
#include "uisettingsmanager.h"
#include "..\..\NewGeneBackEnd\Utilities\NewGeneException.h"
#include "uiallprojectsettings.h"
#include "uiallglobalsettings.h"
#include "uimodelmanager.h"
#include "uisettingsmanager.h"
#include "uidocumentmanager.h"
#include "uistatusmanager.h"
#include "uiinputproject.h"
#include "uioutputproject.h"
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

std::unique_ptr<BackendGlobalSetting> UISettingsManager::getSetting(UIMessager & messager, GLOBAL_SETTINGS_BACKEND_NAMESPACE::GLOBAL_SETTINGS_BACKEND const which_setting)
{
	std::unique_ptr<BackendGlobalSetting> the_setting_ = _global_settings->getBackendSettings().GetSetting<false>(messager, which_setting);
	BackendGlobalSetting * the_setting = the_setting_.get();
	if (the_setting == NULL)
	{
		boost::format msg("Cannot retrieve setting \"%1%\"");
		msg % which_setting;
		messager.AppendMessage(new MessagerWarningMessage(MESSAGER_MESSAGE__SETTING_NOT_FOUND, msg.str()));
		return std::unique_ptr<BackendGlobalSetting>(new BackendGlobalSetting());
	}
	return the_setting_;
}

std::unique_ptr<UIGlobalSetting> UISettingsManager::getSetting(UIMessager & messager, GLOBAL_SETTINGS_UI_NAMESPACE::GLOBAL_SETTINGS_UI const which_setting)
{
	std::unique_ptr<UIGlobalSetting> the_setting_ = _global_settings->getUISettings().GetSetting<true>(messager, which_setting);
	UIGlobalSetting * the_setting = the_setting_.get();
	if (the_setting == NULL)
	{
		boost::format msg("Cannot retrieve setting \"%1%\"");
		msg % which_setting;
		messager.AppendMessage(new MessagerWarningMessage(MESSAGER_MESSAGE__SETTING_NOT_FOUND, msg.str()));
		return std::unique_ptr<UIGlobalSetting>(new UIGlobalSetting());
	}
	return the_setting_;
}

std::unique_ptr<BackendProjectInputSetting> UISettingsManager::getSetting(UIMessager & messager, UIInputProject * project, INPUT_PROJECT_SETTINGS_BACKEND_NAMESPACE::INPUT_PROJECT_SETTINGS_BACKEND const which_setting)
{
	if (project == NULL)
	{
		boost::format msg("Cannot retrieve setting (\"%1%\") for a NULL project.");
		msg % which_setting;
		messager.AppendMessage(new MessagerWarningMessage(MESSAGER_MESSAGE__PROJECT_NOT_AVAILABLE, msg.str()));
		return std::unique_ptr<BackendProjectInputSetting>(new BackendProjectInputSetting());
	}
	std::unique_ptr<BackendProjectInputSetting> the_setting_ = project->settings().getBackendSettings().GetSetting<false>(messager, which_setting);
	BackendProjectInputSetting * the_setting = the_setting_.get();
	if (the_setting == NULL)
	{
		boost::format msg("Cannot retrieve setting \"%1%\"");
		msg % which_setting;
		messager.AppendMessage(new MessagerWarningMessage(MESSAGER_MESSAGE__SETTING_NOT_FOUND, msg.str()));
		return std::unique_ptr<BackendProjectInputSetting>(new BackendProjectInputSetting());
	}
	return the_setting_;
}

std::unique_ptr<BackendProjectOutputSetting> UISettingsManager::getSetting(UIMessager & messager, UIOutputProject * project, OUTPUT_PROJECT_SETTINGS_BACKEND_NAMESPACE::OUTPUT_PROJECT_SETTINGS_BACKEND const which_setting)
{
	if (project == NULL)
	{
		boost::format msg("Cannot retrieve setting (\"%1%\") for a NULL project.");
		msg % which_setting;
		messager.AppendMessage(new MessagerWarningMessage(MESSAGER_MESSAGE__PROJECT_NOT_AVAILABLE, msg.str()));
		return std::unique_ptr<BackendProjectOutputSetting>(new BackendProjectOutputSetting());
	}
	std::unique_ptr<BackendProjectOutputSetting> the_setting_ = project->settings().getBackendSettings().GetSetting<false>(messager, which_setting);
	BackendProjectOutputSetting * the_setting = the_setting_.get();
	if (the_setting == NULL)
	{
		boost::format msg("Cannot retrieve setting \"%1%\"");
		msg % which_setting;
		messager.AppendMessage(new MessagerWarningMessage(MESSAGER_MESSAGE__SETTING_NOT_FOUND, msg.str()));
		return std::unique_ptr<BackendProjectOutputSetting>(new BackendProjectOutputSetting());
	}
	return the_setting_;
}

std::unique_ptr<UIProjectInputSetting> UISettingsManager::getSetting(UIMessager & messager, UIInputProject * project, INPUT_PROJECT_SETTINGS_UI_NAMESPACE::INPUT_PROJECT_SETTINGS_UI const which_setting)
{
	if (project == NULL)
	{
		boost::format msg("Cannot retrieve setting (\"%1%\") for a NULL project.");
		msg % which_setting;
		messager.AppendMessage(new MessagerWarningMessage(MESSAGER_MESSAGE__PROJECT_NOT_AVAILABLE, msg.str()));
		return std::unique_ptr<UIProjectInputSetting>(new UIProjectInputSetting());
	}
	std::unique_ptr<UIProjectInputSetting> the_setting_ = project->settings().getUISettings().GetSetting<true>(messager, which_setting);
	UIProjectInputSetting * the_setting = the_setting_.get();
	if (the_setting == NULL)
	{
		boost::format msg("Cannot retrieve setting \"%1%\"");
		msg % which_setting;
		messager.AppendMessage(new MessagerWarningMessage(MESSAGER_MESSAGE__SETTING_NOT_FOUND, msg.str()));
		return std::unique_ptr<UIProjectInputSetting>(new UIProjectInputSetting());
	}
	return the_setting_;
}

std::unique_ptr<UIProjectOutputSetting> UISettingsManager::getSetting(UIMessager & messager, UIOutputProject * project, OUTPUT_PROJECT_SETTINGS_UI_NAMESPACE::OUTPUT_PROJECT_SETTINGS_UI const which_setting)
{
	if (project == NULL)
	{
		boost::format msg("Cannot retrieve setting (\"%1%\") for a NULL project.");
		msg % which_setting;
		messager.AppendMessage(new MessagerWarningMessage(MESSAGER_MESSAGE__PROJECT_NOT_AVAILABLE, msg.str()));
		return std::unique_ptr<UIProjectOutputSetting>(new UIProjectOutputSetting());
	}
	std::unique_ptr<UIProjectOutputSetting> the_setting_ = project->settings().getUISettings().GetSetting<true>(messager, which_setting);
	UIProjectOutputSetting * the_setting = the_setting_.get();
	if (the_setting == NULL)
	{
		boost::format msg("Cannot retrieve setting \"%1%\"");
		msg % which_setting;
		messager.AppendMessage(new MessagerWarningMessage(MESSAGER_MESSAGE__SETTING_NOT_FOUND, msg.str()));
		return std::unique_ptr<UIProjectOutputSetting>(new UIProjectOutputSetting());
	}
	return the_setting_;
}
