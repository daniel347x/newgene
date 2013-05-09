
#include <QWidget>
#include "newgenemainwindow.h"
#include "newgenewidget.h"
#include "uiprojectmanager.h"
#include "uimodelmanager.h"
#include "uisettingsmanager.h"
#include "uidocumentmanager.h"
#include "uistatusmanager.h"

NewGeneWidget::NewGeneWidget(QWidget * self_) :
	self(self_)
{
}

NewGeneMainWindow & NewGeneWidget::mainWindow()
{
	QWidget * pMainWindow = self->window();
	NewGeneMainWindow * pNewGeneMainWindow = dynamic_cast<NewGeneMainWindow *>(pMainWindow);
	return *pNewGeneMainWindow;
}

UISettingsManager &NewGeneWidget::settingsManager(NewGeneMainWindow * parent)
{
	return *UISettingsManager::getSettingsManager(parent);
}

UIProjectManager &NewGeneWidget::projectManager(NewGeneMainWindow * parent)
{
	return *UIProjectManager::projectManager(parent);
}
