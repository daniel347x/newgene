#ifndef NEWGENEVARIABLESTOOLBOX_H
#define NEWGENEVARIABLESTOOLBOX_H

#include <QToolBox>
#include "..\..\..\newgenewidget.h"
#include "newgenevariablegroup.h"

class NewGeneVariablesToolbox : public QToolBox, public NewGeneWidget // do not reorder base classes; QWidget instance must be instantiated first
{
		Q_OBJECT
	public:
		explicit NewGeneVariablesToolbox( QWidget * parent = 0 );

	signals:
		void RefreshWidget(DATA_WIDGETS);

	public slots:
		void UpdateOutputConnections(UIProjectManager::UPDATE_CONNECTIONS_TYPE connection_type, UIOutputProject * project);
		void RefreshAllWidgets();
		void WidgetDataRefreshReceive(WidgetDataItem_VARIABLE_GROUPS_TOOLBOX);

	private:
		NewGeneVariableGroup * groups;

};

#endif // NEWGENEVARIABLESTOOLBOX_H
