#ifndef NEWGENEVARIABLESUMMARY_H
#define NEWGENEVARIABLESUMMARY_H

#include <QWidget>
#include "..\..\..\newgenewidget.h"
#include <QStringListModel>

namespace Ui
{
	class NewGeneVariableSummary;
}

class NewGeneVariableSummary : public QWidget, public NewGeneWidget // do not reorder base classes; QWidget instance must be instantiated first
{
		Q_OBJECT

	public:
		explicit NewGeneVariableSummary( QWidget * parent = 0 );
		~NewGeneVariableSummary();

	public slots:

	protected:
		void changeEvent( QEvent * e );

	private:
		Ui::NewGeneVariableSummary * ui;

	protected:
		QStringListModel model1;
		QStringListModel model2;
		QStringListModel model3;
		QStringListModel model4;
		QStringListModel model5;
};

#endif // NEWGENEVARIABLESUMMARY_H
