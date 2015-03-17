#include "uiloggingmanager.h"
#include "uimodelmanager.h"
#include "uisettingsmanager.h"
#include "uidocumentmanager.h"
#include "uistatusmanager.h"
#include <QStandardPaths>
#include <QCoreApplication>
#include <fstream>

UILoggingManager::UILoggingManager(QObject * parent, UIMessager & messager)
	: QObject(parent)
	, UIManager(messager)
{

	// *************************************************************************
	// All Managers are instantiated AFTER the application event loop is running
	// *************************************************************************

	bool found = ObtainLogfilePath();

	if (!found)
	{
		QString error_message = "Unable to open NewGene logfile for writing.  No logging will occur.";

		if (current_error.length() > 0)
		{
			error_message += " (";
			error_message += current_error;
			error_message += ")";
		}

		statusManagerUI().PostStatus(error_message, UIStatusManager::IMPORTANCE_STANDARD, true);
		current_error.clear();
	}

}

bool UILoggingManager::ObtainLogfilePath()
{
	loggingPath = settingsManagerUI(&messager).ObtainGlobalPath(QStandardPaths::DataLocation, "", NewGeneFileNames::logFileName, true);

	if (loggingPath == boost::filesystem::path())
	{
		return false;
	}

	return true;
}
