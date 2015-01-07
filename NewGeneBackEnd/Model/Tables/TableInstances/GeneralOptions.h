#ifndef GENERALOPTIONS_H
#define GENERALOPTIONS_H

#include "../../../globals.h"
#include "../Table.h"

class Table_GENERAL_OPTIONS : public Table<TABLE__GENERAL_OPTIONS, TABLE_INSTANCE_IDENTIFIER_CONTAINER_TYPE__NONE>
{

public:

	static std::string const GENERAL_OPTIONS__DO_RANDOM_SAMPLING;
	static std::string const GENERAL_OPTIONS__RANDOM_SAMPLING_COUNT_PER_STAGE;
	static std::string const GENERAL_OPTIONS__CONSOLIDATE_ROWS;
	static std::string const GENERAL_OPTIONS__DISPLAY_ABSOLUTE_TIME_COLUMNS;

	enum COLUMN_INDEX
	{
		  INDEX__GENERAL_OPTIONS__DO_RANDOM_SAMPLING = 0
		, INDEX__GENERAL_OPTIONS__RANDOM_SAMPLING_COUNT_PER_STAGE
		, INDEX__GENERAL_OPTIONS__CONSOLIDATE_ROWS
		, INDEX__GENERAL_OPTIONS__DISPLAY_ABSOLUTE_TIME_COLUMNS
	};

public:

	Table_GENERAL_OPTIONS()
		: Table<TABLE__GENERAL_OPTIONS, TABLE_INSTANCE_IDENTIFIER_CONTAINER_TYPE__NONE>(Table_basemost::TABLE_MODEL_TYPE__OUTPUT_MODEL)
	{

	}

	void Load(sqlite3 * db, OutputModel * output_model_, InputModel * input_model_);
	bool UpdateDoRandomSampling(sqlite3 * db, OutputModel & output_model_, InputModel & input_model_, DataChangeMessage & change_message);
	bool UpdateKadSamplerCountPerStage(sqlite3 * db, OutputModel & output_model_, InputModel & input_model_, DataChangeMessage & change_message);
	bool UpdateConsolidateRows(sqlite3 * db, OutputModel & output_model_, InputModel & input_model_, DataChangeMessage & change_message);
	bool UpdateDisplayAbsoluteTimeColumns(sqlite3 * db, OutputModel & output_model_, InputModel & input_model_, DataChangeMessage & change_message);
	std::tuple<bool, std::int64_t, bool, bool> getKadSamplerInfo(sqlite3 * db);

private:

	void ModifyDoRandomSampling(sqlite3 * db);
	void ModifyKadSamplerCountPerStage(sqlite3 * db);
	void ModifyConsolidateRows(sqlite3 * db);
	void ModifyDisplayAbsoluteTimeColumns(sqlite3 * db);

	bool do_random_sampling;
	std::int64_t random_sampling_count_per_stage;
	bool consolidate_rows;
	bool display_absolute_time_columns;

};

#endif
