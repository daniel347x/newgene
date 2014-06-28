#ifndef NEWGENEVARIABLESTOOLBOX_H
#define NEWGENEVARIABLESTOOLBOX_H

#include <QToolBox>
#include "../../../newgenewidget.h"
#include "newgenevariablegroup.h"

class NewGeneVariablesToolbox : public QToolBox, public NewGeneWidget // do not reorder base classes; QWidget instance must be instantiated first
{

		Q_OBJECT

	public:

		explicit NewGeneVariablesToolbox( QWidget * parent = 0 );

		void HandleChanges(DataChangeMessage const &);

	signals:

		void RefreshWidget(WidgetDataItemRequest_VARIABLE_GROUPS_TOOLBOX);

	public slots:

		void UpdateOutputConnections(NewGeneWidget::UPDATE_CONNECTIONS_TYPE connection_type, UIOutputProject * project);
		void UpdateInputConnections(NewGeneWidget::UPDATE_CONNECTIONS_TYPE connection_type, UIInputProject * project);
		void RefreshAllWidgets();
		void WidgetDataRefreshReceive(WidgetDataItem_VARIABLE_GROUPS_TOOLBOX); // us, parent
		void WidgetDataRefreshReceive(WidgetDataItem_VARIABLE_GROUP_VARIABLE_GROUP_INSTANCE); // child

        // For Squish only: exception to rule regarding slots; return something here
        QListView * GetActiveListView(int const index);

    private:

		NewGeneVariableGroup * groups;

	protected:

		void Empty();

};

#endif // NEWGENEVARIABLESTOOLBOX_H
