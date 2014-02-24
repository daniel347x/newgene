#include "VariableGroupData.h"
#include "../TableManager.h"
#include "../../InputModel.h"

#ifndef Q_MOC_RUN
#	include <boost/lexical_cast.hpp>
#endif

std::string const Table_VariableGroupMetadata_PrimaryKeys::VG_DATA_TABLE_NAME = "VG_DATA_TABLE_NAME";
std::string const Table_VariableGroupMetadata_PrimaryKeys::VG_DATA_TABLE_PRIMARY_KEY_COLUMN_NAME = "VG_DATA_TABLE_PRIMARY_KEY_COLUMN_NAME";
std::string const Table_VariableGroupMetadata_PrimaryKeys::VG_DATA_TABLE_FK_DMU_CATEGORY_CODE = "VG_DATA_TABLE_FK_DMU_CATEGORY_CODE";
std::string const Table_VariableGroupMetadata_PrimaryKeys::VG_DATA_TABLE_PRIMARY_KEY_SEQUENCE_NUMBER = "VG_DATA_TABLE_PRIMARY_KEY_SEQUENCE_NUMBER";

std::string const Table_VariableGroupMetadata_DateTimeColumns::VG_DATA_TABLE_NAME = "VG_DATA_TABLE_NAME";
std::string const Table_VariableGroupMetadata_DateTimeColumns::VG_DATA_TABLE_DATETIME_START_COLUMN_NAME = "VG_DATETIME_START_COLUMN_NAME";
std::string const Table_VariableGroupMetadata_DateTimeColumns::VG_DATA_TABLE_DATETIME_END_COLUMN_NAME = "VG_DATETIME_END_COLUMN_NAME";

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
				case FIELD_TYPE_FLOAT:
					{
						sql_create += "REAL";
					}
					break;
                default:
                    {
                        sql_create += "TEXT";
                    }
                    break;
				}
			});

			sql_create += ")";

			sqlite3_stmt * stmt = NULL;
			sqlite3_prepare_v2(db, sql_create.c_str(), static_cast<int>(sql_create.size()) + 1, &stmt, NULL);
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
			if (stmt)
			{
				sqlite3_finalize(stmt);
				stmt = nullptr;
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
			sql_insert += ", ";
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
			case FIELD_TYPE_FLOAT:
				{
					Field<FIELD_TYPE_FLOAT> const & field = static_cast<Field<FIELD_TYPE_FLOAT> const & >(*field_data);
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
            default:
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
	sqlite3_prepare_v2(db, sql_insert.c_str(), static_cast<int>(sql_insert.size()) + 1, &stmt, NULL);
	if (stmt == NULL)
	{
		// TODO: Log error
		return false;
	}
	int step_result = 0;
	if ((step_result = sqlite3_step(stmt)) != SQLITE_DONE)
	{
		// TODO: Log error
		if (stmt)
		{
			sqlite3_finalize(stmt);
			stmt = nullptr;
		}
		return false;
	}

	executor.success();
	if (stmt)
	{
		sqlite3_finalize(stmt);
		stmt = nullptr;
	}
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

std::string Table_VariableGroupData::ViewNameFromCountTemp(int const view_number, int const multiplicity_number)
{
	std::string view_name("vtmp");
	view_name += std::to_string(view_number);
	view_name += "_";
	view_name += std::to_string(multiplicity_number);
	return view_name;
}

std::string Table_VariableGroupData::ViewNameFromCountTempTimeRanged(int const view_number, int const multiplicity_number)
{
	std::string view_name("vtmp_TR");
	view_name += std::to_string(view_number);
	view_name += "_";
	view_name += std::to_string(multiplicity_number);
	return view_name;
}

std::string Table_VariableGroupData::ViewNameFromCount(int const view_number)
{
	std::string view_name("v");
	view_name += std::to_string(view_number);
	return view_name;
}

std::string Table_VariableGroupData::JoinViewNameFromCount(int const join_number)
{
	std::string join_view_name("j");
	join_view_name += std::to_string(join_number);
	return join_view_name;
}

std::string Table_VariableGroupData::JoinViewNameWithTimeRangesFromCount(int const join_number)
{
	std::string join_view_name("jt");
	join_view_name += std::to_string(join_number);
	return join_view_name;
}

Table_VariableGroupData * Table_VariableGroupData::GetInstanceTableFromTableName(sqlite3 * db, InputModel * input_model_, std::string const & table_name)
{

	if (!db)
	{
		return false;
	}

	if (!input_model_)
	{
		return false;
	}

	auto vg_instance_table = std::find_if(input_model_->t_vgp_data_vector.begin(), input_model_->t_vgp_data_vector.end(), [&](std::unique_ptr<Table_VariableGroupData> & test_vg_table)
	{
		if (test_vg_table)
		{
			if (boost::iequals(test_vg_table->table_name, table_name))
			{
				return true;
			}
		}
		return false;
	});

	if (vg_instance_table != input_model_->t_vgp_data_vector.end())
	{
		return vg_instance_table->get();
	}

	return nullptr;

}

bool Table_VariableGroupData::DeleteDataTable(sqlite3 * db, InputModel * input_model_, DataChangeMessage & change_message)
{

	Executor theExecutor(db);

	if (!db)
	{
		return false;
	}

	if (!input_model_)
	{
		return false;
	}

	bool dropped_primary_key_metadata = input_model_->t_vgp_data_metadata__primary_keys.DeleteDataTable(db, input_model_, table_name);
	bool dropped_datetime_metadata = input_model_->t_vgp_data_metadata__datetime_columns.DeleteDataTable(db, input_model_, table_name);
	if (!dropped_primary_key_metadata || !dropped_datetime_metadata)
	{
		boost::format msg("Unable to delete metadata for variable group: %1%");
		throw NewGeneException() << newgene_error_description(msg.str());
	}

	boost::format drop_stmt("DROP TABLE IF EXISTS %1%");
	drop_stmt % table_name;
	char * errmsg = nullptr;
	sqlite3_exec(db, drop_stmt.str().c_str(), NULL, NULL, &errmsg);
	if (errmsg != nullptr)
	{
		boost::format msg("Unable to delete data for variable group: %1%");
		msg % errmsg;
		sqlite3_free(errmsg);
		throw NewGeneException() << newgene_error_description(msg.str());
	}

	// ***************************************** //
	// Prepare data to send back to user interface
	// ***************************************** //

	WidgetInstanceIdentifier vg;
	bool found(input_model_->t_vgp_identifiers.getIdentifierFromStringCode(vg_category_string_code, vg));
	if (!found)
	{
		return false;
	}

	DATA_CHANGE_TYPE type = DATA_CHANGE_TYPE__INPUT_MODEL__VG_INSTANCE_DATA_CHANGE;
	DATA_CHANGE_INTENTION intention = DATA_CHANGE_INTENTION__REMOVE;
	DataChange change(type, intention, vg, WidgetInstanceIdentifiers());
	change_message.changes.push_back(change);

	theExecutor.success();

	return theExecutor.succeeded();

}

bool Table_VariableGroupData::DeleteDmuMemberRows(sqlite3 * db, InputModel * input_model_, WidgetInstanceIdentifier const & dmu_member, std::string const & column_name)
{

	std::lock_guard<std::recursive_mutex> data_lock(data_mutex);

	Executor theExecutor(db);

	if (!dmu_member.uuid || dmu_member.uuid->empty() || !dmu_member.identifier_parent || !dmu_member.identifier_parent->code || !dmu_member.identifier_parent->uuid)
	{
		return false;
	}

	WidgetInstanceIdentifier dmu_category = *dmu_member.identifier_parent;

	sqlite3_stmt * stmt = NULL;
	boost::format sql("DELETE FROM %1% WHERE %2% = '%3%'");
	sql % table_name % column_name % *dmu_member.uuid;
	int err = sqlite3_prepare_v2(db, sql.str().c_str(), static_cast<int>(sql.str().size()) + 1, &stmt, NULL);
	if (stmt == NULL)
	{
		boost::format msg("Unable to prepare DELETE statement to delete DMU member data from column %1% in table %2% corresponding to DMU member %3% in DMU category %4%: %5%");
		msg % column_name % table_name % Table_DMU_Instance::GetDmuMemberDisplayText(dmu_member) % Table_DMU_Identifier::GetDmuCategoryDisplayText(dmu_category) % sqlite3_errstr(err);
		throw NewGeneException() << newgene_error_description(msg.str());
	}
	int step_result = 0;
	step_result = sqlite3_step(stmt);
	if (step_result != SQLITE_DONE)
	{
		boost::format msg("Unable to execute DELETE statement to delete DMU member data from column %1% in table %2% corresponding to DMU member %3% in DMU category %4%: %5%");
		msg % column_name % table_name % Table_DMU_Instance::GetDmuMemberDisplayText(dmu_member) % Table_DMU_Identifier::GetDmuCategoryDisplayText(dmu_category) % sqlite3_errstr(err);
		throw NewGeneException() << newgene_error_description(msg.str());
	}
	if (stmt)
	{
		sqlite3_finalize(stmt);
		stmt = nullptr;
	}

	theExecutor.success();

	return theExecutor.succeeded();

}

void Table_VariableGroupMetadata_PrimaryKeys::Load(sqlite3 * db, InputModel * input_model_)
{

	std::lock_guard<std::recursive_mutex> data_lock(data_mutex);

	identifiers_map.clear();

	sqlite3_stmt * stmt = NULL;
	std::string sql("SELECT * FROM VG_DATA_METADATA__PRIMARY_KEYS");
	sqlite3_prepare_v2(db, sql.c_str(), static_cast<int>(sql.size()) + 1, &stmt, NULL);
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
		int vg_data_primary_key_is_numeric = sqlite3_column_int(stmt, INDEX__VG_DATA_TABLE_PRIMARY_KEY_IS_NUMERIC);
		if (vg_data_dmu_category_code && strlen(vg_data_dmu_category_code))
		{
			// maps:
			// vg_data_table_name =>
			// a vector of primary keys (each a DMU category identifier)
			// In the WidgetInstanceIdentifier, the CODE is set to the DMU category code,
			// the LONGHAND is set to the column name corresponding to this DMU in the variable group data table,
			// and the SEQUENCE NUMBER is set to the sequence number of the primary key in this variable group.
			std::string flags;
			if (vg_data_primary_key_is_numeric)
			{
				flags = "n";
			}
			identifiers_map[vg_data_table_name].push_back(WidgetInstanceIdentifier(std::string(vg_data_dmu_category_code), vg_data_primary_key_column_name, vg_data_primary_key_sequence_number, flags.c_str()));
		}
	}

	if (stmt)
	{
		sqlite3_finalize(stmt);
		stmt = nullptr;
	}

}

bool Table_VariableGroupMetadata_PrimaryKeys::DeleteDataTable(sqlite3 * db, InputModel * input_model_, std::string const & table_name)
{

	Executor theExecutor(db);

	if (!db)
	{
		return false;
	}

	if (!input_model_)
	{
		return false;
	}

	boost::format delete_stmt("DELETE FROM VG_DATA_METADATA__PRIMARY_KEYS WHERE VG_DATA_TABLE_NAME = '%1%");
	delete_stmt % table_name;
	char * errmsg = nullptr;
	sqlite3_exec(db, delete_stmt.str().c_str(), NULL, NULL, &errmsg);
	if (errmsg != nullptr)
	{
		boost::format msg("Unable to delete data for variable group in primary keys table: %1%");
		msg % errmsg;
		sqlite3_free(errmsg);
		throw NewGeneException() << newgene_error_description(msg.str());
	}

	// delete from cache
	identifiers_map.erase(table_name);

	theExecutor.success();

	return theExecutor.succeeded();

}

std::vector<std::pair<WidgetInstanceIdentifier, std::vector<std::string>>> Table_VariableGroupMetadata_PrimaryKeys::GetColumnNamesCorrespondingToPrimaryKeys(sqlite3 * db, InputModel * input_model_, std::string const & table_name)
{

	std::lock_guard<std::recursive_mutex> data_lock(data_mutex);

	WidgetInstanceIdentifiers primary_keys = identifiers_map[table_name];

	// temp storage
	std::map<std::string, std::vector<std::string>> map_from_dmu_category_code_to_vector_of_primary_key_column_names;
	std::for_each(primary_keys.cbegin(), primary_keys.cend(), [&](WidgetInstanceIdentifier const & primary_key)
	{
		// From above:
		// In the WidgetInstanceIdentifier, the CODE is set to the DMU category code,
		// the LONGHAND is set to the column name corresponding to this DMU in the variable group data table,
		// and the SEQUENCE NUMBER is set to the sequence number of the primary key in this variable group.
		map_from_dmu_category_code_to_vector_of_primary_key_column_names[*primary_key.code].push_back(*primary_key.longhand);
	});

	// create output in desired format
	std::vector<std::pair<WidgetInstanceIdentifier, std::vector<std::string>>> output;
	std::for_each(map_from_dmu_category_code_to_vector_of_primary_key_column_names.cbegin(), map_from_dmu_category_code_to_vector_of_primary_key_column_names.cend(), [&](std::pair<std::string const, std::vector<std::string>> const & single_dmu_category_primary_key_columns)
	{
		std::string const & dmu_category_string_code = single_dmu_category_primary_key_columns.first;
		std::vector<std::string> const & primary_key_column_names = single_dmu_category_primary_key_columns.second;
		WidgetInstanceIdentifier dmu_category;
		bool found = input_model_->t_dmu_category.getIdentifierFromStringCode(dmu_category_string_code, dmu_category);
		if (!found)
		{
			return;
		}
		output.push_back(std::make_pair(dmu_category, primary_key_column_names));
	});

	return output;

}

void Table_VariableGroupMetadata_DateTimeColumns::Load(sqlite3 * db, InputModel * input_model_)
{

	std::lock_guard<std::recursive_mutex> data_lock(data_mutex);

	identifiers_map.clear();

	sqlite3_stmt * stmt = NULL;
	std::string sql("SELECT * FROM VG_DATA_METADATA__DATETIME_COLUMNS");
	sqlite3_prepare_v2(db, sql.c_str(), static_cast<int>(sql.size()) + 1, &stmt, NULL);
	if (stmt == NULL)
	{
		return;
	}
	int step_result = 0;

	while ((step_result = sqlite3_step(stmt)) == SQLITE_ROW)
	{
		char const * vg_data_table_name = reinterpret_cast<char const *>(sqlite3_column_text(stmt, INDEX__VG_DATA_TABLE_NAME));
		char const * vg_data_datetime_start_column_name = reinterpret_cast<char const *>(sqlite3_column_text(stmt, INDEX__VG_DATA_TABLE_DATETIME_START_COLUMN_NAME));
		char const * vg_data_datetime_end_column_name = reinterpret_cast<char const *>(sqlite3_column_text(stmt, INDEX__VG_DATA_TABLE_DATETIME_END_COLUMN_NAME));
		// maps:
		// vg_data_table_name =>
		// a pair of datetime keys stuck into a vector (the first for the start datetime; the second for the end datetime)
		// In the WidgetInstanceIdentifier, the CODE is set to the column name
		identifiers_map[vg_data_table_name].push_back(WidgetInstanceIdentifier(std::string(vg_data_datetime_start_column_name)));
		identifiers_map[vg_data_table_name].push_back(WidgetInstanceIdentifier(std::string(vg_data_datetime_end_column_name)));
	}

	if (stmt)
	{
		sqlite3_finalize(stmt);
		stmt = nullptr;
	}

}

bool Table_VariableGroupMetadata_DateTimeColumns::DeleteDataTable(sqlite3 * db, InputModel * input_model_, std::string const & table_name)
{

	Executor theExecutor(db);

	if (!db)
	{
		return false;
	}

	if (!input_model_)
	{
		return false;
	}

	boost::format delete_stmt("DELETE FROM VG_DATA_METADATA__DATETIME_COLUMNS WHERE VG_DATA_TABLE_NAME = '%1%");
	delete_stmt % table_name;
	char * errmsg = nullptr;
	sqlite3_exec(db, delete_stmt.str().c_str(), NULL, NULL, &errmsg);
	if (errmsg != nullptr)
	{
		sqlite3_free(errmsg);
		boost::format msg("Unable to delete data for variable group in datetime table.");
		throw NewGeneException() << newgene_error_description(msg.str());
	}

	// delete from cache
	identifiers_map.erase(table_name);

	theExecutor.success();

	return theExecutor.succeeded();

}
