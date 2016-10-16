#ifndef KADWIDGETSSCROLLAREA_H
#define KADWIDGETSSCROLLAREA_H

#include <QWidget>
#include "../../../newgenewidget.h"

class KadWidgetsScrollArea : public QWidget, public NewGeneWidget
{

		Q_OBJECT

	public:

		explicit KadWidgetsScrollArea(QWidget * parent = 0);

		void HandleChanges(DataChangeMessage const &);
		void EmptyTextCheck();
		void ShowLoading(bool const loading_ = true);
		void Resequence();
		QString getFullWarningText(bool newline);

	signals:

		void RefreshWidget(WidgetDataItemRequest_KAD_SPIN_CONTROLS_AREA);

	public slots:

		void UpdateOutputConnections(NewGeneWidget::UPDATE_CONNECTIONS_TYPE connection_type, UIOutputProject * project);
		void UpdateInputConnections(NewGeneWidget::UPDATE_CONNECTIONS_TYPE connection_type, UIInputProject * project);
		void RefreshAllWidgets();
		void WidgetDataRefreshReceive(WidgetDataItem_KAD_SPIN_CONTROLS_AREA); // us, parent
		void WidgetDataRefreshReceive(WidgetDataItem_KAD_SPIN_CONTROL_WIDGET); // child
		void DoTabChange(WidgetInstanceIdentifier);

	protected:

		void AddKadSpinWidget(WidgetInstanceIdentifier const & identifier, WidgetInstanceIdentifiers const & active_dmus);
		void Empty();
		void resizeEvent(QResizeEvent *);
		void paintEvent(QPaintEvent *);

	private:
		bool loading;
		WidgetInstanceIdentifier cached_active_vg;

};

#endif // KADWIDGETSSCROLLAREA_H
