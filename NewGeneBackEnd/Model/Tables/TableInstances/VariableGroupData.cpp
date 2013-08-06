#include "VariableGroupData.h"
#include "..\TableManager.h"

#ifndef Q_MOC_RUN
#	include <boost/lexical_cast.hpp>
#endif

std::string const Table_VariableGroupMetadata::VG_DATA_TABLE_NAME = "VG_DATA_TABLE_NAME";
std::string const Table_VariableGroupMetadata::VG_DATA_TABLE_PRIMARY_KEY_COLUMN_NAME = "VG_DATA_TABLE_PRIMARY_KEY_COLUMN_NAME";
std::string const Table_VariableGroupMetadata::VG_DATA_TABLE_FK_DMU_CATEGORY_CODE = "VG_DATA_TABLE_FK_DMU_CATEGORY_CODE";
std::string const Table_VariableGroupMetadata::VG_DATA_TABLE_PRIMARY_KEY_SEQUENCE_NUMBER = "VG_DATA_TABLE_PRIMARY_KEY_SEQUENCE_NUMBER";

void Table_VariableGroupData::Load(sqlite3 * db, InputModel * input_model_)
{
}

std::string Table_VariableGroupData::EscapeTicks(std::string const & s)
{
	std::string out;
	char const * cs = s.c_str();
	while (*cs != '\0')
	{
		if (*cs == '\'')
		{
			out += '\'';
		}
		out += *cs;
		++cs;
	}
	return out;
}

bool Table_VariableGroupData::ImportStart(sqlite3 * db, std::string code, ImportDefinition const & import_definition, OutputModel * output_model_, InputModel * input_model_)
{
	
	std::lock_guard<std::recursive_mutex> data_lock(data_mutex);

	Executor executor(db);

	if (db)
	{
		if (!tableManager().TableExists(db, TableNameFromVGCode(code)))
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
					sql_create += ", ";
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
			if ((step_result = sqlite3_step(stmt)) == SQLITE_DONE)
			{
				executor.success();
				return true;
			}
		}
	}

	return false;

}

bool Table_VariableGroupData::ImportBlock(sqlite3 * db, ImportDefinition const & import_definition, OutputModel * output_model_, InputModel * input_model_, DataBlock const & block, int const number_rows_in_block)
{

	std::lock_guard<std::recursive_mutex> data_lock(data_mutex);

	Executor executor(db);

	std::string sql_insert;

	sql_insert += "INSERT OR FAIL INTO ";
	sql_insert += table_name;
	sql_insert += " (";

	bool first = true;
	std::for_each(import_definition.output_schema.schema.cbegin(), import_definition.output_schema.schema.cend(), [&import_definition, &sql_insert, &first](SchemaEntry const & schema_entry)
	{
		if (!first)
		{
			sql_insert += ",";
		}
		first = false;

		sql_insert += schema_entry.field_name;
	});

	sql_insert += ") VALUES ";

	bool failed = false;
	bool first_row = true;
	for (int row = 0; row < number_rows_in_block; ++row)
	{

		if (!first_row)
		{
			sql_insert += ", ";
		}
		first_row = false;

		sql_insert += "(";

		DataFields const & row_fields = block[row];
		first = true;
		std::for_each(row_fields.cbegin(), row_fields.cend(), [&import_definition, &sql_insert, &first, &failed](std::shared_ptr<BaseField> const & field_data)
		{
			if (failed)
			{
				return; // from lambda
			}

			if (!first)
			{
				sql_insert += ", ";
			}
			first = false;

			if (!field_data)
			{
				// Todo: log error
				failed = true;
				return; // from lambda
			}

			switch (field_data->GetType())
			{
			case FIELD_TYPE_INT32:
				{
					Field<FIELD_TYPE_INT32> const & field = static_cast<Field<FIELD_TYPE_INT32> const & >(*field_data);
					sql_insert += boost::lexical_cast<std::string>(field.GetValueReference());
				}
				break;
			case FIELD_TYPE_INT64:
				{
					Field<FIELD_TYPE_INT64> const & field = static_cast<Field<FIELD_TYPE_INT64> const & >(*field_data);
					sql_insert += boost::lexical_cast<std::string>(field.GetValueReference());
				}
				break;
			case FIELD_TYPE_UINT32:
				{
					Field<FIELD_TYPE_UINT32> const & field = static_cast<Field<FIELD_TYPE_UINT32> const & >(*field_data);
					sql_insert += boost::lexical_cast<std::string>(field.GetValueReference());
				}
				break;
			case FIELD_TYPE_UINT64:
				{
					Field<FIELD_TYPE_UINT64> const & field = static_cast<Field<FIELD_TYPE_UINT64> const & >(*field_data);
					sql_insert += boost::lexical_cast<std::string>(field.GetValueReference());
				}
				break;
			case FIELD_TYPE_STRING_FIXED:
				{
					Field<FIELD_TYPE_STRING_FIXED> const & field = static_cast<Field<FIELD_TYPE_STRING_FIXED> const & >(*field_data);
					sql_insert += '\'';
					sql_insert += Table_VariableGroupData::EscapeTicks(boost::lexical_cast<std::string>(field.GetValueReference()));
					sql_insert += '\'';
				}
				break;
			case FIELD_TYPE_STRING_VAR:
				{
					Field<FIELD_TYPE_STRING_VAR> const & field = static_cast<Field<FIELD_TYPE_STRING_VAR> const & >(*field_data);
					sql_insert += '\'';
					sql_insert += Table_VariableGroupData::EscapeTicks(boost::lexical_cast<std::string>(field.GetValueReference()));
					sql_insert += '\'';
				}
				break;
			case FIELD_TYPE_TIMESTAMP:
				{
					Field<FIELD_TYPE_TIMESTAMP> const & field = static_cast<Field<FIELD_TYPE_TIMESTAMP> const & >(*field_data);
					sql_insert += boost::lexical_cast<std::string>(field.GetValueReference());
				}
				break;
			case FIELD_TYPE_UUID:
				{
					Field<FIELD_TYPE_UUID> const & field = static_cast<Field<FIELD_TYPE_UUID> const & >(*field_data);
					sql_insert += '\'';
					sql_insert += Table_VariableGroupData::EscapeTicks(boost::lexical_cast<std::string>(field.GetValueReference()));
					sql_insert += '\'';
				}
				break;
			case FIELD_TYPE_UUID_FOREIGN:
				{
					Field<FIELD_TYPE_UUID_FOREIGN> const & field = static_cast<Field<FIELD_TYPE_UUID_FOREIGN> const & >(*field_data);
					sql_insert += '\'';
					sql_insert += Table_VariableGroupData::EscapeTicks(boost::lexical_cast<std::string>(field.GetValueReference()));
					sql_insert += '\'';
				}
				break;
			case FIELD_TYPE_STRING_CODE:
				{
					Field<FIELD_TYPE_STRING_CODE> const & field = static_cast<Field<FIELD_TYPE_STRING_CODE> const & >(*field_data);
					sql_insert += '\'';
					sql_insert += Table_VariableGroupData::EscapeTicks(boost::lexical_cast<std::string>(field.GetValueReference()));
					sql_insert += '\'';
				}
				break;
			case FIELD_TYPE_STRING_LONGHAND:
				{
					Field<FIELD_TYPE_STRING_LONGHAND> const & field = static_cast<Field<FIELD_TYPE_STRING_LONGHAND> const & >(*field_data);
					sql_insert += '\'';
					sql_insert += Table_VariableGroupData::EscapeTicks(boost::lexical_cast<std::string>(field.GetValueReference()));
					sql_insert += '\'';
				}
				break;
			case FIELD_TYPE_TIME_RANGE:
				{
					Field<FIELD_TYPE_TIME_RANGE> const & field = static_cast<Field<FIELD_TYPE_TIME_RANGE> const & >(*field_data);
					sql_insert += boost::lexical_cast<std::string>(field.GetValueReference());
				}
				break;
			case FIELD_TYPE_NOTES_1:
				{
					Field<FIELD_TYPE_NOTES_1> const & field = static_cast<Field<FIELD_TYPE_NOTES_1> const & >(*field_data);
					sql_insert += '\'';
					sql_insert += Table_VariableGroupData::EscapeTicks(boost::lexical_cast<std::string>(field.GetValueReference()));
					sql_insert += '\'';
				}
				break;
			case FIELD_TYPE_NOTES_2:
				{
					Field<FIELD_TYPE_NOTES_2> const & field = static_cast<Field<FIELD_TYPE_NOTES_2> const & >(*field_data);
					sql_insert += '\'';
					sql_insert += Table_VariableGroupData::EscapeTicks(boost::lexical_cast<std::string>(field.GetValueReference()));
					sql_insert += '\'';
				}
				break;
			case FIELD_TYPE_NOTES_3:
				{
					Field<FIELD_TYPE_NOTES_3> const & field = static_cast<Field<FIELD_TYPE_NOTES_3> const & >(*field_data);
					sql_insert += '\'';
					sql_insert += Table_VariableGroupData::EscapeTicks(boost::lexical_cast<std::string>(field.GetValueReference()));
					sql_insert += '\'';
				}
				break;
			}
		});

		if (failed)
		{
			break;
		}

		sql_insert += ")";

	}

	if (failed)
	{
		return false;
	}

	sqlite3_stmt * stmt = NULL;
	sqlite3_prepare_v2(db, sql_insert.c_str(), sql_insert.size() + 1, &stmt, NULL);
	if (stmt == NULL)
	{
		// TODO: Log error
		return false;
	}
	int step_result = 0;
	if ((step_result = sqlite3_step(stmt)) != SQLITE_DONE)
	{
		// TODO: Log error
		return false;
	}

	executor.success();
	return true;

}

bool Table_VariableGroupData::ImportEnd(sqlite3 * db, ImportDefinition const & import_definition, OutputModel * output_model_, InputModel * input_model_)
{

	return true;

}

std::string Table_VariableGroupData::TableNameFromVGCode(std::string variable_group_code)
{
	std::string variable_group_data_table_name("VG_INSTANCE_DATA_");
	variable_group_data_table_name += variable_group_code;
	return variable_group_data_table_name;
}

std::string Table_VariableGroupData::ViewNameFromCount(int const view_number)
{
	char vns[1024];
	std::string view_name("v");
	view_name += itoa(view_number, vns, 10);
	return view_name;
}

std::string Table_VariableGroupData::JoinViewNameFromCount(int const join_number)
{
	char vns[1024];
	std::string join_view_name("j");
	join_view_name += itoa(join_number, vns, 10);
	return join_view_name;
}

void Table_VariableGroupMetadata::Load(sqlite3 * db, InputModel * input_model_)
{

	std::lock_guard<std::recursive_mutex> data_lock(data_mutex);

	identifiers_map.clear();

	sqlite3_stmt * stmt = NULL;
	std::string sql("SELECT * FROM VG_DATA_METADATA");
	sqlite3_prepare_v2(db, sql.c_str(), sql.size() + 1, &stmt, NULL);
	if (stmt == NULL)
	{
		return;
	}
	int step_result = 0;

	while ((step_result = sqlite3_step(stmt)) == SQLITE_ROW)
	{
		char const * vg_data_table_name = reinterpret_cast<char const *>(sqlite3_column_text(stmt, INDEX__VG_DATA_TABLE_NAME));
		char const * vg_data_primary_key_column_name = reinterpret_cast<char const *>(sqlite3_column_text(stmt, INDEX__VG_DATA_TABLE_PRIMARY_KEY_COLUMN_NAME));
		char const * vg_data_dmu_category_code = reinterpret_cast<char const *>(sqlite3_column_text(stmt, INDEX__VG_DATA_TABLE_FK_DMU_CATEGORY_CODE));
		int vg_data_primary_key_sequence_number = sqlite3_column_int(stmt, INDEX__VG_DATA_TABLE_PRIMARY_KEY_SEQUENCE_NUMBER);
		if (vg_data_dmu_category_code && strlen(vg_data_dmu_category_code))
		{
			// maps:
			// vg_data_table_name =>
			// a vector of primary keys (each a DMU category identifier)
			// In the WidgetInstanceIdentifier, the CODE is set to the DMU category code,
			// and the LONGHAND is set to the column name corresponding to this DMU in the variable group data table.
			identifiers_map[vg_data_table_name].push_back(WidgetInstanceIdentifier(std::string(vg_data_dmu_category_code), vg_data_primary_key_column_name, vg_data_primary_key_sequence_number));
		}
	}

}
