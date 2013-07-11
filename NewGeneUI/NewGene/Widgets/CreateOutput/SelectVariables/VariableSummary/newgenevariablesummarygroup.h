#ifndef NEWGENEVARIABLESUMMARYGROUP_H
#define NEWGENEVARIABLESUMMARYGROUP_H

#include <QGroupBox>
#include "..\..\..\newgenewidget.h"

class QStandardItem;

namespace Ui
{
	class NewGeneVariableSummaryGroup;
}

class NewGeneVariableSummaryGroup : public QGroupBox, public NewGeneWidget // do not reorder base classes; QWidget instance must be instantiated first
{
		Q_OBJECT

	public:

		explicit NewGeneVariableSummaryGroup( QWidget * parent = 0, WidgetInstanceIdentifier data_instance = WidgetInstanceIdentifier(), UIOutputProject * project = nullptr );
		~NewGeneVariableSummaryGroup();

		void HandleChanges(DataChangeMessage const &);

	signals:

		void RefreshWidget(WidgetDataItemRequest_VARIABLE_GROUPS_SUMMARY_VARIABLE_GROUP_INSTANCE);
		void SignalReceiveVariableItemChanged(WidgetActionItemRequest_ACTION_VARIABLE_GROUP_SET_MEMBER_SELECTION_CHANGED);

	public slots:

		void UpdateOutputConnections(UIProjectManager::UPDATE_CONNECTIONS_TYPE connection_type, UIOutputProject * project);
		void RefreshAllWidgets();
		void WidgetDataRefreshReceive(WidgetDataItem_VARIABLE_GROUPS_SUMMARY_VARIABLE_GROUP_INSTANCE);
		void ReceiveVariableItemChanged(QStandardItem*);

	protected:

		void changeEvent( QEvent * e );

	private:

		Ui::NewGeneVariableSummaryGroup * ui;

};

#endif // NEWGENEVARIABLESUMMARYGROUP_H
