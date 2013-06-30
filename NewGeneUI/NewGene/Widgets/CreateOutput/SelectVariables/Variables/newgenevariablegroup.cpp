#include "newgenevariablegroup.h"
#include "ui_newgenevariablegroup.h"

#include <QStandardItem>

NewGeneVariableGroup::NewGeneVariableGroup( QWidget * parent, WidgetInstanceIdentifier data_instance_, UIOutputProject * project ) :

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
	connect(this, SIGNAL(SignalReceiveVariableItemChanged(WidgetActionItemRequest_ACTION_VARIABLE_GROUP_SET_MEMBER_SELECTION_CHANGED)), outp->getConnector(), SLOT(ReceiveVariableItemChanged(WidgetActionItemRequest_ACTION_VARIABLE_GROUP_SET_MEMBER_SELECTION_CHANGED)));
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

	if (!ui->listView)
	{
		boost::format msg("Invalid list view in NewGeneVariableGroup widget.");
		QMessageBox msgBox;
		msgBox.setText( msg.str().c_str() );
		msgBox.exec();
		return;
	}

	QStandardItemModel * oldModel = static_cast<QStandardItemModel*>(ui->listView->model());
	if (oldModel != nullptr)
	{
		delete oldModel;
	}

	QItemSelectionModel * oldSelectionModel = ui->listView->selectionModel();
	QStandardItemModel * model = new QStandardItemModel(ui->listView);

	int index = 0;
	std::for_each(widget_data.identifiers.cbegin(), widget_data.identifiers.cend(), [this, &index, &model](WidgetInstanceIdentifier const & identifier)
	{
		if (identifier.longhand && !identifier.longhand->empty())
		{

			QStandardItem * item = new QStandardItem();
			item->setText(QString(identifier.longhand->c_str()));
			item->setCheckable( true );
			item->setCheckState(Qt::Unchecked);
			QVariant v;
			v.setValue(identifier);
			item->setData(v);
			model->setItem( index, item );

			connect(model, SIGNAL(dataChanged(const QModelIndex &, const QModelIndex &, const QVector<int>)), this, SLOT(ReceiveVariableItemChanged(const QModelIndex &, const QModelIndex &, const QVector<int>)));

			++index;

		}
	});

	ui->listView->setModel(model);
	if (oldSelectionModel) delete oldSelectionModel;

}

void NewGeneVariableGroup::ReceiveVariableItemChanged(const QModelIndex & topLeft, const QModelIndex & bottomRight, const QVector<int> roles)
{

	QStandardItemModel * model = static_cast<QStandardItemModel*>(ui->listView->model());
	if (model == nullptr)
	{
		// Todo: messager error
		return;
	}

	QStandardItem * topLeftItem = (model->itemFromIndex(topLeft));
	QStandardItem * bottomRightItem = (model->itemFromIndex(bottomRight));
	if (topLeftItem == nullptr || bottomRightItem == nullptr)
	{
		// Todo: messager error
		return;
	}

	int topLeftRow = topLeftItem->row();
	int bottomRightRow = bottomRightItem->row();

	WidgetInstanceIdentifiers identifiers;

	for (int row = topLeftRow; row <= bottomRightRow; ++row)
	{
		QStandardItem * currentItem = model->item(row);
		if (currentItem)
		{
			QVariant currentIdentifier = currentItem->data();
			WidgetInstanceIdentifier identifier = currentIdentifier.value<WidgetInstanceIdentifier>();
			identifiers.push_back(identifier);
		}
	}

	WidgetActionItemRequest_ACTION_VARIABLE_GROUP_SET_MEMBER_SELECTION_CHANGED action_request(WIDGET_ACTION_ITEM_REQUEST_REASON__UPDATE_ITEMS, identifiers);
	emit SignalReceiveVariableItemChanged(action_request);

}
