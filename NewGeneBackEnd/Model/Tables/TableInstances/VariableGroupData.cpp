#include "VariableGroupData.h"
#include "../TableManager.h"
#include "../../InputModel.h"

#ifndef Q_MOC_RUN
#	include <boost/lexical_cast.hpp>
#endif

#include <set>

std::string const Table_VariableGroupMetadata_PrimaryKeys::VG_DATA_TABLE_NAME = "VG_DATA_TABLE_NAME";
std::string const Table_VariableGroupMetadata_PrimaryKeys::VG_DATA_TABLE_PRIMARY_KEY_COLUMN_NAME = "VG_DATA_TABLE_PRIMARY_KEY_COLUMN_NAME";
std::string const Table_VariableGroupMetadata_PrimaryKeys::VG_DATA_TABLE_FK_DMU_CATEGORY_CODE = "VG_DATA_TABLE_FK_DMU_CATEGORY_CODE";
std::string const Table_VariableGroupMetadata_PrimaryKeys::VG_DATA_TABLE_PRIMARY_KEY_SEQUENCE_NUMBER = "VG_DATA_TABLE_PRIMARY_KEY_SEQUENCE_NUMBER";

std::string const Table_VariableGroupMetadata_DateTimeColumns::VG_DATA_TABLE_NAME = "VG_DATA_TABLE_NAME";
std::string const Table_VariableGroupMetadata_DateTimeColumns::VG_DATA_TABLE_DATETIME_START_COLUMN_NAME = "VG_DATETIME_START_COLUMN_NAME";
std::string const Table_VariableGroupMetadata_DateTimeColumns::VG_DATA_TABLE_DATETIME_END_COLUMN_NAME = "VG_DATETIME_END_COLUMN_NAME";

bool Table_VariableGroupData::ImportStart(sqlite3 * db, WidgetInstanceIdentifier const & identifier, ImportDefinition const & import_definition, OutputModel * output_model_,
		InputModel * input_model_)
{

	std::lock_guard<std::recursive_mutex> data_lock(data_mutex);

	Executor executor(db);

	if (!identifier.code || identifier.code->empty() || !identifier.uuid || identifier.uuid->empty())
	{
		return false;
	}

	if (db)
	{
		if (!tableManager().TableExists(db, TableNameFromVGCode(*identifier.code)))
		{


			// Create the VG data table
			std::string sql_create;
			sql_create += "CREATE TABLE ";
			sql_create += table_name;
			sql_create += " (";

			bool first = true;
			std::for_each(import_definition.output_schema.schema.cbegin(),
						  import_definition.output_schema.schema.cend(), [&import_definition, &sql_create, &first](SchemaEntry const & table_schema_entry)
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






			// Add the data columns of the table to the VG_SET_MEMBER table
			int sequence_number = 0;
			std::for_each(import_definition.output_schema.schema.cbegin(),
						  import_definition.output_schema.schema.cend(), [&](SchemaEntry const & table_schema_entry)
			{

				std::string sql;
				std::string new_uuid(boost::to_upper_copy(newUUID(false)));
				sql += "INSERT INTO VG_SET_MEMBER (VG_SET_MEMBER_UUID, VG_SET_MEMBER_STRING_CODE, VG_SET_MEMBER_STRING_LONGHAND, VG_SET_MEMBER_SEQUENCE_NUMBER, VG_SET_MEMBER_NOTES1, VG_SET_MEMBER_NOTES2, VG_SET_MEMBER_NOTES3, VG_SET_MEMBER_FK_VG_CATEGORY_UUID, VG_SET_MEMBER_FLAGS, VG_SET_MEMBER_DATA_TYPE)";
				sql += " VALUES ('";
				sql += new_uuid;
				sql += "', '";
				sql += table_schema_entry.field_name;
				//sql += table_schema_entry.field_description; // TODO
				sql += "', '', ";
				sql += boost::lexical_cast<std::string>(sequence_number);
				sql += ", '', '', '', '";
				sql += *identifier.uuid;
				sql += "', '', '";
				switch (table_schema_entry.field_type)
				{
					case FIELD_TYPE_INT32:
					case FIELD_TYPE_UINT32:
						{
							sql += "INT32";
						}
						break;

					case FIELD_TYPE_INT64:
					case FIELD_TYPE_UINT64:
						{
							sql += "INT64";
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
							sql += "STRING";
						}
						break;

					case FIELD_TYPE_TIMESTAMP:
						{
							sql += "INT64";
						}
						break;

					case FIELD_TYPE_FLOAT:
						{
							sql += "FLOAT";
						}
						break;

					default:
						{
							sql += "STRING";
						}
						break;
				}

				sql += "')";
				sqlite3_stmt * stmt = NULL;
				sqlite3_prepare_v2(db, sql_create.c_str(), static_cast<int>(sql_create.size()) + 1, &stmt, NULL);

				if (stmt == NULL)
				{
					return;
				}

				int step_result = 0;

				if ((step_result = sqlite3_step(stmt)) == SQLITE_DONE)
				{
					executor.success();
				}

				if (stmt)
				{
					sqlite3_finalize(stmt);
					stmt = nullptr;
				}

				++sequence_number;
			});



			// Add entry to the VG_DATA_METADATA__DATETIME_COLUMNS table
			{
				std::string sql;
				sql += "INSERT INTO VG_DATA_METADATA__DATETIME_COLUMNS (VG_DATA_TABLE_NAME, VG_DATETIME_START_COLUMN_NAME, VG_DATETIME_END_COLUMN_NAME, VG_DATA_FK_VG_CATEGORY_UUID)";
				sql += " VALUES ('";
				sql += table_name;
				sql += "', 'DATETIME_ROW_START', 'DATETIME_ROW_END', '";
				sql += *identifier.uuid;
				sql += "')";
				sqlite3_stmt * stmt = NULL;
				sqlite3_prepare_v2(db, sql_create.c_str(), static_cast<int>(sql_create.size()) + 1, &stmt, NULL);

				if (stmt == NULL)
				{
					return;
				}

				int step_result = 0;

				if ((step_result = sqlite3_step(stmt)) == SQLITE_DONE)
				{
					executor.success();
				}

				if (stmt)
				{
					sqlite3_finalize(stmt);
					stmt = nullptr;
				}
			}






			// Add entry to the VG_DATA_METADATA__PRIMARY_KEYS table
			{
				sequence_number = 0;
				std::for_each(import_definition.output_schema.schema.cbegin(),
					import_definition.output_schema.schema.cend(), [&](SchemaEntry const & table_schema_entry)
				{

					if (table_schema_entry.IsPrimaryKey())
					{
						if (!table_schema_entry.dmu_category_string_code)
						{
							boost::format msg("Bad DMU code in construction of entries for primary key table.");
							throw NewGeneException() << newgene_error_description(msg.str());
						}
						std::string sql;
						sql += "INSERT INTO VG_DATA_METADATA__PRIMARY_KEYS (VG_DATA_TABLE_NAME, VG_DATA_TABLE_PRIMARY_KEY_COLUMN_NAME, VG_DATA_TABLE_FK_DMU_CATEGORY_CODE, VG_DATA_TABLE_PRIMARY_KEY_SEQUENCE_NUMBER, VG_DATA_TABLE_PRIMARY_KEY_IS_NUMERIC)";
						sql += " VALUES ('";
						sql += table_name;
						sql += "', '";
						sql += table_schema_entry.field_name;
						sql += "', '";
						sql += *table_schema_entry.dmu_category_string_code;
						sql += "', ";
						sql += boost::lexical_cast<std::string>(sequence_number);
						sql += "', 0)";
						sqlite3_stmt * stmt = NULL;
						sqlite3_prepare_v2(db, sql_create.c_str(), static_cast<int>(sql_create.size()) + 1, &stmt, NULL);

						if (stmt == NULL)
						{
							return;
						}

						int step_result = 0;

						if ((step_result = sqlite3_step(stmt)) == SQLITE_DONE)
						{
							executor.success();
						}

						if (stmt)
						{
							sqlite3_finalize(stmt);
							stmt = nullptr;
						}
						++sequence_number;
					}

				});
			}


		}
	}

	return false;

}

bool Table_VariableGroupData::ImportEnd(sqlite3 * db, WidgetInstanceIdentifier const & identifier, ImportDefinition const & import_definition, OutputModel * output_model_,
										InputModel * input_model_)
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

bool Table_VariableGroupData::DeleteDataTable(sqlite3 * db, InputModel * input_model_)
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
		boost::format msg("Unable to delete metadata for variable group");
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

bool Table_VariableGroupData::BuildImportDefinition
(
	sqlite3 * db,
	InputModel * input_model_,
	WidgetInstanceIdentifier const & vg,
	std::vector<std::string> const & timeRangeCols,
	std::vector<std::pair<WidgetInstanceIdentifier, std::string>> const & dmusAndCols,
	boost::filesystem::path const & filepathname,
	TIME_GRANULARITY const & the_time_granularity,
	ImportDefinition & definition,
	std::string & errorMsg
)
{

	if (!vg.uuid || vg.uuid->empty() || !vg.code || vg.code->empty())
	{
		boost::format msg("Missing the VG to refresh in the model.");
		throw NewGeneException() << newgene_error_description(msg.str());
	}

	std::string table_name = Table_VariableGroupData::TableNameFromVGCode(*vg.code);

	definition.import_type = ImportDefinition::IMPORT_TYPE__INPUT_MODEL;
	definition.input_file = filepathname;
	definition.first_row_is_header_row = true;
	definition.second_row_is_data_type_row = true;
	definition.format_qualifiers = ImportDefinition::FORMAT_QUALIFIERS__COMMA_DELIMITED;

	Schema schema_input;
	Schema schema_output;

	SchemaVector input_schema_vector;
	SchemaVector output_schema_vector;

	ImportDefinition::ImportMappings mappings;

	{

		std::fstream data_file;
		data_file.open(definition.input_file.c_str(), std::ios::in);

		if (!data_file.is_open() || !data_file.good())
		{
			data_file.close();
			boost::format msg("Cannot open data file \"%1%\"");
			msg % definition.input_file.c_str();
			errorMsg = msg.str();
			return false;
		}

		// Get column names from the file
		char line[MAX_LINE_SIZE];

		// skip first row if necessary
		data_file.getline(line, MAX_LINE_SIZE - 1);

		if (!data_file.good())
		{
			data_file.close();
			boost::format msg("Cannot read first line of data file \"%1%\"");
			msg % definition.input_file.c_str();
			errorMsg = msg.str();
			return false;
		}

		std::vector<std::string> colnames;
		boost::split(colnames, line, boost::is_any_of(","));
		std::for_each(colnames.begin(), colnames.end(), std::bind(boost::trim<std::string>, std::placeholders::_1, std::locale()));

		data_file.getline(line, MAX_LINE_SIZE - 1);

		if (!data_file.good())
		{
			data_file.close();
			boost::format msg("Cannot read second line of data file \"%1%\"");
			msg % definition.input_file.c_str();
			errorMsg = msg.str();
			return false;
		}

		std::vector<std::string> coltypes;
		boost::split(coltypes, line, boost::is_any_of(","));
		std::for_each(coltypes.begin(), coltypes.end(), std::bind(boost::trim<std::string>, std::placeholders::_1, std::locale()));

		data_file.close();

		std::set<std::string> testcols(colnames.cbegin(), colnames.cend());

		if (testcols.size() != colnames.size())
		{
			boost::format msg("There are duplicate column names in the input.");
			errorMsg = msg.str();
			return false;
		}

		if (coltypes.size() != colnames.size())
		{
			boost::format msg("The number of column types on the second line does not match the number of columns.");
			errorMsg = msg.str();
			return false;
		}

		std::vector<FIELD_TYPE> fieldtypes;
		bool validtype = true;
		std::for_each(coltypes.cbegin(), coltypes.cend(), [&](std::string coltype)
		{
			if (!validtype)
			{
				return;
			}

			boost::trim(coltype);
			boost::to_lower(coltype);

			if (coltype == "int" || coltype == "int32")
			{
				fieldtypes.push_back(FIELD_TYPE_INT32);
			}
			else if (coltype == "int64")
			{
				fieldtypes.push_back(FIELD_TYPE_INT64);
			}
			else if (coltype == "string")
			{
				fieldtypes.push_back(FIELD_TYPE_STRING_FIXED);
			}
			else if (coltype == "float" || coltype == "double")
			{
				fieldtypes.push_back(FIELD_TYPE_FLOAT);
			}
			else
			{
				validtype = false;
			}
		});

		if (!validtype)
		{
			boost::format msg("The column type specifications on the second line are invalid.");
			errorMsg = msg.str();
			return false;
		}

		// If the table already exists, check that the columns match
		WidgetInstanceIdentifiers existing_column_identifiers = input_model_->t_vgp_setmembers.getIdentifiers(*vg.uuid);

		if (!existing_column_identifiers.empty())
		{

			if (existing_column_identifiers.size() != colnames.size())
			{
				boost::format msg("The number of columns in the input file does not match the number of columns in the existing data.");
				msg % definition.input_file.c_str();
				errorMsg = msg.str();
				return false;
			}

			bool mismatch = false;
			int index = 0;
			std::for_each(existing_column_identifiers.cbegin(), existing_column_identifiers.cend(), [&](WidgetInstanceIdentifier const & existing_column_identifier)
			{

				if (mismatch)
				{
					return;
				}

				if (!existing_column_identifier.code || existing_column_identifier.code->empty())
				{
					mismatch = true;
					return;
				}

				if (*existing_column_identifier.code != colnames[index])
				{
					mismatch = true;
					return;
				}

				++index;

			});

			if (mismatch)
			{
				boost::format msg("The column names in the input file do not match the column names for the existing data.");
				msg % definition.input_file.c_str();
				errorMsg = msg.str();
				return false;
			}

		}

		int number_primary_key_cols = 0;
		int number_time_range_cols = 0;
		int colindex = 0;
		std::for_each(colnames.cbegin(), colnames.cend(), [&](std::string const & colname)
		{
			// Determine if this is a primary key field
			bool primary_key_col = true;

			if (std::find_if(dmusAndCols.cbegin(), dmusAndCols.cend(), [&](std::pair<WidgetInstanceIdentifier, std::string> const & dmuAndCol) -> bool
		{
			if (dmuAndCol.second == colname)
				{
					return true;
				}
				return false;
			}) == dmusAndCols.cend())
			{
				// Not a key field
				primary_key_col = false;
			}
			else
			{
				++number_primary_key_cols;
			}

			// Determine if this is a time range column
			bool time_range_col = true;

			if (std::find_if(timeRangeCols.cbegin(), timeRangeCols.cend(), [&](std::string const & timeRangeCol) -> bool
		{
			if (timeRangeCol == colname)
				{
					return true;
				}
				return false;
			}) == timeRangeCols.cend())
			{
				// Not a time range column
				// no-op
			}
			else
			{
				++number_time_range_cols;
			}

			if (primary_key_col)
			{
				input_schema_vector.push_back(SchemaEntry(colname, fieldtypes[colindex], colname));
				output_schema_vector.push_back(SchemaEntry(colname, fieldtypes[colindex], colname, true));
			}
			else
			{
				input_schema_vector.push_back(SchemaEntry(fieldtypes[colindex], colname));
				output_schema_vector.push_back(SchemaEntry(fieldtypes[colindex], colname, true));
			}

			++colindex;
		});

		// No double-counting, because all column names are guaranteed unique,
		// with a test both on the client side (for duplicate DMU primary key column names)
		// and a test above (for any duplicate column names)
		if (number_primary_key_cols != dmusAndCols.size())
		{
			boost::format msg("Not all specified DMU primary key columns could be found in the input file.");
			errorMsg = msg.str();
			return false;
		}

		// Time-range duplicates have also been screened
		if (number_time_range_cols != timeRangeCols.size())
		{
			bool good = false;

			if (the_time_granularity == TIME_GRANULARITY__YEAR)
			{
				if (timeRangeCols.size() == 2 && boost::trim_copy(timeRangeCols[1]).empty())
				{
					if (number_time_range_cols == 1)
					{
						// special case: this is OK, just one column
						good = true;
					}
				}
			}

			if (!good)
			{
				boost::format msg("Not all specified time range columns could be found in the input file.");
				errorMsg = msg.str();
				return false;
			}
		}

		// First, the time range mapping
		switch (the_time_granularity)
		{

			case TIME_GRANULARITY__DAY:
				{
					if (timeRangeCols.size() != 6)
					{
						boost::format msg("There should be 6 time columns.");
						errorMsg = msg.str();
						return false;
					}

					std::shared_ptr<TimeRangeFieldMapping> time_range_mapping = std::make_shared<TimeRangeFieldMapping>
							(TimeRangeFieldMapping::TIME_RANGE_FIELD_MAPPING_TYPE__DAY__RANGE__FROM__YR_MNTH_DAY);
					FieldTypeEntries input_file_fields;
					FieldTypeEntries output_table_fields;
					FieldTypeEntry input_time_field__YearStart = std::make_pair(NameOrIndex(NameOrIndex::NAME, timeRangeCols[0]), FIELD_TYPE_INT32);
					FieldTypeEntry input_time_field__MonthStart = std::make_pair(NameOrIndex(NameOrIndex::NAME, timeRangeCols[1]), FIELD_TYPE_INT32);
					FieldTypeEntry input_time_field__DayStart = std::make_pair(NameOrIndex(NameOrIndex::NAME, timeRangeCols[2]), FIELD_TYPE_INT32);
					FieldTypeEntry input_time_field__YearEnd = std::make_pair(NameOrIndex(NameOrIndex::NAME, timeRangeCols[3]), FIELD_TYPE_INT32);
					FieldTypeEntry input_time_field__MonthEnd = std::make_pair(NameOrIndex(NameOrIndex::NAME, timeRangeCols[4]), FIELD_TYPE_INT32);
					FieldTypeEntry input_time_field__DayEnd = std::make_pair(NameOrIndex(NameOrIndex::NAME, timeRangeCols[5]), FIELD_TYPE_INT32);
					input_file_fields.push_back(input_time_field__YearStart);
					input_file_fields.push_back(input_time_field__MonthStart);
					input_file_fields.push_back(input_time_field__DayStart);
					input_file_fields.push_back(input_time_field__YearEnd);
					input_file_fields.push_back(input_time_field__MonthEnd);
					input_file_fields.push_back(input_time_field__DayEnd);
					FieldTypeEntry output_time_field__DayStart = std::make_pair(NameOrIndex(NameOrIndex::NAME, "DATETIME_ROW_START"), FIELD_TYPE_INT64);
					FieldTypeEntry output_time_field__DayEnd = std::make_pair(NameOrIndex(NameOrIndex::NAME, "DATETIME_ROW_END"), FIELD_TYPE_INT64);
					output_table_fields.push_back(output_time_field__DayStart);
					output_table_fields.push_back(output_time_field__DayEnd);
					time_range_mapping->input_file_fields = input_file_fields;
					time_range_mapping->output_table_fields = output_table_fields;
					mappings.push_back(time_range_mapping);
				}
				break;

			case TIME_GRANULARITY__YEAR:
				{
					// Time-range mapping
					if (timeRangeCols.size() != 2)
					{
						boost::format msg("There should be 2 time columns.");
						errorMsg = msg.str();
						return false;
					}

					if (boost::trim_copy(timeRangeCols[1]).empty())
					{
						std::shared_ptr<TimeRangeFieldMapping> time_range_mapping = std::make_shared<TimeRangeFieldMapping>(TimeRangeFieldMapping::TIME_RANGE_FIELD_MAPPING_TYPE__YEAR);
						FieldTypeEntries input_file_fields;
						FieldTypeEntries output_table_fields;
						FieldTypeEntry input_time_field__Year = std::make_pair(NameOrIndex(NameOrIndex::NAME, timeRangeCols[0]), FIELD_TYPE_INT32);
						input_file_fields.push_back(input_time_field__Year);
						FieldTypeEntry output_time_field__YearStart = std::make_pair(NameOrIndex(NameOrIndex::NAME, "DATETIME_ROW_START"), FIELD_TYPE_INT64);
						FieldTypeEntry output_time_field__YearEnd = std::make_pair(NameOrIndex(NameOrIndex::NAME, "DATETIME_ROW_END"), FIELD_TYPE_INT64);
						output_table_fields.push_back(output_time_field__YearStart);
						output_table_fields.push_back(output_time_field__YearEnd);
						time_range_mapping->input_file_fields = input_file_fields;
						time_range_mapping->output_table_fields = output_table_fields;
						mappings.push_back(time_range_mapping);
					}
					else
					{
						std::shared_ptr<TimeRangeFieldMapping> time_range_mapping = std::make_shared<TimeRangeFieldMapping>
								(TimeRangeFieldMapping::TIME_RANGE_FIELD_MAPPING_TYPE__YEAR__FROM__START_YEAR__TO__END_YEAR);
						FieldTypeEntries input_file_fields;
						FieldTypeEntries output_table_fields;
						FieldTypeEntry input_time_field__YearStart = std::make_pair(NameOrIndex(NameOrIndex::NAME, timeRangeCols[0]), FIELD_TYPE_INT32);
						FieldTypeEntry input_time_field__YearEnd = std::make_pair(NameOrIndex(NameOrIndex::NAME, timeRangeCols[1]), FIELD_TYPE_INT32);
						input_file_fields.push_back(input_time_field__YearStart);
						input_file_fields.push_back(input_time_field__YearEnd);
						FieldTypeEntry output_time_field__YearStart = std::make_pair(NameOrIndex(NameOrIndex::NAME, "DATETIME_ROW_START"), FIELD_TYPE_INT64);
						FieldTypeEntry output_time_field__YearEnd = std::make_pair(NameOrIndex(NameOrIndex::NAME, "DATETIME_ROW_END"), FIELD_TYPE_INT64);
						output_table_fields.push_back(output_time_field__YearStart);
						output_table_fields.push_back(output_time_field__YearEnd);
						time_range_mapping->input_file_fields = input_file_fields;
						time_range_mapping->output_table_fields = output_table_fields;
						mappings.push_back(time_range_mapping);
					}
				}
				break;

			default:
				{
					boost::format msg("Invalid time range specification in construction of import definition for variable group.");
					errorMsg = msg.str();
					return false;
				}
				break;

		}

		colindex = 0;
		std::for_each(colnames.cbegin(), colnames.cend(), [&](std::string const & colname)
		{
			mappings.push_back(std::make_shared<OneToOneFieldMapping>(std::make_pair(NameOrIndex(NameOrIndex::NAME, colname), fieldtypes[colindex]),
							   std::make_pair(NameOrIndex(NameOrIndex::NAME, colname), fieldtypes[colindex])));
			++colindex;
		});

	}

	schema_input.schema = input_schema_vector;
	schema_output.schema = output_schema_vector;

	definition.input_schema = schema_input;
	definition.output_schema = schema_output;

	definition.mappings = mappings;

	return true;

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

			identifiers_map[vg_data_table_name].push_back(WidgetInstanceIdentifier(std::string(vg_data_dmu_category_code), vg_data_primary_key_column_name, vg_data_primary_key_sequence_number,
					flags.c_str()));
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

	boost::format delete_stmt("DELETE FROM VG_DATA_METADATA__PRIMARY_KEYS WHERE VG_DATA_TABLE_NAME = '%1%'");
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

std::vector<std::pair<WidgetInstanceIdentifier, std::vector<std::string>>> Table_VariableGroupMetadata_PrimaryKeys::GetColumnNamesCorrespondingToPrimaryKeys(sqlite3 * db,
		InputModel * input_model_, std::string const & table_name)
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
	std::for_each(map_from_dmu_category_code_to_vector_of_primary_key_column_names.cbegin(),
				  map_from_dmu_category_code_to_vector_of_primary_key_column_names.cend(), [&](std::pair<std::string const, std::vector<std::string>> const & single_dmu_category_primary_key_columns)
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

	boost::format delete_stmt("DELETE FROM VG_DATA_METADATA__DATETIME_COLUMNS WHERE VG_DATA_TABLE_NAME = '%1%'");
	delete_stmt % table_name;
	char * errmsg = nullptr;
	sqlite3_exec(db, delete_stmt.str().c_str(), NULL, NULL, &errmsg);

	if (errmsg != nullptr)
	{
		boost::format msg("Unable to delete data for variable group in datetime table: %1%");
		msg % errmsg;
		sqlite3_free(errmsg);
		throw NewGeneException() << newgene_error_description(msg.str());
	}

	// delete from cache
	identifiers_map.erase(table_name);

	theExecutor.success();

	return theExecutor.succeeded();

}
