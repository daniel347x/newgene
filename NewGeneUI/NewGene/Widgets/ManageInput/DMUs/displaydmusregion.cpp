#include "displaydmusregion.h"
#include "ui_displaydmusregion.h"

#include <QStandardItem>

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
	std::for_each(widget_data.dmus_and_members.cbegin(), widget_data.dmus_and_members.cend(), [this, &index, &model](std::pair<WidgetInstanceIdentifier, WidgetInstanceIdentifiers> const & dmu_and_member)
	{
		WidgetInstanceIdentifier const & dmu = dmu_and_member.first;
		WidgetInstanceIdentifiers const & dmu_members = dmu_and_member.second;
		if (dmu.code && !dmu.code->empty())
		{

			QStandardItem * item = new QStandardItem();
			QString text(dmu.code->c_str());
			if (dmu.longhand && !dmu.longhand->empty())
			{
				text += " (";
				text += dmu.longhand->c_str();
				text += ")";
			}
			item->setText(text);
			item->setEditable(false);
			item->setCheckable(false);
			QVariant v;
			v.setValue(dmu_and_member);
			item->setData(v);
			model->setItem( index, item );

			++index;

		}
	});

	//connect(model, SIGNAL(itemChanged(QStandardItem*)), this, SLOT(ReceiveVariableItemChanged(QStandardItem*)));

	ui->listView->setModel(model);
	if (oldSelectionModel) delete oldSelectionModel;

}

void DisplayDMUsRegion::Empty()
{

}

