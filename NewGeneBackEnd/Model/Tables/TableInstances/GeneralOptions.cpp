#include "GeneralOptions.h"
#include "../../InputModel.h"
#include "../../OutputModel.h"
#include "../../../sqlite/sqlite-amalgamation-3071700/sqlite3.h"

std::string const Table_GENERAL_OPTIONS::GENERAL_OPTIONS__DO_RANDOM_SAMPLING = "DO_RANDOM_SAMPLING";
std::string const Table_GENERAL_OPTIONS::GENERAL_OPTIONS__RANDOM_SAMPLING_COUNT_PER_STAGE = "RANDOM_SAMPLING_COUNT_PER_STAGE";

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

	}
	if (stmt)
	{
		sqlite3_finalize(stmt);
		stmt = nullptr;
	}

}

bool Table_GENERAL_OPTIONS::UpdateDoRandomSampling(sqlite3 * db, OutputModel & output_model_, InputModel & input_model_, DataChangeMessage & change_message)
{

	std::lock_guard<std::recursive_mutex> data_lock(data_mutex);

	Executor theExecutor(db);

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
							ModifyDoRandomSampling(db);
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

	theExecutor.success();

	return theExecutor.succeeded();

}

bool Table_GENERAL_OPTIONS::UpdateRandomSamplingCountPerStage(sqlite3 * db, OutputModel & output_model_, InputModel & input_model_, DataChangeMessage & change_message)
{

	std::lock_guard<std::recursive_mutex> data_lock(data_mutex);

	Executor theExecutor(db);

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
							ModifyRandomSamplingCountPerStage(db);
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

	theExecutor.success();

	return theExecutor.succeeded();

}

void Table_GENERAL_OPTIONS::ModifyDoRandomSampling(sqlite3 * db)
{
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

void Table_GENERAL_OPTIONS::ModifyRandomSamplingCountPerStage(sqlite3 * db)
{
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

std::pair<bool, std::int64_t> Table_GENERAL_OPTIONS::getRandomSamplingInfo(sqlite3 * db)
{

	std::lock_guard<std::recursive_mutex> data_lock(data_mutex);

	sqlite3_stmt * stmt = NULL;
	std::string sql("SELECT * FROM GENERAL_OPTIONS");	
	sqlite3_prepare_v2(db, sql.c_str(), sql.size() + 1, &stmt, NULL);
	if (stmt == NULL)
	{
		return std::make_pair<bool, std::int64_t>(false, 1);
	}
	int step_result = 0;
	if ((step_result = sqlite3_step(stmt)) == SQLITE_ROW)
	{

		// ****************************************************************************************//
		// Use codes as foreign keys, not UUID's, so that this output model can be shared with others
		// ****************************************************************************************//
		do_random_sampling = (sqlite3_column_int(stmt, INDEX__GENERAL_OPTIONS__DO_RANDOM_SAMPLING) != 0);
		random_sampling_count_per_stage = sqlite3_column_int(stmt, INDEX__GENERAL_OPTIONS__RANDOM_SAMPLING_COUNT_PER_STAGE);

	}
	if (stmt)
	{
		sqlite3_finalize(stmt);
		stmt = nullptr;
	}

	return std::make_pair(do_random_sampling, random_sampling_count_per_stage);

}

