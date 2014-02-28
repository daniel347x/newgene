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
#include <QSortFilterProxyModel>
#include <QPushButton>
#include <QFileDialog>
#include <QRadioButton>

#ifndef Q_MOC_RUN
#	include <boost/algorithm/string.hpp>
#	include <boost/regex.hpp>
#endif

#include "../Project/uiprojectmanager.h"
#include "../Project/uiinputproject.h"
#include "../../Utilities/qsortfilterproxymodel_numberslast.h"
#include "../../Utilities/importdialoghelper.h"
#include "../../../../../NewGeneBackEnd/Utilities/Validation.h"
#include "../../../../NewGeneBackEnd/Utilities/TimeRangeHelper.h"

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
			project->RegisterInterestInChange(this, DATA_CHANGE_TYPE__INPUT_MODEL__DMU_MEMBERS_CHANGE, false, "");
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
		}
	}
	else if (connection_type == NewGeneWidget::RELEASE_CONNECTIONS_INPUT_PROJECT)
	{
		if (project)
		{
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

	QSortFilterProxyModel_NumbersLast * oldModel = static_cast<QSortFilterProxyModel_NumbersLast*>(ui->listView_dmu_members->model());
	if (oldModel != nullptr)
	{
		delete oldModel;
	}

	if(!selected.indexes().isEmpty())
	{

		QStandardItemModel * dmuModel = static_cast<QStandardItemModel*>(ui->listView_dmus->model());
		QModelIndex selectedIndex = selected.indexes().first();
		QVariant dmu_and_members_variant = dmuModel->item(selectedIndex.row())->data();
		std::pair<WidgetInstanceIdentifier, WidgetInstanceIdentifiers> dmu_and_members = dmu_and_members_variant.value<std::pair<WidgetInstanceIdentifier, WidgetInstanceIdentifiers>>();
		WidgetInstanceIdentifier & dmu_category = dmu_and_members.first;
		WidgetInstanceIdentifiers & dmu_members = dmu_and_members.second;

		ResetDmuMembersPane(dmu_category, dmu_members);

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
			if (proposed_dmu_name.empty())
			{
				boost::format msg("The DMU category must have a name.");
				QMessageBox msgBox;
				msgBox.setText( msg.str().c_str() );
				msgBox.exec();
				return;
			}
		}
		else
		{
			boost::format msg("Unable to determine new DMU name and description.");
			QMessageBox msgBox;
			msgBox.setText( msg.str().c_str() );
			msgBox.exec();
			return;
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
				invalid_string = ": The length is too long (maximum length: 255).";
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
		boost::format msg("The description is too long (maximum length: 4096).");
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

	WidgetInstanceIdentifier dmu;
	WidgetInstanceIdentifiers dmu_members;
	bool is_selected = GetSelectedDmuCategory(dmu, dmu_members);
	if (!is_selected)
	{
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

	InstanceActionItems actionItems;
	actionItems.push_back(std::make_pair(WidgetInstanceIdentifier(dmu), std::shared_ptr<WidgetActionItem>(static_cast<WidgetActionItem*>(new WidgetActionItem__WidgetInstanceIdentifier(dmu)))));
	WidgetActionItemRequest_ACTION_DELETE_DMU action_request(WIDGET_ACTION_ITEM_REQUEST_REASON__REMOVE_ITEMS, actionItems);
	emit DeleteDMU(action_request);

}

void DisplayDMUsRegion::on_pushButton_refresh_dmu_members_from_file_clicked()
{

	WidgetInstanceIdentifier dmu_category;
	WidgetInstanceIdentifiers dmu_members;
	bool found = GetSelectedDmuCategory(dmu_category, dmu_members);
	if (!found)
	{
		boost::format msg("A DMU category must be selected.");
		QMessageBox msgBox;
		msgBox.setText( msg.str().c_str() );
		msgBox.exec();
		return;
	}

	QList<QLineEdit *> fields;

	QDialog dialog(this);
	QFormLayout form(&dialog);

	form.addRow(new QLabel("DMU member refresh details"));

	QString labelColumnNameUuid = QString("Enter 'code' column label:");
	QLineEdit *lineEditColumnNameUuid = new QLineEdit(&dialog);
	form.addRow(labelColumnNameUuid, lineEditColumnNameUuid);
	fields << lineEditColumnNameUuid;

	QString labelColumnNameCode = QString("Enter 'abbreviation' column label (optional):");
	QLineEdit *lineEditColumnNameCode = new QLineEdit(&dialog);
	form.addRow(labelColumnNameCode, lineEditColumnNameCode);
	fields << lineEditColumnNameCode;

	QString labelColumnNameDescription = QString("Enter 'description' column label (optional):");
	QLineEdit *lineEditColumnNameDescription = new QLineEdit(&dialog);
	form.addRow(labelColumnNameDescription, lineEditColumnNameDescription);
	fields << lineEditColumnNameDescription;

	QList<QLineEdit *> fieldsFileChooser;
	std::vector<std::string> const & fileChooserStrings { "Choose comma-delimited file", "Choose DMU comma-delimited data file location", "", "" };
	ImportDialogHelper::AddFileChooserBlock(dialog, form, fieldsFileChooser, fileChooserStrings);

	QList<QLineEdit *> fieldsTimeRange;
	QList<QRadioButton *> radioButtonsTimeRange;
	ImportDialogHelper::AddTimeRangeSelectorBlock(dialog, form, fieldsTimeRange, radioButtonsTimeRange);

	// Add some standard buttons (Cancel/Ok) at the bottom of the dialog
	QDialogButtonBox buttonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal, &dialog);
	form.addRow(&buttonBox);

	std::string data_column_name_uuid;
	std::string data_column_name_code;
	std::string data_column_name_description;

	std::vector<std::string> dataFileChooser;
	std::vector<std::string> dataTimeRange;

	TimeRange::TimeRangeImportMode timeRangeMode = TimeRange::TIME_RANGE_IMPORT_MODE__NONE;

	QObject::connect(&buttonBox, SIGNAL(rejected()), &dialog, SLOT(reject()));
	QObject::connect(&buttonBox, &QDialogButtonBox::accepted, [&]()
	{
		// data validation here
		bool valid = true;

		std::string errorMsg;

		QLineEdit * data_column_name_uuid_field = fields[0];
		QLineEdit * data_column_name_code_field = fields[1];
		QLineEdit * data_column_name_description_field = fields[2];

		// Custom validation
		if (valid)
		{

			if (!data_column_name_uuid_field && !data_column_name_code_field && !data_column_name_description_field)
			{
				valid = false;
				errorMsg = "Invalid DMU.";
			}

		}

		if (valid)
		{

			data_column_name_uuid = data_column_name_uuid_field->text().toStdString();
			data_column_name_code = data_column_name_code_field->text().toStdString();
			data_column_name_description = data_column_name_description_field->text().toStdString();

		}

		if (valid)
		{
			valid = Validation::ValidateColumnName(data_column_name_uuid, "Code", true, errorMsg);
		}

		if (valid)
		{
			valid = Validation::ValidateColumnName(data_column_name_code, "Abbreviation", false, errorMsg);
		}

		if (valid)
		{
			valid = Validation::ValidateColumnName(data_column_name_description, "Description", false, errorMsg);
		}

		// Factored validation
		if (valid)
		{
			valid = ImportDialogHelper::ValidateFileChooserBlock(fieldsFileChooser, dataFileChooser, errorMsg);
		}
		if (valid)
		{
			valid = ImportDialogHelper::ValidateTimeRangeBlock(fieldsTimeRange, radioButtonsTimeRange, dataTimeRange, timeRangeMode, errorMsg);
		}

		if (!valid)
		{
			boost::format msg(errorMsg);
			QMessageBox msgBox;
			msgBox.setText( msg.str().c_str() );
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

	InstanceActionItems actionItems;
	actionItems.push_back(std::make_pair(dmu_category, std::shared_ptr<WidgetActionItem>(static_cast<WidgetActionItem*>(new WidgetActionItem__StringVector(std::vector<std::string>{data_column_file_pathname.string(), data_column_name_uuid, data_column_name_code, data_column_name_description})))));
	WidgetActionItemRequest_ACTION_REFRESH_DMUS_FROM_FILE action_request(WIDGET_ACTION_ITEM_REQUEST_REASON__DO_ACTION, actionItems);
	emit RefreshDMUsFromFile(action_request);

}

void DisplayDMUsRegion::on_pushButton_add_dmu_member_by_hand_clicked()
{

	// Get selected DMU category
	WidgetInstanceIdentifier dmu_category;
	WidgetInstanceIdentifiers dmu_members;
	bool is_selected = GetSelectedDmuCategory(dmu_category, dmu_members);
	if (!is_selected)
	{
		return;
	}

	// From http://stackoverflow.com/a/17512615/368896
	QDialog dialog(this);
	QFormLayout form(&dialog);
	boost::format title("Add DMU member to %1%");
	title % *dmu_category.code;
	form.addRow(new QLabel(title.str().c_str()));
	QList<QLineEdit *> fields;
	QLineEdit *lineEditCode = new QLineEdit(&dialog);
	QString labelCode = QString("Enter uniquely identirying DMU member code:");
	form.addRow(labelCode, lineEditCode);
	fields << lineEditCode;
	QLineEdit *lineEditName = new QLineEdit(&dialog);
	QString labelName = QString("(Optional) Enter a short abbreviation:");
	form.addRow(labelName, lineEditName);
	fields << lineEditName;
	QLineEdit *lineEditDescription = new QLineEdit(&dialog);
	QString labelDescription = QString("(Optional) Enter full descriptive text:");
	form.addRow(labelDescription, lineEditDescription);
	fields << lineEditDescription;

	// Add some standard buttons (Cancel/Ok) at the bottom of the dialog
	QDialogButtonBox buttonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal, &dialog);
	form.addRow(&buttonBox);

	std::string proposed_dmu_member_uuid;
	std::string proposed_dmu_member_code;
	std::string proposed_dmu_member_description;

	QObject::connect(&buttonBox, SIGNAL(rejected()), &dialog, SLOT(reject()));
	QObject::connect(&buttonBox, &QDialogButtonBox::accepted, [&]()
	{

		bool valid = true;
		std::string errorMsg;

		QLineEdit * proposed_dmu_member_uuid_field = fields[0];
		QLineEdit * proposed_dmu_member_code_field = fields[1];
		QLineEdit * proposed_dmu_member_description_field = fields[2];
		if (!proposed_dmu_member_uuid_field || !proposed_dmu_member_code_field || !proposed_dmu_member_description_field)
		{

			boost::format msg("Unable to determine new DMU member code.");
			errorMsg = msg.str();
			valid = false;

		}

		if (valid)
		{

			proposed_dmu_member_uuid = proposed_dmu_member_uuid_field->text().toStdString();
			proposed_dmu_member_code = proposed_dmu_member_code_field->text().toStdString();
			proposed_dmu_member_description = proposed_dmu_member_description_field->text().toStdString();

		}

		if (valid)
		{
			valid = Validation::ValidateDmuMemberUUID(proposed_dmu_member_uuid, errorMsg);
		}

		if (valid)
		{
			valid = Validation::ValidateDmuMemberCode(proposed_dmu_member_code, errorMsg);
		}

		if (valid)
		{
			valid = Validation::ValidateDmuMemberCode(proposed_dmu_member_description, errorMsg);
		}

		if (!valid)
		{
			QMessageBox msgBox;
			msgBox.setText(errorMsg.c_str());
			msgBox.exec();
			return;
		}

		dialog.accept();

	});

	if (dialog.exec() != QDialog::Accepted)
	{
		return;
	}

	InstanceActionItems actionItems;
	actionItems.push_back(std::make_pair(dmu_category, std::shared_ptr<WidgetActionItem>(static_cast<WidgetActionItem*>(new WidgetActionItem__StringVector(std::vector<std::string>{proposed_dmu_member_uuid, proposed_dmu_member_code, proposed_dmu_member_description})))));
	WidgetActionItemRequest_ACTION_ADD_DMU_MEMBERS action_request(WIDGET_ACTION_ITEM_REQUEST_REASON__ADD_ITEMS, actionItems);

	emit AddDMUMembers(action_request);

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

	QSortFilterProxyModel_NumbersLast * dmuMembersModel = static_cast<QSortFilterProxyModel_NumbersLast*>(ui->listView_dmu_members->model());
	if (dmuMembersModel == nullptr)
	{
		boost::format msg("Invalid model in DisplayDMUsRegion DMU category widget.");
		QMessageBox msgBox;
		msgBox.setText( msg.str().c_str() );
		msgBox.exec();
		return;
	}

	QStandardItemModel * model = nullptr;
	try
	{
		model = dynamic_cast<QStandardItemModel *>(dmuMembersModel->sourceModel());
	}
	catch (std::bad_cast &)
	{
		boost::format msg("Unable to obtain model for DMU member list.");
		QMessageBox msgBox;
		msgBox.setText( msg.str().c_str() );
		msgBox.exec();
		return;
	}

	InstanceActionItems actionItems;

	for (int i = 0; i < dmuMembersModel->rowCount(); ++i)
	{

		QModelIndex testIndex = dmuMembersModel->index(i, 0);
		QStandardItem * testItem = model->item(testIndex.row());
		if (testItem->checkState() == Qt::Checked)
		{
			WidgetInstanceIdentifier dmu_member = testItem->data().value<WidgetInstanceIdentifier>();
			actionItems.push_back(std::make_pair(WidgetInstanceIdentifier(dmu_member), std::shared_ptr<WidgetActionItem>(static_cast<WidgetActionItem*>(new WidgetActionItem__WidgetInstanceIdentifier(dmu_member)))));
		}

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
	QSortFilterProxyModel_NumbersLast * model = static_cast<QSortFilterProxyModel_NumbersLast*>(ui->listView_dmu_members->model());
	if (model == nullptr)
	{
		return;
	}

	QAbstractItemModel * sourceModel = model->sourceModel();
	if (sourceModel == nullptr)
	{
		return;
	}

	QStandardItemModel * sourceStandardModel = nullptr;
	try
	{
		sourceStandardModel = dynamic_cast<QStandardItemModel*>(sourceModel);
	}
	catch (std::bad_cast &)
	{
		// guess not
		return;
	}

	int nrows = sourceStandardModel->rowCount();
	for (int row=0; row<nrows; ++row) {
		QStandardItem * item = sourceStandardModel->item(row, 0);
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
	QSortFilterProxyModel_NumbersLast * model = static_cast<QSortFilterProxyModel_NumbersLast*>(ui->listView_dmu_members->model());
	if (model == nullptr)
	{
		return;
	}

	QAbstractItemModel * sourceModel = model->sourceModel();
	if (sourceModel == nullptr)
	{
		return;
	}

	QStandardItemModel * sourceStandardModel = nullptr;
	try
	{
		sourceStandardModel = dynamic_cast<QStandardItemModel*>(sourceModel);
	}
	catch (std::bad_cast &)
	{
		// guess not
		return;
	}

	int nrows = sourceStandardModel->rowCount();
	for (int row=0; row<nrows; ++row) {
		QStandardItem * item = sourceStandardModel->item(row, 0);
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

	QSortFilterProxyModel_NumbersLast * memberModel = static_cast<QSortFilterProxyModel_NumbersLast*>(ui->listView_dmu_members->model());
	if (memberModel == nullptr)
	{
		boost::format msg("Invalid list view items in DisplayDMUsRegion widget.");
		QMessageBox msgBox;
		msgBox.setText( msg.str().c_str() );
		msgBox.exec();
		return;
	}

	QSortFilterProxyModel_NumbersLast * proxyModel = nullptr;

	std::for_each(change_message.changes.cbegin(), change_message.changes.cend(), [this, &itemModel, &memberModel, &proxyModel](DataChange const & change)
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

bool DisplayDMUsRegion::GetSelectedDmuCategory(WidgetInstanceIdentifier & dmu_category, WidgetInstanceIdentifiers & dmu_members)
{

	QItemSelectionModel * dmu_selectionModel = ui->listView_dmus->selectionModel();
	if (dmu_selectionModel == nullptr)
	{
		boost::format msg("Invalid selection in DisplayDMUsRegion widget.");
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

	QStandardItemModel * dmuModel = static_cast<QStandardItemModel*>(ui->listView_dmus->model());
	if (dmuModel == nullptr)
	{
		boost::format msg("Invalid model in DisplayDMUsRegion DMU category widget.");
		QMessageBox msgBox;
		msgBox.setText( msg.str().c_str() );
		msgBox.exec();
		return false;
	}

	QVariant dmu_and_members_variant = dmuModel->item(selectedIndex.row())->data();
	std::pair<WidgetInstanceIdentifier, WidgetInstanceIdentifiers> dmu_and_members = dmu_and_members_variant.value<std::pair<WidgetInstanceIdentifier, WidgetInstanceIdentifiers>>();
	dmu_category = dmu_and_members.first;
	dmu_members = dmu_and_members.second;

	return true;

}

void DisplayDMUsRegion::ResetDmuMembersPane(WidgetInstanceIdentifier const & dmu_category, WidgetInstanceIdentifiers const & dmu_members)
{

	QSortFilterProxyModel_NumbersLast * oldModel = static_cast<QSortFilterProxyModel_NumbersLast*>(ui->listView_dmu_members->model());
	if (oldModel != nullptr)
	{
		delete oldModel;
	}

	QItemSelectionModel * oldSelectionModel = ui->listView_dmu_members->selectionModel();
	QStandardItemModel * model = new QStandardItemModel();

	int index = 0;
	std::for_each(dmu_members.cbegin(), dmu_members.cend(), [this, &index, &model](WidgetInstanceIdentifier const & dmu_member)
	{
		if (dmu_member.uuid && !dmu_member.uuid->empty())
		{

			std::string text = Table_DMU_Instance::GetDmuMemberDisplayText(dmu_member);

			QStandardItem * item = new QStandardItem();
			item->setText(text.c_str());
			item->setEditable(false);
			item->setCheckable(true);
			QVariant v;
			v.setValue(dmu_member);
			item->setData(v);
			model->setItem( index, item );

			++index;

		}
	});

	QSortFilterProxyModel_NumbersLast *proxyModel = new QSortFilterProxyModel_NumbersLast(ui->listView_dmu_members);
	proxyModel->setDynamicSortFilter(true);
	proxyModel->setSourceModel(model);
	proxyModel->sort(0);
	ui->listView_dmu_members->setModel(proxyModel);
	if (oldSelectionModel) delete oldSelectionModel;

}
