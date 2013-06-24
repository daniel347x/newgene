#include "newgenevariablegroup.h"
#include "ui_newgenevariablegroup.h"

NewGeneVariableGroup::NewGeneVariableGroup( QWidget * parent, DataInstanceIdentifier data_instance ) :

	QWidget( parent ),

	NewGeneWidget( WidgetCreationInfo(
										this, // 'this' pointer is cast by compiler to proper Widget instance, which is already created due to order in which base classes appear in class definition
										WIDGET_NATURE_OUTPUT_WIDGET,
										VARIABLE_GROUP_VARIABLE_GROUP_INSTANCE,
										false,
										data_instance
									 )
				 ),

	ui( new Ui::NewGeneVariableGroup )

{

	ui->setupUi( this );

	PrepareOutputWidget();

	WidgetDataItemRequest_VARIABLE_GROUP_VARIABLE_GROUP_INSTANCE request(WIDGET_DATA_ITEM_REQUEST_REASON__REFRESH_ALL_WIDGETS);
	emit RefreshWidget(request);

}

NewGeneVariableGroup::~NewGeneVariableGroup()
{
	delete ui;
}

void NewGeneVariableGroup::UpdateOutputConnections(UIProjectManager::UPDATE_CONNECTIONS_TYPE connection_type, UIOutputProject * project)
{
	NewGeneWidget::UpdateOutputConnections(connection_type, project);
	connect(this, SIGNAL(RefreshWidget(WidgetDataItem_VARIABLE_GROUP_VARIABLE_GROUP_INSTANCE)), outp->getConnector(), SLOT(RefreshWidget(WidgetDataItem_VARIABLE_GROUP_VARIABLE_GROUP_INSTANCE)));
	connect(project->getConnector(), SIGNAL(WidgetDataRefresh(WidgetDataItem_VARIABLE_GROUP_VARIABLE_GROUP_INSTANCE)), this, SLOT(WidgetDataRefreshReceive(WidgetDataItem_VARIABLE_GROUP_VARIABLE_GROUP_INSTANCE)));
}

void NewGeneVariableGroup::changeEvent( QEvent * e )
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

void NewGeneVariableGroup::RefreshAllWidgets()
{
	WidgetDataItem_VARIABLE_GROUP_VARIABLE_GROUP_INSTANCE request(WIDGET_DATA_ITEM_REQUEST_REASON__REFRESH_ALL_WIDGETS);
	emit RefreshWidget(request);
}

void NewGeneVariableGroup::WidgetDataRefreshReceive(WidgetDataItem_VARIABLE_GROUP_VARIABLE_GROUP_INSTANCE widget_data)
{

	std::string variables("Variables: ");

	bool first = true;
	std::for_each(widget_data.identifiers.cbegin(), widget_data.identifiers.cend(), [&variables, &first](DataInstanceIdentifier const & identifier)
	{
		if (identifier.longhand)
		{
			if (!first)
			{
				variables += ", ";
			}
			variables += *identifier.longhand;
			first = false;
		}
	});

	QMessageBox msgBox;
	msgBox.setText( variables.c_str() );
	msgBox.exec();

}
