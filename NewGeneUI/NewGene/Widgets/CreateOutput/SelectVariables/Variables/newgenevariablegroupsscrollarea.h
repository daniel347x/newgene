#ifndef NEWGENEVARIABLEGROUPSSCROLLAREA_H
#define NEWGENEVARIABLEGROUPSSCROLLAREA_H

#include <QWidget>
#include "..\..\..\newgenewidget.h"

namespace Ui
{
	class NewGeneVariableGroupsScrollArea;
}

class NewGeneVariableGroupsScrollArea : public QWidget, public NewGeneWidget // do not reorder base classes; QWidget instance must be instantiated first
{
		Q_OBJECT

	public:
		explicit NewGeneVariableGroupsScrollArea( QWidget * parent = 0 );
		~NewGeneVariableGroupsScrollArea();

	signals:
		void TestSignal();

	public slots:
		void UpdateOutputConnections(UIProjectManager::UPDATE_CONNECTIONS_TYPE connection_type, UIOutputProject * project);
		void TestSlot();

	protected:
		void changeEvent( QEvent * e );

	private:
		Ui::NewGeneVariableGroupsScrollArea * ui;
};

#endif // NEWGENEVARIABLEGROUPSSCROLLAREA_H
