#include "newgenevariablesummarygroup.h"
#include "ui_newgenevariablesummarygroup.h"
#include "../../../NewGeneBackEnd/Utilities/WidgetIdentifier.h"

#include <QStandardItem>

#include "../Project/uiprojectmanager.h"
#include "../Project/uiinputproject.h"
#include "../Project/uioutputproject.h"

NewGeneVariableSummaryGroup::NewGeneVariableSummaryGroup( QWidget * parent, WidgetInstanceIdentifier data_instance_, UIOutputProject * project ) :

	QGroupBox( parent ),

	NewGeneWidget( WidgetCreationInfo(
										this, // 'this' pointer is cast by compiler to proper Widget instance, which is already created due to order in which base classes appear in class definition
										parent,
										WIDGET_NATURE_OUTPUT_WIDGET,
										VARIABLE_GROUPS_SUMMARY_VARIABLE_GROUP_INSTANCE,
										false,
										data_instance_
									 )
				   ),

	ui( new Ui::NewGeneVariableSummaryGroup )

{

	ui->setupUi( this );

	PrepareOutputWidget();

	if (data_instance.uuid && project)
	{

		project->RegisterInterestInChange(this, DATA_CHANGE_TYPE__OUTPUT_MODEL__VG_CATEGORY_SET_MEMBER_SELECTION, true, *data_instance.uuid);

		UpdateOutputConnections(NewGeneWidget::ESTABLISH_CONNECTIONS_OUTPUT_PROJECT, project);
		WidgetDataItemRequest_VARIABLE_GROUPS_SUMMARY_VARIABLE_GROUP_INSTANCE request(WIDGET_DATA_ITEM_REQUEST_REASON__REFRESH_ALL_WIDGETS, data_instance);
		emit RefreshWidget(request);

	}

}

NewGeneVariableSummaryGroup::~NewGeneVariableSummaryGroup()
{
	if (outp)
	{
		outp->UnregisterInterestInChanges(this);
	}
	delete ui;
}

void NewGeneVariableSummaryGroup::changeEvent( QEvent * e )
{
	QGroupBox::changeEvent( e );

	switch ( e->type() )
	{
		case QEvent::LanguageChange:
			ui->retranslateUi( this );
			break;

		default:
			break;
	}
}

void NewGeneVariableSummaryGroup::UpdateOutputConnections(NewGeneWidget::UPDATE_CONNECTIONS_TYPE connection_type, UIOutputProject * project)
{
	if (connection_type == NewGeneWidget::RELEASE_CONNECTIONS_OUTPUT_PROJECT)
	{
		outp->UnregisterInterestInChanges(this);
	}

	NewGeneWidget::UpdateOutputConnections(connection_type, project);

	if (connection_type == NewGeneWidget::ESTABLISH_CONNECTIONS_OUTPUT_PROJECT)
	{
		connect(this, SIGNAL(RefreshWidget(WidgetDataItemRequest_VARIABLE_GROUPS_SUMMARY_VARIABLE_GROUP_INSTANCE)), outp->getConnector(), SLOT(RefreshWidget(WidgetDataItemRequest_VARIABLE_GROUPS_SUMMARY_VARIABLE_GROUP_INSTANCE)));
		connect(this, SIGNAL(SignalReceiveVariableItemChanged(WidgetActionItemRequest_ACTION_VARIABLE_GROUP_SET_MEMBER_SELECTION_CHANGED)), outp->getConnector(), SLOT(ReceiveVariableItemChanged(WidgetActionItemRequest_ACTION_VARIABLE_GROUP_SET_MEMBER_SELECTION_CHANGED)));
	}
	else if (connection_type == NewGeneWidget::RELEASE_CONNECTIONS_OUTPUT_PROJECT)
	{
		Empty();
	}
}

void NewGeneVariableSummaryGroup::RefreshAllWidgets()
{
	if (outp == nullptr)
	{
		Empty();
		return;
	}
	WidgetDataItemRequest_VARIABLE_GROUPS_SUMMARY_VARIABLE_GROUP_INSTANCE request(WIDGET_DATA_ITEM_REQUEST_REASON__REFRESH_ALL_WIDGETS);
	emit RefreshWidget(request);
}

void NewGeneVariableSummaryGroup::WidgetDataRefreshReceive(WidgetDataItem_VARIABLE_GROUPS_SUMMARY_VARIABLE_GROUP_INSTANCE widget_data)
{

	if (!data_instance.uuid || !widget_data.identifier || !widget_data.identifier->uuid || (*data_instance.uuid) != (*widget_data.identifier->uuid) )
	{
		boost::format msg("Invalid widget refresh in NewGeneVariableSummary widget.");
		QMessageBox msgBox;
		msgBox.setText( msg.str().c_str() );
		msgBox.exec();
		return;
	}

	WidgetInstanceIdentifiers vg_members = widget_data.identifiers;
	bool success = ResetAll(vg_members);

}

void NewGeneVariableSummaryGroup::ReceiveVariableItemChanged(QStandardItem * /* currentItem */)
{

	// Since the items are not checkboxes yet, this slot will never currently be called - unlike the Variable Group items (not the summary), which *are* checkboxes

	QStandardItemModel * model = static_cast<QStandardItemModel*>(ui->listView->model());
	if (model == nullptr)
	{
		// Todo: messager error
		return;
	}

	InstanceActionItems actionItems;
	WidgetActionItemRequest_ACTION_VARIABLE_GROUP_SET_MEMBER_SELECTION_CHANGED action_request(WIDGET_ACTION_ITEM_REQUEST_REASON__UPDATE_ITEMS, actionItems);
	emit SignalReceiveVariableItemChanged(action_request);

}

void NewGeneVariableSummaryGroup::HandleChanges(DataChangeMessage const & change_message)
{
  std::for_each(change_message.changes.cbegin(), change_message.changes.cend(), [this](DataChange const & change)
  {
	  switch (change.change_type)
	  {
		  case DATA_CHANGE_TYPE::DATA_CHANGE_TYPE__OUTPUT_MODEL__VG_CATEGORY_SET_MEMBER_SELECTION:
			  {
				  switch (change.change_intention)
				  {
					  case DATA_CHANGE_INTENTION__ADD:
					  case DATA_CHANGE_INTENTION__REMOVE:
						  {
							  // This is the OUTPUT model changing.
							  // "Add" means to simply add an item that is CHECKED (previously unchecked) -
							  // NOT to add a new variable.  That would be an input model change type.

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
								   bool found = false;
								   for (int n=0; n<number_variables; ++n)
								   {
									   QStandardItem * currentItem = model->item(n);
									   if (currentItem)
									   {
										   QVariant currentIdentifier = currentItem->data();
										   WidgetInstanceIdentifier identifier = currentIdentifier.value<WidgetInstanceIdentifier>();
										   if (identifier.uuid && child_identifier.uuid && *identifier.uuid == *child_identifier.uuid)
										   {

											   // Existing item found - remove it

											   if (change.change_intention == DATA_CHANGE_INTENTION__REMOVE)
											   {
												   model->removeRow(n);
												   --n;
												   --number_variables;
												   found = true;
											   }

										   }

									   }
								   }

								   if (!found)
								   {

									   // Item not found - add it

									   if (change.change_intention == DATA_CHANGE_INTENTION__ADD)
									   {
										   bool added = false;
										   for (int n=0; n<number_variables; ++n)
										   {
											   QStandardItem * currentItem = model->item(n);
											   if (currentItem)
											   {
												   QVariant currentIdentifier = currentItem->data();
												   WidgetInstanceIdentifier identifier = currentIdentifier.value<WidgetInstanceIdentifier>();
												   if (child_identifier < identifier)
												   {
													   QStandardItem * item = new QStandardItem();
													   item->setText(QString(child_identifier.longhand->c_str()));
													   item->setEditable(false);
													   QVariant v;
													   v.setValue(child_identifier);
													   item->setData(v);
													   model->insertRow( n, item );
													   ++n;
													   ++number_variables;
													   added = true;
													   break;
												   }
											   }
										   }
										   if (!added)
										   {
											   QStandardItem * item = new QStandardItem();
											   item->setText(QString(child_identifier.longhand->c_str()));
											   item->setEditable(false);
											   QVariant v;
											   v.setValue(child_identifier);
											   item->setData(v);
											   model->insertRow( number_variables, item );
											   added = true;
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
					  case DATA_CHANGE_INTENTION__RESET_ALL:
						  {

							  WidgetInstanceIdentifiers vg_members = change.child_identifiers;
							  bool success = ResetAll(vg_members);

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

bool NewGeneVariableSummaryGroup::ResetAll(WidgetInstanceIdentifiers const & vg_members)
{

	if (!ui->listView)
	{
		boost::format msg("Invalid list view in NewGeneVariableSummaryGroup widget.");
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
	std::for_each(vg_members.cbegin(), vg_members.cend(), [this, &index, &model](WidgetInstanceIdentifier const & identifier)
	{
		if (identifier.longhand && !identifier.longhand->empty())
		{

			QStandardItem * item = new QStandardItem();
			item->setText(QString(identifier.longhand->c_str()));
			item->setEditable(false);
			QVariant v;
			v.setValue(identifier);
			item->setData(v);
			model->setItem( index, item );

			++index;

		}
	});

	// Since these are not checkboxes yet, the following signal will never currently be called - unlike the Variable Group items (not the summary), which *are* checkboxes
	connect(model, SIGNAL(itemChanged(QStandardItem*)), this, SLOT(ReceiveVariableItemChanged(QStandardItem*)));

	ui->listView->setModel(model);
	if (oldSelectionModel) delete oldSelectionModel;

}
