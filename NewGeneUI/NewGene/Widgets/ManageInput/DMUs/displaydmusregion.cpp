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
		connect(this, SIGNAL(AddDMU(WidgetActionItemRequest_ACTION_ADD_DMU)), inp->getConnector(), SLOT(AddDMU(WidgetActionItemRequest_ACTION_ADD_DMU)));
		connect(this, SIGNAL(DeleteDMU(WidgetActionItemRequest_ACTION_DELETE_DMU)), inp->getConnector(), SLOT(DeleteDMU(WidgetActionItemRequest_ACTION_DELETE_DMU)));
		connect(this, SIGNAL(AddDMUMembers(WidgetActionItemRequest_ACTION_ADD_DMU_MEMBERS)), inp->getConnector(), SLOT(AddDMUMembers(WidgetActionItemRequest_ACTION_ADD_DMU_MEMBERS)));
		connect(this, SIGNAL(DeleteDMUMembers(WidgetActionItemRequest_ACTION_DELETE_DMU_MEMBERS)), inp->getConnector(), SLOT(DeleteDMUMembers(WidgetActionItemRequest_ACTION_DELETE_DMU_MEMBERS)));
		connect(this, SIGNAL(RefreshDMUsFromFile(WidgetActionItemRequest_ACTION_REFRESH_DMUS_FROM_FILE)), inp->getConnector(), SLOT(RefreshDMUsFromFile(WidgetActionItemRequest_ACTION_REFRESH_DMUS_FROM_FILE)));
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

	if (!ui->listView_dmus)
	{
		boost::format msg("Invalid list view in DisplayDMUsRegion widget.");
		QMessageBox msgBox;
		msgBox.setText( msg.str().c_str() );
		msgBox.exec();
		return;
	}

	QStandardItemModel * oldModel = static_cast<QStandardItemModel*>(ui->listView_dmus->model());
	if (oldModel != nullptr)
	{
		delete oldModel;
	}

	QItemSelectionModel * oldSelectionModel = ui->listView_dmus->selectionModel();
	QStandardItemModel * model = new QStandardItemModel(ui->listView_dmus);

	int index = 0;
	std::for_each(widget_data.dmus_and_members.cbegin(), widget_data.dmus_and_members.cend(), [this, &index, &model](std::pair<WidgetInstanceIdentifier, WidgetInstanceIdentifiers> const & dmu_and_members)
	{
		WidgetInstanceIdentifier const & dmu = dmu_and_members.first;
		WidgetInstanceIdentifiers const & dmu_members = dmu_and_members.second;
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
			v.setValue(dmu_and_members);
			item->setData(v);
			model->setItem( index, item );

			++index;

		}
	});

	ui->listView_dmus->setModel(model);
	if (oldSelectionModel) delete oldSelectionModel;

	connect( ui->listView_dmus->selectionModel(), SIGNAL(selectionChanged(QItemSelection, QItemSelection)), this, SLOT(ReceiveDMUSelectionChanged(const QItemSelection &, const QItemSelection &)));

}

void DisplayDMUsRegion::Empty()
{

}

void DisplayDMUsRegion::ReceiveDMUSelectionChanged(const QItemSelection & selected, const QItemSelection & deselected)
{

	UIInputProject * project = projectManagerUI().getActiveUIInputProject();
	if (project == nullptr)
	{
		return;
	}

	UIMessager messager(project);

	if (!ui->listView_dmus || !ui->listView_dmu_members)
	{
		boost::format msg("Invalid list view in DisplayDMUsRegion widget.");
		QMessageBox msgBox;
		msgBox.setText( msg.str().c_str() );
		msgBox.exec();
		return;
	}

	QStandardItemModel * oldModel = static_cast<QStandardItemModel*>(ui->listView_dmu_members->model());
	if (oldModel != nullptr)
	{
		delete oldModel;
	}

	if(!selected.indexes().isEmpty())
	{

		QStandardItemModel * dmuModel = static_cast<QStandardItemModel*>(ui->listView_dmus->model());

		QItemSelectionModel * oldSelectionModel = ui->listView_dmu_members->selectionModel();
		QStandardItemModel * model = new QStandardItemModel(ui->listView_dmu_members);

		QModelIndex selectedIndex = selected.indexes().first();

		QVariant dmu_and_members_variant = dmuModel->item(selectedIndex.row())->data();
		std::pair<WidgetInstanceIdentifier, WidgetInstanceIdentifiers> dmu_and_members = dmu_and_members_variant.value<std::pair<WidgetInstanceIdentifier, WidgetInstanceIdentifiers>>();
		WidgetInstanceIdentifier & dmu = dmu_and_members.first;
		WidgetInstanceIdentifiers & dmu_members = dmu_and_members.second;

		int index = 0;
		std::for_each(dmu_members.cbegin(), dmu_members.cend(), [this, &index, &model](WidgetInstanceIdentifier const & dmu_member)
		{
			if ((dmu_member.code && !dmu_member.code->empty())
				||
				(dmu_member.longhand && !dmu_member.longhand->empty()))
			{

				QStandardItem * item = new QStandardItem();
				QString text(dmu_member.code->c_str());
				if (dmu_member.longhand && !dmu_member.longhand->empty())
				{
					bool use_parentheses = false;
					if (dmu_member.code && !dmu_member.code->empty())
					{
						use_parentheses = true;
					}
					if (use_parentheses)
					{
						text += " (";
					}
					text += dmu_member.longhand->c_str();
					if (use_parentheses)
					{
						text += ")";
					}
				}
				item->setText(text);
				item->setEditable(false);
				item->setCheckable(true);
				QVariant v;
				v.setValue(dmu_member);
				item->setData(v);
				model->setItem( index, item );

				++index;

			}
		});

		ui->listView_dmu_members->setModel(model);
		if (oldSelectionModel) delete oldSelectionModel;
	}

}

void DisplayDMUsRegion::on_pushButton_add_dmu_clicked()
{
	WidgetActionItemRequest_ACTION_ADD_DMU dummy;
	emit AddDMU(dummy);
}

void DisplayDMUsRegion::on_pushButton_delete_dmu_clicked()
{
	WidgetActionItemRequest_ACTION_DELETE_DMU dummy;
	emit DeleteDMU(dummy);
}

void DisplayDMUsRegion::on_pushButton_refresh_dmu_members_from_file_clicked()
{
	WidgetActionItemRequest_ACTION_ADD_DMU_MEMBERS dummy;
	emit AddDMUMembers(dummy);
}

void DisplayDMUsRegion::on_pushButton_add_dmu_member_by_hand_clicked()
{
	WidgetActionItemRequest_ACTION_DELETE_DMU_MEMBERS dummy;
	emit DeleteDMUMembers(dummy);
}

void DisplayDMUsRegion::on_pushButton_delete_selected_dmu_members_clicked()
{
	WidgetActionItemRequest_ACTION_REFRESH_DMUS_FROM_FILE dummy;
	emit RefreshDMUsFromFile(dummy);
}

void DisplayDMUsRegion::on_pushButton_deselect_all_dmu_members_clicked()
{
	if (!ui->listView_dmu_members)
	{
		boost::format msg("Invalid list view in DisplayDMUsRegion widget.");
		QMessageBox msgBox;
		msgBox.setText( msg.str().c_str() );
		msgBox.exec();
		return;
	}
	QStandardItemModel * model = static_cast<QStandardItemModel*>(ui->listView_dmu_members->model());
	if (model == nullptr)
	{
		return;
	}
	int nrows = model->rowCount();
	for (int row=0; row<nrows; ++row) {
		QStandardItem * item = model->item(row, 0);
		if (item)
		{
			item->setCheckState(Qt::Unchecked);
		}
	}
}

void DisplayDMUsRegion::on_pushButton_select_all_dmu_members_clicked()
{
	if (!ui->listView_dmu_members)
	{
		boost::format msg("Invalid list view in DisplayDMUsRegion widget.");
		QMessageBox msgBox;
		msgBox.setText( msg.str().c_str() );
		msgBox.exec();
		return;
	}
	QStandardItemModel * model = static_cast<QStandardItemModel*>(ui->listView_dmu_members->model());
	if (model == nullptr)
	{
		return;
	}
	int nrows = model->rowCount();
	for (int row=0; row<nrows; ++row) {
		QStandardItem * item = model->item(row, 0);
		if (item)
		{
			item->setCheckState(Qt::Checked);
		}
	}
}
