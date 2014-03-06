#include "newgenevariablegroup.h"
#include "ui_newgenevariablegroup.h"

#include <QStandardItem>

#include "../Project/uiprojectmanager.h"
#include "../Project/uiinputproject.h"
#include "../Project/uioutputproject.h"

NewGeneVariableGroup::NewGeneVariableGroup( QWidget * parent, WidgetInstanceIdentifier data_instance_, UIOutputProject * project ) :

	QWidget( parent ),

	NewGeneWidget( WidgetCreationInfo(
										this, // 'this' pointer is cast by compiler to proper Widget instance, which is already created due to order in which base classes appear in class definition
										parent,
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

		project->RegisterInterestInChange(this, DATA_CHANGE_TYPE__OUTPUT_MODEL__VG_CATEGORY_SET_MEMBER_SELECTION, true, *data_instance.uuid);
		project->RegisterInterestInChange(this, DATA_CHANGE_TYPE__INPUT_MODEL__VG_CHANGE, true, *data_instance.uuid);

		UpdateOutputConnections(NewGeneWidget::ESTABLISH_CONNECTIONS_OUTPUT_PROJECT, project);
		WidgetDataItemRequest_VARIABLE_GROUP_VARIABLE_GROUP_INSTANCE request(WIDGET_DATA_ITEM_REQUEST_REASON__REFRESH_ALL_WIDGETS, data_instance);
		emit RefreshWidget(request);

	}

}

NewGeneVariableGroup::~NewGeneVariableGroup()
{
	if (outp)
	{
		outp->UnregisterInterestInChanges(this);
	}
	delete ui;
}

void NewGeneVariableGroup::UpdateOutputConnections(NewGeneWidget::UPDATE_CONNECTIONS_TYPE connection_type, UIOutputProject * project)
{
	NewGeneWidget::UpdateOutputConnections(connection_type, project);

	if (connection_type == NewGeneWidget::ESTABLISH_CONNECTIONS_OUTPUT_PROJECT)
	{
		connect(this, SIGNAL(RefreshWidget(WidgetDataItemRequest_VARIABLE_GROUP_VARIABLE_GROUP_INSTANCE)), outp->getConnector(), SLOT(RefreshWidget(WidgetDataItemRequest_VARIABLE_GROUP_VARIABLE_GROUP_INSTANCE)));
		connect(this, SIGNAL(SignalReceiveVariableItemChanged(WidgetActionItemRequest_ACTION_VARIABLE_GROUP_SET_MEMBER_SELECTION_CHANGED)), outp->getConnector(), SLOT(ReceiveVariableItemChanged(WidgetActionItemRequest_ACTION_VARIABLE_GROUP_SET_MEMBER_SELECTION_CHANGED)));
	}
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

	auto vg_members_and_bools = widget_data.identifiers;
	bool success = ResetAll(vg_members_and_bools);

}

void NewGeneVariableGroup::ReceiveVariableItemChanged(QStandardItem * currentItem)
{

	QStandardItemModel * model = static_cast<QStandardItemModel*>(ui->listView->model());
	if (model == nullptr)
	{
		// Todo: messager error
		return;
	}

	InstanceActionItems actionItems;

	if (currentItem)
	{
		bool checked = false;
		if (currentItem->checkState() == Qt::Checked)
		{
			checked = true;
		}
		QVariant currentIdentifier = currentItem->data();
		WidgetInstanceIdentifier identifier = currentIdentifier.value<WidgetInstanceIdentifier>();
		actionItems.push_back(std::make_pair(identifier, std::shared_ptr<WidgetActionItem>(static_cast<WidgetActionItem*>(new WidgetActionItem__Checkbox(checked)))));
	}

	WidgetActionItemRequest_ACTION_VARIABLE_GROUP_SET_MEMBER_SELECTION_CHANGED action_request(WIDGET_ACTION_ITEM_REQUEST_REASON__UPDATE_ITEMS, actionItems);
	emit SignalReceiveVariableItemChanged(action_request);

}

void NewGeneVariableGroup::HandleChanges(DataChangeMessage const & change_message)
{
	std::for_each(change_message.changes.cbegin(), change_message.changes.cend(), [this](DataChange const & change)
	{
		switch (change.change_type)
		{
			case DATA_CHANGE_TYPE__INPUT_MODEL__VG_CHANGE:
				{

					switch (change.change_intention)
					{

						case DATA_CHANGE_INTENTION__UPDATE:
							{
								WidgetInstanceIdentifiers vg_members = change.child_identifiers;
								std::vector<std::pair<WidgetInstanceIdentifier, bool>> vg_members_and_bools;
								std::for_each(vg_members.cbegin(), vg_members.cend(), [&](WidgetInstanceIdentifier const & identifier)
								{
									vg_members_and_bools.push_back(std::make_pair(identifier, false));
								});
								bool success = ResetAll(vg_members_and_bools);
							}
							break;

						default:
							{

							}
							break;

					}
				}
				break;

			case DATA_CHANGE_TYPE::DATA_CHANGE_TYPE__OUTPUT_MODEL__VG_CATEGORY_SET_MEMBER_SELECTION:
				{
					switch (change.change_intention)
					{

						case DATA_CHANGE_INTENTION__ADD:
						case DATA_CHANGE_INTENTION__REMOVE:
							{
								// This is the OUTPUT model changing.
								// "Add" means to simply add an item that is CHECKED (previously unchecked) -
								// NOT to add a new variable.  That would be input model change type.

								QStandardItemModel * model = static_cast<QStandardItemModel*>(ui->listView->model());
								if (model == nullptr)
								{
									return; // from lambda
								}

								if (change.child_identifiers.size() == 0)
								{
									return; // from lambda
								}

								std::for_each(change.child_identifiers.cbegin(), change.child_identifiers.cend(), [&model, &change, this](WidgetInstanceIdentifier const & child_identifier)
								{
									int number_variables = model->rowCount();
									for (int n=0; n<number_variables; ++n)
									{
										QStandardItem * currentItem = model->item(n);
										if (currentItem)
										{
											bool checked = false;
											if (currentItem->checkState() == Qt::Checked)
											{
												checked = true;
											}
											QVariant currentIdentifier = currentItem->data();
											WidgetInstanceIdentifier identifier = currentIdentifier.value<WidgetInstanceIdentifier>();
											if (identifier.uuid && child_identifier.uuid && *identifier.uuid == *child_identifier.uuid)
											{

												if (change.change_intention == DATA_CHANGE_INTENTION__ADD)
												{
													if (!checked)
													{
														currentItem->setCheckState(Qt::Checked);
													}
												}
												else if (change.change_intention == DATA_CHANGE_INTENTION__REMOVE)
												{
													if (checked)
													{
														currentItem->setCheckState(Qt::Unchecked);
													}
												}

											}

										}
									}
								});

							}
							break;

						case DATA_CHANGE_INTENTION__UPDATE:
							{
								// Should never receive this.
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
			default:
				{
				}
				break;
		}
	});
}

bool NewGeneVariableGroup::ResetAll(std::vector<std::pair<WidgetInstanceIdentifier, bool>> const & vg_members_and_bools)
{

	if (!ui->listView)
	{
		boost::format msg("Invalid list view in NewGeneVariableGroup widget.");
		QMessageBox msgBox;
		msgBox.setText( msg.str().c_str() );
		msgBox.exec();
		return false;
	}

	QStandardItemModel * oldModel = static_cast<QStandardItemModel*>(ui->listView->model());
	if (oldModel != nullptr)
	{
		delete oldModel;
	}

	QItemSelectionModel * oldSelectionModel = ui->listView->selectionModel();
	QStandardItemModel * model = new QStandardItemModel(ui->listView);

	int index = 0;
	std::for_each(vg_members_and_bools.cbegin(), vg_members_and_bools.cend(), [this, &index, &model](std::pair<WidgetInstanceIdentifier, bool> const & vg_member_and_bool)
	{
		WidgetInstanceIdentifier const & identifier = vg_member_and_bool.first;
		bool checked = vg_member_and_bool.second;
		if (identifier.longhand && !identifier.longhand->empty())
		{

			QStandardItem * item = new QStandardItem();
			item->setText(QString(identifier.longhand->c_str()));
			item->setEditable(false);
			item->setCheckable(true);
			item->setCheckState(Qt::Unchecked);
			if (checked)
			{
				item->setCheckState(Qt::Checked);
			}
			QVariant v;
			v.setValue(identifier);
			item->setData(v);
			model->setItem( index, item );

			++index;

		}
	});

	connect(model, SIGNAL(itemChanged(QStandardItem*)), this, SLOT(ReceiveVariableItemChanged(QStandardItem*)));

	ui->listView->setModel(model);
	if (oldSelectionModel) delete oldSelectionModel;

	return true;

}
