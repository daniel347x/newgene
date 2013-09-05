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

}

NewGeneDateTimeWidget::~NewGeneDateTimeWidget()
{

}

void NewGeneDateTimeWidget::RefreshAllWidgets()
{
	if (outp == nullptr)
	{
		Empty();
		return;
	}

	QDateTime const & currentValue = dateTime();

	QDate date_1970(1970, 1, 1);
	QTime time_12am(0, 0);
	QDateTime datetime_1970(date_1970, time_12am);

	std::int64_t different_from_1970_in_ms = static_cast<std::int64_t>(datetime_1970.msecsTo(currentValue));

	WidgetDataItemRequest_DATETIME_WIDGET request(different_from_1970_in_ms, WIDGET_DATA_ITEM_REQUEST_REASON__REFRESH_ALL_WIDGETS);
	request.identifier = std::make_shared<WidgetInstanceIdentifier>(this->data_instance);
	emit RefreshWidget(request);
}

void NewGeneDateTimeWidget::UpdateOutputConnections(UIProjectManager::UPDATE_CONNECTIONS_TYPE connection_type, UIOutputProject * project)
{
	if (connection_type == UIProjectManager::RELEASE_CONNECTIONS_OUTPUT_PROJECT)
	{
		outp->UnregisterInterestInChanges(this);
	}

	if (connection_type == UIProjectManager::ESTABLISH_CONNECTIONS_OUTPUT_PROJECT)
	{
		/* For some reason, setting the "objectName" property in the form editor is not reflected here */
		/*
		if (this->objectName() == "dateTimeEdit_start")
		{
			this->data_instance.code = std::make_shared<std::string>("0");
			this->data_instance.flags = "s";
		}
		else if (this->objectName() == "dateTimeEdit_end")
		{
			this->data_instance.code = std::make_shared<std::string>("0");
			this->data_instance.flags = "e";
		}
		*/

		if (project->number_timerange_widgets_created == 0)
		{
			this->data_instance.code = std::make_shared<std::string>("0");
			this->data_instance.uuid = this->data_instance.code;
			this->data_instance.flags = "s";
			++project->number_timerange_widgets_created;
		}
		else if (project->number_timerange_widgets_created == 1)
		{
			this->data_instance.code = std::make_shared<std::string>("0");
			this->data_instance.uuid = this->data_instance.code;
			this->data_instance.flags = "e";
			++project->number_timerange_widgets_created;
		}

	}

	NewGeneWidget::UpdateOutputConnections(connection_type, project);

	if (connection_type == UIProjectManager::ESTABLISH_CONNECTIONS_OUTPUT_PROJECT)
	{

		if (data_instance.uuid && project)
		{

			project->RegisterInterestInChange(this, DATA_CHANGE_TYPE__OUTPUT_MODEL__DATETIME_RANGE_CHANGE, true, *data_instance.uuid);
			project->RegisterInterestInChange(this, DATA_CHANGE_TYPE__OUTPUT_MODEL__DATETIME_RANGE_CHANGE, true, *data_instance.uuid);

		}

		connect(this, SIGNAL(RefreshWidget(WidgetDataItemRequest_DATETIME_WIDGET)), outp->getConnector(), SLOT(RefreshWidget(WidgetDataItemRequest_DATETIME_WIDGET)));
		connect(this, SIGNAL(SignalReceiveVariableItemChanged(WidgetActionItemRequest_ACTION_DATETIME_RANGE_CHANGE)), outp->getConnector(), SLOT(ReceiveVariableItemChanged(WidgetActionItemRequest_ACTION_DATETIME_RANGE_CHANGE)));
		connect(project->getConnector(), SIGNAL(WidgetDataRefresh(WidgetDataItem_DATETIME_WIDGET)), this, SLOT(WidgetDataRefreshReceive(WidgetDataItem_DATETIME_WIDGET)));

	}
	else if (connection_type == UIProjectManager::RELEASE_CONNECTIONS_OUTPUT_PROJECT)
	{
		Empty();
	}
}

void NewGeneDateTimeWidget::WidgetDataRefreshReceive(WidgetDataItem_DATETIME_WIDGET widget_data)
{

	if (!data_instance.code || !widget_data.identifier || !widget_data.identifier->code || *widget_data.identifier->code != "0" || (widget_data.identifier->flags != "s" && widget_data.identifier->flags != "e"))
	{
		boost::format msg("Invalid widget refresh in NewGeneDateTimeWidget widget.");
		QMessageBox msgBox;
		msgBox.setText( msg.str().c_str() );
		msgBox.exec();
		return;
	}

	if (widget_data.identifier->flags != data_instance.flags)
	{
		return;
	}

	QDate date_1970(1970, 1, 1);
	QTime time_12am(0, 0);
	QDateTime datetime_1970(date_1970, time_12am);

	QDateTime proposed_date_time = datetime_1970.addMSecs(widget_data.the_date_time);

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
	WidgetActionItemRequest_ACTION_DATETIME_RANGE_CHANGE action_request(WIDGET_ACTION_ITEM_REQUEST_REASON__UPDATE_ITEMS, actionItems);
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

									if (data_instance.code && child_identifier.code && *data_instance.code == *child_identifier.code && data_instance.flags == child_identifier.flags)
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

													setDateTime(datetime_newvalue);
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
