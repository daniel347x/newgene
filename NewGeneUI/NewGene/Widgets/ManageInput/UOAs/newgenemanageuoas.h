#ifndef NEWGENEMANAGEUOAS_H
#define NEWGENEMANAGEUOAS_H

#include <QWidget>
#include <QItemSelection>
#include "../../newgenewidget.h"

namespace Ui
{
	class NewGeneManageUOAs;
}

class NewGeneManageUOAs : public QWidget, public NewGeneWidget // do not reorder base classes; QWidget instance must be instantiated first
{

		Q_OBJECT

	public:

		explicit NewGeneManageUOAs(QWidget * parent = 0);
		~NewGeneManageUOAs();

		void HandleChanges(DataChangeMessage const &);

	protected:

		void changeEvent(QEvent * e);
		void Empty();

	private:

		Ui::NewGeneManageUOAs * ui;

	public:

	signals:
		void RefreshWidget(WidgetDataItemRequest_MANAGE_UOAS_WIDGET);

		// Actions
		void AddUOA(WidgetActionItemRequest_ACTION_ADD_UOA);
		void DeleteUOA(WidgetActionItemRequest_ACTION_DELETE_UOA);

	public slots:
		void UpdateInputConnections(NewGeneWidget::UPDATE_CONNECTIONS_TYPE connection_type, UIInputProject * project);
		void UpdateOutputConnections(NewGeneWidget::UPDATE_CONNECTIONS_TYPE connection_type, UIOutputProject * project);
		void RefreshAllWidgets();
		void WidgetDataRefreshReceive(WidgetDataItem_MANAGE_UOAS_WIDGET);

	private slots:
		void ReceiveUOASelectionChanged(const QItemSelection & selected, const QItemSelection & deselected);
		void on_pushButton_deleteUOA_clicked();
		void on_pushButton_createUOA_clicked();

	protected:

		bool GetSelectedUoaCategory(WidgetInstanceIdentifier & uoa_category, WidgetInstanceIdentifiers & uoa_dmu_categories);

};

#endif // NEWGENEMANAGEUOAS_H
