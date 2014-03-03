#include "newgenemanageuoas.h"
#include "ui_newgenemanageuoas.h"

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
			std::string text = Table_DMU_Identifier::GetDmuCategoryDisplayText(dmu);
			item->setText(text.c_str());
			item->setEditable(false);
			item->setCheckable(false);
			QVariant v;
			v.setValue(dmu_and_members);
			item->setData(v);
			model->setItem( index, item );

			++index;

		}
	});

	model->sort(0);

	ui->listView_dmus->setModel(model);
	if (oldSelectionModel) delete oldSelectionModel;

	connect( ui->listView_dmus->selectionModel(), SIGNAL(selectionChanged(QItemSelection, QItemSelection)), this, SLOT(ReceiveDMUSelectionChanged(const QItemSelection &, const QItemSelection &)));

}

void NewGeneManageUOAs::DisplayDMUsRegion::Empty()
{

	if (!ui->listView_dmus || !ui->listView_dmu_members)
	{
		boost::format msg("Invalid list view in DisplayDMUsRegion widget.");
		QMessageBox msgBox;
		msgBox.setText( msg.str().c_str() );
		msgBox.exec();
		return;
	}

	QSortFilterProxyModel * oldModel = nullptr;
	QAbstractItemModel * oldSourceModel = nullptr;
	QStandardItemModel * oldDmuModel = nullptr;
	QItemSelectionModel * oldSelectionModel = nullptr;

	oldModel = static_cast<QSortFilterProxyModel_NumbersLast*>(ui->listView_dmu_members->model());
	if (oldModel != nullptr)
	{
		oldSourceModel = oldModel->sourceModel();
		if (oldSourceModel != nullptr)
		{
			delete oldSourceModel;
			oldSourceModel = nullptr;
		}
		delete oldModel;
		oldModel = nullptr;
	}

	oldSelectionModel = ui->listView_dmu_members->selectionModel();
	if (oldSelectionModel != nullptr)
	{
		delete oldSelectionModel;
		oldSelectionModel = nullptr;
	}

	oldDmuModel = static_cast<QStandardItemModel*>(ui->listView_dmus->model());
	if (oldDmuModel != nullptr)
	{
		delete oldDmuModel;
		oldDmuModel = nullptr;
	}

	oldSelectionModel = ui->listView_dmus->selectionModel();
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
	emit DeleteDMU(action_request);

}

void NewGeneManageUOAs::on_pushButton_createUOA_clicked()
{

	InstanceActionItems actionItems;
	//actionItems.push_back(std::make_pair(WidgetInstanceIdentifier(), std::shared_ptr<WidgetActionItem>(static_cast<WidgetActionItem*>(new WidgetActionItem__StringVector(std::vector<std::string>{proposed_dmu_name, dmu_description})))));
	WidgetActionItemRequest_ACTION_ADD_UOA action_request(WIDGET_ACTION_ITEM_REQUEST_REASON__ADD_ITEMS, actionItems);

	emit AddDMU(action_request);

}

void NewGeneManageUOAs::HandleChanges(DataChangeMessage const & change_message)
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

	QStandardItemModel * itemModel = static_cast<QStandardItemModel*>(ui->listView_dmus->model());
	if (itemModel == nullptr)
	{
		boost::format msg("Invalid list view items in DisplayDMUsRegion widget.");
		QMessageBox msgBox;
		msgBox.setText( msg.str().c_str() );
		msgBox.exec();
		return;
	}

	QSortFilterProxyModel_NumbersLast * proxyModel = nullptr;

	std::for_each(change_message.changes.cbegin(), change_message.changes.cend(), [this, &itemModel, &proxyModel](DataChange const & change)
	{

		switch (change.change_type)
		{

			case DATA_CHANGE_TYPE::DATA_CHANGE_TYPE__INPUT_MODEL__DMU_CHANGE:
				{

					switch (change.change_intention)
					{

						case DATA_CHANGE_INTENTION__ADD:
							{

								if (!change.parent_identifier.code || (*change.parent_identifier.code).empty() || !change.parent_identifier.longhand)
								{
									boost::format msg("Invalid new DMU name or description.");
									QMessageBox msgBox;
									msgBox.setText( msg.str().c_str() );
									msgBox.exec();
									return;
								}

								WidgetInstanceIdentifier const & dmu_category = change.parent_identifier;

								std::string text = Table_DMU_Identifier::GetDmuCategoryDisplayText(dmu_category);

								QStandardItem * item = new QStandardItem();
								item->setText(text.c_str());
								item->setEditable(false);
								item->setCheckable(false);

								std::pair<WidgetInstanceIdentifier, WidgetInstanceIdentifiers> dmu_and_members = std::make_pair(change.parent_identifier, change.child_identifiers);
								QVariant v;
								v.setValue(dmu_and_members);
								item->setData(v);
								itemModel->appendRow( item );

							}
							break;

						case DATA_CHANGE_INTENTION__REMOVE:
							{

								if (!change.parent_identifier.code || (*change.parent_identifier.code).empty() || !change.parent_identifier.longhand)
								{
									boost::format msg("Invalid DMU name or description.");
									QMessageBox msgBox;
									msgBox.setText( msg.str().c_str() );
									msgBox.exec();
									return;
								}

								std::string text = Table_DMU_Identifier::GetDmuCategoryDisplayText(change.parent_identifier);
								QList<QStandardItem *> items = itemModel->findItems(text.c_str());
								if (items.count() == 1)
								{
									QStandardItem * dmu_to_remove = items.at(0);
									if (dmu_to_remove != nullptr)
									{
										QModelIndex index_to_remove = itemModel->indexFromItem(dmu_to_remove);
										itemModel->takeRow(index_to_remove.row());

										delete dmu_to_remove;
										dmu_to_remove = nullptr;

										QItemSelectionModel * selectionModel = ui->listView_dmus->selectionModel();
										if (selectionModel != nullptr)
										{
											selectionModel->clearSelection();
											QItemSelectionModel * oldDmuSetMembersSelectionModel = ui->listView_dmu_members->selectionModel();
											QSortFilterProxyModel_NumbersLast * dmuSetMembersModel = static_cast<QSortFilterProxyModel_NumbersLast*>(ui->listView_dmu_members->model());
											if (dmuSetMembersModel != nullptr)
											{
												QAbstractItemModel * dmuSetMembersSourceModel = dmuSetMembersModel->sourceModel();
												if (dmuSetMembersSourceModel != nullptr)
												{
													QStandardItemModel * dmuSetMembersStandardSourceModel = nullptr;
													try
													{
														dmuSetMembersStandardSourceModel = dynamic_cast<QStandardItemModel*>(dmuSetMembersSourceModel);
													}
													catch (std::bad_cast &)
													{
														// guess not
														return;
													}
													delete dmuSetMembersStandardSourceModel;
													dmuSetMembersStandardSourceModel = nullptr;
													delete dmuSetMembersModel;
													dmuSetMembersModel = nullptr;
													QStandardItemModel * model = new QStandardItemModel();
													proxyModel = new QSortFilterProxyModel_NumbersLast(ui->listView_dmu_members);
													proxyModel->setDynamicSortFilter(true);
													proxyModel->setSourceModel(model);
													ui->listView_dmu_members->setModel(proxyModel);
													if (oldDmuSetMembersSelectionModel)
													{
														delete oldDmuSetMembersSelectionModel;
														oldDmuSetMembersSelectionModel = nullptr;
													}
												}
											}
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

			case DATA_CHANGE_TYPE::DATA_CHANGE_TYPE__INPUT_MODEL__DMU_MEMBERS_CHANGE:
				{

					QSortFilterProxyModel_NumbersLast * memberModel = static_cast<QSortFilterProxyModel_NumbersLast*>(ui->listView_dmu_members->model());
					if (memberModel == nullptr)
					{
						boost::format msg("Invalid members list view items in DisplayDMUsRegion widget.");
						QMessageBox msgBox;
						msgBox.setText( msg.str().c_str() );
						msgBox.exec();
						return;
					}

					switch (change.change_intention)
					{

						case DATA_CHANGE_INTENTION__ADD:
							{

								if (!change.parent_identifier.uuid || (*change.parent_identifier.uuid).empty())
								{
									boost::format msg("Invalid new DMU member.");
									QMessageBox msgBox;
									msgBox.setText( msg.str().c_str() );
									msgBox.exec();
									return;
								}

								WidgetInstanceIdentifier new_dmu_member(change.parent_identifier);

								QAbstractItemModel * memberSourceModel = memberModel->sourceModel();
								if (memberSourceModel)
								{

									QStandardItemModel * memberStandardSourceModel = nullptr;
									try
									{
										memberStandardSourceModel = dynamic_cast<QStandardItemModel*>(memberSourceModel);
									}
									catch (std::bad_cast &)
									{
										// guess not
										return;
									}

									QStandardItem * item = new QStandardItem();
									std::string text = Table_DMU_Instance::GetDmuMemberDisplayText(new_dmu_member);

									item->setText(text.c_str());
									item->setEditable(false);
									item->setCheckable(true);
									QVariant v;
									v.setValue(new_dmu_member);
									item->setData(v);

									memberStandardSourceModel->appendRow(item);

								}

							}
							break;

						case DATA_CHANGE_INTENTION__REMOVE:
							{

								if (!change.parent_identifier.uuid || (*change.parent_identifier.uuid).empty())
								{
									boost::format msg("Invalid DMU member.");
									QMessageBox msgBox;
									msgBox.setText( msg.str().c_str() );
									msgBox.exec();
									return;
								}

								WidgetInstanceIdentifier const & dmu_member = change.parent_identifier;

								QAbstractItemModel * memberSourceModel = memberModel->sourceModel();

								if (memberSourceModel)
								{

									QStandardItemModel * memberStandardSourceModel = nullptr;
									try
									{
										memberStandardSourceModel = dynamic_cast<QStandardItemModel*>(memberSourceModel);
									}
									catch (std::bad_cast &)
									{
										// guess not
										return;
									}

									std::string text = Table_DMU_Instance::GetDmuMemberDisplayText(dmu_member);
									QList<QStandardItem *> items = memberStandardSourceModel->findItems(text.c_str());
									if (items.count() == 1)
									{
										QStandardItem * dmu_member_to_remove = items.at(0);
										if (dmu_member_to_remove != nullptr)
										{
											QModelIndex index_to_remove = memberStandardSourceModel->indexFromItem(dmu_member_to_remove);
											memberStandardSourceModel->takeRow(index_to_remove.row());

											delete dmu_member_to_remove;
											dmu_member_to_remove = nullptr;
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

								if (!change.parent_identifier.uuid || (*change.parent_identifier.uuid).empty())
								{
									boost::format msg("Invalid DMU member.");
									QMessageBox msgBox;
									msgBox.setText( msg.str().c_str() );
									msgBox.exec();
									return;
								}

								WidgetInstanceIdentifier const & dmu_category = change.parent_identifier;
								WidgetInstanceIdentifiers const & dmu_members = change.child_identifiers;

								ResetDmuMembersPane(dmu_category, dmu_members);

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

	if (proxyModel != nullptr)
	{
		proxyModel->sort(0);
	}

}
