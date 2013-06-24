#ifndef NEWGENEVARIABLEGROUP_H
#define NEWGENEVARIABLEGROUP_H

#include <QWidget>
#include "..\..\..\newgenewidget.h"

namespace Ui
{
	class NewGeneVariableGroup;
}

class NewGeneVariableGroup : public QWidget, public NewGeneWidget // do not reorder base classes; QWidget instance must be instantiated first
{
		Q_OBJECT

	public:
		explicit NewGeneVariableGroup( QWidget * parent = 0, DataInstanceIdentifier data_instance = DataInstanceIdentifier() );
		~NewGeneVariableGroup();

	signals:
		void RefreshWidget(WidgetDataItem_VARIABLE_GROUP_VARIABLE_GROUP_INSTANCE);

	public slots:
		void UpdateOutputConnections(UIProjectManager::UPDATE_CONNECTIONS_TYPE connection_type, UIOutputProject * project);
		void RefreshAllWidgets();
		void WidgetDataRefreshReceive(WidgetDataItem_VARIABLE_GROUP_VARIABLE_GROUP_INSTANCE);

	protected:
		void changeEvent( QEvent * e );

	private:
		Ui::NewGeneVariableGroup * ui;
};

#endif // NEWGENEVARIABLEGROUP_H
