#include "table.h"

bool Table_basemost::ImportBlockBulk(sqlite3 * db, ImportDefinition const & import_definition, OutputModel * output_model_, InputModel * input_model_, DataBlock const & block,
									 int const number_rows_in_block)
{

	std::lock_guard<std::recursive_mutex> data_lock(data_mutex);

	Executor executor(db);

	std::string sql_insert;

	sql_insert += "INSERT OR FAIL INTO ";
	sql_insert += GetTableName();
	sql_insert += " (";

	bool first = true;
	std::for_each(import_definition.output_schema.schema.cbegin(),
				  import_definition.output_schema.schema.cend(), [&import_definition, &sql_insert, &first](SchemaEntry const & schema_entry)
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
		std::for_each(row_fields.cbegin(), row_fields.cend(), [&](std::shared_ptr<BaseField> const & field_data)
		{

			//if (failed)
			//{
			//	return; // from lambda
			//}

			if (!field_data)
			{
				// Todo: log error
				failed = true;
				return; // from lambda
			}

			if (!first)
			{
				sql_insert += ", ";
			}

			first = false;

			FieldDataAsSqlText(field_data, sql_insert);

		});

		//if (failed)
		//{
		//	break;
		//}

		sql_insert += ")";

	}

	//if (failed)
	//{
	//	return false;
	//}

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

bool Table_basemost::ImportBlockUpdate(sqlite3 * db, ImportDefinition const & import_definition, OutputModel * output_model_, InputModel * input_model_, DataBlock const & block,
									   int const number_rows_in_block)
{

	std::lock_guard<std::recursive_mutex> data_lock(data_mutex);

	Executor executor(db);

	bool failed = false;

	for (int row = 0; row < number_rows_in_block; ++row)
	{

		int changes = TryUpdateRow(block, row, failed, import_definition, db);

		if (failed)
		{
			continue;
		}

		if (changes == 0)
		{
			// Update did not affect any row.  Insert a new row
			TryInsertRow(block, row, failed, import_definition, db);
		}

		if (failed)
		{
			continue;
		}

	}

	//if (failed)
	//{
	//	return false;
	//}

	executor.success();

	return true;

}

void Table_basemost::FieldDataAsSqlText(std::shared_ptr<BaseField> const & field_data, std::string & sql_insert)
{
	switch (field_data->GetType())
	{
		case FIELD_TYPE_INT32:
			{
				Field<FIELD_TYPE_INT32> const & field = static_cast<Field<FIELD_TYPE_INT32> const &>(*field_data);
				sql_insert += boost::lexical_cast<std::string>(field.GetValueReference());
			}
			break;

		case FIELD_TYPE_INT64:
			{
				Field<FIELD_TYPE_INT64> const & field = static_cast<Field<FIELD_TYPE_INT64> const &>(*field_data);
				sql_insert += boost::lexical_cast<std::string>(field.GetValueReference());
			}
			break;

		case FIELD_TYPE_UINT32:
			{
				Field<FIELD_TYPE_UINT32> const & field = static_cast<Field<FIELD_TYPE_UINT32> const &>(*field_data);
				sql_insert += boost::lexical_cast<std::string>(field.GetValueReference());
			}
			break;

		case FIELD_TYPE_UINT64:
			{
				Field<FIELD_TYPE_UINT64> const & field = static_cast<Field<FIELD_TYPE_UINT64> const &>(*field_data);
				sql_insert += boost::lexical_cast<std::string>(field.GetValueReference());
			}
			break;

		case FIELD_TYPE_FLOAT:
			{
				Field<FIELD_TYPE_FLOAT> const & field = static_cast<Field<FIELD_TYPE_FLOAT> const &>(*field_data);
				sql_insert += boost::lexical_cast<std::string>(field.GetValueReference());
			}
			break;

		case FIELD_TYPE_STRING_FIXED:
			{
				Field<FIELD_TYPE_STRING_FIXED> const & field = static_cast<Field<FIELD_TYPE_STRING_FIXED> const &>(*field_data);
				sql_insert += '\'';
				sql_insert += Table_basemost::EscapeTicks(boost::lexical_cast<std::string>(field.GetValueReference()));
				sql_insert += '\'';
			}
			break;

		case FIELD_TYPE_STRING_VAR:
			{
				Field<FIELD_TYPE_STRING_VAR> const & field = static_cast<Field<FIELD_TYPE_STRING_VAR> const &>(*field_data);
				sql_insert += '\'';
				sql_insert += Table_basemost::EscapeTicks(boost::lexical_cast<std::string>(field.GetValueReference()));
				sql_insert += '\'';
			}
			break;

		case FIELD_TYPE_TIMESTAMP:
			{
				Field<FIELD_TYPE_TIMESTAMP> const & field = static_cast<Field<FIELD_TYPE_TIMESTAMP> const &>(*field_data);
				sql_insert += boost::lexical_cast<std::string>(field.GetValueReference());
			}
			break;

		case FIELD_TYPE_UUID:
			{
				Field<FIELD_TYPE_UUID> const & field = static_cast<Field<FIELD_TYPE_UUID> const &>(*field_data);
				sql_insert += '\'';
				sql_insert += Table_basemost::EscapeTicks(boost::lexical_cast<std::string>(field.GetValueReference()));
				sql_insert += '\'';
			}
			break;

		case FIELD_TYPE_UUID_FOREIGN:
			{
				Field<FIELD_TYPE_UUID_FOREIGN> const & field = static_cast<Field<FIELD_TYPE_UUID_FOREIGN> const &>(*field_data);
				sql_insert += '\'';
				sql_insert += Table_basemost::EscapeTicks(boost::lexical_cast<std::string>(field.GetValueReference()));
				sql_insert += '\'';
			}
			break;

		case FIELD_TYPE_STRING_CODE:
			{
				Field<FIELD_TYPE_STRING_CODE> const & field = static_cast<Field<FIELD_TYPE_STRING_CODE> const &>(*field_data);
				sql_insert += '\'';
				sql_insert += Table_basemost::EscapeTicks(boost::lexical_cast<std::string>(field.GetValueReference()));
				sql_insert += '\'';
			}
			break;

		case FIELD_TYPE_STRING_LONGHAND:
			{
				Field<FIELD_TYPE_STRING_LONGHAND> const & field = static_cast<Field<FIELD_TYPE_STRING_LONGHAND> const &>(*field_data);
				sql_insert += '\'';
				sql_insert += Table_basemost::EscapeTicks(boost::lexical_cast<std::string>(field.GetValueReference()));
				sql_insert += '\'';
			}
			break;

		case FIELD_TYPE_TIME_RANGE:
			{
				Field<FIELD_TYPE_TIME_RANGE> const & field = static_cast<Field<FIELD_TYPE_TIME_RANGE> const &>(*field_data);
				sql_insert += boost::lexical_cast<std::string>(field.GetValueReference());
			}
			break;

		case FIELD_TYPE_NOTES_1:
			{
				Field<FIELD_TYPE_NOTES_1> const & field = static_cast<Field<FIELD_TYPE_NOTES_1> const &>(*field_data);
				sql_insert += '\'';
				sql_insert += Table_basemost::EscapeTicks(boost::lexical_cast<std::string>(field.GetValueReference()));
				sql_insert += '\'';
			}
			break;

		case FIELD_TYPE_NOTES_2:
			{
				Field<FIELD_TYPE_NOTES_2> const & field = static_cast<Field<FIELD_TYPE_NOTES_2> const &>(*field_data);
				sql_insert += '\'';
				sql_insert += Table_basemost::EscapeTicks(boost::lexical_cast<std::string>(field.GetValueReference()));
				sql_insert += '\'';
			}
			break;

		case FIELD_TYPE_NOTES_3:
			{
				Field<FIELD_TYPE_NOTES_3> const & field = static_cast<Field<FIELD_TYPE_NOTES_3> const &>(*field_data);
				sql_insert += '\'';
				sql_insert += Table_basemost::EscapeTicks(boost::lexical_cast<std::string>(field.GetValueReference()));
				sql_insert += '\'';
			}
			break;

		default:
			break;
	}

}

int Table_basemost::TryUpdateRow(DataBlock const & block, int row, bool & failed, ImportDefinition const & import_definition, sqlite3 * db)
{
	std::string sql_insert;

	DataFields const & row_fields = block[row];

	sql_insert += "UPDATE ";
	sql_insert += GetTableName();
	sql_insert += " SET ";

	bool first = true;
	int index = 0;
	bool innerFailed = false;
	std::for_each(import_definition.output_schema.schema.cbegin(),
				  import_definition.output_schema.schema.cend(), [&](SchemaEntry const & schema_entry)
	{

		if (innerFailed)
		{
			return;
		}

		std::shared_ptr<BaseField> const & field_data = row_fields[index];

		if (!field_data)
		{
			// Todo: log error
			innerFailed = true;
			return;
		}

		if (!first)
		{
			sql_insert += ", ";
		}

		first = false;

		sql_insert += schema_entry.field_name;

		sql_insert += " = ";

		FieldDataAsSqlText(field_data, sql_insert);

		++index;

	});

	if (innerFailed)
	{
		failed = true;
		return 0;
	}

	sql_insert += " WHERE ";

	first = true;
	index = 0;
	innerFailed = false;
	std::for_each(import_definition.output_schema.schema.cbegin(),
				  import_definition.output_schema.schema.cend(), [&](SchemaEntry const & schema_entry)
	{

		if (innerFailed)
		{
			return;
		}

		if (schema_entry.IsPrimaryKey())
		{

			std::shared_ptr<BaseField> const & field_data = row_fields[index];

			if (!field_data)
			{
				// Todo: log error
				innerFailed = true;
				return;
			}

			if (!first)
			{
				sql_insert += " AND ";
			}

			first = false;

			sql_insert += schema_entry.field_name;

			sql_insert += " = ";

			FieldDataAsSqlText(field_data, sql_insert);

		}

		++index;

	});

	if (innerFailed)
	{
		failed = true;
		return 0;
	}

	sqlite3_stmt * stmt = nullptr;
	sqlite3_prepare_v2(db, sql_insert.c_str(), static_cast<int>(sql_insert.size()) + 1, &stmt, NULL);

	if (stmt == NULL)
	{
		// TODO: Log error
		failed = true;
		return 0;
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

		failed = true;
		return 0;
	}

	if (stmt)
	{
		sqlite3_finalize(stmt);
		stmt = nullptr;
	}

	return sqlite3_changes(db);

}

void Table_basemost::TryInsertRow(DataBlock const & block, int row, bool & failed, ImportDefinition const & import_definition, sqlite3 * db)
{

	std::string sql_insert;

	DataFields const & row_fields = block[row];

	sql_insert += "INSERT INTO ";
	sql_insert += GetTableName();
	sql_insert += " ( ";

	bool first = true;
	int index = 0;
	std::for_each(import_definition.output_schema.schema.cbegin(),
				  import_definition.output_schema.schema.cend(), [&](SchemaEntry const & schema_entry)
	{

		if (!first)
		{
			sql_insert += ", ";
		}

		first = false;

		sql_insert += schema_entry.field_name;

		++index;

	});

	sql_insert += ") ";

	sql_insert += " VALUES ";

	sql_insert += "(";

	first = true;
	index = 0;
	bool innerFailed = false;
	std::for_each(import_definition.output_schema.schema.cbegin(),
				  import_definition.output_schema.schema.cend(), [&](SchemaEntry const & schema_entry)
	{

		if (innerFailed)
		{
			return;
		}

		std::shared_ptr<BaseField> const & field_data = row_fields[index];

		if (!field_data)
		{
			// Todo: log error
			innerFailed = true;
			return;
		}

		if (!first)
		{
			sql_insert += ", ";
		}

		first = false;

		FieldDataAsSqlText(field_data, sql_insert);

		++index;

	});

	sql_insert += ") ";

	if (innerFailed)
	{
		failed = true;
		return;
	}

	sqlite3_stmt * stmt = nullptr;
	sqlite3_prepare_v2(db, sql_insert.c_str(), static_cast<int>(sql_insert.size()) + 1, &stmt, NULL);

	if (stmt == NULL)
	{
		// TODO: Log error
		failed = true;
		return;
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

		failed = true;
		return;
	}

	if (stmt)
	{
		sqlite3_finalize(stmt);
		stmt = nullptr;
	}

}
