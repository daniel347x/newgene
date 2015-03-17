#ifndef NEWGENEMANAGEDMUS_H
#define NEWGENEMANAGEDMUS_H

#include <QWidget>
#include "../../newgenewidget.h"

namespace Ui
{
	class NewGeneManageDMUs;
}

class NewGeneManageDMUs : public QWidget, public NewGeneWidget // do not reorder base classes; QWidget instance must be instantiated first
{

		Q_OBJECT

	public:

		explicit NewGeneManageDMUs(QWidget * parent = 0);
		~NewGeneManageDMUs();

	protected:

		void changeEvent(QEvent * e);

	private:

		Ui::NewGeneManageDMUs * ui;

	public:

	signals:

	public slots:

};

#endif // NEWGENEMANAGEDMUS_H
