#ifndef NEWGENECREATEOUTPUT_H
#define NEWGENECREATEOUTPUT_H

#include <QWidget>
#include "..\newgenewidget.h"

namespace Ui
{
	class NewGeneCreateOutput;
}

class NewGeneCreateOutput : public QWidget, public NewGeneWidget // do not reorder base classes; QWidget instance must be instantiated first
{
		Q_OBJECT

	public:
		explicit NewGeneCreateOutput( QWidget * parent = 0 );
		~NewGeneCreateOutput();

	protected:
		void changeEvent( QEvent * e );

	private:
		Ui::NewGeneCreateOutput * ui;
};

#endif // NEWGENECREATEOUTPUT_H
