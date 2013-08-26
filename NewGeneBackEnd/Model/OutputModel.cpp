#include "OutputModel.h"
#include "..\Utilities\UUID.h"

#include <cstdint>

#ifndef Q_MOC_RUN
#	include <boost/lexical_cast.hpp>
#endif

void OutputModel::LoadTables()
{

	LoadDatabase();

	if (db != nullptr)
	{
		t_variables_selected_identifiers.Load(db, this, input_model.get());
		t_variables_selected_identifiers.Sort();

		t_kad_count.Load(db, this, input_model.get());
		t_kad_count.Sort();
	}

}

bool OutputModelImportTableFn(Model_basemost * model_, ImportDefinition & import_definition, Table_basemost * table_, DataBlock const & table_block, int const number_rows)
{
	try
	{
		if (table_->table_model_type == Table_basemost::TABLE_MODEL_TYPE__OUTPUT_MODEL)
		{
			OutputModel * output_model = dynamic_cast<OutputModel*>(model_);
			if (!output_model)
			{
				// Todo: log warning
				return false;
			}
			if (output_model->getDb() == nullptr)
			{
				// Todo: log warning
				return false;
			}
			table_->ImportBlock(output_model->getDb(), import_definition, output_model, &output_model->getInputModel(), table_block, number_rows);
		}
		else
		{
			// Todo: log warning
			return false;
		}
	}
	catch (std::bad_cast &)
	{
		// Todo: log warning
		return false;
	}
	return true;
}

std::string OutputModel::StripUUIDFromVariableName(std::string const & variable_name)
{
	std::string test_uuid = newUUID();
	size_t length_uuid = test_uuid.size();
	length_uuid += 1; // preceding underscore
	size_t length_variable_name = variable_name.size();
	int left_chars = (int)length_variable_name - (int)length_uuid;
	if (left_chars < 0)
	{
		return variable_name;
	}
	std::string stripped_variable_name = variable_name.substr(0, left_chars);
	return stripped_variable_name;
}

OutputModel::OutputGenerator::OutputGenerator(OutputModel & model_)
	: model(&model_)
	, stmt_result(nullptr)
	, executor(nullptr, false)
{
	debug_ordering = true;
}

OutputModel::OutputGenerator::~OutputGenerator()
{

	if (executor.transaction_begun)
	{
		EndTransaction();
	}

	std::for_each(primary_variable_group_column_sets.begin(), primary_variable_group_column_sets.end(), [](SqlAndColumnSets & sql_and_column_sets)
	{
		
		std::for_each(sql_and_column_sets.begin(), sql_and_column_sets.end(), [](SqlAndColumnSet & sql_and_column_set)
		{

			std::for_each(sql_and_column_set.first.begin(), sql_and_column_set.first.end(), [](SQLExecutor & sql_executor)
			{
				
				sql_executor.Empty();

			});

		});

	});

}

void OutputModel::OutputGenerator::GenerateOutput(DataChangeMessage & change_response)
{

	InputModel & input_model = model->getInputModel();
	Table_VARIABLES_SELECTED::UOA_To_Variables_Map the_map_ = model->t_variables_selected_identifiers.GetSelectedVariablesByUOA(model->getDb(), model, &input_model);
	the_map = &the_map_;

	Prepare();

	if (failed)
	{
		// failed
		return;
	}

	ObtainColumnInfoForRawDataTables();

	if (failed)
	{
		// failed
		return;
	}

	LoopThroughPrimaryVariableGroups();

	if (failed)
	{
		// failed
		return;
	}

	MergeHighLevelGroupResults();

	if (failed)
	{
		// failed
		return;
	}

}

void OutputModel::OutputGenerator::MergeHighLevelGroupResults()
{

}

void OutputModel::OutputGenerator::LoopThroughPrimaryVariableGroups()
{

	int primary_group_number = 1;
	std::for_each(primary_variable_groups_column_info.cbegin(), primary_variable_groups_column_info.cend(), [this, &primary_group_number](ColumnsInTempView const & primary_variable_group_raw_data_columns)
	{
		if (failed)
		{
			return;
		}
		primary_variable_group_column_sets.push_back(SqlAndColumnSets());
		SqlAndColumnSets & primary_group_column_sets = primary_variable_group_column_sets.back();
		ConstructFullOutputForSinglePrimaryGroup(primary_variable_group_raw_data_columns, primary_group_column_sets, primary_group_number);
		++primary_group_number;
	});

}

void OutputModel::OutputGenerator::ConstructFullOutputForSinglePrimaryGroup(ColumnsInTempView const & primary_variable_group_raw_data_columns, SqlAndColumnSets & sql_and_column_sets, int const primary_group_number)
{

	SqlAndColumnSet x_table_result = CreateInitialPrimaryXTable(primary_variable_group_raw_data_columns, primary_group_number);
	x_table_result.second.most_recent_sql_statement_executed__index = -1;
	ExecuteSQL(x_table_result);
	sql_and_column_sets.push_back(x_table_result);
	if (failed)
	{
		return;
	}

	SqlAndColumnSet xr_table_result = CreateInitialPrimaryXRTable(x_table_result.second, primary_group_number);
	xr_table_result.second.most_recent_sql_statement_executed__index = -1;
	ExecuteSQL(xr_table_result);
	sql_and_column_sets.push_back(xr_table_result);
	if (failed)
	{
		return;
	}

	for (int current_multiplicity = 2; current_multiplicity <= highest_multiplicity_primary_uoa; ++current_multiplicity)
	{

		x_table_result = CreatePrimaryXTable(primary_variable_group_raw_data_columns, xr_table_result.second, current_multiplicity, primary_group_number);
		x_table_result.second.most_recent_sql_statement_executed__index = -1;
		ExecuteSQL(x_table_result);
		sql_and_column_sets.push_back(x_table_result);
		if (failed)
		{
			return;
		}

		xr_table_result = CreateXRTable(x_table_result.second, current_multiplicity, primary_group_number, false, 0, current_multiplicity);
		sql_and_column_sets.push_back(xr_table_result);
		if (failed)
		{
			return;
		}

	}

	// Child tables
	int current_child_view_name_index = 1;
	int child_set_number = 1;
	std::for_each(secondary_variable_groups_column_info.cbegin(), secondary_variable_groups_column_info.cend(), [this, &current_child_view_name_index, &child_set_number, &x_table_result, &xr_table_result, &primary_group_number, &sql_and_column_sets](ColumnsInTempView const & child_variable_group_raw_data_columns)
	{
		
		WidgetInstanceIdentifier const & dmu_category_multiplicity_greater_than_1_for_child = child_uoas__which_multiplicity_is_greater_than_1[*(child_variable_group_raw_data_columns.variable_groups[0].identifier_parent)].first;
		int const the_child_multiplicity = child_uoas__which_multiplicity_is_greater_than_1[*(child_variable_group_raw_data_columns.variable_groups[0].identifier_parent)].second;
		for (int current_multiplicity = 1; current_multiplicity <= the_child_multiplicity; ++current_multiplicity)
		{
			x_table_result = CreateChildXTable(child_variable_group_raw_data_columns, xr_table_result.second, current_multiplicity, primary_group_number, child_set_number, current_child_view_name_index);
			x_table_result.second.most_recent_sql_statement_executed__index = -1;
			ExecuteSQL(x_table_result);
			sql_and_column_sets.push_back(x_table_result);
			if (failed)
			{
				return;
			}

			xr_table_result = CreateXRTable(x_table_result.second, current_multiplicity, primary_group_number, true, child_set_number, current_child_view_name_index);
			sql_and_column_sets.push_back(xr_table_result);
			if (failed)
			{
				return;
			}

			++current_child_view_name_index;
		}

		++child_set_number;

	});

	SqlAndColumnSet final_top_level_variable_group_result = RemoveDuplicates(xr_table_result.second, primary_group_number);

}

OutputModel::OutputGenerator::SqlAndColumnSet OutputModel::OutputGenerator::RemoveDuplicates(ColumnsInTempView const & final_xr_columns, int const primary_group_number)
{

	char c[256];

	SqlAndColumnSet result = std::make_pair(std::vector<SQLExecutor>(), ColumnsInTempView());
	std::vector<SQLExecutor> & sql_strings = result.first;
	ColumnsInTempView & result_columns = result.second;

	result_columns = final_xr_columns;
	result_columns.most_recent_sql_statement_executed__index = -1;

	std::string view_name = "F";
	view_name += itoa(primary_group_number, c, 10);
	result_columns.view_name_no_uuid = view_name;
	view_name += "_";
	view_name += newUUID(true);
	result_columns.view_name = view_name;
	result_columns.view_number = 1;
	result_columns.has_no_datetime_columns = false;

	std::string sql_create_final_primary_group_table;
	sql_create_final_primary_group_table += "CREATE TABLE ";
	sql_create_final_primary_group_table += result_columns.view_name;
	sql_create_final_primary_group_table += " AS SELECT * FROM ";
	sql_create_final_primary_group_table += final_xr_columns.view_name;

	bool first = true;

	if (highest_multiplicity_primary_uoa > 1)
	{

		// Determine how many columns there are corresponding to the DMU category with multiplicity greater than 1
		int number_primary_key_columns_in_dmu_category_with_multiplicity_greater_than_1 = 0;
		std::for_each(result_columns.columns_in_view.begin(), result_columns.columns_in_view.end(), [this, &number_primary_key_columns_in_dmu_category_with_multiplicity_greater_than_1](ColumnsInTempView::ColumnInTempView & view_column)
		{
			if (view_column.column_type == ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__PRIMARY)
			{
				if (view_column.is_within_inner_table_corresponding_to_top_level_uoa)
				{
					if (view_column.total_multiplicity__of_current_dmu_category__within_uoa_corresponding_to_the_current_inner_tables_variable_group == highest_multiplicity_primary_uoa)
					{
						if (view_column.current_multiplicity__corresponding_to__current_inner_table == 1)
						{
							++number_primary_key_columns_in_dmu_category_with_multiplicity_greater_than_1;
						}
					}
				}
			}
		});

		// Create the ORDER BY clause, taking the proper primary key columns that compose the DMU category with multiplicity greater than 1, in sequence
		for (int inner_dmu_multiplicity = 0; inner_dmu_multiplicity < number_primary_key_columns_in_dmu_category_with_multiplicity_greater_than_1; ++inner_dmu_multiplicity)
		{
			for (int outer_dmu_multiplicity = 1; outer_dmu_multiplicity <= highest_multiplicity_primary_uoa; ++outer_dmu_multiplicity)
			{
				std::for_each(result_columns.columns_in_view.begin(), result_columns.columns_in_view.end(), [this, &inner_dmu_multiplicity, &outer_dmu_multiplicity, &sql_create_final_primary_group_table, &first](ColumnsInTempView::ColumnInTempView & view_column)
				{
					if (view_column.column_type == ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__PRIMARY)
					{
						if (view_column.total_multiplicity__of_current_dmu_category__within_uoa_corresponding_to_the_current_inner_tables_variable_group == highest_multiplicity_primary_uoa)
						{
							if (view_column.primary_key_index__within_uoa_corresponding_to_variable_group_corresponding_to_current_inner_table__for_dmu_category == inner_dmu_multiplicity)
							{
								if (view_column.current_multiplicity__corresponding_to__current_inner_table == outer_dmu_multiplicity)
								{
									if (view_column.is_within_inner_table_corresponding_to_top_level_uoa)
									{
										if (!first)
										{
											sql_create_final_primary_group_table += ", ";
										}
										else
										{
											sql_create_final_primary_group_table += " ORDER BY ";
										}
										first = false;
										if (view_column.primary_key_should_be_treated_as_numeric)
										{
											sql_create_final_primary_group_table += "CAST (";
										}
										sql_create_final_primary_group_table += view_column.column_name_in_temporary_table;
										if (view_column.primary_key_should_be_treated_as_numeric)
										{
											sql_create_final_primary_group_table += " AS INTEGER)";
										}
									}
								}
							}
						}
					}
				});
			}
		}

	}

	// Now order by remaining primary key columns (with multiplicity 1)
	int current_column = 0;
	std::for_each(result_columns.columns_in_view.begin(), result_columns.columns_in_view.end(), [this, &sql_create_final_primary_group_table, &result_columns, &current_column, &inner_table_column_count, &first](ColumnsInTempView::ColumnInTempView & view_column)
	{
		if (current_column >= inner_table_column_count)
		{
			return;
		}
		// Determine how many columns there are corresponding to the DMU category
		int number_primary_key_columns_in_dmu_category_with_multiplicity_of_1 = 0;
		int column_count_ = 0;
		std::for_each(result_columns.columns_in_view.begin(), result_columns.columns_in_view.end(), [this, &view_column, &column_count_, &inner_table_column_count, &number_primary_key_columns_in_dmu_category_with_multiplicity_of_1, &sql_create_final_primary_group_table](ColumnsInTempView::ColumnInTempView & view_column_)
		{
			if (column_count_ >= inner_table_column_count)
			{
				return;
			}
			if (view_column_.column_type == ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__PRIMARY)
			{
				if (view_column_.primary_key_dmu_category_identifier.IsEqual(WidgetInstanceIdentifier::EQUALITY_CHECK_TYPE__STRING_CODE, view_column.primary_key_dmu_category_identifier))
				{
					if (view_column_.total_multiplicity__of_current_dmu_category__within_uoa_corresponding_to_the_current_inner_tables_variable_group == 1)
					{
						if (view_column.is_within_inner_table_corresponding_to_top_level_uoa)
						{
							++number_primary_key_columns_in_dmu_category_with_multiplicity_of_1;
						}
					}
				}
			}
			++column_count_;
		});

		if (view_column.column_type == ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__PRIMARY)
		{
			if (view_column.total_multiplicity__of_current_dmu_category__within_uoa_corresponding_to_the_current_inner_tables_variable_group == 1)
			{
				for (int inner_dmu_multiplicity = 0; inner_dmu_multiplicity < number_primary_key_columns_in_dmu_category_with_multiplicity_of_1; ++inner_dmu_multiplicity)
				{
					if (view_column.primary_key_index__within_uoa_corresponding_to_variable_group_corresponding_to_current_inner_table__for_dmu_category == inner_dmu_multiplicity)
					{
						if (view_column.is_within_inner_table_corresponding_to_top_level_uoa)
						{
							if (!first)
							{
								sql_create_final_primary_group_table += ", ";
							}
							else
							{
								sql_create_final_primary_group_table += " ORDER BY ";
							}
							first = false;
							if (view_column.primary_key_should_be_treated_as_numeric)
							{
								sql_create_final_primary_group_table += "CAST (";
							}
							sql_create_final_primary_group_table += view_column.column_name_in_temporary_table;
							if (view_column.primary_key_should_be_treated_as_numeric)
							{
								sql_create_final_primary_group_table += " AS INTEGER)";
							}
						}
					}
				}
			}
		}
		++current_column;
	});

	// Finally, order by the time range columns
	sql_create_final_primary_group_table += final_xr_columns.columns_in_view[final_xr_columns.columns_in_view.size()-2].column_name_in_temporary_table; // final merged datetime start column
	sql_create_final_primary_group_table += ", ";
	sql_create_final_primary_group_table += final_xr_columns.columns_in_view[final_xr_columns.columns_in_view.size()-1].column_name_in_temporary_table; // final merged datetime end column
	sql_strings.push_back(SQLExecutor(db, sql_create_final_primary_group_table));

}

bool OutputModel::OutputGenerator::StepData()
{

	if (stmt_result == nullptr)
	{
		return false;
	}

	if (failed)
	{
		return false;
	}

	int step_result = 0;
	if ((step_result = sqlite3_step(stmt_result)) != SQLITE_ROW)
	{

		if (step_result == SQLITE_DONE)
		{
			return false;
		}

		sql_error = sqlite3_errmsg(db);
		failed = true;
		return false;
	}

	return true;
}

void OutputModel::OutputGenerator::ObtainData(ColumnsInTempView & column_set)
{

	if (stmt_result)
	{
		sqlite3_finalize(stmt_result);
		stmt_result = nullptr;
	}

	std::string sql;
	sql += "SELECT * FROM ";
	sql += column_set.view_name;

	sqlite3_prepare_v2(db, sql.c_str(), sql.size() + 1, &stmt_result, NULL);
	if (stmt_result == NULL)
	{
		sql_error = sqlite3_errmsg(db);
		failed = true;
		return;
	}

}

void OutputModel::OutputGenerator::BeginNewTransaction()
{
	executor.BeginTransaction();
}

void OutputModel::OutputGenerator::EndTransaction()
{
	executor.success();
	executor.EndTransaction();
}

void OutputModel::OutputGenerator::ExecuteSQL(SqlAndColumnSet & sql_and_column_set)
{

	std::vector<SQLExecutor> & sql_commands = sql_and_column_set.first;

	if (sql_and_column_set.second.most_recent_sql_statement_executed__index >= (long)sql_commands.size() - 1)
	{
		// All SQL commands have been executed successfully
		return;
	}

	int number_executed = 0;
	std::for_each(sql_commands.begin() + (sql_and_column_set.second.most_recent_sql_statement_executed__index + 1), sql_commands.end(), [this, &number_executed, &sql_and_column_set](SQLExecutor & sql_executor)
	{
		
		if (failed)
		{
			return;
		}

		sql_executor.Execute();
		if (sql_executor.failed)
		{
			failed = true;
			return;
		}

		++sql_and_column_set.second.most_recent_sql_statement_executed__index;
		++number_executed;

	});

	// Clean preceding SQL
	while (number_executed > 0)
	{
		if (!sql_commands.back().statement_is_owned)
		{
			sql_commands.pop_back();
			--number_executed;
			--sql_and_column_set.second.most_recent_sql_statement_executed__index;
		}
		else
		{
			break;
		}
	}

}

OutputModel::OutputGenerator::SQLExecutor::SQLExecutor(sqlite3 * db_)
	: statement_type(DOES_NOT_RETURN_ROWS)
	, db(db_)
	, stmt(nullptr)
	, failed(false)
	, statement_is_owned(true)
	, statement_is_prepared(false)
{

}

OutputModel::OutputGenerator::SQLExecutor::SQLExecutor(sqlite3 * db_, std::string const & sql_)
	: sql(sql_)
	, statement_type(DOES_NOT_RETURN_ROWS)
	, db(db_)
	, stmt(nullptr)
	, failed(false)
	, statement_is_owned(true)
	, statement_is_prepared(false)
{

}

OutputModel::OutputGenerator::SQLExecutor::SQLExecutor(sqlite3 * db_, std::string const & sql_, std::vector<std::string> const & bound_parameter_strings_, std::vector<std::int64_t> const & bound_parameter_ints_, std::vector<SQLExecutor::WHICH_BINDING> & bound_parameter_which_binding_to_use_, sqlite3_stmt * stmt_to_use, bool const prepare_statement_if_null)
	: sql(sql_)
	, statement_type(DOES_NOT_RETURN_ROWS)
	, db(db_)
	, stmt(stmt_to_use)
	, failed(false)
	, statement_is_owned(false)
	, statement_is_prepared(stmt_to_use != nullptr)
	, bound_parameter_strings(bound_parameter_strings_)
	, bound_parameter_ints(bound_parameter_ints_)
	, bound_parameter_which_binding_to_use(bound_parameter_which_binding_to_use_)
{
	if (!failed && prepare_statement_if_null && stmt == nullptr)
	{
		if (!statement_is_prepared)
		{
			sqlite3_prepare_v2(db, sql.c_str(), sql.size() + 1, &stmt, NULL);
			if (stmt == NULL)
			{
				sql_error = sqlite3_errmsg(db);
				failed = true;
				return;
			}
			statement_is_owned = true;
			statement_is_prepared = true;
		}
	}
}

OutputModel::OutputGenerator::SQLExecutor::~SQLExecutor()
{
	Empty();
}

OutputModel::OutputGenerator::SQLExecutor::SQLExecutor(SQLExecutor const & rhs)
{
	Copy(rhs);
}

OutputModel::OutputGenerator::SQLExecutor & OutputModel::OutputGenerator::SQLExecutor::operator=(SQLExecutor const & rhs)
{
	if (&rhs != this)
	{
		Copy(rhs);
	}
	return *this;
}

OutputModel::OutputGenerator::SQLExecutor::SQLExecutor(SQLExecutor && rhs)
{
	CopyOwned(rhs);
}

OutputModel::OutputGenerator::SQLExecutor & OutputModel::OutputGenerator::SQLExecutor::operator=(SQLExecutor && rhs)
{
	CopyOwned(rhs);
	return *this;
}

void OutputModel::OutputGenerator::SQLExecutor::Copy(SQLExecutor const & rhs)
{
	// The following line, not the default, is why we explicitly define a copy constructor and assignment operator
	this->statement_is_owned = false;

	this->bound_parameter_ints = rhs.bound_parameter_ints;
	this->bound_parameter_strings = rhs.bound_parameter_strings;
	this->bound_parameter_which_binding_to_use = rhs.bound_parameter_which_binding_to_use;
	this->db = rhs.db;
	this->failed = rhs.failed;
	this->sql = rhs.sql;
	this->sql_error = sql_error;
	this->statement_is_prepared = rhs.statement_is_prepared;
	this->statement_type = rhs.statement_type;
	this->stmt = rhs.stmt;
}

void OutputModel::OutputGenerator::SQLExecutor::CopyOwned(SQLExecutor & rhs)
{
	// The following line DOES use the default here
	this->statement_is_owned = rhs.statement_is_owned;
	rhs.statement_is_owned = false;

	this->bound_parameter_ints = rhs.bound_parameter_ints;
	this->bound_parameter_strings = rhs.bound_parameter_strings;
	this->bound_parameter_which_binding_to_use = rhs.bound_parameter_which_binding_to_use;
	this->db = rhs.db;
	this->failed = rhs.failed;
	this->sql = rhs.sql;
	this->sql_error = sql_error;
	this->statement_is_prepared = rhs.statement_is_prepared;
	this->statement_type = rhs.statement_type;
	this->stmt = rhs.stmt;
}

void OutputModel::OutputGenerator::SQLExecutor::Empty(bool const empty_sql)
{

	if (statement_is_owned && stmt)
	{
		sqlite3_finalize(stmt);
		stmt = nullptr;
	}

	if (empty_sql)
	{
		sql.clear();
		bound_parameter_strings.clear();
		bound_parameter_ints.clear();
		bound_parameter_which_binding_to_use.clear();
	}

	if (statement_is_owned)
	{
		statement_is_prepared = false;
	}

	failed = false;

}

void OutputModel::OutputGenerator::SQLExecutor::Execute()
{

	if (failed)
	{
		return;
	}

	switch(statement_type)
	{

		case DOES_NOT_RETURN_ROWS:
			{

				if (statement_is_owned && !statement_is_prepared)
				{
					sqlite3_prepare_v2(db, sql.c_str(), sql.size() + 1, &stmt, NULL);
					if (stmt == NULL)
					{
						sql_error = sqlite3_errmsg(db);
						failed = true;
						return;
					}
					statement_is_prepared = true;
				}

			}
			break;

		case RETURNS_ROWS:
			{

				if (statement_is_owned && !statement_is_prepared)
				{
					sqlite3_prepare_v2(db, sql.c_str(), sql.size() + 1, &stmt, NULL);
					if (stmt == NULL)
					{
						sql_error = sqlite3_errmsg(db);
						failed = true;
						return;
					}
					statement_is_prepared = true;
				}

			}
			break;

	}

	if (stmt == nullptr)
	{
		return;
	}

	if (bound_parameter_which_binding_to_use.size() > 0)
	{

		sqlite3_reset(stmt); // OK even if the prepared statement has not been executed yet

		int current_string_index = 0;
		int current_int64_index = 0;
		int current_index = 1;
		std::for_each(bound_parameter_which_binding_to_use.cbegin(), bound_parameter_which_binding_to_use.cend(), [this, &current_string_index, &current_int64_index, &current_index](WHICH_BINDING const & which_binding)
		{
			switch (which_binding)
			{

				case STRING:
					{
						std::string & the_string = this->bound_parameter_strings[current_string_index];
						sqlite3_bind_text(this->stmt, current_index, the_string.c_str(), the_string.size(), SQLITE_STATIC);
						++current_string_index;
						++current_index;
					}
					break;

				case INT64:
					{
						std::int64_t the_int64 = this->bound_parameter_ints[current_int64_index];
						sqlite3_bind_int64(this->stmt, current_index, the_int64);
						++current_int64_index;
						++current_index;
					}
					break;

				case NULL_BINDING:
					{
						sqlite3_bind_null(this->stmt, current_index);
						++current_index;
					}
					break;

			}
		});

	}

	switch(statement_type)
	{

		case DOES_NOT_RETURN_ROWS:
			{

				int step_result = 0;
				if ((step_result = sqlite3_step(stmt)) != SQLITE_DONE)
				{
					sql_error = sqlite3_errmsg(db);
					failed = true;
					return;
				}

			}
			break;

		case RETURNS_ROWS:
			{

				// no-op

			}
			break;

	}

}

bool OutputModel::OutputGenerator::SQLExecutor::Step()
{

	if (stmt == nullptr)
	{
		return false;
	}

	if (!statement_is_prepared)
	{
		return false;
	}

	if (failed)
	{
		return false;
	}

	int step_result = 0;
	if ((step_result = sqlite3_step(stmt)) != SQLITE_ROW)
	{

		if (step_result == SQLITE_DONE)
		{
			return false;
		}

		sql_error = sqlite3_errmsg(db);
		failed = true;
		return false;
	}

	return true;

}

OutputModel::OutputGenerator::SqlAndColumnSet OutputModel::OutputGenerator::CreateInitialPrimaryXTable(ColumnsInTempView const & primary_variable_group_raw_data_columns, int const primary_group_number)
{
	char c[256];

	SqlAndColumnSet result = std::make_pair(std::vector<SQLExecutor>(), ColumnsInTempView());
	std::vector<SQLExecutor> & sql_strings = result.first;
	ColumnsInTempView & result_columns = result.second;

	result_columns = primary_variable_group_raw_data_columns;

	result_columns.view_number = 1;
	result_columns.has_no_datetime_columns = false;
	std::string view_name = "V";
	view_name += itoa(primary_group_number, c, 10);
	view_name += "_x";
	view_name += "1";
	result_columns.view_name_no_uuid = view_name;
	view_name += "_";
	view_name += newUUID(true);
	result_columns.view_name = view_name;

	WidgetInstanceIdentifiers const & variables_selected = (*the_map)[*primary_variable_group_raw_data_columns.variable_groups[0].identifier_parent][primary_variable_group_raw_data_columns.variable_groups[0]];

	result_columns.columns_in_view.clear();

	// Add the columns from the raw data table into this initial temporary table.
	std::for_each(primary_variable_group_raw_data_columns.columns_in_view.cbegin(), primary_variable_group_raw_data_columns.columns_in_view.cend(), [&result_columns, &variables_selected](ColumnsInTempView::ColumnInTempView const & column_in_view)
	{
		if (column_in_view.column_type == ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__DATETIMESTART || column_in_view.column_type == ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__DATETIMEEND)
		{
			return; // Enforce that datetime columns appear last.
		}
		bool match = true;
		if (column_in_view.column_type == ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__SECONDARY)
		{
			match = false;
			std::for_each(variables_selected.cbegin(), variables_selected.cend(), [&column_in_view, &match](WidgetInstanceIdentifier const & variable_selected)
			{
				if (boost::iequals(column_in_view.column_name_in_original_data_table, *variable_selected.code))
				{
					match = true;
				}
			});
		}
		if (match)
		{
			result_columns.columns_in_view.push_back(column_in_view);
		}
	});
	std::for_each(primary_variable_group_raw_data_columns.columns_in_view.cbegin(), primary_variable_group_raw_data_columns.columns_in_view.cend(), [&result_columns](ColumnsInTempView::ColumnInTempView const & column_in_view)
	{
		// Now do the datetime_start column
		if (column_in_view.column_type == ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__DATETIMESTART)
		{
			result_columns.columns_in_view.push_back(column_in_view);
		}
	});
	std::for_each(primary_variable_group_raw_data_columns.columns_in_view.cbegin(), primary_variable_group_raw_data_columns.columns_in_view.cend(), [&result_columns](ColumnsInTempView::ColumnInTempView const & column_in_view)
	{
		// Now do the datetime_end column
		if (column_in_view.column_type == ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__DATETIMEEND)
		{
			result_columns.columns_in_view.push_back(column_in_view);
		}
	});

	WidgetInstanceIdentifier variable_group_saved;
	WidgetInstanceIdentifier uoa_saved;

	// Make column names for this temporary table unique (not the same as the column names from the previous table that is being copied).
	// Also, set the primary UOA flag.
	bool first = true;
	std::for_each(result_columns.columns_in_view.begin(), result_columns.columns_in_view.end(), [&first, &variable_group_saved, &uoa_saved](ColumnsInTempView::ColumnInTempView & new_column)
	{
		new_column.column_name_in_temporary_table = new_column.column_name_in_temporary_table_no_uuid;
		new_column.column_name_in_temporary_table += "_";
		new_column.column_name_in_temporary_table += newUUID(true);

		new_column.inner_table_set_number = 0;
		new_column.is_within_inner_table_corresponding_to_top_level_uoa = true;

		if (first)
		{
			first = false;
			variable_group_saved = new_column.variable_group_associated_with_current_inner_table;
			uoa_saved = new_column.uoa_associated_with_variable_group_associated_with_current_inner_table;
		}
	});

	sql_strings.push_back(SQLExecutor(db));
	std::string & sql_string = sql_strings.back().sql;

	sql_string = "CREATE TABLE ";
	sql_string += result_columns.view_name;
	sql_string += " AS SELECT ";
	first = true;
	std::for_each(result_columns.columns_in_view.begin(), result_columns.columns_in_view.end(), [&sql_string, &first](ColumnsInTempView::ColumnInTempView & new_column)
	{
		if (new_column.column_type == ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__DATETIMESTART || new_column.column_type == ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__DATETIMEEND)
		{
			return; // Enforce that datetime columns appear last.
		}
		if (!first)
		{
			sql_string += ", ";
		}
		first = false;
		sql_string += new_column.column_name_in_temporary_table_no_uuid; // This is the original column name
		sql_string += " AS ";
		sql_string += new_column.column_name_in_temporary_table;
	});
	std::for_each(result_columns.columns_in_view.begin(), result_columns.columns_in_view.end(), [&sql_string, &first](ColumnsInTempView::ColumnInTempView & new_column)
	{
		// Now do the datetime_start column
		if (new_column.column_type == ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__DATETIMESTART)
		{
			if (!first)
			{
				sql_string += ", ";
			}
			first = false;
			sql_string += new_column.column_name_in_temporary_table_no_uuid; // This is the original column name
			sql_string += " AS ";
			sql_string += new_column.column_name_in_temporary_table;
		}
	});
	std::for_each(result_columns.columns_in_view.begin(), result_columns.columns_in_view.end(), [&sql_string, &first](ColumnsInTempView::ColumnInTempView & new_column)
	{
		// Now do the datetime_end column
		if (new_column.column_type == ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__DATETIMEEND)
		{
			if (!first)
			{
				sql_string += ", ";
			}
			first = false;
			sql_string += new_column.column_name_in_temporary_table_no_uuid; // This is the original column name
			sql_string += " AS ";
			sql_string += new_column.column_name_in_temporary_table;
		}
	});
	sql_string += " FROM ";
	sql_string += result_columns.original_table_names[0];

	// Add the ORDER BY column/s
	if (debug_ordering)
	{

		bool first = true;

		if (highest_multiplicity_primary_uoa > 1)
		{

			// Determine how many columns there are corresponding to the DMU category with multiplicity greater than 1
			int number_primary_key_columns_in_dmu_category_with_multiplicity_greater_than_1 = 0;
			std::for_each(result_columns.columns_in_view.begin(), result_columns.columns_in_view.end(), [this, &number_primary_key_columns_in_dmu_category_with_multiplicity_greater_than_1, &sql_string](ColumnsInTempView::ColumnInTempView & view_column)
			{
				if (view_column.column_type == ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__PRIMARY)
				{
					if (view_column.is_within_inner_table_corresponding_to_top_level_uoa)
					{
						if (view_column.total_multiplicity__of_current_dmu_category__within_uoa_corresponding_to_the_current_inner_tables_variable_group == highest_multiplicity_primary_uoa)
						{
							++number_primary_key_columns_in_dmu_category_with_multiplicity_greater_than_1;
						}
					}
				}
			});

			// Create the ORDER BY clause, taking the proper primary key columns that compose the DMU category with multiplicity greater than 1, in sequence
			for (int inner_dmu_multiplicity = 0; inner_dmu_multiplicity < number_primary_key_columns_in_dmu_category_with_multiplicity_greater_than_1; ++inner_dmu_multiplicity)
			{
				std::for_each(result_columns.columns_in_view.begin(), result_columns.columns_in_view.end(), [this, &inner_dmu_multiplicity, &sql_string, &first](ColumnsInTempView::ColumnInTempView & view_column)
				{
					if (view_column.column_type == ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__PRIMARY)
					{
						if (view_column.total_multiplicity__of_current_dmu_category__within_uoa_corresponding_to_the_current_inner_tables_variable_group == highest_multiplicity_primary_uoa)
						{
							if (view_column.primary_key_index__within_uoa_corresponding_to_variable_group_corresponding_to_current_inner_table__for_dmu_category == inner_dmu_multiplicity)
							{
								if (view_column.is_within_inner_table_corresponding_to_top_level_uoa)
								{
									if (!first)
									{
										sql_string += ", ";
									}
									else
									{
										sql_string += " ORDER BY ";
									}
									first = false;
									if (view_column.primary_key_should_be_treated_as_numeric)
									{
										sql_string += "CAST (";
									}
									sql_string += view_column.column_name_in_temporary_table;
									if (view_column.primary_key_should_be_treated_as_numeric)
									{
										sql_string += " AS INTEGER)";
									}
								}
							}
						}
					}
				});
			}

		}
	
		// Now order by remaining primary key columns (with multiplicity 1)
		std::for_each(result_columns.columns_in_view.begin(), result_columns.columns_in_view.end(), [this, &sql_string, &result_columns, &first](ColumnsInTempView::ColumnInTempView & view_column)
		{
			// Determine how many columns there are corresponding to the DMU category
			int number_primary_key_columns_in_dmu_category_with_multiplicity_of_1 = 0;
			std::for_each(result_columns.columns_in_view.begin(), result_columns.columns_in_view.end(), [this, &view_column, &number_primary_key_columns_in_dmu_category_with_multiplicity_of_1, &sql_string](ColumnsInTempView::ColumnInTempView & view_column_)
			{
				if (view_column_.column_type == ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__PRIMARY)
				{
					if (view_column_.primary_key_dmu_category_identifier.IsEqual(WidgetInstanceIdentifier::EQUALITY_CHECK_TYPE__STRING_CODE, view_column.primary_key_dmu_category_identifier))
					{
						if (view_column_.total_multiplicity__of_current_dmu_category__within_uoa_corresponding_to_the_current_inner_tables_variable_group == 1)
						{
							if (view_column.is_within_inner_table_corresponding_to_top_level_uoa)
							{
								++number_primary_key_columns_in_dmu_category_with_multiplicity_of_1;
							}
						}
					}
				}
			});

			if (view_column.column_type == ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__PRIMARY)
			{
				if (view_column.total_multiplicity__of_current_dmu_category__within_uoa_corresponding_to_the_current_inner_tables_variable_group == 1)
				{
					for (int inner_dmu_multiplicity = 0; inner_dmu_multiplicity < number_primary_key_columns_in_dmu_category_with_multiplicity_of_1; ++inner_dmu_multiplicity)
					{
						if (view_column.primary_key_index__within_uoa_corresponding_to_variable_group_corresponding_to_current_inner_table__for_dmu_category == inner_dmu_multiplicity)
						{
							if (view_column.is_within_inner_table_corresponding_to_top_level_uoa)
							{
								if (!first)
								{
									sql_string += ", ";
								}
								else
								{
									sql_string += " ORDER BY ";
								}
								first = false;
								if (view_column.primary_key_should_be_treated_as_numeric)
								{
									sql_string += "CAST (";
								}
								sql_string += view_column.column_name_in_temporary_table;
								if (view_column.primary_key_should_be_treated_as_numeric)
								{
									sql_string += " AS INTEGER)";
								}
							}
						}
					}
				}
			}
		});

	}

	// SQL to add the datetime columns, if they are not present in the raw data table (filled with 0)
	if (primary_variable_group_raw_data_columns.has_no_datetime_columns_originally)
	{
		std::string datetime_start_col_name_no_uuid = "DATETIME_ROW_START";
		std::string datetime_start_col_name = datetime_start_col_name_no_uuid;
		datetime_start_col_name += "_";
		datetime_start_col_name += newUUID(true);

		std::string alter_string;
		alter_string += "ALTER TABLE ";
		alter_string += result_columns.view_name;
		alter_string += " ADD COLUMN ";
		alter_string += datetime_start_col_name;
		alter_string += " INTEGER DEFAULT 0";
		sql_strings.push_back(SQLExecutor(db, alter_string));

		result_columns.columns_in_view.push_back(ColumnsInTempView::ColumnInTempView());
		ColumnsInTempView::ColumnInTempView & datetime_start_column = result_columns.columns_in_view.back();
		datetime_start_column.column_name_in_temporary_table = datetime_start_col_name;
		datetime_start_column.column_name_in_temporary_table_no_uuid = datetime_start_col_name_no_uuid;
		datetime_start_column.column_type = ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__DATETIMESTART_INTERNAL;
		datetime_start_column.variable_group_associated_with_current_inner_table = variable_group_saved;
		datetime_start_column.uoa_associated_with_variable_group_associated_with_current_inner_table = uoa_saved;
		datetime_start_column.column_name_in_original_data_table = "";
		datetime_start_column.inner_table_set_number = 0;
		datetime_start_column.is_within_inner_table_corresponding_to_top_level_uoa = true;

		std::string datetime_end_col_name_no_uuid = "DATETIME_ROW_END";
		std::string datetime_end_col_name = datetime_end_col_name_no_uuid;
		datetime_end_col_name += "_";
		datetime_end_col_name += newUUID(true);

		alter_string.clear();
		alter_string += "ALTER TABLE ";
		alter_string += result_columns.view_name;
		alter_string += " ADD COLUMN ";
		alter_string += datetime_end_col_name;
		alter_string += " INTEGER DEFAULT 0";
		sql_strings.push_back(SQLExecutor(db, alter_string));

		result_columns.columns_in_view.push_back(ColumnsInTempView::ColumnInTempView());
		ColumnsInTempView::ColumnInTempView & datetime_end_column = result_columns.columns_in_view.back();
		datetime_end_column.column_name_in_temporary_table = datetime_end_col_name;
		datetime_end_column.column_name_in_temporary_table_no_uuid = datetime_end_col_name_no_uuid;
		datetime_end_column.column_type = ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__DATETIMEEND_INTERNAL;
		datetime_end_column.variable_group_associated_with_current_inner_table = variable_group_saved;
		datetime_end_column.uoa_associated_with_variable_group_associated_with_current_inner_table = uoa_saved;
		datetime_end_column.column_name_in_original_data_table = "";
		datetime_end_column.inner_table_set_number = 0;
		datetime_end_column.is_within_inner_table_corresponding_to_top_level_uoa = true;
	}

	return result;
}

OutputModel::OutputGenerator::SqlAndColumnSet OutputModel::OutputGenerator::CreateInitialPrimaryXRTable(ColumnsInTempView const & primary_variable_group_x1_columns, int const primary_group_number)
{

	char c[256];

	SqlAndColumnSet result = std::make_pair(std::vector<SQLExecutor>(), ColumnsInTempView());
	std::vector<SQLExecutor> & sql_strings = result.first;
	ColumnsInTempView & result_columns = result.second;

	result_columns = primary_variable_group_x1_columns;

	std::string view_name = "V";
	view_name += itoa(primary_group_number, c, 10);
	view_name += "_xr";
	view_name += "1";
	result_columns.view_name_no_uuid = view_name;
	view_name += "_";
	view_name += newUUID(true);
	result_columns.view_name = view_name;

	WidgetInstanceIdentifier variable_group;
	WidgetInstanceIdentifier uoa;

	// x1 table is guaranteed to have datetime columns
	int x1_datetime_start_column_index = -1;
	int x1_datetime_end_column_index = -1;

	bool first = true;
	int column_index = 0;
	std::for_each(result_columns.columns_in_view.begin(), result_columns.columns_in_view.end(), [&variable_group, &uoa, &x1_datetime_start_column_index, &x1_datetime_end_column_index, &column_index, &first](ColumnsInTempView::ColumnInTempView & new_column)
	{
		new_column.column_name_in_temporary_table = new_column.column_name_in_temporary_table_no_uuid;
		new_column.column_name_in_temporary_table += "_";
		new_column.column_name_in_temporary_table += newUUID(true);
		if (first)
		{
			first = false;
			variable_group = new_column.variable_group_associated_with_current_inner_table;
			uoa = new_column.uoa_associated_with_variable_group_associated_with_current_inner_table;
		}
		if (new_column.column_type == ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__DATETIMESTART || new_column.column_type == ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__DATETIMESTART_INTERNAL)
		{
			x1_datetime_start_column_index = column_index;
		}
		if (new_column.column_type == ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__DATETIMEEND || new_column.column_type == ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__DATETIMEEND_INTERNAL)
		{
			x1_datetime_end_column_index = column_index;
		}
		++column_index;
	});

	sql_strings.push_back(SQLExecutor(db));
	std::string & sql_string = sql_strings.back().sql;

	sql_string = "CREATE TABLE ";
	sql_string += result_columns.view_name;
	sql_string += " AS SELECT ";
	first = true;
	int the_index = 0;
	std::for_each(result_columns.columns_in_view.begin(), result_columns.columns_in_view.end(), [&sql_string, &the_index, &primary_variable_group_x1_columns, &first](ColumnsInTempView::ColumnInTempView & new_column)
	{
		if (!first)
		{
			sql_string += ", ";
		}
		first = false;
		sql_string += primary_variable_group_x1_columns.columns_in_view[the_index].column_name_in_temporary_table; // This is the original column name
		sql_string += " AS ";
		sql_string += new_column.column_name_in_temporary_table;
		++the_index;
	});
	sql_string += " FROM ";
	sql_string += primary_variable_group_x1_columns.view_name;


	// Add the "merged" time range columns

	std::string datetime_start_col_name_no_uuid = "DATETIME_ROW_START_MERGED";
	std::string datetime_start_col_name = datetime_start_col_name_no_uuid;
	datetime_start_col_name += "_";
	datetime_start_col_name += newUUID(true);

	std::string alter_string;
	alter_string += "ALTER TABLE ";
	alter_string += result_columns.view_name;
	alter_string += " ADD COLUMN ";
	alter_string += datetime_start_col_name;
	alter_string += " INTEGER DEFAULT 0";
	sql_strings.push_back(SQLExecutor(db, alter_string));

	result_columns.columns_in_view.push_back(ColumnsInTempView::ColumnInTempView());
	ColumnsInTempView::ColumnInTempView & datetime_start_column = result_columns.columns_in_view.back();
	datetime_start_column.column_name_in_temporary_table = datetime_start_col_name;
	datetime_start_column.column_name_in_temporary_table_no_uuid = datetime_start_col_name_no_uuid;
	datetime_start_column.column_type = ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__DATETIMESTART_MERGED;
	datetime_start_column.variable_group_associated_with_current_inner_table = variable_group;
	datetime_start_column.uoa_associated_with_variable_group_associated_with_current_inner_table = uoa;
	datetime_start_column.column_name_in_original_data_table = "";
	datetime_start_column.inner_table_set_number = 0;
	datetime_start_column.is_within_inner_table_corresponding_to_top_level_uoa = true;

	std::string datetime_end_col_name_no_uuid = "DATETIME_ROW_END_MERGED";
	std::string datetime_end_col_name = datetime_end_col_name_no_uuid;
	datetime_end_col_name += "_";
	datetime_end_col_name += newUUID(true);

	alter_string.clear();
	alter_string += "ALTER TABLE ";
	alter_string += result_columns.view_name;
	alter_string += " ADD COLUMN ";
	alter_string += datetime_end_col_name;
	alter_string += " INTEGER DEFAULT 0";
	sql_strings.push_back(SQLExecutor(db, alter_string));

	result_columns.columns_in_view.push_back(ColumnsInTempView::ColumnInTempView());
	ColumnsInTempView::ColumnInTempView & datetime_end_column = result_columns.columns_in_view.back();
	datetime_end_column.column_name_in_temporary_table = datetime_end_col_name;
	datetime_end_column.column_name_in_temporary_table_no_uuid = datetime_end_col_name_no_uuid;
	datetime_end_column.column_type = ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__DATETIMEEND_MERGED;
	datetime_end_column.variable_group_associated_with_current_inner_table = variable_group;
	datetime_end_column.uoa_associated_with_variable_group_associated_with_current_inner_table = uoa;
	datetime_end_column.column_name_in_original_data_table = "";
	datetime_end_column.inner_table_set_number = 0;
	datetime_end_column.is_within_inner_table_corresponding_to_top_level_uoa = true;


	// Set the "merged" time range columns to be equal to the original time range columns
	std::string sql_time_range;
	sql_time_range += "UPDATE OR FAIL ";
	sql_time_range += result_columns.view_name;
	sql_time_range += " SET ";
	sql_time_range += datetime_start_col_name;
	sql_time_range += " = ";
	sql_time_range += result_columns.columns_in_view[x1_datetime_start_column_index].column_name_in_temporary_table;
	sql_time_range += ", ";
	sql_time_range += datetime_end_col_name;
	sql_time_range += " = ";
	sql_time_range += result_columns.columns_in_view[x1_datetime_end_column_index].column_name_in_temporary_table;
	sql_strings.push_back(SQLExecutor(db, sql_time_range));

	return result;

}

OutputModel::OutputGenerator::SqlAndColumnSet OutputModel::OutputGenerator::CreatePrimaryXTable(ColumnsInTempView const & primary_variable_group_raw_data_columns, ColumnsInTempView const & previous_xr_columns, int const current_multiplicity, int const primary_group_number)
{

	char c[256];

	SqlAndColumnSet result = std::make_pair(std::vector<SQLExecutor>(), ColumnsInTempView());
	std::vector<SQLExecutor> & sql_strings = result.first;
	ColumnsInTempView & result_columns = result.second;

	result_columns = previous_xr_columns;

	std::string view_name = "V";
	view_name += itoa(primary_group_number, c, 10);
	view_name += "_x";
	view_name += itoa(current_multiplicity, c, 10);
	result_columns.view_name_no_uuid = view_name;
	view_name += "_";
	view_name += newUUID(true);
	result_columns.view_name = view_name;
	result_columns.view_number = current_multiplicity;
	result_columns.has_no_datetime_columns = false;

	int first_full_table_column_count = 0;
	int inner_table_column_count = 0;
	int second_table_column_count = 0;

	std::vector<std::string> previous_column_names_first_table;

	WidgetInstanceIdentifier variable_group;
	WidgetInstanceIdentifier uoa;

	// These columns are from the previous XR temporary table, which is guaranteed to have all columns in place, including datetime columns.
	// Further, the "current_multiplicity" of these columns is guaranteed to be correct.
	bool first = true;
	bool in_first_inner_table = true;
	bool reached_first_datetime_start_merged_column = false;
	bool reached_first_datetime_end_merged_column = false;
	std::for_each(result_columns.columns_in_view.begin(), result_columns.columns_in_view.end(), [&first_full_table_column_count, &inner_table_column_count, &reached_first_datetime_start_merged_column, &reached_first_datetime_end_merged_column, &in_first_inner_table, &previous_column_names_first_table, &variable_group, &uoa, &first](ColumnsInTempView::ColumnInTempView & new_column)
	{
		previous_column_names_first_table.push_back(new_column.column_name_in_temporary_table);
		new_column.column_name_in_temporary_table = new_column.column_name_in_temporary_table_no_uuid;
		new_column.column_name_in_temporary_table += "_";
		new_column.column_name_in_temporary_table += newUUID(true);
		++first_full_table_column_count;
		if (new_column.column_type == ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__DATETIMESTART_MERGED)
		{
			reached_first_datetime_start_merged_column = true;
		}
		if (new_column.column_type == ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__DATETIMEEND_MERGED)
		{
			reached_first_datetime_end_merged_column = true;
		}
		if (in_first_inner_table)
		{
			++inner_table_column_count;
		}
		if (reached_first_datetime_start_merged_column && reached_first_datetime_end_merged_column)
		{
			in_first_inner_table = false;
		}
		if (first)
		{
			first = false;
			variable_group = new_column.variable_group_associated_with_current_inner_table;
			uoa = new_column.uoa_associated_with_variable_group_associated_with_current_inner_table;
		}
	});

	WidgetInstanceIdentifiers const & variables_selected = (*the_map)[*primary_variable_group_raw_data_columns.variable_groups[0].identifier_parent][primary_variable_group_raw_data_columns.variable_groups[0]];

	// These columns are from the new table (the raw data table) being added.
	// Make column names for this temporary table unique (not the same as the column names from the previous table that is being copied)
	// These columns are from the original raw data table, which may or may not have datetime columns.
	// Further, the "current_multiplicity" of these columns is 1, and must be updated.
	std::for_each(primary_variable_group_raw_data_columns.columns_in_view.cbegin(), primary_variable_group_raw_data_columns.columns_in_view.cend(), [&result_columns, &variables_selected, &second_table_column_count, &current_multiplicity](ColumnsInTempView::ColumnInTempView const & new_column_)
	{
		if (new_column_.column_type == ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__DATETIMESTART || new_column_.column_type == ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__DATETIMESTART_INTERNAL || new_column_.column_type == ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__DATETIMEEND || new_column_.column_type == ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__DATETIMEEND_INTERNAL)
		{
			return; // Add these columns last
		}

		bool match = true;
		if (new_column_.column_type == ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__SECONDARY)
		{
			match = false;
			std::for_each(variables_selected.cbegin(), variables_selected.cend(), [&new_column_, &match](WidgetInstanceIdentifier const & variable_selected)
			{
				if (boost::iequals(new_column_.column_name_in_original_data_table, *variable_selected.code))
				{
					match = true;
				}
			});
		}

		if (match)
		{
			result_columns.columns_in_view.push_back(new_column_);
			ColumnsInTempView::ColumnInTempView & new_column = result_columns.columns_in_view.back();
			new_column.column_name_in_temporary_table = new_column.column_name_in_temporary_table_no_uuid;
			new_column.column_name_in_temporary_table += "_";
			new_column.column_name_in_temporary_table += newUUID(true);
			new_column.inner_table_set_number = 0;
			new_column.is_within_inner_table_corresponding_to_top_level_uoa = true;
			if (new_column.column_type == ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__PRIMARY)
			{
				if (new_column.total_multiplicity__of_current_dmu_category__within_uoa_corresponding_to_the_current_inner_tables_variable_group > 1)
				{
					new_column.current_multiplicity__corresponding_to__current_inner_table = current_multiplicity; // update current multiplicity
					new_column.primary_key_index_within_total_kad_for_dmu_category = new_column.primary_key_index__within_uoa_corresponding_to_variable_group_corresponding_to_current_inner_table__for_dmu_category
						+ (current_multiplicity - 1) * new_column.total_k_count__within_uoa_corresponding_to_current_variable_group__for_current_dmu_category;
				}
			}
			++second_table_column_count;
		}
	});
	// Datetime columns, if present
	std::for_each(primary_variable_group_raw_data_columns.columns_in_view.cbegin(), primary_variable_group_raw_data_columns.columns_in_view.cend(), [&result_columns, &second_table_column_count](ColumnsInTempView::ColumnInTempView const & new_column_)
	{
		if (new_column_.column_type == ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__DATETIMESTART || new_column_.column_type == ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__DATETIMESTART_INTERNAL)
		{
			result_columns.columns_in_view.push_back(new_column_);
			ColumnsInTempView::ColumnInTempView & new_column = result_columns.columns_in_view.back();
			new_column.column_name_in_temporary_table = new_column.column_name_in_temporary_table_no_uuid;
			new_column.column_name_in_temporary_table += "_";
			new_column.column_name_in_temporary_table += newUUID(true);
			new_column.inner_table_set_number = 0;
			new_column.is_within_inner_table_corresponding_to_top_level_uoa = true;
			++second_table_column_count;
		}
	});
	std::for_each(primary_variable_group_raw_data_columns.columns_in_view.cbegin(), primary_variable_group_raw_data_columns.columns_in_view.cend(), [&result_columns, &second_table_column_count](ColumnsInTempView::ColumnInTempView const & new_column_)
	{
		if (new_column_.column_type == ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__DATETIMEEND || new_column_.column_type == ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__DATETIMEEND_INTERNAL)
		{
			result_columns.columns_in_view.push_back(new_column_);
			ColumnsInTempView::ColumnInTempView & new_column = result_columns.columns_in_view.back();
			new_column.column_name_in_temporary_table = new_column.column_name_in_temporary_table_no_uuid;
			new_column.column_name_in_temporary_table += "_";
			new_column.column_name_in_temporary_table += newUUID(true);
			new_column.inner_table_set_number = 0;
			new_column.is_within_inner_table_corresponding_to_top_level_uoa = true;
			++second_table_column_count;
		}
	});

	sql_strings.push_back(SQLExecutor(db));
	std::string & sql_string = sql_strings.back().sql;

	sql_string = "CREATE TABLE ";
	sql_string += result_columns.view_name;
	sql_string += " AS SELECT ";
	first = true;
	int column_count = 0;
	std::for_each(result_columns.columns_in_view.begin(), result_columns.columns_in_view.end(), [&sql_string, &first, &column_count, &first_full_table_column_count, &second_table_column_count, &previous_column_names_first_table](ColumnsInTempView::ColumnInTempView & new_column)
	{
		if (!first)
		{
			sql_string += ", ";
		}
		first = false;
		if (column_count < first_full_table_column_count)
		{
			sql_string += "t1.";
			sql_string += previous_column_names_first_table[column_count];
		}
		else
		{
			sql_string += "t2.";
			sql_string += new_column.column_name_in_temporary_table_no_uuid; // This is the original column name
		}
		sql_string += " AS ";
		sql_string += new_column.column_name_in_temporary_table;
		++column_count;
	});
	sql_string += " FROM ";
	sql_string += previous_xr_columns.view_name;
	sql_string += " t1 JOIN ";
	sql_string += primary_variable_group_raw_data_columns.original_table_names[0];
	sql_string += " t2 ON ";
	bool and_ = false;
	std::for_each(sequence.primary_key_sequence_info.cbegin(), sequence.primary_key_sequence_info.cend(), [&sql_string, &variable_group, &result_columns, &first_full_table_column_count, &inner_table_column_count, &second_table_column_count, &previous_column_names_first_table, &and_](PrimaryKeySequence::PrimaryKeySequenceEntry const & primary_key)
	{
		std::for_each(primary_key.variable_group_info_for_primary_keys.cbegin(), primary_key.variable_group_info_for_primary_keys.cend(), [&sql_string, &variable_group, &primary_key, &result_columns, &first_full_table_column_count, &inner_table_column_count, &second_table_column_count, &previous_column_names_first_table, &and_](PrimaryKeySequence::VariableGroup_PrimaryKey_Info const & primary_key_info)
		{
			if (primary_key_info.vg_identifier.IsEqual(WidgetInstanceIdentifier::EQUALITY_CHECK_TYPE__STRING_CODE, variable_group))
			{
				if (primary_key_info.total_multiplicity == 1)
				{
					// Only join on primary keys whose total multiplicity is 1
					int column_count = 0;
					std::for_each(result_columns.columns_in_view.cbegin(), result_columns.columns_in_view.cend(), [&sql_string, &first_full_table_column_count, &inner_table_column_count, &second_table_column_count, &column_count, &previous_column_names_first_table, &primary_key, &and_](ColumnsInTempView::ColumnInTempView const & new_column)
					{
						if (column_count < inner_table_column_count)
						{
							if (new_column.primary_key_dmu_category_identifier.IsEqual(WidgetInstanceIdentifier::EQUALITY_CHECK_TYPE__STRING_CODE, primary_key.dmu_category))
							{
								// there is only one set of primary keys for this DMU category,
								// so the following "if" statement only matches once
								if (new_column.primary_key_index_within_primary_uoa_for_dmu_category == primary_key.sequence_number_within_dmu_category_primary_uoa)
								{
									if (and_)
									{
										sql_string += " AND ";
									}
									and_ = true;
									sql_string += "t1.";
									sql_string += previous_column_names_first_table[column_count];
									sql_string += " = t2.";
									sql_string += new_column.column_name_in_original_data_table;
								}
							}
						}
						++column_count;
					});
				}
			}
		});
	});

	// For use in both the WHERE and ORDER BY clauses
	// Determine how many columns there are corresponding to the DMU category with multiplicity greater than 1
	int number_primary_key_columns_in_dmu_category_with_multiplicity_greater_than_1 = 0;
	if (debug_ordering)
	{
		if (highest_multiplicity_primary_uoa > 1)
		{
			std::for_each(result_columns.columns_in_view.begin(), result_columns.columns_in_view.end(), [this, &number_primary_key_columns_in_dmu_category_with_multiplicity_greater_than_1, &sql_string](ColumnsInTempView::ColumnInTempView & view_column)
			{
				if (view_column.column_type == ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__PRIMARY)
				{
					if (view_column.is_within_inner_table_corresponding_to_top_level_uoa)
					{
						if (view_column.total_multiplicity__of_current_dmu_category__within_uoa_corresponding_to_the_current_inner_tables_variable_group == highest_multiplicity_primary_uoa)
						{
							if (view_column.current_multiplicity__corresponding_to__current_inner_table == 1)
							{
								++number_primary_key_columns_in_dmu_category_with_multiplicity_greater_than_1;
							}
						}
					}
				}
			});
		}
	}

	// Add the WHERE clause to guarantee that NULL primary key columns are at the right on each row,
	// and that non-NULL primary key columns are sorted from left to right on each row
	if (debug_ordering)
	{

		if (highest_multiplicity_primary_uoa > 1)
		{
	
			// Obtain the columns for the two primary keys being compared in a convenient vector.
			// The number of columns within a single primary key corresponding to the DMU category with multiplicity greater than 1 can be more than 1.
			for (int outer_dmu_multiplicity = 1; outer_dmu_multiplicity < highest_multiplicity_primary_uoa; ++outer_dmu_multiplicity)
			{

				if (outer_dmu_multiplicity > 1)
				{
					sql_string += " AND ";
				}
				else
				{
					sql_string += " WHERE ";
				}

				sql_string += "( CASE ";

				std::vector<ColumnsInTempView::ColumnInTempView> columns_for_active_dmu_category_lhs;
				std::vector<ColumnsInTempView::ColumnInTempView> columns_for_active_dmu_category_rhs;

				for (int inner_dmu_multiplicity = 0; inner_dmu_multiplicity < number_primary_key_columns_in_dmu_category_with_multiplicity_greater_than_1; ++inner_dmu_multiplicity)
				{

					std::for_each(result_columns.columns_in_view.begin(), result_columns.columns_in_view.end(), [this, &inner_dmu_multiplicity, &outer_dmu_multiplicity, &sql_string, &columns_for_active_dmu_category_lhs, &columns_for_active_dmu_category_rhs](ColumnsInTempView::ColumnInTempView & view_column)
					{
						if (view_column.column_type == ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__PRIMARY)
						{
							if (view_column.total_multiplicity__of_current_dmu_category__within_uoa_corresponding_to_the_current_inner_tables_variable_group == highest_multiplicity_primary_uoa)
							{
								if (view_column.primary_key_index__within_uoa_corresponding_to_variable_group_corresponding_to_current_inner_table__for_dmu_category == inner_dmu_multiplicity)
								{
									if (view_column.is_within_inner_table_corresponding_to_top_level_uoa)
									{
										if (view_column.current_multiplicity__corresponding_to__current_inner_table == outer_dmu_multiplicity)
										{
											columns_for_active_dmu_category_lhs.push_back(view_column);
										}
										else if (view_column.current_multiplicity__corresponding_to__current_inner_table == outer_dmu_multiplicity + 1)
										{
											columns_for_active_dmu_category_rhs.push_back(view_column);
										}
									}
								}
							}
						}
					});

				}

				sql_string += "WHEN ";
				sql_string += columns_for_active_dmu_category_lhs[0].column_name_in_temporary_table; // The first column in any primary key group suffices for the NULL check
				sql_string += " IS NULL AND ";
				sql_string += columns_for_active_dmu_category_rhs[0].column_name_in_temporary_table;
				sql_string += " IS NULL ";
				sql_string += "THEN 1 ";

				sql_string += "WHEN ";
				sql_string += columns_for_active_dmu_category_lhs[0].column_name_in_temporary_table; // The first column in any primary key group suffices for the NULL check
				sql_string += " IS NULL AND ";
				sql_string += columns_for_active_dmu_category_rhs[0].column_name_in_temporary_table;
				sql_string += " IS NOT NULL ";
				sql_string += "THEN 0 ";

				sql_string += "WHEN ";
				sql_string += columns_for_active_dmu_category_lhs[0].column_name_in_temporary_table; // The first column in any primary key group suffices for the NULL check
				sql_string += " IS NOT NULL AND ";
				sql_string += columns_for_active_dmu_category_rhs[0].column_name_in_temporary_table;
				sql_string += " IS NULL ";
				sql_string += "THEN 1 ";

				sql_string += "ELSE ";

				if (number_primary_key_columns_in_dmu_category_with_multiplicity_greater_than_1 == 1)
				{
					if (columns_for_active_dmu_category_lhs[0].primary_key_should_be_treated_as_numeric)
					{
						sql_string += "CAST (";
					}
					sql_string += columns_for_active_dmu_category_lhs[0].column_name_in_temporary_table;
					if (columns_for_active_dmu_category_lhs[0].primary_key_should_be_treated_as_numeric)
					{
						sql_string += " AS INTEGER)";
					}
					sql_string += " <= ";
					if (columns_for_active_dmu_category_rhs[0].primary_key_should_be_treated_as_numeric)
					{
						sql_string += "CAST (";
					}
					sql_string += columns_for_active_dmu_category_rhs[0].column_name_in_temporary_table;
					if (columns_for_active_dmu_category_rhs[0].primary_key_should_be_treated_as_numeric)
					{
						sql_string += " AS INTEGER)";
					}
				}
				else
				{
					sql_string += "( CASE";

					for (int inner_dmu_multiplicity = 0; inner_dmu_multiplicity < number_primary_key_columns_in_dmu_category_with_multiplicity_greater_than_1; ++inner_dmu_multiplicity)
					{
						sql_string += " WHEN ";
						sql_string += columns_for_active_dmu_category_lhs[inner_dmu_multiplicity].column_name_in_temporary_table;
						sql_string += " != ";
						sql_string += columns_for_active_dmu_category_rhs[inner_dmu_multiplicity].column_name_in_temporary_table;
						sql_string += " THEN ";
						if (columns_for_active_dmu_category_lhs[0].primary_key_should_be_treated_as_numeric)
						{
							sql_string += "CAST (";
						}
						sql_string += columns_for_active_dmu_category_lhs[inner_dmu_multiplicity].column_name_in_temporary_table;
						if (columns_for_active_dmu_category_lhs[0].primary_key_should_be_treated_as_numeric)
						{
							sql_string += " AS INTEGER)";
						}
						sql_string += " <= ";
						if (columns_for_active_dmu_category_rhs[0].primary_key_should_be_treated_as_numeric)
						{
							sql_string += "CAST (";
						}
						sql_string += columns_for_active_dmu_category_rhs[inner_dmu_multiplicity].column_name_in_temporary_table;
						if (columns_for_active_dmu_category_rhs[0].primary_key_should_be_treated_as_numeric)
						{
							sql_string += " AS INTEGER)";
						}
					}

					sql_string += " ELSE 1 "; // Equal primary key column sets

					sql_string += "END )";
				}

				sql_string += " END )";

			}

		}
	}

	// Add the ORDER BY column/s
	if (debug_ordering)
	{

		bool first = true;

		if (highest_multiplicity_primary_uoa > 1)
		{

			// Create the ORDER BY clause, taking the proper primary key columns that compose the DMU category with multiplicity greater than 1, in sequence
			for (int inner_dmu_multiplicity = 0; inner_dmu_multiplicity < number_primary_key_columns_in_dmu_category_with_multiplicity_greater_than_1; ++inner_dmu_multiplicity)
			{
				for (int outer_dmu_multiplicity = 1; outer_dmu_multiplicity <= highest_multiplicity_primary_uoa; ++outer_dmu_multiplicity)
				{
					std::for_each(result_columns.columns_in_view.begin(), result_columns.columns_in_view.end(), [this, &inner_dmu_multiplicity, &outer_dmu_multiplicity, &sql_string, &first](ColumnsInTempView::ColumnInTempView & view_column)
					{
						if (view_column.column_type == ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__PRIMARY)
						{
							if (view_column.total_multiplicity__of_current_dmu_category__within_uoa_corresponding_to_the_current_inner_tables_variable_group == highest_multiplicity_primary_uoa)
							{
								if (view_column.primary_key_index__within_uoa_corresponding_to_variable_group_corresponding_to_current_inner_table__for_dmu_category == inner_dmu_multiplicity)
								{
									if (view_column.current_multiplicity__corresponding_to__current_inner_table == outer_dmu_multiplicity)
									{
										if (view_column.is_within_inner_table_corresponding_to_top_level_uoa)
										{
											if (!first)
											{
												sql_string += ", ";
											}
											else
											{
												sql_string += " ORDER BY ";
											}
											first = false;
											if (view_column.primary_key_should_be_treated_as_numeric)
											{
												sql_string += "CAST (";
											}
											sql_string += view_column.column_name_in_temporary_table;
											if (view_column.primary_key_should_be_treated_as_numeric)
											{
												sql_string += " AS INTEGER)";
											}
										}
									}
								}
							}
						}
					});
				}
			}
		
		}

		// Now order by remaining primary key columns (with multiplicity 1)
		int current_column = 0;
		std::for_each(result_columns.columns_in_view.begin(), result_columns.columns_in_view.end(), [this, &sql_string, &result_columns, &current_column, &inner_table_column_count, &first](ColumnsInTempView::ColumnInTempView & view_column)
		{
			if (current_column >= inner_table_column_count)
			{
				return;
			}
			// Determine how many columns there are corresponding to the DMU category
			int number_primary_key_columns_in_dmu_category_with_multiplicity_of_1 = 0;
			int column_count_ = 0;
			std::for_each(result_columns.columns_in_view.begin(), result_columns.columns_in_view.end(), [this, &view_column, &column_count_, &inner_table_column_count, &number_primary_key_columns_in_dmu_category_with_multiplicity_of_1, &sql_string](ColumnsInTempView::ColumnInTempView & view_column_)
			{
				if (column_count_ >= inner_table_column_count)
				{
					return;
				}
				if (view_column_.column_type == ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__PRIMARY)
				{
					if (view_column_.primary_key_dmu_category_identifier.IsEqual(WidgetInstanceIdentifier::EQUALITY_CHECK_TYPE__STRING_CODE, view_column.primary_key_dmu_category_identifier))
					{
						if (view_column_.total_multiplicity__of_current_dmu_category__within_uoa_corresponding_to_the_current_inner_tables_variable_group == 1)
						{
							if (view_column.is_within_inner_table_corresponding_to_top_level_uoa)
							{
								++number_primary_key_columns_in_dmu_category_with_multiplicity_of_1;
							}
						}
					}
				}
				++column_count_;
			});

			if (view_column.column_type == ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__PRIMARY)
			{
				if (view_column.total_multiplicity__of_current_dmu_category__within_uoa_corresponding_to_the_current_inner_tables_variable_group == 1)
				{
					for (int inner_dmu_multiplicity = 0; inner_dmu_multiplicity < number_primary_key_columns_in_dmu_category_with_multiplicity_of_1; ++inner_dmu_multiplicity)
					{
						if (view_column.primary_key_index__within_uoa_corresponding_to_variable_group_corresponding_to_current_inner_table__for_dmu_category == inner_dmu_multiplicity)
						{
							if (view_column.is_within_inner_table_corresponding_to_top_level_uoa)
							{
								if (!first)
								{
									sql_string += ", ";
								}
								else
								{
									sql_string += " ORDER BY ";
								}
								first = false;
								if (view_column.primary_key_should_be_treated_as_numeric)
								{
									sql_string += "CAST (";
								}
								sql_string += view_column.column_name_in_temporary_table;
								if (view_column.primary_key_should_be_treated_as_numeric)
								{
									sql_string += " AS INTEGER)";
								}
							}
						}
					}
				}
			}
			++current_column;
		});

	}

	// SQL to add the datetime columns, if they are not present in the raw data table (filled with 0)
	if (primary_variable_group_raw_data_columns.has_no_datetime_columns_originally)
	{
		std::string datetime_start_col_name_no_uuid = "DATETIME_ROW_START";
		std::string datetime_start_col_name = datetime_start_col_name_no_uuid;
		datetime_start_col_name += "_";
		datetime_start_col_name += newUUID(true);

		std::string alter_string;
		alter_string += "ALTER TABLE ";
		alter_string += result_columns.view_name;
		alter_string += " ADD COLUMN ";
		alter_string += datetime_start_col_name;
		alter_string += " INTEGER DEFAULT 0";
		sql_strings.push_back(SQLExecutor(db, alter_string));

		result_columns.columns_in_view.push_back(ColumnsInTempView::ColumnInTempView());
		ColumnsInTempView::ColumnInTempView & datetime_start_column = result_columns.columns_in_view.back();
		datetime_start_column.column_name_in_temporary_table = datetime_start_col_name;
		datetime_start_column.column_name_in_temporary_table_no_uuid = datetime_start_col_name_no_uuid;
		datetime_start_column.column_type = ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__DATETIMESTART_INTERNAL;
		datetime_start_column.variable_group_associated_with_current_inner_table = variable_group;
		datetime_start_column.uoa_associated_with_variable_group_associated_with_current_inner_table = uoa;
		datetime_start_column.column_name_in_original_data_table = "";
		datetime_start_column.inner_table_set_number = 0;
		datetime_start_column.is_within_inner_table_corresponding_to_top_level_uoa = true;

		std::string datetime_end_col_name_no_uuid = "DATETIME_ROW_END";
		std::string datetime_end_col_name = datetime_end_col_name_no_uuid;
		datetime_end_col_name += "_";
		datetime_end_col_name += newUUID(true);

		alter_string.clear();
		alter_string += "ALTER TABLE ";
		alter_string += result_columns.view_name;
		alter_string += " ADD COLUMN ";
		alter_string += datetime_end_col_name;
		alter_string += " INTEGER DEFAULT 0";
		sql_strings.push_back(SQLExecutor(db, alter_string));

		result_columns.columns_in_view.push_back(ColumnsInTempView::ColumnInTempView());
		ColumnsInTempView::ColumnInTempView & datetime_end_column = result_columns.columns_in_view.back();
		datetime_end_column.column_name_in_temporary_table = datetime_end_col_name;
		datetime_end_column.column_name_in_temporary_table_no_uuid = datetime_end_col_name_no_uuid;
		datetime_end_column.column_type = ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__DATETIMEEND_INTERNAL;
		datetime_end_column.variable_group_associated_with_current_inner_table = variable_group;
		datetime_end_column.uoa_associated_with_variable_group_associated_with_current_inner_table = uoa;
		datetime_end_column.column_name_in_original_data_table = "";
		datetime_end_column.inner_table_set_number = 0;
		datetime_end_column.is_within_inner_table_corresponding_to_top_level_uoa = true;
	}

	return result;

}

void OutputModel::OutputGenerator::CreateNewXRRow(bool & first_row_added, std::string const & datetime_start_col_name, std::string const & datetime_end_col_name, std::string const & xr_view_name, std::string & sql_add_xr_row, std::vector<std::string> & bound_parameter_strings, std::vector<std::int64_t> & bound_parameter_ints, std::vector<SQLExecutor::WHICH_BINDING> & bound_parameter_which_binding_to_use, std::int64_t const datetime_start, std::int64_t const datetime_end, ColumnsInTempView & previous_x_columns, bool const include_previous_data, bool const include_current_data)
{

	if (first_row_added)
	{
		
		// Create SQL statement here, including placeholders for bound parameters

		sql_add_xr_row.clear();

		sql_add_xr_row += "INSERT OR FAIL INTO ";
		sql_add_xr_row += xr_view_name;
		sql_add_xr_row += "(";

		bool first_column_name = true;
		std::for_each(previous_x_columns.columns_in_view.cbegin(), previous_x_columns.columns_in_view.cend(), [&first_column_name, &sql_add_xr_row, &bound_parameter_strings, &bound_parameter_ints, &bound_parameter_which_binding_to_use, &include_previous_data, &include_current_data](ColumnsInTempView::ColumnInTempView const & column_in_view)
		{
			
			if (!first_column_name)
			{
				sql_add_xr_row += ", ";
			}
			first_column_name = false;

			sql_add_xr_row += column_in_view.column_name_in_temporary_table;

		});

		// The two new "merged" time range columns
		if (!first_column_name)
		{
			sql_add_xr_row += ", ";
		}
		first_column_name = false;
		sql_add_xr_row += datetime_start_col_name;
		sql_add_xr_row += ", ";
		sql_add_xr_row += datetime_end_col_name;

		sql_add_xr_row += ") VALUES (";

		int index = 1;
		char cindex[256];

		bool first_column_value = true;
		std::for_each(previous_x_columns.columns_in_view.cbegin(), previous_x_columns.columns_in_view.cend(), [&first_column_value, &index, &cindex, &sql_add_xr_row, &bound_parameter_strings, &bound_parameter_ints, &bound_parameter_which_binding_to_use, &include_previous_data, &include_current_data](ColumnsInTempView::ColumnInTempView const & column_in_view)
		{

			if (!first_column_value)
			{
				sql_add_xr_row += ", ";
			}
			first_column_value = false;

			sql_add_xr_row += "?";
			sql_add_xr_row += itoa(index, cindex, 10);
			++index;

		});

		// The two new "merged" time range columns
		if (!first_column_value)
		{
			sql_add_xr_row += ", ";
		}
		first_column_value = false;
		sql_add_xr_row += "?";
		sql_add_xr_row += itoa(index, cindex, 10);
		++index;
		sql_add_xr_row += ", ";
		sql_add_xr_row += "?";
		sql_add_xr_row += itoa(index, cindex, 10);
		++index;

		sql_add_xr_row += ")";

		first_row_added = false;

	}

	if (failed)
	{
		return;
	}

	int highest_index_previous_table = (int)previous_x_columns.columns_in_view.size() - 1;
	bool found_highest_index = false;
	std::for_each(previous_x_columns.columns_in_view.crbegin(), previous_x_columns.columns_in_view.crend(), [&highest_index_previous_table, &found_highest_index](ColumnsInTempView::ColumnInTempView const & column_in_view)
	{
		if (found_highest_index)
		{
			return;
		}
		if (column_in_view.column_type == ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__DATETIMEEND_MERGED)
		{
			found_highest_index = true;
			return;
		}
		--highest_index_previous_table;
	});

	// Set the list of bound parameters, regardless of whether or not the SQL string was created
	int index = 0;
	char cindex[256];
	bool first_column_value = true;
	std::int64_t data_int64 = 0;
	std::string data_string;
	long double data_long = 0.0;
	bool data_is_null = false;
	int column_data_type = 0;
	bound_parameter_strings.clear();
	bound_parameter_ints.clear();
	bound_parameter_which_binding_to_use.clear();
	std::for_each(previous_x_columns.columns_in_view.cbegin(), previous_x_columns.columns_in_view.cend(), [this, &highest_index_previous_table, &data_int64, &data_string, &data_long, &data_is_null, &column_data_type, &first_column_value, &index, &cindex, &sql_add_xr_row, &bound_parameter_strings, &bound_parameter_ints, &bound_parameter_which_binding_to_use, &include_previous_data, &include_current_data](ColumnsInTempView::ColumnInTempView const & column_in_view)
	{

		if (failed)
		{
			return;
		}

		data_is_null = false;

		if (index <= highest_index_previous_table)
		{
			if (!include_previous_data)
			{
				data_is_null = true;
				bound_parameter_which_binding_to_use.push_back(SQLExecutor::NULL_BINDING);
			}
		}
		else
		{
			if (!include_current_data)
			{
				data_is_null = true;
				bound_parameter_which_binding_to_use.push_back(SQLExecutor::NULL_BINDING);
			}
		}

		if (!data_is_null)
		{

			column_data_type = sqlite3_column_type(stmt_result, index);
			switch (column_data_type)
			{

				case SQLITE_INTEGER:
					{
						data_int64 = sqlite3_column_int64(stmt_result, index);
						bound_parameter_ints.push_back(data_int64);
						bound_parameter_which_binding_to_use.push_back(SQLExecutor::INT64);
					}
					break;

				case SQLITE_FLOAT:
					{
						// Currently not implemented!!!!!!!  Just add new bound_paramenter_longs as argument to this function, and as member of SQLExecutor just like the other bound_parameter data members, to implement.
						data_long = sqlite3_column_double(stmt_result, index);
						// Todo: Error message
						failed = true;
						return; // from lambda
					}
					break;

				case SQLITE_TEXT:
					{
						data_string = reinterpret_cast<char const *>(sqlite3_column_text(stmt_result, index));
						bound_parameter_strings.push_back(data_string);
						bound_parameter_which_binding_to_use.push_back(SQLExecutor::STRING);
					}
					break;

				case SQLITE_BLOB:
					{
						// Todo: Error message
						failed = true;
						return; // from lambda
					}
					break;

				case SQLITE_NULL:
					{
						data_is_null = true;
						bound_parameter_which_binding_to_use.push_back(SQLExecutor::NULL_BINDING);
					}
					break;

				default:
					{
						// Todo: Error message
						failed = true;
						return; // from lambda
					}

			}

		}

		++index;

	});

	// The two new "merged" time range columns
	bound_parameter_ints.push_back(datetime_start);
	bound_parameter_which_binding_to_use.push_back(SQLExecutor::INT64);
	bound_parameter_ints.push_back(datetime_end);
	bound_parameter_which_binding_to_use.push_back(SQLExecutor::INT64);

}

OutputModel::OutputGenerator::SqlAndColumnSet OutputModel::OutputGenerator::CreateChildXTable(ColumnsInTempView const & child_variable_group_raw_data_columns, ColumnsInTempView const & previous_xr_columns, int const current_multiplicity, int const primary_group_number, int const child_set_number, int const current_child_view_name_index)
{

	char c[256];

	SqlAndColumnSet result = std::make_pair(std::vector<SQLExecutor>(), ColumnsInTempView());
	std::vector<SQLExecutor> & sql_strings = result.first;
	ColumnsInTempView & result_columns = result.second;

	result_columns = previous_xr_columns;

	std::string view_name = "CV";
	view_name += itoa(primary_group_number, c, 10);
	view_name += "_x";
	view_name += itoa(current_child_view_name_index, c, 10);
	result_columns.view_name_no_uuid = view_name;
	view_name += "_";
	view_name += newUUID(true);
	result_columns.view_name = view_name;
	result_columns.view_number = current_multiplicity;
	result_columns.has_no_datetime_columns = false;

	int first_full_table_column_count = 0;
	int top_level_inner_table_column_count = 0;
	int second_table_column_count = 0;

	std::vector<std::string> previous_column_names_first_table;

	WidgetInstanceIdentifier variable_group_primary;
	WidgetInstanceIdentifier uoa_primary;

	WidgetInstanceIdentifier variable_group_child;
	WidgetInstanceIdentifier uoa_child;

	// These columns are from the previous XR temporary table, which is guaranteed to have all columns in place, including datetime columns.
	// Further, the "current_multiplicity" of these columns is guaranteed to be correct.
	// Also, the first columns always correspond to the primary variable group.
	bool first = true;
	bool in_first_inner_table = true;
	bool reached_first_datetime_start_merged_column = false;
	bool reached_first_datetime_end_merged_column = false;
	std::for_each(result_columns.columns_in_view.begin(), result_columns.columns_in_view.end(), [&first_full_table_column_count, &top_level_inner_table_column_count, &in_first_inner_table, &reached_first_datetime_start_merged_column, &reached_first_datetime_end_merged_column, &previous_column_names_first_table, &variable_group_primary, &uoa_primary, &first](ColumnsInTempView::ColumnInTempView & new_column)
	{
		previous_column_names_first_table.push_back(new_column.column_name_in_temporary_table);
		new_column.column_name_in_temporary_table = new_column.column_name_in_temporary_table_no_uuid;
		new_column.column_name_in_temporary_table += "_";
		new_column.column_name_in_temporary_table += newUUID(true);
		++first_full_table_column_count;
		if (new_column.column_type == ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__DATETIMESTART_MERGED)
		{
			reached_first_datetime_start_merged_column = true;
		}
		if (new_column.column_type == ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__DATETIMEEND_MERGED)
		{
			reached_first_datetime_end_merged_column = true;
		}
		if (in_first_inner_table)
		{
			++top_level_inner_table_column_count;
		}
		if (reached_first_datetime_start_merged_column && reached_first_datetime_end_merged_column)
		{
			in_first_inner_table = false;
		}
		if (first)
		{
			first = false;
			variable_group_primary = new_column.variable_group_associated_with_current_inner_table;
			uoa_primary = new_column.uoa_associated_with_variable_group_associated_with_current_inner_table;
		}
	});

	WidgetInstanceIdentifiers const & variables_selected = (*the_map)[*child_variable_group_raw_data_columns.variable_groups[0].identifier_parent][child_variable_group_raw_data_columns.variable_groups[0]];

	// These columns are from the new table (the raw child data table) being added.
	// Make column names for this temporary table unique (not the same as the column names from the previous table that is being copied)
	// which may or may not have datetime columns.
	// Further, the "current_multiplicity" of these columns is 1, and must be updated.
	first = true;
	std::for_each(child_variable_group_raw_data_columns.columns_in_view.cbegin(), child_variable_group_raw_data_columns.columns_in_view.cend(), [&first, &child_set_number, &variable_group_child, &uoa_child, &variables_selected, &result_columns, &second_table_column_count, &current_multiplicity](ColumnsInTempView::ColumnInTempView const & new_column_)
	{
		if (new_column_.column_type == ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__DATETIMESTART || new_column_.column_type == ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__DATETIMESTART_INTERNAL || new_column_.column_type == ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__DATETIMEEND || new_column_.column_type == ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__DATETIMEEND_INTERNAL)
		{
			return; // Add these columns last
		}

		bool match = true;
		if (new_column_.column_type == ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__SECONDARY)
		{
			match = false;
			std::for_each(variables_selected.cbegin(), variables_selected.cend(), [&new_column_, &match](WidgetInstanceIdentifier const & variable_selected)
			{
				if (boost::iequals(new_column_.column_name_in_original_data_table, *variable_selected.code))
				{
					match = true;
				}
			});
		}

		if (match)
		{
			result_columns.columns_in_view.push_back(new_column_);
			ColumnsInTempView::ColumnInTempView & new_column = result_columns.columns_in_view.back();
			new_column.column_name_in_temporary_table = new_column.column_name_in_temporary_table_no_uuid;
			new_column.column_name_in_temporary_table += "_";
			new_column.column_name_in_temporary_table += newUUID(true);
			new_column.inner_table_set_number = child_set_number;
			new_column.is_within_inner_table_corresponding_to_top_level_uoa = false;
			if (new_column.column_type == ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__PRIMARY)
			{
				if (new_column.total_multiplicity__of_current_dmu_category__within_uoa_corresponding_to_the_current_inner_tables_variable_group > 1)
				{
					new_column.current_multiplicity__corresponding_to__current_inner_table = current_multiplicity; // update current multiplicity
					if (new_column.total_k_count__within_uoa_corresponding_to_current_variable_group__for_current_dmu_category < new_column.total_k_count__within_uoa_corresponding_to_top_level_variable_group__for_current_dmu_category)
					{
						new_column.primary_key_index_within_total_kad_for_dmu_category = current_multiplicity;
					}
					else
					{
						// must have: new_column.total_k_count__within_uoa_corresponding_to_current_variable_group__for_current_dmu_category
						//         == new_column.total_k_count__within_uoa_corresponding_to_top_level_variable_group__for_current_dmu_category
						new_column.primary_key_index_within_total_kad_for_dmu_category = new_column.primary_key_index__within_uoa_corresponding_to_variable_group_corresponding_to_current_inner_table__for_dmu_category
							+ (current_multiplicity - 1) * new_column.total_k_count__within_uoa_corresponding_to_current_variable_group__for_current_dmu_category;
					}
				}
			}
			++second_table_column_count;
			if (first)
			{
				first = false;
				variable_group_child = new_column.variable_group_associated_with_current_inner_table;
				uoa_child = new_column.uoa_associated_with_variable_group_associated_with_current_inner_table;
			}
		}
	});
	// Datetime columns, if present
	std::for_each(child_variable_group_raw_data_columns.columns_in_view.cbegin(), child_variable_group_raw_data_columns.columns_in_view.cend(), [&result_columns, &child_set_number, &second_table_column_count](ColumnsInTempView::ColumnInTempView const & new_column_)
	{
		if (new_column_.column_type == ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__DATETIMESTART || new_column_.column_type == ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__DATETIMESTART_INTERNAL)
		{
			result_columns.columns_in_view.push_back(new_column_);
			ColumnsInTempView::ColumnInTempView & new_column = result_columns.columns_in_view.back();
			new_column.column_name_in_temporary_table = new_column.column_name_in_temporary_table_no_uuid;
			new_column.column_name_in_temporary_table += "_";
			new_column.column_name_in_temporary_table += newUUID(true);
			new_column.inner_table_set_number = child_set_number;
			new_column.is_within_inner_table_corresponding_to_top_level_uoa = false;
			++second_table_column_count;
		}
	});
	std::for_each(child_variable_group_raw_data_columns.columns_in_view.cbegin(), child_variable_group_raw_data_columns.columns_in_view.cend(), [&result_columns, &child_set_number, &second_table_column_count](ColumnsInTempView::ColumnInTempView const & new_column_)
	{
		if (new_column_.column_type == ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__DATETIMEEND || new_column_.column_type == ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__DATETIMEEND_INTERNAL)
		{
			result_columns.columns_in_view.push_back(new_column_);
			ColumnsInTempView::ColumnInTempView & new_column = result_columns.columns_in_view.back();
			new_column.column_name_in_temporary_table = new_column.column_name_in_temporary_table_no_uuid;
			new_column.column_name_in_temporary_table += "_";
			new_column.column_name_in_temporary_table += newUUID(true);
			new_column.inner_table_set_number = child_set_number;
			new_column.is_within_inner_table_corresponding_to_top_level_uoa = false;
			++second_table_column_count;
		}
	});

	sql_strings.push_back(SQLExecutor(db));
	std::string & sql_string = sql_strings.back().sql;

	sql_string = "CREATE TABLE ";
	sql_string += result_columns.view_name;
	sql_string += " AS SELECT ";
	first = true;
	int column_count = 0;
	std::for_each(result_columns.columns_in_view.begin(), result_columns.columns_in_view.end(), [&sql_string, &first, &column_count, &first_full_table_column_count, &second_table_column_count, &previous_column_names_first_table](ColumnsInTempView::ColumnInTempView & new_column)
	{
		if (!first)
		{
			sql_string += ", ";
		}
		first = false;
		if (column_count < first_full_table_column_count)
		{
			sql_string += "t1.";
			sql_string += previous_column_names_first_table[column_count];
		}
		else
		{
			sql_string += "t2.";
			sql_string += new_column.column_name_in_temporary_table_no_uuid; // This is the original column name
		}
		sql_string += " AS ";
		sql_string += new_column.column_name_in_temporary_table;
		++column_count;
	});
	sql_string += " FROM ";
	sql_string += previous_xr_columns.view_name;
	sql_string += " t1 JOIN ";
	sql_string += child_variable_group_raw_data_columns.original_table_names[0];
	sql_string += " t2 ON ";
	bool and_ = false;
	std::for_each(sequence.primary_key_sequence_info.cbegin(), sequence.primary_key_sequence_info.cend(), [this, &current_multiplicity, &sql_string, &variable_group_primary, &variable_group_child, &result_columns, &first_full_table_column_count, &top_level_inner_table_column_count, &second_table_column_count, &previous_column_names_first_table, &and_](PrimaryKeySequence::PrimaryKeySequenceEntry const & primary_key)
	{
		std::for_each(primary_key.variable_group_info_for_primary_keys.cbegin(), primary_key.variable_group_info_for_primary_keys.cend(), [this, &current_multiplicity, &sql_string, &variable_group_primary, &variable_group_child, &primary_key, &result_columns, &first_full_table_column_count, &top_level_inner_table_column_count, &second_table_column_count, &previous_column_names_first_table, &and_](PrimaryKeySequence::VariableGroup_PrimaryKey_Info const & primary_key_info_this_variable_group)
		{
			if (primary_key_info_this_variable_group.vg_identifier.IsEqual(WidgetInstanceIdentifier::EQUALITY_CHECK_TYPE__STRING_CODE, variable_group_child))
			{
				if (primary_key_info_this_variable_group.current_multiplicity == current_multiplicity)
				{
					int column_count = 0;
					std::for_each(result_columns.columns_in_view.cbegin(), result_columns.columns_in_view.cend(), [this, &current_multiplicity, &sql_string, &primary_key_info_this_variable_group, &first_full_table_column_count, &top_level_inner_table_column_count, &second_table_column_count, &column_count, &previous_column_names_first_table, &primary_key, &and_](ColumnsInTempView::ColumnInTempView const & new_column)
					{
						if (column_count >= highest_multiplicity_primary_uoa * top_level_inner_table_column_count)
						{
							return;
						}
						if (new_column.primary_key_dmu_category_identifier.IsEqual(WidgetInstanceIdentifier::EQUALITY_CHECK_TYPE__STRING_CODE, primary_key.dmu_category))
						{
							int desired_inner_table_index = 0;
							bool match_condition = false;

							// First, join on primary keys whose total multiplicity is 1
							if (primary_key_info_this_variable_group.total_multiplicity == 1)
							{
								if (current_multiplicity == 1)
								{
									match_condition = (new_column.primary_key_index_within_total_kad_for_dmu_category >= 0 && (new_column.primary_key_index_within_total_kad_for_dmu_category == primary_key.sequence_number_within_dmu_category_spin_control));
								}
							}

							// Also join on the current multiplicity
							// Note: As currently enforced, child UOA's can have only one
							// DMU with multiplicity greater than 1.
							else
							{
								// Break this into different cases.

								// Case 1: The highest multiplicity of the PRIMARY uoa's is 1.
								// All columns exist in the first inner table.
								if (highest_multiplicity_primary_uoa == 1)
								{
									if (current_multiplicity == 1)
									{
										match_condition = (new_column.primary_key_index_within_total_kad_for_dmu_category >= 0 && (new_column.primary_key_index_within_total_kad_for_dmu_category == primary_key.sequence_number_within_dmu_category_spin_control));
									}
								}

								// Cases 2-4 correspond to the highest multiplicity of the primary UOA's
								// being greater than 1.
								if (highest_multiplicity_primary_uoa > 1)
								{

									// Case 2: The highest multiplicity of the primary UOA's is greater than 1,
									// but the DMU category of this child group does not match the one
									// corresponding to that multiplicity.
									// All columns therefore exist in the first inner table.
									if (!boost::iequals(highest_multiplicity_primary_uoa_dmu_string_code, *primary_key.dmu_category.code))
									{
										if (current_multiplicity == 1)
										{
											// Same match condition as above.
											match_condition = (new_column.primary_key_index_within_total_kad_for_dmu_category >= 0 && (new_column.primary_key_index_within_total_kad_for_dmu_category == primary_key.sequence_number_within_dmu_category_spin_control));
										}
									}

									// Cases 3 & 4 correspond to the highest multiplicity of the primary UOA's being greater than 1,
									// and the DMU category of this child group matches the one corresponding
									// to that multiplicity.
									else
									{

										// ... Case 3: The K-value for the *UOA* of the child group for this DMU category
										// ... is less than the K-value for the *UOA* of the primary groups for this DMU category
										// ... (due to current constraints enforced on the user's settings,
										// ... the child group's K-value must in this scenario be equal to 1).
										// ... We must therefore iterate through every column of this DMU category INSIDE each inner table,
										// ... along with iterating through every inner table.
										if (primary_key_info_this_variable_group.total_number_columns_for_dmu_category__internal_to_uoa_corresponding_to_this_variable_group < primary_key.total_k_count_within_high_level_variable_group_uoa_for_this_dmu_category)
										{
											desired_inner_table_index = (current_multiplicity - 1) / primary_key.total_k_count_within_high_level_variable_group_uoa_for_this_dmu_category;
											match_condition = (current_multiplicity - 1 == new_column.primary_key_index_within_total_kad_for_dmu_category);
										}

										// ... Case 4: The K-value for the *UOA* of the child group for this DMU category
										// ... matches the K-value for the *UOA* of the primary groups for this DMU category.
										// ... Therefore, we need to iterate through every inner table,
										// ... but inside each inner table, there is only one match for the child
										// ... that includes all columns in that table for this DMU category.
										else
										{
											desired_inner_table_index = current_multiplicity - 1;
											match_condition = true;
										}

									}

								}

							}

							if (column_count >= desired_inner_table_index * top_level_inner_table_column_count && column_count < (desired_inner_table_index + 1) * top_level_inner_table_column_count)
							{
								if (match_condition)
								{
									if (and_)
									{
										sql_string += " AND ";
									}
									and_ = true;
									sql_string += "t1.";
									sql_string += previous_column_names_first_table[column_count];
									sql_string += " = t2.";
									sql_string += primary_key_info_this_variable_group.table_column_name;
								}
							}

						}
						++column_count;
					});
				}
			}
		});
	});

	// For use in both the WHERE and ORDER BY clauses
	// Determine how many columns there are corresponding to the DMU category with multiplicity greater than 1
	int number_primary_key_columns_in_dmu_category_with_multiplicity_greater_than_1 = 0;
	if (debug_ordering)
	{
		if (highest_multiplicity_primary_uoa > 1)
		{
			std::for_each(result_columns.columns_in_view.begin(), result_columns.columns_in_view.end(), [this, &number_primary_key_columns_in_dmu_category_with_multiplicity_greater_than_1, &sql_string](ColumnsInTempView::ColumnInTempView & view_column)
			{
				if (view_column.column_type == ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__PRIMARY)
				{
					if (view_column.is_within_inner_table_corresponding_to_top_level_uoa)
					{
						if (view_column.total_multiplicity__of_current_dmu_category__within_uoa_corresponding_to_the_current_inner_tables_variable_group == highest_multiplicity_primary_uoa)
						{
							if (view_column.current_multiplicity__corresponding_to__current_inner_table == 1)
							{
								++number_primary_key_columns_in_dmu_category_with_multiplicity_greater_than_1;
							}
						}
					}
				}
			});
		}
	}

	// For use in both the WHERE and ORDER BY clauses
	// Determine how many columns there are corresponding to the DMU category with multiplicity greater than 1
	int number_primary_key_columns_in_dmu_category_with_multiplicity_greater_than_1_child = 0;
	std::pair<WidgetInstanceIdentifier, int> & uoa_child__which_multiplicity_is_greater_than_1 = child_uoas__which_multiplicity_is_greater_than_1[uoa_child];
	int const highest_multiplicity_child_uoa = uoa_child__which_multiplicity_is_greater_than_1.second;
	WidgetInstanceIdentifier const & dmu_category_multiplicity = uoa_child__which_multiplicity_is_greater_than_1.first;
	if (debug_ordering)
	{
		if (highest_multiplicity_child_uoa > 1)
		{
			int number_primary_key_columns_in_dmu_category_with_multiplicity_greater_than_1_temp = 0;
			std::for_each(result_columns.columns_in_view.begin(), result_columns.columns_in_view.end(), [this, &number_primary_key_columns_in_dmu_category_with_multiplicity_greater_than_1_temp, &sql_string, &highest_multiplicity_child_uoa, &dmu_category_multiplicity](ColumnsInTempView::ColumnInTempView & view_column)
			{
				if (view_column.column_type == ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__PRIMARY)
				{
					if (view_column.primary_key_dmu_category_identifier.IsEqual(WidgetInstanceIdentifier::EQUALITY_CHECK_TYPE__STRING_CODE, dmu_category_multiplicity))
					{
						if (view_column.current_multiplicity__corresponding_to__current_inner_table == 1)
						{
							++number_primary_key_columns_in_dmu_category_with_multiplicity_greater_than_1_temp;
						}
					}
				}
			});
			number_primary_key_columns_in_dmu_category_with_multiplicity_greater_than_1_child = number_primary_key_columns_in_dmu_category_with_multiplicity_greater_than_1_temp / highest_multiplicity_child_uoa;
		}
	}

	// Add the ORDER BY column/s
	if (debug_ordering)
	{

		bool first = true;

		if (highest_multiplicity_primary_uoa > 1)
		{

			// Create the ORDER BY clause, taking the proper primary key columns that compose the DMU category with multiplicity greater than 1, in sequence
			for (int inner_dmu_multiplicity = 0; inner_dmu_multiplicity < number_primary_key_columns_in_dmu_category_with_multiplicity_greater_than_1; ++inner_dmu_multiplicity)
			{
				for (int outer_dmu_multiplicity = 1; outer_dmu_multiplicity <= highest_multiplicity_primary_uoa; ++outer_dmu_multiplicity)
				{
					std::for_each(result_columns.columns_in_view.begin(), result_columns.columns_in_view.end(), [this, &inner_dmu_multiplicity, &outer_dmu_multiplicity, &sql_string, &first](ColumnsInTempView::ColumnInTempView & view_column)
					{
						if (view_column.column_type == ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__PRIMARY)
						{
							if (view_column.total_multiplicity__of_current_dmu_category__within_uoa_corresponding_to_the_current_inner_tables_variable_group == highest_multiplicity_primary_uoa)
							{
								if (view_column.primary_key_index__within_uoa_corresponding_to_variable_group_corresponding_to_current_inner_table__for_dmu_category == inner_dmu_multiplicity)
								{
									if (view_column.current_multiplicity__corresponding_to__current_inner_table == outer_dmu_multiplicity)
									{
										if (view_column.is_within_inner_table_corresponding_to_top_level_uoa)
										{
											if (!first)
											{
												sql_string += ", ";
											}
											else
											{
												sql_string += " ORDER BY ";
											}
											first = false;
											if (view_column.primary_key_should_be_treated_as_numeric)
											{
												sql_string += "CAST (";
											}
											sql_string += view_column.column_name_in_temporary_table;
											if (view_column.primary_key_should_be_treated_as_numeric)
											{
												sql_string += " AS INTEGER)";
											}
										}
									}
								}
							}
						}
					});
				}
			}

		}

		// Now order by remaining primary key columns (with multiplicity 1)
		int current_column = 0;
		std::for_each(result_columns.columns_in_view.begin(), result_columns.columns_in_view.end(), [this, &sql_string, &result_columns, &current_column, &top_level_inner_table_column_count, &first](ColumnsInTempView::ColumnInTempView & view_column)
		{
			if (current_column >= top_level_inner_table_column_count)
			{
				return;
			}
			// Determine how many columns there are corresponding to the DMU category
			int number_primary_key_columns_in_dmu_category_with_multiplicity_of_1 = 0;
			int column_count_ = 0;
			std::for_each(result_columns.columns_in_view.begin(), result_columns.columns_in_view.end(), [this, &view_column, &column_count_, &top_level_inner_table_column_count, &number_primary_key_columns_in_dmu_category_with_multiplicity_of_1, &sql_string](ColumnsInTempView::ColumnInTempView & view_column_)
			{
				if (column_count_ >= top_level_inner_table_column_count)
				{
					return;
				}
				if (view_column_.column_type == ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__PRIMARY)
				{
					if (view_column_.primary_key_dmu_category_identifier.IsEqual(WidgetInstanceIdentifier::EQUALITY_CHECK_TYPE__STRING_CODE, view_column.primary_key_dmu_category_identifier))
					{
						if (view_column_.total_multiplicity__of_current_dmu_category__within_uoa_corresponding_to_the_current_inner_tables_variable_group == 1)
						{
							if (view_column_.is_within_inner_table_corresponding_to_top_level_uoa)
							{
								++number_primary_key_columns_in_dmu_category_with_multiplicity_of_1;
							}
						}
					}
				}
				++column_count_;
			});

			if (view_column.column_type == ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__PRIMARY)
			{
				if (view_column.total_multiplicity__of_current_dmu_category__within_uoa_corresponding_to_the_current_inner_tables_variable_group == 1)
				{
					for (int inner_dmu_multiplicity = 0; inner_dmu_multiplicity < number_primary_key_columns_in_dmu_category_with_multiplicity_of_1; ++inner_dmu_multiplicity)
					{
						if (view_column.primary_key_index__within_uoa_corresponding_to_variable_group_corresponding_to_current_inner_table__for_dmu_category == inner_dmu_multiplicity)
						{
							if (view_column.is_within_inner_table_corresponding_to_top_level_uoa)
							{
								if (!first)
								{
									sql_string += ", ";
								}
								else
								{
									sql_string += " ORDER BY ";
								}
								first = false;
								if (view_column.primary_key_should_be_treated_as_numeric)
								{
									sql_string += "CAST (";
								}
								sql_string += view_column.column_name_in_temporary_table;
								if (view_column.primary_key_should_be_treated_as_numeric)
								{
									sql_string += " AS INTEGER)";
								}
							}
						}
					}
				}
			}
			++current_column;
		});

	}

	// SQL to add the datetime columns, if they are not present in the raw data table (filled with 0)
	if (child_variable_group_raw_data_columns.has_no_datetime_columns_originally)
	{
		std::string datetime_start_col_name_no_uuid = "DATETIME_ROW_START";
		std::string datetime_start_col_name = datetime_start_col_name_no_uuid;
		datetime_start_col_name += "_";
		datetime_start_col_name += newUUID(true);

		std::string alter_string;
		alter_string += "ALTER TABLE ";
		alter_string += result_columns.view_name;
		alter_string += " ADD COLUMN ";
		alter_string += datetime_start_col_name;
		alter_string += " INTEGER DEFAULT 0";
		sql_strings.push_back(SQLExecutor(db, alter_string));

		result_columns.columns_in_view.push_back(ColumnsInTempView::ColumnInTempView());
		ColumnsInTempView::ColumnInTempView & datetime_start_column = result_columns.columns_in_view.back();
		datetime_start_column.column_name_in_temporary_table = datetime_start_col_name;
		datetime_start_column.column_name_in_temporary_table_no_uuid = datetime_start_col_name_no_uuid;
		datetime_start_column.column_type = ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__DATETIMESTART_INTERNAL;
		datetime_start_column.variable_group_associated_with_current_inner_table = variable_group_child;
		datetime_start_column.uoa_associated_with_variable_group_associated_with_current_inner_table = uoa_child;
		datetime_start_column.column_name_in_original_data_table = "";
		datetime_start_column.inner_table_set_number = child_set_number;
		datetime_start_column.is_within_inner_table_corresponding_to_top_level_uoa = false;

		std::string datetime_end_col_name_no_uuid = "DATETIME_ROW_END";
		std::string datetime_end_col_name = datetime_end_col_name_no_uuid;
		datetime_end_col_name += "_";
		datetime_end_col_name += newUUID(true);

		alter_string.clear();
		alter_string += "ALTER TABLE ";
		alter_string += result_columns.view_name;
		alter_string += " ADD COLUMN ";
		alter_string += datetime_end_col_name;
		alter_string += " INTEGER DEFAULT 0";
		sql_strings.push_back(SQLExecutor(db, alter_string));

		result_columns.columns_in_view.push_back(ColumnsInTempView::ColumnInTempView());
		ColumnsInTempView::ColumnInTempView & datetime_end_column = result_columns.columns_in_view.back();
		datetime_end_column.column_name_in_temporary_table = datetime_end_col_name;
		datetime_end_column.column_name_in_temporary_table_no_uuid = datetime_end_col_name_no_uuid;
		datetime_end_column.column_type = ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__DATETIMEEND_INTERNAL;
		datetime_end_column.variable_group_associated_with_current_inner_table = variable_group_child;
		datetime_end_column.uoa_associated_with_variable_group_associated_with_current_inner_table = uoa_child;
		datetime_end_column.column_name_in_original_data_table = "";
		datetime_end_column.inner_table_set_number = child_set_number;
		datetime_end_column.is_within_inner_table_corresponding_to_top_level_uoa = false;
	}

	return result;

}

OutputModel::OutputGenerator::SqlAndColumnSet OutputModel::OutputGenerator::CreateXRTable(ColumnsInTempView & previous_x_columns, int const current_multiplicity, int const primary_group_number, bool const is_child_inner_table, int const current_set_number, int const current_view_name_index)
{

	char c[256];

	SqlAndColumnSet result = std::make_pair(std::vector<SQLExecutor>(), ColumnsInTempView());
	std::vector<SQLExecutor> & sql_strings = result.first;
	ColumnsInTempView & result_columns = result.second;

	result_columns = previous_x_columns;
	result_columns.most_recent_sql_statement_executed__index = -1;

	std::string view_name;
	if (is_child_inner_table)
	{
		view_name += "CV";
	}
	else
	{
		view_name += "V";
	}
	view_name += itoa(primary_group_number, c, 10);
	view_name += "_xr";
	view_name += itoa(current_view_name_index, c, 10);
	result_columns.view_name_no_uuid = view_name;
	view_name += "_";
	view_name += newUUID(true);
	result_columns.view_name = view_name;
	result_columns.view_number = current_multiplicity;
	result_columns.has_no_datetime_columns = false;

	std::string sql_create_empty_table;
	sql_create_empty_table += "CREATE TABLE ";
	sql_create_empty_table += result_columns.view_name;
	sql_create_empty_table += " AS SELECT * FROM ";
	sql_create_empty_table += previous_x_columns.view_name;
	sql_create_empty_table += " WHERE 0";
	sql_strings.push_back(SQLExecutor(db, sql_create_empty_table));


	// Add the "merged" time range columns

	WidgetInstanceIdentifier variable_group = previous_x_columns.columns_in_view[previous_x_columns.columns_in_view.size()-1].variable_group_associated_with_current_inner_table;
	WidgetInstanceIdentifier uoa = previous_x_columns.columns_in_view[previous_x_columns.columns_in_view.size()-1].uoa_associated_with_variable_group_associated_with_current_inner_table;

	std::string datetime_start_col_name_no_uuid = "DATETIME_ROW_START_MERGED";
	std::string datetime_start_col_name = datetime_start_col_name_no_uuid;
	datetime_start_col_name += "_";
	datetime_start_col_name += newUUID(true);

	std::string alter_string;
	alter_string += "ALTER TABLE ";
	alter_string += result_columns.view_name;
	alter_string += " ADD COLUMN ";
	alter_string += datetime_start_col_name;
	alter_string += " INTEGER DEFAULT 0";
	sql_strings.push_back(SQLExecutor(db, alter_string));

	result_columns.columns_in_view.push_back(ColumnsInTempView::ColumnInTempView());
	ColumnsInTempView::ColumnInTempView & datetime_start_column = result_columns.columns_in_view.back();
	datetime_start_column.column_name_in_temporary_table = datetime_start_col_name;
	datetime_start_column.column_name_in_temporary_table_no_uuid = datetime_start_col_name_no_uuid;
	datetime_start_column.column_type = ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__DATETIMESTART_MERGED;
	datetime_start_column.variable_group_associated_with_current_inner_table = variable_group;
	datetime_start_column.uoa_associated_with_variable_group_associated_with_current_inner_table = uoa;
	datetime_start_column.column_name_in_original_data_table = "";
	datetime_start_column.inner_table_set_number = current_set_number;
	if (!is_child_inner_table)
	{
		datetime_start_column.is_within_inner_table_corresponding_to_top_level_uoa = true;
	}
	else
	{
		datetime_start_column.is_within_inner_table_corresponding_to_top_level_uoa = false;
	}

	std::string datetime_end_col_name_no_uuid = "DATETIME_ROW_END_MERGED";
	std::string datetime_end_col_name = datetime_end_col_name_no_uuid;
	datetime_end_col_name += "_";
	datetime_end_col_name += newUUID(true);

	alter_string.clear();
	alter_string += "ALTER TABLE ";
	alter_string += result_columns.view_name;
	alter_string += " ADD COLUMN ";
	alter_string += datetime_end_col_name;
	alter_string += " INTEGER DEFAULT 0";
	sql_strings.push_back(SQLExecutor(db, alter_string));

	result_columns.columns_in_view.push_back(ColumnsInTempView::ColumnInTempView());
	ColumnsInTempView::ColumnInTempView & datetime_end_column = result_columns.columns_in_view.back();
	datetime_end_column.column_name_in_temporary_table = datetime_end_col_name;
	datetime_end_column.column_name_in_temporary_table_no_uuid = datetime_end_col_name_no_uuid;
	datetime_end_column.column_type = ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__DATETIMEEND_MERGED;
	datetime_end_column.variable_group_associated_with_current_inner_table = variable_group;
	datetime_end_column.uoa_associated_with_variable_group_associated_with_current_inner_table = uoa;
	datetime_end_column.column_name_in_original_data_table = "";
	datetime_end_column.inner_table_set_number = current_set_number;
	if (!is_child_inner_table)
	{
		datetime_end_column.is_within_inner_table_corresponding_to_top_level_uoa = true;
	}
	else
	{
		datetime_end_column.is_within_inner_table_corresponding_to_top_level_uoa = false;
	}


	int previous_datetime_start_column_index = -1;
	int previous_datetime_end_column_index = -1;
	int current_datetime_start_column_index = -1;
	int current_datetime_end_column_index = -1;
	int column_index = (int)previous_x_columns.columns_in_view.size() - 1;
	std::for_each(previous_x_columns.columns_in_view.crbegin(), previous_x_columns.columns_in_view.crend(), [&previous_datetime_start_column_index, &previous_datetime_end_column_index, &current_datetime_start_column_index, &current_datetime_end_column_index, &column_index](ColumnsInTempView::ColumnInTempView const & schema_column)
	{
		// COLUMN_TYPE__DATETIMESTART_MERGED can only be for the previous data
		if (schema_column.column_type == ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__DATETIMESTART_MERGED)
		{
			if (previous_datetime_start_column_index == -1)
			{
				previous_datetime_start_column_index = column_index;
			}
		}
		// COLUMN_TYPE__DATETIMEEND_MERGED can only be for the previous data
		else if (schema_column.column_type == ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__DATETIMEEND_MERGED)
		{
			if (previous_datetime_end_column_index == -1)
			{
				previous_datetime_end_column_index = column_index;
			}
		}
		// COLUMN_TYPE__DATETIMESTART and COLUMN_TYPE__DATETIMESTART_INTERNAL, when first seen, can only be for the current data
		else if (schema_column.column_type == ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__DATETIMESTART || schema_column.column_type == ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__DATETIMESTART_INTERNAL)
		{
			if (current_datetime_start_column_index == -1)
			{
				current_datetime_start_column_index = column_index;
			}
		}
		// COLUMN_TYPE__DATETIMEEND and COLUMN_TYPE__DATETIMEEND_INTERNAL, when first seen, can only be for the current data
		else if (schema_column.column_type == ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__DATETIMEEND || schema_column.column_type == ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__DATETIMEEND_INTERNAL)
		{
			if (current_datetime_end_column_index == -1)
			{
				current_datetime_end_column_index = column_index;
			}
		}

		// The previous values are always located after the current values, but in arbitrary order,
		// so this check suffices to be certain that all 4 values have been obtained
		if (previous_datetime_start_column_index != -1 && previous_datetime_end_column_index != -1)
		{
			return;
		}
		--column_index;
	});

	ExecuteSQL(result); // Executes all SQL queries up to the current one

	if (failed)
	{
		return result;
	}


	ObtainData(previous_x_columns);

	if (failed)
	{
		return result;
	}


	int const minimum_desired_rows_per_transaction = 1024;

	int current_rows_added = 0;
	int current_rows_added_since_execution = 0;
	std::string sql_add_xr_row;
	bool first_row_added = true;
	std::vector<std::string> bound_parameter_strings;
	std::vector<std::int64_t> bound_parameter_ints;
	std::vector<SQLExecutor::WHICH_BINDING> bound_parameter_which_binding_to_use;
	sqlite3_stmt * the_prepared_stmt = nullptr;

	BeginNewTransaction();

	while (StepData())
	{
		std::int64_t previous_datetime_start = sqlite3_column_int64(stmt_result, previous_datetime_start_column_index);
		std::int64_t previous_datetime_end = sqlite3_column_int64(stmt_result, previous_datetime_end_column_index);
		std::int64_t current_datetime_start = sqlite3_column_int64(stmt_result, current_datetime_start_column_index);
		std::int64_t current_datetime_end = sqlite3_column_int64(stmt_result, current_datetime_end_column_index);

		bool previous_is_0 = false;
		if (previous_datetime_start == 0 && previous_datetime_end == 0)
		{
			previous_is_0 = true;
		}

		bool current_is_0 = false;
		if (current_datetime_start == 0 && current_datetime_end == 0)
		{
			current_is_0 = true;
		}

		if (previous_is_0 && current_is_0)
		{

			// Add row as-is, setting new time range columns to 0
			CreateNewXRRow(first_row_added, datetime_start_col_name, datetime_end_col_name, result_columns.view_name, sql_add_xr_row, bound_parameter_strings, bound_parameter_ints, bound_parameter_which_binding_to_use, 0, 0, previous_x_columns, true, true);
			sql_strings.push_back(SQLExecutor(db, sql_add_xr_row, bound_parameter_strings, bound_parameter_ints, bound_parameter_which_binding_to_use, the_prepared_stmt, true));
			the_prepared_stmt = sql_strings.back().stmt;
			++current_rows_added;
			++current_rows_added_since_execution;

		}
		else if (previous_is_0 && !current_is_0)
		{

			// Add row as-is, setting new time range columns to current time range values
			CreateNewXRRow(first_row_added, datetime_start_col_name, datetime_end_col_name, result_columns.view_name, sql_add_xr_row, bound_parameter_strings, bound_parameter_ints, bound_parameter_which_binding_to_use, current_datetime_start, current_datetime_end, previous_x_columns, true, true);
			sql_strings.push_back(SQLExecutor(db, sql_add_xr_row, bound_parameter_strings, bound_parameter_ints, bound_parameter_which_binding_to_use, the_prepared_stmt, true));
			the_prepared_stmt = sql_strings.back().stmt;
			++current_rows_added;
			++current_rows_added_since_execution;

		}
		else if (!previous_is_0 && current_is_0)
		{

			// Add row as-is, setting new time range columns to previous time range values
			CreateNewXRRow(first_row_added, datetime_start_col_name, datetime_end_col_name, result_columns.view_name, sql_add_xr_row, bound_parameter_strings, bound_parameter_ints, bound_parameter_which_binding_to_use, previous_datetime_start, previous_datetime_end, previous_x_columns, true, true);
			sql_strings.push_back(SQLExecutor(db, sql_add_xr_row, bound_parameter_strings, bound_parameter_ints, bound_parameter_which_binding_to_use, the_prepared_stmt, true));
			the_prepared_stmt = sql_strings.back().stmt;
			++current_rows_added;
			++current_rows_added_since_execution;

		}
		else
		{

			// Both current and previous rows have non-zero time range columns.
			// Perform the algorithm that possibly splits the row into multiple rows,
			// one for each time range sub-region in the overlap of the time ranges between current and previous.

			// Rule out garbage
			if (previous_datetime_start >= previous_datetime_end)
			{
				// invalid previous time range values
				continue;
			}
			else if (current_datetime_start >= current_datetime_end)
			{
				// invalid current time range values
				continue;
			}

			// Both current and previous time range windows
			// are now guaranteed to have a non-zero, and positive, width

			std::int64_t lower_range_start = 0;
			std::int64_t lower_range_end = 0;
			std::int64_t upper_range_start = 0;
			std::int64_t upper_range_end = 0;

			bool previous_is_lower = false;
			if (previous_datetime_start <= current_datetime_start)
			{
				lower_range_start = previous_datetime_start;
				lower_range_end = previous_datetime_end;
				upper_range_start = current_datetime_start;
				upper_range_end = current_datetime_end;
				previous_is_lower = true;
			}
			else
			{
				lower_range_start = current_datetime_start;
				lower_range_end = current_datetime_end;
				upper_range_start = previous_datetime_start;
				upper_range_end = previous_datetime_end;
			}

			bool previous__DO_include_lower_range_data__DO_include_upper_range_data = true;
			bool previous__DO_include_lower_range_data__DO_NOT_include_upper_range_data = true;
			bool previous__DO_NOT_include_lower_range_data__DO_include_upper_range_data = false;
			bool previous__DO_NOT_include_lower_range_data__DO_NOT_include_upper_range_data = false;
			bool current__DO_include_lower_range_data__DO_include_upper_range_data = true;
			bool current__DO_include_lower_range_data__DO_NOT_include_upper_range_data = false;
			bool current__DO_NOT_include_lower_range_data__DO_include_upper_range_data = true;
			bool current__DO_NOT_include_lower_range_data__DO_NOT_include_upper_range_data = false;
			if (!previous_is_lower)
			{
				previous__DO_include_lower_range_data__DO_include_upper_range_data = true;
				previous__DO_include_lower_range_data__DO_NOT_include_upper_range_data = false;
				previous__DO_NOT_include_lower_range_data__DO_include_upper_range_data = true;
				previous__DO_NOT_include_lower_range_data__DO_NOT_include_upper_range_data = false;
				current__DO_include_lower_range_data__DO_include_upper_range_data = true;
				current__DO_include_lower_range_data__DO_NOT_include_upper_range_data = true;
				current__DO_NOT_include_lower_range_data__DO_include_upper_range_data = false;
				current__DO_NOT_include_lower_range_data__DO_NOT_include_upper_range_data = false;
			}

			if (lower_range_start == upper_range_start)
			{

				// special case: The lower range and the upper range
				// begin at the same time value


				// There is guaranteed to be overlap between the lower range
				// and the upper range

				if (lower_range_end == upper_range_end)
				{

					// special case: The lower range and the upper range
					// end at the same time value

					// Add row as-is, setting new time range columns
					// to either the previous or the current time range columns,
					// because they are the same
					CreateNewXRRow(first_row_added, datetime_start_col_name, datetime_end_col_name, result_columns.view_name, sql_add_xr_row, bound_parameter_strings, bound_parameter_ints, bound_parameter_which_binding_to_use, current_datetime_start, current_datetime_end, previous_x_columns, previous__DO_include_lower_range_data__DO_include_upper_range_data, current__DO_include_lower_range_data__DO_include_upper_range_data);
					sql_strings.push_back(SQLExecutor(db, sql_add_xr_row, bound_parameter_strings, bound_parameter_ints, bound_parameter_which_binding_to_use, the_prepared_stmt, true));
					the_prepared_stmt = sql_strings.back().stmt;
					++current_rows_added;
					++current_rows_added_since_execution;

				}
				else if (lower_range_end < upper_range_end)
				{

					// The upper range ends higher than the lower range

					// First, add a row that includes all data,
					// setting new time range columns to:
					// lower_range_start - lower_range_end
					CreateNewXRRow(first_row_added, datetime_start_col_name, datetime_end_col_name, result_columns.view_name, sql_add_xr_row, bound_parameter_strings, bound_parameter_ints, bound_parameter_which_binding_to_use, lower_range_start, lower_range_end, previous_x_columns, previous__DO_include_lower_range_data__DO_include_upper_range_data, current__DO_include_lower_range_data__DO_include_upper_range_data);
					sql_strings.push_back(SQLExecutor(db, sql_add_xr_row,bound_parameter_strings, bound_parameter_ints, bound_parameter_which_binding_to_use, the_prepared_stmt, true));
					the_prepared_stmt = sql_strings.back().stmt;
					++current_rows_added;
					++current_rows_added_since_execution;

					// Second, add a row that includes only the upper range's data,
					// setting new time range columns to:
					// lower_range_end - upper_range_end
					if (!is_child_inner_table || !previous_is_lower)
					{
						CreateNewXRRow(first_row_added, datetime_start_col_name, datetime_end_col_name, result_columns.view_name, sql_add_xr_row, bound_parameter_strings, bound_parameter_ints, bound_parameter_which_binding_to_use, lower_range_end, upper_range_end, previous_x_columns, previous__DO_NOT_include_lower_range_data__DO_include_upper_range_data, current__DO_NOT_include_lower_range_data__DO_include_upper_range_data);
						sql_strings.push_back(SQLExecutor(db, sql_add_xr_row, bound_parameter_strings, bound_parameter_ints, bound_parameter_which_binding_to_use, the_prepared_stmt, true));
						the_prepared_stmt = sql_strings.back().stmt;
						++current_rows_added;
						++current_rows_added_since_execution;
					}

				}
				else
				{

					// The lower range ends higher than the upper range

					// First, add a row that includes all data,
					// setting new time range columns to:
					// upper_range_start - upper_range_end
					CreateNewXRRow(first_row_added, datetime_start_col_name, datetime_end_col_name, result_columns.view_name, sql_add_xr_row, bound_parameter_strings, bound_parameter_ints, bound_parameter_which_binding_to_use, upper_range_start, upper_range_end, previous_x_columns, previous__DO_include_lower_range_data__DO_include_upper_range_data, current__DO_include_lower_range_data__DO_include_upper_range_data);
					sql_strings.push_back(SQLExecutor(db, sql_add_xr_row, bound_parameter_strings, bound_parameter_ints, bound_parameter_which_binding_to_use, the_prepared_stmt, true));
					the_prepared_stmt = sql_strings.back().stmt;
					++current_rows_added;
					++current_rows_added_since_execution;


					// Second, add a row that includes only the lower range's data,
					// setting new time range columns to:
					// upper_range_end - lower_range_end
					if (!is_child_inner_table || previous_is_lower)
					{
						CreateNewXRRow(first_row_added, datetime_start_col_name, datetime_end_col_name, result_columns.view_name, sql_add_xr_row, bound_parameter_strings, bound_parameter_ints, bound_parameter_which_binding_to_use, upper_range_end, lower_range_end, previous_x_columns, previous__DO_include_lower_range_data__DO_NOT_include_upper_range_data, current__DO_include_lower_range_data__DO_NOT_include_upper_range_data);
						sql_strings.push_back(SQLExecutor(db, sql_add_xr_row, bound_parameter_strings, bound_parameter_ints, bound_parameter_which_binding_to_use, the_prepared_stmt, true));
						the_prepared_stmt = sql_strings.back().stmt;
						++current_rows_added;
						++current_rows_added_since_execution;
					}


				}

			}
			else
			{

				// The lower range is guaranteed to start
				// before the upper range starts

				if (lower_range_end <= upper_range_start)
				{

					// No overlap between the lower range and the upper range

					// First, add a row corresponding to the lower range,
					// setting new time range columns to:
					// lower_range_start - lower_range_end
					if (!is_child_inner_table || previous_is_lower)
					{
						CreateNewXRRow(first_row_added, datetime_start_col_name, datetime_end_col_name, result_columns.view_name, sql_add_xr_row, bound_parameter_strings, bound_parameter_ints, bound_parameter_which_binding_to_use, lower_range_start, lower_range_end, previous_x_columns, previous__DO_include_lower_range_data__DO_NOT_include_upper_range_data, current__DO_include_lower_range_data__DO_NOT_include_upper_range_data);
						sql_strings.push_back(SQLExecutor(db, sql_add_xr_row, bound_parameter_strings, bound_parameter_ints, bound_parameter_which_binding_to_use, the_prepared_stmt, true));
						the_prepared_stmt = sql_strings.back().stmt;
						++current_rows_added;
						++current_rows_added_since_execution;
					}

					// Second, add a row corresponding to the upper range,
					// setting new time range columns to:
					// upper_range_start - upper_range_end
					if (!is_child_inner_table || !previous_is_lower)
					{
						CreateNewXRRow(first_row_added, datetime_start_col_name, datetime_end_col_name, result_columns.view_name, sql_add_xr_row, bound_parameter_strings, bound_parameter_ints, bound_parameter_which_binding_to_use, upper_range_start, upper_range_end, previous_x_columns, previous__DO_NOT_include_lower_range_data__DO_include_upper_range_data, current__DO_NOT_include_lower_range_data__DO_include_upper_range_data);
						sql_strings.push_back(SQLExecutor(db, sql_add_xr_row, bound_parameter_strings, bound_parameter_ints, bound_parameter_which_binding_to_use, the_prepared_stmt, true));
						the_prepared_stmt = sql_strings.back().stmt;
						++current_rows_added;
						++current_rows_added_since_execution;
					}

				}
				else
				{

					// There is guaranteed to be overlap between the lower range
					// and the upper range

					// First, add a row to cover the region of the lower range
					// that is before the upper range begins,
					// i.e., including only the lower range's data,
					// setting new time range columns to:
					// lower_range_start - upper_range_start
					if (!is_child_inner_table || previous_is_lower)
					{
						CreateNewXRRow(first_row_added, datetime_start_col_name, datetime_end_col_name, result_columns.view_name, sql_add_xr_row, bound_parameter_strings, bound_parameter_ints, bound_parameter_which_binding_to_use, lower_range_start, upper_range_start, previous_x_columns, previous__DO_include_lower_range_data__DO_NOT_include_upper_range_data, current__DO_include_lower_range_data__DO_NOT_include_upper_range_data);
						sql_strings.push_back(SQLExecutor(db, sql_add_xr_row, bound_parameter_strings, bound_parameter_ints, bound_parameter_which_binding_to_use, the_prepared_stmt, true));
						the_prepared_stmt = sql_strings.back().stmt;
						++current_rows_added;
						++current_rows_added_since_execution;
					}

					if (lower_range_end == upper_range_end)
					{

						// special case: The lower range and the upper range
						// end at the same time value

						// So second, add a row that covers the entire upper range
						// that includes all data,
						// therefore setting new time range columns to:
						// upper_range_start - upper_range_end
						CreateNewXRRow(first_row_added, datetime_start_col_name, datetime_end_col_name, result_columns.view_name, sql_add_xr_row, bound_parameter_strings, bound_parameter_ints, bound_parameter_which_binding_to_use, upper_range_start, upper_range_end, previous_x_columns, previous__DO_include_lower_range_data__DO_include_upper_range_data, current__DO_include_lower_range_data__DO_include_upper_range_data);
						sql_strings.push_back(SQLExecutor(db, sql_add_xr_row, bound_parameter_strings, bound_parameter_ints, bound_parameter_which_binding_to_use, the_prepared_stmt, true));
						the_prepared_stmt = sql_strings.back().stmt;
						++current_rows_added;
						++current_rows_added_since_execution;

					}
					else if (lower_range_end < upper_range_end)
					{

						// The upper range ends higher than the lower range

						// So second, add a row that includes all data,
						// setting new time range columns to:
						// upper_range_start - lower_range_end
						CreateNewXRRow(first_row_added, datetime_start_col_name, datetime_end_col_name, result_columns.view_name, sql_add_xr_row, bound_parameter_strings, bound_parameter_ints, bound_parameter_which_binding_to_use, upper_range_start, lower_range_end, previous_x_columns, previous__DO_include_lower_range_data__DO_include_upper_range_data, current__DO_include_lower_range_data__DO_include_upper_range_data);
						sql_strings.push_back(SQLExecutor(db, sql_add_xr_row, bound_parameter_strings, bound_parameter_ints, bound_parameter_which_binding_to_use, the_prepared_stmt, true));
						the_prepared_stmt = sql_strings.back().stmt;
						++current_rows_added;
						++current_rows_added_since_execution;


						// And third, add a row that includes only the upper range's data,
						// setting new time range columns to:
						// lower_range_end - upper_range_end
						if (!is_child_inner_table || !previous_is_lower)
						{
							CreateNewXRRow(first_row_added, datetime_start_col_name, datetime_end_col_name, result_columns.view_name, sql_add_xr_row, bound_parameter_strings, bound_parameter_ints, bound_parameter_which_binding_to_use, lower_range_end, upper_range_end, previous_x_columns, previous__DO_NOT_include_lower_range_data__DO_include_upper_range_data, current__DO_NOT_include_lower_range_data__DO_include_upper_range_data);
							sql_strings.push_back(SQLExecutor(db, sql_add_xr_row, bound_parameter_strings, bound_parameter_ints, bound_parameter_which_binding_to_use, the_prepared_stmt, true));
							the_prepared_stmt = sql_strings.back().stmt;
							++current_rows_added;
							++current_rows_added_since_execution;
						}


					}
					else
					{

						// The lower range ends higher than the upper range

						// So second, add a row that covers the entire upper range
						// that includes all data,
						// therefore setting new time range columns to:
						// upper_range_start - upper_range_end
						CreateNewXRRow(first_row_added, datetime_start_col_name, datetime_end_col_name, result_columns.view_name, sql_add_xr_row, bound_parameter_strings, bound_parameter_ints, bound_parameter_which_binding_to_use, upper_range_start, upper_range_end, previous_x_columns, previous__DO_include_lower_range_data__DO_include_upper_range_data, current__DO_include_lower_range_data__DO_include_upper_range_data);
						sql_strings.push_back(SQLExecutor(db, sql_add_xr_row, bound_parameter_strings, bound_parameter_ints, bound_parameter_which_binding_to_use, the_prepared_stmt, true));
						the_prepared_stmt = sql_strings.back().stmt;
						++current_rows_added;
						++current_rows_added_since_execution;


						// And third, add a row that includes only the lower range's data,
						// setting new time range columns to:
						// upper_range_end - lower_range_end
						if (!is_child_inner_table || previous_is_lower)
						{
							CreateNewXRRow(first_row_added, datetime_start_col_name, datetime_end_col_name, result_columns.view_name, sql_add_xr_row, bound_parameter_strings, bound_parameter_ints, bound_parameter_which_binding_to_use, upper_range_end, lower_range_end, previous_x_columns, previous__DO_include_lower_range_data__DO_NOT_include_upper_range_data, current__DO_include_lower_range_data__DO_NOT_include_upper_range_data);
							sql_strings.push_back(SQLExecutor(db, sql_add_xr_row, bound_parameter_strings, bound_parameter_ints, bound_parameter_which_binding_to_use, the_prepared_stmt, true));
							the_prepared_stmt = sql_strings.back().stmt;
							++current_rows_added;
							++current_rows_added_since_execution;
						}

					}

				}

			}

		}

		if (current_rows_added_since_execution >= minimum_desired_rows_per_transaction)
		{
			ExecuteSQL(result);
			EndTransaction();
			BeginNewTransaction();
			current_rows_added_since_execution = 0;
		}

	}

	if (current_rows_added_since_execution > 0)
	{
		ExecuteSQL(result);
		EndTransaction();
	}
	else
	{
		EndTransaction();
	}

	if (failed)
	{
		return result;
	}

	return result;

}

void OutputModel::OutputGenerator::Prepare()
{

	failed = false;

	// If we ever switch to using the SQLite "temp" mechanism, utilize temp_dot
	//temp_dot = "temp.";
	temp_dot = "";

	input_model = &model->getInputModel();

	db = input_model->getDb();

	executor.db = db;

	PopulateUOAs();

	PopulateDMUCounts();

	ValidateUOAs();

	DetermineChildMultiplicitiesGreaterThanOne();

	PopulateVariableGroups();

	PopulatePrimaryKeySequenceInfo();

}

void OutputModel::OutputGenerator::ObtainColumnInfoForRawDataTables()
{

	int primary_view_count = 0;
	std::for_each(primary_variable_groups_vector.cbegin(), primary_variable_groups_vector.cend(), [this, &primary_view_count](std::pair<WidgetInstanceIdentifier, WidgetInstanceIdentifiers> const & the_primary_variable_group)
	{
		PopulateColumnsFromRawDataTable(the_primary_variable_group, primary_view_count, primary_variable_groups_column_info, true);
	});

	int secondary_view_count = 0;
	std::for_each(secondary_variable_groups_vector.cbegin(), secondary_variable_groups_vector.cend(), [this, &secondary_view_count](std::pair<WidgetInstanceIdentifier, WidgetInstanceIdentifiers> const & the_secondary_variable_group)
	{
		PopulateColumnsFromRawDataTable(the_secondary_variable_group, secondary_view_count, secondary_variable_groups_column_info, false);
	});

}

void OutputModel::OutputGenerator::PopulateColumnsFromRawDataTable(std::pair<WidgetInstanceIdentifier, WidgetInstanceIdentifiers> const & the_variable_group, int view_count, std::vector<ColumnsInTempView> & variable_groups_column_info, bool const & is_primary)
{

	// ************************************************************************************************ //
	// the_variable_group:
	// A pair: VG identifier -> Variables in this group selected by the user.
	// Note that even though only the variables selected by the user appear as the second member
	// of the pair, that nonetheless in this function we bypass this data structure
	// and retrieve the *full* set of columns from the vg_set_member table
	// via the VG identifier.
	// ************************************************************************************************ //

	// Convert data into a far more useful form for construction of K-adic output

	if (failed)
	{
		return;
	}

	std::string vg_code = *the_variable_group.first.code;
	std::string vg_data_table_name = Table_VariableGroupData::TableNameFromVGCode(vg_code);

	variable_groups_column_info.push_back(ColumnsInTempView());
	ColumnsInTempView & columns_in_variable_group_view = variable_groups_column_info.back();



	columns_in_variable_group_view.view_number = view_count;
	std::string view_name;
	view_name = vg_data_table_name;
	columns_in_variable_group_view.view_name_no_uuid = view_name;
	columns_in_variable_group_view.view_name = view_name;

	columns_in_variable_group_view.original_table_names.push_back(vg_data_table_name);
	columns_in_variable_group_view.variable_group_codes.push_back(*the_variable_group.first.code);
	columns_in_variable_group_view.variable_group_longhand_names.push_back(*the_variable_group.first.longhand);
	columns_in_variable_group_view.variable_groups.push_back(the_variable_group.first);

	WidgetInstanceIdentifiers & variables_in_group = input_model->t_vgp_setmembers.getIdentifiers(*the_variable_group.first.uuid);
	WidgetInstanceIdentifiers & variables_in_group_primary_keys_metadata = input_model->t_vgp_data_metadata__primary_keys.getIdentifiers(vg_data_table_name);

	std::set<WidgetInstanceIdentifier> variables_in_group_sorted;
	std::for_each(variables_in_group.cbegin(), variables_in_group.cend(), [&variables_in_group_sorted](WidgetInstanceIdentifier const & variable_group_set_member)
	{
		variables_in_group_sorted.insert(variable_group_set_member);
	});

	WidgetInstanceIdentifiers & datetime_columns = input_model->t_vgp_data_metadata__datetime_columns.getIdentifiers(vg_data_table_name);
	if (datetime_columns.size() > 0 && datetime_columns.size() != 2)
	{
		failed = true;
		return;
	}

	Table_UOA_Identifier::DMU_Counts dmu_counts_corresponding_to_uoa_for_current_primary_or_child_variable_group;
	std::for_each(biggest_counts.cbegin(), biggest_counts.cend(), [&dmu_counts_corresponding_to_uoa_for_current_primary_or_child_variable_group, &the_variable_group](std::pair<WidgetInstanceIdentifier, Table_UOA_Identifier::DMU_Counts> const & uoa_plus_dmu_counts)
	{
		if (uoa_plus_dmu_counts.first.IsEqual(WidgetInstanceIdentifier::EQUALITY_CHECK_TYPE__STRING_CODE, *the_variable_group.first.identifier_parent))
		{
			dmu_counts_corresponding_to_uoa_for_current_primary_or_child_variable_group = uoa_plus_dmu_counts.second;
		}
	});
	std::for_each(child_counts.cbegin(), child_counts.cend(), [&dmu_counts_corresponding_to_uoa_for_current_primary_or_child_variable_group, &the_variable_group](std::pair<WidgetInstanceIdentifier, Table_UOA_Identifier::DMU_Counts> const & uoa_plus_dmu_counts)
	{
		if (uoa_plus_dmu_counts.first.IsEqual(WidgetInstanceIdentifier::EQUALITY_CHECK_TYPE__STRING_CODE, *the_variable_group.first.identifier_parent))
		{
			dmu_counts_corresponding_to_uoa_for_current_primary_or_child_variable_group = uoa_plus_dmu_counts.second;
		}
	});

	Table_UOA_Identifier::DMU_Counts & dmu_counts_corresponding_to_top_level_uoa = biggest_counts[0].second;
	
	columns_in_variable_group_view.has_no_datetime_columns = false;
	columns_in_variable_group_view.has_no_datetime_columns_originally = false;
	if (datetime_columns.size() == 0)
	{
		columns_in_variable_group_view.has_no_datetime_columns = true;
		columns_in_variable_group_view.has_no_datetime_columns_originally = true;
	}

	std::for_each(variables_in_group_sorted.cbegin(), variables_in_group_sorted.cend(), [this, &dmu_counts_corresponding_to_top_level_uoa, &dmu_counts_corresponding_to_uoa_for_current_primary_or_child_variable_group, &is_primary, &columns_in_variable_group_view, &datetime_columns, &the_variable_group, &variables_in_group_primary_keys_metadata](WidgetInstanceIdentifier const & variable_group_set_member)
	{
		columns_in_variable_group_view.columns_in_view.push_back(ColumnsInTempView::ColumnInTempView());
		ColumnsInTempView::ColumnInTempView & column_in_variable_group_data_table = columns_in_variable_group_view.columns_in_view.back();

		std::string column_name_no_uuid = *variable_group_set_member.code;
		column_in_variable_group_data_table.column_name_in_temporary_table_no_uuid = column_name_no_uuid;

		std::string column_name = column_name_no_uuid;

		column_in_variable_group_data_table.column_name_in_temporary_table = column_name;

		column_in_variable_group_data_table.column_name_in_original_data_table = column_name_no_uuid;

		column_in_variable_group_data_table.variable_group_associated_with_current_inner_table = the_variable_group.first;

		if (!the_variable_group.first.identifier_parent)
		{
			failed = true;
			return;
		}

		column_in_variable_group_data_table.uoa_associated_with_variable_group_associated_with_current_inner_table = *the_variable_group.first.identifier_parent;

		// Is this a primary key field?
		bool primary_key_field = false;
		std::for_each(variables_in_group_primary_keys_metadata.cbegin(), variables_in_group_primary_keys_metadata.cend(), [&column_in_variable_group_data_table, &primary_key_field](WidgetInstanceIdentifier const & primary_key_in_variable_group_metadata)
		{
			if (boost::iequals(column_in_variable_group_data_table.column_name_in_original_data_table, *primary_key_in_variable_group_metadata.longhand))
			{
				primary_key_field = true;
			}
		});

		if (primary_key_field)
		{
			column_in_variable_group_data_table.column_type = ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__PRIMARY;
		}
		else
		{
			column_in_variable_group_data_table.column_type = ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__SECONDARY;
		}

		if (datetime_columns.size())
		{
			if (datetime_columns[0].code && variable_group_set_member.code && *datetime_columns[0].code == *variable_group_set_member.code)
			{
				// The current column is the datetime_start column
				column_in_variable_group_data_table.column_type = ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__DATETIMESTART;
			}
			else if (datetime_columns[1].code && variable_group_set_member.code && *datetime_columns[1].code == *variable_group_set_member.code)
			{
				// The current column is the datetime_end column
				column_in_variable_group_data_table.column_type = ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__DATETIMEEND;
			}
		}

		std::for_each(sequence.primary_key_sequence_info.cbegin(), sequence.primary_key_sequence_info.cend(), [this, &dmu_counts_corresponding_to_top_level_uoa, &dmu_counts_corresponding_to_uoa_for_current_primary_or_child_variable_group, &the_variable_group, &column_in_variable_group_data_table, &variables_in_group_primary_keys_metadata](PrimaryKeySequence::PrimaryKeySequenceEntry const & primary_key_entry__output__including_multiplicities)
		{

			int k_count__corresponding_to_top_level_uoa__and_current_dmu_category;
			std::for_each(dmu_counts_corresponding_to_top_level_uoa.cbegin(), dmu_counts_corresponding_to_top_level_uoa.cend(), [&k_count__corresponding_to_top_level_uoa__and_current_dmu_category, &primary_key_entry__output__including_multiplicities](Table_UOA_Identifier::DMU_Plus_Count const & dmu_plus_count)
			{
				if (dmu_plus_count.first.IsEqual(WidgetInstanceIdentifier::EQUALITY_CHECK_TYPE__STRING_CODE, primary_key_entry__output__including_multiplicities.dmu_category))
				{
					k_count__corresponding_to_top_level_uoa__and_current_dmu_category = dmu_plus_count.second;
				}
			});

			std::for_each(primary_key_entry__output__including_multiplicities.variable_group_info_for_primary_keys.cbegin(), primary_key_entry__output__including_multiplicities.variable_group_info_for_primary_keys.cend(), [this, &k_count__corresponding_to_top_level_uoa__and_current_dmu_category, &dmu_counts_corresponding_to_top_level_uoa, &dmu_counts_corresponding_to_uoa_for_current_primary_or_child_variable_group, &the_variable_group, &column_in_variable_group_data_table, &primary_key_entry__output__including_multiplicities, &variables_in_group_primary_keys_metadata](PrimaryKeySequence::VariableGroup_PrimaryKey_Info const & current_variable_group_primary_key_entry)
			{
				if (current_variable_group_primary_key_entry.vg_identifier.IsEqual(WidgetInstanceIdentifier::EQUALITY_CHECK_TYPE__STRING_CODE, the_variable_group.first))
				{
					if (!current_variable_group_primary_key_entry.column_name.empty())
					{
						if (boost::iequals(current_variable_group_primary_key_entry.table_column_name, column_in_variable_group_data_table.column_name_in_original_data_table))
						{

							bool matched = false;

							if (current_variable_group_primary_key_entry.current_multiplicity == 1)
							{
								// For the raw data table, there is only one instance of the primary keys associated with multiplicity greater than 1.
								// But the total primary key sequence ("sequence") stores the columns of the OUTPUT, which contains all multiplicities.
								// So use the first occurrence of the primary keys (current_multiplicity == 1) to obtain the information.
								matched = true;
							}
							else
							{
								// deal with child tables that have a smaller number of columns in this DMU category
								// than the primary variable groups do.
								if (current_variable_group_primary_key_entry.total_number_columns_for_dmu_category__internal_to_uoa_corresponding_to_this_variable_group
									<
									current_variable_group_primary_key_entry.total_number_columns_for_dmu_category__internal_to_the_uoa_corresponding_to_primary_uoa_for_the_same_dmu_category)
								{
									if (current_variable_group_primary_key_entry.current_multiplicity <= current_variable_group_primary_key_entry.total_number_columns_for_dmu_category__internal_to_the_uoa_corresponding_to_primary_uoa_for_the_same_dmu_category)
									{
										// The current column corresponds to the first inner table of the top-level variable groups,
										// though it corresponds to the second or greater inner table of a current child variable group.
										// i.e., this is the second or following call to this function corresponding to a second or
										// higher multiplicity of a child variable group.
										matched = true;
									}
								}
							}

							if (matched)
							{
								column_in_variable_group_data_table.primary_key_dmu_category_identifier = primary_key_entry__output__including_multiplicities.dmu_category;
								column_in_variable_group_data_table.primary_key_index_within_total_kad_for_dmu_category = primary_key_entry__output__including_multiplicities.sequence_number_within_dmu_category_spin_control;
								column_in_variable_group_data_table.primary_key_index__within_uoa_corresponding_to_variable_group_corresponding_to_current_inner_table__for_dmu_category = current_variable_group_primary_key_entry.sequence_number_within_dmu_category_for_this_variable_groups_uoa;
								column_in_variable_group_data_table.primary_key_index_within_primary_uoa_for_dmu_category = primary_key_entry__output__including_multiplicities.sequence_number_within_dmu_category_primary_uoa;
								column_in_variable_group_data_table.current_multiplicity__corresponding_to__current_inner_table = current_variable_group_primary_key_entry.current_multiplicity;
								column_in_variable_group_data_table.total_multiplicity__of_current_dmu_category__within_uoa_corresponding_to_the_current_inner_tables_variable_group = current_variable_group_primary_key_entry.total_multiplicity;
								column_in_variable_group_data_table.total_k_count__within_uoa_corresponding_to_top_level_variable_group__for_current_dmu_category = k_count__corresponding_to_top_level_uoa__and_current_dmu_category;

								std::for_each(dmu_counts_corresponding_to_uoa_for_current_primary_or_child_variable_group.cbegin(), dmu_counts_corresponding_to_uoa_for_current_primary_or_child_variable_group.cend(), [this, &column_in_variable_group_data_table, &primary_key_entry__output__including_multiplicities](Table_UOA_Identifier::DMU_Plus_Count const & dmu_count)
								{
									if (dmu_count.first.IsEqual(WidgetInstanceIdentifier::EQUALITY_CHECK_TYPE__STRING_CODE, primary_key_entry__output__including_multiplicities.dmu_category))
									{
										column_in_variable_group_data_table.total_k_count__within_uoa_corresponding_to_current_variable_group__for_current_dmu_category = dmu_count.second;
										WidgetInstanceIdentifier_Int_Pair Kad_Data = model->t_kad_count.getIdentifier(*primary_key_entry__output__including_multiplicities.dmu_category.uuid);
										column_in_variable_group_data_table.total_k_spin_count_across_multiplicities_for_dmu_category = Kad_Data.second;
									}
								});

								// Now determine if this primary key field should be treated as numeric for sorting and ordering
								std::for_each(variables_in_group_primary_keys_metadata.cbegin(), variables_in_group_primary_keys_metadata.cend(), [&column_in_variable_group_data_table, &primary_key_entry__output__including_multiplicities, &current_variable_group_primary_key_entry](WidgetInstanceIdentifier const & primary_key_in_variable_group_metadata)
								{
									if (boost::iequals(current_variable_group_primary_key_entry.table_column_name, *primary_key_in_variable_group_metadata.longhand))
									{
										if (primary_key_in_variable_group_metadata.flags == "n")
										{
											column_in_variable_group_data_table.primary_key_should_be_treated_as_numeric = true;
										}
									}
								});
							}

						}
					}
				}
			});
		});

	});

}

void OutputModel::OutputGenerator::PopulateDMUCounts()
{

	bool first = true;
	std::for_each(UOAs.cbegin(), UOAs.cend(), [this, &first](std::pair<WidgetInstanceIdentifier, Table_UOA_Identifier::DMU_Counts> const & uoa__to__dmu_counts__pair)
	{

		if (failed)
		{
			return; // from lambda
		}

		if (first)
		{
			first = false;
			biggest_counts.push_back(uoa__to__dmu_counts__pair);
			return; // from lambda
		}

		// Check if the current DMU_Counts overlaps (which is currently an error)
		// or is bigger
		Table_UOA_Identifier::DMU_Counts current_dmu_counts = uoa__to__dmu_counts__pair.second;
		bool current_is_bigger = false;
		bool current_is_smaller = false;
		bool current_is_same = true;

		int current = 0;
		std::for_each(current_dmu_counts.cbegin(), current_dmu_counts.cend(), [this, &current_is_bigger, &current_is_smaller, &current_is_same, &current](Table_UOA_Identifier::DMU_Plus_Count const & current_dmu_plus_count)
		{
			bool matched_current_dmu = false;
			// Looking at the first entry in biggest_counts is the same as looking at any other entry
			// in terms of the DMU counts
			std::for_each(biggest_counts[0].second.cbegin(), biggest_counts[0].second.cend(), [&matched_current_dmu, &current_dmu_plus_count, &current_is_bigger, &current_is_smaller, &current_is_same, &current](Table_UOA_Identifier::DMU_Plus_Count const & k_count_for_primary_uoa_for_given_dmu_category__info)
			{
				if (current_dmu_plus_count.first.IsEqual(WidgetInstanceIdentifier::EQUALITY_CHECK_TYPE__STRING_CODE, k_count_for_primary_uoa_for_given_dmu_category__info.first))
				{
					matched_current_dmu = true;
					if (current_dmu_plus_count.second > k_count_for_primary_uoa_for_given_dmu_category__info.second)
					{
						current_is_same = false;
						current_is_bigger = true;
					}
					else if (current_dmu_plus_count.second < k_count_for_primary_uoa_for_given_dmu_category__info.second)
					{
						current_is_same = false;
						current_is_smaller = true;
					}
				}
			});
			if (!matched_current_dmu)
			{
				// The DMU in the current UOA being tested does not exist in the "biggest" UOA previous to this
				current_is_same = false;
				current_is_bigger = true;
			}
			++current;
		});

		// Looking at the first entry in biggest_counts is the same as looking at any other entry
		// in terms of the DMU counts
		std::for_each(biggest_counts[0].second.cbegin(), biggest_counts[0].second.cend(), [&current_dmu_counts, &current_is_bigger, &current_is_smaller, &current_is_same](Table_UOA_Identifier::DMU_Plus_Count const & k_count_for_primary_uoa_for_given_dmu_category__info)
		{
			bool matched_biggest_dmu = false;
			std::for_each(current_dmu_counts.cbegin(), current_dmu_counts.cend(), [&matched_biggest_dmu, &k_count_for_primary_uoa_for_given_dmu_category__info, &current_is_bigger, &current_is_smaller, &current_is_same](Table_UOA_Identifier::DMU_Plus_Count const & current_dmu_plus_count)
			{
				if (current_dmu_plus_count.first.IsEqual(WidgetInstanceIdentifier::EQUALITY_CHECK_TYPE__STRING_CODE, k_count_for_primary_uoa_for_given_dmu_category__info.first))
				{
					matched_biggest_dmu = true;
				}
			});
			if (!matched_biggest_dmu)
			{
				// The DMU in the "biggest" UOA does not exist in the current UOA being tested
				current_is_same = false;
				current_is_smaller = true;
			}
		});

		if (current_is_same)
		{
			if (current_is_smaller || current_is_bigger)
			{
				// error in algorithm logic
				failed = true;
				return; // from lambda
			}
			else
			{
				// ambiguity: two UOA's are the same
				// This is just fine
				biggest_counts.push_back(uoa__to__dmu_counts__pair);
			}
		}
		else if (current_is_bigger && current_is_smaller)
		{
			// overlapping UOA's: not yet implemented
			// Todo: Error message
			failed = true;
			return; // from labmda
		}
		else if (current_is_bigger)
		{
			// Wipe the current UOA's and start fresh with this new, bigger one
			std::vector<std::pair<WidgetInstanceIdentifier, Table_UOA_Identifier::DMU_Counts>> new_child_uoas;
			biggest_counts.swap(new_child_uoas);
			child_counts.insert(child_counts.end(), new_child_uoas.cbegin(), new_child_uoas.cend());
			biggest_counts.push_back(uoa__to__dmu_counts__pair);
		}
		else if (current_is_smaller)
		{
			// Add to child UOA's here
			child_counts.push_back(uoa__to__dmu_counts__pair);
		}
		else
		{
			// error in algorithm logic
			failed = true;
			return; // from lambda
		}

	});

}

void OutputModel::OutputGenerator::ValidateUOAs()
{
	// Check for overlap in UOA's

	// For primary UOA:
	// Make sure that at most 1 DMU has a multiplicity greater than 1,
	// and save the name/index of the DMU category with the highest multiplicity.

	multiplicities_primary_uoa.clear();
	highest_multiplicity_primary_uoa = 0;
	highest_multiplicity_primary_uoa_dmu_string_code.clear();

	// Looking at the first entry in biggest_counts is the same as looking at any other entry
	// in terms of the DMU counts
	std::for_each(biggest_counts[0].second.cbegin(), biggest_counts[0].second.cend(), [this](Table_UOA_Identifier::DMU_Plus_Count const & k_count_for_primary_uoa_for_given_dmu_category__info)
	{
		if (failed)
		{
			return; // from lamda
		}
		WidgetInstanceIdentifier const & the_dmu_category = k_count_for_primary_uoa_for_given_dmu_category__info.first;
		if (!the_dmu_category.uuid || !the_dmu_category.code)
		{
			failed = true;
			return; // from lambda
		}
		WidgetInstanceIdentifier_Int_Pair kad_count_pair = this->model->t_kad_count.getIdentifier(*the_dmu_category.uuid);
		int uoa_count_current_dmu_category = k_count_for_primary_uoa_for_given_dmu_category__info.second;
		int kad_count_current_dmu_category = kad_count_pair.second;
		int multiplicity = 0;
		if (kad_count_current_dmu_category >= uoa_count_current_dmu_category)
		{
			multiplicity = 1;
		}
		if (multiplicity == 0)
		{
			// User's K-ad selection is too small in this DMU category to support the variables they have selected
			// Todo: Error message
			failed = true;
			return; // from lambda
		}
		int test_kad_count = uoa_count_current_dmu_category;
		while (test_kad_count <= kad_count_current_dmu_category)
		{
			test_kad_count += uoa_count_current_dmu_category;
			if (test_kad_count <= kad_count_current_dmu_category)
			{
				++multiplicity;
			}
		}
		multiplicities_primary_uoa.push_back(multiplicity);
		if (multiplicity > highest_multiplicity_primary_uoa)
		{
			highest_multiplicity_primary_uoa = multiplicity;
			highest_multiplicity_primary_uoa_dmu_string_code = *the_dmu_category.code;
		}
	});

	if (failed)
	{
		// Todo: Error message
		return;
	}

	// check that only 1 primary UOA's DMU category has multiplicity > 1 (for now)
	any_primary_dmu_has_multiplicity_greater_than_1 = false;
	which_primary_index_has_multiplicity_greater_than_1 = -1;

	int current_index = 0;
	std::for_each(multiplicities_primary_uoa.cbegin(), multiplicities_primary_uoa.cend(), [this, &current_index](int const & test_multiplicity)
	{
		if (failed)
		{
			return; // from lambda
		}
		if (test_multiplicity > 1)
		{
			if (any_primary_dmu_has_multiplicity_greater_than_1)
			{
				// ********************************************************************************************************************** //
				// A second DMU category's multiplicity is greater than 1 - for now, not allowed.  This can be implemented in the future.
				// ********************************************************************************************************************** //
				failed = true;
				return; // from lambda
			}
			any_primary_dmu_has_multiplicity_greater_than_1 = true;
			which_primary_index_has_multiplicity_greater_than_1 = current_index;
		}
		++current_index;
	});

	if (failed)
	{
		// Todo: Error message
		return;
	}

	// Validate child UOA's
	// For child UOA's:
	// Make sure that, for each child UOA, the UOA k-value for all DMU categories
	// is either 0, 1, or the corresponding UOA k-value of the primary UOA
	// and that where not 0, it is less than the corresponding UOA k-value of the primary UOA
	// (i.e., has a value of 1)
	// for only 1 DMU category,
	// and this DMU category must match the DMU category with multiplicity greater than 1 (if any)
	// for the primary UOAs
	std::for_each(child_counts.cbegin(), child_counts.cend(), [this](std::pair<WidgetInstanceIdentifier, Table_UOA_Identifier::DMU_Counts> const & uoa__to__dmu_counts__pair)
	{
		if (failed)
		{
			return; // from lambda
		}
		int primary_dmu_categories_for_which_child_has_less = 0;
		Table_UOA_Identifier::DMU_Counts const & current_dmu_counts = uoa__to__dmu_counts__pair.second;
		std::for_each(current_dmu_counts.cbegin(), current_dmu_counts.cend(), [this, &primary_dmu_categories_for_which_child_has_less](Table_UOA_Identifier::DMU_Plus_Count const & current_dmu_plus_count)
		{
			if (failed)
			{
				return; // from lambda
			}
			if (current_dmu_plus_count.second == 0)
			{
				// just fine
				return; // from lambda
			}
			std::for_each(biggest_counts[0].second.cbegin(), biggest_counts[0].second.cend(), [this, &current_dmu_plus_count, &primary_dmu_categories_for_which_child_has_less](Table_UOA_Identifier::DMU_Plus_Count const & k_count_for_primary_uoa_for_given_dmu_category__info)
			{
				if (failed)
				{
					return; // from lambda
				}
				if (current_dmu_plus_count.first.IsEqual(WidgetInstanceIdentifier::EQUALITY_CHECK_TYPE__STRING_CODE, k_count_for_primary_uoa_for_given_dmu_category__info.first))
				{
					if (current_dmu_plus_count.second == k_count_for_primary_uoa_for_given_dmu_category__info.second) // biggest_dmu_plus_count.second is the K-value of the unit of analysis, not the K-value chosen by the user in the spin control
					{
						if (highest_multiplicity_primary_uoa > 1)
						{
							// Special case:
							// The child UOA has the same K-value in this DMU category
							// as the K-value of the primary UOA,
							// but the K-value chosen by the user in the spin control for this DMU category
							// has incremented the multiplicity of the primary UOA for this DMU category greater than 1.
							++primary_dmu_categories_for_which_child_has_less;
						}
						return; // from lambda
					}
					if (current_dmu_plus_count.second > 1)
					{
						// Todo: error message
						// Invalid child UOA for this output
						failed = true;
						return; // from lambda
					}
					// Redundant 0-check for future code foolproofing
					else if (current_dmu_plus_count.second == 0)
					{
						// just fine
						return; // from lambda
					}
					// Current UOA's current DMU category's K-value is 1
					else if (!boost::iequals(*current_dmu_plus_count.first.code, highest_multiplicity_primary_uoa_dmu_string_code))
					{
						if (highest_multiplicity_primary_uoa > 1)
						{
							// Todo: error message

							// *************************************************************************************** //
							// Subtle edge case failure:
							//
							// Is this edge case common enough that it should be implemented for version 1 of NewGene?
							//
							// The current child DMU category has K-value less than the K-value
							// of the primary UOA's corresponding DMU category,
							// AND this is not the DMU category for which the primary UOA has
							// multiplicity greater than 1
							// *************************************************************************************** //

							// Invalid child UOA for this output
							failed = true;
							return; // from lambda
						}
					}
					++primary_dmu_categories_for_which_child_has_less;
				}
			});
		});

		if (primary_dmu_categories_for_which_child_has_less > 1)
		{
			// Todo: error message
			// Invalid child UOA for this output
			failed = true;
			return; // from lambda
		}

	});
}

void OutputModel::OutputGenerator::PopulateUOAs()
{
	std::for_each(the_map->cbegin(), the_map->cend(), [this](std::pair<WidgetInstanceIdentifier /* UOA identifier */,
		Table_VARIABLES_SELECTED::VariableGroup_To_VariableSelections_Map> /* map: VG identifier => List of variables */
		const & uoa__to__variable_groups__pair)
	{
		if (failed)
		{
			return; // from lambda
		}
		WidgetInstanceIdentifier const & uoa = uoa__to__variable_groups__pair.first;
		if (!uoa.uuid)
		{
			failed = true;
			return; // from lambda
		}
		Table_UOA_Identifier::DMU_Counts dmu_counts = input_model->t_uoa_category.RetrieveDMUCounts(input_model->getDb(), input_model, *uoa.uuid);
		UOAs.push_back(std::make_pair(uoa, dmu_counts));
	});
}

void OutputModel::OutputGenerator::DetermineChildMultiplicitiesGreaterThanOne()
{
	// For child UOA's:
	// Save the name/index of the DMU category multiplicity greater than one.
	// Currently, this can be only one of the DMU categories.
	std::for_each(child_counts.cbegin(), child_counts.cend(), [this](std::pair<WidgetInstanceIdentifier, Table_UOA_Identifier::DMU_Counts> const & child_uoa__dmu_counts__pair)
	{

		if (failed)
		{
			return; // from lambda
		}

		WidgetInstanceIdentifier uoa_identifier = child_uoa__dmu_counts__pair.first;
		bool first = true;
		std::for_each(child_uoa__dmu_counts__pair.second.cbegin(), child_uoa__dmu_counts__pair.second.cend(), [this, &uoa_identifier, &first](Table_UOA_Identifier::DMU_Plus_Count const & dmu_category)
		{

			if (failed)
			{
				return; // from lamda
			}

			WidgetInstanceIdentifier const & the_dmu_category_identifier = dmu_category.first;
			if (!the_dmu_category_identifier.uuid || !the_dmu_category_identifier.code)
			{
				failed = true;
				return; // from lambda
			}

			WidgetInstanceIdentifier_Int_Pair kad_count_pair = this->model->t_kad_count.getIdentifier(*the_dmu_category_identifier.uuid);
			int uoa_k_count__current_dmu_category = dmu_category.second;
			int k_spin_control_count__current_dmu_category = kad_count_pair.second;
			int multiplicity = 0;

			if (k_spin_control_count__current_dmu_category >= uoa_k_count__current_dmu_category)
			{
				multiplicity = 1;
			}

			if (multiplicity == 0)
			{
				// User's K-ad selection is too small in this DMU category to support the variables they have selected
				// Todo: Error message
				failed = true;
				return; // from lambda
			}

			int test_kad_count = uoa_k_count__current_dmu_category;
			while (test_kad_count <= k_spin_control_count__current_dmu_category)
			{
				test_kad_count += uoa_k_count__current_dmu_category;
				if (test_kad_count <= k_spin_control_count__current_dmu_category)
				{
					++multiplicity;
				}
			}

			// Note: Validation code has already validated that there is only 1 DMU category for which the multiplicity is greater than 1.
			// Always add at least one entry, even if multiplicity is 1.
			// This is to support the way things are currently designed, which requires
			// that there be an entry here in the special case that all multiplicities are 1.
			if (first || multiplicity > 1)
			{
				child_uoas__which_multiplicity_is_greater_than_1[uoa_identifier] = std::make_pair(the_dmu_category_identifier, multiplicity);
				first = false;
			}

		});

	});
}

void OutputModel::OutputGenerator::PopulateVariableGroups()
{
	std::for_each(biggest_counts.cbegin(), biggest_counts.cend(), [this](std::pair<WidgetInstanceIdentifier, Table_UOA_Identifier::DMU_Counts> const & uoa__to__dmu_category_counts)
	{
		// Get all the variable groups corresponding to the primary UOA (identical except possibly for time granularity)
		Table_VARIABLES_SELECTED::VariableGroup_To_VariableSelections_Map const & variable_groups_map_current = (*the_map)[uoa__to__dmu_category_counts.first];
		primary_variable_groups_vector.insert(primary_variable_groups_vector.end(), variable_groups_map_current.cbegin(), variable_groups_map_current.cend());
	});
	std::for_each(child_counts.cbegin(), child_counts.cend(), [this](std::pair<WidgetInstanceIdentifier, Table_UOA_Identifier::DMU_Counts> const & uoa__to__dmu_category_counts)
	{
		Table_VARIABLES_SELECTED::VariableGroup_To_VariableSelections_Map const & variable_groups_map_current = (*the_map)[uoa__to__dmu_category_counts.first];
		secondary_variable_groups_vector.insert(secondary_variable_groups_vector.end(), variable_groups_map_current.cbegin(), variable_groups_map_current.cend());
	});
}

void OutputModel::OutputGenerator::PopulatePrimaryKeySequenceInfo()
{
	int overall_primary_key_sequence_number = 0;
	std::for_each(biggest_counts[0].second.cbegin(), biggest_counts[0].second.cend(), [this, &overall_primary_key_sequence_number](Table_UOA_Identifier::DMU_Plus_Count const & k_count_for_primary_uoa_for_given_dmu_category__info)
	{
		if (failed)
		{
			return; // from lamda
		}
		WidgetInstanceIdentifier const & the_dmu_category = k_count_for_primary_uoa_for_given_dmu_category__info.first;
		if (!the_dmu_category.uuid || !the_dmu_category.code)
		{
			failed = true;
			return; // from lambda
		}
		WidgetInstanceIdentifier_Int_Pair kad_count_pair = this->model->t_kad_count.getIdentifier(*the_dmu_category.uuid);
		int k_count_for_primary_uoa_for_given_dmu_category = k_count_for_primary_uoa_for_given_dmu_category__info.second;
		int total_spin_control_k_count_for_given_dmu_category = kad_count_pair.second;

		int k_sequence_number_count_for_given_dmu_category_out_of_k_count_for_primary_uoa_for_that_dmu_category = 0;
		for (int k_sequence_number_count_for_given_dmu_category_out_of_total_spin_count_for_that_dmu_category = 0; k_sequence_number_count_for_given_dmu_category_out_of_total_spin_count_for_that_dmu_category < total_spin_control_k_count_for_given_dmu_category; ++k_sequence_number_count_for_given_dmu_category_out_of_total_spin_count_for_that_dmu_category)
		{
			if (k_sequence_number_count_for_given_dmu_category_out_of_k_count_for_primary_uoa_for_that_dmu_category == k_count_for_primary_uoa_for_given_dmu_category)
			{
				k_sequence_number_count_for_given_dmu_category_out_of_k_count_for_primary_uoa_for_that_dmu_category = 0;
			}

			sequence.primary_key_sequence_info.push_back(PrimaryKeySequence::PrimaryKeySequenceEntry());
			PrimaryKeySequence::PrimaryKeySequenceEntry & current_primary_key_sequence = sequence.primary_key_sequence_info.back();
			std::vector<PrimaryKeySequence::VariableGroup_PrimaryKey_Info> & variable_group_info_for_primary_keys = current_primary_key_sequence.variable_group_info_for_primary_keys;

			current_primary_key_sequence.dmu_category = the_dmu_category;
			current_primary_key_sequence.sequence_number_within_dmu_category_spin_control = k_sequence_number_count_for_given_dmu_category_out_of_total_spin_count_for_that_dmu_category;
			current_primary_key_sequence.sequence_number_within_dmu_category_primary_uoa = k_sequence_number_count_for_given_dmu_category_out_of_k_count_for_primary_uoa_for_that_dmu_category;
			current_primary_key_sequence.sequence_number_in_all_primary_keys = overall_primary_key_sequence_number;
			current_primary_key_sequence.total_k_count_within_high_level_variable_group_uoa_for_this_dmu_category = k_count_for_primary_uoa_for_given_dmu_category;
			current_primary_key_sequence.total_kad_spin_count_for_this_dmu_category = total_spin_control_k_count_for_given_dmu_category;

			std::map<WidgetInstanceIdentifier, int> map__dmu_category__total_multiplicity_of_dmu_category_in_primary_uoa_corresponding_to_top_level_variable_group;

			int view_count = 0;
			std::for_each(primary_variable_groups_vector.cbegin(), primary_variable_groups_vector.cend(), [this, &the_dmu_category, &k_sequence_number_count_for_given_dmu_category_out_of_total_spin_count_for_that_dmu_category, &k_count_for_primary_uoa_for_given_dmu_category, &total_spin_control_k_count_for_given_dmu_category, &current_primary_key_sequence, &variable_group_info_for_primary_keys, &map__dmu_category__total_multiplicity_of_dmu_category_in_primary_uoa_corresponding_to_top_level_variable_group](std::pair<WidgetInstanceIdentifier, WidgetInstanceIdentifiers> const & the_variable_group)
			{
				if (failed)
				{
					return; // from lambda
				}

				if (!the_variable_group.first.code)
				{
					// Todo: error message
					failed = true;
					return;
				}

				if (!the_variable_group.first.identifier_parent)
				{
					// Todo: error message
					failed = true;
					return;
				}

				WidgetInstanceIdentifier current_uoa_identifier = *the_variable_group.first.identifier_parent;

				std::string vg_code = *the_variable_group.first.code;
				std::string vg_data_table_name = Table_VariableGroupData::TableNameFromVGCode(vg_code);

				// dmu_primary_key_codes_metadata:
				// The primary key metadata for the_variable_group - a possible SUBSET of all primary keys from the primary UOA.
				// This include *all* DMU categories that form the primary keys for this variable group,
				// each of which might appear multiple times as a separate entry here.
				// This metadata just states which DMU category the key refers to,
				// what the total (overall) sequence number is of the primary key in the variable group table,
				// and the column name in the variable group table for this column.
				// In the WidgetInstanceIdentifier, the CODE is set to the DMU category code,
				// the LONGHAND is set to the column name corresponding to this DMU in the variable group data table,
				// and the SEQUENCE NUMBER is set to the sequence number of the primary key in this variable group.
				WidgetInstanceIdentifiers const & dmu_category_metadata__for_current_primary_or_child_uoa = input_model->t_vgp_data_metadata__primary_keys.getIdentifiers(vg_data_table_name);

				// Todo: To implement global variables (i.e., variables with no primary key),
				// make the necessary changes and then remove the following requirement
				if (dmu_category_metadata__for_current_primary_or_child_uoa.size() == 0)
				{
					// Todo: error message
					failed = true;
					return;
				}

				int total_number_columns_in_given_dmu_category_for_uoa_corresponding_to_current_variable_group = 0;
				std::for_each(dmu_category_metadata__for_current_primary_or_child_uoa.cbegin(), dmu_category_metadata__for_current_primary_or_child_uoa.cend(), [this, the_dmu_category, &total_number_columns_in_given_dmu_category_for_uoa_corresponding_to_current_variable_group, &k_sequence_number_count_for_given_dmu_category_out_of_total_spin_count_for_that_dmu_category, &k_count_for_primary_uoa_for_given_dmu_category, &total_spin_control_k_count_for_given_dmu_category, &current_primary_key_sequence, &variable_group_info_for_primary_keys](WidgetInstanceIdentifier const & current_primary_or_child_variable_group__current_dmu_category__primary_key_instance)
				{
					if (current_primary_or_child_variable_group__current_dmu_category__primary_key_instance.IsEqual(WidgetInstanceIdentifier::EQUALITY_CHECK_TYPE__STRING_CODE, the_dmu_category))
					{
						++total_number_columns_in_given_dmu_category_for_uoa_corresponding_to_current_variable_group;
					}
				});

				int multiplicity_current_primary_or_child_uoa = 0;
				if (total_number_columns_in_given_dmu_category_for_uoa_corresponding_to_current_variable_group > 0)
				{
					multiplicity_current_primary_or_child_uoa = 1;
					int test_kad_count = total_number_columns_in_given_dmu_category_for_uoa_corresponding_to_current_variable_group;
					while (test_kad_count <= total_spin_control_k_count_for_given_dmu_category)
					{
						test_kad_count += total_number_columns_in_given_dmu_category_for_uoa_corresponding_to_current_variable_group;
						if (test_kad_count <= total_spin_control_k_count_for_given_dmu_category)
						{
							++multiplicity_current_primary_or_child_uoa;
						}
					}
				}

				// The current iteration is through primary variable groups
				current_primary_key_sequence.total_multiplicity_of_uoa_corresponding_to_top_level_variable_group_for_the_current_dmu_category = multiplicity_current_primary_or_child_uoa;
				map__dmu_category__total_multiplicity_of_dmu_category_in_primary_uoa_corresponding_to_top_level_variable_group[the_dmu_category] = multiplicity_current_primary_or_child_uoa;

				variable_group_info_for_primary_keys.push_back(PrimaryKeySequence::VariableGroup_PrimaryKey_Info());
				PrimaryKeySequence::VariableGroup_PrimaryKey_Info & current_variable_group_current_primary_key_info = variable_group_info_for_primary_keys.back();
				current_variable_group_current_primary_key_info.vg_identifier = the_variable_group.first;
				current_variable_group_current_primary_key_info.is_primary_column_selected = false;
				current_variable_group_current_primary_key_info.associated_uoa_identifier = current_uoa_identifier;

				int current_variable_group_current_primary_key_dmu_category__total_sequence_number = 0;
				for (int m=0; m<multiplicity_current_primary_or_child_uoa; ++m)
				{
					int current_variable_group_current_primary_key_dmu_category__internal_sequence_number = 0;
					std::for_each(dmu_category_metadata__for_current_primary_or_child_uoa.cbegin(), dmu_category_metadata__for_current_primary_or_child_uoa.cend(), [this, &m, &multiplicity_current_primary_or_child_uoa, &current_variable_group_current_primary_key_info, &the_variable_group, &current_variable_group_current_primary_key_dmu_category__total_sequence_number, &the_dmu_category, &current_variable_group_current_primary_key_dmu_category__internal_sequence_number, &k_sequence_number_count_for_given_dmu_category_out_of_total_spin_count_for_that_dmu_category, &k_count_for_primary_uoa_for_given_dmu_category, &total_spin_control_k_count_for_given_dmu_category, &current_primary_key_sequence, &variable_group_info_for_primary_keys](WidgetInstanceIdentifier const & current_variable_group__current_dmu_primary_key_instance)
					{
						if (failed)
						{
							return; // from lambda
						}
						if (current_variable_group__current_dmu_primary_key_instance.IsEqual(WidgetInstanceIdentifier::EQUALITY_CHECK_TYPE__STRING_CODE, the_dmu_category))
						{
							if (k_sequence_number_count_for_given_dmu_category_out_of_total_spin_count_for_that_dmu_category == current_variable_group_current_primary_key_dmu_category__total_sequence_number)
							{
								// In the WidgetInstanceIdentifier, the CODE is set to the DMU category code,
								// the LONGHAND is set to the column name corresponding to this DMU in the variable group data table,
								// and the SEQUENCE NUMBER is set to the sequence number of the primary key in this variable group.
								current_variable_group_current_primary_key_info.table_column_name = *current_variable_group__current_dmu_primary_key_instance.longhand;
								current_variable_group_current_primary_key_info.sequence_number_within_dmu_category_for_this_variable_groups_uoa = current_variable_group_current_primary_key_dmu_category__internal_sequence_number;
								current_variable_group_current_primary_key_info.current_multiplicity = m+1;
								current_variable_group_current_primary_key_info.total_multiplicity = multiplicity_current_primary_or_child_uoa;
								current_variable_group_current_primary_key_info.total_multiplicity_of_dmu_category_in_primary_uoa_corresponding_to_top_level_variable_group = current_primary_key_sequence.total_multiplicity_of_uoa_corresponding_to_top_level_variable_group_for_the_current_dmu_category;
								current_variable_group_current_primary_key_info.total_number_columns_for_dmu_category__internal_to_the_uoa_corresponding_to_primary_uoa_for_the_same_dmu_category = k_count_for_primary_uoa_for_given_dmu_category;

								// We are currently iterating through primary variable groups, so this is easy
								current_variable_group_current_primary_key_info.total_number_columns_for_dmu_category__internal_to_uoa_corresponding_to_this_variable_group = k_count_for_primary_uoa_for_given_dmu_category;

								char ns__[256];

								std::string this_variable_group__this_primary_key__unique_name;
								this_variable_group__this_primary_key__unique_name += *current_variable_group__current_dmu_primary_key_instance.longhand;
								current_variable_group_current_primary_key_info.column_name_no_uuid = this_variable_group__this_primary_key__unique_name;
								if (multiplicity_current_primary_or_child_uoa > 1)
								{
									this_variable_group__this_primary_key__unique_name += "_";
									this_variable_group__this_primary_key__unique_name += itoa(m+1, ns__, 10);
								}
								this_variable_group__this_primary_key__unique_name += "_";
								this_variable_group__this_primary_key__unique_name += newUUID(true);
								current_variable_group_current_primary_key_info.column_name = this_variable_group__this_primary_key__unique_name;
								WidgetInstanceIdentifier vg_setmember_identifier;
								bool found_variable_group_set_member_identifier = input_model->t_vgp_setmembers.getIdentifierFromStringCodeAndParentUUID(*current_variable_group__current_dmu_primary_key_instance.longhand, *the_variable_group.first.uuid, vg_setmember_identifier);
								if (!found_variable_group_set_member_identifier)
								{
									failed = true;
									return; // from lambda
								}

								// Is this primary key selected by the user for output?
								bool found = false;
								std::for_each(the_variable_group.second.cbegin(), the_variable_group.second.cend(), [&found, &vg_setmember_identifier](WidgetInstanceIdentifier const & selected_variable_identifier)
								{
									if (found)
									{
										return; // from lambda
									}
									if (selected_variable_identifier.IsEqual(WidgetInstanceIdentifier::EQUALITY_CHECK_TYPE__UUID_PLUS_STRING_CODE, vg_setmember_identifier))
									{
										found = true;
									}
								});
								if (found)
								{
									current_variable_group_current_primary_key_info.is_primary_column_selected = true;
								}
								else
								{
									current_variable_group_current_primary_key_info.is_primary_column_selected = false;
								}
							}
							++current_variable_group_current_primary_key_dmu_category__internal_sequence_number;
							++current_variable_group_current_primary_key_dmu_category__total_sequence_number;
						}
					});
				}

				if (failed)
				{
					return; // from lambda
				}
			});

			if (failed)
			{
				return; // from lambda
			}

			view_count = 0;
			std::for_each(secondary_variable_groups_vector.cbegin(), secondary_variable_groups_vector.cend(), [this, &the_dmu_category, &k_sequence_number_count_for_given_dmu_category_out_of_total_spin_count_for_that_dmu_category, &k_count_for_primary_uoa_for_given_dmu_category, &total_spin_control_k_count_for_given_dmu_category, &current_primary_key_sequence, &variable_group_info_for_primary_keys, &map__dmu_category__total_multiplicity_of_dmu_category_in_primary_uoa_corresponding_to_top_level_variable_group](std::pair<WidgetInstanceIdentifier, WidgetInstanceIdentifiers> const & the_variable_group)
			{
				if (failed)
				{
					return; // from lambda
				}

				if (!the_variable_group.first.code)
				{
					// Todo: error message
					failed = true;
					return;
				}

				if (!the_variable_group.first.identifier_parent)
				{
					// Todo: error message
					failed = true;
					return;
				}

				WidgetInstanceIdentifier current_uoa_identifier = *the_variable_group.first.identifier_parent;

				std::string vg_code = *the_variable_group.first.code;
				std::string vg_data_table_name = Table_VariableGroupData::TableNameFromVGCode(vg_code);

				// dmu_primary_key_codes_metadata:
				// The primary key metadata for the_variable_group - a possible SUBSET of all primary keys from the primary UOA.
				// This include *all* DMU categories that form the primary keys for this variable group,
				// each of which might appear multiple times as a separate entry here.
				// This metadata just states which DMU category the key refers to,
				// what the total (overall) sequence number is of the primary key in the variable group table,
				// and the column name in the variable group table for this column.
				// In the WidgetInstanceIdentifier, the CODE is set to the DMU category code,
				// the LONGHAND is set to the column name corresponding to this DMU in the variable group data table,
				// and the SEQUENCE NUMBER is set to the sequence number of the primary key in this variable group.
				WidgetInstanceIdentifiers const & dmu_category_metadata__for_current_primary_or_child_uoa = input_model->t_vgp_data_metadata__primary_keys.getIdentifiers(vg_data_table_name);

				// Todo: To implement global variables (i.e., variables with no primary key),
				// make the necessary changes and then remove the following requirement
				if (dmu_category_metadata__for_current_primary_or_child_uoa.size() == 0)
				{
					// Todo: error message
					failed = true;
					return;
				}

				int total_number_columns_in_given_dmu_category_for_uoa_corresponding_to_current_variable_group = 0;
				std::for_each(dmu_category_metadata__for_current_primary_or_child_uoa.cbegin(), dmu_category_metadata__for_current_primary_or_child_uoa.cend(), [this, the_dmu_category, &total_number_columns_in_given_dmu_category_for_uoa_corresponding_to_current_variable_group, &k_sequence_number_count_for_given_dmu_category_out_of_total_spin_count_for_that_dmu_category, &k_count_for_primary_uoa_for_given_dmu_category, &total_spin_control_k_count_for_given_dmu_category, &current_primary_key_sequence, &variable_group_info_for_primary_keys](WidgetInstanceIdentifier const & current_primary_or_child_variable_group__current_dmu_category__primary_key_instance)
				{
					if (current_primary_or_child_variable_group__current_dmu_category__primary_key_instance.IsEqual(WidgetInstanceIdentifier::EQUALITY_CHECK_TYPE__STRING_CODE, the_dmu_category))
					{
						++total_number_columns_in_given_dmu_category_for_uoa_corresponding_to_current_variable_group;
					}
				});

				int multiplicity_current_primary_or_child_uoa = 0;
				if (total_number_columns_in_given_dmu_category_for_uoa_corresponding_to_current_variable_group > 0)
				{
					multiplicity_current_primary_or_child_uoa = 1;
					int test_kad_count = total_number_columns_in_given_dmu_category_for_uoa_corresponding_to_current_variable_group;
					while (test_kad_count <= total_spin_control_k_count_for_given_dmu_category)
					{
						test_kad_count += total_number_columns_in_given_dmu_category_for_uoa_corresponding_to_current_variable_group;
						if (test_kad_count <= total_spin_control_k_count_for_given_dmu_category)
						{
							++multiplicity_current_primary_or_child_uoa;
						}
					}
				}

				variable_group_info_for_primary_keys.push_back(PrimaryKeySequence::VariableGroup_PrimaryKey_Info());
				PrimaryKeySequence::VariableGroup_PrimaryKey_Info & current_variable_group_current_primary_key_info = variable_group_info_for_primary_keys.back();
				current_variable_group_current_primary_key_info.vg_identifier = the_variable_group.first;
				current_variable_group_current_primary_key_info.is_primary_column_selected = false;
				current_variable_group_current_primary_key_info.associated_uoa_identifier = current_uoa_identifier;
				current_variable_group_current_primary_key_info.total_multiplicity_of_dmu_category_in_primary_uoa_corresponding_to_top_level_variable_group = map__dmu_category__total_multiplicity_of_dmu_category_in_primary_uoa_corresponding_to_top_level_variable_group[the_dmu_category];

				int current_variable_group_current_primary_key_dmu_category__total_sequence_number = 0;
				for (int m=0; m<multiplicity_current_primary_or_child_uoa; ++m)
				{
					int current_variable_group_current_primary_key_dmu_category__internal_sequence_number = 0;
					std::for_each(dmu_category_metadata__for_current_primary_or_child_uoa.cbegin(), dmu_category_metadata__for_current_primary_or_child_uoa.cend(), [this, &m, &multiplicity_current_primary_or_child_uoa, &current_variable_group_current_primary_key_info, &the_variable_group, &current_variable_group_current_primary_key_dmu_category__total_sequence_number, &the_dmu_category, &current_variable_group_current_primary_key_dmu_category__internal_sequence_number, &k_sequence_number_count_for_given_dmu_category_out_of_total_spin_count_for_that_dmu_category, &k_count_for_primary_uoa_for_given_dmu_category, &total_spin_control_k_count_for_given_dmu_category, &current_primary_key_sequence, &variable_group_info_for_primary_keys, &total_number_columns_in_given_dmu_category_for_uoa_corresponding_to_current_variable_group](WidgetInstanceIdentifier const & current_variable_group_current_dmu_primary_key)
					{
						if (failed)
						{
							return; // from lambda
						}
						if (current_variable_group_current_dmu_primary_key.IsEqual(WidgetInstanceIdentifier::EQUALITY_CHECK_TYPE__STRING_CODE, the_dmu_category))
						{
							if (k_sequence_number_count_for_given_dmu_category_out_of_total_spin_count_for_that_dmu_category == current_variable_group_current_primary_key_dmu_category__total_sequence_number)
							{
								// In the WidgetInstanceIdentifier, the CODE is set to the DMU category code,
								// the LONGHAND is set to the column name corresponding to this DMU in the variable group data table,
								// and the SEQUENCE NUMBER is set to the sequence number of the primary key in this variable group.
								current_variable_group_current_primary_key_info.table_column_name = *current_variable_group_current_dmu_primary_key.longhand;
								current_variable_group_current_primary_key_info.sequence_number_within_dmu_category_for_this_variable_groups_uoa = current_variable_group_current_primary_key_dmu_category__internal_sequence_number;
								current_variable_group_current_primary_key_info.current_multiplicity = m+1;
								current_variable_group_current_primary_key_info.total_multiplicity = multiplicity_current_primary_or_child_uoa;
								current_variable_group_current_primary_key_info.total_multiplicity_of_dmu_category_in_primary_uoa_corresponding_to_top_level_variable_group = current_primary_key_sequence.total_multiplicity_of_uoa_corresponding_to_top_level_variable_group_for_the_current_dmu_category;
								current_variable_group_current_primary_key_info.total_number_columns_for_dmu_category__internal_to_the_uoa_corresponding_to_primary_uoa_for_the_same_dmu_category = k_count_for_primary_uoa_for_given_dmu_category;
								current_variable_group_current_primary_key_info.total_number_columns_for_dmu_category__internal_to_uoa_corresponding_to_this_variable_group = total_number_columns_in_given_dmu_category_for_uoa_corresponding_to_current_variable_group;

								char ns__[256];

								std::string this_variable_group__this_primary_key__unique_name;
								this_variable_group__this_primary_key__unique_name += *current_variable_group_current_dmu_primary_key.longhand;
								current_variable_group_current_primary_key_info.column_name_no_uuid = this_variable_group__this_primary_key__unique_name;
								if (multiplicity_current_primary_or_child_uoa > 1)
								{
									this_variable_group__this_primary_key__unique_name += "_";
									this_variable_group__this_primary_key__unique_name += itoa(m+1, ns__, 10);
								}
								this_variable_group__this_primary_key__unique_name += "_";
								this_variable_group__this_primary_key__unique_name += newUUID(true);
								current_variable_group_current_primary_key_info.column_name = this_variable_group__this_primary_key__unique_name;
								WidgetInstanceIdentifier vg_setmember_identifier;
								bool found_variable_group_set_member_identifier = input_model->t_vgp_setmembers.getIdentifierFromStringCodeAndParentUUID(*current_variable_group_current_dmu_primary_key.longhand, *the_variable_group.first.uuid, vg_setmember_identifier);
								if (!found_variable_group_set_member_identifier)
								{
									failed = true;
									return; // from lambda
								}

								// Is this primary key selected by the user for output?
								bool found = false;
								std::for_each(the_variable_group.second.cbegin(), the_variable_group.second.cend(), [&found, &vg_setmember_identifier](WidgetInstanceIdentifier const & selected_variable_identifier)
								{
									if (found)
									{
										return; // from lambda
									}
									if (selected_variable_identifier.IsEqual(WidgetInstanceIdentifier::EQUALITY_CHECK_TYPE__UUID_PLUS_STRING_CODE, vg_setmember_identifier))
									{
										found = true;
									}
								});
								if (found)
								{
									current_variable_group_current_primary_key_info.is_primary_column_selected = true;
								}
								else
								{
									current_variable_group_current_primary_key_info.is_primary_column_selected = false;
								}
							}
							++current_variable_group_current_primary_key_dmu_category__internal_sequence_number;
							++current_variable_group_current_primary_key_dmu_category__total_sequence_number;
						}
					});
				}

				if (failed)
				{
					return; // from lambda
				}
			});

			++overall_primary_key_sequence_number;
			++k_sequence_number_count_for_given_dmu_category_out_of_k_count_for_primary_uoa_for_that_dmu_category;
		}

	});
}
