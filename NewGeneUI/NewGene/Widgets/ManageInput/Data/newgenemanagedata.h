#ifndef NEWGENEMANAGEDATA_H
#define NEWGENEMANAGEDATA_H

#include <QWidget>
#include "../../newgenewidget.h"

namespace Ui
{
	class NewGeneManageData;
}

class NewGeneManageData : public QWidget, public NewGeneWidget // do not reorder base classes; QWidget instance must be instantiated first
{

		Q_OBJECT

	public:

		explicit NewGeneManageData( QWidget * parent = 0 );
		~NewGeneManageData();

	protected:

		void changeEvent( QEvent * e );

	private:

		Ui::NewGeneManageData * ui;

	public:

	signals:

	public slots:

};

#endif // NEWGENEMANAGEDATA_H
