#ifndef VARIABLESSELECTED_H
#define VARIABLESSELECTED_H

#include "../../../globals.h"
#include "../Table.h"

class Table_VARIABLES_SELECTED : public Table<TABLE__VG_SET_MEMBER_SELECTED, TABLE_INSTANCE_IDENTIFIER_CONTAINER_TYPE__MAP>
{

public:

	static std::string const VG_SET_MEMBER_STRING_CODE;
	static std::string const VG_CATEGORY_STRING_CODE;

	enum COLUMN_INDEX
	{
		  INDEX__VG_SET_MEMBER_STRING_CODE = 0
		, INDEX__VG_CATEGORY_STRING_CODE
	};

public:

	Table_VARIABLES_SELECTED()
		: Table<TABLE__VG_SET_MEMBER_SELECTED, TABLE_INSTANCE_IDENTIFIER_CONTAINER_TYPE__MAP>(Table_basemost::TABLE_MODEL_TYPE__OUTPUT_MODEL)
	{

	}

	void Load(sqlite3 * db, OutputModel * output_model_, InputModel * input_model_);
	bool Update(sqlite3 * db, OutputModel & output_model_, InputModel & input_model_, DataChangeMessage & change_message);

private:

	void Add(sqlite3 * db, std::string const & vg_set_member_code, std::string const & vg_category_code);
	void Remove(sqlite3 * db, std::string const & vg_set_member_code, std::string const & vg_category_code);

};

#endif
