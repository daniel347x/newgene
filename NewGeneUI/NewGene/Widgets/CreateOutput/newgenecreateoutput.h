#ifndef NEWGENECREATEOUTPUT_H
#define NEWGENECREATEOUTPUT_H

#include <QWidget>
#include "../newgenewidget.h"

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

	public:

        void SetOutputDatasetText();

	signals:

	public slots:
		void UpdateOutputConnections(NewGeneWidget::UPDATE_CONNECTIONS_TYPE connection_type, UIOutputProject * project);

	private:

		Ui::NewGeneCreateOutput * ui;

    protected:

        static QString titleBarBaseText;


};

#endif // NEWGENECREATEOUTPUT_H
