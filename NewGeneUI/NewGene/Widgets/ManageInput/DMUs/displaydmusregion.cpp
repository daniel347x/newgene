#include "displaydmusregion.h"
#include "ui_displaydmusregion.h"

#include <QStandardItem>
#include <QInputDialog>
#include <QDialog>
#include <QFormLayout>
#include <QList>
#include <QLineEdit>
#include <QString>
#include <QLabel>
#include <QDialogButtonBox>
#include <QModelIndexList>

#ifndef Q_MOC_RUN
#	include <boost/algorithm/string.hpp>
#	include <boost/regex.hpp>
#endif

#include "../Project/uiprojectmanager.h"
#include "../Project/uiinputproject.h"

DisplayDMUsRegion::DisplayDMUsRegion(QWidget *parent) :
	QWidget(parent),
	NewGeneWidget( WidgetCreationInfo(this, parent, WIDGET_NATURE_INPUT_WIDGET, MANAGE_DMUS_WIDGET, true) ), // 'this' pointer is cast by compiler to proper Widget instance, which is already created due to order in which base classes appear in class definition
	ui(new Ui::DisplayDMUsRegion)
{

	ui->setupUi(this);

	PrepareInputWidget(true);

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

		if (project)
		{
			project->RegisterInterestInChange(this, DATA_CHANGE_TYPE__INPUT_MODEL__DMU_CHANGE, false, "");
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

void DisplayDMUsRegion::UpdateOutputConnections(NewGeneWidget::UPDATE_CONNECTIONS_TYPE connection_type, UIOutputProject * project)
{

	NewGeneWidget::UpdateOutputConnections(connection_type, project);

	if (connection_type == NewGeneWidget::ESTABLISH_CONNECTIONS_OUTPUT_PROJECT)
	{
		if (project)
		{
			connect(this, SIGNAL(DeleteDMU(WidgetActionItemRequest_ACTION_DELETE_DMU)), project->getConnector(), SLOT(DeleteDMU(WidgetActionItemRequest_ACTION_DELETE_DMU)));
			project->RegisterInterestInChange(this, DATA_CHANGE_TYPE__INPUT_MODEL__DMU_CHANGE, false, "");
		}
	}
	else if (connection_type == NewGeneWidget::RELEASE_CONNECTIONS_INPUT_PROJECT)
	{
		if (project)
		{
			project->UnregisterInterestInChanges(this);
		}
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

	model->sort(0);

	ui->listView_dmus->setModel(model);
	if (oldSelectionModel) delete oldSelectionModel;

	connect( ui->listView_dmus->selectionModel(), SIGNAL(selectionChanged(QItemSelection, QItemSelection)), this, SLOT(ReceiveDMUSelectionChanged(const QItemSelection &, const QItemSelection &)));

}

void DisplayDMUsRegion::Empty()
{

	if (!ui->listView_dmus || !ui->listView_dmu_members)
	{
		boost::format msg("Invalid list view in DisplayDMUsRegion widget.");
		QMessageBox msgBox;
		msgBox.setText( msg.str().c_str() );
		msgBox.exec();
		return;
	}

	QStandardItemModel * oldModel = nullptr;
	QItemSelectionModel * oldSelectionModel = nullptr;

	oldModel = static_cast<QStandardItemModel*>(ui->listView_dmu_members->model());
	if (oldModel != nullptr)
	{
		delete oldModel;
		oldModel = nullptr;
	}

	oldSelectionModel = ui->listView_dmu_members->selectionModel();
	if (oldSelectionModel != nullptr)
	{
		delete oldSelectionModel;
		oldSelectionModel = nullptr;
	}

	oldModel = static_cast<QStandardItemModel*>(ui->listView_dmus->model());
	if (oldModel != nullptr)
	{
		delete oldModel;
		oldModel = nullptr;
	}

	oldSelectionModel = ui->listView_dmus->selectionModel();
	if (oldSelectionModel != nullptr)
	{
		delete oldSelectionModel;
		oldSelectionModel = nullptr;
	}

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
			if (dmu_member.uuid && !dmu_member.uuid->empty())
			{

				QStandardItem * item = new QStandardItem();
				QString text;
				if (dmu_member.code && !dmu_member.code->empty())
				{
					text += dmu_member.code->c_str();
				}
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
				bool use_parentheses = false;
				if ((dmu_member.code && !dmu_member.code->empty()) || (dmu_member.longhand && !dmu_member.longhand->empty()))
				{
					use_parentheses = true;
				}
				if (use_parentheses)
				{
					text += " (";
				}
				text += dmu_member.uuid->c_str();
				if (use_parentheses)
				{
					text += ")";
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

		model->sort(0);

		ui->listView_dmu_members->setModel(model);
		if (oldSelectionModel) delete oldSelectionModel;
	}

}

void DisplayDMUsRegion::on_pushButton_add_dmu_clicked()
{

	// From http://stackoverflow.com/a/17512615/368896
	QDialog dialog(this);
	QFormLayout form(&dialog);
	form.addRow(new QLabel("Create New DMU"));
	QList<QLineEdit *> fields;
	QLineEdit *lineEditName = new QLineEdit(&dialog);
	QString labelName = QString("Enter Decision Making Unit (DMU) category name:");
	form.addRow(labelName, lineEditName);
	fields << lineEditName;
	QLineEdit *lineEditDescription = new QLineEdit(&dialog);
	QString labelDescription = QString("Description:");
	form.addRow(labelDescription, lineEditDescription);
	fields << lineEditDescription;

	// Add some standard buttons (Cancel/Ok) at the bottom of the dialog
	QDialogButtonBox buttonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal, &dialog);
	form.addRow(&buttonBox);

	QObject::connect(&buttonBox, SIGNAL(accepted()), &dialog, SLOT(accept()));
	QObject::connect(&buttonBox, SIGNAL(rejected()), &dialog, SLOT(reject()));

	std::string proposed_dmu_name;
	std::string dmu_description;
	if (dialog.exec() == QDialog::Accepted) {
		QLineEdit * proposed_dmu_name_field = fields[0];
		QLineEdit * dmu_description_field = fields[1];
		if (proposed_dmu_name_field && dmu_description_field)
		{
			proposed_dmu_name = proposed_dmu_name_field->text().toStdString();
			dmu_description = dmu_description_field->text().toStdString();
		}
		else
		{
			boost::format msg("Unable to determine new DMU name and description.");
			throw NewGeneException() << newgene_error_description(msg.str());
		}
	}
	else
	{
		return;
	}

	boost::trim(proposed_dmu_name);
	boost::trim(dmu_description);

	if (proposed_dmu_name.empty())
	{
		boost::format msg("The DMU category you entered is empty.");
		QMessageBox msgBox;
		msgBox.setText( msg.str().c_str() );
		msgBox.exec();
		return;
	}

	std::string regex_string("([a-zA-Z_][a-zA-Z0-9_]*)");
	boost::regex regex(regex_string);
	boost::cmatch matches;

	std::string invalid_string;

	bool valid = false;
	if (boost::regex_match(proposed_dmu_name.c_str(), matches, regex))
	{
		// matches[0] contains the original string.  matches[n]
		// contains a sub_match object for each matching
		// subexpression
		// ... see http://www.onlamp.com/pub/a/onlamp/2006/04/06/boostregex.html?page=3
		// for an exapmle usage
		if (matches.size() == 2)
		{
			std::string the_dmu_string_match(matches[1].first, matches[1].second);

			if (the_dmu_string_match.size() <= 255)
			{
				if (the_dmu_string_match == proposed_dmu_name)
				{
					valid = true;
				}
			}
			else
			{
				invalid_string = ": The length is too long.";
			}

		}
	}

	if (!valid)
	{
		boost::format msg("The DMU category you entered is invalid%1%");
		msg % invalid_string;
		QMessageBox msgBox;
		msgBox.setText( msg.str().c_str() );
		msgBox.exec();
		return;
	}

	if (dmu_description.size() > 4096)
	{
		boost::format msg("The description is too long.");
		QMessageBox msgBox;
		msgBox.setText( msg.str().c_str() );
		msgBox.exec();
		return;
	}

	InstanceActionItems actionItems;
	actionItems.push_back(std::make_pair(WidgetInstanceIdentifier(), std::shared_ptr<WidgetActionItem>(static_cast<WidgetActionItem*>(new WidgetActionItem__StringVector(std::vector<std::string>{proposed_dmu_name, dmu_description})))));
	WidgetActionItemRequest_ACTION_ADD_DMU action_request(WIDGET_ACTION_ITEM_REQUEST_REASON__ADD_ITEMS, actionItems);

	emit AddDMU(action_request);

}

void DisplayDMUsRegion::on_pushButton_delete_dmu_clicked()
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

	QItemSelectionModel * dmu_selectionModel = ui->listView_dmus->selectionModel();
	if (dmu_selectionModel == nullptr)
	{
		boost::format msg("Invalid selection in DisplayDMUsRegion widget.");
		QMessageBox msgBox;
		msgBox.setText( msg.str().c_str() );
		msgBox.exec();
		return;
	}

	QModelIndex selectedIndex = dmu_selectionModel->currentIndex();
	if (!selectedIndex.isValid())
	{
		// No selection
		return;
	}

	QStandardItemModel * dmuModel = static_cast<QStandardItemModel*>(ui->listView_dmus->model());
	if (dmuModel == nullptr)
	{
		boost::format msg("Invalid model in DisplayDMUsRegion DMU category widget.");
		QMessageBox msgBox;
		msgBox.setText( msg.str().c_str() );
		msgBox.exec();
		return;
	}

	QVariant dmu_and_members_variant = dmuModel->item(selectedIndex.row())->data();
	std::pair<WidgetInstanceIdentifier, WidgetInstanceIdentifiers> dmu_and_members = dmu_and_members_variant.value<std::pair<WidgetInstanceIdentifier, WidgetInstanceIdentifiers>>();
	WidgetInstanceIdentifier & dmu = dmu_and_members.first;
	WidgetInstanceIdentifiers & dmu_members = dmu_and_members.second;

	InstanceActionItems actionItems;
	actionItems.push_back(std::make_pair(WidgetInstanceIdentifier(dmu), std::shared_ptr<WidgetActionItem>(static_cast<WidgetActionItem*>(new WidgetActionItem__WidgetInstanceIdentifier(dmu)))));
	WidgetActionItemRequest_ACTION_DELETE_DMU action_request(WIDGET_ACTION_ITEM_REQUEST_REASON__REMOVE_ITEMS, actionItems);
	emit DeleteDMU(action_request);

}

void DisplayDMUsRegion::on_pushButton_refresh_dmu_members_from_file_clicked()
{
	//WidgetActionItemRequest_ACTION_REFRESH_DMUS_FROM_FILE action_request(WIDGET_ACTION_ITEM_REQUEST_REASON__REMOVE_ITEMS, actionItems);
	//emit RefreshDMUsFromFile(action_request);
}

void DisplayDMUsRegion::on_pushButton_add_dmu_member_by_hand_clicked()
{
	WidgetActionItemRequest_ACTION_ADD_DMU_MEMBERS dummy;
	emit AddDMUMembers(dummy);
}

void DisplayDMUsRegion::on_pushButton_delete_selected_dmu_members_clicked()
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

	QItemSelectionModel * dmu_selectionModel = ui->listView_dmus->selectionModel();
	if (dmu_selectionModel == nullptr)
	{
		boost::format msg("Invalid selection in DisplayDMUsRegion widget.");
		QMessageBox msgBox;
		msgBox.setText( msg.str().c_str() );
		msgBox.exec();
		return;
	}

	QModelIndexList selectedDmuMembers = dmu_selectionModel->selectedRows();

	QStandardItemModel * dmuMembersModel = static_cast<QStandardItemModel*>(ui->listView_dmu_members->model());
	if (dmuMembersModel == nullptr)
	{
		boost::format msg("Invalid model in DisplayDMUsRegion DMU category widget.");
		QMessageBox msgBox;
		msgBox.setText( msg.str().c_str() );
		msgBox.exec();
		return;
	}

	InstanceActionItems actionItems;

	for (int i = 0; i < selectedDmuMembers.size(); ++i)
	{

		QModelIndex selectedIndex = selectedDmuMembers.at(i);

		QVariant dmu_member_variant = dmuMembersModel->item(selectedIndex.row())->data();
		WidgetInstanceIdentifier dmu_member = dmu_member_variant.value<WidgetInstanceIdentifier>();

		actionItems.push_back(std::make_pair(WidgetInstanceIdentifier(dmu_member), std::shared_ptr<WidgetActionItem>(static_cast<WidgetActionItem*>(new WidgetActionItem__WidgetInstanceIdentifier(dmu_member)))));

	}

	WidgetActionItemRequest_ACTION_DELETE_DMU_MEMBERS action_request(WIDGET_ACTION_ITEM_REQUEST_REASON__REMOVE_ITEMS, actionItems);
	emit DeleteDMUMembers(action_request);

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

void DisplayDMUsRegion::HandleChanges(DataChangeMessage const & change_message)
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

	std::for_each(change_message.changes.cbegin(), change_message.changes.cend(), [this, &itemModel](DataChange const & change)
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
									throw NewGeneException() << newgene_error_description(msg.str());
								}

								std::string dmu_code = *change.parent_identifier.code;
								std::string dmu_description = *change.parent_identifier.longhand;

								QStandardItem * item = new QStandardItem();
								QString text(dmu_code.c_str());
								if (!dmu_description.empty())
								{
									text += " (";
									text += dmu_description.c_str();
									text += ")";
								}
								item->setText(text);
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
									boost::format msg("Invalid new DMU name or description.");
									throw NewGeneException() << newgene_error_description(msg.str());
								}

								std::string dmu_code = *change.parent_identifier.code;
								std::string dmu_description = *change.parent_identifier.longhand;

								QString text(dmu_code.c_str());
								if (!dmu_description.empty())
								{
									text += " (";
									text += dmu_description.c_str();
									text += ")";
								}
								QList<QStandardItem *> items = itemModel->findItems(text);
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
											QStandardItemModel * dmuSetMembersModel = static_cast<QStandardItemModel*>(ui->listView_dmu_members->model());
											if (dmuSetMembersModel != nullptr)
											{
												delete dmuSetMembersModel;
												QStandardItemModel * model = new QStandardItemModel(ui->listView_dmu_members);
												ui->listView_dmu_members->setModel(model);
												if (oldDmuSetMembersSelectionModel)
												{
													delete oldDmuSetMembersSelectionModel;
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
			default:
				{
				}
				break;
		}
	});

	itemModel->sort(0);

}
