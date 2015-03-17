#ifndef NEWGENEDATETIMEWIDGET_H
#define NEWGENEDATETIMEWIDGET_H

#include <QDateTimeEdit>
#include "../../../newgenewidget.h"

class NewGeneDateTimeWidget : public QDateTimeEdit, public NewGeneWidget
{
		Q_OBJECT

	public:

		explicit NewGeneDateTimeWidget(QWidget * parent = 0, WidgetInstanceIdentifier data_instance = WidgetInstanceIdentifier(), UIOutputProject * project = nullptr);
		~NewGeneDateTimeWidget();

		void HandleChanges(DataChangeMessage const &);

	signals:

		void RefreshWidget(WidgetDataItemRequest_DATETIME_WIDGET);
		void SignalReceiveVariableItemChanged(WidgetActionItemRequest_ACTION_DATETIME_RANGE_CHANGE);

	public slots:

		void UpdateOutputConnections(NewGeneWidget::UPDATE_CONNECTIONS_TYPE connection_type, UIOutputProject * project);
		void UpdateInputConnections(NewGeneWidget::UPDATE_CONNECTIONS_TYPE connection_type, UIInputProject * project);
		void RefreshAllWidgets();
		void WidgetDataRefreshReceive(WidgetDataItem_DATETIME_WIDGET);

		// *********************************//
		// Directly receive the OS-level event
		// *********************************//
		void ReceiveVariableItemChanged(QDateTime const &);

};

#endif // NEWGENEDATETIMEWIDGET_H
