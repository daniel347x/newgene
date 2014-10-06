#include "GeneralOptions.h"
#include "../../InputModel.h"
#include "../../OutputModel.h"
#include "../../../sqlite/sqlite-amalgamation-3071700/sqlite3.h"

std::string const Table_GENERAL_OPTIONS::GENERAL_OPTIONS__DO_RANDOM_SAMPLING = "DO_RANDOM_SAMPLING";
std::string const Table_GENERAL_OPTIONS::GENERAL_OPTIONS__RANDOM_SAMPLING_COUNT_PER_STAGE = "RANDOM_SAMPLING_COUNT_PER_STAGE";
std::string const Table_GENERAL_OPTIONS::GENERAL_OPTIONS__CONSOLIDATE_ROWS = "CONSOLIDATE_ROWS";
std::string const Table_GENERAL_OPTIONS::GENERAL_OPTIONS__DISPLAY_ABSOLUTE_TIME_COLUMNS = "DISPLAY_ABSOLUTE_TIME_COLUMNS";

void Table_GENERAL_OPTIONS::Load(sqlite3 * db, OutputModel * output_model_, InputModel * input_model_)
{

	std::lock_guard<std::recursive_mutex> data_lock(data_mutex);

	sqlite3_stmt * stmt = NULL;
	std::string sql("SELECT * FROM GENERAL_OPTIONS");	
	sqlite3_prepare_v2(db, sql.c_str(), sql.size() + 1, &stmt, NULL);
	if (stmt == NULL)
	{
		return;
	}
	int step_result = 0;
	if ((step_result = sqlite3_step(stmt)) == SQLITE_ROW)
	{

		// ****************************************************************************************//
		// Use codes as foreign keys, not UUID's, so that this output model can be shared with others
		// ****************************************************************************************//
		do_random_sampling = (sqlite3_column_int(stmt, INDEX__GENERAL_OPTIONS__DO_RANDOM_SAMPLING) != 0);
		random_sampling_count_per_stage = sqlite3_column_int(stmt, INDEX__GENERAL_OPTIONS__RANDOM_SAMPLING_COUNT_PER_STAGE);
		consolidate_rows = (sqlite3_column_int(stmt, INDEX__GENERAL_OPTIONS__CONSOLIDATE_ROWS) != 0);
		display_absolute_time_columns = (sqlite3_column_int(stmt, INDEX__GENERAL_OPTIONS__DISPLAY_ABSOLUTE_TIME_COLUMNS) != 0);

	}
	if (stmt)
	{
		sqlite3_finalize(stmt);
		stmt = nullptr;
	}

}

bool Table_GENERAL_OPTIONS::UpdateDoKadSampler(sqlite3 * db, OutputModel & output_model_, InputModel & input_model_, DataChangeMessage & change_message)
{

	std::lock_guard<std::recursive_mutex> data_lock(data_mutex);

	//Executor theExecutor(db);

	std::for_each(change_message.changes.cbegin(), change_message.changes.cend(), [&db, &input_model_, this](DataChange const & change)
	{
		switch (change.change_type)
		{
		case DATA_CHANGE_TYPE::DATA_CHANGE_TYPE__OUTPUT_MODEL__DO_RANDOM_SAMPLING_CHANGE:
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
						DataChangePacket_bool * packet = static_cast<DataChangePacket_bool *>(change.getPacket());
						if (packet)
						{
							do_random_sampling = packet->getValue();
							ModifyDoKadSampler(db);
						}
						else
						{
							// error condition ... todo
						}
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

	//theExecutor.success();

	//return theExecutor.succeeded();
	return true;

}

bool Table_GENERAL_OPTIONS::UpdateKadSamplerCountPerStage(sqlite3 * db, OutputModel & output_model_, InputModel & input_model_, DataChangeMessage & change_message)
{

	std::lock_guard<std::recursive_mutex> data_lock(data_mutex);

	//Executor theExecutor(db);

	std::for_each(change_message.changes.cbegin(), change_message.changes.cend(), [&db, &input_model_, this](DataChange const & change)
	{
		switch (change.change_type)
		{
		case DATA_CHANGE_TYPE::DATA_CHANGE_TYPE__OUTPUT_MODEL__RANDOM_SAMPLING_COUNT_PER_STAGE_CHANGE:
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
						DataChangePacket_int64 * packet = static_cast<DataChangePacket_int64 *>(change.getPacket());
						if (packet)
						{
							random_sampling_count_per_stage = packet->getValue();
							ModifyKadSamplerCountPerStage(db);
						}
						else
						{
							// error condition ... todo
						}
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

	//theExecutor.success();

	//return theExecutor.succeeded();
	return true;

}

bool Table_GENERAL_OPTIONS::UpdateConsolidateRows(sqlite3 * db, OutputModel & output_model_, InputModel & input_model_, DataChangeMessage & change_message)
{

	std::lock_guard<std::recursive_mutex> data_lock(data_mutex);

	//Executor theExecutor(db);

	std::for_each(change_message.changes.cbegin(), change_message.changes.cend(), [&db, &input_model_, this](DataChange const & change)
	{
		switch (change.change_type)
		{
		case DATA_CHANGE_TYPE::DATA_CHANGE_TYPE__OUTPUT_MODEL__CONSOLIDATE_ROWS_CHANGE:
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
						DataChangePacket_bool * packet = static_cast<DataChangePacket_bool *>(change.getPacket());
						if (packet)
						{
							consolidate_rows = packet->getValue();
							ModifyConsolidateRows(db);
						}
						else
						{
							// error condition ... todo
						}
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

	//theExecutor.success();

	//return theExecutor.succeeded();
	return true;

}

bool Table_GENERAL_OPTIONS::UpdateDisplayAbsoluteTimeColumns(sqlite3 * db, OutputModel & output_model_, InputModel & input_model_, DataChangeMessage & change_message)
{

	std::lock_guard<std::recursive_mutex> data_lock(data_mutex);

	//Executor theExecutor(db);

	std::for_each(change_message.changes.cbegin(), change_message.changes.cend(), [&db, &input_model_, this](DataChange const & change)
	{
		switch (change.change_type)
		{
		case DATA_CHANGE_TYPE::DATA_CHANGE_TYPE__OUTPUT_MODEL__DISPLAY_ABSOLUTE_TIME_COLUMNS:
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
						DataChangePacket_bool * packet = static_cast<DataChangePacket_bool *>(change.getPacket());
						if (packet)
						{
							display_absolute_time_columns = packet->getValue();
							ModifyDisplayAbsoluteTimeColumns(db);
						}
						else
						{
							// error condition ... todo
						}
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

	//theExecutor.success();

	//return theExecutor.succeeded();
	return true;

}

void Table_GENERAL_OPTIONS::ModifyDoKadSampler(sqlite3 * db)
{

	std::lock_guard<std::recursive_mutex> data_lock(data_mutex);

	std::string sqlAdd("UPDATE GENERAL_OPTIONS SET ");
	sqlAdd += GENERAL_OPTIONS__DO_RANDOM_SAMPLING;
	sqlAdd += "=";
	sqlAdd += boost::lexical_cast<std::string>(do_random_sampling ? 1 : 0);
	sqlite3_stmt * stmt = NULL;
	sqlite3_prepare_v2(db, sqlAdd.c_str(), sqlAdd.size() + 1, &stmt, NULL);
	if (stmt == NULL)
	{
		return;
	}
	sqlite3_step(stmt);
	if (stmt)
	{
		sqlite3_finalize(stmt);
		stmt = nullptr;
	}

}

void Table_GENERAL_OPTIONS::ModifyKadSamplerCountPerStage(sqlite3 * db)
{

	std::lock_guard<std::recursive_mutex> data_lock(data_mutex);

	std::string sqlAdd("UPDATE GENERAL_OPTIONS SET ");
	sqlAdd += GENERAL_OPTIONS__RANDOM_SAMPLING_COUNT_PER_STAGE;
	sqlAdd += "=";
	sqlAdd += boost::lexical_cast<std::string>(random_sampling_count_per_stage);
	sqlite3_stmt * stmt = NULL;
	sqlite3_prepare_v2(db, sqlAdd.c_str(), sqlAdd.size() + 1, &stmt, NULL);
	if (stmt == NULL)
	{
		return;
	}
	sqlite3_step(stmt);
	if (stmt)
	{
		sqlite3_finalize(stmt);
		stmt = nullptr;
	}

}

void Table_GENERAL_OPTIONS::ModifyConsolidateRows(sqlite3 * db)
{

	std::lock_guard<std::recursive_mutex> data_lock(data_mutex);

	std::string sqlAdd("UPDATE GENERAL_OPTIONS SET ");
	sqlAdd += GENERAL_OPTIONS__CONSOLIDATE_ROWS;
	sqlAdd += "=";
	sqlAdd += boost::lexical_cast<std::string>(consolidate_rows ? 1 : 0);
	sqlite3_stmt * stmt = NULL;
	sqlite3_prepare_v2(db, sqlAdd.c_str(), sqlAdd.size() + 1, &stmt, NULL);
	if (stmt == NULL)
	{
		return;
	}
	sqlite3_step(stmt);
	if (stmt)
	{
		sqlite3_finalize(stmt);
		stmt = nullptr;
	}

}

void Table_GENERAL_OPTIONS::ModifyDisplayAbsoluteTimeColumns(sqlite3 * db)
{

	std::lock_guard<std::recursive_mutex> data_lock(data_mutex);

	std::string sqlAdd("UPDATE GENERAL_OPTIONS SET ");
	sqlAdd += GENERAL_OPTIONS__DISPLAY_ABSOLUTE_TIME_COLUMNS;
	sqlAdd += "=";
	sqlAdd += boost::lexical_cast<std::string>(display_absolute_time_columns ? 1 : 0);
	sqlite3_stmt * stmt = NULL;
	sqlite3_prepare_v2(db, sqlAdd.c_str(), sqlAdd.size() + 1, &stmt, NULL);
	if (stmt == NULL)
	{
		return;
	}
	sqlite3_step(stmt);
	if (stmt)
	{
		sqlite3_finalize(stmt);
		stmt = nullptr;
	}

}

std::tuple<bool, std::int64_t, bool, bool> Table_GENERAL_OPTIONS::getKadSamplerInfo(sqlite3 * db)
{

	std::lock_guard<std::recursive_mutex> data_lock(data_mutex);

	sqlite3_stmt * stmt = NULL;
	std::string sql("SELECT * FROM GENERAL_OPTIONS");	
	sqlite3_prepare_v2(db, sql.c_str(), sql.size() + 1, &stmt, NULL);
	if (stmt == NULL)
	{
		std::tuple<bool, std::int64_t, bool, bool> ret(false, 1, true, false);
		return ret;
	}
	int step_result = 0;
	if ((step_result = sqlite3_step(stmt)) == SQLITE_ROW)
	{

		// ****************************************************************************************//
		// Use codes as foreign keys, not UUID's, so that this output model can be shared with others
		// ****************************************************************************************//
		do_random_sampling = (sqlite3_column_int(stmt, INDEX__GENERAL_OPTIONS__DO_RANDOM_SAMPLING) != 0);
		random_sampling_count_per_stage = sqlite3_column_int(stmt, INDEX__GENERAL_OPTIONS__RANDOM_SAMPLING_COUNT_PER_STAGE);
		consolidate_rows = (sqlite3_column_int(stmt, INDEX__GENERAL_OPTIONS__CONSOLIDATE_ROWS) != 0);
		display_absolute_time_columns = (sqlite3_column_int(stmt, INDEX__GENERAL_OPTIONS__DISPLAY_ABSOLUTE_TIME_COLUMNS) != 0);

	}
	if (stmt)
	{
		sqlite3_finalize(stmt);
		stmt = nullptr;
	}

	// workaround compiler bug that will not allow temporary tuple with bool param
	std::tuple<bool, std::int64_t, bool, bool> ret(do_random_sampling, random_sampling_count_per_stage, consolidate_rows, display_absolute_time_columns);
	return ret;

}

