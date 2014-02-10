#ifndef NEWGENEMANAGEVGS_H
#define NEWGENEMANAGEVGS_H

#include <QWidget>
#include "../../newgenewidget.h"

namespace Ui
{
	class NewGeneManageVGs;
}

class NewGeneManageVGs : public QWidget, public NewGeneWidget // do not reorder base classes; QWidget instance must be instantiated first
{

		Q_OBJECT

	public:

		explicit NewGeneManageVGs( QWidget * parent = 0 );
		~NewGeneManageVGs();

	protected:

		void changeEvent( QEvent * e );

	private:

		Ui::NewGeneManageVGs * ui;

	public:

	signals:

	public slots:

};

#endif // NEWGENEMANAGEVGS_H
