#include "newgenevariablegroup.h"
#include "ui_newgenevariablegroup.h"

NewGeneVariableGroup::NewGeneVariableGroup( QWidget * parent, DataInstanceIdentifier data_instance_, UIOutputProject * project ) :

	QWidget( parent ),

	NewGeneWidget( WidgetCreationInfo(
										this, // 'this' pointer is cast by compiler to proper Widget instance, which is already created due to order in which base classes appear in class definition
										WIDGET_NATURE_OUTPUT_WIDGET,
										VARIABLE_GROUP_VARIABLE_GROUP_INSTANCE,
										false,
										data_instance_
									 )
				 ),

	ui( new Ui::NewGeneVariableGroup )

{

	ui->setupUi( this );

	PrepareOutputWidget();

	if (data_instance.uuid && project)
	{
		UpdateOutputConnections(UIProjectManager::ESTABLISH_CONNECTIONS_OUTPUT_PROJECT, project);
		WidgetDataItemRequest_VARIABLE_GROUP_VARIABLE_GROUP_INSTANCE request(WIDGET_DATA_ITEM_REQUEST_REASON__REFRESH_ALL_WIDGETS, data_instance);
		emit RefreshWidget(request);
	}

}

NewGeneVariableGroup::~NewGeneVariableGroup()
{
	delete ui;
}

void NewGeneVariableGroup::UpdateOutputConnections(UIProjectManager::UPDATE_CONNECTIONS_TYPE connection_type, UIOutputProject * project)
{
	NewGeneWidget::UpdateOutputConnections(connection_type, project);
	connect(this, SIGNAL(RefreshWidget(WidgetDataItemRequest_VARIABLE_GROUP_VARIABLE_GROUP_INSTANCE)), outp->getConnector(), SLOT(RefreshWidget(WidgetDataItemRequest_VARIABLE_GROUP_VARIABLE_GROUP_INSTANCE)));
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
	WidgetDataItemRequest_VARIABLE_GROUP_VARIABLE_GROUP_INSTANCE request(WIDGET_DATA_ITEM_REQUEST_REASON__REFRESH_ALL_WIDGETS);
	emit RefreshWidget(request);
}

void NewGeneVariableGroup::WidgetDataRefreshReceive(WidgetDataItem_VARIABLE_GROUP_VARIABLE_GROUP_INSTANCE widget_data)
{

	if (!data_instance.uuid || !widget_data.identifier || !widget_data.identifier->uuid || (*data_instance.uuid) != (*widget_data.identifier->uuid) )
	{
		boost::format msg("Invalid widget refresh in NewGeneVariableGroup widget.");
		QMessageBox msgBox;
		msgBox.setText( msg.str().c_str() );
		msgBox.exec();
		return;
	}

	//std::string variables("Variables: ");

	//bool first = true;
	std::for_each(widget_data.identifiers.cbegin(), widget_data.identifiers.cend(), [this /* , &variables, &first */ ](DataInstanceIdentifier const & identifier)
	{
		if (identifier.longhand && !identifier.longhand->empty() && ui->listWidget)
		{

			QListWidgetItem * variable_name_item = new QListWidgetItem(QString(identifier.longhand->c_str()), ui->listWidget);
			variable_name_item->setCheckState(Qt::Unchecked);
			ui->listWidget->addItem(variable_name_item);

			//if (!first)
			//{
			//	variables += ", ";
			//}
			//variables += *identifier.longhand;
			//first = false;

		}
	});

	//QMessageBox msgBox;
	//msgBox.setText( variables.c_str() );
	//msgBox.exec();

}
