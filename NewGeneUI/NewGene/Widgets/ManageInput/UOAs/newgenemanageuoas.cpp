#include "newgenemanageuoas.h"
#include "ui_newgenemanageuoas.h"

#include <QStandardItemModel>
#include <QListView>
#include <QDialogButtonBox>

#include "../Project/uiprojectmanager.h"
#include "../Project/uiinputproject.h"
#include "../../Utilities/qsortfilterproxymodel_numberslast.h"
#include "../../Utilities/importdialoghelper.h"
#include "../../../../../NewGeneBackEnd/Utilities/Validation.h"
#include "../../../../NewGeneBackEnd/Utilities/TimeRangeHelper.h"
#include "../../../../NewGeneBackEnd/Model/InputModel.h"

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
			connect(this, SIGNAL(DeleteUOA(WidgetActionItemRequest_ACTION_DELETE_UOA)), project->getConnector(), SLOT(DeleteUOA(WidgetActionItemRequest_ACTION_DELETE_UOA)));
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

	WidgetInstanceIdentifier uoa_category;
	WidgetInstanceIdentifiers uoa_dmu_categories;
	bool is_selected = GetSelectedUoaCategory(uoa_category, uoa_dmu_categories);
	if (!is_selected)
	{
		return;
	}

	QStandardItemModel * dmuModel = static_cast<QStandardItemModel*>(ui->listViewManageUOAs->model());
	if (dmuModel == nullptr)
	{
		boost::format msg("Invalid model in NewGeneManageUOAs DMU category widget.");
		QMessageBox msgBox;
		msgBox.setText( msg.str().c_str() );
		msgBox.exec();
		return;
	}

	InstanceActionItems actionItems;
	actionItems.push_back(std::make_pair(uoa_category, std::shared_ptr<WidgetActionItem>(static_cast<WidgetActionItem*>(new WidgetActionItem__WidgetInstanceIdentifiers(uoa_dmu_categories)))));
	WidgetActionItemRequest_ACTION_DELETE_UOA action_request(WIDGET_ACTION_ITEM_REQUEST_REASON__REMOVE_ITEMS, actionItems);
	emit DeleteUOA(action_request);

}

void NewGeneManageUOAs::on_pushButton_createUOA_clicked()
{

	// From http://stackoverflow.com/a/17512615/368896
	QDialog dialog(this);
	QFormLayout form(&dialog);
	form.addRow(new QLabel("Create New UOA"));
	QList<QLineEdit *> fields;
	QLineEdit *lineEditCode = new QLineEdit(&dialog);
	QString labelCode = QString("Enter a brief identifying code for the new Unit of Analysis (all caps):");
	form.addRow(labelCode, lineEditCode);
	fields << lineEditCode;
	//QLineEdit *lineEditDescription = new QLineEdit(&dialog);
	//QString labelDescription = QString("Description:");
	//form.addRow(labelDescription, lineEditDescription);
	//fields << lineEditDescription;

	UIInputProject * project = projectManagerUI().getActiveUIInputProject();
	if (project == nullptr)
	{
		boost::format msg("Bad input project.  Unable to create \"New UOA\" dialog.");
		QMessageBox msgBox;
		msgBox.setText( msg.str().c_str() );
		msgBox.exec();
		return;
	}

	UIInputModel & ui_input_model = project->model();
	InputModel & backend_input_model = ui_input_model.getBackendModel();
	WidgetInstanceIdentifiers dmu_categories = backend_input_model.t_dmu_category.getIdentifiers();

	QWidget UoaConstructionWidget;
	QVBoxLayout formOverall;
	QWidget UoaConstructionPanes;
	QHBoxLayout formConstructionPanes;
	QVBoxLayout formConstructionDivider;
	QListView * lhs = nullptr;
	QListView * rhs = nullptr;
	ImportDialogHelper::AddUoaCreationBlock(dialog, form, UoaConstructionWidget, formOverall, UoaConstructionPanes, formConstructionPanes, formConstructionDivider, lhs, rhs, dmu_categories);

	if (!lhs || !rhs)
	{
		boost::format msg("Unable to create \"New UOA\" dialog.");
		QMessageBox msgBox;
		msgBox.setText( msg.str().c_str() );
		msgBox.exec();
		return;
	}

	// Add some standard buttons (Cancel/Ok) at the bottom of the dialog
	QDialogButtonBox buttonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal, &dialog);
	form.addRow(&buttonBox);

	std::string proposed_uoa_code;
	//std::string uoa_description;

	WidgetInstanceIdentifiers dmu_categories_to_use;

	QObject::connect(&buttonBox, SIGNAL(rejected()), &dialog, SLOT(reject()));
	QObject::connect(&buttonBox, &QDialogButtonBox::accepted, [&]()
	{

		std::string errorMsg;

		QLineEdit * proposed_uoa_code_field = fields[0];
		//QLineEdit * uoa_description_field = fields[1];
		if (proposed_uoa_code_field)
		{
			proposed_uoa_code = proposed_uoa_code_field->text().toStdString();
			//uoa_description = uoa_description_field->text().toStdString();
			if (proposed_uoa_code.empty())
			{
				boost::format msg("The UOA must have an identifying code (typically, a short, all-caps string).");
				QMessageBox msgBox;
				msgBox.setText( msg.str().c_str() );
				msgBox.exec();
				return false;
			}
		}
		else
		{
			boost::format msg("Unable to determine new UOA code.");
			QMessageBox msgBox;
			msgBox.setText( msg.str().c_str() );
			msgBox.exec();
			return false;
		}

		boost::trim(proposed_uoa_code);
		//boost::trim(uoa_description);

		bool valid = true;

		if (valid)
		{
			valid = Validation::ValidateUoaCode(proposed_uoa_code, errorMsg);
		}

		//if (valid)
		//{
		//	valid = Validation::ValidateUoaDescription(uoa_description, errorMsg);
		//}

		if (!valid)
		{
			boost::format msg("%1%");
			msg % errorMsg;
			QMessageBox msgBox;
			msgBox.setText( msg.str().c_str() );
			msgBox.exec();
			return false;
		}

		if (valid)
		{

			// retrieve the chosen DMU categories
			QStandardItemModel * rhsModel = static_cast<QStandardItemModel*>(rhs->model());
			if (rhsModel == nullptr)
			{
				boost::format msg("Invalid rhs list view items in Construct UOA popup.");
				QMessageBox msgBox;
				msgBox.setText( msg.str().c_str() );
				msgBox.exec();
				return false;
			}

			int dmurows = rhsModel->rowCount();
			QList<QStandardItem*> list;
			for (int dmurow = 0; dmurow < dmurows; ++dmurow)
			{
				QVariant dmu_category_variant = rhsModel->item(dmurow)->data();
				WidgetInstanceIdentifier dmu_category = dmu_category_variant.value<WidgetInstanceIdentifier>();
				dmu_categories_to_use.push_back(dmu_category);
			}

			dialog.accept();
		}

	});

	if (dialog.exec() != QDialog::Accepted)
	{
		return;
	}

	std::string new_uoa_code(proposed_uoa_code);

	InstanceActionItems actionItems;
	actionItems.push_back(std::make_pair(WidgetInstanceIdentifier(), std::shared_ptr<WidgetActionItem>(static_cast<WidgetActionItem*>(new WidgetActionItem__WidgetInstanceIdentifiers_Plus_String(dmu_categories_to_use, new_uoa_code)))));
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

bool NewGeneManageUOAs::GetSelectedUoaCategory(WidgetInstanceIdentifier & uoa_category, WidgetInstanceIdentifiers & uoa_dmu_categories)
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
