#include "newgenemanagevgs.h"
#include "ui_newgenemanagevgs.h"

#include <QStandardItemModel>
#include <QListView>
#include <QDialogButtonBox>
#include <QCheckBox>
#include <QGroupBox>
#include <QInputDialog>
#include <QPlainTextEdit>

#include "../Project/uiprojectmanager.h"
#include "../Project/uiinputproject.h"
#include "../../Utilities/dialoghelper.h"
#include "../../../../../NewGeneBackEnd/Utilities/Validation.h"
#include "../../../../NewGeneBackEnd/Utilities/TimeRangeHelper.h"
#include "../../../../NewGeneBackEnd/Model/InputModel.h"
#include "../../../../NewGeneBackEnd/Model/TimeGranularity.h"
#include "../../../../NewGeneBackEnd/Model/Tables/Import/Import.h"
#include "../../Utilities/htmldelegate.h"

#include <set>

NewGeneManageVGs::NewGeneManageVGs(QWidget * parent) :
	QWidget(parent),
	NewGeneWidget(WidgetCreationInfo(this, parent, WIDGET_NATURE_INPUT_WIDGET, MANAGE_VGS_WIDGET,
									 true)),   // 'this' pointer is cast by compiler to proper Widget instance, which is already created due to order in which base classes appear in class definition
	ui(new Ui::NewGeneManageVGs)
{
	ui->setupUi(this);
	ui->label_importProgress->hide();
	ui->progressBar_importVG->hide();
	ui->pushButton_cancel->hide();
	PrepareInputWidget(true);

	ui->pushButton_add_vg->setEnabled(false);
	ui->pushButton_remove_vg->setEnabled(false);
	ui->pushButton_refresh_vg->setEnabled(false);
	ui->pushButton_set_description_for_vg->setEnabled(false);
	ui->pushButton_set_warning_for_vg->setEnabled(false);

	refresh_vg_called_after_create = false;
}

NewGeneManageVGs::~NewGeneManageVGs()
{
	delete ui;
}

void NewGeneManageVGs::changeEvent(QEvent * e)
{
	QWidget::changeEvent(e);

	switch (e->type())
	{
		case QEvent::LanguageChange:
			ui->retranslateUi(this);
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
		connect(this, SIGNAL(CreateVG(WidgetActionItemRequest_ACTION_CREATE_VG)), inp->getConnector(), SLOT(CreateVG(WidgetActionItemRequest_ACTION_CREATE_VG)));
		connect(this, SIGNAL(DeleteVG(WidgetActionItemRequest_ACTION_DELETE_VG)), inp->getConnector(), SLOT(DeleteVG(WidgetActionItemRequest_ACTION_DELETE_VG)));
		connect(this, SIGNAL(SetVGDescriptions(WidgetActionItemRequest_ACTION_SET_VG_DESCRIPTIONS)), inp->getConnector(), SLOT(SetVGDescriptions(WidgetActionItemRequest_ACTION_SET_VG_DESCRIPTIONS)));
		connect(this, SIGNAL(RefreshVG(WidgetActionItemRequest_ACTION_REFRESH_VG)), inp->getConnector(), SLOT(RefreshVG(WidgetActionItemRequest_ACTION_REFRESH_VG)));
		connect(project->getConnector(), SIGNAL(SignalUpdateVGImportProgressBar(int, int, int, int)), this, SLOT(UpdateVGImportProgressBar(int, int, int, int)));

		if (project)
		{
			project->RegisterInterestInChange(this, DATA_CHANGE_TYPE__INPUT_MODEL__VG_CHANGE, false, "");
		}

		ui->pushButton_add_vg->setEnabled(true);
		ui->pushButton_remove_vg->setEnabled(false);
		ui->pushButton_refresh_vg->setEnabled(false);
		ui->pushButton_set_description_for_vg->setEnabled(false);
		ui->pushButton_set_warning_for_vg->setEnabled(false);
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
			connect(this, SIGNAL(DeleteVG(WidgetActionItemRequest_ACTION_DELETE_VG)), project->getConnector(), SLOT(DeleteVG(WidgetActionItemRequest_ACTION_DELETE_VG)));
			connect(this, SIGNAL(SetVGDescriptions(WidgetActionItemRequest_ACTION_SET_VG_DESCRIPTIONS)), project->getConnector(), SLOT(SetVGDescriptions(WidgetActionItemRequest_ACTION_SET_VG_DESCRIPTIONS)));
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

	if (!ui->listViewManageVGs)
	{
		boost::format msg("Invalid list view in NewGeneManageVGs widget.");
		QMessageBox msgBox;
		msgBox.setText(msg.str().c_str());
		msgBox.exec();
		return;
	}

	QStandardItemModel * oldModel = static_cast<QStandardItemModel *>(ui->listViewManageVGs->model());

	if (oldModel != nullptr)
	{
		delete oldModel;
	}

	QItemSelectionModel * oldSelectionModel = ui->listViewManageVGs->selectionModel();
	QStandardItemModel * model = new QStandardItemModel(ui->listViewManageVGs);

	int index = 0;
	std::for_each(widget_data.vgs_and_uoa.cbegin(), widget_data.vgs_and_uoa.cend(), [this, &index,
				  &model](std::pair<WidgetInstanceIdentifier, WidgetInstanceIdentifier> const & vg_and_uoa)
	{
		WidgetInstanceIdentifier const & vg = vg_and_uoa.first;
		WidgetInstanceIdentifier const & uoa = vg_and_uoa.second;

		if (vg.uuid && !vg.uuid->empty() && vg.code && !vg.code->empty())
		{

			QStandardItem * item = new QStandardItem();
			std::string text = Table_VG_CATEGORY::GetVgDisplayText(vg, true);
			item->setText(text.c_str());
			item->setEditable(false);
			item->setCheckable(false);
			QVariant v;
			v.setValue(vg_and_uoa);
			item->setData(v);
			model->setItem(index, item);

			++index;

		}
	});

	model->sort(0);

	ui->listViewManageVGs->setModel(model);
	ui->listViewManageVGs->setItemDelegate(new HtmlDelegate{});

	if (oldSelectionModel) { delete oldSelectionModel; }

	connect(ui->listViewManageVGs->selectionModel(), SIGNAL(selectionChanged(QItemSelection, QItemSelection)), this, SLOT(ReceiveVGSelectionChanged(const QItemSelection &,
			const QItemSelection &)));

	ui->pushButton_add_vg->setEnabled(true);
	ui->pushButton_remove_vg->setEnabled(false);
	ui->pushButton_refresh_vg->setEnabled(false);
	ui->pushButton_set_description_for_vg->setEnabled(false);
	ui->pushButton_set_warning_for_vg->setEnabled(false);

}

void NewGeneManageVGs::Empty()
{

	if (!ui->listViewManageVGs)
	{
		boost::format msg("Invalid list view in NewGeneManageVGs widget.");
		QMessageBox msgBox;
		msgBox.setText(msg.str().c_str());
		msgBox.exec();
		return;
	}

	QStandardItemModel * oldModel = nullptr;
	QItemSelectionModel * oldSelectionModel = nullptr;

	oldSelectionModel = ui->listViewManageVGs->selectionModel();

	if (oldSelectionModel != nullptr)
	{
		delete oldSelectionModel;
		oldSelectionModel = nullptr;
	}

	oldModel = static_cast<QStandardItemModel *>(ui->listViewManageVGs->model());

	if (oldModel != nullptr)
	{
		delete oldModel;
		oldModel = nullptr;
	}

	oldSelectionModel = ui->listViewManageVGs->selectionModel();

	if (oldSelectionModel != nullptr)
	{
		delete oldSelectionModel;
		oldSelectionModel = nullptr;
	}

	ui->pushButton_add_vg->setEnabled(false);
	ui->pushButton_remove_vg->setEnabled(false);
	ui->pushButton_refresh_vg->setEnabled(false);
	ui->pushButton_set_description_for_vg->setEnabled(false);
	ui->pushButton_set_warning_for_vg->setEnabled(false);

}

void NewGeneManageVGs::HandleChanges(DataChangeMessage const & change_message)
{

	UIInputProject * project = projectManagerUI().getActiveUIInputProject();

	if (project == nullptr)
	{
		return;
	}

	UIMessager messager(project);

	if (!ui->listViewManageVGs)
	{
		boost::format msg("Invalid list view in NewGeneManageVGs widget.");
		QMessageBox msgBox;
		msgBox.setText(msg.str().c_str());
		msgBox.exec();
		return;
	}

	QStandardItemModel * itemModel = static_cast<QStandardItemModel *>(ui->listViewManageVGs->model());

	if (itemModel == nullptr)
	{
		boost::format msg("Invalid list view items in NewGeneManageVGs widget.");
		QMessageBox msgBox;
		msgBox.setText(msg.str().c_str());
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

								if (!change.parent_identifier.uuid || (*change.parent_identifier.uuid).empty() || !change.parent_identifier.code || change.parent_identifier.code->empty()
									|| !change.parent_identifier.identifier_parent)
								{
									boost::format msg("Invalid new VG ID, code, or associated UOA.");
									QMessageBox msgBox;
									msgBox.setText(msg.str().c_str());
									msgBox.exec();
									return;
								}

								WidgetInstanceIdentifier const & vg = change.parent_identifier;
								WidgetInstanceIdentifier const & uoa = *vg.identifier_parent;

								std::string text = Table_VG_CATEGORY::GetVgDisplayText(vg, true);

								QStandardItem * item = new QStandardItem();
								item->setText(text.c_str());
								item->setEditable(false);
								item->setCheckable(false);

								std::pair<WidgetInstanceIdentifier, WidgetInstanceIdentifier> vg_and_uoa = std::make_pair(vg, uoa);
								QVariant v;
								v.setValue(vg_and_uoa);
								item->setData(v);
								itemModel->appendRow(item);

								QItemSelectionModel * selectionModel = ui->listViewManageVGs->selectionModel();

								if (selectionModel != nullptr)
								{
									QModelIndex itemIndex = itemModel->indexFromItem(item);
									ui->listViewManageVGs->setCurrentIndex(itemIndex);
								}

								QEvent * event = new QEvent(QEVENT_PROMPT_FOR_VG_REFRESH);
								QApplication::postEvent(this, event);

							}
							break;

						case DATA_CHANGE_INTENTION__REMOVE:
							{

								if (!change.parent_identifier.uuid || (*change.parent_identifier.uuid).empty() || !change.parent_identifier.code || change.parent_identifier.code->empty()
									|| !change.parent_identifier.identifier_parent)
								{
									boost::format msg("Invalid VG to remove.");
									QMessageBox msgBox;
									msgBox.setText(msg.str().c_str());
									msgBox.exec();
									return;
								}

								WidgetInstanceIdentifier const & vg = change.parent_identifier;
								WidgetInstanceIdentifier const & uoa = *vg.identifier_parent;

								int numberItems = itemModel->rowCount();

								for (int currentItem = 0; currentItem < numberItems; ++currentItem)
								{
									QStandardItem * vg_to_remove_item = itemModel->item(currentItem);

									if (vg_to_remove_item != nullptr)
									{

										QVariant vg_and_uoa_variant = vg_to_remove_item->data();
										std::pair<WidgetInstanceIdentifier, WidgetInstanceIdentifier> const vg_and_uoa = vg_and_uoa_variant.value<std::pair<WidgetInstanceIdentifier, WidgetInstanceIdentifier>>();
										WidgetInstanceIdentifier const & vg_test = vg_and_uoa.first;

										if (!vg_test.identifier_parent)
										{
											continue;
										}

										WidgetInstanceIdentifier const & uoa_test = *vg_test.identifier_parent;

										if (vg_test.IsEqual(WidgetInstanceIdentifier::EQUALITY_CHECK_TYPE__UUID_PLUS_STRING_CODE, vg)
											&&
											uoa_test.IsEqual(WidgetInstanceIdentifier::EQUALITY_CHECK_TYPE__UUID_PLUS_STRING_CODE, uoa))
										{

											QModelIndex index_to_remove = itemModel->indexFromItem(vg_to_remove_item);
											itemModel->takeRow(index_to_remove.row());

											delete vg_to_remove_item;
											vg_to_remove_item = nullptr;

											QItemSelectionModel * selectionModel = ui->listViewManageVGs->selectionModel();

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
								// No-op
							}
							break;

						case DATA_CHANGE_INTENTION__RESET_ALL:
							{
								RefreshAllWidgets();
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

bool NewGeneManageVGs::GetSelectedVG(WidgetInstanceIdentifier & vg, WidgetInstanceIdentifier & uoa)
{

	QItemSelectionModel * vg_selectionModel = ui->listViewManageVGs->selectionModel();

	if (vg_selectionModel == nullptr)
	{
		boost::format msg("Invalid selection in NewGeneManageVGs widget.");
		QMessageBox msgBox;
		msgBox.setText(msg.str().c_str());
		msgBox.exec();
		return false;
	}

	QModelIndexList selectedIndexes = vg_selectionModel->selectedIndexes();

	if (selectedIndexes.empty())
	{
		// No selection
		return false;
	}

	if (selectedIndexes.size() > 1)
	{
		boost::format msg("Simultaneous selections not allowed in NewGeneManageVGs DMU category widget.");
		QMessageBox msgBox;
		msgBox.setText(msg.str().c_str());
		msgBox.exec();
		return false;
	}

	QModelIndex selectedIndex = selectedIndexes[0];

	if (!selectedIndex.isValid())
	{
		// No selection
		return false;
	}

	QStandardItemModel * vgModel = static_cast<QStandardItemModel *>(ui->listViewManageVGs->model());

	if (vgModel == nullptr)
	{
		boost::format msg("Invalid model in NewGeneManageVGs DMU category widget.");
		QMessageBox msgBox;
		msgBox.setText(msg.str().c_str());
		msgBox.exec();
		return false;
	}

	QVariant vg_and_uoa_variant = vgModel->item(selectedIndex.row())->data();
	std::pair<WidgetInstanceIdentifier, WidgetInstanceIdentifier> vg_and_uoa = vg_and_uoa_variant.value<std::pair<WidgetInstanceIdentifier, WidgetInstanceIdentifier>>();
	vg = vg_and_uoa.first;

	if (!vg.identifier_parent)
	{
		return false;
	}

	uoa = *vg.identifier_parent;

	return true;

}

void NewGeneManageVGs::on_pushButton_add_vg_clicked()
{

	UIInputProject * project = projectManagerUI().getActiveUIInputProject();

	if (project == nullptr)
	{
		boost::format msg("Bad input project.  Unable to create \"New VG\" dialog.");
		QMessageBox msgBox;
		msgBox.setText(msg.str().c_str());
		msgBox.exec();
		return;
	}

	UIInputModel & ui_input_model = project->model();
	InputModel & backend_input_model = ui_input_model.getBackendModel();
	WidgetInstanceIdentifiers dmu_categories = backend_input_model.t_dmu_category.getIdentifiers();

	// From http://stackoverflow.com/a/17512615/368896
	QDialog dialog(this);
	dialog.setWindowTitle("Create variable group");
	dialog.setWindowFlags(dialog.windowFlags() & ~(Qt::WindowContextHelpButtonHint | Qt::WindowMinimizeButtonHint | Qt::WindowMaximizeButtonHint));
	QFormLayout form(&dialog);
	QList<QLineEdit *> fields;
	QLineEdit * warningEdit = nullptr;

	QLineEdit * lineEditCode = new QLineEdit(&dialog);
	setLineEditWidth(lineEditCode, 12);
	QString labelCode = QString("Enter a brief identifying code for the new variable group:");
	form.addRow(labelCode, lineEditCode);
	fields << lineEditCode;

	QLineEdit * lineEditDescription = new QLineEdit(&dialog);
	setLineEditWidth(lineEditDescription, 20);
	QString labelDescription = QString("Enter a short description for the new variable group:");
	form.addRow(labelDescription, lineEditDescription);
	fields << lineEditDescription;

	QLineEdit * lineEditWarning = new QLineEdit(&dialog);
	setLineEditWidth(lineEditWarning);
	QString labelWarning = QString("Enter an optional warning for the new variable group:");
	form.addRow(labelWarning, lineEditWarning);
	warningEdit = lineEditWarning;

	QWidget VgConstructionWidget;
	QVBoxLayout formOverall;
	QWidget VgConstructionPanes;
	QHBoxLayout formConstructionPane;
	QListView * listpane = nullptr;
	WidgetInstanceIdentifiers uoas = backend_input_model.t_uoa_category.getIdentifiers();
	DialogHelper::AddVgCreationBlock(dialog, form, VgConstructionWidget, formOverall, VgConstructionPanes, formConstructionPane, listpane, uoas);

	if (!listpane)
	{
		boost::format msg("Unable to create \"New VG\" dialog.");
		QMessageBox msgBox;
		msgBox.setText(msg.str().c_str());
		msgBox.exec();
		return;
	}

	// Add some standard buttons (Cancel/Ok) at the bottom of the dialog
	QDialogButtonBox buttonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal, &dialog);
	form.addRow(&buttonBox);

	std::string proposed_vg_code;
	std::string vg_description;
	std::string vg_warning;

	WidgetInstanceIdentifier uoa_to_use;

	QObject::connect(&buttonBox, SIGNAL(rejected()), &dialog, SLOT(reject()));
	QObject::connect(&buttonBox, &QDialogButtonBox::accepted, [&]()
	{

		proposed_vg_code.clear();
		vg_description.clear();
		vg_warning.clear();

		std::string errorMsg;

		QLineEdit * proposed_vg_code_field = fields[0];
		QLineEdit * vg_description_field = fields[1];
		QLineEdit * vg_warning_field = warningEdit;

		if (proposed_vg_code_field && vg_description_field)
		{
			proposed_vg_code = proposed_vg_code_field->text().toStdString();
			vg_description = vg_description_field->text().toStdString();

			if (proposed_vg_code.empty())
			{
				boost::format msg("The variable group must have an identifying code (an all-caps, typically short, string).");
				QMessageBox msgBox;
				msgBox.setText(msg.str().c_str());
				msgBox.exec();
				return false;
			}

			if (vg_description.empty())
			{
				boost::format msg("You must provide a description for the variable group.");
				QMessageBox msgBox;
				msgBox.setText(msg.str().c_str());
				msgBox.exec();
				return false;
			}
		}
		else
		{
			boost::format msg("Unable to determine new VG code or description.");
			QMessageBox msgBox;
			msgBox.setText(msg.str().c_str());
			msgBox.exec();
			return false;
		}

		vg_warning = vg_warning_field->text().toStdString();

		boost::trim(proposed_vg_code);
		boost::trim(vg_description);
		boost::trim(vg_warning);

		bool valid = true;

		bool hasErrors (false);
		if (valid)
		{
			std::string newErrMsg;
			valid = Validation::ValidateVgCode(proposed_vg_code, newErrMsg);
			if (hasErrors)
			{
				errorMsg += "\n";
			}
			hasErrors = true;
			errorMsg += newErrMsg;
		}

		if (valid)
		{
			std::string newErrMsg;
			valid = Validation::ValidateVgDescription(vg_description, newErrMsg);
			if (hasErrors)
			{
				errorMsg += "\n";
			}
			hasErrors = true;
			errorMsg += newErrMsg;
		}

		if (valid)
		{
			std::string newErrMsg;
			valid = Validation::ValidateVgNotes(vg_warning, newErrMsg);
			if (hasErrors)
			{
				errorMsg += "\n";
			}
			hasErrors = true;
			errorMsg += newErrMsg;
		}

		if (!valid)
		{
			std::string theErr = "There are errors with the new variable group:\n";
			theErr += errorMsg;
			boost::format msg("%1%");
			msg % theErr;
			QMessageBox msgBox;
			msgBox.setText(msg.str().c_str());
			msgBox.exec();
			return false;
		}

		if (valid)
		{

			// retrieve the UOA to associate with the new variable group
			QStandardItemModel * listpaneModel = static_cast<QStandardItemModel *>(listpane->model());

			if (listpaneModel == nullptr)
			{
				boost::format msg("Invalid list view items in Construct VG popup.");
				QMessageBox msgBox;
				msgBox.setText(msg.str().c_str());
				msgBox.exec();
				return false;
			}

			QItemSelectionModel * listpane_selectionModel = listpane->selectionModel();

			if (listpane_selectionModel == nullptr)
			{
				boost::format msg("Invalid selection in New VG popup.");
				QMessageBox msgBox;
				msgBox.setText(msg.str().c_str());
				msgBox.exec();
				return false;
			}

			QModelIndexList selectedIndexes = listpane_selectionModel->selectedIndexes();

			if (selectedIndexes.empty())
			{
				// No selection
				boost::format msg("You must select a unit of analysis (UOA) for the variable group.");
				QMessageBox msgBox;
				msgBox.setText(msg.str().c_str());
				msgBox.exec();
				return false;
			}

			if (selectedIndexes.size() > 1)
			{
				boost::format msg("Simultaneous selections not allowed.");
				QMessageBox msgBox;
				msgBox.setText(msg.str().c_str());
				msgBox.exec();
				return false;
			}

			QModelIndex selectedIndex = selectedIndexes[0];

			if (!selectedIndex.isValid())
			{
				boost::format msg("A unit of analysis must be associated with the new variable group.");
				QMessageBox msgBox;
				msgBox.setText(msg.str().c_str());
				msgBox.exec();
				return false;
			}

			QVariant uoa_variant = listpaneModel->item(selectedIndex.row())->data();
			uoa_to_use = uoa_variant.value<WidgetInstanceIdentifier>();

			dialog.accept();
		}

	});

	dialog.setMinimumWidth(640);

	if (dialog.exec() != QDialog::Accepted)
	{
		return;
	}

	std::string new_vg_code(proposed_vg_code);

	InstanceActionItems actionItems;
	actionItems.push_back(std::make_pair(uoa_to_use, std::shared_ptr<WidgetActionItem>(static_cast<WidgetActionItem *>(new WidgetActionItem__StringVector(std::vector<std::string> {new_vg_code, vg_description, vg_warning})))));
	WidgetActionItemRequest_ACTION_CREATE_VG action_request(WIDGET_ACTION_ITEM_REQUEST_REASON__ADD_ITEMS, actionItems);

	emit CreateVG(action_request);

}

void NewGeneManageVGs::on_pushButton_remove_vg_clicked()
{

	UIInputProject * project = projectManagerUI().getActiveUIInputProject();

	if (project == nullptr)
	{
		return;
	}

	UIMessager messager(project);

	if (!ui->listViewManageVGs)
	{
		boost::format msg("Invalid list view in NewGeneManageVGs widget.");
		QMessageBox msgBox;
		msgBox.setText(msg.str().c_str());
		msgBox.exec();
		return;
	}

	WidgetInstanceIdentifier vg;
	WidgetInstanceIdentifier uoa;
	bool is_selected = GetSelectedVG(vg, uoa);

	if (!is_selected)
	{
		return;
	}

	if (!vg.code || vg.code->empty())
	{
		boost::format msg("Invalid variable group being deleted.");
		QMessageBox msgBox;
		msgBox.setText(msg.str().c_str());
		msgBox.exec();
		return;
	}

	QMessageBox::StandardButton reply;
	boost::format msg("Are you certain you wish to delete the variable group \"%1%\"?");
	msg % *vg.code;
	boost::format msgTitle("Delete variable group \"%1%\"?");
	msgTitle % *vg.code;
	reply = QMessageBox::question(nullptr, QString(msgTitle.str().c_str()), QString(msg.str().c_str()), QMessageBox::StandardButtons(QMessageBox::Yes | QMessageBox::No));

	if (reply == QMessageBox::No)
	{
		return;
	}

	InstanceActionItems actionItems;
	actionItems.push_back(std::make_pair(vg, std::shared_ptr<WidgetActionItem>(static_cast<WidgetActionItem *>(new WidgetActionItem()))));
	WidgetActionItemRequest_ACTION_DELETE_VG action_request(WIDGET_ACTION_ITEM_REQUEST_REASON__REMOVE_ITEMS, actionItems);
	emit DeleteVG(action_request);

}

void NewGeneManageVGs::on_pushButton_refresh_vg_clicked()
{

	if (!refresh_vg_called_after_create)
	{
		// Refreshing data is VASTLY slower than simply inserting new data;
		// give warning
		refresh_vg_called_after_create = false;
		boost::format msg_title("Refreshing data can be slow");
		boost::format
		msg_text("Warning: If there is a large amount of existing data AND a large amount of data being refreshed, it will be MUCH, MUCH faster to delete the existing variable group, and re-import from scratch.  Continue?");
		QMessageBox::StandardButton reply;
		reply = QMessageBox::question(nullptr, QString(msg_title.str().c_str()), QString(msg_text.str().c_str()), QMessageBox::StandardButtons(QMessageBox::Yes | QMessageBox::No));

		if (reply == QMessageBox::No)
		{
			return;
		}
	}

	bool do_refresh_not_plain_insert = !refresh_vg_called_after_create;
	refresh_vg_called_after_create = false;

	WidgetInstanceIdentifier vg;
	WidgetInstanceIdentifier uoa;
	bool found = GetSelectedVG(vg, uoa);

	if (!found)
	{
		boost::format msg("A variable group must be selected.");
		QMessageBox msgBox;
		msgBox.setText(msg.str().c_str());
		msgBox.exec();
		return;
	}

	if (!uoa.foreign_key_identifiers || uoa.foreign_key_identifiers->size() == 0)
	{
		boost::format msg("DMUs cannot be found for the UOA associated with the selected variable group.");
		QMessageBox msgBox;
		msgBox.setText(msg.str().c_str());
		msgBox.exec();
		return;
	}

	WidgetInstanceIdentifiers dmu_categories = *(uoa.foreign_key_identifiers);



	// ********************************************************************************************************************** //
	// Variable Group import dialog
	// ********************************************************************************************************************** //

	// Widget and layout form a pair.
	//
	// Or, you can just have a layout by itself
	//
	// Widget needs no parent
	// Layout constructs with widget as parent, or no parent
	// OR Widget sets layout
	// ... You can even do both (redundantly) - have the form set the widget as its parent and have the parent add the layout as its layout
	//
	// Layout "adds row" or "adds widget", adding EITHER widget OR layout
	// ... if a layout has no widget, it can still add widgets or other layouts as rows
	//
	// If a layout has no widget, it will be added as a "row" to another layout
	//
	// You would use a WIDGET, with its layout, rather than just the layout, in order to HIDE the widget, use the widget on other forms, etc.

	// Pair: dialog (a widget) and its layout
	QDialog dialog(this);
	dialog.setWindowTitle("Variable group refresh");
	dialog.setWindowFlags(dialog.windowFlags() & ~(Qt::WindowContextHelpButtonHint | Qt::WindowMinimizeButtonHint | Qt::WindowMaximizeButtonHint));
	QFormLayout form(&dialog);


	// ********************************************************************************************************************** //
	// File chooser block
	// ********************************************************************************************************************** //

	// Pair: file chooser widget and its layout (added as layout in DialogHelper::AddFileChooserBlock(), below)
	QWidget FileChooserWidget;
	QBoxLayout formFileSelection(QBoxLayout::LeftToRight);

	boost::format msg_import_header("Variable group: %1%");
	msg_import_header % Table_VG_CATEGORY::GetVgDisplayText(vg, true);
	QLabel * rowLabel { new QLabel(msg_import_header.str().c_str()) };
	rowLabel->setTextFormat(Qt::RichText);
	form.addRow(rowLabel);

	QList<QLineEdit *> fieldsFileChooser;
	std::vector<std::string> const & fileChooserStrings { "Choose comma-delimited data file", "Choose comma-delimited data file location", "", "" };
	DialogHelper::AddFileChooserBlock(dialog, form, formFileSelection, FileChooserWidget, fieldsFileChooser, fileChooserStrings);




	// ********************************************************************************************************************** //
	// DMU column choosers
	// ********************************************************************************************************************** //

	QList<QLineEdit *> fieldsDMU;
	std::for_each(dmu_categories.cbegin(), dmu_categories.cend(), [&](WidgetInstanceIdentifier const & dmu)
	{

		std::string dmu_description = Table_DMU_Identifier::GetDmuCategoryDisplayText(dmu);
		boost::format msg("Enter the column name for \"%1%\":");
		msg % dmu_description;
		QString labelDmu = QString(msg.str().c_str());
		QLineEdit * lineEditDMU = new QLineEdit(&dialog);
		setLineEditWidth(lineEditDMU, 20);
		form.addRow(labelDmu, lineEditDMU);
		fieldsDMU << lineEditDMU;

	});

	{
		QWidget * spacer { new QWidget{} };
		spacer->setMinimumHeight(20);
		form.addRow(spacer);
	}

	// ********************************************************************************************************************** //
	// Time range block
	// ********************************************************************************************************************** //

	QList<QLineEdit *> fieldsTimeRange;
	QList<QRadioButton *> radioButtonsTimeRange;

	QBoxLayout formTimeRangeSelection(QBoxLayout::LeftToRight);

	// Pair: "year" widget and its layout
	QWidget YearWidget;
	QFormLayout formYearOptions(&YearWidget);

	// Pair: "year-month-day" widget and its layout
	QWidget YearMonthDayWidget;
	QFormLayout formYearMonthDayOptions(&YearMonthDayWidget);
	QGroupBox YearMonthDayWidget_ints("Time range columns (as year, month, and day)");
	QFormLayout formYearMonthDayOptions_ints(&YearMonthDayWidget_ints);
	QGroupBox YearMonthDayWidget_strings("Time range columns (as text)");
	QFormLayout formYearMonthDayOptions_strings(&YearMonthDayWidget_strings);
	QVBoxLayout formYMDTimeRange_StringVsInt;
	QList<QRadioButton *> radioButtonsYMD_StringVsInt_TimeRange;

	// Pair: "year-month" widget and its layout
	QWidget YearMonthWidget;
	QFormLayout formYearMonthOptions(&YearMonthWidget);
	QGroupBox YearMonthWidget_ints("Time range columns (as year, month, and day)");
	QFormLayout formYearMonthOptions_ints(&YearMonthWidget_ints);
	QGroupBox YearMonthWidget_strings("Time range columns (as text)");
	QFormLayout formYearMonthOptions_strings(&YearMonthWidget_strings);
	QVBoxLayout formYMTimeRange_StringVsInt;
	QList<QRadioButton *> radioButtonsYM_StringVsInt_TimeRange;

	if (uoa.time_granularity != TIME_GRANULARITY__NONE)
	{
		DialogHelper::AddTimeRangeSelectorBlock(

			dialog,
			form,
			fieldsTimeRange,
			radioButtonsTimeRange,
			formTimeRangeSelection,

			YearWidget,
			formYearOptions,

			YearMonthDayWidget,
			formYearMonthDayOptions,
			YearMonthDayWidget_ints,
			formYearMonthDayOptions_ints,
			YearMonthDayWidget_strings,
			formYearMonthDayOptions_strings,
			formYMDTimeRange_StringVsInt,
			radioButtonsYMD_StringVsInt_TimeRange,

			YearMonthWidget,
			formYearMonthOptions,
			YearMonthWidget_ints,
			formYearMonthOptions_ints,
			YearMonthWidget_strings,
			formYearMonthOptions_strings,
			formYMTimeRange_StringVsInt,
			radioButtonsYM_StringVsInt_TimeRange,

			uoa.time_granularity

		);
	}

	{
		QWidget * spacer { new QWidget{} };
		spacer->setMinimumHeight(20);
		form.addRow(spacer);
	}

	// Add rows that allow the user to state whether the input file has rows containing the column descriptions, or describing the column data types
	QList<QCheckBox *> fieldsCheckboxes;

	form.addRow(new QLabel("      (Note: A row with the names of the given columns is required.)"));

	QString labelIncludeColumnDescriptions = QString("Data file includes an additional row with column descriptions");
	QCheckBox * checkboxIncludeColumnDescriptions = new QCheckBox(labelIncludeColumnDescriptions, &dialog);
	form.addRow(checkboxIncludeColumnDescriptions);
	fieldsCheckboxes << checkboxIncludeColumnDescriptions;

	QString labelIncludeDataTypes = QString("Data file includes an additional row with column data types (one of 'int', 'int64', 'float', or 'string'; default is 'int')");
	QCheckBox * checkboxIncludeDataTypes = new QCheckBox(labelIncludeDataTypes, &dialog);
	form.addRow(checkboxIncludeDataTypes);
	fieldsCheckboxes << checkboxIncludeDataTypes;

	{
		QWidget * spacer { new QWidget{} };
		spacer->setMinimumHeight(20);
		form.addRow(spacer);
	}

	// Add some standard buttons (Cancel/Ok) at the bottom of the dialog
	QDialogButtonBox buttonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal, &dialog);
	form.addRow(&buttonBox);

	std::string data_column_name_uuid;
	std::string data_column_name_code;
	std::string data_column_name_description;

	std::vector<std::string> dataFileChooser;
	std::vector<std::string> dataTimeRange;
	std::vector<std::string> dataDmuColNames;

	bool inputFileContainsColumnDescriptions = false;
	bool inputFileContainsColumnDataTypes = false;

	QObject::connect(&buttonBox, SIGNAL(rejected()), &dialog, SLOT(reject()));
	QObject::connect(&buttonBox, &QDialogButtonBox::accepted, [&]()
	{

		data_column_name_uuid.clear();
		data_column_name_code.clear();
		data_column_name_description.clear();

		dataFileChooser.clear();
		dataTimeRange.clear();
		dataDmuColNames.clear();

		inputFileContainsColumnDescriptions = false;
		inputFileContainsColumnDataTypes = false;

		// data validation here
		bool valid = true;

		std::string errorMsg;

		if (valid)
		{
			int index = 0;
			std::for_each(dmu_categories.cbegin(), dmu_categories.cend(), [&](WidgetInstanceIdentifier const & dmu)
			{

				if (!valid)
				{
					++index;
					return;
				}

				QLineEdit * data_column_dmu = nullptr;

				if (valid)
				{
					data_column_dmu = fieldsDMU[index];

					if (!data_column_dmu)
					{
						valid = false;
						errorMsg = "Invalid DMU.";
						++index;
						return;
					}
				}

				std::string data_column_name;

				if (valid)
				{
					data_column_name = data_column_dmu->text().toStdString();
				}

				if (valid)
				{

					valid = Validation::ValidateColumnName(data_column_name, "DMU", true, errorMsg);

					if (!valid)
					{
						valid = false;
						errorMsg = "Invalid DMU column name.";
						++index;
						return;
					}

				}

				if (valid)
				{
					dataDmuColNames.push_back(data_column_name);
				}

				++index;

			});
		}

		if (valid)
		{
			// Test that primary key column names are unique
			std::set<std::string> testkeys(dataDmuColNames.cbegin(), dataDmuColNames.cend());

			if (testkeys.size() != dataDmuColNames.size())
			{
				valid = false;
				errorMsg = "Duplicate column names detected for DMU primary key columns.";
			}
		}

		if (valid)
		{
			valid = DialogHelper::ValidateFileChooserBlock(fieldsFileChooser, dataFileChooser, errorMsg);
		}

		bool warnEmptyEndingTimeCols = false;

		if (valid)
		{
			if (uoa.time_granularity != TIME_GRANULARITY__NONE)
			{
				valid = DialogHelper::ValidateTimeRangeBlock(

							dialog,
							form,
							fieldsTimeRange,
							radioButtonsTimeRange,

							YearWidget,
							formYearOptions,

							YearMonthDayWidget,
							formYearMonthDayOptions,
							YearMonthDayWidget_ints,
							formYearMonthDayOptions_ints,
							YearMonthDayWidget_strings,
							formYearMonthDayOptions_strings,
							radioButtonsYMD_StringVsInt_TimeRange,

							YearMonthWidget,
							formYearMonthOptions,
							YearMonthWidget_ints,
							formYearMonthOptions_ints,
							YearMonthWidget_strings,
							formYearMonthOptions_strings,
							radioButtonsYM_StringVsInt_TimeRange,

							uoa.time_granularity,
							dataTimeRange,
							warnEmptyEndingTimeCols,
							errorMsg

						);
			}
		}

		if (valid)
		{
			if (warnEmptyEndingTimeCols)
			{
				QRadioButton * YButton = radioButtonsTimeRange[0];
				QRadioButton * YMDButton = radioButtonsTimeRange[1];
				QRadioButton * YMButton = radioButtonsTimeRange[2];

				QString ymd;
				QString fin;
				QString colOrCols;
				QString isOrAre;

				if (YButton->isChecked())
				{
					ymd = "year ";
					fin = "year";
					colOrCols = "column";
					isOrAre = "is ";
				}
				else if (YMButton->isChecked())
				{
					ymd = "year and month ";
					fin = "month";
					colOrCols = "columns";
					isOrAre = "are ";
				}
				else
				{
					ymd = "year, month, and day ";
					fin = "day";
					colOrCols = "columns";
					isOrAre = "are ";
				}

				QMessageBox::StandardButton reply;
				QString msg;
				msg += "You have opted to leave the ending ";
				msg += ymd;
				msg += colOrCols;
				msg += " blank.";
				msg += "\n";
				msg += "NewGene will automatically set the ending ";
				msg += fin;
				msg += " of each row to precisely the end of the ";
				msg += fin;
				msg += " specified by the starting ";
				msg += ymd;
				msg += colOrCols;
				msg += ".\n";
				msg += "Please confirm this is what you would like by clicking Yes.  Otherwise, to cancel, click No.";

				QString title;
				title += "Ending ";
				title += ymd;
				title += colOrCols;
				title += " ";
				title += isOrAre;
				title += "blank.";

				reply = QMessageBox::question(nullptr, title, msg, QMessageBox::StandardButtons(QMessageBox::Yes | QMessageBox::No));

				if (reply != QMessageBox::Yes)
				{
					valid = false;
					errorMsg = "Canceled import because ending time ";
					errorMsg += colOrCols.toStdString();
					errorMsg += " ";
					errorMsg += isOrAre.toStdString();
					errorMsg += "blank.";
				}
			}
		}

		if (valid)
		{
			inputFileContainsColumnDescriptions = checkboxIncludeColumnDescriptions->isChecked();
			inputFileContainsColumnDataTypes = checkboxIncludeDataTypes->isChecked();
		}

		if (!valid)
		{
			boost::format msg(errorMsg);
			QMessageBox msgBox;
			msgBox.setText(msg.str().c_str());
			msgBox.exec();
		}

		if (valid)
		{
			dialog.accept();
		}

	});

	if (dialog.exec() != QDialog::Accepted)
	{
		return;
	}

	// validation has already taken place
	boost::filesystem::path data_column_file_pathname(dataFileChooser[0]);

	std::vector<std::pair<WidgetInstanceIdentifier, std::string>> dmusAndColumnNames;
	int index = 0;
	std::for_each(dmu_categories.cbegin(), dmu_categories.cend(), [&](WidgetInstanceIdentifier const & dmu)
	{
		dmusAndColumnNames.push_back(std::make_pair(dmu, dataDmuColNames[index]));
		++index;
	});

	InstanceActionItems actionItems;
	actionItems.push_back(std::make_pair(WidgetInstanceIdentifier(), std::shared_ptr<WidgetActionItem>(static_cast<WidgetActionItem *>(new WidgetActionItem__ImportVariableGroup(vg,
										 dataTimeRange, dmusAndColumnNames, data_column_file_pathname, uoa.time_granularity, inputFileContainsColumnDescriptions, inputFileContainsColumnDataTypes,
										 do_refresh_not_plain_insert)))));
	WidgetActionItemRequest_ACTION_REFRESH_VG action_request(WIDGET_ACTION_ITEM_REQUEST_REASON__DO_ACTION, actionItems);
	emit RefreshVG(action_request);

}

bool NewGeneManageVGs::event(QEvent * e)
{

	bool returnVal = false;

	if (e->type() == QEVENT_PROMPT_FOR_VG_REFRESH)
	{
		WidgetInstanceIdentifier vg;
		WidgetInstanceIdentifier uoa;
		bool is_selected = GetSelectedVG(vg, uoa);

		if (!is_selected)
		{
			return true; // Even though no VG is selected, we have recognized and processed our own custom event
		}

		QMessageBox::StandardButton reply;
		boost::format msg("Variable group '%1%' successfully created.\n\nWould you like to import data for the new variable group \"%1%\" now?");
		msg % *vg.code;
		boost::format msgTitle("Import data?");
		reply = QMessageBox::question(nullptr, QString(msgTitle.str().c_str()), QString(msg.str().c_str()), QMessageBox::StandardButtons(QMessageBox::Yes | QMessageBox::No));

		if (reply == QMessageBox::Yes)
		{
			QEvent * event = new QEvent(QEVENT_CLICK_VG_REFRESH);
			QApplication::postEvent(this, event);
		}

		returnVal = true;
	}
	else if (e->type() == QEVENT_CLICK_VG_REFRESH)
	{
		refresh_vg_called_after_create = true;
		ui->pushButton_refresh_vg->click();
		returnVal = true;
	}
	else
	{
		returnVal = QWidget::event(e);
	}

	return returnVal;

}

void NewGeneManageVGs::UpdateVGImportProgressBar(int mode_, int min_, int max_, int val_)
{

	PROGRESS_UPDATE_MODE mode = (PROGRESS_UPDATE_MODE)(mode_);

	switch (mode)
	{

		case PROGRESS_UPDATE_MODE__SHOW:
			{
				ui->label_importProgress->show();
				ui->progressBar_importVG->setTextVisible(true);
				ui->progressBar_importVG->show();
				ui->pushButton_cancel->show();
			}
			break;

		case PROGRESS_UPDATE_MODE__SET_LIMITS:
			{
				ui->progressBar_importVG->setRange(min_, max_);
				ui->progressBar_importVG->setValue(min_);
			}
			break;

		case PROGRESS_UPDATE_MODE__SET_VALUE:
			{
				ui->progressBar_importVG->setValue(val_);
			}
			break;

		case PROGRESS_UPDATE_MODE__HIDE:
			{
				ui->label_importProgress->hide();
				ui->progressBar_importVG->hide();
				ui->pushButton_cancel->hide();
			}
			break;

		default:
			{
				// no-op
			}
			break;

	}
}

void NewGeneManageVGs::on_pushButton_cancel_clicked()
{
	{
		std::lock_guard<std::recursive_mutex> guard(Importer::is_performing_import_mutex);

		if (Importer::is_performing_import)
		{
			QMessageBox::StandardButton reply;
			reply = QMessageBox::question(nullptr, QString("Cancel?"), QString("Are you sure you wish to cancel?"), QMessageBox::StandardButtons(QMessageBox::Yes | QMessageBox::No));

			if (reply == QMessageBox::Yes)
			{
				// No lock - not necessary for a boolean whose read/write is guaranteed to be in proper sequence
				Importer::cancelled = true;
			}
		}
	}
}

void NewGeneManageVGs::ReceiveVGSelectionChanged(const QItemSelection & selected, const QItemSelection & deselected)
{

	UIInputProject * project = projectManagerUI().getActiveUIInputProject();

	if (project == nullptr)
	{
		return;
	}

	UIMessager messager(project);

	if (!ui->listViewManageVGs)
	{
		boost::format msg("Invalid list view in Manage VGs tab.");
		QMessageBox msgBox;
		msgBox.setText(msg.str().c_str());
		msgBox.exec();
		return;
	}

	if (!selected.indexes().isEmpty())
	{
		ui->pushButton_add_vg->setEnabled(true);
		ui->pushButton_remove_vg->setEnabled(true);
		ui->pushButton_refresh_vg->setEnabled(true);
		ui->pushButton_set_description_for_vg->setEnabled(true);
		ui->pushButton_set_warning_for_vg->setEnabled(true);
	}
	else
	{
		ui->pushButton_add_vg->setEnabled(true);
		ui->pushButton_remove_vg->setEnabled(false);
		ui->pushButton_refresh_vg->setEnabled(false);
		ui->pushButton_set_description_for_vg->setEnabled(false);
		ui->pushButton_set_warning_for_vg->setEnabled(false);
	}

}

void NewGeneManageVGs::on_pushButton_set_description_for_vg_clicked()
{

	UIInputProject * project = projectManagerUI().getActiveUIInputProject();

	if (project == nullptr)
	{
		return;
	}

	UIMessager messager(project);

	if (!ui->listViewManageVGs)
	{
		boost::format msg("Invalid list view in NewGeneManageVGs widget.");
		QMessageBox msgBox;
		msgBox.setText(msg.str().c_str());
		msgBox.exec();
		return;
	}

	WidgetInstanceIdentifier vg;
	WidgetInstanceIdentifier uoa;
	bool is_selected = GetSelectedVG(vg, uoa);

	if (!is_selected)
	{
		return;
	}

	if (!vg.code || vg.code->empty())
	{
		boost::format msg("Invalid variable group.");
		QMessageBox msgBox;
		msgBox.setText(msg.str().c_str());
		msgBox.exec();
		return;
	}

	std::string vg_description;
	std::string vg_warning;
	if (vg.notes.notes1)
	{
		vg_warning = *vg.notes.notes1;
	}

	bool ok {false};

	while (!ok)
	{
		boost::format msg("Set a short description for variable group \"%1%\"");
		msg % *vg.code;
		std::string longhand;

		if (vg.longhand)
		{
			longhand = *vg.longhand;
		}

		QDialog dialog(this);
		QFormLayout form(&dialog);
		form.addRow(new QLabel(QString(msg.str().c_str())));
		QLineEdit * longhandEdit = nullptr;
		QLineEdit * warningEdit = nullptr;

		int i = 0;
		{
			QLineEdit *lineEdit = new QLineEdit(&dialog);
			setLineEditWidth(lineEdit, 20);
			QString label = QString("Short description:").arg(++i);
			form.addRow(label, lineEdit);
			lineEdit->setText(longhand.c_str());
			longhandEdit = lineEdit;
		}

		// Add some standard buttons (Cancel/Ok) at the bottom of the dialog
		QDialogButtonBox buttonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal, &dialog);
		form.addRow(&buttonBox);

		QObject::connect(&buttonBox, SIGNAL(accepted()), &dialog, SLOT(accept()));
		QObject::connect(&buttonBox, SIGNAL(rejected()), &dialog, SLOT(reject()));

		dialog.setMinimumWidth(640);

		if (dialog.exec() != QDialog::Accepted)
		{
			return;
		}

		longhand = longhandEdit->text().toStdString();
		boost::trim(longhand);
		bool validDescription = true;
		std::string errorMsgDescription;

		if (validDescription)
		{
			validDescription = Validation::ValidateVgDescription(longhand, errorMsgDescription);
		}

		if (!validDescription)
		{
			std::string errors;
			if (!validDescription)
			{
				errors += errorMsgDescription;
			}
			boost::format msg("%1%");
			msg % errors;
			QMessageBox msgBox;
			msgBox.setText(msg.str().c_str());
			msgBox.exec();
			ok = false;
		}
		else
		{
			if (vg.longhand)
			{
				*vg.longhand = longhand;
				vg_description = longhand;
			}

			ok = true;
		}
	}

	InstanceActionItems actionItems;
	std::vector<std::string> descriptions;
	descriptions.push_back(vg_description);
	descriptions.push_back(vg_warning);
	actionItems.push_back(std::make_pair(vg, std::shared_ptr<WidgetActionItem>(static_cast<WidgetActionItem *>(new WidgetActionItem__StringVector(descriptions)))));
	WidgetActionItemRequest_ACTION_SET_VG_DESCRIPTIONS action_request(WIDGET_ACTION_ITEM_REQUEST_REASON__UPDATE_ITEMS, actionItems);
	emit SetVGDescriptions(action_request);

}

void NewGeneManageVGs::on_pushButton_set_warning_for_vg_clicked()
{

	UIInputProject * project = projectManagerUI().getActiveUIInputProject();

	if (project == nullptr)
	{
		return;
	}

	UIMessager messager(project);

	if (!ui->listViewManageVGs)
	{
		boost::format msg("Invalid list view in NewGeneManageVGs widget.");
		QMessageBox msgBox;
		msgBox.setText(msg.str().c_str());
		msgBox.exec();
		return;
	}

	WidgetInstanceIdentifier vg;
	WidgetInstanceIdentifier uoa;
	bool is_selected = GetSelectedVG(vg, uoa);

	if (!is_selected)
	{
		return;
	}

	if (!vg.code || vg.code->empty())
	{
		boost::format msg("Invalid variable group being deleted.");
		QMessageBox msgBox;
		msgBox.setText(msg.str().c_str());
		msgBox.exec();
		return;
	}

	std::string vg_description;
	std::string vg_warning;
	if (vg.longhand)
	{
		vg_description = *vg.longhand;
	}

	bool ok {false};

	while (!ok)
	{
		boost::format msg("Set a warning message for variable group \"%1%\".");
		msg % *vg.code;
		std::string longhand;
		std::string notes;

		if (vg.notes.notes1)
		{
			notes = *vg.notes.notes1;
		}

		QDialog dialog(this);
		QFormLayout form(&dialog);
		form.addRow(new QLabel(QString(msg.str().c_str())));
		QLineEdit * longhandEdit = nullptr;
		QLineEdit * warningEdit = nullptr;

		int i = 0;
		{
			warningEdit = new QLineEdit(&dialog);
			setLineEditWidth(warningEdit);
			QString label = QString("Warning message:").arg(++i);
			form.addRow(label, warningEdit);
			warningEdit->setText(notes.c_str());
		}

		// Add some standard buttons (Cancel/Ok) at the bottom of the dialog
		QDialogButtonBox buttonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal, &dialog);
		form.addRow(&buttonBox);

		QObject::connect(&buttonBox, SIGNAL(accepted()), &dialog, SLOT(accept()));
		QObject::connect(&buttonBox, SIGNAL(rejected()), &dialog, SLOT(reject()));

		dialog.setMinimumWidth(640);

		if (dialog.exec() != QDialog::Accepted)
		{
			return;
		}

		notes = warningEdit->text().toStdString();

		boost::trim(notes);

		bool validNotes = true;
		std::string errorMsgDescription;
		std::string errorMsgNotes;

		if (validNotes)
		{
			validNotes = Validation::ValidateVgNotes(notes, errorMsgNotes);
		}

		if (!validNotes)
		{
			std::string errors;
			if (!validNotes)
			{
				errors += errorMsgNotes;
			}
			boost::format msg("%1%");
			msg % errors;
			QMessageBox msgBox;
			msgBox.setText(msg.str().c_str());
			msgBox.exec();
			ok = false;
		}
		else
		{
			if (vg.notes.notes1)
			{
				*vg.notes.notes1 = notes;
				vg_warning = notes;
			}

			ok = true;
		}
	}

	InstanceActionItems actionItems;
	std::vector<std::string> descriptions;
	descriptions.push_back(vg_description);
	descriptions.push_back(vg_warning);
	actionItems.push_back(std::make_pair(vg, std::shared_ptr<WidgetActionItem>(static_cast<WidgetActionItem *>(new WidgetActionItem__StringVector(descriptions)))));
	WidgetActionItemRequest_ACTION_SET_VG_DESCRIPTIONS action_request(WIDGET_ACTION_ITEM_REQUEST_REASON__UPDATE_ITEMS, actionItems);
	emit SetVGDescriptions(action_request);

}
