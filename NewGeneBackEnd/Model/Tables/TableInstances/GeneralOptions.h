#ifndef GENERALOPTIONS_H
#define GENERALOPTIONS_H

#include "../../../globals.h"
#include "../Table.h"

class Table_GENERAL_OPTIONS : public Table<TABLE__GENERAL_OPTIONS, TABLE_INSTANCE_IDENTIFIER_CONTAINER_TYPE__NONE>
{

public:

	static std::string const GENERAL_OPTIONS__DO_RANDOM_SAMPLING;
	static std::string const GENERAL_OPTIONS__RANDOM_SAMPLING_COUNT_PER_STAGE;

	enum COLUMN_INDEX
	{
		  INDEX__GENERAL_OPTIONS__DO_RANDOM_SAMPLING = 0
		, INDEX__GENERAL_OPTIONS__RANDOM_SAMPLING_COUNT_PER_STAGE
	};

public:

	Table_GENERAL_OPTIONS()
		: Table<TABLE__GENERAL_OPTIONS, TABLE_INSTANCE_IDENTIFIER_CONTAINER_TYPE__NONE>(Table_basemost::TABLE_MODEL_TYPE__OUTPUT_MODEL)
	{

	}

	void Load(sqlite3 * db, OutputModel * output_model_, InputModel * input_model_);
	bool UpdateDoRandomSampling(sqlite3 * db, OutputModel & output_model_, InputModel & input_model_, DataChangeMessage & change_message);
	bool UpdateRandomSamplingCountPerStage(sqlite3 * db, OutputModel & output_model_, InputModel & input_model_, DataChangeMessage & change_message);
	std::pair<bool, std::int64_t> getRandomSamplingInfo(sqlite3 * db);

private:

	void ModifyDoRandomSampling(sqlite3 * db);
	void ModifyRandomSamplingCountPerStage(sqlite3 * db);

	bool do_random_sampling;
	std::int64_t random_sampling_count_per_stage;

};

#endif
