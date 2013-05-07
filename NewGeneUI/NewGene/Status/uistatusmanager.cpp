#include "uistatusmanager.h"
#include "..\..\NewGeneBackEnd\Utilities\NewGeneException.h"
#include <QMessageBox>
#include <QStatusBar>
#include "newgenemainwindow.h"

UIStatusManager * UIStatusManager::status_ = NULL;

UIStatusManager::UIStatusManager(NewGeneMainWindow *parent) :
	UIManager(parent)
{
}

UIStatusManager * UIStatusManager::getStatusManager(NewGeneMainWindow * parent)
{

	// *****************************************************************
	// TODO: Create std::map<> from parent to manager, and retrieve that
	// ... this will support multiple main windows in the future.
	// *****************************************************************

	if (status_ == NULL)
	{
		status_ = new UIStatusManager(parent);
		if (status_)
		{
			status_->which = MANAGER_STATUS;
			status_->which_descriptor = "UIStatusManager";
		}
	}
	if (status_ == NULL)
	{
		boost::format msg("Status manager not instantiated.");
		throw NewGeneException() << newgene_error_description(msg.str());
	}
	if (status_->parent() == NULL)
	{
		boost::format msg("Status manager's main window not set.");
		throw NewGeneException() << newgene_error_description(msg.str());
	}
	return status_;

}

void UIStatusManager::LogStatus(const QString &status_, const UIStatusManager::IMPORTANCE importance_level)
{
}

void UIStatusManager::PostStatus(const QString &status_, const UIStatusManager::IMPORTANCE importance_level)
{

	switch (importance_level)
	{
	case IMPORTANCE_DEBUG:
	case IMPORTANCE_HIGH:
	case IMPORTANCE_CRITICAL:
		{
			LogStatus(status_, importance_level);
		}
		break;
	}

	if (importance_level == IMPORTANCE_CRITICAL)
	{
		QMessageBox msgBox;
		msgBox.setText(status_);
		msgBox.exec();
	}

	NewGeneMainWindow & mainWindow = getMainWindow();
	mainWindow.statusBar()->showMessage(status_);

}
