#include "VariableGroupData.h"
#include "..\TableManager.h"

void Table_VariableGroupData::Load(sqlite3 * db, InputModel * input_model_)
{
}

void Table_VariableGroupData::Import(sqlite3 * db, std::string vg_code, ImportDefinition const & import_definition, OutputModel * output_model_, InputModel * input_model_)
{
	
	std::lock_guard<std::recursive_mutex> data_lock(data_mutex);

	if (db)
	{
		if (tableManager().TableExists(db, TableNameFromVGCode(vg_code)))
		{
			std::string sql_create;
			sql_create += "CREATE TABLE ";
			sql_create += table_name;
			sql_create += " ";
		}
	}

}

std::string TableNameFromVGcode(std::string variable_group_code)
{
	std::string variable_group_data_table_name("VG_INSTANCE_DATA_");
	variable_group_data_table_name += variable_group_code;
	return variable_group_data_table_name;
}
