#include "newgenevariablesummaryscrollarea.h"
#include "ui_newgenevariablesummaryscrollarea.h"

NewGeneVariableSummaryScrollArea::NewGeneVariableSummaryScrollArea( QWidget * parent ) :
	QWidget( parent ),
	NewGeneWidget( WidgetCreationInfo(this, WIDGET_NATURE_OUTPUT_WIDGET, VARIABLE_GROUPS_SUMMARY, true) ), // 'this' pointer is cast by compiler to proper Widget instance, which is already created due to order in which base classes appear in class definition
	ui( new Ui::NewGeneVariableSummaryScrollArea )
{

	ui->setupUi( this );

	PrepareOutputWidget();

}

NewGeneVariableSummaryScrollArea::~NewGeneVariableSummaryScrollArea()
{
	delete ui;
}

void NewGeneVariableSummaryScrollArea::changeEvent( QEvent * e )
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

void NewGeneVariableSummaryScrollArea::UpdateOutputConnections(UIProjectManager::UPDATE_CONNECTIONS_TYPE connection_type, UIOutputProject * project)
{
	NewGeneWidget::UpdateOutputConnections(connection_type, project);
	connect(this, SIGNAL(RefreshWidget(WidgetDataItemRequest_VARIABLE_GROUPS_SUMMARY)), outp->getConnector(), SLOT(RefreshWidget(WidgetDataItemRequest_VARIABLE_GROUPS_SUMMARY)));

	// *** This is a parent (top-level) widget, so connect to refreshes here (... child widgets don't connect to refreshes) *** //
	connect(project->getConnector(), SIGNAL(WidgetDataRefresh(WidgetDataItem_VARIABLE_GROUPS_SUMMARY)), this, SLOT(WidgetDataRefreshReceive(WidgetDataItem_VARIABLE_GROUPS_SUMMARY)));

	// *** Has child widgets, so refer refresh signals directed at child to be received by us, the parent *** //
	connect(project->getConnector(), SIGNAL(WidgetDataRefresh(WidgetDataItem_VARIABLE_GROUPS_SUMMARY_VARIABLE_GROUP_INSTANCE)), this, SLOT(WidgetDataRefreshReceive(WidgetDataItem_VARIABLE_GROUPS_SUMMARY_VARIABLE_GROUP_INSTANCE)));
}

void NewGeneVariableSummaryScrollArea::RefreshAllWidgets()
{
	WidgetDataItemRequest_VARIABLE_GROUPS_SUMMARY request(WIDGET_DATA_ITEM_REQUEST_REASON__REFRESH_ALL_WIDGETS);
	emit RefreshWidget(request);
}

void NewGeneVariableSummaryScrollArea::WidgetDataRefreshReceive(WidgetDataItem_VARIABLE_GROUPS_SUMMARY widget_data)
{

	int nItems = layout()->count();
	for (int n = 0; n < nItems; ++n)
	{
		QWidget * child = find(n);
		layout()->removeItem(0);
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
			NewGeneVariableSummaryGroup * group = new NewGeneVariableSummaryGroup( this, identifier, outp );
			group->setTitle(identifier.longhand->c_str());
			layout()->addWidget(group);
		}
	});

}

void NewGeneVariableSummaryScrollArea::WidgetDataRefreshReceive(WidgetDataItem_VARIABLE_GROUPS_SUMMARY_VARIABLE_GROUP_INSTANCE widget_data)
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
