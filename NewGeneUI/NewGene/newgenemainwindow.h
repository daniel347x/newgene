#ifndef NEWGENEMAINWINDOW_H
#define NEWGENEMAINWINDOW_H

#include "globals.h"
#include <QMainWindow>
#include "newgenewidget.h"

#include <memory>

class UIProject;

namespace Ui
{
	class NewGeneMainWindow;
}

class NewGeneMainWindow : public QMainWindow, public NewGeneWidget // do not reorder base classes; QWidget instance must be instantiated first
{
		Q_OBJECT

	public:
		explicit NewGeneMainWindow( QWidget * parent = 0 );
		~NewGeneMainWindow();

	signals:

	public slots:
		void doInitialize();

	protected:
		void changeEvent( QEvent * e );

	private:
		Ui::NewGeneMainWindow * ui;

		UIProject * project; // owned by this NewGeneMainWindow

		friend class NewGeneWidget; // saved using Dropbox + SyncBack Pro - and a second time.  Now, editing it on the Xeon machine.
};

#endif // NEWGENEMAINWINDOW_H
