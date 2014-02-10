#ifndef DISPLAYDMUSREGION_H
#define DISPLAYDMUSREGION_H

#include <QWidget>
#include <QItemSelection>
#include "../../newgenewidget.h"

namespace Ui
{
	class DisplayDMUsRegion;
}

class DisplayDMUsRegion : public QWidget, public NewGeneWidget // do not reorder base classes; QWidget instance must be instantiated first
{

		Q_OBJECT

	public:

		explicit DisplayDMUsRegion(QWidget *parent = 0);
		~DisplayDMUsRegion();

	protected:

		void changeEvent( QEvent * e );

	private:

		Ui::DisplayDMUsRegion *ui;

	signals:
		void RefreshWidget(WidgetDataItemRequest_MANAGE_DMUS_WIDGET);

	public slots:
		void UpdateInputConnections(NewGeneWidget::UPDATE_CONNECTIONS_TYPE connection_type, UIInputProject * project);
		void RefreshAllWidgets();
		void WidgetDataRefreshReceive(WidgetDataItem_MANAGE_DMUS_WIDGET);

	private slots:
		void ReceiveVariableItemChanged(const QItemSelection & selected, const QItemSelection & deselected);

	protected:
		void Empty();

};

#endif // DISPLAYDMUSREGION_H
