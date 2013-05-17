
#include <QWidget>
#include "newgenemainwindow.h"
#include "newgenewidget.h"
#include "uiprojectmanager.h"
#include "uimodelmanager.h"
#include "uisettingsmanager.h"
#include "uidocumentmanager.h"
#include "uistatusmanager.h"
#include "uiloggingmanager.h"

NewGeneWidget::NewGeneWidget( QWidget * self_ ) :
	self( self_ )
{
}

NewGeneMainWindow & NewGeneWidget::mainWindow()
{
	QWidget * pMainWindow = self->window();
	NewGeneMainWindow * pNewGeneMainWindow = dynamic_cast<NewGeneMainWindow *>( pMainWindow );
	return *pNewGeneMainWindow;
}

UISettingsManager & NewGeneWidget::settingsManager()
{
	return UISettingsManager::getSettingsManager();
}

UILoggingManager & NewGeneWidget::loggingManager()
{
	return UILoggingManager::loggingManager();
}

UIProjectManager & NewGeneWidget::projectManager()
{
	return UIProjectManager::projectManager();
}
