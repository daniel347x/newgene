#ifndef KADSPINBOX_H
#define KADSPINBOX_H

#include <QSpinBox>
#include "../../../newgenewidget.h"

class KadSpinBox : public QSpinBox, public NewGeneWidget // do not reorder base classes; QWidget instance must be instantiated first
{

		Q_OBJECT

	public:

		explicit KadSpinBox( QWidget *parent = 0, WidgetInstanceIdentifier data_instance = WidgetInstanceIdentifier(), UIOutputProject * project = nullptr );
		~KadSpinBox();

		void HandleChanges(DataChangeMessage const &);

	signals:

		void RefreshWidget(WidgetDataItemRequest_KAD_SPIN_CONTROL_WIDGET);
		void SignalReceiveVariableItemChanged(WidgetActionItemRequest_ACTION_KAD_COUNT_CHANGE);

	public slots:

		void UpdateOutputConnections(NewGeneWidget::UPDATE_CONNECTIONS_TYPE connection_type, UIOutputProject * project);
		void RefreshAllWidgets();
		void WidgetDataRefreshReceive(WidgetDataItem_KAD_SPIN_CONTROL_WIDGET);

		// *********************************//
		// Directly receive the OS-level event
		// *********************************//
		void ReceiveVariableItemChanged(int);

};

#endif // KADSPINBOX_H
