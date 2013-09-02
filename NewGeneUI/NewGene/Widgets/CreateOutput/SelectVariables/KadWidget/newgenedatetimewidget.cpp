#include "newgenedatetimewidget.h"

#ifndef Q_MOC_RUN
#	include <boost/algorithm/string.hpp>
#	include <boost/date_time/local_time/local_time.hpp>
#endif

NewGeneDateTimeWidget::NewGeneDateTimeWidget( QWidget * parent, WidgetInstanceIdentifier data_instance_, UIOutputProject * project ) :

	QDateTimeEdit( parent ),

	NewGeneWidget( WidgetCreationInfo(
										this, // 'this' pointer is cast by compiler to proper Widget instance, which is already created due to order in which base classes appear in class definition
										parent,
										WIDGET_NATURE_OUTPUT_WIDGET,
										DATETIME_WIDGET,
										true,
										data_instance_
									 )
				   )

{

   PrepareOutputWidget();

   connect(this, SIGNAL(dateTimeChanged(QDateTime const &)), this, SLOT(ReceiveVariableItemChanged(QDateTime const &)));

   if (data_instance.uuid && project)
   {

	   project->RegisterInterestInChange(this, DATA_CHANGE_TYPE__OUTPUT_MODEL__DATETIME_RANGE_CHANGE, true, *data_instance.uuid);
	   project->RegisterInterestInChange(this, DATA_CHANGE_TYPE__OUTPUT_MODEL__DATETIME_RANGE_CHANGE, true, *data_instance.uuid);

	   UpdateOutputConnections(UIProjectManager::ESTABLISH_CONNECTIONS_OUTPUT_PROJECT, project);
	   WidgetDataItemRequest_DATETIME_WIDGET request(0, WIDGET_DATA_ITEM_REQUEST_REASON__REFRESH_ALL_WIDGETS, data_instance);
	   emit RefreshWidget(request);

   }

}

~NewGeneDateTimeWidget()
{

}

void NewGeneDateTimeWidget::RefreshAllWidgets()
{
	WidgetDataItemRequest_DATETIME_WIDGET request(value(), WIDGET_DATA_ITEM_REQUEST_REASON__REFRESH_ALL_WIDGETS);
	emit RefreshWidget(request);
}

void NewGeneDateTimeWidget::UpdateOutputConnections(UIProjectManager::UPDATE_CONNECTIONS_TYPE connection_type, UIOutputProject * project)
{
	if (connection_type == UIProjectManager::RELEASE_CONNECTIONS_OUTPUT_PROJECT)
	{
		outp->UnregisterInterestInChanges(this);
	}

	NewGeneWidget::UpdateOutputConnections(connection_type, project);

	if (connection_type == UIProjectManager::ESTABLISH_CONNECTIONS_OUTPUT_PROJECT)
	{
		connect(this, SIGNAL(RefreshWidget(WidgetDataItemRequest_DATETIME_WIDGET)), outp->getConnector(), SLOT(RefreshWidget(WidgetDataItemRequest_DATETIME_WIDGET)));
		connect(this, SIGNAL(SignalReceiveVariableItemChanged(WidgetActionItemRequest_ACTION_DATETIME_RANGE_CHANGE)), outp->getConnector(), SLOT(ReceiveVariableItemChanged(WidgetActionItemRequest_ACTION_DATETIME_RANGE_CHANGE)));
	}
}

void NewGeneDateTimeWidget::WidgetDataRefreshReceive(WidgetDataItem_DATETIME_WIDGET widget_data)
{

	if (!data_instance.uuid || !widget_data.identifier || !widget_data.identifier->uuid || (*data_instance.uuid) != (*widget_data.identifier->uuid) )
	{
		boost::format msg("Invalid widget refresh in NewGeneDateTimeWidget widget.");
		QMessageBox msgBox;
		msgBox.setText( msg.str().c_str() );
		msgBox.exec();
		return;
	}

	QDate date_1970(1970, 1, 1);
	QTime time_12am(0, 0);
	QDateTime datetime_1970(date_1970, time_12am);

	QDateTime proposed_date_time = datetime_1970;
	proposed_date_time.addMSecs(widget_data.the_date_time);

	if (dateTime() != proposed_date_time)
	{
		setDateTime(proposed_date_time);
	}

}

void NewGeneDateTimeWidget::ReceiveVariableItemChanged(QDateTime const & newValue)
{

	QDate date_1970(1970, 1, 1);
	QTime time_12am(0, 0);
	QDateTime datetime_1970(date_1970, time_12am);

	std::int64_t different_from_1970_in_ms = static_cast<std::int64_t>(datetime_1970.msecsTo(newValue));

	InstanceActionItems actionItems;
	actionItems.push_back(std::make_pair(data_instance, std::shared_ptr<WidgetActionItem>(static_cast<WidgetActionItem*>(new WidgetActionItem__DateTime(different_from_1970_in_ms)))));
	WidgetActionItemRequest_ACTION_KAD_COUNT_CHANGE action_request(WIDGET_ACTION_ITEM_REQUEST_REASON__UPDATE_ITEMS, actionItems);
	emit SignalReceiveVariableItemChanged(action_request);

}

void NewGeneDateTimeWidget::HandleChanges(DataChangeMessage const & change_message)
{
	std::for_each(change_message.changes.cbegin(), change_message.changes.cend(), [this](DataChange const & change)
	{
		switch (change.change_type)
		{
			case DATA_CHANGE_TYPE::DATA_CHANGE_TYPE__OUTPUT_MODEL__DATETIME_RANGE_CHANGE:
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

								if (change.child_identifiers.size() == 0)
								{
									return; // from lambda
								}

								std::for_each(change.child_identifiers.cbegin(), change.child_identifiers.cend(), [&change, this](WidgetInstanceIdentifier const & child_identifier)
								{

									if (data_instance.uuid && child_identifier.uuid && *data_instance.uuid == *child_identifier.uuid)
									{

										if (change.change_intention == DATA_CHANGE_INTENTION__UPDATE)
										{

											DataChangePacket_int64 * packet = static_cast<DataChangePacket_int64 *>(change.getPacket());
											if (packet)
											{

												QDateTime const & currentValue = dateTime();

												QDate date_1970(1970, 1, 1);
												QTime time_12am(0, 0);
												QDateTime datetime_1970(date_1970, time_12am);

												std::int64_t different_from_1970_in_ms = static_cast<std::int64_t>(datetime_1970.msecsTo(currentValue));

												if (different_from_1970_in_ms != packet->getValue())
												{

													QDateTime datetime_newvalue = datetime_1970;
													datetime_newvalue.addMSecs(packet->getValue());

													setDateTime(datetime_newValue);
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
