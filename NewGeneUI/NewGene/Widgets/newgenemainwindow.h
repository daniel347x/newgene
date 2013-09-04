#ifndef NEWGENEMAINWINDOW_H
#define NEWGENEMAINWINDOW_H

#include "globals.h"
#include <QMainWindow>
#include <QProgressBar>
#include <QStatusBar>
#include "newgenewidget.h"

#include <memory>

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
		void SignalMessageBox(STD_STRING);
		void ReceiveSignalStartProgressBar(int, std::int64_t const, std::int64_t const);
		void ReceiveSignalStopProgressBar(int);
		void ReceiveSignalUpdateProgressBarValue(int, std::int64_t const);
		void ReceiveSignalUpdateStatusBarText(int, STD_STRING const);

	protected:
		void changeEvent( QEvent * e );

	private:
		Ui::NewGeneMainWindow * ui;

		friend class NewGeneWidget;


};

#endif // NEWGENEMAINWINDOW_H
