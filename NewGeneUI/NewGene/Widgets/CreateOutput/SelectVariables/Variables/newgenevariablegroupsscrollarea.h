#ifndef NEWGENEVARIABLEGROUPSSCROLLAREA_H
#define NEWGENEVARIABLEGROUPSSCROLLAREA_H

#include <QWidget>
#include "../../../newgenewidget.h"

namespace Ui
{
	class NewGeneVariableGroupsScrollArea;
}

class NewGeneVariableGroupsScrollArea : public QWidget, public NewGeneWidget // do not reorder base classes; QWidget instance must be instantiated first
{
		Q_OBJECT

	public:
		explicit NewGeneVariableGroupsScrollArea(QWidget * parent = 0);
		~NewGeneVariableGroupsScrollArea();

	signals:
		void TestSignal();
		void RefreshWidget(WidgetDataItemRequest_VARIABLE_GROUPS_SCROLL_AREA);

	public slots:
		void UpdateInputConnections(NewGeneWidget::UPDATE_CONNECTIONS_TYPE connection_type, UIInputProject * project);
		void UpdateOutputConnections(NewGeneWidget::UPDATE_CONNECTIONS_TYPE connection_type, UIOutputProject * project);
		void TestSlot();
		void RefreshAllWidgets();
		void WidgetDataRefreshReceive(WidgetDataItem_VARIABLE_GROUPS_SCROLL_AREA);

	protected:
		void changeEvent(QEvent * e);

	private:
		Ui::NewGeneVariableGroupsScrollArea * ui;

	public:

		friend class NewGeneMainWindow;

};

#endif // NEWGENEVARIABLEGROUPSSCROLLAREA_H
