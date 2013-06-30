#include "newgenevariablestoolbox.h"

#include <QLayout>

NewGeneVariablesToolbox::NewGeneVariablesToolbox( QWidget * parent ) :
	QToolBox( parent ),
	NewGeneWidget( WidgetCreationInfo(this, WIDGET_NATURE_OUTPUT_WIDGET, VARIABLE_GROUPS_TOOLBOX, true) ) // 'this' pointer is cast by compiler to proper Widget instance, which is already created due to order in which base classes appear in class definition
{
	PrepareOutputWidget();
	layout()->setSpacing( 1 );
}

void NewGeneVariablesToolbox::UpdateOutputConnections(UIProjectManager::UPDATE_CONNECTIONS_TYPE connection_type, UIOutputProject * project)
{
	NewGeneWidget::UpdateOutputConnections(connection_type, project);
	connect(this, SIGNAL(RefreshWidget(WidgetDataItemRequest_VARIABLE_GROUPS_TOOLBOX)), outp->getConnector(), SLOT(RefreshWidget(WidgetDataItemRequest_VARIABLE_GROUPS_TOOLBOX)));
	connect(project->getConnector(), SIGNAL(WidgetDataRefresh(WidgetDataItem_VARIABLE_GROUPS_TOOLBOX)), this, SLOT(WidgetDataRefreshReceive(WidgetDataItem_VARIABLE_GROUPS_TOOLBOX)));

	// *** Child widget, so refer refresh signals directed at child to be received by us, the parent *** //
	connect(project->getConnector(), SIGNAL(WidgetDataRefresh(WidgetDataItem_VARIABLE_GROUP_VARIABLE_GROUP_INSTANCE)), this, SLOT(WidgetDataRefreshReceive(WidgetDataItem_VARIABLE_GROUP_VARIABLE_GROUP_INSTANCE)));
}

void NewGeneVariablesToolbox::RefreshAllWidgets()
{
	WidgetDataItemRequest_VARIABLE_GROUPS_TOOLBOX request(WIDGET_DATA_ITEM_REQUEST_REASON__REFRESH_ALL_WIDGETS);
	emit RefreshWidget(request);
}

void NewGeneVariablesToolbox::WidgetDataRefreshReceive(WidgetDataItem_VARIABLE_GROUPS_TOOLBOX widget_data)
{

	int nItems = count();
	for (int n = 0; n < nItems; ++n)
	{
		QWidget * child = find(n);
		removeItem(0);
		if (child)
		{
			delete child;
			child = nullptr;
		}
	}

	std::for_each(widget_data.identifiers.cbegin(), widget_data.identifiers.cend(), [&](WidgetInstanceIdentifier const & identifier)
	{
		if (identifier.uuid && identifier.code && identifier.longhand)
		{
			NewGeneVariableGroup * tmpGrp = new NewGeneVariableGroup( this, identifier, outp );
			addItem( tmpGrp, identifier.longhand->c_str() );
		}
	});

}

void NewGeneVariablesToolbox::WidgetDataRefreshReceive(WidgetDataItem_VARIABLE_GROUP_VARIABLE_GROUP_INSTANCE widget_data)
{
	if (widget_data.identifier && widget_data.identifier->uuid)
	{
		NewGeneWidget * child = outp->FindWidgetFromDataItem(*widget_data.identifier->uuid);
		if (child)
		{
			child->WidgetDataRefreshReceive(widget_data);
		}
	}
}
