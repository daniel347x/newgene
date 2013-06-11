#ifndef NEWGENEVARIABLESUMMARYSCROLLAREA_H
#define NEWGENEVARIABLESUMMARYSCROLLAREA_H

#include <QWidget>
#include "..\..\..\newgenewidget.h"
#include "NewGeneVariableSummaryGroup.h"

namespace Ui
{
	class NewGeneVariableSummaryScrollArea;
}

class NewGeneVariableSummaryScrollArea : public QWidget, public NewGeneWidget // do not reorder base classes; QWidget instance must be instantiated first
{
		Q_OBJECT

	public:
		explicit NewGeneVariableSummaryScrollArea( QWidget * parent = 0 );
		~NewGeneVariableSummaryScrollArea();

	protected:
		void changeEvent( QEvent * e );

	private:
		Ui::NewGeneVariableSummaryScrollArea * ui;

	private:
		NewGeneVariableSummaryGroup * groups;

};

#endif // NEWGENEVARIABLESUMMARYSCROLLAREA_H
