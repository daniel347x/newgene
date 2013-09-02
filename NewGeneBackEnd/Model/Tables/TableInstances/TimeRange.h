#ifndef TIMERANGE_H
#define TIMERANGE_H

#include "../../../globals.h"
#include "../Table.h"

class Table_TIME_RANGE : public Table<TABLE__TIME_RANGE, TABLE_INSTANCE_IDENTIFIER_CONTAINER_TYPE__VECTOR_PLUS_INT64>
{

public:

	static std::string const TIME_RANGE__TIME_RANGE_START;
	static std::string const TIME_RANGE__TIME_RANGE_END;

	enum COLUMN_INDEX
	{
		   INDEX__TIME_RANGE_START = 0
		 , INDEX__TIME_RANGE_END
	};

public:

	Table_TIME_RANGE()
		: Table<TABLE__TIME_RANGE, TABLE_INSTANCE_IDENTIFIER_CONTAINER_TYPE__VECTOR_PLUS_INT64>(Table_basemost::TABLE_MODEL_TYPE__OUTPUT_MODEL)
	{

	}

	void Load(sqlite3 * db, OutputModel * output_model_, InputModel * input_model_);
	bool Update(sqlite3 * db, OutputModel & output_model_, InputModel & input_model_, DataChangeMessage & change_message);

private:

	void Modify(sqlite3 * db, std::string const & dmu_category_code, int const value_);

};

#endif
