#ifndef NEWGENEMANAGEINPUT_H
#define NEWGENEMANAGEINPUT_H

#include <QWidget>
#include "../newgenewidget.h"

namespace Ui
{
	class NewGeneManageInput;
}

class NewGeneManageInput : public QWidget, public NewGeneWidget // do not reorder base classes; QWidget instance must be instantiated first
{

		Q_OBJECT

	public:

		explicit NewGeneManageInput( QWidget * parent = 0 );
		~NewGeneManageInput();

	protected:

		void changeEvent( QEvent * e );

	public:

	signals:

	public slots:
		void UpdateInputConnections(NewGeneWidget::UPDATE_CONNECTIONS_TYPE connection_type, UIInputProject * project);

	private:

		Ui::NewGeneManageInput * ui;

};

#endif // NEWGENEMANAGEINPUT_H
