#include "kadspinbox.h"

#include "../Project/uiprojectmanager.h"
#include "../Project/uiinputproject.h"
#include "../Project/uioutputproject.h"

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

   connect(this, SIGNAL(valueChanged(int)), this, SLOT(ReceiveVariableItemChanged(int)));

   if (data_instance.uuid && project)
   {

	   project->RegisterInterestInChange(this, DATA_CHANGE_TYPE__OUTPUT_MODEL__KAD_COUNT_CHANGE, true, *data_instance.uuid);
	   project->RegisterInterestInChange(this, DATA_CHANGE_TYPE__OUTPUT_MODEL__VG_CATEGORY_SET_MEMBER_SELECTION, false, *data_instance.uuid);

	   UpdateOutputConnections(NewGeneWidget::ESTABLISH_CONNECTIONS_OUTPUT_PROJECT, project);
	   WidgetDataItemRequest_KAD_SPIN_CONTROL_WIDGET request(0, WIDGET_DATA_ITEM_REQUEST_REASON__REFRESH_ALL_WIDGETS, data_instance);
	   emit RefreshWidget(request);

   }

}

KadSpinBox::~KadSpinBox()
{
	if (outp)
	{
		outp->UnregisterInterestInChanges(this);
	}
}

void KadSpinBox::RefreshAllWidgets()
{
	WidgetDataItemRequest_KAD_SPIN_CONTROL_WIDGET request(value(), WIDGET_DATA_ITEM_REQUEST_REASON__REFRESH_ALL_WIDGETS);
	emit RefreshWidget(request);
}

void KadSpinBox::UpdateOutputConnections(NewGeneWidget::UPDATE_CONNECTIONS_TYPE connection_type, UIOutputProject * project)
{
	if (connection_type == NewGeneWidget::RELEASE_CONNECTIONS_OUTPUT_PROJECT)
	{
		outp->UnregisterInterestInChanges(this);
	}

	NewGeneWidget::UpdateOutputConnections(connection_type, project);

	if (connection_type == NewGeneWidget::ESTABLISH_CONNECTIONS_OUTPUT_PROJECT)
	{
		connect(this, SIGNAL(RefreshWidget(WidgetDataItemRequest_KAD_SPIN_CONTROL_WIDGET)), outp->getConnector(), SLOT(RefreshWidget(WidgetDataItemRequest_KAD_SPIN_CONTROL_WIDGET)));
		connect(this, SIGNAL(SignalReceiveVariableItemChanged(WidgetActionItemRequest_ACTION_KAD_COUNT_CHANGE)), outp->getConnector(), SLOT(ReceiveVariableItemChanged(WidgetActionItemRequest_ACTION_KAD_COUNT_CHANGE)));
	}
}

void KadSpinBox::WidgetDataRefreshReceive(WidgetDataItem_KAD_SPIN_CONTROL_WIDGET widget_data)
{

	if (!data_instance.uuid || !widget_data.identifier || !widget_data.identifier->uuid || (*data_instance.uuid) != (*widget_data.identifier->uuid) )
	{
		boost::format msg("Invalid widget refresh in KadSpinBox widget.");
		QMessageBox msgBox;
		msgBox.setText( msg.str().c_str() );
		msgBox.exec();
		return;
	}

	if (value() != widget_data.count)
	{
		setValue(widget_data.count);
	}

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
								bool not_me = true;
								std::for_each(change.set_of_identifiers.cbegin(), change.set_of_identifiers.cend(), [&](WidgetInstanceIdentifier const & the_dmu)
								{
									if (data_instance.IsEqual(WidgetInstanceIdentifier::EQUALITY_CHECK_TYPE__STRING_CODE, the_dmu))
									{
										not_me = false;
									}
								});
								if (not_me)
								{
									this->setVisible(false);
								}
								else
								{
									this->setVisible(true);
								}
							}
							break;
						case DATA_CHANGE_INTENTION__UPDATE:
						case DATA_CHANGE_INTENTION__RESET_ALL:
							{
								// This message is only received when a new VG has been added,
								// which is guaranteed to have nothing selected to start
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
