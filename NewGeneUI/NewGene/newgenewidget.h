#ifndef NEWGENEWIDGET_H
#define NEWGENEWIDGET_H

#include "globals.h"
#include "..\..\NewGeneBackEnd\Utilities\NewGeneException.h"

class QWidget;
class NewGeneMainWindow;
class UISettingsManager;

class NewGeneWidget
{
public:
	explicit NewGeneWidget(QWidget * self_ = 0);

	NewGeneMainWindow & mainWindow();
	UISettingsManager & settingsManager(NewGeneMainWindow * parent = NULL);

private:
	QWidget * self;

};

#endif // NEWGENEWIDGET_H
