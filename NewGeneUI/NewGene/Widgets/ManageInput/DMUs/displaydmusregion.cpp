#include "displaydmusregion.h"
#include "ui_displaydmusregion.h"

#include "../Project/uiprojectmanager.h"
#include "../Project/uiinputproject.h"

DisplayDMUsRegion::DisplayDMUsRegion(QWidget *parent) :
	QWidget(parent),
	NewGeneWidget( WidgetCreationInfo(this, parent, WIDGET_NATURE_INPUT_WIDGET, MANAGE_DMUS_WIDGET, true) ), // 'this' pointer is cast by compiler to proper Widget instance, which is already created due to order in which base classes appear in class definition
	ui(new Ui::DisplayDMUsRegion)
{

	ui->setupUi(this);

	PrepareInputWidget();

}

DisplayDMUsRegion::~DisplayDMUsRegion()
{
	delete ui;
}

void DisplayDMUsRegion::changeEvent( QEvent * e )
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

void DisplayDMUsRegion::UpdateInputConnections(NewGeneWidget::UPDATE_CONNECTIONS_TYPE connection_type, UIInputProject * project)
{

	NewGeneWidget::UpdateInputConnections(connection_type, project);

	if (connection_type == NewGeneWidget::ESTABLISH_CONNECTIONS_INPUT_PROJECT)
	{
		connect(this, SIGNAL(RefreshWidget(WidgetDataItemRequest_MANAGE_DMUS_WIDGET)), inp->getConnector(), SLOT(RefreshWidget(WidgetDataItemRequest_MANAGE_DMUS_WIDGET)));
		connect(project->getConnector(), SIGNAL(WidgetDataRefresh(WidgetDataItem_MANAGE_DMUS_WIDGET)), this, SLOT(WidgetDataRefreshReceive(WidgetDataItem_MANAGE_DMUS_WIDGET)));
	}
	else if (connection_type == NewGeneWidget::RELEASE_CONNECTIONS_INPUT_PROJECT)
	{
		Empty();
	}

}

void DisplayDMUsRegion::RefreshAllWidgets()
{
	if (inp == nullptr)
	{
		Empty();
		return;
	}
	WidgetDataItemRequest_MANAGE_DMUS_WIDGET request(WIDGET_DATA_ITEM_REQUEST_REASON__REFRESH_ALL_WIDGETS);
	emit RefreshWidget(request);
}

void DisplayDMUsRegion::WidgetDataRefreshReceive(WidgetDataItem_MANAGE_DMUS_WIDGET widget_data)
{

	UIInputProject * project = projectManagerUI().getActiveUIInputProject();
	if (project == nullptr)
	{
		return;
	}

	UIMessager messager(project);

}

void DisplayDMUsRegion::Empty()
{

}

