#ifndef NEWGENEVARIABLESUMMARYGROUP_H
#define NEWGENEVARIABLESUMMARYGROUP_H

#include <QGroupBox>
#include "..\..\..\newgenewidget.h"

namespace Ui
{
	class NewGeneVariableSummaryGroup;
}

class NewGeneVariableSummaryGroup : public QGroupBox, public NewGeneWidget // do not reorder base classes; QWidget instance must be instantiated first
{
		Q_OBJECT

	public:
		explicit NewGeneVariableSummaryGroup( QWidget * parent = 0 );
		~NewGeneVariableSummaryGroup();

	protected:
		void changeEvent( QEvent * e );

	private:
		Ui::NewGeneVariableSummaryGroup * ui;
};

#endif // NEWGENEVARIABLESUMMARYGROUP_H
