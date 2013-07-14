#ifndef KADSPINBOX_H
#define KADSPINBOX_H

#include <QSpinBox>
#include "..\..\..\newgenewidget.h"

class KadSpinBox : public QSpinBox, public NewGeneWidget // do not reorder base classes; QWidget instance must be instantiated first
{

		Q_OBJECT

	public:

		explicit KadSpinBox( QWidget *parent = 0, WidgetInstanceIdentifier data_instance = WidgetInstanceIdentifier(), UIOutputProject * project = nullptr );

	signals:

		void RefreshWidget(WidgetDataItemRequest_KAD_SPIN_CONTROL_WIDGET);

	public slots:

		void UpdateOutputConnections(UIProjectManager::UPDATE_CONNECTIONS_TYPE connection_type, UIOutputProject * project);
		void RefreshAllWidgets();
		void WidgetDataRefreshReceive(WidgetDataItem_KAD_SPIN_CONTROL_WIDGET);
		void ReceiveVariableItemChanged(int);

};

#endif // KADSPINBOX_H
