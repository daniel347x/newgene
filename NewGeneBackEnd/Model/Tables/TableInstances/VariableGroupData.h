#ifndef VARIABLEGROUPDATA_H
#define VARIABLEGROUPDATA_H

#include "../Table.h"

class Table_VariableGroupData : public Table<TABLE__VG_INPUT_DATA, TABLE_INSTANCE_IDENTIFIER_CONTAINER_TYPE__NONE>
{

public:

	Table_VariableGroupData(std::string const & vg_category_string_code_)
		: Table<TABLE__VG_INPUT_DATA, TABLE_INSTANCE_IDENTIFIER_CONTAINER_TYPE__NONE>(Table_basemost::TABLE_MODEL_TYPE__INPUT_MODEL)
		, vg_category_string_code(vg_category_string_code_)
		, table_name(Table_VariableGroupData::TableNameFromVGCode(vg_category_string_code))
	{

	}

	void Load(sqlite3 * db, InputModel * input_model_);
	bool ImportStart(sqlite3 * db, std::string code, ImportDefinition const & import_definition, OutputModel * output_model_, InputModel * input_model_);
	bool ImportBlock(sqlite3 * db, ImportDefinition const & import_definition, OutputModel * output_model_, InputModel * input_model_, DataBlock const & block, int const number_rows_in_block);
	bool ImportEnd(sqlite3 * db, ImportDefinition const & import_definition, OutputModel * output_model_, InputModel * input_model_);

	static std::string TableNameFromVGCode(std::string variable_group_code);

	std::string vg_category_string_code;
	std::string table_name;

};

#endif
