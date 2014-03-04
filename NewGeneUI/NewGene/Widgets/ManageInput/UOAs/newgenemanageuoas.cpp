#include "newgenemanageuoas.h"
#include "ui_newgenemanageuoas.h"

#include <QStandardItemModel>

#include "../Project/uiprojectmanager.h"
#include "../Project/uiinputproject.h"
#include "../../Utilities/qsortfilterproxymodel_numberslast.h"
#include "../../Utilities/importdialoghelper.h"
#include "../../../../../NewGeneBackEnd/Utilities/Validation.h"
#include "../../../../NewGeneBackEnd/Utilities/TimeRangeHelper.h"

NewGeneManageUOAs::NewGeneManageUOAs( QWidget * parent ) :
	QWidget( parent ),
	NewGeneWidget( WidgetCreationInfo(this, parent, WIDGET_NATURE_INPUT_WIDGET, MANAGE_UOAS_WIDGET, true) ), // 'this' pointer is cast by compiler to proper Widget instance, which is already created due to order in which base classes appear in class definition
	ui( new Ui::NewGeneManageUOAs )
{
	ui->setupUi( this );
	PrepareInputWidget(true);
}

NewGeneManageUOAs::~NewGeneManageUOAs()
{
	delete ui;
}


void NewGeneManageUOAs::changeEvent( QEvent * e )
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

void NewGeneManageUOAs::UpdateInputConnections(NewGeneWidget::UPDATE_CONNECTIONS_TYPE connection_type, UIInputProject * project)
{

	NewGeneWidget::UpdateInputConnections(connection_type, project);

	if (connection_type == NewGeneWidget::ESTABLISH_CONNECTIONS_INPUT_PROJECT)
	{
		connect(this, SIGNAL(RefreshWidget(WidgetDataItemRequest_MANAGE_UOAS_WIDGET)), inp->getConnector(), SLOT(RefreshWidget(WidgetDataItemRequest_MANAGE_UOAS_WIDGET)));
		connect(project->getConnector(), SIGNAL(WidgetDataRefresh(WidgetDataItem_MANAGE_UOAS_WIDGET)), this, SLOT(WidgetDataRefreshReceive(WidgetDataItem_MANAGE_UOAS_WIDGET)));
		connect(this, SIGNAL(AddUOA(WidgetActionItemRequest_ACTION_ADD_UOA)), inp->getConnector(), SLOT(AddUOA(WidgetActionItemRequest_ACTION_ADD_UOA)));
		connect(this, SIGNAL(DeleteUOA(WidgetActionItemRequest_ACTION_DELETE_UOA)), inp->getConnector(), SLOT(DeleteUOA(WidgetActionItemRequest_ACTION_DELETE_UOA)));

		if (project)
		{
			project->RegisterInterestInChange(this, DATA_CHANGE_TYPE__INPUT_MODEL__UOA_CHANGE, false, "");
		}
	}
	else if (connection_type == NewGeneWidget::RELEASE_CONNECTIONS_INPUT_PROJECT)
	{
		if (inp)
		{
			inp->UnregisterInterestInChanges(this);
		}
		Empty();
	}

}

void NewGeneManageUOAs::UpdateOutputConnections(NewGeneWidget::UPDATE_CONNECTIONS_TYPE connection_type, UIOutputProject * project)
{

	NewGeneWidget::UpdateOutputConnections(connection_type, project);

	if (connection_type == NewGeneWidget::ESTABLISH_CONNECTIONS_OUTPUT_PROJECT)
	{
		if (project)
		{
			connect(this, SIGNAL(DeleteUOA(WidgetActionItemRequest_ACTION_DELETE_DMU)), project->getConnector(), SLOT(DeleteUOA(WidgetActionItemRequest_ACTION_DELETE_UOA)));
		}
	}
	else if (connection_type == NewGeneWidget::RELEASE_CONNECTIONS_INPUT_PROJECT)
	{
		if (project)
		{
		}
	}

}

void NewGeneManageUOAs::RefreshAllWidgets()
{
	if (inp == nullptr)
	{
		Empty();
		return;
	}
	WidgetDataItemRequest_MANAGE_UOAS_WIDGET request(WIDGET_DATA_ITEM_REQUEST_REASON__REFRESH_ALL_WIDGETS);
	emit RefreshWidget(request);
}

void NewGeneManageUOAs::WidgetDataRefreshReceive(WidgetDataItem_MANAGE_UOAS_WIDGET widget_data)
{

	UIInputProject * project = projectManagerUI().getActiveUIInputProject();
	if (project == nullptr)
	{
		return;
	}

	UIMessager messager(project);

	if (!ui->listViewManageUOAs)
	{
		boost::format msg("Invalid list view in NewGeneManageUOAs widget.");
		QMessageBox msgBox;
		msgBox.setText( msg.str().c_str() );
		msgBox.exec();
		return;
	}

	QStandardItemModel * oldModel = static_cast<QStandardItemModel*>(ui->listViewManageUOAs->model());
	if (oldModel != nullptr)
	{
		delete oldModel;
	}

	QItemSelectionModel * oldSelectionModel = ui->listViewManageUOAs->selectionModel();
	QStandardItemModel * model = new QStandardItemModel(ui->listViewManageUOAs);

	int index = 0;
	std::for_each(widget_data.uoas_and_dmu_categories.cbegin(), widget_data.uoas_and_dmu_categories.cend(), [this, &index, &model](std::pair<WidgetInstanceIdentifier, WidgetInstanceIdentifiers> const & uoa_and_dmu_categories)
	{
		WidgetInstanceIdentifier const & uoa_category = uoa_and_dmu_categories.first;
		WidgetInstanceIdentifiers const & dmu_categories = uoa_and_dmu_categories.second;
		if (uoa_category.uuid && !uoa_category.uuid->empty())
		{

			std::pair<WidgetInstanceIdentifier, WidgetInstanceIdentifiers> uoa_and_dmu_categories = std::make_pair(uoa_category, dmu_categories);

			QStandardItem * item = new QStandardItem();
			std::string text = Table_UOA_Identifier::GetUoaCategoryDisplayText(uoa_category, dmu_categories);
			item->setText(text.c_str());
			item->setEditable(false);
			item->setCheckable(false);
			QVariant v;
			v.setValue(uoa_and_dmu_categories);
			item->setData(v);
			model->setItem( index, item );

			++index;

		}
	});

	model->sort(0);

	ui->listViewManageUOAs->setModel(model);
	if (oldSelectionModel) delete oldSelectionModel;

}

void NewGeneManageUOAs::Empty()
{

	if (!ui->listViewManageUOAs)
	{
		boost::format msg("Invalid list view in NewGeneManageUOAs widget.");
		QMessageBox msgBox;
		msgBox.setText( msg.str().c_str() );
		msgBox.exec();
		return;
	}

	QStandardItemModel * oldModel = nullptr;
	QItemSelectionModel * oldSelectionModel = nullptr;

	oldSelectionModel = ui->listViewManageUOAs->selectionModel();
	if (oldSelectionModel != nullptr)
	{
		delete oldSelectionModel;
		oldSelectionModel = nullptr;
	}

	oldModel = static_cast<QStandardItemModel*>(ui->listViewManageUOAs->model());
	if (oldModel != nullptr)
	{
		delete oldModel;
		oldModel = nullptr;
	}

	oldSelectionModel = ui->listViewManageUOAs->selectionModel();
	if (oldSelectionModel != nullptr)
	{
		delete oldSelectionModel;
		oldSelectionModel = nullptr;
	}

}

void NewGeneManageUOAs::on_pushButton_deleteUOA_clicked()
{

	InstanceActionItems actionItems;
	//actionItems.push_back(std::make_pair(WidgetInstanceIdentifier(dmu), std::shared_ptr<WidgetActionItem>(static_cast<WidgetActionItem*>(new WidgetActionItem__WidgetInstanceIdentifier(dmu)))));
	WidgetActionItemRequest_ACTION_DELETE_UOA action_request(WIDGET_ACTION_ITEM_REQUEST_REASON__REMOVE_ITEMS, actionItems);
	emit DeleteUOA(action_request);

}

void NewGeneManageUOAs::on_pushButton_createUOA_clicked()
{

	InstanceActionItems actionItems;
	//actionItems.push_back(std::make_pair(WidgetInstanceIdentifier(), std::shared_ptr<WidgetActionItem>(static_cast<WidgetActionItem*>(new WidgetActionItem__StringVector(std::vector<std::string>{proposed_dmu_name, dmu_description})))));
	WidgetActionItemRequest_ACTION_ADD_UOA action_request(WIDGET_ACTION_ITEM_REQUEST_REASON__ADD_ITEMS, actionItems);

	emit AddUOA(action_request);

}

void NewGeneManageUOAs::HandleChanges(DataChangeMessage const & change_message)
{

	UIInputProject * project = projectManagerUI().getActiveUIInputProject();
	if (project == nullptr)
	{
		return;
	}

	UIMessager messager(project);

	if (!ui->listViewManageUOAs)
	{
		boost::format msg("Invalid list view in NewGeneManageUOAs widget.");
		QMessageBox msgBox;
		msgBox.setText( msg.str().c_str() );
		msgBox.exec();
		return;
	}

	QStandardItemModel * itemModel = static_cast<QStandardItemModel*>(ui->listViewManageUOAs->model());
	if (itemModel == nullptr)
	{
		boost::format msg("Invalid list view items in NewGeneManageUOAs widget.");
		QMessageBox msgBox;
		msgBox.setText( msg.str().c_str() );
		msgBox.exec();
		return;
	}

	std::for_each(change_message.changes.cbegin(), change_message.changes.cend(), [this, &itemModel](DataChange const & change)
	{

		switch (change.change_type)
		{

			case DATA_CHANGE_TYPE::DATA_CHANGE_TYPE__INPUT_MODEL__UOA_CHANGE:
				{

					switch (change.change_intention)
					{

						case DATA_CHANGE_INTENTION__ADD:
							{

								if (!change.parent_identifier.uuid || (*change.parent_identifier.uuid).empty())
								{
									boost::format msg("Invalid new UOA ID.");
									QMessageBox msgBox;
									msgBox.setText( msg.str().c_str() );
									msgBox.exec();
									return;
								}

								WidgetInstanceIdentifier const & uoa_category = change.parent_identifier;
								WidgetInstanceIdentifiers const & dmu_categories = change.child_identifiers;

								std::string text = Table_UOA_Identifier::GetUoaCategoryDisplayText(uoa_category, dmu_categories);

								QStandardItem * item = new QStandardItem();
								item->setText(text.c_str());
								item->setEditable(false);
								item->setCheckable(false);

								std::pair<WidgetInstanceIdentifier, WidgetInstanceIdentifiers> uoa_and_dmu_categories = std::make_pair(uoa_category, dmu_categories);
								QVariant v;
								v.setValue(uoa_and_dmu_categories);
								item->setData(v);
								itemModel->appendRow( item );

							}
							break;

						case DATA_CHANGE_INTENTION__REMOVE:
							{

								if (!change.parent_identifier.uuid || (*change.parent_identifier.uuid).empty())
								{
									boost::format msg("Invalid new UOA ID.");
									QMessageBox msgBox;
									msgBox.setText( msg.str().c_str() );
									msgBox.exec();
									return;
								}

								WidgetInstanceIdentifier const & uoa_category = change.parent_identifier;
								WidgetInstanceIdentifiers const & dmu_categories = change.child_identifiers;

								std::string text = Table_UOA_Identifier::GetUoaCategoryDisplayText(uoa_category, dmu_categories);
								QList<QStandardItem *> items = itemModel->findItems(text.c_str());
								if (items.count() == 1)
								{
									QStandardItem * uoa_to_remove = items.at(0);
									if (uoa_to_remove != nullptr)
									{
										QModelIndex index_to_remove = itemModel->indexFromItem(uoa_to_remove);
										itemModel->takeRow(index_to_remove.row());

										delete uoa_to_remove;
										uoa_to_remove = nullptr;

										QItemSelectionModel * selectionModel = ui->listViewManageUOAs->selectionModel();
										if (selectionModel != nullptr)
										{
											selectionModel->clearSelection();
										}

									}
								}

							}
							break;

						case DATA_CHANGE_INTENTION__UPDATE:
							{
								// Should never receive this.
							}
							break;

						case DATA_CHANGE_INTENTION__RESET_ALL:
							{
								// Ditto above.
							}
							break;

						default:
							{
							}
							break;

					}

				}
				break;

			default:
				{
				}
				break;

		}

	});

}
