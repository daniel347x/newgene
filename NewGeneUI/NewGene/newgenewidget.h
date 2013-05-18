#ifndef NEWGENEWIDGET_H
#define NEWGENEWIDGET_H

#include "globals.h"

class QWidget;
class NewGeneMainWindow;

class NewGeneWidget
{
	public:
		explicit NewGeneWidget( QWidget * self_ = 0 );

		NewGeneMainWindow & mainWindow();

	private:
		QWidget * self;

};

#endif // NEWGENEWIDGET_H
