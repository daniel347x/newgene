#include "newgenevariablesummary.h"
#include "ui_newgenevariablesummary.h"

#include <QListView>
#include <QStringList>

NewGeneVariableSummary::NewGeneVariableSummary( QWidget * parent ) :
	QWidget( parent ),
	NewGeneWidget( WidgetCreationInfo(this, WIDGET_NATURE_OUTPUT_WIDGET) ), // 'this' pointer is cast by compiler to proper Widget instance, which is already created due to order in which base classes appear in class definition
	ui( new Ui::NewGeneVariableSummary )
{
	ui->setupUi( this );
	PrepareOutputWidget();
}

NewGeneVariableSummary::~NewGeneVariableSummary()
{
	delete ui;
}

void NewGeneVariableSummary::changeEvent( QEvent * e )
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

void NewGeneVariableSummary::UpdateOutputConnections(UIProjectManager::UPDATE_CONNECTIONS_TYPE connection_type, UIOutputProject * project)
{
	NewGeneWidget::UpdateOutputConnections(connection_type, project);
	connect(this, SIGNAL(RefreshWidget(WidgetDataItemRequest_VARIABLE_GROUPS_SUMMARY)), outp->getConnector(), SLOT(RefreshWidget(WidgetDataItemRequest_VARIABLE_GROUPS_SUMMARY)));

	// *** This is a parent (top-level) widget, so connect to refreshes here (... child widgets don't connect to refreshes) *** //
	connect(project->getConnector(), SIGNAL(WidgetDataRefresh(WidgetDataItem_VARIABLE_GROUPS_SUMMARY)), this, SLOT(WidgetDataRefreshReceive(WidgetDataItem_VARIABLE_GROUPS_SUMMARY)));

	// *** Has child widgets, so refer refresh signals directed at child to be received by us, the parent *** //
	connect(project->getConnector(), SIGNAL(WidgetDataRefresh(WidgetDataItem_VARIABLE_GROUPS_SUMMARY_VARIABLE_GROUP_INSTANCE)), this, SLOT(WidgetDataRefreshReceive(WidgetDataItem_VARIABLE_GROUPS_SUMMARY_VARIABLE_GROUP_INSTANCE)));
}

void NewGeneVariableSummary::RefreshAllWidgets()
{
	WidgetDataItemRequest_VARIABLE_GROUPS_SUMMARY request(WIDGET_DATA_ITEM_REQUEST_REASON__REFRESH_ALL_WIDGETS);
	emit RefreshWidget(request);
}

void NewGeneVariableSummary::WidgetDataRefreshReceive(WidgetDataItem_VARIABLE_GROUPS_SUMMARY widget_data)
{

	QLayoutItem *child;
	while ((child = layout()->takeAt(0)) != 0)
	{
		delete child;
	}

	std::for_each(widget_data.identifiers.cbegin(), widget_data.identifiers.cend(), [&](WidgetInstanceIdentifier const & identifier)
	{
		if (identifier.uuid && identifier.code && identifier.longhand)
		{
			WidgetInstanceIdentifier new_identifier(identifier);
			new_identifier.uuid_parent = std::make_shared<UUID>(uuid);
			NewGeneVariableSummaryGroup * tmpGrp = new NewGeneVariableSummaryGroup( this, new_identifier, outp );
			tmpGrp->setTitle(identifier.longhand->c_str());
			layout()->addWidget(tmpGrp);
		}
	});

}

void NewGeneVariableSummary::WidgetDataRefreshReceive(WidgetDataItem_VARIABLE_GROUPS_SUMMARY_VARIABLE_GROUP_INSTANCE widget_data)
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
