#ifndef NEWGENEVARIABLEGROUP_H
#define NEWGENEVARIABLEGROUP_H

#include <QWidget>
#include "../../../newgenewidget.h"
#include <string>

class QStandardItem;
class QListView;
class NewGeneVariablesToolbox;
class QStandardItemModel;

namespace Ui
{
	class NewGeneVariableGroup;
}

class NewGeneVariableGroup : public QWidget, public NewGeneWidget // do not reorder base classes; QWidget instance must be instantiated first
{
		Q_OBJECT

	public:
		explicit NewGeneVariableGroup(NewGeneVariablesToolbox * toolbox_, QWidget * parent = 0, WidgetInstanceIdentifier data_instance = WidgetInstanceIdentifier(),
									  UIOutputProject * project = nullptr);
		~NewGeneVariableGroup();

		void HandleChanges(DataChangeMessage const &);

	signals:
		void RefreshWidget(WidgetDataItemRequest_VARIABLE_GROUP_VARIABLE_GROUP_INSTANCE);
		void SignalReceiveVariableItemChanged(WidgetActionItemRequest_ACTION_VARIABLE_GROUP_SET_MEMBER_SELECTION_CHANGED);

	public slots:
		void UpdateInputConnections(NewGeneWidget::UPDATE_CONNECTIONS_TYPE connection_type, UIInputProject * project) {}
		void UpdateOutputConnections(NewGeneWidget::UPDATE_CONNECTIONS_TYPE connection_type, UIOutputProject * project);
		void RefreshAllWidgets();
		void WidgetDataRefreshReceive(WidgetDataItem_VARIABLE_GROUP_VARIABLE_GROUP_INSTANCE);
		void ReceiveVariableItemChanged(QStandardItem *);
		void enableUpdates();

		// For Squish only: exception to rule regarding slots; return something here
		QListView * GetListView();

	protected:
		void changeEvent(QEvent * e);
		bool ResetAll(std::vector<std::pair<WidgetInstanceIdentifier, bool>> const & vg_members_and_bools);
		void SetEnabledStateSelectAllButtons(QStandardItemModel *);

	private slots:
		void on_toolButtonSelectAll_clicked();
		void on_toolButtonDeselectAll_clicked();

	private:
		NewGeneVariablesToolbox * toolbox;
		Ui::NewGeneVariableGroup * ui;

	public:
		bool hasChecked();

	public:
		static int const bottomMargin;
		static std::string const activeTabColor;
		static std::string const inactiveTabColor;

};

#endif // NEWGENEVARIABLEGROUP_H
