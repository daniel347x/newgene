#include "kadspinbox.h"

KadSpinBox::KadSpinBox( QWidget * parent, WidgetInstanceIdentifier data_instance_, UIOutputProject * project ) :

	QSpinBox( parent ),

	NewGeneWidget( WidgetCreationInfo(
										this, // 'this' pointer is cast by compiler to proper Widget instance, which is already created due to order in which base classes appear in class definition
										parent,
										WIDGET_NATURE_OUTPUT_WIDGET,
										KAD_SPIN_CONTROL_WIDGET,
										false,
										data_instance_
									 )
				   )

{

   PrepareOutputWidget();

   if (data_instance.uuid && project)
   {

       project->RegisterInterestInChange(this, DATA_CHANGE_TYPE__OUTPUT_MODEL__KAD_COUNT_CHANGE, true, *data_instance.uuid);

	   UpdateOutputConnections(UIProjectManager::ESTABLISH_CONNECTIONS_OUTPUT_PROJECT, project);
	   WidgetDataItemRequest_KAD_SPIN_CONTROL_WIDGET request(WIDGET_DATA_ITEM_REQUEST_REASON__REFRESH_ALL_WIDGETS, data_instance);
	   emit RefreshWidget(request);

   }

}

void KadSpinBox::RefreshAllWidgets()
{
	WidgetDataItemRequest_KAD_SPIN_CONTROL_WIDGET request(WIDGET_DATA_ITEM_REQUEST_REASON__REFRESH_ALL_WIDGETS);
	emit RefreshWidget(request);
}

void KadSpinBox::UpdateOutputConnections(UIProjectManager::UPDATE_CONNECTIONS_TYPE connection_type, UIOutputProject * project)
{
  NewGeneWidget::UpdateOutputConnections(connection_type, project);
  connect(this, SIGNAL(RefreshWidget(WidgetDataItemRequest_KAD_SPIN_CONTROL_WIDGET)), outp->getConnector(), SLOT(RefreshWidget(WidgetDataItemRequest_KAD_SPIN_CONTROL_WIDGET)));
  connect(this, SIGNAL(SignalReceiveVariableItemChanged(WidgetActionItemRequest_ACTION_KAD_COUNT_CHANGE)), outp->getConnector(), SLOT(ReceiveVariableItemChanged(WidgetActionItemRequest_ACTION_KAD_COUNT_CHANGE)));
}

void KadSpinBox::WidgetDataRefreshReceive(WidgetDataItem_KAD_SPIN_CONTROL_WIDGET widget_data)
{

  if (!data_instance.uuid || !widget_data.identifier || !widget_data.identifier->uuid || (*data_instance.uuid) != (*widget_data.identifier->uuid) )
  {
      boost::format msg("Invalid widget refresh in NewGeneVariableSummary widget.");
      QMessageBox msgBox;
      msgBox.setText( msg.str().c_str() );
      msgBox.exec();
      return;
  }

   // Since these are not checkboxes yet, the following signal will never currently be called
   //connect(model, SIGNAL(itemChanged(QStandardItem*)), this, SLOT(ReceiveVariableItemChanged(QStandardItem*)));

}
