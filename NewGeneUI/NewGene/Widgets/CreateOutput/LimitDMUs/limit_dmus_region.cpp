#include "limit_dmus_region.h"
#include "ui_limit_dmus_region.h"

#include "../Project/uiprojectmanager.h"
#include "../Project/uiinputproject.h"
#include "../Project/uioutputproject.h"

#include <QStandardItemModel>
#include "../../Utilities/qsortfilterproxymodel_numberslast.h"

#include <algorithm>
#include <iterator>

#include <QModelIndexList>
#include <QBrush>
#include <QFont>

limit_dmus_region::limit_dmus_region(QWidget *parent) :
	QWidget(parent),
	NewGeneWidget( WidgetCreationInfo(this, parent, WIDGET_NATURE_OUTPUT_WIDGET, LIMIT_DMUS_TAB, true) ), // 'this' pointer is cast by compiler to proper Widget instance, which is already created due to order in which base classes appear in class definition
	ui( new Ui::limit_dmus_region )
{

	ui->setupUi( this );

	PrepareOutputWidget();

	ui->limit_dmus_bottom_half_widget->hide(); // initially, no DMU category is checked

}

limit_dmus_region::~limit_dmus_region()
{

	delete ui;

}

void limit_dmus_region::changeEvent( QEvent * e )
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

void limit_dmus_region::UpdateOutputConnections(NewGeneWidget::UPDATE_CONNECTIONS_TYPE connection_type, UIOutputProject * project)
{
	NewGeneWidget::UpdateOutputConnections(connection_type, project);

	if (connection_type == NewGeneWidget::ESTABLISH_CONNECTIONS_OUTPUT_PROJECT)
	{
		connect(this, SIGNAL(RefreshWidget(WidgetDataItemRequest_LIMIT_DMUS_TAB)), outp->getConnector(), SLOT(RefreshWidget(WidgetDataItemRequest_LIMIT_DMUS_TAB)));
		connect(project->getConnector(), SIGNAL(WidgetDataRefresh(WidgetDataItem_LIMIT_DMUS_TAB)), this, SLOT(WidgetDataRefreshReceive(WidgetDataItem_LIMIT_DMUS_TAB)));
		connect(this, SIGNAL(LimitDMUsChange(WidgetActionItemRequest_ACTION_LIMIT_DMU_MEMBERS_CHANGE)), outp->getConnector(), SLOT(LimitDMUsChange(WidgetActionItemRequest_ACTION_LIMIT_DMU_MEMBERS_CHANGE)));
		if (project)
		{
			project->RegisterInterestInChange(this, DATA_CHANGE_TYPE__OUTPUT_MODEL__LIMIT_DMUS_CHANGE, false, "");
			project->RegisterInterestInChange(this, DATA_CHANGE_TYPE__OUTPUT_MODEL__INPUT_DMU_OR_DMU_MEMBER_CHANGE, false, "");
		}
	}
	else if (connection_type == NewGeneWidget::RELEASE_CONNECTIONS_OUTPUT_PROJECT)
	{
		if (outp)
		{
			outp->UnregisterInterestInChanges(this);
		}
		Empty();
	}
}

void limit_dmus_region::UpdateInputConnections(NewGeneWidget::UPDATE_CONNECTIONS_TYPE connection_type, UIInputProject * project)
{
	NewGeneWidget::UpdateInputConnections(connection_type, project);
	if (connection_type == NewGeneWidget::ESTABLISH_CONNECTIONS_INPUT_PROJECT)
	{
		if (project)
		{
			project->RegisterInterestInChange(this, DATA_CHANGE_TYPE__INPUT_MODEL__DMU_CHANGE, false, "");
			project->RegisterInterestInChange(this, DATA_CHANGE_TYPE__INPUT_MODEL__DMU_MEMBERS_CHANGE, false, "");
		}
	}
	else if (connection_type == NewGeneWidget::RELEASE_CONNECTIONS_INPUT_PROJECT)
	{
		if (inp)
		{
			inp->UnregisterInterestInChanges(this);
		}
	}
}

void limit_dmus_region::RefreshAllWidgets()
{

	if (outp == nullptr)
	{
		Empty();
		return;
	}
	WidgetDataItemRequest_LIMIT_DMUS_TAB request(WIDGET_DATA_ITEM_REQUEST_REASON__REFRESH_ALL_WIDGETS);
	emit RefreshWidget(request);

}

void limit_dmus_region::PrepareItem(QStandardItem * item, std::string const & text, WidgetInstanceIdentifier const & identifier, bool const is_limited)
{

	item->setText(text.c_str());
	item->setEditable(false);
	item->setCheckable(false);
	if (is_limited)
	{
		 QFont font = item->font();
		 font.setBold(true);
		 item->setFont(font);
	}
	else
	{
		QFont font = item->font();
		font.setBold(false);
		item->setFont(font);
	}
	QVariant v;
	v.setValue(identifier);
	item->setData(v);

}

void limit_dmus_region::WidgetDataRefreshReceive(WidgetDataItem_LIMIT_DMUS_TAB widget_data)
{

	Empty();

	UIOutputProject * project = projectManagerUI().getActiveUIOutputProject();
	if (project == nullptr)
	{
		return;
	}

	UIMessager messager(project);

	if (!ui->listView_limit_dmus_top_pane)
	{
		boost::format msg("Invalid top pane list view in Limit DMUs widget.");
		QMessageBox msgBox;
		msgBox.setText( msg.str().c_str() );
		msgBox.exec();
		return;
	}

	QStandardItemModel * oldModel = static_cast<QStandardItemModel*>(ui->listView_limit_dmus_top_pane->model());
	if (oldModel != nullptr)
	{
		delete oldModel;
	}

	QItemSelectionModel * oldSelectionModel = ui->listView_limit_dmus_top_pane->selectionModel();
	QStandardItemModel * model = new QStandardItemModel(ui->listView_limit_dmus_top_pane);

	std::vector<dmu_category_limit_members_info_tuple> & dmu_category_limit_members_info = widget_data.dmu_category_limit_members_info;

	int index = 0;
	for (auto & dmu_category_tuple : dmu_category_limit_members_info)
	{
		WidgetInstanceIdentifier const & dmu_category = std::get<0>(dmu_category_tuple);
		bool const is_limited = std::get<1>(dmu_category_tuple);
		WidgetInstanceIdentifiers const & dmu_set_members_all = std::get<2>(dmu_category_tuple);
		WidgetInstanceIdentifiers const & dmu_set_members_not_limited = std::get<3>(dmu_category_tuple);
		WidgetInstanceIdentifiers const & dmu_set_members_limited = std::get<4>(dmu_category_tuple);
		if (dmu_category.code && !dmu_category.code->empty())
		{

			QStandardItem * item = new QStandardItem();
			std::string text = Table_DMU_Identifier::GetDmuCategoryDisplayText(dmu_category);
			PrepareItem(item, text, dmu_category, is_limited);
			model->setItem( index, item );

			++index;

		}
	};

	model->sort(0);

	ui->listView_limit_dmus_top_pane->setModel(model);
	if (oldSelectionModel) delete oldSelectionModel;

	EmptyDmuMembersPanes();

	connect( ui->listView_limit_dmus_top_pane->selectionModel(), SIGNAL(selectionChanged(QItemSelection, QItemSelection)), this, SLOT(ReceiveDMUSelectionChanged(const QItemSelection &, const QItemSelection &)));

}

void limit_dmus_region::Empty()
{

	if (!ui->listView_limit_dmus_top_pane || !ui->listView_limit_dmus_bottom_left_pane || !ui->listView_limit_dmus_bottom_right_pane)
	{
		boost::format msg("Invalid list view in Limit DMU's tab.");
		QMessageBox msgBox;
		msgBox.setText( msg.str().c_str() );
		msgBox.exec();
		return;
	}

	QSortFilterProxyModel * oldModel = nullptr;
	QAbstractItemModel * oldSourceModel = nullptr;
	QStandardItemModel * oldDmuModel = nullptr;
	QItemSelectionModel * oldSelectionModel = nullptr;

	EmptyDmuMembersPanes();

	oldDmuModel = static_cast<QStandardItemModel*>(ui->listView_limit_dmus_top_pane->model());
	if (oldDmuModel != nullptr)
	{
		delete oldDmuModel;
		oldDmuModel = nullptr;
	}

	oldSelectionModel = ui->listView_limit_dmus_top_pane->selectionModel();
	if (oldSelectionModel != nullptr)
	{
		delete oldSelectionModel;
		oldSelectionModel = nullptr;
	}

}

void limit_dmus_region::EmptyDmuMembersPanes()
{

	EmptyBottomLeftPane();
	EmptyBottomRightPane();

}

void limit_dmus_region::EmptyBottomLeftPane()
{

	QItemSelectionModel * oldSelectionModel = ui->listView_limit_dmus_bottom_left_pane->selectionModel();
	if (oldSelectionModel != nullptr)
	{
		delete oldSelectionModel;
		oldSelectionModel = nullptr;
	}

	QSortFilterProxyModel_NumbersLast * dmuSetMembersProxyModel = static_cast<QSortFilterProxyModel_NumbersLast*>(ui->listView_limit_dmus_bottom_left_pane->model());
	if (dmuSetMembersProxyModel != nullptr)
	{

		QAbstractItemModel * dmuSetMembersSourceModel = dmuSetMembersProxyModel->sourceModel();
		if (dmuSetMembersSourceModel != nullptr)
		{

			delete dmuSetMembersSourceModel;
			dmuSetMembersSourceModel = nullptr;

		}

		delete dmuSetMembersProxyModel;
		dmuSetMembersProxyModel = nullptr;

	}

}

void limit_dmus_region::EmptyBottomRightPane()
{

	QItemSelectionModel * oldSelectionModel = ui->listView_limit_dmus_bottom_right_pane->selectionModel();
	if (oldSelectionModel != nullptr)
	{
		delete oldSelectionModel;
		oldSelectionModel = nullptr;
	}

	QSortFilterProxyModel_NumbersLast * dmuSetMembersProxyModel = static_cast<QSortFilterProxyModel_NumbersLast*>(ui->listView_limit_dmus_bottom_right_pane->model());
	if (dmuSetMembersProxyModel != nullptr)
	{

		QAbstractItemModel * dmuSetMembersSourceModel = dmuSetMembersProxyModel->sourceModel();
		if (dmuSetMembersSourceModel != nullptr)
		{

			delete dmuSetMembersSourceModel;
			dmuSetMembersSourceModel = nullptr;

		}

		delete dmuSetMembersProxyModel;
		dmuSetMembersProxyModel = nullptr;

	}

}

void limit_dmus_region::ReceiveDMUSelectionChanged(const QItemSelection & selected, const QItemSelection & deselected)
{

	UIOutputProject * project = projectManagerUI().getActiveUIOutputProject();
	if (project == nullptr)
	{
		return;
	}

	UIMessager messager(project);

	if (!ui->listView_limit_dmus_top_pane || !ui->listView_limit_dmus_bottom_left_pane || !ui->listView_limit_dmus_bottom_right_pane)
	{
		boost::format msg("Invalid list view in Limit DMU's tab.");
		QMessageBox msgBox;
		msgBox.setText( msg.str().c_str() );
		msgBox.exec();
		return;
	}

	if(!selected.indexes().isEmpty())
	{

		QStandardItemModel * dmuModel = static_cast<QStandardItemModel*>(ui->listView_limit_dmus_top_pane->model());
		QModelIndex selectedIndex = selected.indexes().first();
		QVariant dmu_category_variant = dmuModel->item(selectedIndex.row())->data();
		WidgetInstanceIdentifier dmu_category = dmu_category_variant.value<WidgetInstanceIdentifier>();

		WidgetInstanceIdentifiers dmu_set_members__all = project->model().backend().getInputModel().t_dmu_setmembers.getIdentifiers(*dmu_category.uuid);
		WidgetInstanceIdentifiers dmu_set_members__limited = project->model().backend().t_limit_dmus_set_members.getIdentifiers(*dmu_category.code);
		std::sort(dmu_set_members__all.begin(), dmu_set_members__all.end());
		std::sort(dmu_set_members__limited.begin(), dmu_set_members__limited.end());

		WidgetInstanceIdentifiers dmu_set_members_not_limited;
		std::set_difference(dmu_set_members__all.cbegin(), dmu_set_members__all.cend(), dmu_set_members__limited.cbegin(), dmu_set_members__limited.cend(), std::inserter(dmu_set_members_not_limited, dmu_set_members_not_limited.begin()));

		bool is_limited = project->model().backend().t_limit_dmus_categories.Exists(project->model().backend().getDb(), project->model().backend(), project->model().backend().getInputModel(), *dmu_category.code);
		ui->checkBox_limit_dmus->setChecked(is_limited);
		if (is_limited)
		{
			ui->limit_dmus_bottom_half_widget->show();
		}
		else
		{
			ui->limit_dmus_bottom_half_widget->hide();
		}

		ResetDmuMembersPanes(dmu_category, is_limited, dmu_set_members__all, dmu_set_members_not_limited, dmu_set_members__limited);

	}

}

void limit_dmus_region::ResetDmuMembersPanes(WidgetInstanceIdentifier const & dmu_category, bool const is_limited, WidgetInstanceIdentifiers const & dmu_set_members__all, WidgetInstanceIdentifiers const & dmu_set_members_not_limited, WidgetInstanceIdentifiers const & dmu_set_members__limited)
{

	EmptyDmuMembersPanes();

	ResetBottomLeftPane(dmu_set_members_not_limited);
	ResetBottomRightPane(dmu_set_members__limited);

}

void limit_dmus_region::ResetBottomLeftPane(WidgetInstanceIdentifiers const & dmu_set_members__not_limited)
{

	ui->listView_limit_dmus_bottom_left_pane->setUpdatesEnabled(false);

	QItemSelectionModel * oldSelectionModel = ui->listView_limit_dmus_bottom_left_pane->selectionModel();
	QStandardItemModel * model = new QStandardItemModel();

	int index = 0;
	for (auto & dmu_member : dmu_set_members__not_limited)
	{
		if (dmu_member.uuid && !dmu_member.uuid->empty())
		{

			QStandardItem * item = new QStandardItem();
			std::string text = Table_DMU_Instance::GetDmuMemberDisplayText(dmu_member);
			PrepareItem(item, text, dmu_member, false);
			model->setItem( index, item );

			++index;

		}
	}

	QSortFilterProxyModel_NumbersLast *proxyModel = new QSortFilterProxyModel_NumbersLast(ui->listView_limit_dmus_bottom_left_pane);
	proxyModel->setDynamicSortFilter(true);
	proxyModel->setSourceModel(model);
	proxyModel->sort(0);
	ui->listView_limit_dmus_bottom_left_pane->setModel(proxyModel);
	if (oldSelectionModel) delete oldSelectionModel;

	ui->listView_limit_dmus_bottom_left_pane->setUpdatesEnabled(true);

}

void limit_dmus_region::ResetBottomRightPane(WidgetInstanceIdentifiers const & dmu_set_members__limited)
{

	ui->listView_limit_dmus_bottom_right_pane->setUpdatesEnabled(false);

	QItemSelectionModel * oldSelectionModel = ui->listView_limit_dmus_bottom_right_pane->selectionModel();
	QStandardItemModel * model = new QStandardItemModel();

	int index = 0;
	for (auto & dmu_member : dmu_set_members__limited)
	{
		if (dmu_member.uuid && !dmu_member.uuid->empty())
		{

			QStandardItem * item = new QStandardItem();
			std::string text = Table_DMU_Instance::GetDmuMemberDisplayText(dmu_member);
			PrepareItem(item, text, dmu_member, false);
			model->setItem( index, item );

			++index;

		}
	}

	QSortFilterProxyModel_NumbersLast *proxyModel = new QSortFilterProxyModel_NumbersLast(ui->listView_limit_dmus_bottom_right_pane);
	proxyModel->setDynamicSortFilter(true);
	proxyModel->setSourceModel(model);
	proxyModel->sort(0);
	ui->listView_limit_dmus_bottom_right_pane->setModel(proxyModel);
	if (oldSelectionModel) delete oldSelectionModel;

	ui->listView_limit_dmus_bottom_right_pane->setUpdatesEnabled(true);

}

void limit_dmus_region::HandleChanges(DataChangeMessage const & change_message)
{

	UIInputProject * project = projectManagerUI().getActiveUIInputProject();
	if (project == nullptr)
	{
		return;
	}

	UIMessager messager(project);

	std::for_each(change_message.changes.cbegin(), change_message.changes.cend(), [this](DataChange const & change)
	{

		switch (change.change_type)
		{

			case DATA_CHANGE_TYPE__OUTPUT_MODEL__INPUT_DMU_OR_DMU_MEMBER_CHANGE:
				{

					RefreshAllWidgets();

				}
				break;

			case DATA_CHANGE_TYPE__INPUT_MODEL__DMU_CHANGE:
				{

					switch (change.change_intention)
					{

						case DATA_CHANGE_INTENTION__ADD:
						case DATA_CHANGE_INTENTION__REMOVE:
							{
								RefreshAllWidgets();
							}
							break;

						case DATA_CHANGE_INTENTION__UPDATE:
							{
							}
							break;

						case DATA_CHANGE_INTENTION__RESET_ALL:
							{
							}
							break;

						default:
							{
							}
							break;

					}

				}
				break;

			case DATA_CHANGE_TYPE__INPUT_MODEL__DMU_MEMBERS_CHANGE:
				{

					switch (change.change_intention)
					{

						case DATA_CHANGE_INTENTION__ADD:
						case DATA_CHANGE_INTENTION__REMOVE:
						case DATA_CHANGE_INTENTION__RESET_ALL:
							{
								RefreshAllWidgets();
							}
							break;

						case DATA_CHANGE_INTENTION__UPDATE:
							{
							}
							break;

						default:
							{
							}
							break;

					}

				}
				break;

			case DATA_CHANGE_TYPE::DATA_CHANGE_TYPE__OUTPUT_MODEL__LIMIT_DMUS_CHANGE:
				{

					switch (change.change_intention)
					{

						case DATA_CHANGE_INTENTION__ADD:
							{
							}
							break;

						case DATA_CHANGE_INTENTION__REMOVE:
							{
							}
							break;

						case DATA_CHANGE_INTENTION__UPDATE:
							{
							}
							break;

						case DATA_CHANGE_INTENTION__RESET_ALL:
							{

								if (change.parent_identifier.code && change.parent_identifier.uuid)
								{

									WidgetInstanceIdentifier dmu_category(change.parent_identifier);

									QSortFilterProxyModel_NumbersLast * modelLeft = static_cast<QSortFilterProxyModel_NumbersLast*>(ui->listView_limit_dmus_bottom_left_pane->model());
									if (modelLeft == nullptr)
									{
										return;
									}

									QAbstractItemModel * sourceModelLeft = modelLeft->sourceModel();
									if (sourceModelLeft == nullptr)
									{
										return;
									}

									QStandardItemModel * dmusModelBottomLeft = nullptr;
									try
									{
										dmusModelBottomLeft = dynamic_cast<QStandardItemModel*>(sourceModelLeft);
									}
									catch (std::bad_cast &)
									{
										// guess not
										boost::format msg("Invalid model in Limit DMU's bottom-left pane.");
										QMessageBox msgBox;
										msgBox.setText( msg.str().c_str() );
										msgBox.exec();
										return;
									}

									QSortFilterProxyModel_NumbersLast * modelRight = static_cast<QSortFilterProxyModel_NumbersLast*>(ui->listView_limit_dmus_bottom_right_pane->model());
									if (modelRight == nullptr)
									{
										return;
									}

									QAbstractItemModel * sourceModelRight = modelRight->sourceModel();
									if (sourceModelRight == nullptr)
									{
										return;
									}

									QStandardItemModel * dmusModelBottomRight = nullptr;
									try
									{
										dmusModelBottomRight = dynamic_cast<QStandardItemModel*>(sourceModelRight);
									}
									catch (std::bad_cast &)
									{
										// guess not
										boost::format msg("Invalid model in Limit DMU's bottom-right pane.");
										QMessageBox msgBox;
										msgBox.setText( msg.str().c_str() );
										msgBox.exec();
										return;
									}

									int rowsBottomLeft = modelLeft->rowCount();
									int rowsBottomRight = modelRight->rowCount();

									WidgetInstanceIdentifiers dmu_set_members_bottom_left;
									for (int n=0; n<rowsBottomLeft; ++n)
									{
										QModelIndex indexBottomLeftProxy = modelLeft->index(n, 0);
										QModelIndex indexBottomLeft = modelLeft->mapToSource(indexBottomLeftProxy);
										QStandardItem * itemBottomLeft = dmusModelBottomLeft->itemFromIndex(indexBottomLeft);
										WidgetInstanceIdentifier dmu_set_member = itemBottomLeft->data().value<WidgetInstanceIdentifier>();
										dmu_set_members_bottom_left.push_back(dmu_set_member);
									}

									WidgetInstanceIdentifiers dmu_set_members_bottom_right;
									for (int n=0; n<rowsBottomRight; ++n)
									{
										QModelIndex indexBottomRightProxy = modelRight->index(n, 0);
										QModelIndex indexBottomRight = modelRight->mapToSource(indexBottomRightProxy);
										QStandardItem * itemBottomRight = dmusModelBottomRight->itemFromIndex(indexBottomRight);
										WidgetInstanceIdentifier dmu_set_member = itemBottomRight->data().value<WidgetInstanceIdentifier>();
										dmu_set_members_bottom_right.push_back(dmu_set_member);
									}

									WidgetInstanceIdentifiers dmu_set_members_bottom_left__new = change.child_identifiers;
									WidgetInstanceIdentifiers dmu_set_members_bottom_right__new = change.vector_of_identifiers;

									std::sort(dmu_set_members_bottom_left.begin(), dmu_set_members_bottom_left.end());
									std::sort(dmu_set_members_bottom_right.begin(), dmu_set_members_bottom_right.end());
									std::sort(dmu_set_members_bottom_left__new.begin(), dmu_set_members_bottom_left__new.end());
									std::sort(dmu_set_members_bottom_right__new.begin(), dmu_set_members_bottom_right__new.end());

									WidgetInstanceIdentifiers dmu_set_members__not_limited__to_add;
									WidgetInstanceIdentifiers dmu_set_members__not_limited__to_remove;
									std::set_difference(dmu_set_members_bottom_left__new.cbegin(), dmu_set_members_bottom_left__new.cend(), dmu_set_members_bottom_left.cbegin(), dmu_set_members_bottom_left.cend(),
												   std::inserter(dmu_set_members__not_limited__to_add, dmu_set_members__not_limited__to_add.begin()));
									std::set_difference(dmu_set_members_bottom_left.cbegin(), dmu_set_members_bottom_left.cend(), dmu_set_members_bottom_left__new.cbegin(), dmu_set_members_bottom_left__new.cend(),
												   std::inserter(dmu_set_members__not_limited__to_remove, dmu_set_members__not_limited__to_remove.begin()));

									WidgetInstanceIdentifiers dmu_set_members__limited__to_add;
									WidgetInstanceIdentifiers dmu_set_members__limited__to_remove;
									std::set_difference(dmu_set_members_bottom_right__new.cbegin(), dmu_set_members_bottom_right__new.cend(), dmu_set_members_bottom_right.cbegin(), dmu_set_members_bottom_right.cend(),
												   std::inserter(dmu_set_members__limited__to_add, dmu_set_members__limited__to_add.begin()));
									std::set_difference(dmu_set_members_bottom_right.cbegin(), dmu_set_members_bottom_right.cend(), dmu_set_members_bottom_right__new.cbegin(), dmu_set_members_bottom_right__new.cend(),
												   std::inserter(dmu_set_members__limited__to_remove, dmu_set_members__limited__to_remove.begin()));

									for (auto const & dmu_set_member : dmu_set_members__not_limited__to_remove)
									{
										std::string text = Table_DMU_Instance::GetDmuMemberDisplayText(dmu_set_member);
										QList<QStandardItem *> items = dmusModelBottomLeft->findItems(text.c_str());
										if (items.count() == 1)
										{
											QStandardItem * dmu_member_to_remove = items.at(0);
											if (dmu_member_to_remove != nullptr)
											{
												QModelIndex index_to_remove = dmusModelBottomLeft->indexFromItem(dmu_member_to_remove);
												dmusModelBottomLeft->takeRow(index_to_remove.row());

												delete dmu_member_to_remove;
												dmu_member_to_remove = nullptr;
											}
										}
									}

									for (auto const & dmu_set_member : dmu_set_members__limited__to_remove)
									{
										std::string text = Table_DMU_Instance::GetDmuMemberDisplayText(dmu_set_member);
										QList<QStandardItem *> items = dmusModelBottomRight->findItems(text.c_str());
										if (items.count() == 1)
										{
											QStandardItem * dmu_member_to_remove = items.at(0);
											if (dmu_member_to_remove != nullptr)
											{
												QModelIndex index_to_remove = dmusModelBottomRight->indexFromItem(dmu_member_to_remove);
												dmusModelBottomRight->takeRow(index_to_remove.row());

												delete dmu_member_to_remove;
												dmu_member_to_remove = nullptr;
											}
										}
									}

									QItemSelectionModel * selectionModelBottomLeft = ui->listView_limit_dmus_bottom_left_pane->selectionModel();
									selectionModelBottomLeft->clearSelection();
                                    ui->listView_limit_dmus_bottom_left_pane->setUpdatesEnabled(false);
                                    for (auto const & dmu_set_member : dmu_set_members__not_limited__to_add)
									{
										QStandardItem * item = new QStandardItem();
										std::string text = Table_DMU_Instance::GetDmuMemberDisplayText(dmu_set_member);
										PrepareItem(item, text, dmu_set_member, false);
										dmusModelBottomLeft->appendRow( item );
									}
                                    ui->listView_limit_dmus_bottom_left_pane->setUpdatesEnabled(true);

									QItemSelectionModel * selectionModelBottomRight = ui->listView_limit_dmus_bottom_right_pane->selectionModel();
									selectionModelBottomRight->clearSelection();

                                    ui->listView_limit_dmus_bottom_right_pane->setUpdatesEnabled(false);
									for (auto const & dmu_set_member : dmu_set_members__limited__to_add)
									{
										QStandardItem * item = new QStandardItem();
										std::string text = Table_DMU_Instance::GetDmuMemberDisplayText(dmu_set_member);
										PrepareItem(item, text, dmu_set_member, false);
										dmusModelBottomRight->appendRow( item );

										QModelIndex newDmuMemberIndex = dmusModelBottomRight->indexFromItem(item);
										QModelIndex newDmuMemberIndexProxy = modelRight->mapFromSource(newDmuMemberIndex);
                                        //selectionModelBottomRight->select(newDmuMemberIndexProxy, QItemSelectionModel::Select);
									}
                                    ui->listView_limit_dmus_bottom_right_pane->setUpdatesEnabled(true);

								}

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

bool limit_dmus_region::GetSelectedDmuCategory(WidgetInstanceIdentifier & dmu_category)
{

	QItemSelectionModel * dmu_selectionModel = ui->listView_limit_dmus_top_pane->selectionModel();
	if (dmu_selectionModel == nullptr)
	{
		boost::format msg("Invalid selection in Limit DMU's DMU category pane.");
		QMessageBox msgBox;
		msgBox.setText( msg.str().c_str() );
		msgBox.exec();
		return false;
	}

	QModelIndexList selectedIndexes = dmu_selectionModel->selectedIndexes();

	if (selectedIndexes.empty())
	{
		// No selection
		return false;
	}

	if (selectedIndexes.size() > 1)
	{
		boost::format msg("Two items cannot be simultaneously selected in the top pane of the Limit DMU's tab.");
		QMessageBox msgBox;
		msgBox.setText( msg.str().c_str() );
		msgBox.exec();
		return false;
	}

	QModelIndex selectedIndex = selectedIndexes[0];

	if (!selectedIndex.isValid())
	{
		// No selection
		return false;
	}

	QStandardItemModel * dmuModel = static_cast<QStandardItemModel*>(ui->listView_limit_dmus_top_pane->model());
	if (dmuModel == nullptr)
	{
		boost::format msg("Invalid model in DisplayDMUsRegion DMU category widget.");
		QMessageBox msgBox;
		msgBox.setText( msg.str().c_str() );
		msgBox.exec();
		return false;
	}

	QVariant dmu_variant = dmuModel->item(selectedIndex.row())->data();
	dmu_category = dmu_variant.value<WidgetInstanceIdentifier>();

	return true;

}

void limit_dmus_region::on_pushButton_limit_dmus_move_right_clicked()
{

	// Get selected DMU category
	WidgetInstanceIdentifier dmu_category;
	bool is_selected = GetSelectedDmuCategory(dmu_category);
	if (!is_selected)
	{
		return;
	}

	QItemSelectionModel * dmus_selectionModel = ui->listView_limit_dmus_bottom_left_pane->selectionModel();
	if (dmus_selectionModel == nullptr)
	{
		boost::format msg("Invalid selection in Limit DMU's bottom-left pane.");
		QMessageBox msgBox;
		msgBox.setText( msg.str().c_str() );
		msgBox.exec();
		return;
	}

	QSortFilterProxyModel_NumbersLast * model = static_cast<QSortFilterProxyModel_NumbersLast*>(ui->listView_limit_dmus_bottom_left_pane->model());
	if (model == nullptr)
	{
		return;
	}

	QAbstractItemModel * sourceModel = model->sourceModel();
	if (sourceModel == nullptr)
	{
		return;
	}

	QStandardItemModel * dmusModel = nullptr;
	try
	{
		dmusModel = dynamic_cast<QStandardItemModel*>(sourceModel);
	}
	catch (std::bad_cast &)
	{
		// guess not
		boost::format msg("Invalid model in Limit DMU's bottom-left pane.");
		QMessageBox msgBox;
		msgBox.setText( msg.str().c_str() );
		msgBox.exec();
		return;
	}

	WidgetInstanceIdentifiers dmuCategoriesToMoveToLimitingList;
	QModelIndexList selectedDMUs = dmus_selectionModel->selectedRows();
	for (auto & selectedDmuIndexProxy : selectedDMUs)
	{
		QModelIndex selectedDmuIndex = model->mapToSource(selectedDmuIndexProxy);
		QVariant dmu_category_variant = dmusModel->item(selectedDmuIndex.row())->data();
		WidgetInstanceIdentifier dmu_category = dmu_category_variant.value<WidgetInstanceIdentifier>();
		dmuCategoriesToMoveToLimitingList.push_back(dmu_category);
	}

    if (dmuCategoriesToMoveToLimitingList.size() > 100)
    {
        QMessageBox::StandardButton reply;
        reply = QMessageBox::question(nullptr, QString("Excessive number of DMU's"), QString("You have selected a large number of DMU's to limit by.  NewGene is not intended to support a large number of limiting DMU's.  If you proceed, this operation may take a long time after you click 'Yes'.  Proceed?"), QMessageBox::StandardButtons(QMessageBox::Yes | QMessageBox::No));
        if (reply != QMessageBox::Yes)
        {
            return;
        }
    }

	if (!dmuCategoriesToMoveToLimitingList.empty())
	{

		// Submit the action
		InstanceActionItems actionItems;
		std::string no_checkbox_change; // empty string passed to back end means no change in checkbox state
		actionItems.push_back(std::make_pair(dmu_category, std::shared_ptr<WidgetActionItem>(static_cast<WidgetActionItem*>(new WidgetActionItem__WidgetInstanceIdentifiers_Plus_String(dmuCategoriesToMoveToLimitingList, no_checkbox_change)))));
		WidgetActionItemRequest_ACTION_LIMIT_DMU_MEMBERS_CHANGE action_request(WIDGET_ACTION_ITEM_REQUEST_REASON__ADD_ITEMS, actionItems);

		emit LimitDMUsChange(action_request);

	}

}

void limit_dmus_region::on_pushButton_limit_dmus_move_left_clicked()
{

	// Get selected DMU category
	WidgetInstanceIdentifier dmu_category;
	bool is_selected = GetSelectedDmuCategory(dmu_category);
	if (!is_selected)
	{
		return;
	}

	QItemSelectionModel * dmus_selectionModel = ui->listView_limit_dmus_bottom_right_pane->selectionModel();
	if (dmus_selectionModel == nullptr)
	{
		boost::format msg("Invalid selection in Limit DMU's bottom-left pane.");
		QMessageBox msgBox;
		msgBox.setText( msg.str().c_str() );
		msgBox.exec();
		return;
	}

	QSortFilterProxyModel_NumbersLast * model = static_cast<QSortFilterProxyModel_NumbersLast*>(ui->listView_limit_dmus_bottom_right_pane->model());
	if (model == nullptr)
	{
		return;
	}

	QAbstractItemModel * sourceModel = model->sourceModel();
	if (sourceModel == nullptr)
	{
		return;
	}

	QStandardItemModel * dmusModel = nullptr;
	try
	{
		dmusModel = dynamic_cast<QStandardItemModel*>(sourceModel);
	}
	catch (std::bad_cast &)
	{
		// guess not
		boost::format msg("Invalid model in Limit DMU's bottom-right pane.");
		QMessageBox msgBox;
		msgBox.setText( msg.str().c_str() );
		msgBox.exec();
		return;
	}

	WidgetInstanceIdentifiers dmuCategoriesToMoveToNonLimitingList;
	QModelIndexList selectedDMUs = dmus_selectionModel->selectedRows();
	for (auto & selectedDmuIndexProxy : selectedDMUs)
	{
		QModelIndex selectedDmuIndex = model->mapToSource(selectedDmuIndexProxy);
		QVariant dmu_category_variant = dmusModel->item(selectedDmuIndex.row())->data();
		WidgetInstanceIdentifier dmu_category = dmu_category_variant.value<WidgetInstanceIdentifier>();
		dmuCategoriesToMoveToNonLimitingList.push_back(dmu_category);
	}

	if (!dmuCategoriesToMoveToNonLimitingList.empty())
	{

		// Submit the action
		InstanceActionItems actionItems;
		std::string no_checkbox_change; // empty string passed to back end means no change in checkbox state
		actionItems.push_back(std::make_pair(dmu_category, std::shared_ptr<WidgetActionItem>(static_cast<WidgetActionItem*>(new WidgetActionItem__WidgetInstanceIdentifiers_Plus_String(dmuCategoriesToMoveToNonLimitingList, no_checkbox_change)))));
		WidgetActionItemRequest_ACTION_LIMIT_DMU_MEMBERS_CHANGE action_request(WIDGET_ACTION_ITEM_REQUEST_REASON__REMOVE_ITEMS, actionItems);

		emit LimitDMUsChange(action_request);

	}

}

void limit_dmus_region::on_checkBox_limit_dmus_toggled(bool checked)
{

	// Get selected DMU category
	WidgetInstanceIdentifier dmu_category;
	bool is_selected = GetSelectedDmuCategory(dmu_category);
	if (!is_selected)
	{
		return;
	}

	// Submit the action: Note: no response is necessary
	InstanceActionItems actionItems;
	std::string checkbox_state;
	if (checked)
	{
		checkbox_state = "y";
		ui->limit_dmus_bottom_half_widget->show();
	}
	else
	{
		checkbox_state = "n";
		ui->limit_dmus_bottom_half_widget->hide();
	}

	std::string text = Table_DMU_Identifier::GetDmuCategoryDisplayText(dmu_category);
	QStandardItemModel * model = static_cast<QStandardItemModel*>(ui->listView_limit_dmus_top_pane->model());
	QList<QStandardItem *> items = model->findItems(text.c_str());
	if (items.count() == 1)
	{
		QStandardItem * dmu_category_item = items.at(0);
		if (dmu_category_item != nullptr)
		{
			QFont font = dmu_category_item->font();
			font.setBold(checked);
			dmu_category_item->setFont(font);
		}
	}

	actionItems.push_back(std::make_pair(dmu_category, std::shared_ptr<WidgetActionItem>(static_cast<WidgetActionItem*>(new WidgetActionItem__WidgetInstanceIdentifiers_Plus_String(WidgetInstanceIdentifiers(), checkbox_state)))));
	WidgetActionItemRequest_ACTION_LIMIT_DMU_MEMBERS_CHANGE action_request(WIDGET_ACTION_ITEM_REQUEST_REASON__UPDATE_ITEMS, actionItems);
	emit LimitDMUsChange(action_request);

}

void limit_dmus_region::on_toolButtonSelectAllBottomLeft_clicked()
{

	QSortFilterProxyModel_NumbersLast * modelLeft = static_cast<QSortFilterProxyModel_NumbersLast*>(ui->listView_limit_dmus_bottom_left_pane->model());
	if (modelLeft == nullptr)
	{
		return;
	}

	QAbstractItemModel * sourceModelLeft = modelLeft->sourceModel();
	if (sourceModelLeft == nullptr)
	{
		return;
	}

	QStandardItemModel * dmusModelBottomLeft = nullptr;
	try
	{
		dmusModelBottomLeft = dynamic_cast<QStandardItemModel*>(sourceModelLeft);
	}
	catch (std::bad_cast &)
	{
		// guess not
		boost::format msg("Invalid model in Limit DMU's bottom-left pane.");
		QMessageBox msgBox;
		msgBox.setText( msg.str().c_str() );
		msgBox.exec();
		return;
	}

	QItemSelectionModel * selectionModelBottomLeft = ui->listView_limit_dmus_bottom_left_pane->selectionModel();
	if (selectionModelBottomLeft == nullptr)
	{
		return;
	}

	selectionModelBottomLeft->clearSelection();

	int rows = dmusModelBottomLeft->rowCount();

	QModelIndex topLeft = dmusModelBottomLeft->index(0, 0);
	QModelIndex bottomRight = dmusModelBottomLeft->index(rows-1, 0);
	selectionModelBottomLeft->select(QItemSelection(topLeft, bottomRight), QItemSelectionModel::Select);

}

void limit_dmus_region::on_toolButtonDeselectAllBottomLeft_clicked()
{

	QItemSelectionModel * selectionModelBottomLeft = ui->listView_limit_dmus_bottom_left_pane->selectionModel();
	if (selectionModelBottomLeft == nullptr)
	{
		return;
	}

	selectionModelBottomLeft->clearSelection();

}

void limit_dmus_region::on_toolButtonSelectAllBottomRight_clicked()
{

	QSortFilterProxyModel_NumbersLast * modelRight = static_cast<QSortFilterProxyModel_NumbersLast*>(ui->listView_limit_dmus_bottom_right_pane->model());
	if (modelRight == nullptr)
	{
		return;
	}

	QAbstractItemModel * sourceModelRight = modelRight->sourceModel();
	if (sourceModelRight == nullptr)
	{
		return;
	}

	QStandardItemModel * dmusModelBottomRight = nullptr;
	try
	{
		dmusModelBottomRight = dynamic_cast<QStandardItemModel*>(sourceModelRight);
	}
	catch (std::bad_cast &)
	{
		// guess not
		boost::format msg("Invalid model in Limit DMU's bottom-right pane.");
		QMessageBox msgBox;
		msgBox.setText( msg.str().c_str() );
		msgBox.exec();
		return;
	}

	QItemSelectionModel * selectionModelBottomRight = ui->listView_limit_dmus_bottom_right_pane->selectionModel();
	if (selectionModelBottomRight == nullptr)
	{
		return;
	}

	selectionModelBottomRight->clearSelection();

	int rows = dmusModelBottomRight->rowCount();

	QModelIndex topLeft = dmusModelBottomRight->index(0, 0);
	QModelIndex bottomRight = dmusModelBottomRight->index(rows-1, 0);
	selectionModelBottomRight->select(QItemSelection(topLeft, bottomRight), QItemSelectionModel::Select);

}

void limit_dmus_region::on_toolButtonDeselectAllBottomRight_clicked()
{

	QItemSelectionModel * selectionModelBottomRight = ui->listView_limit_dmus_bottom_right_pane->selectionModel();
	if (selectionModelBottomRight == nullptr)
	{
		return;
	}

	selectionModelBottomRight->clearSelection();

}
