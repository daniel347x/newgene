
#include "uisettingsmanager.h"
#include "../../NewGeneBackEnd/Utilities/NewGeneException.h"
#include "uiallprojectsettings.h"
#include "uiallglobalsettings.h"
#include "uimodelmanager.h"
#include "uisettingsmanager.h"
#include "uidocumentmanager.h"
#include "uistatusmanager.h"
#include "uiinputproject.h"
#include "uioutputproject.h"
#include <fstream>
#include <QDebug>

UISettingsManager::UISettingsManager(QObject * parent, UIMessager & messager)
	: QObject(parent)
	, UIManager(messager)
{

	// *************************************************************************
	// All Managers are instantiated AFTER the application event loop is running
	// *************************************************************************

	boost::filesystem::path the_path = ObtainGlobalPath(QStandardPaths::AppLocalDataLocation, "", NewGeneFileNames::settingsFileName, true);

	{
		UIMessager messager;

		if (the_path == boost::filesystem::path())
		{
			statusManagerUI().PostStatus("Cannot load global applicaton settings; using built-in default global settings.", UIStatusManager::IMPORTANCE_HIGH);
			_global_settings.reset(new UIAllGlobalSettings(messager));
		}
		else
		{
			global_settings_path = the_path;
			_global_settings.reset(new UIAllGlobalSettings(messager, getGlobalSettingsPath()));
		}
	}

}

boost::filesystem::path UISettingsManager::ObtainGlobalPath(QStandardPaths::StandardLocation const & location, QString const & postfix_part_of_path,
		QString const & filename_no_path, bool const create_if_not_found)
{

	boost::filesystem::path the_found_path;

	QStringList settingsPathStringList = QStandardPaths::standardLocations(location);

	for (int n = 0; n < settingsPathStringList.size(); ++n)
	{
		QString settingsPathString = settingsPathStringList.at(n);

		boost::filesystem::path settingsPathTest(settingsPathString.toStdString());

		settingsPathTest /= postfix_part_of_path.toStdString();

		settingsPathTest /= filename_no_path.toStdString();

		if (boost::filesystem::exists(settingsPathTest) && boost::filesystem::is_regular_file(settingsPathTest))
		{
			the_found_path = settingsPathTest;
			break;
		}
	}

	if (create_if_not_found)
	{
		if (the_found_path == boost::filesystem::path())
		{
			for (int n = 0; n < settingsPathStringList.size(); ++n)
			{
				QString settingsPathString = settingsPathStringList.at(n);

				boost::filesystem::path settingsPathTest(settingsPathString.toStdString());

				settingsPathTest /= postfix_part_of_path.toStdString();

				if (!boost::filesystem::exists(settingsPathTest))
				{
					if (!boost::filesystem::create_directories(settingsPathTest))
					{
						continue;
					}
				}

				settingsPathTest /= filename_no_path.toStdString();

				std::ofstream settingsPathTestFile;
				settingsPathTestFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);

				try
				{
					settingsPathTestFile.open(settingsPathTest.c_str());

					if (settingsPathTestFile.is_open())
					{
						settingsPathTestFile << std::endl; // Write something to make sure permissions are accepted
						settingsPathTestFile.close();
						the_found_path = settingsPathTest;		\
						break;
					}
					else
					{
						continue;
					}
				}
				catch (std::ofstream::failure)
				{
					continue;
				}
			}
		}
	}

	return the_found_path;
}

std::unique_ptr<BackendGlobalSetting> UISettingsManager::getSetting(Messager & messager_, GLOBAL_SETTINGS_BACKEND_NAMESPACE::GLOBAL_SETTINGS_BACKEND const which_setting)
{
	UIMessager & messager = static_cast<UIMessager &>(messager_);

	std::unique_ptr<BackendGlobalSetting> the_setting_ = _global_settings->getBackendSettings().GetSetting(messager, which_setting);
	BackendGlobalSetting * the_setting = the_setting_.get();

	if (the_setting == NULL)
	{
		boost::format msg("Cannot retrieve setting \"%1%\"");
		msg % which_setting;
		messager.AppendMessage(new MessagerWarningMessage(MESSAGER_MESSAGE__SETTING_NOT_FOUND, msg.str()));
		return std::unique_ptr<BackendGlobalSetting>(new BackendGlobalSetting(messager));
	}

	return the_setting_;
}

std::unique_ptr<UIGlobalSetting> UISettingsManager::getSetting(Messager & messager_, GLOBAL_SETTINGS_UI_NAMESPACE::GLOBAL_SETTINGS_UI const which_setting)
{
	UIMessager & messager = static_cast<UIMessager &>(messager_);

	std::unique_ptr<UIGlobalSetting> the_setting_ = _global_settings->getUISettings().GetSetting(messager, which_setting);
	UIGlobalSetting * the_setting = the_setting_.get();

	if (the_setting == NULL)
	{
		boost::format msg("Cannot retrieve setting \"%1%\"");
		msg % which_setting;
		messager.AppendMessage(new MessagerWarningMessage(MESSAGER_MESSAGE__SETTING_NOT_FOUND, msg.str()));
		return std::unique_ptr<UIGlobalSetting>(new UIGlobalSetting(messager));
	}

	return the_setting_;
}

std::unique_ptr<BackendProjectInputSetting> UISettingsManager::getSetting(Messager & messager_, InputProjectSettings * project_settings,
		INPUT_PROJECT_SETTINGS_BACKEND_NAMESPACE::INPUT_PROJECT_SETTINGS_BACKEND const which_setting)
{
	UIMessager & messager = static_cast<UIMessager &>(messager_);

	if (project_settings == NULL)
	{
		boost::format msg("Cannot retrieve setting (\"%1%\") for a NULL project settings.");
		msg % which_setting;
		messager.AppendMessage(new MessagerWarningMessage(MESSAGER_MESSAGE__PROJECT_NOT_AVAILABLE, msg.str()));
		return std::unique_ptr<BackendProjectInputSetting>(new BackendProjectInputSetting(messager));
	}

	std::unique_ptr<BackendProjectInputSetting> the_setting_ = project_settings->GetSetting(messager, which_setting);
	BackendProjectInputSetting * the_setting = the_setting_.get();

	if (the_setting == NULL)
	{
		boost::format msg("Cannot retrieve setting \"%1%\"");
		msg % which_setting;
		messager.AppendMessage(new MessagerWarningMessage(MESSAGER_MESSAGE__SETTING_NOT_FOUND, msg.str()));
		return std::unique_ptr<BackendProjectInputSetting>(new BackendProjectInputSetting(messager));
	}

	return the_setting_;
}

std::unique_ptr<BackendProjectOutputSetting> UISettingsManager::getSetting(Messager & messager_, OutputProjectSettings * project_settings,
		OUTPUT_PROJECT_SETTINGS_BACKEND_NAMESPACE::OUTPUT_PROJECT_SETTINGS_BACKEND const which_setting)
{
	UIMessager & messager = static_cast<UIMessager &>(messager_);

	if (project_settings == NULL)
	{
		boost::format msg("Cannot retrieve setting (\"%1%\") for a NULL project settings.");
		msg % which_setting;
		messager.AppendMessage(new MessagerWarningMessage(MESSAGER_MESSAGE__PROJECT_NOT_AVAILABLE, msg.str()));
		return std::unique_ptr<BackendProjectOutputSetting>(new BackendProjectOutputSetting(messager));
	}

	std::unique_ptr<BackendProjectOutputSetting> the_setting_ = project_settings->GetSetting(messager, which_setting);
	BackendProjectOutputSetting * the_setting = the_setting_.get();

	if (the_setting == NULL)
	{
		boost::format msg("Cannot retrieve setting \"%1%\"");
		msg % which_setting;
		messager.AppendMessage(new MessagerWarningMessage(MESSAGER_MESSAGE__SETTING_NOT_FOUND, msg.str()));
		return std::unique_ptr<BackendProjectOutputSetting>(new BackendProjectOutputSetting(messager));
	}

	return the_setting_;
}

std::unique_ptr<InputModelSetting> UISettingsManager::getSetting(Messager & messager_, InputModelSettings * model_settings,
		INPUT_MODEL_SETTINGS_NAMESPACE::INPUT_MODEL_SETTINGS const which_setting)
{
	UIMessager & messager = static_cast<UIMessager &>(messager_);

	if (model_settings == NULL)
	{
		boost::format msg("Cannot retrieve setting (\"%1%\") for a NULL model settings.");
		msg % which_setting;
		messager.AppendMessage(new MessagerWarningMessage(MESSAGER_MESSAGE__PROJECT_NOT_AVAILABLE, msg.str()));
		return std::unique_ptr<InputModelSetting>(new InputModelSetting(messager));
	}

	std::unique_ptr<InputModelSetting> the_setting_ = model_settings->GetSetting(messager, which_setting);
	InputModelSetting * the_setting = the_setting_.get();

	if (the_setting == NULL)
	{
		boost::format msg("Cannot retrieve setting \"%1%\"");
		msg % which_setting;
		messager.AppendMessage(new MessagerWarningMessage(MESSAGER_MESSAGE__SETTING_NOT_FOUND, msg.str()));
		return std::unique_ptr<InputModelSetting>(new InputModelSetting(messager));
	}

	return the_setting_;
}

std::unique_ptr<OutputModelSetting> UISettingsManager::getSetting(Messager & messager_, OutputModelSettings * model_settings,
		OUTPUT_MODEL_SETTINGS_NAMESPACE::OUTPUT_MODEL_SETTINGS const which_setting)
{
	UIMessager & messager = static_cast<UIMessager &>(messager_);

	if (model_settings == NULL)
	{
		boost::format msg("Cannot retrieve setting (\"%1%\") for a NULL model settings.");
		msg % which_setting;
		messager.AppendMessage(new MessagerWarningMessage(MESSAGER_MESSAGE__PROJECT_NOT_AVAILABLE, msg.str()));
		return std::unique_ptr<OutputModelSetting>(new OutputModelSetting(messager));
	}

	std::unique_ptr<OutputModelSetting> the_setting_ = model_settings->GetSetting(messager, which_setting);
	OutputModelSetting * the_setting = the_setting_.get();

	if (the_setting == NULL)
	{
		boost::format msg("Cannot retrieve setting \"%1%\"");
		msg % which_setting;
		messager.AppendMessage(new MessagerWarningMessage(MESSAGER_MESSAGE__SETTING_NOT_FOUND, msg.str()));
		return std::unique_ptr<OutputModelSetting>(new OutputModelSetting(messager));
	}

	return the_setting_;
}

std::unique_ptr<UIProjectInputSetting> UISettingsManager::getSetting(Messager & messager_, UIInputProjectSettings * project_settings,
		INPUT_PROJECT_SETTINGS_UI_NAMESPACE::INPUT_PROJECT_SETTINGS_UI const which_setting)
{
	UIMessager & messager = static_cast<UIMessager &>(messager_);

	if (project_settings == NULL)
	{
		boost::format msg("Cannot retrieve setting (\"%1%\") for a NULL project settings.");
		msg % which_setting;
		messager.AppendMessage(new MessagerWarningMessage(MESSAGER_MESSAGE__PROJECT_NOT_AVAILABLE, msg.str()));
		return std::unique_ptr<UIProjectInputSetting>(new UIProjectInputSetting(messager));
	}

	std::unique_ptr<UIProjectInputSetting> the_setting_ = project_settings->getUISettings().GetSetting(messager, which_setting);
	UIProjectInputSetting * the_setting = the_setting_.get();

	if (the_setting == NULL)
	{
		boost::format msg("Cannot retrieve setting \"%1%\"");
		msg % which_setting;
		messager.AppendMessage(new MessagerWarningMessage(MESSAGER_MESSAGE__SETTING_NOT_FOUND, msg.str()));
		return std::unique_ptr<UIProjectInputSetting>(new UIProjectInputSetting(messager));
	}

	return the_setting_;
}

std::unique_ptr<UIProjectOutputSetting> UISettingsManager::getSetting(Messager & messager_, UIOutputProjectSettings * project_settings,
		OUTPUT_PROJECT_SETTINGS_UI_NAMESPACE::OUTPUT_PROJECT_SETTINGS_UI const which_setting)
{
	UIMessager & messager = static_cast<UIMessager &>(messager_);

	if (project_settings == NULL)
	{
		boost::format msg("Cannot retrieve setting (\"%1%\") for a NULL project settings.");
		msg % which_setting;
		messager.AppendMessage(new MessagerWarningMessage(MESSAGER_MESSAGE__PROJECT_NOT_AVAILABLE, msg.str()));
		return std::unique_ptr<UIProjectOutputSetting>(new UIProjectOutputSetting(messager));
	}

	std::unique_ptr<UIProjectOutputSetting> the_setting_ = project_settings->getUISettings().GetSetting(messager, which_setting);
	UIProjectOutputSetting * the_setting = the_setting_.get();

	if (the_setting == NULL)
	{
		boost::format msg("Cannot retrieve setting \"%1%\"");
		msg % which_setting;
		messager.AppendMessage(new MessagerWarningMessage(MESSAGER_MESSAGE__SETTING_NOT_FOUND, msg.str()));
		return std::unique_ptr<UIProjectOutputSetting>(new UIProjectOutputSetting(messager));
	}

	return the_setting_;
}
