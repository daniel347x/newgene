#include "newgenevariablegroupsscrollarea.h"
#include "ui_newgenevariablegroupsscrollarea.h"

NewGeneVariableGroupsScrollArea::NewGeneVariableGroupsScrollArea( QWidget * parent ) :
	QWidget( parent ),
	NewGeneWidget( WidgetCreationInfo(this, WIDGET_NATURE_OUTPUT_WIDGET, VARIABLE_GROUPS_SCROLL_AREA) ), // 'this' pointer is cast by compiler to proper Widget instance, which is already created due to order in which base classes appear in class definition
	ui( new Ui::NewGeneVariableGroupsScrollArea )
{
	PrepareOutputWidget();
	ui->setupUi( this );
}

NewGeneVariableGroupsScrollArea::~NewGeneVariableGroupsScrollArea()
{
	delete ui;
}

void NewGeneVariableGroupsScrollArea::changeEvent( QEvent * e )
{
	QWidget::changeEvent( e );

	switch ( e->type() )
	{
		case QEvent::LanguageChange:
			ui->retranslateUi( this );
			break;

		default:
			break;
	}
}

void NewGeneVariableGroupsScrollArea::UpdateInputConnections(UIProjectManager::UPDATE_CONNECTIONS_TYPE connection_type, UIInputProject * project)
{
	NewGeneWidget::UpdateInputConnections(connection_type, project);
}

void NewGeneVariableGroupsScrollArea::UpdateOutputConnections(UIProjectManager::UPDATE_CONNECTIONS_TYPE connection_type, UIOutputProject * project)
{
	NewGeneWidget::UpdateOutputConnections(connection_type, project);
	if (connection_type == UIProjectManager::ESTABLISH_CONNECTIONS_OUTPUT_PROJECT)
	{
		connect(this, SIGNAL(RefreshWidget(WidgetDataItemRequest_VARIABLE_GROUPS_SCROLL_AREA)), outp->getConnector(), SLOT(RefreshWidget(WidgetDataItemRequest_VARIABLE_GROUPS_SCROLL_AREA)));
		connect(project->getConnector(), SIGNAL(WidgetDataRefresh(WidgetDataItem_VARIABLE_GROUPS_SCROLL_AREA)), this, SLOT(WidgetDataRefreshReceive(WidgetDataItem_VARIABLE_GROUPS_SCROLL_AREA)));
	}
}

void NewGeneVariableGroupsScrollArea::TestSlot()
{
}

void NewGeneVariableGroupsScrollArea::WidgetDataRefreshReceive(WidgetDataItem_VARIABLE_GROUPS_SCROLL_AREA widget_data)
{
}

void NewGeneVariableGroupsScrollArea::RefreshAllWidgets()
{
	emit RefreshWidget(WidgetDataItemRequest_VARIABLE_GROUPS_SCROLL_AREA(WIDGET_DATA_ITEM_REQUEST_REASON__REFRESH_ALL_WIDGETS));
}
