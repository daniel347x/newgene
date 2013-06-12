#ifndef NEWGENEWIDGETTEST_H
#define NEWGENEWIDGETTEST_H

#include "globals.h"

class QWidget;
class NewGeneMainWindow;

class NewGeneWidgetTest : public QObject
{

		Q_OBJECT

	public:
		explicit NewGeneWidgetTest( QWidget * self_ = 0, QObject * parent = nullptr )
			: QObject(parent)
			, self(self_)
		{

		}

		NewGeneMainWindow & mainWindow();


	signals:

	public slots:


	private:
		QWidget * self;

	public:

		static NewGeneMainWindow * theMainWindow;

};

#endif // NEWGENEWIDGET_H
