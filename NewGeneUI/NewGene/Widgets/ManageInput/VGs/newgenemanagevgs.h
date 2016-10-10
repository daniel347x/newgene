#ifndef NEWGENEMANAGEVGS_H
#define NEWGENEMANAGEVGS_H

#include <QWidget>
#include <QItemSelection>
#include "../../newgenewidget.h"

namespace Ui
{
	class NewGeneManageVGs;
}

class NewGeneManageVGs : public QWidget, public NewGeneWidget // do not reorder base classes; QWidget instance must be instantiated first
{

		Q_OBJECT

	public:

		explicit NewGeneManageVGs(QWidget * parent = 0);
		~NewGeneManageVGs();

		bool event(QEvent * e);

		void HandleChanges(DataChangeMessage const &);

	protected:

		void changeEvent(QEvent * e);
		void Empty();

	private:

		Ui::NewGeneManageVGs * ui;

	public:

	signals:
		void RefreshWidget(WidgetDataItemRequest_MANAGE_VGS_WIDGET);

		// Actions
		void CreateVG(WidgetActionItemRequest_ACTION_CREATE_VG);
		void DeleteVG(WidgetActionItemRequest_ACTION_DELETE_VG);
		void SetVGDescriptions(WidgetActionItemRequest_ACTION_SET_VG_DESCRIPTIONS);
		void RefreshVG(WidgetActionItemRequest_ACTION_REFRESH_VG);

	public slots:
		void UpdateInputConnections(NewGeneWidget::UPDATE_CONNECTIONS_TYPE connection_type, UIInputProject * project);
		void UpdateOutputConnections(NewGeneWidget::UPDATE_CONNECTIONS_TYPE connection_type, UIOutputProject * project);
		void RefreshAllWidgets();
		void WidgetDataRefreshReceive(WidgetDataItem_MANAGE_VGS_WIDGET);
		void UpdateVGImportProgressBar(int mode_, int min_, int max_, int val_);

	protected:

		bool GetSelectedVG(WidgetInstanceIdentifier & vg, WidgetInstanceIdentifier & uoa);

	private slots:
		void ReceiveVGSelectionChanged(const QItemSelection & selected, const QItemSelection & deselected);
		void on_pushButton_add_vg_clicked();
		void on_pushButton_remove_vg_clicked();
		void on_pushButton_refresh_vg_clicked();
		void on_pushButton_cancel_clicked();
		void on_pushButton_set_descriptions_for_vg_clicked();

private:
		bool refresh_vg_called_after_create;

};

#endif // NEWGENEMANAGEVGS_H
