#ifndef VARIABLEGROUPDATA_H
#define VARIABLEGROUPDATA_H

#include "../Table.h"

class Table_VariableGroupData : public Table<TABLE__VG_INPUT_DATA, TABLE_INSTANCE_IDENTIFIER_CONTAINER_TYPE__NONE>
{

public:

	Table_VariableGroupData(std::string const & vg_category_string_code_, std::string const & table_name_)
		: Table<TABLE__VG_INPUT_DATA, TABLE_INSTANCE_IDENTIFIER_CONTAINER_TYPE__NONE>(Table_basemost::TABLE_MODEL_TYPE__INPUT_MODEL)
		, vg_category_string_code(vg_category_string_code_)
		, table_name(table_name_)
	{

	}

	void Load(sqlite3 * db, InputModel * input_model_);
	void Import(sqlite3 * db, OutputModel * output_model_, InputModel * input_model_);

	std::string vg_category_string_code;
	std::string table_name;

};

#endif
