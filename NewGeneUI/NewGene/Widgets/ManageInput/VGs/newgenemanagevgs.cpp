#include "newgenemanagevgs.h"
#include "ui_newgenemanagevgs.h"

NewGeneManageVGs::NewGeneManageVGs( QWidget * parent ) :
	QWidget( parent ),
	NewGeneWidget( WidgetCreationInfo(this, WIDGET_NATURE_INPUT_WIDGET, MANAGE_VGS_WIDGET, true) ), // 'this' pointer is cast by compiler to proper Widget instance, which is already created due to order in which base classes appear in class definition
	ui( new Ui::NewGeneManageVGs )
{
	ui->setupUi( this );
	PrepareInputWidget(true);
}

NewGeneManageVGs::~NewGeneManageVGs()
{
	delete ui;
}

void NewGeneManageVGs::changeEvent( QEvent * e )
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

void NewGeneManageVGs::UpdateInputConnections(NewGeneWidget::UPDATE_CONNECTIONS_TYPE connection_type, UIInputProject * project)
{

	NewGeneWidget::UpdateInputConnections(connection_type, project);

	if (connection_type == NewGeneWidget::ESTABLISH_CONNECTIONS_INPUT_PROJECT)
	{
		connect(this, SIGNAL(RefreshWidget(WidgetDataItemRequest_MANAGE_VGS_WIDGET)), inp->getConnector(), SLOT(RefreshWidget(WidgetDataItemRequest_MANAGE_VGS_WIDGET)));
		connect(project->getConnector(), SIGNAL(WidgetDataRefresh(WidgetDataItem_MANAGE_VGS_WIDGET)), this, SLOT(WidgetDataRefreshReceive(WidgetDataItem_MANAGE_VGS_WIDGET)));
		connect(this, SIGNAL(CreateVG(WidgetActionItemRequest_ACTION_CREATE_VG)), inp->getConnector(), SLOT(AddUOA(WidgetActionItemRequest_ACTION_CREATE_VG)));
		connect(this, SIGNAL(DeleteVG(WidgetActionItemRequest_ACTION_DELETE_VG)), inp->getConnector(), SLOT(DeleteUOA(WidgetActionItemRequest_ACTION_DELETE_VG)));
		connect(this, SIGNAL(RefreshVG(WidgetActionItemRequest_ACTION_REFRESH_VG)), inp->getConnector(), SLOT(DeleteUOA(WidgetActionItemRequest_ACTION_REFRESH_VG)));

		if (project)
		{
			project->RegisterInterestInChange(this, DATA_CHANGE_TYPE__INPUT_MODEL__VG_CHANGE, false, "");
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

void NewGeneManageVGs::UpdateOutputConnections(NewGeneWidget::UPDATE_CONNECTIONS_TYPE connection_type, UIOutputProject * project)
{

	NewGeneWidget::UpdateOutputConnections(connection_type, project);

	if (connection_type == NewGeneWidget::ESTABLISH_CONNECTIONS_OUTPUT_PROJECT)
	{
		if (project)
		{
			connect(this, SIGNAL(DeleteVG(WidgetActionItemRequest_ACTION_DELETE_UOA)), project->getConnector(), SLOT(DeleteVG(WidgetActionItemRequest_ACTION_DELETE_UOA)));
		}
	}
	else if (connection_type == NewGeneWidget::RELEASE_CONNECTIONS_INPUT_PROJECT)
	{
		if (project)
		{
		}
	}

}

void NewGeneManageVGs::RefreshAllWidgets()
{
	if (inp == nullptr)
	{
		Empty();
		return;
	}
	WidgetDataItemRequest_MANAGE_VGS_WIDGET request(WIDGET_DATA_ITEM_REQUEST_REASON__REFRESH_ALL_WIDGETS);
	emit RefreshWidget(request);
}

void NewGeneManageVGs::WidgetDataRefreshReceive(WidgetDataItem_MANAGE_VGS_WIDGET widget_data)
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

void NewGeneManageVGs::Empty()
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

void NewGeneManageVGs::HandleChanges(DataChangeMessage const & change_message)
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

			case DATA_CHANGE_TYPE::DATA_CHANGE_TYPE__INPUT_MODEL__VG_CHANGE:
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

								int numberItems = itemModel->rowCount();
								for(int currentItem = 0; currentItem < numberItems; ++currentItem)
								{
									QStandardItem * uoa_to_remove_item = itemModel->item(currentItem);
									if (uoa_to_remove_item != nullptr)
									{

										QVariant uoa_and_dmu_categories_variant = uoa_to_remove_item->data();
										std::pair<WidgetInstanceIdentifier, WidgetInstanceIdentifiers> const uoa_and_dmu_categories = uoa_and_dmu_categories_variant.value<std::pair<WidgetInstanceIdentifier, WidgetInstanceIdentifiers>>();
										WidgetInstanceIdentifier const & uoa = uoa_and_dmu_categories.first;

										if (uoa.IsEqual(WidgetInstanceIdentifier::EQUALITY_CHECK_TYPE__UUID_PLUS_STRING_CODE, uoa_category))
										{

											QModelIndex index_to_remove = itemModel->indexFromItem(uoa_to_remove_item);
											itemModel->takeRow(index_to_remove.row());

											delete uoa_to_remove_item;
											uoa_to_remove_item = nullptr;

											QItemSelectionModel * selectionModel = ui->listViewManageUOAs->selectionModel();
											if (selectionModel != nullptr)
											{
												selectionModel->clearSelection();
											}

											break;

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

bool NewGeneManageVGs::GetSelectedVG(WidgetInstanceIdentifier & vg_category, WidgetInstanceIdentifier & uoa_category)
{

	QItemSelectionModel * uoa_selectionModel = ui->listViewManageUOAs->selectionModel();
	if (uoa_selectionModel == nullptr)
	{
		boost::format msg("Invalid selection in NewGeneManageUOAs widget.");
		QMessageBox msgBox;
		msgBox.setText( msg.str().c_str() );
		msgBox.exec();
		return false;
	}

	QModelIndex selectedIndex = uoa_selectionModel->currentIndex();
	if (!selectedIndex.isValid())
	{
		// No selection
		return false;
	}

	QStandardItemModel * uoaModel = static_cast<QStandardItemModel*>(ui->listViewManageUOAs->model());
	if (uoaModel == nullptr)
	{
		boost::format msg("Invalid model in NewGeneManageUOAs DMU category widget.");
		QMessageBox msgBox;
		msgBox.setText( msg.str().c_str() );
		msgBox.exec();
		return false;
	}

	QVariant uoa_and_dmu_categories_variant = uoaModel->item(selectedIndex.row())->data();
	std::pair<WidgetInstanceIdentifier, WidgetInstanceIdentifiers> uoa_and_dmu_categories = uoa_and_dmu_categories_variant.value<std::pair<WidgetInstanceIdentifier, WidgetInstanceIdentifiers>>();
	uoa_category = uoa_and_dmu_categories.first;
	uoa_dmu_categories = uoa_and_dmu_categories.second;

	return true;

}
