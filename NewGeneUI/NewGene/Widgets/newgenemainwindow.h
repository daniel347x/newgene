#ifndef NEWGENEMAINWINDOW_H
#define NEWGENEMAINWINDOW_H

#include "globals.h"
#include <QMainWindow>
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
		void ReceiveSignalStartProgressBar(STD_STRING const, STD_STRING const);
		void ReceiveSignalStopProgressBar();
		void ReceiveSignalUpdateProgressBarValue(std::int64_t const);
		void ReceiveSignalUpdateStatusBarText(STD_STRING const);

	protected:
		void changeEvent( QEvent * e );

	private:
		Ui::NewGeneMainWindow * ui;

		friend class NewGeneWidget;
};

#endif // NEWGENEMAINWINDOW_H
