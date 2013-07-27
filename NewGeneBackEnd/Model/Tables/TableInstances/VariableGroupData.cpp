#include "VariableGroupData.h"
#include "..\TableManager.h"

void Table_VariableGroupData::Load(sqlite3 * db, InputModel * input_model_)
{
}

bool Table_VariableGroupData::ImportStart(sqlite3 * db, std::string vg_code, ImportDefinition const & import_definition, OutputModel * output_model_, InputModel * input_model_)
{
	
	std::lock_guard<std::recursive_mutex> data_lock(data_mutex);

	if (db)
	{
		if (!tableManager().TableExists(db, TableNameFromVGCode(vg_code)))
		{
			std::string sql_create;
			sql_create += "CREATE TABLE ";
			sql_create += table_name;
			sql_create += " (";

			bool first = true;
			std::for_each(import_definition.output_schema.schema.cbegin(), import_definition.output_schema.schema.cend(), [&import_definition, &sql_create, &first](SchemaEntry const & table_schema_entry)
			{
				if (!first)
				{
					sql_create += ",";
				}
				first = false;
				sql_create += table_schema_entry.field_name;
				sql_create += " ";
				switch (table_schema_entry.field_type)
				{
				case FIELD_TYPE_INT32:
				case FIELD_TYPE_INT64:
				case FIELD_TYPE_UINT32:
				case FIELD_TYPE_UINT64:
					{
						sql_create += "INTEGER";
					}
					break;
				case FIELD_TYPE_STRING_FIXED:
				case FIELD_TYPE_STRING_VAR:
				case FIELD_TYPE_UUID:
				case FIELD_TYPE_UUID_FOREIGN:
				case FIELD_TYPE_STRING_CODE:
				case FIELD_TYPE_STRING_LONGHAND:
				case FIELD_TYPE_TIME_RANGE:
				case FIELD_TYPE_NOTES_1:
				case FIELD_TYPE_NOTES_2:
				case FIELD_TYPE_NOTES_3:
					{
						sql_create += "TEXT";
					}
					break;
				case FIELD_TYPE_TIMESTAMP:
					{
						sql_create += "INTEGER";
					}
					break;
				}
			});

			sql_create += ")";

			sqlite3_stmt * stmt = NULL;
			sqlite3_prepare_v2(db, sql_create.c_str(), sql_create.size() + 1, &stmt, NULL);
			if (stmt == NULL)
			{
				return false;
			}
			int step_result = 0;
			if ((step_result = sqlite3_step(stmt)) == SQLITE_ROW)
			{
				return true;
			}
		}
	}

	return false;

}

bool Table_VariableGroupData::ImportBlock(sqlite3 * db, std::string vg_code, ImportDefinition const & import_definition, OutputModel * output_model_, InputModel * input_model_, Importer::DataBlock const & block, int const number_rows_in_block)
{

	std::lock_guard<std::recursive_mutex> data_lock(data_mutex);

	return true;

}

bool Table_VariableGroupData::ImportEnd(sqlite3 * db, std::string vg_code, ImportDefinition const & import_definition, OutputModel * output_model_, InputModel * input_model_)
{

	return true;

}

std::string TableNameFromVGcode(std::string variable_group_code)
{
	std::string variable_group_data_table_name("VG_INSTANCE_DATA_");
	variable_group_data_table_name += variable_group_code;
	return variable_group_data_table_name;
}
