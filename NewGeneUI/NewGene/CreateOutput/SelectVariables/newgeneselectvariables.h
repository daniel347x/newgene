#ifndef NEWGENESELECTVARIABLES_H
#define NEWGENESELECTVARIABLES_H

#include <QWidget>
#include "..\..\newgenewidget.h"

namespace Ui
{
	class NewGeneSelectVariables;
}

class NewGeneSelectVariables : public QWidget, public NewGeneWidget // do not reorder base classes; QWidget instance must be instantiated first
{
		Q_OBJECT

	public:
		explicit NewGeneSelectVariables( QWidget * parent = 0 );
		~NewGeneSelectVariables();

	protected:
		void changeEvent( QEvent * e );

	private:
		Ui::NewGeneSelectVariables * ui;
};

#endif // NEWGENESELECTVARIABLES_H
