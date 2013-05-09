#ifndef NEWGENEWIDGET_H
#define NEWGENEWIDGET_H

#include "globals.h"

class QWidget;
class NewGeneMainWindow;
class UISettingsManager;
class UIProjectManager;

class NewGeneWidget
{
public:
	explicit NewGeneWidget(QWidget * self_ = 0);

	NewGeneMainWindow & mainWindow();
	UIProjectManager & projectManager(NewGeneMainWindow * parent = NULL);
	UISettingsManager & settingsManager(NewGeneMainWindow * parent = NULL);

private:
	QWidget * self;

};

#endif // NEWGENEWIDGET_H
