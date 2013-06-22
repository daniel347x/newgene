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
}

void NewGeneVariablesToolbox::RefreshAllWidgets()
{
	WidgetDataItemRequest_VARIABLE_GROUPS_TOOLBOX request(WIDGET_DATA_ITEM_REQUEST_REASON__REFRESH_ALL_WIDGETS);
	request.s = "And another custom group!!!";
	emit RefreshWidget(request);
}

void NewGeneVariablesToolbox::WidgetDataRefreshReceive(WidgetDataItem_VARIABLE_GROUPS_TOOLBOX widget_data)
{
	int nItems = count();
	for (int n = 0; n < nItems; ++n)
	{
		removeItem(0);
	}

	std::for_each(widget_data.variable_group_long_names.cbegin(), widget_data.variable_group_long_names.cend(), [&](std::string const & variable_group_long_name)
	{
		NewGeneVariableGroup * tmpGrp = new NewGeneVariableGroup( this );
		addItem( tmpGrp, variable_group_long_name.c_str() );
	});

//	groups = new NewGeneVariableGroup( this );
//	addItem( groups, "Country Variables" );

//	NewGeneVariableGroup * tmpGrp = new NewGeneVariableGroup( this );
//	addItem( tmpGrp, "MID Detail Variables" );
}
