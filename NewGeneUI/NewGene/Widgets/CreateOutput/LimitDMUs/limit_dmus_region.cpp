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
			//project->RegisterInterestInChange(this, DATA_CHANGE_TYPE__INPUT_MODEL__VG_CHANGE, false, "");
		}
	}
	else if (connection_type == NewGeneWidget::RELEASE_CONNECTIONS_INPUT_PROJECT)
	{
		if (inp)
		{
			//inp->UnregisterInterestInChanges(this);
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
			item->setText(text.c_str());
			item->setEditable(false);
			item->setCheckable(false);
			QVariant v;
			v.setValue(dmu_category);
			item->setData(v);
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

		ResetDmuMembersPanes(dmu_category, is_limited, dmu_set_members__all, dmu_set_members_not_limited, dmu_set_members__limited);

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

	QItemSelectionModel * oldSelectionModel = ui->listView_limit_dmus_bottom_left_pane->selectionModel();
	QStandardItemModel * model = new QStandardItemModel();

	int index = 0;
	for (auto & dmu_member : dmu_set_members__not_limited)
	{
		if (dmu_member.uuid && !dmu_member.uuid->empty())
		{

			std::string text = Table_DMU_Instance::GetDmuMemberDisplayText(dmu_member);

			QStandardItem * item = new QStandardItem();
			item->setText(text.c_str());
			item->setEditable(false);
			item->setCheckable(false);
			QVariant v;
			v.setValue(dmu_member);
			item->setData(v);
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

}

void limit_dmus_region::ResetBottomRightPane(WidgetInstanceIdentifiers const & dmu_set_members__limited)
{

	QItemSelectionModel * oldSelectionModel = ui->listView_limit_dmus_bottom_right_pane->selectionModel();
	QStandardItemModel * model = new QStandardItemModel();

	int index = 0;
	for (auto & dmu_member : dmu_set_members__limited)
	{
		if (dmu_member.uuid && !dmu_member.uuid->empty())
		{

			std::string text = Table_DMU_Instance::GetDmuMemberDisplayText(dmu_member);

			QStandardItem * item = new QStandardItem();
			item->setText(text.c_str());
			item->setEditable(false);
			item->setCheckable(false);
			QVariant v;
			v.setValue(dmu_member);
			item->setData(v);
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
									for (auto const & dmu_set_member : dmu_set_members__not_limited__to_add)
									{
										std::string text = Table_DMU_Instance::GetDmuMemberDisplayText(dmu_set_member);

										QStandardItem * item = new QStandardItem();
										item->setText(text.c_str());
										item->setEditable(false);
										item->setCheckable(false);
										QVariant v;
										v.setValue(dmu_set_member);
										item->setData(v);
                                        dmusModelBottomLeft->appendRow( item );

                                        QModelIndex newDmuMemberIndex = dmusModelBottomLeft->indexFromItem(item);
                                        QModelIndex newDmuMemberIndexProxy = modelLeft->mapFromSource(newDmuMemberIndex);
                                        selectionModelBottomLeft->select(newDmuMemberIndexProxy, QItemSelectionModel::Select);
									}

									QItemSelectionModel * selectionModelBottomRight = ui->listView_limit_dmus_bottom_right_pane->selectionModel();
                                    selectionModelBottomRight->clearSelection();
                                    for (auto const & dmu_set_member : dmu_set_members__limited__to_add)
									{
										std::string text = Table_DMU_Instance::GetDmuMemberDisplayText(dmu_set_member);

										QStandardItem * item = new QStandardItem();
										item->setText(text.c_str());
										item->setEditable(false);
										item->setCheckable(false);
										QVariant v;
										v.setValue(dmu_set_member);
										item->setData(v);
                                        dmusModelBottomRight->appendRow( item );

                                        QModelIndex newDmuMemberIndex = dmusModelBottomRight->indexFromItem(item);
                                        QModelIndex newDmuMemberIndexProxy = modelRight->mapFromSource(newDmuMemberIndex);
                                        selectionModelBottomRight->select(newDmuMemberIndexProxy, QItemSelectionModel::Select);
									}

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

	QModelIndex selectedIndex = dmu_selectionModel->currentIndex();
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
    std::string did_checkbox_change;
    if (checked)
    {
        did_checkbox_change = "y";
        ui->limit_dmus_bottom_half_widget->hide();
    }
    else
    {
        did_checkbox_change = "n";
        ui->limit_dmus_bottom_half_widget->show();
    }
    actionItems.push_back(std::make_pair(dmu_category, std::shared_ptr<WidgetActionItem>(static_cast<WidgetActionItem*>(new WidgetActionItem__WidgetInstanceIdentifiers_Plus_String(WidgetInstanceIdentifiers(), did_checkbox_change)))));
    WidgetActionItemRequest_ACTION_LIMIT_DMU_MEMBERS_CHANGE action_request(WIDGET_ACTION_ITEM_REQUEST_REASON__UPDATE_ITEMS, actionItems);
    emit LimitDMUsChange(action_request);

}
