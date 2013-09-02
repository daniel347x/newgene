#include "TimeRange.h"
#include "../../InputModel.h"
#include "../../OutputModel.h"
#include "../../../sqlite/sqlite-amalgamation-3071700/sqlite3.h"

std::string const Table_TIME_RANGE::TIME_RANGE__TIME_RANGE_START = "TIMERANGE_START";
std::string const Table_TIME_RANGE::TIME_RANGE__TIME_RANGE_END = "TIMERANGE_END";

void Table_TIME_RANGE::Load(sqlite3 * db, OutputModel * output_model_, InputModel * input_model_)
{

	if (!input_model_)
	{
		return;
	}

	std::lock_guard<std::recursive_mutex> data_lock(data_mutex);

	identifiers.clear();

	sqlite3_stmt * stmt = NULL;
	std::string sql("SELECT * FROM TIMERANGE_SELECTED");	
	sqlite3_prepare_v2(db, sql.c_str(), sql.size() + 1, &stmt, NULL);
	if (stmt == NULL)
	{
		return;
	}
	int step_result = 0;
	while ((step_result = sqlite3_step(stmt)) == SQLITE_ROW)
	{
		std::int64_t const timerange_start = sqlite3_column_int(stmt, INDEX__TIME_RANGE_START);
		std::int64_t const timerange_end = sqlite3_column_int(stmt, INDEX__TIME_RANGE_END);

		// The CODE is 0, and there is only a single row
		WidgetInstanceIdentifier identifier("0");

		identifiers.push_back(std::make_pair(identifier, timerange_start));
		identifiers.push_back(std::make_pair(identifier, timerange_end));
	}
}

bool Table_TIME_RANGE::Update(sqlite3 * db, OutputModel & output_model_, InputModel & input_model_, DataChangeMessage & change_message)
{
	std::lock_guard<std::recursive_mutex> data_lock(data_mutex);

	Executor theExecutor(db);

	std::for_each(change_message.changes.cbegin(), change_message.changes.cend(), [&db, &input_model_, this](DataChange const & change)
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
						// This is the OUTPUT model changing.
						// "Add" means to simply add an item that is CHECKED (previously unchecked) -
						// NOT to add a new variable.  That would be input model change type.

						if (change.child_identifiers.size() == 0)
						{
							return; // from lambda
						}

						std::for_each(change.child_identifiers.cbegin(), change.child_identifiers.cend(), [&db, &input_model_, &change, this](WidgetInstanceIdentifier const & child_identifier)
						{
							if (child_identifier.code && child_identifier.code->size() != 0)
							{
								if (change.change_intention == DATA_CHANGE_INTENTION__UPDATE)
								{
									DataChangePacket_int64 * packet = static_cast<DataChangePacket_int64 *>(change.getPacket());
									if (packet)
									{
										std::for_each(this->identifiers.begin(), this->identifiers.end(), [&packet, &child_identifier](WidgetInstanceIdentifier_Int64_Pair & cache_identifier)
										{
											if (boost::iequals(*child_identifier.uuid, *cache_identifier.first.uuid))
											{
												if (packet->getValue() != cache_identifier.second)
												{
													cache_identifier.second = packet->getValue();
												}
												return; // from lambda
											}
										});
										Modify(db, *child_identifier.code, packet->getValue());
									}
									else
									{
										// error condition ... todo
									}
								}
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

	theExecutor.success();

	return theExecutor.succeeded();

}

void Table_KAD_COUNT::Modify(sqlite3 * db, std::string const & dmu_category_code, int const value_)
{
	char c_[64];
	std::string sqlAdd("UPDATE KAD_COUNT SET ");
	sqlAdd += KAD_COUNT__COUNT;
	sqlAdd += "=";
	sqlAdd += itoa(value_, c_, 10);
	sqlAdd += " WHERE ";
	sqlAdd += KAD_COUNT__DMU_CATEGORY_STRING_CODE;
	sqlAdd += "='";
	sqlAdd += dmu_category_code;
	sqlAdd += "'";
	sqlite3_stmt * stmt = NULL;
	sqlite3_prepare_v2(db, sqlAdd.c_str(), sqlAdd.size() + 1, &stmt, NULL);
	if (stmt == NULL)
	{
		return;
	}
	sqlite3_step(stmt);
}
