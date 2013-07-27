#include "VariableGroupData.h"

void Table_VariableGroupData::Load(sqlite3 * db, InputModel * input_model_)
{
}

void Table_VariableGroupData::Import(sqlite3 * db, OutputModel * output_model_, InputModel * input_model_)
{
	
	std::lock_guard<std::recursive_mutex> data_lock(data_mutex);

	std::string sql_create;
	sql_create += "CREATE TABLE ";
	sql_create += table_name;
	sql_create += " ";

}
