#include "table.h"

void Table_basemost::ImportBlockBulk(sqlite3 * db, ImportDefinition const & import_definition, OutputModel * output_model_, InputModel * input_model_, DataBlock const & block,
	int const number_rows_in_block, long & linenum, long & badwritelines, long & goodwritelines, long & goodupdatelines, std::vector<std::string> & errors)
{

	std::lock_guard<std::recursive_mutex> data_lock(data_mutex);

	Executor executor(db);

	std::string sql_insert;

	sql_insert += "INSERT OR REPLACE INTO \"";
	sql_insert += GetTableName();
	sql_insert += "\" (";

	bool first = true;
	std::for_each(import_definition.output_schema.schema.cbegin(),
				  import_definition.output_schema.schema.cend(), [&import_definition, &sql_insert, &first](SchemaEntry const & schema_entry)
	{

		if (!first)
		{
			sql_insert += ", ";
		}

		first = false;

		sql_insert += "`";
		sql_insert += schema_entry.field_name;
		sql_insert += "`";

	});

	sql_insert += ") VALUES ";

	bool failed = false;
	bool first_row = true;
	long goodwritelines_current = 0;

	for (int row = 0; row < number_rows_in_block; ++row)
	{

		if (Importer::cancelled)
		{
			return;
		}

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

			if (failed)
			{
				return; // from lambda
			}

			if (!field_data)
			{
				boost::format msg("Bad row/col in block cache");
				errors.push_back(msg.str());
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

		++goodwritelines_current;
		++linenum;

	}

	//if (failed)
	//{
	//	return false;
	//}

	// BULK INSERT
	sqlite3_stmt * stmt = NULL;
	sqlite3_prepare_v2(db, sql_insert.c_str(), static_cast<int>(sql_insert.size()) + 1, &stmt, NULL);

	if (stmt == NULL)
	{
		std::string sql_error = sqlite3_errmsg(db);
		boost::format msg("Unable to prepare SQL query: %1% (%2%)");
		msg % sql_error.c_str() % sql_insert.c_str();
		errors.push_back(msg.str());
		return;
	}

	int step_result = 0;

	if ((step_result = sqlite3_step(stmt)) != SQLITE_DONE)
	{
		boost::format msg("Unable to execute SQL query: %1%");
		msg % sql_insert.c_str();
		errors.push_back(msg.str());
		if (stmt)
		{
			sqlite3_finalize(stmt);
			stmt = nullptr;
		}

		return;
	}

	goodwritelines += goodwritelines_current;

	executor.success();

	if (stmt)
	{
		sqlite3_finalize(stmt);
		stmt = nullptr;
	}

}

void Table_basemost::ImportBlockUpdate(sqlite3 * db, ImportDefinition const & import_definition, OutputModel * output_model_, InputModel * input_model_, DataBlock const & block,
	int const number_rows_in_block, long & linenum, long & badwritelines, long & goodwritelines, long & goodupdatelines, long & numlinesupdated, std::vector<std::string> & errors)
{

	std::lock_guard<std::recursive_mutex> data_lock(data_mutex);

	Executor executor(db);

	std::string errorMsg;
	for (int row = 0; row < number_rows_in_block; ++row)
	{

		if (Importer::cancelled)
		{
			return;
		}

		errorMsg.clear();
		bool failed = false;
		int changes = TryUpdateRow(block, row, failed, import_definition, db, errorMsg);

		if (failed || !errorMsg.empty())
		{
			boost::format msg("Unable to update line %1% during import: %2%");
			msg % boost::lexical_cast<std::string>(linenum + 1) % errorMsg.c_str();
			errorMsg = msg.str();
			errors.push_back(errorMsg);
			errorMsg.clear();
			++linenum;
			++badwritelines;
			continue; // try some other rows
		}

		if (changes == 0)
		{
			// Update did not affect any row.  Insert a new row
			TryInsertRow(block, row, failed, import_definition, db, errorMsg);
		}
		else
		{
			++goodupdatelines;
		}

		if (failed || !errorMsg.empty())
		{
			boost::format msg("Unable to insert line %1% during import: %2%");
			msg % boost::lexical_cast<std::string>(linenum + 1) % errorMsg.c_str();
			errorMsg = msg.str();
			errors.push_back(errorMsg);
			errorMsg.clear();
			++linenum;
			++badwritelines;
			continue; // try some other rows
		}
		else
		{
			++goodwritelines;
		}

		++linenum;

	}

	executor.success();

}

void Table_basemost::FieldDataAsSqlText(std::shared_ptr<BaseField> const & field_data, std::string & sql_insert, bool const no_quotes)
{

	FIELD_TYPE const field_type = field_data->GetType();

	if (IsFieldTypeInt32(field_type))
	{
		sql_insert += boost::lexical_cast<std::string>(field_data->GetInt32Ref());
	}
	else
	if (IsFieldTypeInt64(field_type))
	{
		sql_insert += boost::lexical_cast<std::string>(field_data->GetInt64Ref());
	}
	else
	if (IsFieldTypeFloat(field_type))
	{
		sql_insert += boost::lexical_cast<std::string>(field_data->GetDouble());
	}
	else
	if (IsFieldTypeString(field_type))
	{
		if (!no_quotes)
		{
			sql_insert += '\'';
		}
		sql_insert += Table_basemost::EscapeTicks(field_data->GetString());
		if (!no_quotes)
		{
			sql_insert += '\'';
		}
	}
	else
	{
		boost::format msg("Invalid data type when obtaining raw data for field!");
		throw NewGeneException() << newgene_error_description(msg.str());
	}

}

int Table_basemost::TryUpdateRow(DataBlock const & block, int row, bool & failed, ImportDefinition const & import_definition, sqlite3 * db, std::string & errorMsg)
{

	DataFields const & row_fields = block[row];
	int bind_index = 1;

	if (import_definition.stmt_update == nullptr)
	{

		std::string sql_update;

		sql_update += "UPDATE \"";
		sql_update += GetTableName();
		sql_update += "\" SET ";

		bool first = true;
		int index = 0;
		bool innerFailed = false;
		std::for_each(import_definition.output_schema.schema.cbegin(),
			import_definition.output_schema.schema.cend(), [&](SchemaEntry const & schema_entry)
		{

			if (innerFailed)
			{
				++index;
				return;
			}

			std::shared_ptr<BaseField> const & field_data = row_fields[index];

			if (!field_data)
			{
				boost::format msg("Unable to obtain BaseField in TryUpdateRow.");
				errorMsg = msg.str();
				++index;
				innerFailed = true;
				return;
			}

			if (!first)
			{
				sql_update += ", ";
			}

			first = false;

			sql_update += "`";
			sql_update += schema_entry.field_name;
			sql_update += "`";

			sql_update += " = ";

			sql_update += "?";

			++index;

		});

		if (innerFailed)
		{
			failed = true;
			return 0;
		}

		sql_update += " WHERE ";

		first = true;
		index = 0;
		innerFailed = false;
		std::for_each(import_definition.output_schema.schema.cbegin(),
			import_definition.output_schema.schema.cend(), [&](SchemaEntry const & schema_entry)
		{

			if (innerFailed)
			{
				++index;
				return;
			}

			if (schema_entry.IsPrimaryKey() || schema_entry.IsTimeRange())
			{

				std::shared_ptr<BaseField> const & field_data = row_fields[index];

				if (!field_data)
				{
					++index;
					innerFailed = true;
					return;
				}

				if (!first)
				{
					sql_update += " AND ";
				}

				first = false;

				sql_update += "`";
				sql_update += schema_entry.field_name;
				sql_update += "`";

				sql_update += " = ";

				sql_update += "?";

			}

			++index;

		});

		if (innerFailed)
		{
			failed = true;
			return 0;
		}

		sqlite3_stmt * stmt = nullptr;
		sqlite3_prepare_v2(db, sql_update.c_str(), static_cast<int>(sql_update.size()) + 1, &stmt, NULL);

		if (stmt == NULL)
		{
			std::string sql_error = sqlite3_errmsg(db);
			boost::format msg("Unable to prepare SQL query in TryUpdateRow: %1% (%2%)");
			msg % sql_error.c_str() % sql_update.c_str();
			errorMsg = msg.str();
			failed = true;
			return 0;
		}

		import_definition.stmt_update = stmt;

	}

	std::vector<std::pair<FIELD_TYPE, std::string>> values;
	std::vector<std::pair<FIELD_TYPE, std::string>> where_values;

	bool first = true;
	int index = 0;
	bool innerFailed = false;
	std::for_each(import_definition.output_schema.schema.cbegin(),
		import_definition.output_schema.schema.cend(), [&](SchemaEntry const & schema_entry)
	{

		if (innerFailed)
		{
			++index;
			return;
		}

		std::shared_ptr<BaseField> const & field_data = row_fields[index];

		if (!field_data)
		{
			boost::format msg("Unable to obtain BaseField in TryUpdateRow.");
			errorMsg = msg.str();
			++index;
			innerFailed = true;
			return;
		}

		values.push_back(std::make_pair(field_data->GetType(), std::string()));
		FieldDataAsSqlText(field_data, values.back().second, true);

		++index;

	});

	if (innerFailed)
	{
		failed = true;
		return 0;
	}

	first = true;
	index = 0;
	innerFailed = false;
	std::for_each(import_definition.output_schema.schema.cbegin(),
		import_definition.output_schema.schema.cend(), [&](SchemaEntry const & schema_entry)
	{

		if (innerFailed)
		{
			++index;
			return;
		}

		if (schema_entry.IsPrimaryKey() || schema_entry.IsTimeRange())
		{

			std::shared_ptr<BaseField> const & field_data = row_fields[index];

			if (!field_data)
			{
				++index;
				innerFailed = true;
				return;
			}

			where_values.push_back(std::make_pair(field_data->GetType(), std::string()));
			FieldDataAsSqlText(field_data, where_values.back().second, true);

		}

		++index;

	});

	if (innerFailed)
	{
		failed = true;
		return 0;
	}

	size_t numberValues = values.size();
	for (size_t n = 0; n < numberValues; ++n)
	{
		BindSqlField(import_definition.stmt_update, bind_index, values[n]);
	}

	size_t numberWheres = where_values.size();
	for (size_t n = 0; n < numberWheres; ++n)
	{
		BindSqlField(import_definition.stmt_update, bind_index, where_values[n]);
	}

	int step_result = 0;

	if ((step_result = sqlite3_step(import_definition.stmt_update)) != SQLITE_DONE)
	{
		if (import_definition.stmt_update)
		{
			sqlite3_finalize(import_definition.stmt_update);
			import_definition.stmt_update = nullptr;
		}

		boost::format msg("Unable to execute prepared statement in TryUpdateRow");
		errorMsg = msg.str();
		failed = true;
		return 0;
	}

	int number_changes = sqlite3_changes(db);

	sqlite3_clear_bindings(import_definition.stmt_update);
	sqlite3_reset(import_definition.stmt_update);

	return number_changes;

}

void Table_basemost::TryInsertRow(DataBlock const & block, int row, bool & failed, ImportDefinition const & import_definition, sqlite3 * db, std::string & errorMsg)
{

	int bind_index = 1;

	if (import_definition.stmt_insert == nullptr)
	{

		std::string sql_insert;

		DataFields const & row_fields = block[row];

		sql_insert += "INSERT INTO \"";
		sql_insert += GetTableName();
		sql_insert += "\" ( ";

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

			sql_insert += "`";
			sql_insert += schema_entry.field_name;
			sql_insert += "`";

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
				++index;
				return;
			}

			std::shared_ptr<BaseField> const & field_data = row_fields[index];

			if (!field_data)
			{
				boost::format msg("Unable to retrieve base field in TryInsertRow");
				errorMsg = msg.str();
				++index;
				innerFailed = true;
				return;
			}

			if (!first)
			{
				sql_insert += ", ";
			}

			first = false;

			sql_insert += "?";

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
			std::string sql_error = sqlite3_errmsg(db);
			boost::format msg("Unable to prepare SQL query in TryInsertRow: %1% (%2%)");
			msg % sql_error.c_str() % sql_insert.c_str();
			errorMsg = msg.str();
			failed = true;
			return;
		}

		import_definition.stmt_insert = stmt;

	}

	std::vector<std::pair<FIELD_TYPE, std::string>> values;

	std::string sql_insert;

	DataFields const & row_fields = block[row];

	bool first = true;
	int index = 0;
	bool innerFailed = false;
	std::for_each(import_definition.output_schema.schema.cbegin(),
				  import_definition.output_schema.schema.cend(), [&](SchemaEntry const & schema_entry)
	{

		if (innerFailed)
		{
			++index;
			return;
		}

		std::shared_ptr<BaseField> const & field_data = row_fields[index];

		if (!field_data)
		{
			boost::format msg("Unable to retrieve base field in TryInsertRow");
			errorMsg = msg.str();
			++index;
			innerFailed = true;
			return;
		}

		values.push_back(std::make_pair(field_data->GetType(), std::string()));
		FieldDataAsSqlText(field_data, values.back().second, true);

		++index;

	});

	if (innerFailed)
	{
		failed = true;
		return;
	}

	size_t numberFields = values.size();
	for (size_t n = 0; n < numberFields; ++n)
	{
		BindSqlField(import_definition.stmt_insert, bind_index, values[n]);
	}

	int step_result = 0;

	if ((step_result = sqlite3_step(import_definition.stmt_insert)) != SQLITE_DONE)
	{
		// TODO: Log error
		if (import_definition.stmt_insert)
		{
			sqlite3_finalize(import_definition.stmt_insert);
			import_definition.stmt_insert = nullptr;
		}

		boost::format msg("Unable to execute prepared query in TryInsertRow");
		errorMsg = msg.str();
		failed = true;
		return;
	}

	sqlite3_clear_bindings(import_definition.stmt_insert);
	sqlite3_reset(import_definition.stmt_insert);

}
