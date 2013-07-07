#include "newgenevariablestoolbox.h"

#include <QLayout>

NewGeneVariablesToolbox::NewGeneVariablesToolbox( QWidget * parent ) :
	QToolBox( parent ),
	NewGeneWidget( WidgetCreationInfo(this, WIDGET_NATURE_OUTPUT_WIDGET, VARIABLE_GROUPS_TOOLBOX, true) ) // 'this' pointer is cast by compiler to proper Widget instance, which is already created due to order in which base classes appear in class definition
{
	layout()->setSpacing( 1 );
	PrepareOutputWidget();
}

void NewGeneVariablesToolbox::UpdateOutputConnections(UIProjectManager::UPDATE_CONNECTIONS_TYPE connection_type, UIOutputProject * project)
{
	NewGeneWidget::UpdateOutputConnections(connection_type, project);
	connect(this, SIGNAL(RefreshWidget(WidgetDataItemRequest_VARIABLE_GROUPS_TOOLBOX)), outp->getConnector(), SLOT(RefreshWidget(WidgetDataItemRequest_VARIABLE_GROUPS_TOOLBOX)));

	// *** This is a parent (top-level) widget, so connect to refreshes here (... child widgets don't connect to refreshes) *** //
	connect(project->getConnector(), SIGNAL(WidgetDataRefresh(WidgetDataItem_VARIABLE_GROUPS_TOOLBOX)), this, SLOT(WidgetDataRefreshReceive(WidgetDataItem_VARIABLE_GROUPS_TOOLBOX)));

	// *** Has child widgets, so refer refresh signals directed at child to be received by us, the parent *** //
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
		removeItem(n);
		if (child)
		{
			delete child;
			child = nullptr;
		}
		--n;
		--nItems;
	}

	std::for_each(widget_data.identifiers.cbegin(), widget_data.identifiers.cend(), [&](WidgetInstanceIdentifier const & identifier)
	{
		if (identifier.uuid && identifier.code && identifier.longhand)
		{
			WidgetInstanceIdentifier new_identifier(identifier);
			new_identifier.uuid_parent = std::make_shared<UUID>(uuid);
			NewGeneVariableGroup * tmpGrp = new NewGeneVariableGroup( this, new_identifier, outp );
			addItem( tmpGrp, identifier.longhand->c_str() );
		}
	});

}

void NewGeneVariablesToolbox::WidgetDataRefreshReceive(WidgetDataItem_VARIABLE_GROUP_VARIABLE_GROUP_INSTANCE widget_data)
{
	if (widget_data.identifier && widget_data.identifier->uuid)
	{
		NewGeneWidget * child = outp->FindWidgetFromDataItem(uuid, *widget_data.identifier->uuid);
		if (child)
		{
			child->WidgetDataRefreshReceive(widget_data);
		}
	}
}
