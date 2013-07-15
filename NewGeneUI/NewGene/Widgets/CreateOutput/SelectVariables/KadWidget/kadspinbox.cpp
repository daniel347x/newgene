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

KadSpinBox::~KadSpinBox()
{
	outp->UnregisterInterestInChanges(this);
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

	connect(this, SIGNAL(valueChanged(int)), this, SLOT(ReceiveVariableItemChanged(int)));

}

void KadSpinBox::ReceiveVariableItemChanged(int newValue)
{

	InstanceActionItems actionItems;
	actionItems.push_back(std::make_pair(data_instance, std::shared_ptr<WidgetActionItem>(static_cast<WidgetActionItem*>(new WidgetActionItem__Spinbox(newValue)))));
	WidgetActionItemRequest_ACTION_KAD_COUNT_CHANGE action_request(WIDGET_ACTION_ITEM_REQUEST_REASON__UPDATE_ITEMS, actionItems);
	emit SignalReceiveVariableItemChanged(action_request);

}

void KadSpinBox::HandleChanges(DataChangeMessage const & change_message)
{
	std::for_each(change_message.changes.cbegin(), change_message.changes.cend(), [this](DataChange const & change)
	{
		switch (change.change_type)
		{
			case DATA_CHANGE_TYPE::DATA_CHANGE_TYPE__OUTPUT_MODEL__KAD_COUNT_CHANGE:
				{
					switch (change.change_intention)
					{
						case DATA_CHANGE_INTENTION__ADD:
						case DATA_CHANGE_INTENTION__REMOVE:
							{
								// Should never receive this.
							}
							break;
						case DATA_CHANGE_INTENTION__UPDATE:
							{
								// This is the OUTPUT model changing.
								// "Add" means to simply add an item that is CHECKED (previously unchecked) -
								// NOT to add a new variable.  That would be input model change type.

								if (change.child_identifiers.size() == 0)
								{
									return; // from lambda
								}

								std::for_each(change.child_identifiers.cbegin(), change.child_identifiers.cend(), [&change, this](WidgetInstanceIdentifier const & child_identifier)
								{
									int value_ = value();
									if (data_instance.uuid && child_identifier.uuid && *data_instance.uuid == *child_identifier.uuid)
									{

										if (change.change_intention == DATA_CHANGE_INTENTION__UPDATE)
										{
											DataChangePacket_int * packet = static_cast<DataChangePacket_int *>(change.getPacket());
											if (packet)
											{
												if (value_ != packet->getValue())
												{
													setValue(packet->getValue());
												}
											}
											else
											{
												// error condition ... todo
											}
										}

									}
									else
									{
										// error condition ... todo
									}
								});

							}
						case DATA_CHANGE_INTENTION__RESET_ALL:
							{
								// Ditto above.
							}
							break;
					}
				}
				break;
		}
	});
}
