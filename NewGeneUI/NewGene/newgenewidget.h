#ifndef NEWGENEWIDGET_H
#define NEWGENEWIDGET_H

#include "globals.h"

class QWidget;
class NewGeneMainWindow;
class UISettingsManager;
class UIProjectManager;
class UILoggingManager;

class NewGeneWidget
{
	public:
		explicit NewGeneWidget( QWidget * self_ = 0 );

		NewGeneMainWindow & mainWindow();
		UIProjectManager & projectManager();
		UISettingsManager & settingsManager();
		UILoggingManager & loggingManager();

	private:
		QWidget * self;

};

#endif // NEWGENEWIDGET_H
