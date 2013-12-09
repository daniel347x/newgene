#ifndef NEWGENEVARIABLES_H
#define NEWGENEVARIABLES_H

#include <QWidget>
#include "../../../newgenewidget.h"

namespace Ui
{
	class NewGeneVariables;
}

class NewGeneVariables : public QWidget, public NewGeneWidget // do not reorder base classes; QWidget instance must be instantiated first
{
		Q_OBJECT

	public:
		explicit NewGeneVariables( QWidget * parent = 0 );
		~NewGeneVariables();

	protected:
		void changeEvent( QEvent * e );

	private:
		Ui::NewGeneVariables * ui;
};

#endif // NEWGENEVARIABLES_H
