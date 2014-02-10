#ifndef NEWGENEMANAGEUOAS_H
#define NEWGENEMANAGEUOAS_H

#include <QWidget>
#include "../../newgenewidget.h"

namespace Ui
{
	class NewGeneManageUOAs;
}

class NewGeneManageUOAs : public QWidget, public NewGeneWidget // do not reorder base classes; QWidget instance must be instantiated first
{

		Q_OBJECT

	public:

		explicit NewGeneManageUOAs( QWidget * parent = 0 );
		~NewGeneManageUOAs();

	protected:

		void changeEvent( QEvent * e );

	private:

		Ui::NewGeneManageUOAs * ui;

	public:

	signals:

	public slots:

};

#endif // NEWGENEMANAGEUOAS_H
