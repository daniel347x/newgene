#ifndef KADCOUNT_H
#define KADCOUNT_H

#include "../../../globals.h"
#include "../Table.h"

class Table_KAD_COUNT : public Table<TABLE__KAD_COUNT, TABLE_INSTANCE_IDENTIFIER_CONTAINER_TYPE__VECTOR_PLUS_INT>
{

public:

	static std::string const KAD_COUNT__DMU_CATEGORY_STRING_CODE;
	static std::string const KAD_COUNT__COUNT;
	static std::string const KAD_COUNT__FLAGS;

	enum COLUMN_INDEX
	{
		  INDEX__KAD_COUNT__DMU_CATEGORY_STRING_CODE = 0
		, INDEX__KAD_COUNT__COUNT
		, INDEX__KAD_COUNT__FLAGS
	};

public:

	Table_KAD_COUNT()
		: Table<TABLE__KAD_COUNT, TABLE_INSTANCE_IDENTIFIER_CONTAINER_TYPE__VECTOR_PLUS_INT>(Table_basemost::TABLE_MODEL_TYPE__OUTPUT_MODEL)
	{

	}

	void Load(sqlite3 * db, OutputModel * output_model_, InputModel * input_model_);
	bool Update(sqlite3 * db, OutputModel & output_model_, InputModel & input_model_, DataChangeMessage & change_message);

	void Add(sqlite3 * db, std::string const & dmu_category_code, int const value_);
	void Remove(sqlite3 * db, std::string const & dmu_category_code);
	void Modify(sqlite3 * db, std::string const & dmu_category_code, int const value_);

};

#endif
