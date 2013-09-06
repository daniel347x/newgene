#include "OutputModel.h"
#include "..\Utilities\UUID.h"

#include "..\Settings\OutputProjectSettings_list.h"
#include <cstdint>

#ifndef Q_MOC_RUN
#	include <boost/lexical_cast.hpp>
#	include <deque>
#	include <boost/filesystem.hpp>
#	include <boost/format.hpp>
#	include <boost/scope_exit.hpp>
#endif

#include <fstream>

std::recursive_mutex OutputModel::OutputGenerator::is_generating_output_mutex;
std::atomic<bool> OutputModel::OutputGenerator::is_generating_output = false;

void OutputModel::LoadTables()
{

	LoadDatabase();

	if (db != nullptr)
	{
		t_variables_selected_identifiers.Load(db, this, input_model.get());
		t_variables_selected_identifiers.Sort();

		t_kad_count.Load(db, this, input_model.get());
		t_kad_count.Sort();

		t_time_range.Load(db, this, input_model.get());
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

OutputModel::OutputGenerator::OutputGenerator(Messager & messager_, OutputModel & model_, OutputProject & project_)
	: model(&model_)
	, stmt_result(nullptr)
	, executor(nullptr, false)
	, initialized(false)
	, timerange_start(0)
	, timerange_end(0)
	, project(project_)
	, messager(messager_)
	, failed(false)
	, overwrite_if_output_file_already_exists(false)
	, rough_progress_range(0)
	, rough_progress_increment_one_percent(0)
	, total_number_primary_rows(0)
	, current_progress_stage(0)
	, total_progress_stages(0)
	, progress_increment_per_stage(0)
	, current_progress_value(0)
{
	debug_ordering = true;
	messager.StartProgressBar(0, 1000);
}

OutputModel::OutputGenerator::~OutputGenerator()
{

	messager.EndProgressBar();

	if (!initialized)
	{
		return;
	}

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

	std::for_each(primary_group_final_results.begin(), primary_group_final_results.end(), [](SqlAndColumnSet & sql_and_column_set)
	{

		std::for_each(sql_and_column_set.first.begin(), sql_and_column_set.first.end(), [](SQLExecutor & sql_executor)
		{

			sql_executor.Empty();

		});

	});

	std::for_each(intermediate_merging_of_primary_groups_column_sets.begin(), intermediate_merging_of_primary_groups_column_sets.end(), [](SqlAndColumnSet & sql_and_column_set)
	{

		std::for_each(sql_and_column_set.first.begin(), sql_and_column_set.first.end(), [](SQLExecutor & sql_executor)
		{

			sql_executor.Empty();

		});

	});

	std::for_each(all_merged_results_unformatted.first.begin(), all_merged_results_unformatted.first.end(), [](SQLExecutor & sql_executor)
	{

		sql_executor.Empty();

	});

	std::for_each(final_result.first.begin(), final_result.first.end(), [](SQLExecutor & sql_executor)
	{

		sql_executor.Empty();

	});

	ClearTable(final_result);

}

void OutputModel::OutputGenerator::GenerateOutput(DataChangeMessage & change_response)
{

	{
		std::lock_guard<std::recursive_mutex> guard(is_generating_output_mutex);
		if (is_generating_output)
		{
			messager.ShowMessageBox("Another K-ad output generation operation is in progress.  Please wait for that operation to complete first.");
			return;
		}
		is_generating_output = true;
	}

	BOOST_SCOPE_EXIT(&is_generating_output, &is_generating_output_mutex)
	{
		is_generating_output = false;
	} BOOST_SCOPE_EXIT_END

	setting_path_to_kad_output = CheckOutputFileExists();

	if (failed)
	{
		return;
	}

	if (setting_path_to_kad_output.empty())
	{
		return;
	}

	InputModel & input_model = model->getInputModel();
	Table_VARIABLES_SELECTED::UOA_To_Variables_Map the_map_ = model->t_variables_selected_identifiers.GetSelectedVariablesByUOA(model->getDb(), model, &input_model);
	the_map = &the_map_;

	bool found = false;
	WidgetInstanceIdentifier_Int64_Pair timerange_start_identifier;
	found = model->t_time_range.getIdentifierFromStringCodeAndFlags("0", "s", timerange_start_identifier);
	if (!found)
	{
		boost::format msg("Cannot determine time range from database.");
		SetFailureMessage(msg.str());
		failed = true;
		return;
	}
	timerange_start = timerange_start_identifier.second;
	found = false;
	WidgetInstanceIdentifier_Int64_Pair timerange_end_identifier;
	found = model->t_time_range.getIdentifierFromStringCodeAndFlags("0", "e", timerange_end_identifier);
	if (!found)
	{
		boost::format msg("Cannot determine time range end value from database.");
		SetFailureMessage(msg.str());
		failed = true;
		return;
	}
	timerange_end = timerange_end_identifier.second;

	if (timerange_end <= timerange_start)
	{
		boost::format msg("The ending value of the time range must be greater than the starting value.");
		SetFailureMessage(msg.str());
		failed = true;
		return;
	}

	current_progress_stage = 0;
	boost::format msg_1("Generating output to file %1%");
	msg_1 % boost::filesystem::path(setting_path_to_kad_output).filename();
	messager.UpdateStatusBarText(msg_1.str().c_str());
	messager.AppendKadStatusText("Beginning generation of K-ad output.");
	messager.AppendKadStatusText("Initializing...");

	Prepare();

	if (failed)
	{
		// failed
		return;
	}

	messager.AppendKadStatusText("Preparing input data...");
	ObtainColumnInfoForRawDataTables();

	if (failed)
	{
		// failed
		return;
	}

	DetermineTotalNumberRows();

	if (failed)
	{
		// failed
		return;
	}

	messager.AppendKadStatusText("Looping through top-level variable groups...");
	LoopThroughPrimaryVariableGroups();

	if (failed)
	{
		// failed
		return;
	}

	//messager.UpdateProgressBarValue(500);

	messager.AppendKadStatusText("Merging top-level variable groups...");
	MergeHighLevelGroupResults();

	if (failed)
	{
		// failed
		return;
	}

	//messager.UpdateProgressBarValue(750);

	messager.AppendKadStatusText("Merging child variable groups...");
	MergeChildGroups();

	if (failed)
	{
		// failed
		return;
	}

	//messager.UpdateProgressBarValue(900);

	messager.AppendKadStatusText("Formatting results...");
	FormatResultsForOutput();

	if (failed)
	{
		// failed
		return;
	}

	//messager.UpdateProgressBarValue(950);

	messager.AppendKadStatusText("Writing results to disk...");
	WriteResultsToFileOrScreen();

	if (failed)
	{
		return;
	}

	messager.UpdateProgressBarValue(1000);

	boost::format msg_2("Output successfully generated (%1%)");
	msg_2 % boost::filesystem::path(setting_path_to_kad_output).filename();
	messager.UpdateStatusBarText(msg_2.str().c_str());
	messager.AppendKadStatusText("Done.");

}

void OutputModel::OutputGenerator::MergeChildGroups()
{

	SqlAndColumnSet x_table_result = primary_group_merged_results;
	SqlAndColumnSet xr_table_result = x_table_result;

	// Child tables
	int current_child_view_name_index = 1;
	int child_set_number = 1;
	std::for_each(secondary_variable_groups_column_info.cbegin(), secondary_variable_groups_column_info.cend(), [this, &current_child_view_name_index, &child_set_number, &x_table_result, &xr_table_result](ColumnsInTempView const & child_variable_group_raw_data_columns)
	{

		std::int64_t raw_rows_count = total_number_incoming_rows[child_variable_group_raw_data_columns.variable_groups[0]];
		std::int64_t rows_estimate = raw_rows_count;

		WidgetInstanceIdentifier const & dmu_category_multiplicity_greater_than_1_for_child = child_uoas__which_multiplicity_is_greater_than_1[*(child_variable_group_raw_data_columns.variable_groups[0].identifier_parent)].first;
		int const the_child_multiplicity = child_uoas__which_multiplicity_is_greater_than_1[*(child_variable_group_raw_data_columns.variable_groups[0].identifier_parent)].second;
		for (int current_multiplicity = 1; current_multiplicity <= the_child_multiplicity; ++current_multiplicity)
		{
			x_table_result = CreateChildXTable(child_variable_group_raw_data_columns, xr_table_result.second, current_multiplicity, 0, child_set_number, current_child_view_name_index);
			x_table_result.second.most_recent_sql_statement_executed__index = -1;
			ExecuteSQL(x_table_result);
			ClearTable(xr_table_result);
			merging_of_children_column_sets.push_back(x_table_result);
			if (failed)
			{
				return;
			}

			UpdateProgressBarToNextStage();
			rows_estimate *= raw_rows_count;
			xr_table_result = CreateXRTable(x_table_result.second, current_multiplicity, 0, OutputModel::OutputGenerator::CHILD_VARIABLE_GROUP, child_set_number, current_child_view_name_index, rows_estimate);
			ClearTable(x_table_result);
			merging_of_children_column_sets.push_back(xr_table_result);
			if (failed)
			{
				return;
			}

			++current_child_view_name_index;
		}

		++child_set_number;

	});

	// The structure of the table incoming to the following function is this:
	// XR XR ... XR XRMFXR ... XR XR ... XR XRMFXR ... XR XR (with the XR set at the end, optional)
	// ... the child tables appear last (optionally)
	//
	// ... and the same format for the table is returned
	SqlAndColumnSet preliminary_sorted_kad_result = CreateSortedTable(merging_of_children_column_sets.back().second, 0);
	preliminary_sorted_kad_result.second.most_recent_sql_statement_executed__index = -1;
	ExecuteSQL(preliminary_sorted_kad_result);
	ClearTable(merging_of_children_column_sets.back());
	merging_of_children_column_sets.push_back(preliminary_sorted_kad_result);
	if (failed)
	{
		return;
	}

	// The structure of the table incoming to the following function is this:
	// XR XR ... XR XRMFXR ... XR XR ... XR XRMFXR ... XR XR
	//
	// The structure of the table that comes out is:
	// XR XR ... XR XRMFXR ... XR XR ... XR XRMFXR ... XR XR XR_Z (or, if no child tables, the final is XRMFXR_Z)
	std::int64_t rows_added = 0;
	SqlAndColumnSet duplicates_removed_kad_result = RemoveDuplicates(preliminary_sorted_kad_result.second, 0, rows_added);
	ClearTable(preliminary_sorted_kad_result);
	merging_of_children_column_sets.push_back(duplicates_removed_kad_result);
	if (failed)
	{
		return;
	}

	// The structure of the following table is:
	// XR XR ... XR XRMFXR ... XR XR ... XR XRMFXR ... XR XR XR_Z
	all_merged_results_unformatted = duplicates_removed_kad_result;

}

OutputModel::OutputGenerator::SqlAndColumnSet OutputModel::OutputGenerator::CreateInitialChildXRTable(ColumnsInTempView const & primary_variable_group_merge_result_x_columns)
{

	SqlAndColumnSet result = std::make_pair(std::vector<SQLExecutor>(), ColumnsInTempView());
	std::vector<SQLExecutor> & sql_strings = result.first;
	ColumnsInTempView & result_columns = result.second;

	result_columns = primary_variable_group_merge_result_x_columns;

	std::string view_name = "CVXR";
	view_name += "1";
	result_columns.view_name_no_uuid = view_name;
	view_name += "_";
	view_name += newUUID(true);
	result_columns.view_name = view_name;

	std::vector<std::string> previous_column_names;

	bool first = true;
	int column_index = 0;
	std::for_each(result_columns.columns_in_view.begin(), result_columns.columns_in_view.end(), [&previous_column_names, &column_index, &first](ColumnsInTempView::ColumnInTempView & new_column)
	{
		previous_column_names.push_back(new_column.column_name_in_temporary_table);
		new_column.column_name_in_temporary_table = new_column.column_name_in_temporary_table_no_uuid;
		new_column.column_name_in_temporary_table += "_";
		new_column.column_name_in_temporary_table += newUUID(true);
		++column_index;
	});

	sql_strings.push_back(SQLExecutor(db));
	std::string & sql_string = sql_strings.back().sql;

	sql_string = "CREATE TABLE ";
	sql_string += result_columns.view_name;
	sql_string += " AS SELECT ";
	first = true;
	int the_index = 0;
	std::for_each(result_columns.columns_in_view.begin(), result_columns.columns_in_view.end(), [&sql_string, &the_index, &previous_column_names, &first](ColumnsInTempView::ColumnInTempView & new_column)
	{
		if (!first)
		{
			sql_string += ", ";
		}
		first = false;
		sql_string += previous_column_names[the_index];
		sql_string += " AS ";
		sql_string += new_column.column_name_in_temporary_table;
		++the_index;
	});
	sql_string += " FROM ";
	sql_string += primary_variable_group_merge_result_x_columns.view_name;


	// Add the "merged" time range columns

	std::string datetime_start_col_name_no_uuid = "DATETIMESTART_CHILD_MERGE";
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
	datetime_start_column.column_type = ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__DATETIMESTART_CHILD_MERGE;
	datetime_start_column.column_name_in_original_data_table = "";
	datetime_start_column.inner_table_set_number = 0;
	datetime_start_column.is_within_inner_table_corresponding_to_top_level_uoa = false;

	std::string datetime_end_col_name_no_uuid = "DATETIMEEND_CHILD_MERGE";
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
	datetime_end_column.column_type = ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__DATETIMEEND_CHILD_MERGE;
	datetime_end_column.column_name_in_original_data_table = "";
	datetime_end_column.inner_table_set_number = 0;
	datetime_end_column.is_within_inner_table_corresponding_to_top_level_uoa = false;


	// Set the "merged" time range columns to be equal to the original time range columns
	std::string sql_time_range;
	sql_time_range += "UPDATE OR FAIL ";
	sql_time_range += result_columns.view_name;
	sql_time_range += " SET ";
	sql_time_range += datetime_start_col_name;
	sql_time_range += " = ";
	sql_time_range += result_columns.columns_in_view[result_columns.columns_in_view.size() - 2].column_name_in_temporary_table;
	sql_time_range += ", ";
	sql_time_range += datetime_end_col_name;
	sql_time_range += " = ";
	sql_time_range += result_columns.columns_in_view[result_columns.columns_in_view.size() - 1].column_name_in_temporary_table;
	sql_strings.push_back(SQLExecutor(db, sql_time_range));

	return result;
}

void OutputModel::OutputGenerator::WriteResultsToFileOrScreen()
{

	// The final results are stored in "final_result".
	// Do an "ObtainData()" on this result, loop through,
	// and write the output to a CSV file on disk.

	std::string setting_path_to_kad_output = CheckOutputFileExists();

	if (failed)
	{
		return;
	}

	if (setting_path_to_kad_output.empty())
	{
		return;
	}

	std::fstream output_file;
	output_file.open(setting_path_to_kad_output, std::ios::out | std::ios::trunc);
	if (!output_file.good())
	{
		boost::format msg("Cannot open output file %1%");
		msg % setting_path_to_kad_output;
		SetFailureMessage(msg.str());
		failed = true;
		return;
	}

	ObtainData(final_result.second);

	if (failed)
	{
		return;
	}

	int column_data_type = 0;
	int column_index = 0;
	bool first = true;
	std::int64_t data_int64 = 0;
	std::string data_string;
	long double data_long = 0.0;
	while (StepData())
	{

		if (failed)
		{
			break;
		}

		column_index = 0;
		first = true;
		std::map<WidgetInstanceIdentifier, bool> variable_group_appears_more_than_once;
		std::for_each(final_result.second.columns_in_view.begin(), final_result.second.columns_in_view.end(), [this, &output_file, &data_int64, &data_string, &data_long, &first, &column_index, &column_data_type](ColumnsInTempView::ColumnInTempView & unformatted_column)
		{

			if (failed)
			{
				return;
			}

			column_data_type = sqlite3_column_type(stmt_result, column_index);

			switch (column_data_type)
			{

				case SQLITE_INTEGER:
					{

						data_int64 = sqlite3_column_int64(stmt_result, column_index);
						if (!first)
						{
							output_file << ",";
						}
						first = false;
						output_file << data_int64;

					}
					break;

				case SQLITE_FLOAT:
					{
						// Currently not implemented!!!!!!!  Just add new bound_paramenter_longs as argument to this function, and as member of SQLExecutor just like the other bound_parameter data members, to implement.
						data_long = sqlite3_column_double(stmt_result, column_index);
						// Todo: Error message
						boost::format msg("Floating point values are not yet supported.");
						SetFailureMessage(msg.str());
						failed = true;
						return; // from lambda
					}
					break;

				case SQLITE_TEXT:
					{

						data_string = reinterpret_cast<char const *>(sqlite3_column_text(stmt_result, column_index));
						if (!first)
						{
							output_file << ",";
						}
						first = false;
						output_file << data_string;

					}
					break;

				case SQLITE_BLOB:
					{
						// Todo: Error message
						boost::format msg("Blob values are not supported.");
						SetFailureMessage(msg.str());
						failed = true;
						return; // from lambda
					}
					break;

				case SQLITE_NULL:
					{

						if (!first)
						{
							output_file << ",";
						}
						first = false;

					}
					break;

				default:
					{
						// Todo: Error message
						boost::format msg("Unknown data type in database column.");
						SetFailureMessage(msg.str());
						failed = true;
						return; // from lambda
					}

			}

			++column_index;

		});

		if (failed)
		{
			return;
		}

		output_file << std::endl;
		
	}

	if (failed)
	{
		return;
	}

	if (output_file.good())
	{
		output_file.close();
	}

}

void OutputModel::OutputGenerator::FormatResultsForOutput()
{
	
	// The unformatted results are stored in "all_merged_results_unformatted"
	// Just do a SELECT ... AS ... to pluck out the desired columns and give them the proper names.
	// Save this in "final_result"

	char c[256];

	final_result = std::make_pair(std::vector<SQLExecutor>(), ColumnsInTempView());
	std::vector<SQLExecutor> & sql_strings = final_result.first;
	ColumnsInTempView & result_columns = final_result.second;

	std::string view_name = "KAD_Results";
	result_columns.view_name_no_uuid = view_name;
	view_name += "_";
	view_name += newUUID(true);
	result_columns.view_name = view_name;

	sql_strings.push_back(SQLExecutor(db));
	std::string & sql_string = sql_strings.back().sql;

	sql_string = "CREATE TABLE ";
	sql_string += result_columns.view_name;
	sql_string += " AS SELECT ";

	WidgetInstanceIdentifier first_variable_group;
	int highest_index_first_primary_vg_full_final_results_table = -1;

	// Display primary key columns
	bool first = true;
	int column_index = 0;
	bool reached_end_of_first_inner_table_not_including_terminating_datetime_columns = false;
	std::for_each(all_merged_results_unformatted.second.columns_in_view.begin(), all_merged_results_unformatted.second.columns_in_view.end(), [&c, &highest_index_first_primary_vg_full_final_results_table, &reached_end_of_first_inner_table_not_including_terminating_datetime_columns, &sql_string, &first, &first_variable_group, &result_columns, &column_index](ColumnsInTempView::ColumnInTempView & unformatted_column)
	{

		if (column_index == 0)
		{
			first_variable_group = unformatted_column.variable_group_associated_with_current_inner_table;
		}

		if (unformatted_column.column_type == ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__DATETIMEEND_MERGED_FINAL)
		{
			if (highest_index_first_primary_vg_full_final_results_table == -1)
			{
				highest_index_first_primary_vg_full_final_results_table = column_index;
			}
		}

		if (!unformatted_column.variable_group_associated_with_current_inner_table.IsEqual(WidgetInstanceIdentifier::EQUALITY_CHECK_TYPE__STRING_CODE, first_variable_group))
		{
			if (unformatted_column.column_type == ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__PRIMARY)
			{
				++column_index;
				return; // Only display primary key columns from the first primary variable group
			}
		}

		if (unformatted_column.column_type == ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__PRIMARY)
		{
			if (reached_end_of_first_inner_table_not_including_terminating_datetime_columns)
			{
				if (unformatted_column.total_multiplicity__of_current_dmu_category__within_uoa_corresponding_to_the_current_inner_tables_variable_group == 1)
				{
					++column_index;
					return; // only display primary key columns of multiplicity 1 once - from the first inner table of the first primary variable group
				}
			}
		}

		switch (unformatted_column.column_type)
		{
			case ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__DATETIMESTART:
			case ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__DATETIMEEND:
			case ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__DATETIMESTART_INTERNAL:
			case ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__DATETIMEEND_INTERNAL:
			case ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__DATETIMESTART_MERGED:
			case ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__DATETIMEEND_MERGED:
			case ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__DATETIMESTART_MERGED_FINAL:
			case ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__DATETIMEEND_MERGED_FINAL:
			case ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__DATETIMESTART_MERGED_BETWEEN_FINALS:
			case ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__DATETIMEEND_MERGED_BETWEEN_FINALS:
			case ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__DATETIMESTART_CHILD_MERGE:
			case ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__DATETIMEEND_CHILD_MERGE:
				{
					reached_end_of_first_inner_table_not_including_terminating_datetime_columns = true;
					++column_index;
					return; // only display a single pair of time range columns
				}
				break;
		}

		if (unformatted_column.column_type == ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__SECONDARY)
		{
			++column_index;
			return; // display secondary keys only after primary keys
		}

		result_columns.columns_in_view.push_back(unformatted_column);
		ColumnsInTempView::ColumnInTempView & formatted_column = result_columns.columns_in_view.back();

		formatted_column.column_name_in_temporary_table = formatted_column.column_name_in_original_data_table;

		if (unformatted_column.total_multiplicity__of_current_dmu_category__within_uoa_corresponding_to_the_current_inner_tables_variable_group > 1)
		{
			formatted_column.column_name_in_temporary_table += "_";
			formatted_column.column_name_in_temporary_table += itoa(unformatted_column.current_multiplicity__corresponding_to__current_inner_table___is_1_in_all_inner_tables_when_multiplicity_is_1_for_that_vg, c, 10);
		}

		formatted_column.column_name_in_temporary_table_no_uuid = formatted_column.column_name_in_temporary_table;

		switch (unformatted_column.column_type)
		{
			case ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__DATETIMESTART_MERGED_KAD_OUTPUT:
				{
					formatted_column.column_name_in_temporary_table = "DATETIME_START";
				}
				break;
			case ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__DATETIMEEND_MERGED_KAD_OUTPUT:
				{
					formatted_column.column_name_in_temporary_table = "DATETIME_END";
				}
				break;
		}

		if (!first)
		{
			sql_string += ", ";
		}
		first = false;

		sql_string += unformatted_column.column_name_in_temporary_table;
		sql_string += " AS ";
		sql_string += formatted_column.column_name_in_temporary_table;
		++column_index;

	});


	// Display secondary key columns

	// First, do a quick calculation and save which secondary keys appear more than once, and stash that away
	column_index = 0;
	std::map<WidgetInstanceIdentifier, bool> variable_group_appears_more_than_once;
	std::for_each(all_merged_results_unformatted.second.columns_in_view.begin(), all_merged_results_unformatted.second.columns_in_view.end(), [&c, &variable_group_appears_more_than_once, &highest_index_first_primary_vg_full_final_results_table, &sql_string, &first, &first_variable_group, &result_columns, &column_index](ColumnsInTempView::ColumnInTempView & unformatted_column)
	{

		if (unformatted_column.column_type != ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__SECONDARY)
		{
			++column_index;
			return;
		}

		// A little trick: The highest multiplicity always appears last for any variable group,
		// so if the variable group has a multiplicity greater than 1, its highest value which appears last will set the value to true
		variable_group_appears_more_than_once[unformatted_column.variable_group_associated_with_current_inner_table] = (unformatted_column.current_multiplicity__of__current_inner_table__within__current_vg > 1);

		++column_index;

	});

	// now display the secondary key columns themselves

	// ... first, the secondary key columns from the primary variable groups
	column_index = 0;
	std::for_each(all_merged_results_unformatted.second.columns_in_view.begin(), all_merged_results_unformatted.second.columns_in_view.end(), [&c, &variable_group_appears_more_than_once, &sql_string, &first, &first_variable_group, &result_columns, &column_index](ColumnsInTempView::ColumnInTempView & unformatted_column)
	{

		if (unformatted_column.column_type != ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__SECONDARY)
		{
			++column_index;
			return;
		}

		if (!unformatted_column.is_within_inner_table_corresponding_to_top_level_uoa)
		{
			++column_index;
			return;
		}

		result_columns.columns_in_view.push_back(unformatted_column);
		ColumnsInTempView::ColumnInTempView & formatted_column = result_columns.columns_in_view.back();

		formatted_column.column_name_in_temporary_table = formatted_column.column_name_in_original_data_table;
		if (variable_group_appears_more_than_once[unformatted_column.variable_group_associated_with_current_inner_table])
		{
			formatted_column.column_name_in_temporary_table += "_";
			formatted_column.column_name_in_temporary_table += itoa(formatted_column.current_multiplicity__of__current_inner_table__within__current_vg, c, 10);
		}
		formatted_column.column_name_in_temporary_table_no_uuid = formatted_column.column_name_in_temporary_table;

		if (!first)
		{
			sql_string += ", ";
		}
		first = false;

		sql_string += unformatted_column.column_name_in_temporary_table;
		sql_string += " AS ";
		sql_string += formatted_column.column_name_in_temporary_table;

		++column_index;

	});

	// ... next, the secondary key columns from the child variable groups
	bool new_method = true;
	if (!new_method)
	{
		column_index = 0;
		std::for_each(all_merged_results_unformatted.second.columns_in_view.begin(), all_merged_results_unformatted.second.columns_in_view.end(), [&c, &highest_index_first_primary_vg_full_final_results_table, &variable_group_appears_more_than_once, &sql_string, &first, &first_variable_group, &result_columns, &column_index](ColumnsInTempView::ColumnInTempView & unformatted_column)
		{

			if (unformatted_column.column_type != ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__SECONDARY)
			{
				++column_index;
				return;
			}

			if (unformatted_column.is_within_inner_table_corresponding_to_top_level_uoa)
			{
				++column_index;
				return;
			}

			if (column_index > highest_index_first_primary_vg_full_final_results_table)
			{
				++column_index;
				return;
			}

			result_columns.columns_in_view.push_back(unformatted_column);
			ColumnsInTempView::ColumnInTempView & formatted_column = result_columns.columns_in_view.back();

			formatted_column.column_name_in_temporary_table = formatted_column.column_name_in_original_data_table;
			if (variable_group_appears_more_than_once[unformatted_column.variable_group_associated_with_current_inner_table])
			{
				formatted_column.column_name_in_temporary_table += "_";
				formatted_column.column_name_in_temporary_table += itoa(formatted_column.current_multiplicity__of__current_inner_table__within__current_vg, c, 10);
			}
			formatted_column.column_name_in_temporary_table_no_uuid = formatted_column.column_name_in_temporary_table;

			if (!first)
			{
				sql_string += ", ";
			}
			first = false;

			sql_string += unformatted_column.column_name_in_temporary_table;
			sql_string += " AS ";
			sql_string += formatted_column.column_name_in_temporary_table;

			++column_index;

		});
	}
	else
	{
		column_index = 0;
		std::for_each(all_merged_results_unformatted.second.columns_in_view.begin(), all_merged_results_unformatted.second.columns_in_view.end(), [&c, &variable_group_appears_more_than_once, &sql_string, &first, &first_variable_group, &result_columns, &column_index](ColumnsInTempView::ColumnInTempView & unformatted_column)
		{

			if (unformatted_column.column_type != ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__SECONDARY)
			{
				++column_index;
				return;
			}

			if (unformatted_column.is_within_inner_table_corresponding_to_top_level_uoa)
			{
				++column_index;
				return;
			}

			result_columns.columns_in_view.push_back(unformatted_column);
			ColumnsInTempView::ColumnInTempView & formatted_column = result_columns.columns_in_view.back();

			formatted_column.column_name_in_temporary_table = formatted_column.column_name_in_original_data_table;
			if (variable_group_appears_more_than_once[unformatted_column.variable_group_associated_with_current_inner_table])
			{
				formatted_column.column_name_in_temporary_table += "_";
				formatted_column.column_name_in_temporary_table += itoa(formatted_column.current_multiplicity__of__current_inner_table__within__current_vg, c, 10);
			}
			formatted_column.column_name_in_temporary_table_no_uuid = formatted_column.column_name_in_temporary_table;

			if (!first)
			{
				sql_string += ", ";
			}
			first = false;

			sql_string += unformatted_column.column_name_in_temporary_table;
			sql_string += " AS ";
			sql_string += formatted_column.column_name_in_temporary_table;

			++column_index;

		});
	}

	sql_string += " FROM ";
	sql_string += all_merged_results_unformatted.second.view_name;

	final_result.second.most_recent_sql_statement_executed__index = -1;
	ExecuteSQL(final_result);
	ClearTable(all_merged_results_unformatted);

}

void OutputModel::OutputGenerator::MergeHighLevelGroupResults()
{

	// Datetime columns incoming:
	// The "X" tables have a single COLUMN_TYPE__DATETIMESTART and COLUMN_TYPE__DATETIMEEND (or COLUMN_TYPE__DATETIMESTART_INTERNAL and COLUMN_TYPE__DATETIMEEND_INTERNAL, if added by the generator)
	// The "XR" tables have that, followed by COLUMN_TYPE__DATETIMESTART_MERGED and COLUMN_TYPE__DATETIMEEND_MERGED
	// The X tables compose the XR table come in pairs; i.e. XR XR XR XR ... (one per multiplicity of the top-level variable group followed by one per multiplicity per each child variable group, noting that
	//    the child multiplicity might occur on a different DMU category and might have a different numeric value)
	// The individual top-level primary variable group results have all of the above, followed by a SINGLE COLUMN_TYPE__DATETIMESTART_MERGED_FINAL and COLUMN_TYPE__DATETIMEEND_MERGED_FINAL pair
	//    (resulting from the removal of duplicates) at the end of the entire
	//    sequence of XR XR ... pairs, per top-level primary variable group.
	//
	// So we have, for the incoming table:
	// XR XR XR ... XRMF
	//
	// The final inner table, XRMF, of the incoming table, then, has 3 sets:
	// COLUMN_TYPE__DATETIMESTART COLUMN_TYPE__DATETIMESTART_MERGED COLUMN_TYPE__DATETIMESTART_MERGED_FINAL



	// XR tables:
	// COLUMN_TYPE__DATETIMESTART
	// COLUMN_TYPE__DATETIMESTART_MERGED

	// XR_Z tables:
	// COLUMN_TYPE__DATETIMESTART
	// COLUMN_TYPE__DATETIMESTART_MERGED
	// COLUMN_TYPE__DATETIMESTART_MERGED_KAD_OUTPUT

	// XRMF tables:
	// COLUMN_TYPE__DATETIMESTART
	// COLUMN_TYPE__DATETIMESTART_MERGED
	// COLUMN_TYPE__DATETIMESTART_MERGED_FINAL

	// XRMFXR tables:
	// COLUMN_TYPE__DATETIMESTART
	// COLUMN_TYPE__DATETIMESTART_MERGED
	// COLUMN_TYPE__DATETIMESTART_MERGED_FINAL
	// COLUMN_TYPE__DATETIMESTART_MERGED_BETWEEN_FINALS

	// XRMFXR_Z tables:
	// COLUMN_TYPE__DATETIMESTART
	// COLUMN_TYPE__DATETIMESTART_MERGED
	// COLUMN_TYPE__DATETIMESTART_MERGED_FINAL
	// COLUMN_TYPE__DATETIMESTART_MERGED_BETWEEN_FINALS
	// COLUMN_TYPE__DATETIMESTART_MERGED_KAD_OUTPUT

	// The incoming table is XR XR ... XR XRMF

	UpdateProgressBarToNextStage();

	SqlAndColumnSet intermediate_merge_of_top_level_primary_group_results = primary_group_final_results[0];
	intermediate_merge_of_top_level_primary_group_results.second.view_number = 1;
	intermediate_merging_of_primary_groups_column_sets.push_back(intermediate_merge_of_top_level_primary_group_results);

	// The following table is XR XR ... XR XRMFXR
	SqlAndColumnSet xr_table_result = CreateInitialPrimaryMergeXRTable(intermediate_merging_of_primary_groups_column_sets.back().second);
	if (failed)
	{
		return;
	}
	xr_table_result.second.most_recent_sql_statement_executed__index = -1;
	ExecuteSQL(xr_table_result);
	ClearTable(intermediate_merging_of_primary_groups_column_sets.back());
	intermediate_merging_of_primary_groups_column_sets.push_back(xr_table_result);
	if (failed)
	{
		return;
	}
	
	std::int64_t raw_rows_count = 0;
	std::int64_t rows_estimate = 0;

	// The primary variable group data is stored in primary_group_final_results
	int count = 1;
	std::for_each(primary_group_final_results.cbegin(), primary_group_final_results.cend(), [this, &rows_estimate, &raw_rows_count, &xr_table_result, &intermediate_merge_of_top_level_primary_group_results, &count](SqlAndColumnSet const & primary_variable_group_final_result)
	{

		raw_rows_count = total_number_incoming_rows[intermediate_merging_of_primary_groups_column_sets.back().second.columns_in_view[0].variable_group_associated_with_current_inner_table];

		if (count != 1)
		{
			// The structure of the table returned from the following function is this:
			// XR XR ... XR XRMFXR ... XR XR ... XR XRMFXR ... XR XR ... XRMF
			intermediate_merge_of_top_level_primary_group_results = MergeIndividualTopLevelGroupIntoPrevious(primary_variable_group_final_result.second, intermediate_merging_of_primary_groups_column_sets.back(), count);
			if (failed)
			{
				return;
			}
			intermediate_merge_of_top_level_primary_group_results.second.most_recent_sql_statement_executed__index = -1;
			ExecuteSQL(intermediate_merge_of_top_level_primary_group_results);
			ClearTable(intermediate_merging_of_primary_groups_column_sets.back());
			intermediate_merging_of_primary_groups_column_sets.push_back(intermediate_merge_of_top_level_primary_group_results);
			if (failed)
			{
				return;
			}

			// The structure of the table returned from the following function is this:
			// XR XR ... XR XRMFXR ... XR XR ... XR XRMFXR ... XR XR ... XRMFXR
			UpdateProgressBarToNextStage();
			rows_estimate += raw_rows_count;
			xr_table_result = CreateXRTable(intermediate_merge_of_top_level_primary_group_results.second, count, 0, OutputModel::OutputGenerator::FINAL_MERGE_OF_PRIMARY_VARIABLE_GROUP, count, count, rows_estimate);
			ClearTable(intermediate_merging_of_primary_groups_column_sets.back());
			intermediate_merging_of_primary_groups_column_sets.push_back(xr_table_result);
			if (failed)
			{
				return;
			}
		}
		else
		{
			rows_estimate = raw_rows_count;
		}
		++count;
	});

	if (failed)
	{
		return;
	}

	// The structure of the following table is:
	// XR XR ... XR XRMFXR ... XR XR ... XR XRMFXR ... XR XR ... XRMFXR
	// Each XR set is a single top-level primary group.
	primary_group_merged_results = intermediate_merging_of_primary_groups_column_sets.back();

}

OutputModel::OutputGenerator::SqlAndColumnSet OutputModel::OutputGenerator::MergeIndividualTopLevelGroupIntoPrevious(ColumnsInTempView const & primary_variable_group_final_result, OutputModel::OutputGenerator::SqlAndColumnSet & previous_merged_primary_variable_groups_table, int const count)
{

	char c[256];

	SqlAndColumnSet result = std::make_pair(std::vector<SQLExecutor>(), ColumnsInTempView());
	std::vector<SQLExecutor> & sql_strings = result.first;
	ColumnsInTempView & result_columns = result.second;

	result_columns = previous_merged_primary_variable_groups_table.second;

	std::string view_name = "MF";
	view_name += itoa(count, c, 10);
	result_columns.view_name_no_uuid = view_name;
	view_name += "_";
	view_name += newUUID(true);
	result_columns.view_name = view_name;
	result_columns.view_number = count;
	result_columns.has_no_datetime_columns = false;

	int first_full_table_column_count = 0;
	int number_columns_very_first_primary_variable_group_including_multiplicities = 0; // corresponding to primary variable group #1
	int number_columns__in__very_first_primary_variable_group__and__only_its_first_inner_table = 0;
	WidgetInstanceIdentifier very_first_primary_variable_group;

	std::vector<std::string> previous_column_names;
	// The first integer: which index inside the vector is current.
	// The second integer: which index inside result_columns to use for the given primary key corresponding to the given DMU category
	std::map<WidgetInstanceIdentifier, std::pair<int, std::vector<int>>> lhs_primary_keys;
	std::map<WidgetInstanceIdentifier, std::pair<int, std::vector<int>>> rhs_primary_keys;

	// These columns are from the previous MFXR temporary table, which contains 2 pairs of datetime columns
	// in every inner table except for those inner tables that mark the end of the entire top-level variable set final results,
	// which have 4 pairs of datetime columns
	int internal_datetime_column_count = 0;
	std::for_each(result_columns.columns_in_view.begin(), result_columns.columns_in_view.end(), [&lhs_primary_keys, &internal_datetime_column_count, &first_full_table_column_count, &number_columns__in__very_first_primary_variable_group__and__only_its_first_inner_table, &previous_column_names, &number_columns_very_first_primary_variable_group_including_multiplicities, &very_first_primary_variable_group](ColumnsInTempView::ColumnInTempView & previous_column)
	{
		previous_column_names.push_back(previous_column.column_name_in_temporary_table);
		previous_column.column_name_in_temporary_table = previous_column.column_name_in_temporary_table_no_uuid;
		previous_column.column_name_in_temporary_table += "_";
		previous_column.column_name_in_temporary_table += newUUID(true);

		if (first_full_table_column_count == 0)
		{
			very_first_primary_variable_group = previous_column.variable_group_associated_with_current_inner_table;
		}

		if (previous_column.variable_group_associated_with_current_inner_table.IsEqual(WidgetInstanceIdentifier::EQUALITY_CHECK_TYPE__STRING_CODE, very_first_primary_variable_group))
		{
			if (previous_column.column_type != ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__DATETIMESTART && previous_column.column_type != ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__DATETIMEEND && previous_column.column_type != ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__DATETIMESTART_INTERNAL && previous_column.column_type != ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__DATETIMEEND_INTERNAL && previous_column.column_type != ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__DATETIMESTART_MERGED && previous_column.column_type != ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__DATETIMEEND_MERGED && previous_column.column_type != ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__DATETIMESTART_MERGED_FINAL && previous_column.column_type != ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__DATETIMEEND_MERGED_FINAL && previous_column.column_type != ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__DATETIMESTART_MERGED_BETWEEN_FINALS && previous_column.column_type != ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__DATETIMEEND_MERGED_BETWEEN_FINALS)
			{
				++number_columns_very_first_primary_variable_group_including_multiplicities;
				if (internal_datetime_column_count < 4)
				{
					++number_columns__in__very_first_primary_variable_group__and__only_its_first_inner_table;
				}

				if (previous_column.column_type == ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__PRIMARY)
				{
					if (previous_column.current_multiplicity__of__current_inner_table__within__current_vg == 1)
					{
						lhs_primary_keys[previous_column.primary_key_dmu_category_identifier].first = 0;
						lhs_primary_keys[previous_column.primary_key_dmu_category_identifier].second.push_back((int)(previous_column_names.size()) - 1);
					}
					else
					{
						if (previous_column.total_multiplicity__of_current_dmu_category__within_uoa_corresponding_to_the_current_inner_tables_variable_group > 1)
						{
							lhs_primary_keys[previous_column.primary_key_dmu_category_identifier].first = 0;
							lhs_primary_keys[previous_column.primary_key_dmu_category_identifier].second.push_back((int)(previous_column_names.size()) - 1);
						}
					}
				}
			}
		}

		if (previous_column.column_type == ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__DATETIMESTART || previous_column.column_type == ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__DATETIMEEND || previous_column.column_type == ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__DATETIMESTART_INTERNAL || previous_column.column_type == ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__DATETIMEEND_INTERNAL || previous_column.column_type == ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__DATETIMESTART_MERGED || previous_column.column_type == ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__DATETIMEEND_MERGED || previous_column.column_type == ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__DATETIMESTART_MERGED_FINAL || previous_column.column_type == ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__DATETIMEEND_MERGED_FINAL || previous_column.column_type == ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__DATETIMESTART_MERGED_BETWEEN_FINALS || previous_column.column_type == ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__DATETIMEEND_MERGED_BETWEEN_FINALS)
		{
			++internal_datetime_column_count;
		}

		// after this for_each exits, this variable includes all datetime columns:
		// the 2 pairs for all inner tables,
		// and the additional 2 pairs for each top-level primary variable group
		// that is included in this set of previous top-level primary variable groups being merged into
		// (represented by this for_each loop)
		++first_full_table_column_count;
	});

	// sanity check
	if (number_columns_very_first_primary_variable_group_including_multiplicities % number_columns__in__very_first_primary_variable_group__and__only_its_first_inner_table != 0)
	{
		boost::format msg("The number of columns in the full set of inner tables for the first primary variable group is not an even multiple of the number of columns in the first inner table.");
		SetFailureMessage(msg.str());
		failed = true;
		return result;
	}

	int number_inner_tables__very_first_primary_variable_group = number_columns_very_first_primary_variable_group_including_multiplicities / number_columns__in__very_first_primary_variable_group__and__only_its_first_inner_table;

	// Datetime columns were not counted.  Add these counts now.
	number_columns__in__very_first_primary_variable_group__and__only_its_first_inner_table += 4;

	// The following does not include the two pairs of datetime columns possibly stuck on the end
	// (which would only be relevant if this top-level variable group has no child variable groups,
	// because those tables would precede the final two pairs of datetime columns anyways)
	number_columns_very_first_primary_variable_group_including_multiplicities += (4 * number_inner_tables__very_first_primary_variable_group);


	int number_columns_very_last_primary_variable_group_including_multiplicities = 0; // corresponding to newly-being-added primary variable group (the last one)
	int number_columns__in__very_last_primary_variable_group__and__only_its_first_inner_table = 0;
	WidgetInstanceIdentifier very_last_primary_variable_group;
	int second_table_column_count = 0;

	// These columns are from the new table (the current primary variable group final results being merged in) being added.
	// This data has a single extra pair of datetime columns at the very end of the very last inner table.
	internal_datetime_column_count = 0;
	std::for_each(primary_variable_group_final_result.columns_in_view.cbegin(), primary_variable_group_final_result.columns_in_view.cend(), [&rhs_primary_keys, &internal_datetime_column_count, &result_columns, &number_columns_very_last_primary_variable_group_including_multiplicities, &number_columns__in__very_last_primary_variable_group__and__only_its_first_inner_table, &very_last_primary_variable_group, &second_table_column_count, &previous_column_names, &count](ColumnsInTempView::ColumnInTempView const & new_table_column)
	{
		previous_column_names.push_back(new_table_column.column_name_in_temporary_table);
		result_columns.columns_in_view.push_back(new_table_column);
		ColumnsInTempView::ColumnInTempView & new_column = result_columns.columns_in_view.back();
		new_column.column_name_in_temporary_table = new_column.column_name_in_temporary_table_no_uuid;
		new_column.column_name_in_temporary_table += "_";
		new_column.column_name_in_temporary_table += newUUID(true);

		if (second_table_column_count == 0)
		{
			very_last_primary_variable_group = new_table_column.variable_group_associated_with_current_inner_table;
		}

		if (new_table_column.variable_group_associated_with_current_inner_table.IsEqual(WidgetInstanceIdentifier::EQUALITY_CHECK_TYPE__STRING_CODE, very_last_primary_variable_group))
		{
			if (new_table_column.column_type != ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__DATETIMESTART && new_table_column.column_type != ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__DATETIMEEND && new_table_column.column_type != ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__DATETIMESTART_INTERNAL && new_table_column.column_type != ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__DATETIMEEND_INTERNAL && new_table_column.column_type != ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__DATETIMESTART_MERGED && new_table_column.column_type != ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__DATETIMEEND_MERGED && new_table_column.column_type != ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__DATETIMESTART_MERGED_FINAL && new_table_column.column_type != ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__DATETIMEEND_MERGED_FINAL && new_table_column.column_type != ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__DATETIMESTART_MERGED_BETWEEN_FINALS && new_table_column.column_type != ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__DATETIMEEND_MERGED_BETWEEN_FINALS)
			{
				++number_columns_very_last_primary_variable_group_including_multiplicities;
				if (internal_datetime_column_count < 4)
				{
					++number_columns__in__very_last_primary_variable_group__and__only_its_first_inner_table;
				}

				if (new_table_column.column_type == ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__PRIMARY)
				{
					if (new_table_column.current_multiplicity__of__current_inner_table__within__current_vg == 1)
					{
						rhs_primary_keys[new_table_column.primary_key_dmu_category_identifier].first = 0;
						rhs_primary_keys[new_table_column.primary_key_dmu_category_identifier].second.push_back((int)(previous_column_names.size()) - 1);
					}
					else
					{
						if (new_table_column.total_multiplicity__of_current_dmu_category__within_uoa_corresponding_to_the_current_inner_tables_variable_group > 1)
						{
							rhs_primary_keys[new_table_column.primary_key_dmu_category_identifier].first = 0;
							rhs_primary_keys[new_table_column.primary_key_dmu_category_identifier].second.push_back((int)(previous_column_names.size()) - 1);
						}
					}
				}
			}
		}

		if (new_table_column.column_type == ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__DATETIMESTART || new_table_column.column_type == ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__DATETIMEEND || new_table_column.column_type == ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__DATETIMESTART_INTERNAL || new_table_column.column_type == ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__DATETIMEEND_INTERNAL || new_table_column.column_type == ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__DATETIMESTART_MERGED || new_table_column.column_type == ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__DATETIMEEND_MERGED || new_table_column.column_type == ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__DATETIMESTART_MERGED_FINAL || new_table_column.column_type == ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__DATETIMEEND_MERGED_FINAL || new_table_column.column_type == ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__DATETIMESTART_MERGED_BETWEEN_FINALS || new_table_column.column_type == ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__DATETIMEEND_MERGED_BETWEEN_FINALS)
		{
			++internal_datetime_column_count;
		}

		// after this for_each exits, this variable includes all datetime columns:
		// the 2 pairs for all inner tables,
		// and the additional single pair for this top-level primary variable group
		++second_table_column_count;
	});

	// sanity check
	if (number_columns_very_last_primary_variable_group_including_multiplicities % number_columns__in__very_last_primary_variable_group__and__only_its_first_inner_table != 0)
	{
		boost::format msg("The number of columns in the full set of inner tables for the last primary variable group is not an even multiple of the number of columns in its first inner table.");
		SetFailureMessage(msg.str());
		failed = true;
		return result;
	}

	int number_inner_tables__very_last_primary_variable_group = number_columns_very_last_primary_variable_group_including_multiplicities / number_columns__in__very_last_primary_variable_group__and__only_its_first_inner_table;

	// Datetime columns were not counted.  Add these counts now.
	number_columns__in__very_last_primary_variable_group__and__only_its_first_inner_table += 4;

	// The following does not include the single pair of datetime columns possibly stuck on the end
	// (which would only be relevant if this top-level variable group has no child variable groups,
	// because those tables would precede the final single pair of datetime columns anyways)
	number_columns_very_last_primary_variable_group_including_multiplicities += (4 * number_inner_tables__very_last_primary_variable_group);


	std::string sql_select_left;
	std::string sql_select_right;
	bool first = true;
	int column_count = 0;
	std::for_each(result_columns.columns_in_view.begin(), result_columns.columns_in_view.end(), [&lhs_primary_keys, &rhs_primary_keys, &number_columns_very_first_primary_variable_group_including_multiplicities, &number_columns_very_last_primary_variable_group_including_multiplicities, &sql_select_left, &sql_select_right, &first, &column_count, &first_full_table_column_count, &second_table_column_count, &previous_column_names](ColumnsInTempView::ColumnInTempView & new_column)
	{

		if (!first)
		{
			sql_select_left += ", ";
			sql_select_right += ", ";
		}
		first = false;

		bool handled = false;

		if (column_count < number_columns_very_first_primary_variable_group_including_multiplicities)
		{
			if (new_column.column_type == ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__PRIMARY)
			{
				if (new_column.current_multiplicity__of__current_inner_table__within__current_vg == 1)
				{
					handled = true;
				}
				else
				{
					if (new_column.total_multiplicity__of_current_dmu_category__within_uoa_corresponding_to_the_current_inner_tables_variable_group > 1)
					{
						handled = true;
					}
				}
				if (handled)
				{
					sql_select_left += "CASE WHEN t1.";
					sql_select_left += previous_column_names[column_count];
					sql_select_left += " IS NOT NULL THEN t1.";
					sql_select_left += previous_column_names[column_count];
					sql_select_left += " ELSE t2.";
					sql_select_left += previous_column_names[rhs_primary_keys[new_column.primary_key_dmu_category_identifier].second[rhs_primary_keys[new_column.primary_key_dmu_category_identifier].first]];
					sql_select_left += " END AS ";
					sql_select_left += new_column.column_name_in_temporary_table;

					sql_select_right += "CASE WHEN t2.";
					sql_select_right += previous_column_names[column_count];
					sql_select_right += " IS NOT NULL THEN t2.";
					sql_select_right += previous_column_names[column_count];
					sql_select_right += " ELSE t1.";
					sql_select_right += previous_column_names[rhs_primary_keys[new_column.primary_key_dmu_category_identifier].second[rhs_primary_keys[new_column.primary_key_dmu_category_identifier].first++]];
					sql_select_right += " END AS ";
					sql_select_right += new_column.column_name_in_temporary_table;
				}
			}
		}
		else if (column_count >= first_full_table_column_count && column_count < first_full_table_column_count + number_columns_very_last_primary_variable_group_including_multiplicities)
		{
			if (new_column.column_type == ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__PRIMARY)
			{
				if (new_column.current_multiplicity__of__current_inner_table__within__current_vg == 1)
				{
					handled = true;
				}
				else
				{
					if (new_column.total_multiplicity__of_current_dmu_category__within_uoa_corresponding_to_the_current_inner_tables_variable_group > 1)
					{
						handled = true;
					}
				}
				if (handled)
				{
					sql_select_right += "CASE WHEN t1.";
					sql_select_right += previous_column_names[column_count];
					sql_select_right += " IS NOT NULL THEN t1.";
					sql_select_right += previous_column_names[column_count];
					sql_select_right += " ELSE t2.";
					sql_select_right += previous_column_names[lhs_primary_keys[new_column.primary_key_dmu_category_identifier].second[lhs_primary_keys[new_column.primary_key_dmu_category_identifier].first]];
					sql_select_right += " END AS ";
					sql_select_right += new_column.column_name_in_temporary_table;

					sql_select_left += "CASE WHEN t2.";
					sql_select_left += previous_column_names[column_count];
					sql_select_left += " IS NOT NULL THEN t2.";
					sql_select_left += previous_column_names[column_count];
					sql_select_left += " ELSE t1.";
					sql_select_left += previous_column_names[lhs_primary_keys[new_column.primary_key_dmu_category_identifier].second[lhs_primary_keys[new_column.primary_key_dmu_category_identifier].first++]];
					sql_select_left += " END AS ";
					sql_select_left += new_column.column_name_in_temporary_table;
				}
			}
		}

		if (!handled)
		{
			if (column_count < first_full_table_column_count)
			{
				sql_select_left += "t1.";
				sql_select_right += "t2.";
			}
			else
			{
				sql_select_left += "t2.";
				sql_select_right += "t1.";
			}

			sql_select_left += previous_column_names[column_count];
			sql_select_left += " AS ";
			sql_select_left += new_column.column_name_in_temporary_table;

			sql_select_right += previous_column_names[column_count];
			sql_select_right += " AS ";
			sql_select_right += new_column.column_name_in_temporary_table;
		}

		++column_count;

	});

	std::vector<std::string> join_column_names_lhs;
	std::vector<std::string> join_column_names_rhs;
	for (int current_multiplicity = 1; current_multiplicity <= highest_multiplicity_primary_uoa; ++current_multiplicity)
	{
		std::for_each(sequence.primary_key_sequence_info.cbegin(), sequence.primary_key_sequence_info.cend(), [this, &number_columns__in__very_first_primary_variable_group__and__only_its_first_inner_table, &join_column_names_lhs, &join_column_names_rhs, &very_first_primary_variable_group, &very_last_primary_variable_group, &number_columns_very_first_primary_variable_group_including_multiplicities, &number_columns_very_last_primary_variable_group_including_multiplicities, &current_multiplicity, &result_columns, &first_full_table_column_count, &second_table_column_count, &previous_column_names](PrimaryKeySequence::PrimaryKeySequenceEntry const & primary_key)
		{
			std::for_each(primary_key.variable_group_info_for_primary_keys.cbegin(), primary_key.variable_group_info_for_primary_keys.cend(), [this, &number_columns__in__very_first_primary_variable_group__and__only_its_first_inner_table, &join_column_names_lhs, &join_column_names_rhs, &very_first_primary_variable_group, &very_last_primary_variable_group, &number_columns_very_first_primary_variable_group_including_multiplicities, &number_columns_very_last_primary_variable_group_including_multiplicities, &current_multiplicity, &primary_key, &result_columns, &first_full_table_column_count, &second_table_column_count, &previous_column_names](PrimaryKeySequence::VariableGroup_PrimaryKey_Info const & primary_key_info_this_variable_group)
			{
				if (primary_key_info_this_variable_group.vg_identifier.IsEqual(WidgetInstanceIdentifier::EQUALITY_CHECK_TYPE__STRING_CODE, very_first_primary_variable_group)
					||
					primary_key_info_this_variable_group.vg_identifier.IsEqual(WidgetInstanceIdentifier::EQUALITY_CHECK_TYPE__STRING_CODE, very_last_primary_variable_group))
				{
					if (primary_key_info_this_variable_group.current_multiplicity == current_multiplicity)
					{
						int column_count = 0;
						std::for_each(result_columns.columns_in_view.cbegin(), result_columns.columns_in_view.cend(), [this, &number_columns__in__very_first_primary_variable_group__and__only_its_first_inner_table, &current_multiplicity, &join_column_names_lhs, &join_column_names_rhs, &very_first_primary_variable_group, &very_last_primary_variable_group, &number_columns_very_first_primary_variable_group_including_multiplicities, &number_columns_very_last_primary_variable_group_including_multiplicities, &primary_key_info_this_variable_group, &first_full_table_column_count, &second_table_column_count, &column_count, &previous_column_names, &primary_key](ColumnsInTempView::ColumnInTempView const & new_column)
						{

							if (column_count < number_columns_very_first_primary_variable_group_including_multiplicities)
							{

								if (primary_key_info_this_variable_group.vg_identifier.IsEqual(WidgetInstanceIdentifier::EQUALITY_CHECK_TYPE__STRING_CODE, very_first_primary_variable_group))
								{

									if (new_column.primary_key_dmu_category_identifier.IsEqual(WidgetInstanceIdentifier::EQUALITY_CHECK_TYPE__STRING_CODE, primary_key.dmu_category))
									{

										int desired_inner_table_index = 0;
										bool match_condition = false;

										// First, join on primary keys whose total multiplicity is 1
										if (primary_key_info_this_variable_group.total_multiplicity == 1)
										{
											if (current_multiplicity == 1)
											{
												if (new_column.current_multiplicity__of__current_inner_table__within__current_vg == 1)
												{
													match_condition = (new_column.primary_key_index_within_total_kad_for_dmu_category >= 0 && (new_column.primary_key_index_within_total_kad_for_dmu_category == primary_key.sequence_number_within_dmu_category_spin_control));
												}
											}
										}
										// Join on primary keys with multiplicity greater than 1
										else
										{
											desired_inner_table_index = current_multiplicity - 1;
											match_condition = (new_column.primary_key_index_within_total_kad_for_dmu_category >= 0 && (new_column.primary_key_index__within_uoa_corresponding_to_variable_group_corresponding_to_current_inner_table__for_dmu_category == primary_key_info_this_variable_group.sequence_number_within_dmu_category_for_this_variable_groups_uoa));
										}

										if (column_count >= desired_inner_table_index * number_columns__in__very_first_primary_variable_group__and__only_its_first_inner_table && column_count < (desired_inner_table_index + 1) * number_columns__in__very_first_primary_variable_group__and__only_its_first_inner_table)
										{
											if (match_condition)
											{
												join_column_names_lhs.push_back(previous_column_names[column_count]);
											}
										}

									}

								}

							}
							else if (column_count >= first_full_table_column_count && column_count < first_full_table_column_count + number_columns_very_last_primary_variable_group_including_multiplicities)
							{

								if (primary_key_info_this_variable_group.vg_identifier.IsEqual(WidgetInstanceIdentifier::EQUALITY_CHECK_TYPE__STRING_CODE, very_last_primary_variable_group))
								{

									if (new_column.primary_key_dmu_category_identifier.IsEqual(WidgetInstanceIdentifier::EQUALITY_CHECK_TYPE__STRING_CODE, primary_key.dmu_category))
									{

										int desired_inner_table_index = 0;
										bool match_condition = false;

										// First, join on primary keys whose total multiplicity is 1
										if (primary_key_info_this_variable_group.total_multiplicity == 1)
										{
											if (current_multiplicity == 1)
											{
												if (new_column.current_multiplicity__of__current_inner_table__within__current_vg == 1)
												{
													match_condition = (new_column.primary_key_index_within_total_kad_for_dmu_category >= 0 && (new_column.primary_key_index_within_total_kad_for_dmu_category == primary_key.sequence_number_within_dmu_category_spin_control));
												}
											}
										}
										// Join on primary keys with multiplicity greater than 1
										else
										{
											desired_inner_table_index = current_multiplicity - 1;
											match_condition = (new_column.primary_key_index_within_total_kad_for_dmu_category >= 0 && (new_column.primary_key_index__within_uoa_corresponding_to_variable_group_corresponding_to_current_inner_table__for_dmu_category == primary_key_info_this_variable_group.sequence_number_within_dmu_category_for_this_variable_groups_uoa));
										}

										if (column_count >= first_full_table_column_count + (desired_inner_table_index * number_columns__in__very_first_primary_variable_group__and__only_its_first_inner_table) && column_count < first_full_table_column_count + ((desired_inner_table_index + 1) * number_columns__in__very_first_primary_variable_group__and__only_its_first_inner_table))
										{
											if (match_condition)
											{
												join_column_names_rhs.push_back(previous_column_names[column_count]);
											}
										}

									}

								}

							}

							++column_count;

						});
					}
				}
			});
		});
	}

	// sanity check
	if (join_column_names_lhs.size() != join_column_names_rhs.size())
	{
		boost::format msg("The number of JOIN primary key columns in a previous top-level variable group does not match the number in the current top-level variable group.");
		SetFailureMessage(msg.str());
		failed = true;
		return result;
	}

	std::string sql_join_on_left;
	std::string sql_join_on_right;

	std::string sql_null_clause;

	int join_index = 0;
	bool and_ = false;
	std::for_each(join_column_names_lhs.cbegin(), join_column_names_lhs.cend(), [&join_index, &sql_join_on_left, &sql_join_on_right, &sql_null_clause, &and_, &join_column_names_rhs](std::string const & join_column_name_lhs)
	{
		if (and_)
		{
			sql_join_on_left += " AND ";
			sql_join_on_right += " AND ";
			sql_null_clause += " AND ";
		}
		and_ = true;

		sql_null_clause += "t2.";
		sql_null_clause += join_column_name_lhs;
		sql_null_clause += " IS NULL";

		sql_join_on_left += "t1.";
		sql_join_on_left += join_column_name_lhs;
		sql_join_on_left += " = t2.";
		sql_join_on_left += join_column_names_rhs[join_index];

		sql_join_on_right += "t1.";
		sql_join_on_right += join_column_names_rhs[join_index];
		sql_join_on_right += " = t2.";
		sql_join_on_right += join_column_name_lhs;

		++join_index;
	});


	std::string sql_order_by;

	// For use in ORDER BY clause
	// Determine how many columns there are corresponding to the DMU category with multiplicity greater than 1
	int number_primary_key_columns_in_dmu_category_with_multiplicity_greater_than_1__for_top_level_uoa = 0;
	if (debug_ordering)
	{
		if (highest_multiplicity_primary_uoa > 1)
		{
			int current_column_count = 0;
			std::for_each(result_columns.columns_in_view.begin(), result_columns.columns_in_view.end(), [this, &current_column_count, &number_columns__in__very_first_primary_variable_group__and__only_its_first_inner_table, &number_primary_key_columns_in_dmu_category_with_multiplicity_greater_than_1__for_top_level_uoa](ColumnsInTempView::ColumnInTempView & view_column)
			{
				if (current_column_count < number_columns__in__very_first_primary_variable_group__and__only_its_first_inner_table)
				{
					if (view_column.column_type == ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__PRIMARY)
					{
						if (view_column.is_within_inner_table_corresponding_to_top_level_uoa)
						{
							if (view_column.total_multiplicity__of_current_dmu_category__within_uoa_corresponding_to_the_current_inner_tables_variable_group == highest_multiplicity_primary_uoa)
							{
								if (view_column.current_multiplicity__corresponding_to__current_inner_table___is_1_in_all_inner_tables_when_multiplicity_is_1_for_that_vg == 1)
								{
									++number_primary_key_columns_in_dmu_category_with_multiplicity_greater_than_1__for_top_level_uoa;
								}
							}
						}
					}
				}
				++current_column_count;
			});
		}
	}

	// Add the ORDER BY column/s
	if (debug_ordering)
	{

		bool first = true;

		if (highest_multiplicity_primary_uoa > 1)
		{

			// Create the ORDER BY clause, taking the proper primary key columns that compose the DMU category with multiplicity greater than 1, in sequence
			for (int outer_dmu_multiplicity = 1; outer_dmu_multiplicity <= highest_multiplicity_primary_uoa; ++outer_dmu_multiplicity)
			{
				for (int inner_dmu_multiplicity = 0; inner_dmu_multiplicity < number_primary_key_columns_in_dmu_category_with_multiplicity_greater_than_1__for_top_level_uoa; ++inner_dmu_multiplicity)
				{
					int current_column_count = 0;
					std::for_each(result_columns.columns_in_view.begin(), result_columns.columns_in_view.end(), [this, &sql_order_by, &current_column_count, &number_columns_very_first_primary_variable_group_including_multiplicities, &number_columns_very_last_primary_variable_group_including_multiplicities, &inner_dmu_multiplicity, &outer_dmu_multiplicity, &first](ColumnsInTempView::ColumnInTempView & view_column)
					{
						if (current_column_count < number_columns_very_first_primary_variable_group_including_multiplicities)
						{
							if (view_column.column_type == ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__PRIMARY)
							{
								if (view_column.total_multiplicity__of_current_dmu_category__within_uoa_corresponding_to_the_current_inner_tables_variable_group == highest_multiplicity_primary_uoa)
								{
									if (view_column.primary_key_index__within_uoa_corresponding_to_variable_group_corresponding_to_current_inner_table__for_dmu_category == inner_dmu_multiplicity)
									{
										if (view_column.current_multiplicity__corresponding_to__current_inner_table___is_1_in_all_inner_tables_when_multiplicity_is_1_for_that_vg == outer_dmu_multiplicity)
										{
											if (view_column.is_within_inner_table_corresponding_to_top_level_uoa)
											{
												if (!first)
												{
													sql_order_by += ", ";
												}
												else
												{
													sql_order_by += " ORDER BY ";
												}
												first = false;
												if (view_column.primary_key_should_be_treated_as_numeric)
												{
													sql_order_by += "CAST (";
												}
												sql_order_by += view_column.column_name_in_temporary_table;
												if (view_column.primary_key_should_be_treated_as_numeric)
												{
													sql_order_by += " AS INTEGER)";
												}
											}
										}
									}
								}
							}
						}
						++current_column_count;
					});
				}
			}

		}

		// Now order by remaining primary key columns (with multiplicity 1)
		// ... If there are no primary key DMU categories for this top-level UOA with multiplicity greater than 1,
		// then this section will order by all of this top-level's UOA primary key DMU categories.
		int current_column = 0;
		std::for_each(result_columns.columns_in_view.begin(), result_columns.columns_in_view.end(), [this, &sql_order_by, &number_columns__in__very_first_primary_variable_group__and__only_its_first_inner_table, &result_columns, &current_column, &first](ColumnsInTempView::ColumnInTempView & view_column)
		{

			if (current_column >= number_columns__in__very_first_primary_variable_group__and__only_its_first_inner_table)
			{
				return;
			}

			// Determine how many columns there are corresponding to the DMU category
			int number_primary_key_columns_in_dmu_category_with_multiplicity_of_1 = 0;
			int column_count_nested = 0;
			std::for_each(result_columns.columns_in_view.begin(), result_columns.columns_in_view.end(), [this, &number_columns__in__very_first_primary_variable_group__and__only_its_first_inner_table, &view_column, &column_count_nested, &number_primary_key_columns_in_dmu_category_with_multiplicity_of_1](ColumnsInTempView::ColumnInTempView & view_column_nested)
			{
				if (column_count_nested >= number_columns__in__very_first_primary_variable_group__and__only_its_first_inner_table)
				{
					return;
				}
				if (view_column_nested.column_type == ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__PRIMARY)
				{
					if (view_column_nested.primary_key_dmu_category_identifier.IsEqual(WidgetInstanceIdentifier::EQUALITY_CHECK_TYPE__STRING_CODE, view_column.primary_key_dmu_category_identifier))
					{
						if (view_column_nested.total_multiplicity__of_current_dmu_category__within_uoa_corresponding_to_the_current_inner_tables_variable_group == 1)
						{
							if (view_column.is_within_inner_table_corresponding_to_top_level_uoa)
							{
								++number_primary_key_columns_in_dmu_category_with_multiplicity_of_1;
							}
						}
					}
				}
				++column_count_nested;
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
									sql_order_by += ", ";
								}
								else
								{
									sql_order_by += " ORDER BY ";
								}
								first = false;
								if (view_column.primary_key_should_be_treated_as_numeric)
								{
									sql_order_by += "CAST (";
								}
								sql_order_by += view_column.column_name_in_temporary_table;
								if (view_column.primary_key_should_be_treated_as_numeric)
								{
									sql_order_by += " AS INTEGER)";
								}
							}
						}
					}
				}
			}
			++current_column;
		});

	}


	sql_strings.push_back(SQLExecutor(db));
	std::string & sql_string = sql_strings.back().sql;

	sql_string = "CREATE TABLE ";
	sql_string += result_columns.view_name;
	sql_string += " AS SELECT ";
	sql_string += sql_select_left;

	sql_string += " FROM ";

	sql_string += previous_merged_primary_variable_groups_table.second.view_name;
	sql_string += " t1 LEFT OUTER JOIN ";
	sql_string += primary_variable_group_final_result.view_name;
	sql_string += " t2 ON ";

	sql_string += sql_join_on_left;

	// Left-side WHERE goes here for timerange limitation

	sql_string += " UNION ALL ";

	sql_string += " SELECT ";

	sql_string += sql_select_right;

	sql_string += " FROM ";
	sql_string += primary_variable_group_final_result.view_name;
	sql_string += " t1 LEFT OUTER JOIN ";
	sql_string += previous_merged_primary_variable_groups_table.second.view_name;
	sql_string += " t2 ON ";

	sql_string += sql_join_on_right;

	sql_string += " WHERE ";

	// Right-side WHERE goes here for timerange limitation.
	// Do not forget AND keyword.

	sql_string += sql_null_clause;

	// Do not ORDER BY at this time.
	// The "CAST" operation cannot be used in an ORDER BY on a compound SELECT statement.
	// To overcome this, new rows need to be added into the WHERE clause for both sub-SELECT's,
	// and the ORDER BY must reference those columns.
	// These columns would have to be added to the column metadata and accounted for everywhere.
	// However, the ORDER BY at this stage is not a requirement for the algorithm,
	// because the data will be ORDERED at a later stage prior to actual removal of duplicates.
	// The ORDER BY is only here for debugging and development purposes.
	bool use_order_by = false;
	if (use_order_by)
	{
		sql_string += sql_order_by;
	}


	return result;

}

void OutputModel::OutputGenerator::DetermineTotalNumberRows()
{

	total_number_primary_rows = 0;
	total_progress_stages = 0;

	int primary_group_number = 1;
	std::for_each(primary_variable_groups_column_info.cbegin(), primary_variable_groups_column_info.cend(), [this, &primary_group_number](ColumnsInTempView const & primary_variable_group_raw_data_columns)
	{

		if (failed)
		{
			return;
		}
		SqlAndColumnSet x_table_result = CreateInitialPrimaryXTable_OrCount(primary_variable_group_raw_data_columns, primary_group_number, true);
		if (failed)
		{
			return;
		}
		x_table_result.first[0].Execute();
		if (!x_table_result.first[0].Step())
		{
			if (primary_variable_group_raw_data_columns.variable_groups[0].longhand)
			{
				boost::format msg("Unable to determine row count for raw data table for variable group %1% (%2%)");
				msg % *primary_variable_group_raw_data_columns.variable_groups[0].code % *primary_variable_group_raw_data_columns.variable_groups[0].longhand;
				SetFailureMessage(msg.str());
			}
			else
			{
				boost::format msg("Unable to determine row count for raw data table for variable group %1%");
				msg % *primary_variable_group_raw_data_columns.variable_groups[0].code;
				SetFailureMessage(msg.str());
			}
			failed = true;
			return;
		}

		std::int64_t number_rows = sqlite3_column_int64(x_table_result.first[0].stmt, 0);
		total_number_incoming_rows[primary_variable_group_raw_data_columns.variable_groups[0]] = number_rows;

		multiplicities[primary_variable_group_raw_data_columns.variable_groups[0]] = highest_multiplicity_primary_uoa;

		total_number_primary_rows += (highest_multiplicity_primary_uoa * number_rows);

		total_progress_stages += highest_multiplicity_primary_uoa;

		++primary_group_number;

	});

	int child_set_number = 1;
	std::for_each(secondary_variable_groups_column_info.cbegin(), secondary_variable_groups_column_info.cend(), [this, &child_set_number](ColumnsInTempView const & child_variable_group_raw_data_columns)
	{

		if (failed)
		{
			return;
		}
		SqlAndColumnSet x_table_result = CreateInitialPrimaryXTable_OrCount(child_variable_group_raw_data_columns, child_set_number, true);
		if (failed)
		{
			return;
		}
		x_table_result.first[0].Execute();
		if (!x_table_result.first[0].Step())
		{
			if (child_variable_group_raw_data_columns.variable_groups[0].longhand)
			{
				boost::format msg("Unable to determine row count for raw data table for variable group %1% (%2%)");
				msg % *child_variable_group_raw_data_columns.variable_groups[0].code % *child_variable_group_raw_data_columns.variable_groups[0].longhand;
				SetFailureMessage(msg.str());
			}
			else
			{
				boost::format msg("Unable to determine row count for raw data table for variable group %1%");
				msg % *child_variable_group_raw_data_columns.variable_groups[0].code;
				SetFailureMessage(msg.str());
			}
			failed = true;
			return;
		}
		std::int64_t number_rows = sqlite3_column_int64(x_table_result.first[0].stmt, 0);
		total_number_incoming_rows[child_variable_group_raw_data_columns.variable_groups[0]] = number_rows;

		int const the_child_multiplicity = child_uoas__which_multiplicity_is_greater_than_1[*(child_variable_group_raw_data_columns.variable_groups[0].identifier_parent)].second;
		multiplicities[child_variable_group_raw_data_columns.variable_groups[0]] = the_child_multiplicity;

		total_progress_stages += the_child_multiplicity;

		++child_set_number;

	});

	total_progress_stages += primary_variable_groups_column_info.size(); // merging of top-level groups

	rough_progress_range = 0;
	std::for_each(total_number_incoming_rows.cbegin(), total_number_incoming_rows.cend(), [this](std::pair<WidgetInstanceIdentifier const, std::int64_t> const & the_pair)
	{
		WidgetInstanceIdentifier const & variable_group = the_pair.first;
		std::int64_t number_raw_rows = the_pair.second;
		int the_multiplicity = multiplicities[variable_group];
		rough_progress_range += number_raw_rows * the_multiplicity;
	});

	// unused for now
	rough_progress_increment_one_percent = rough_progress_range / 125; // leave an extra 20% for the merging of primary groups

	progress_increment_per_stage = 1000 / total_progress_stages;

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
		SqlAndColumnSet primary_group_final_result = ConstructFullOutputForSinglePrimaryGroup(primary_variable_group_raw_data_columns, primary_group_column_sets, primary_group_number);
		if (failed)
		{
			return;
		}
		primary_group_final_results.push_back(primary_group_final_result);
		++primary_group_number;
	});

}

OutputModel::OutputGenerator::SqlAndColumnSet OutputModel::OutputGenerator::ConstructFullOutputForSinglePrimaryGroup(ColumnsInTempView const & primary_variable_group_raw_data_columns, SqlAndColumnSets & sql_and_column_sets, int const primary_group_number)
{

	std::int64_t raw_rows_count = total_number_incoming_rows[primary_variable_group_raw_data_columns.variable_groups[0]];
	std::int64_t rows_estimate = raw_rows_count;

	UpdateProgressBarToNextStage();

	SqlAndColumnSet x_table_result = CreateInitialPrimaryXTable_OrCount(primary_variable_group_raw_data_columns, primary_group_number, false);
	x_table_result.second.most_recent_sql_statement_executed__index = -1;
	ExecuteSQL(x_table_result);
	sql_and_column_sets.push_back(x_table_result);
	if (failed)
	{
		return SqlAndColumnSet();
	}

	SqlAndColumnSet xr_table_result = CreateInitialPrimaryXRTable(x_table_result.second, primary_group_number);
	xr_table_result.second.most_recent_sql_statement_executed__index = -1;
	ExecuteSQL(xr_table_result);
	ClearTables(sql_and_column_sets);
	sql_and_column_sets.push_back(xr_table_result);
	if (failed)
	{
		return SqlAndColumnSet();
	}

	for (int current_multiplicity = 2; current_multiplicity <= highest_multiplicity_primary_uoa; ++current_multiplicity)
	{

		x_table_result = CreatePrimaryXTable(primary_variable_group_raw_data_columns, xr_table_result.second, current_multiplicity, primary_group_number);
		x_table_result.second.most_recent_sql_statement_executed__index = -1;
		ExecuteSQL(x_table_result);
		ClearTables(sql_and_column_sets);
		sql_and_column_sets.push_back(x_table_result);
		if (failed)
		{
			return SqlAndColumnSet();
		}

		UpdateProgressBarToNextStage();
		rows_estimate *= raw_rows_count;
		xr_table_result = CreateXRTable(x_table_result.second, current_multiplicity, primary_group_number, OutputModel::OutputGenerator::PRIMARY_VARIABLE_GROUP, 0, current_multiplicity, rows_estimate);
		ClearTables(sql_and_column_sets);
		sql_and_column_sets.push_back(xr_table_result);
		if (failed)
		{
			return SqlAndColumnSet();
		}

	}

	if (failed)
	{
		return SqlAndColumnSet();
	}

	SqlAndColumnSet preliminary_sorted_top_level_variable_group_result = CreateSortedTable(xr_table_result.second, primary_group_number);
	preliminary_sorted_top_level_variable_group_result.second.most_recent_sql_statement_executed__index = -1;
	ExecuteSQL(preliminary_sorted_top_level_variable_group_result);
	ClearTables(sql_and_column_sets);
	sql_and_column_sets.push_back(preliminary_sorted_top_level_variable_group_result);
	if (failed)
	{
		return SqlAndColumnSet();
	}

	std::int64_t rows_added = 0;
	SqlAndColumnSet duplicates_removed_top_level_variable_group_result = RemoveDuplicates(preliminary_sorted_top_level_variable_group_result.second, primary_group_number, rows_added);
	ClearTables(sql_and_column_sets);
	sql_and_column_sets.push_back(duplicates_removed_top_level_variable_group_result);
	if (failed)
	{
		return SqlAndColumnSet();
	}
	total_number_primary_merged_rows[primary_variable_group_raw_data_columns.variable_groups[0]] = rows_added;

	return duplicates_removed_top_level_variable_group_result;

}

void OutputModel::OutputGenerator::SavedRowData::Clear()
{
	current_parameter_strings.clear();
	current_parameter_ints.clear();
	current_parameter_which_binding_to_use.clear();
	datetime_start = 0;
	datetime_end = 0;
	failed = false;
	indices_of_primary_key_columns.clear();
	is_index_a_primary_key.clear();
	single_inner_table__indices_of_primary_keys_with_multiplicity_greater_than_1__in_top_level_uoa.clear();
}

void OutputModel::OutputGenerator::SavedRowData::PopulateFromCurrentRowInDatabase(ColumnsInTempView & sorted_result_columns, sqlite3_stmt * stmt_result)
{

	Clear();

	int datetime_start_column_index_of_possible_duplicate = (int)sorted_result_columns.columns_in_view.size() - 2;
	int datetime_end_column_index_of_possible_duplicate = (int)sorted_result_columns.columns_in_view.size() - 1;
	std::int64_t datetime_start_of_possible_duplicate = sqlite3_column_int64(stmt_result, datetime_start_column_index_of_possible_duplicate);
	std::int64_t datetime_end_of_possible_duplicate = sqlite3_column_int64(stmt_result, datetime_end_column_index_of_possible_duplicate);

	datetime_start = datetime_start_of_possible_duplicate;
	datetime_end = datetime_end_of_possible_duplicate;

	WidgetInstanceIdentifier first_variable_group;

	int column_data_type = 0;
	std::int64_t data_int64 = 0;
	std::string data_string;
	long double data_long = 0.0;
	int current_column = 0;
	std::for_each(sorted_result_columns.columns_in_view.begin(), sorted_result_columns.columns_in_view.end(), [this, &first_variable_group, &data_int64, &data_string, &data_long, &stmt_result, &column_data_type, &current_column](ColumnsInTempView::ColumnInTempView & possible_duplicate_view_column)
	{

		if (failed)
		{
			return;
		}

		if (current_column == 0)
		{
			first_variable_group = possible_duplicate_view_column.variable_group_associated_with_current_inner_table;
		}

		bool not_first_variable_group = false;
		if (!possible_duplicate_view_column.variable_group_associated_with_current_inner_table.IsEqual(WidgetInstanceIdentifier::EQUALITY_CHECK_TYPE__STRING_CODE, first_variable_group))
		{
			not_first_variable_group = true;
		}

		bool add_as_primary_key_column = false;
		bool add_as_primary_key_with_multiplicity_greater_than_1_in_first_inner_table = false;

		if (possible_duplicate_view_column.is_within_inner_table_corresponding_to_top_level_uoa)
		{
			if (possible_duplicate_view_column.current_multiplicity__corresponding_to__current_inner_table___is_1_in_all_inner_tables_when_multiplicity_is_1_for_that_vg == 1)
			{
				if (possible_duplicate_view_column.column_type == ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__PRIMARY)
				{
					add_as_primary_key_column = true;
					if (possible_duplicate_view_column.total_multiplicity__of_current_dmu_category__within_uoa_corresponding_to_the_current_inner_tables_variable_group > 1)
					{
						add_as_primary_key_with_multiplicity_greater_than_1_in_first_inner_table = true;
					}
				}
			}
			else
			{
				// Only primary keys have the following value set to anything but -1,
				// and only those primary keys we wish to capture in inner tables beyond the first
				// have the value greater than 1.
				if (possible_duplicate_view_column.total_multiplicity__of_current_dmu_category__within_uoa_corresponding_to_the_current_inner_tables_variable_group > 1)
				{
					add_as_primary_key_column = true;
				}
			}
		}

		if (not_first_variable_group)
		{
			add_as_primary_key_column = false;
		}

		column_data_type = sqlite3_column_type(stmt_result, current_column);
		switch (column_data_type)
		{

			case SQLITE_INTEGER:
				{
					data_int64 = sqlite3_column_int64(stmt_result, current_column);
					current_parameter_ints.push_back(data_int64);
					current_parameter_which_binding_to_use.push_back(SQLExecutor::INT64);
					if (add_as_primary_key_column)
					{
						indices_of_primary_key_columns.push_back(std::make_pair(SQLExecutor::INT64, (int)current_parameter_ints.size()-1));
						is_index_a_primary_key.push_back(true);
						if (add_as_primary_key_with_multiplicity_greater_than_1_in_first_inner_table)
						{
							single_inner_table__indices_of_primary_keys_with_multiplicity_greater_than_1__in_top_level_uoa.push_back(std::make_pair(OutputModel::OutputGenerator::SQLExecutor::INT64, (int)current_parameter_ints.size()-1));
						}
					}
					else
					{
						is_index_a_primary_key.push_back(false);
					}
				}
				break;

			case SQLITE_FLOAT:
				{
					// Currently not implemented!!!!!!!  Just add new bound_paramenter_longs as argument to this function, and as member of SQLExecutor just like the other bound_parameter data members, to implement.
					data_long = sqlite3_column_double(stmt_result, current_column);
					// Todo: Error message
					boost::format msg("Floating point values are not yet supported.");
					error_message = msg.str();
					failed = true;
					return; // from lambda
				}
				break;

			case SQLITE_TEXT:
				{
					data_string = reinterpret_cast<char const *>(sqlite3_column_text(stmt_result, current_column));
					current_parameter_strings.push_back(data_string);
					current_parameter_which_binding_to_use.push_back(SQLExecutor::STRING);
					if (add_as_primary_key_column)
					{
						indices_of_primary_key_columns.push_back(std::make_pair(SQLExecutor::STRING, (int)current_parameter_strings.size()-1));
						is_index_a_primary_key.push_back(true);
						if (add_as_primary_key_with_multiplicity_greater_than_1_in_first_inner_table)
						{
							single_inner_table__indices_of_primary_keys_with_multiplicity_greater_than_1__in_top_level_uoa.push_back(std::make_pair(OutputModel::OutputGenerator::SQLExecutor::STRING, (int)current_parameter_strings.size()-1));
						}
					}
					else
					{
						is_index_a_primary_key.push_back(false);
					}
				}
				break;

			case SQLITE_BLOB:
				{
					// Todo: Error message
					boost::format msg("BLOBs are not supported.");
					error_message = msg.str();
					failed = true;
					return; // from lambda
				}
				break;

			case SQLITE_NULL:
				{
					current_parameter_which_binding_to_use.push_back(SQLExecutor::NULL_BINDING);
					if (add_as_primary_key_column)
					{
						indices_of_primary_key_columns.push_back(std::make_pair(SQLExecutor::NULL_BINDING, 0));
						is_index_a_primary_key.push_back(true);
						if (add_as_primary_key_with_multiplicity_greater_than_1_in_first_inner_table)
						{
							single_inner_table__indices_of_primary_keys_with_multiplicity_greater_than_1__in_top_level_uoa.push_back(std::make_pair(OutputModel::OutputGenerator::SQLExecutor::NULL_BINDING, 0));
						}
					}
					else
					{
						is_index_a_primary_key.push_back(false);
					}
				}
				break;

			default:
				{
					// Todo: Error message
					boost::format msg("Unknown data type in column in database.");
					error_message = msg.str();
					failed = true;
					return; // from lambda
				}

		}

		++current_column;

	});

}

OutputModel::OutputGenerator::SqlAndColumnSet OutputModel::OutputGenerator::RemoveDuplicates(ColumnsInTempView & sorted_result_columns, int const primary_group_number, std::int64_t & current_rows_added)
{

	char c[256];

	bool is_xrmfxr_table = false;
	if (primary_group_number == 0)
	{
		is_xrmfxr_table = true;
	}

	SqlAndColumnSet result = std::make_pair(std::vector<SQLExecutor>(), ColumnsInTempView());
	std::vector<SQLExecutor> & sql_strings = result.first;
	ColumnsInTempView & result_columns = result.second;

	result_columns = sorted_result_columns;
	result_columns.most_recent_sql_statement_executed__index = -1;

	std::string view_name;
	if (!is_xrmfxr_table)
	{
		view_name += "DR";
	}
	else
	{
		view_name += "KAD";
	}
	view_name += itoa(primary_group_number, c, 10);
	result_columns.view_name_no_uuid = view_name;
	view_name += "_";
	view_name += newUUID(true);
	result_columns.view_name = view_name;
	result_columns.view_number = 1;
	result_columns.has_no_datetime_columns = false;

	std::string sql_create_empty_table;
	sql_create_empty_table += "CREATE TABLE ";
	sql_create_empty_table += result_columns.view_name;
	sql_create_empty_table += " AS SELECT * FROM ";
	sql_create_empty_table += sorted_result_columns.view_name;
	sql_create_empty_table += " WHERE 0";
	sql_strings.push_back(SQLExecutor(db, sql_create_empty_table));


	// Add the "merged" time range columns

	// The variable group is that of the primary variable group for this final result set,
	// which is obtained from the first column
	WidgetInstanceIdentifier variable_group = sorted_result_columns.columns_in_view[0].variable_group_associated_with_current_inner_table;
	WidgetInstanceIdentifier uoa = sorted_result_columns.columns_in_view[0].uoa_associated_with_variable_group_associated_with_current_inner_table;

	std::string datetime_start_col_name_no_uuid;
	if (!is_xrmfxr_table)
	{
		datetime_start_col_name_no_uuid = "DATETIME_ROW_START_MERGED_FINAL";
	}
	else
	{
		datetime_start_col_name_no_uuid = "DATETIMESTART_MERGED_KAD_OUTPUT";
	}
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
	if (!is_xrmfxr_table)
	{
		datetime_start_column.column_type = ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__DATETIMESTART_MERGED_FINAL;
	}
	else
	{
		datetime_start_column.column_type = ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__DATETIMESTART_MERGED_KAD_OUTPUT;
	}
	if (!is_xrmfxr_table)
	{
		datetime_start_column.variable_group_associated_with_current_inner_table = variable_group;
		datetime_start_column.uoa_associated_with_variable_group_associated_with_current_inner_table = uoa;
	}
	datetime_start_column.column_name_in_original_data_table = "";
	datetime_start_column.inner_table_set_number = -1;
	datetime_start_column.is_within_inner_table_corresponding_to_top_level_uoa = false;

	std::string datetime_end_col_name_no_uuid;
	if (!is_xrmfxr_table)
	{
		datetime_end_col_name_no_uuid = "DATETIME_ROW_END_MERGED_FINAL";
	}
	else
	{
		datetime_end_col_name_no_uuid = "DATETIMEEND_MERGED_KAD_OUTPUT";
	}
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
	if (!is_xrmfxr_table)
	{
		datetime_end_column.column_type = ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__DATETIMEEND_MERGED_FINAL;
	}
	else
	{
		datetime_end_column.column_type = ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__DATETIMEEND_MERGED_KAD_OUTPUT;
	}
	if (!is_xrmfxr_table)
	{
		datetime_end_column.variable_group_associated_with_current_inner_table = variable_group;
		datetime_end_column.uoa_associated_with_variable_group_associated_with_current_inner_table = uoa;
	}
	datetime_end_column.column_name_in_original_data_table = "";
	datetime_end_column.inner_table_set_number = -1;
	datetime_end_column.is_within_inner_table_corresponding_to_top_level_uoa = false;

	ExecuteSQL(result); // Executes all SQL queries up to the current one

	if (failed)
	{
		return result;
	}


	int const minimum_desired_rows_per_transaction = 256;
	current_rows_added = 0;
	int current_rows_added_since_execution = 0;
	std::string sql_add_xr_row;
	bool first_row_added = true;
	std::vector<std::string> bound_parameter_strings;
	std::vector<std::int64_t> bound_parameter_ints;
	std::vector<SQLExecutor::WHICH_BINDING> bound_parameter_which_binding_to_use;
	sqlite3_stmt * the_prepared_stmt = nullptr;

	ObtainData(sorted_result_columns);

	if (failed)
	{
		return result;
	}

	BeginNewTransaction();

	std::deque<SavedRowData> incoming_rows_of_data;
	std::deque<SavedRowData> intermediate_rows_of_data;
	std::deque<SavedRowData> outgoing_rows_of_data;
	std::deque<SavedRowData> rows_to_sort;
	SavedRowData sorting_row_of_data;

	bool start_fresh = true;

	while (StepData())
	{

		sorting_row_of_data.PopulateFromCurrentRowInDatabase(sorted_result_columns, stmt_result);
		failed = sorting_row_of_data.failed;
		if (failed)
		{
			SetFailureMessage(sorting_row_of_data.error_message);
			return result;
		}

		if (start_fresh)
		{
			rows_to_sort.push_back(sorting_row_of_data);
			start_fresh = false;
			continue;
		}

		// ******************************************************************************************************** //
		// Important note on matching primary key columns:
		// A NULL primary key column will match against a non-NULL one.
		// Noting that this algorithm orders primary key columns from left-to-right across columns
		//    (for multiplicity > 1 keys),
		// then we see that this primary key matching approach involving NULLs
		// only works because SQLite orders rows with NULLs before rows with non-NULLs in the ORDER BY clause.
		// If you think about it, the number of NULL columns can only decrease as new rows are introduced
		//    in the current loop, which must match on all previous (non-NULL) columns.
		// Therefore, it is impossible for two non-matching rows to both match against a previously processed
		//    "matching" row (which had matched on some NULL columns)
		//    which themselves do not match on all primary key columns.
		// ******************************************************************************************************** //
		bool primary_keys_match = TestIfCurrentRowMatchesPrimaryKeys(sorting_row_of_data, rows_to_sort[0]);
		if (primary_keys_match)
		{
			rows_to_sort.push_back(sorting_row_of_data);
		}
		else
		{
			RemoveDuplicatesFromPrimaryKeyMatches(result, rows_to_sort, incoming_rows_of_data, outgoing_rows_of_data, intermediate_rows_of_data, datetime_start_col_name, datetime_end_col_name, the_prepared_stmt, sql_strings, result_columns, sorted_result_columns, current_rows_added, current_rows_added_since_execution, sql_add_xr_row, first_row_added, bound_parameter_strings, bound_parameter_ints, bound_parameter_which_binding_to_use, minimum_desired_rows_per_transaction);
			if (failed)
			{
				return result;
			}
			rows_to_sort.push_back(sorting_row_of_data);
		}

	}

	if (!rows_to_sort.empty())
	{
		RemoveDuplicatesFromPrimaryKeyMatches(result, rows_to_sort, incoming_rows_of_data, outgoing_rows_of_data, intermediate_rows_of_data, datetime_start_col_name, datetime_end_col_name, the_prepared_stmt, sql_strings, result_columns, sorted_result_columns, current_rows_added, current_rows_added_since_execution, sql_add_xr_row, first_row_added, bound_parameter_strings, bound_parameter_ints, bound_parameter_which_binding_to_use, minimum_desired_rows_per_transaction);
		if (failed)
		{
			return result;
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

bool OutputModel::OutputGenerator::ProcessCurrentDataRowOverlapWithFrontSavedRow(SavedRowData & first_incoming_row, SavedRowData & current_row_of_data, std::deque<SavedRowData> & intermediate_rows_of_data, std::int64_t & current_rows_added)
{

	if (current_row_of_data.datetime_start >= first_incoming_row.datetime_end)
	{
		boost::format msg("Time range error during merging of data tables: the starting datetime of the current row (%1%) is greater than or equal to the ending datetime of the previous row (%2%).");
		msg % current_row_of_data.datetime_start % first_incoming_row.datetime_end;
		SetFailureMessage(msg.str());
		failed = true;
		return false;
	}

	if (current_row_of_data.datetime_start < first_incoming_row.datetime_start)
	{
		if (current_row_of_data.datetime_end <= first_incoming_row.datetime_start)
		{
			// Rows do not overlap
			intermediate_rows_of_data.push_back(current_row_of_data);
			intermediate_rows_of_data.push_back(first_incoming_row);
			return true; // current_row_complete
		}
		else
		{
			// Rows overlap
			SavedRowData new_data_row = current_row_of_data;
			new_data_row.datetime_end = first_incoming_row.datetime_start;
			intermediate_rows_of_data.push_back(new_data_row);
			current_row_of_data.datetime_start = first_incoming_row.datetime_start;
		}
	}

	if (current_row_of_data.datetime_start > first_incoming_row.datetime_start)
	{
		if (first_incoming_row.datetime_end <= current_row_of_data.datetime_start)
		{
			// Rows do not overlap
			intermediate_rows_of_data.push_back(first_incoming_row);
			return false; // current_row is not complete
		}
		else
		{
			// Rows overlap
			SavedRowData new_data_row = first_incoming_row;
			new_data_row.datetime_end = current_row_of_data.datetime_start;
			intermediate_rows_of_data.push_back(new_data_row);
			first_incoming_row.datetime_start = current_row_of_data.datetime_start;
		}
	}


	// If we're here, it's guaranteed that:
	// current_row_of_data.datetime_start == first_incoming_row.datetime_start
	// ... with overlap

	if (current_row_of_data.datetime_end < first_incoming_row.datetime_end)
	{
		// merge from:
		// current_row_of_data.datetime_start to current_row_of_data.datetime_end
		SavedRowData merged_data_row = MergeRows(current_row_of_data, first_incoming_row);
		if (failed)
		{
			return false;
		}
		merged_data_row.datetime_start = current_row_of_data.datetime_start;
		merged_data_row.datetime_end = current_row_of_data.datetime_end;
		intermediate_rows_of_data.push_back(merged_data_row);

		// then:
		SavedRowData new_data_row = first_incoming_row;
		new_data_row.datetime_start = current_row_of_data.datetime_end;
		intermediate_rows_of_data.push_back(new_data_row);

		return true; // current_row_complete
	}
	else if (current_row_of_data.datetime_end == first_incoming_row.datetime_end)
	{
		// merge from:
		// current_row_of_data.datetime_start to current_row_of_data.datetime_end
		SavedRowData merged_data_row = MergeRows(current_row_of_data, first_incoming_row);
		if (failed)
		{
			return false;
		}
		merged_data_row.datetime_start = current_row_of_data.datetime_start;
		merged_data_row.datetime_end = current_row_of_data.datetime_end;
		intermediate_rows_of_data.push_back(merged_data_row);

		return true; // current_row_complete
	}
	else
	{
		// current_row_of_data.datetime_end > first_incoming_row.datetime_end

		// merge from:
		// first_incoming_row.datetime_start to first_incoming_row.datetime_end
		SavedRowData merged_data_row = MergeRows(current_row_of_data, first_incoming_row);
		if (failed)
		{
			return false;
		}
		merged_data_row.datetime_start = first_incoming_row.datetime_start;
		merged_data_row.datetime_end = first_incoming_row.datetime_end;
		intermediate_rows_of_data.push_back(merged_data_row);

		current_row_of_data.datetime_start = first_incoming_row.datetime_end;

		return false; // current_row is not complete
	}
}

OutputModel::OutputGenerator::SavedRowData OutputModel::OutputGenerator::MergeRows(SavedRowData const & current_row_of_data, SavedRowData const & first_incoming_row)
{
	int int_index_current = 0;
	int string_index_current = 0;
	int int_index_incoming = 0;
	int string_index_incoming = 0;
	int current_index = 0;
	SavedRowData merged_data_row;
	std::for_each(current_row_of_data.current_parameter_which_binding_to_use.cbegin(), current_row_of_data.current_parameter_which_binding_to_use.cend(), [&current_row_of_data, &int_index_current, &string_index_current, &int_index_incoming, &string_index_incoming, &current_index, &first_incoming_row, &merged_data_row](SQLExecutor::WHICH_BINDING const & current_binding)
	{
		SQLExecutor::WHICH_BINDING const first_incoming_row_binding = first_incoming_row.current_parameter_which_binding_to_use[current_index];

		if (current_binding == SQLExecutor::NULL_BINDING && first_incoming_row_binding == SQLExecutor::NULL_BINDING)
		{
			merged_data_row.current_parameter_which_binding_to_use.push_back(SQLExecutor::NULL_BINDING);
			if (current_row_of_data.is_index_a_primary_key[current_index])
			{
				merged_data_row.is_index_a_primary_key.push_back(true);
				merged_data_row.indices_of_primary_key_columns.push_back(std::make_pair(OutputModel::OutputGenerator::SQLExecutor::NULL_BINDING, 0));
			}
			else
			{
				merged_data_row.is_index_a_primary_key.push_back(false);
			}
		}
		else
		{
			if (current_binding != SQLExecutor::NULL_BINDING)
			{
				switch (current_binding)
				{
					case SQLExecutor::INT64:
						{
							merged_data_row.current_parameter_which_binding_to_use.push_back(SQLExecutor::INT64);
							merged_data_row.current_parameter_ints.push_back(current_row_of_data.current_parameter_ints[int_index_current]);
							if (current_row_of_data.is_index_a_primary_key[current_index])
							{
								merged_data_row.is_index_a_primary_key.push_back(true);
								merged_data_row.indices_of_primary_key_columns.push_back(std::make_pair(OutputModel::OutputGenerator::SQLExecutor::INT64, merged_data_row.current_parameter_ints.size()-1));
							}
							else
							{
								merged_data_row.is_index_a_primary_key.push_back(false);
							}
						}
						break;
					case SQLExecutor::STRING:
						{
							merged_data_row.current_parameter_which_binding_to_use.push_back(SQLExecutor::STRING);
							merged_data_row.current_parameter_strings.push_back(current_row_of_data.current_parameter_strings[string_index_current]);
							if (current_row_of_data.is_index_a_primary_key[current_index])
							{
								merged_data_row.is_index_a_primary_key.push_back(true);
								merged_data_row.indices_of_primary_key_columns.push_back(std::make_pair(OutputModel::OutputGenerator::SQLExecutor::STRING, merged_data_row.current_parameter_strings.size()-1));
							}
							else
							{
								merged_data_row.is_index_a_primary_key.push_back(false);
							}
						}
						break;
				}
			}
			else
			{
				switch (first_incoming_row_binding)
				{
					case SQLExecutor::INT64:
						{
							merged_data_row.current_parameter_which_binding_to_use.push_back(SQLExecutor::INT64);
							merged_data_row.current_parameter_ints.push_back(first_incoming_row.current_parameter_ints[int_index_incoming]);
							if (current_row_of_data.is_index_a_primary_key[current_index])
							{
								merged_data_row.is_index_a_primary_key.push_back(true);
								merged_data_row.indices_of_primary_key_columns.push_back(std::make_pair(OutputModel::OutputGenerator::SQLExecutor::INT64, merged_data_row.current_parameter_ints.size()-1));
							}
							else
							{
								merged_data_row.is_index_a_primary_key.push_back(false);
							}
						}
						break;
					case SQLExecutor::STRING:
						{
							merged_data_row.current_parameter_which_binding_to_use.push_back(SQLExecutor::STRING);
							merged_data_row.current_parameter_strings.push_back(first_incoming_row.current_parameter_strings[string_index_incoming]);
							if (current_row_of_data.is_index_a_primary_key[current_index])
							{
								merged_data_row.is_index_a_primary_key.push_back(true);
								merged_data_row.indices_of_primary_key_columns.push_back(std::make_pair(OutputModel::OutputGenerator::SQLExecutor::STRING, merged_data_row.current_parameter_strings.size()-1));
							}
							else
							{
								merged_data_row.is_index_a_primary_key.push_back(false);
							}
						}
						break;
				}
			}
		}

		switch (current_binding)
		{
			case SQLExecutor::INT64:
				{
					++int_index_current;
				}
				break;
			case SQLExecutor::STRING:
				{
					++string_index_current;
				}
				break;
		}

		switch (first_incoming_row_binding)
		{
			case SQLExecutor::INT64:
				{
					++int_index_incoming;
				}
				break;
			case SQLExecutor::STRING:
				{
					++string_index_incoming;
				}
				break;
		}

		++current_index;
	});

	// sanity checks
	if (current_row_of_data.indices_of_primary_key_columns.size() != first_incoming_row.indices_of_primary_key_columns.size())
	{
		boost::format msg("The number of primary key columns in the current row (%1%) is not equal to the number of primary keys of the previous row (%2%).");
		msg % current_row_of_data.indices_of_primary_key_columns.size() % first_incoming_row.indices_of_primary_key_columns.size();
		SetFailureMessage(msg.str());
		failed = true;
		return merged_data_row;
	}

	return merged_data_row;
}

bool OutputModel::OutputGenerator::TestIfCurrentRowMatchesPrimaryKeys(SavedRowData const & current_row_of_data, SavedRowData const & previous_row_of_data)
{

	bool match_failed = false;
	int entry_number = 0;
	std::for_each(current_row_of_data.indices_of_primary_key_columns.cbegin(), current_row_of_data.indices_of_primary_key_columns.cend(), [this, &entry_number, &current_row_of_data, &previous_row_of_data, &match_failed](std::pair<SQLExecutor::WHICH_BINDING, int> const & current_info)
	{

		if (match_failed)
		{
			return;
		}

		SQLExecutor::WHICH_BINDING binding_current = current_info.first;
		int index_current = current_info.second;
		std::int64_t data_int_current = 0;
		std::string data_string_current;
		bool data_null_current = false;

		SQLExecutor::WHICH_BINDING binding_previous = previous_row_of_data.indices_of_primary_key_columns[entry_number].first;
		int index_previous = previous_row_of_data.indices_of_primary_key_columns[entry_number].second;
		std::int64_t data_int_previous = 0;
		std::string data_string_previous;

		switch (binding_current)
		{

			case SQLExecutor::INT64:
				{

					data_int_current = current_row_of_data.current_parameter_ints[index_current];

					switch (binding_previous)
					{
						case SQLExecutor::INT64:
							{
								data_int_previous = previous_row_of_data.current_parameter_ints[index_previous];
								if (data_int_current != data_int_previous)
								{
									match_failed = true;
								}
							}
							break;
						case SQLExecutor::STRING:
							{
								data_string_previous = previous_row_of_data.current_parameter_strings[index_previous];
								if (boost::lexical_cast<std::int64_t>(data_string_previous) != data_int_current)
								{
									match_failed = true;
								}
							}
							break;
						case SQLExecutor::NULL_BINDING:
							{
								// NULLs match against non-NULLs
							}
							break;
					}

				}
				break;

			case SQLExecutor::STRING:
				{

					data_string_current = current_row_of_data.current_parameter_strings[index_current];

					switch (binding_previous)
					{
						case SQLExecutor::INT64:
							{
								data_int_previous = previous_row_of_data.current_parameter_ints[index_previous];
								if (boost::lexical_cast<std::int64_t>(data_string_current) != data_int_previous)
								{
									match_failed = true;
								}
							}
							break;
						case SQLExecutor::STRING:
							{
								data_string_previous = previous_row_of_data.current_parameter_strings[index_previous];
								if (!boost::iequals(data_string_current, data_string_previous))
								{
									match_failed = true;
								}
							}
							break;
						case SQLExecutor::NULL_BINDING:
							{
								// NULLs match against non-NULLs
							}
							break;
					}

				}
				break;

			case SQLExecutor::NULL_BINDING:
				{

					data_null_current = true;

					switch (binding_previous)
					{
						case SQLExecutor::INT64:
							{
								// NULLs match against non-NULLs
							}
							break;
						case SQLExecutor::STRING:
							{
								// NULLs match against non-NULLs
							}
							break;
						case SQLExecutor::NULL_BINDING:
							{
								// NULLs match against NULLs
							}
							break;
					}

				}
				break;

		}

		++entry_number;

	});

	return !match_failed;

}

void OutputModel::OutputGenerator::WriteRowsToFinalTable(std::deque<SavedRowData> & outgoing_rows_of_data, std::string const & datetime_start_col_name, std::string const & datetime_end_col_name, sqlite3_stmt *& the_prepared_stmt, std::vector<SQLExecutor> & sql_strings, sqlite3 * db, std::string & result_columns_view_name, ColumnsInTempView & preliminary_sorted_top_level_variable_group_result_columns, std::int64_t & current_rows_added, int & current_rows_added_since_execution, std::string & sql_add_xr_row, bool & first_row_added, std::vector<std::string> & bound_parameter_strings, std::vector<std::int64_t> & bound_parameter_ints, std::vector<SQLExecutor::WHICH_BINDING> & bound_parameter_which_binding_to_use)
{

	std::for_each(outgoing_rows_of_data.cbegin(), outgoing_rows_of_data.cend(), [this, &datetime_start_col_name, &datetime_end_col_name, &the_prepared_stmt, &sql_strings, &db, &result_columns_view_name, &preliminary_sorted_top_level_variable_group_result_columns, &current_rows_added, &current_rows_added_since_execution, &sql_add_xr_row, &first_row_added, &bound_parameter_strings, &bound_parameter_ints, &bound_parameter_which_binding_to_use](SavedRowData const & row_of_data)
	{

		bool do_not_check_time_range = false;
		if (row_of_data.datetime_start == 0 && row_of_data.datetime_end == 0)
		{
			do_not_check_time_range = true;
		}
		if (!do_not_check_time_range)
		{
			if (row_of_data.datetime_start >= timerange_end)
			{
				return;
			}
			if (row_of_data.datetime_end <= timerange_start)
			{
				return;
			}
		}
	
		if (first_row_added)
		{

			// Create SQL statement here, including placeholders for bound parameters

			sql_add_xr_row.clear();

			sql_add_xr_row += "INSERT OR FAIL INTO ";
			sql_add_xr_row += result_columns_view_name;
			sql_add_xr_row += "(";

			bool first_column_name = true;
			std::for_each(preliminary_sorted_top_level_variable_group_result_columns.columns_in_view.cbegin(), preliminary_sorted_top_level_variable_group_result_columns.columns_in_view.cend(), [&first_column_name, &sql_add_xr_row, &bound_parameter_strings, &bound_parameter_ints, &bound_parameter_which_binding_to_use](ColumnsInTempView::ColumnInTempView const & column_in_view)
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
			std::for_each(preliminary_sorted_top_level_variable_group_result_columns.columns_in_view.cbegin(), preliminary_sorted_top_level_variable_group_result_columns.columns_in_view.cend(), [&first_column_value, &index, &cindex, &sql_add_xr_row, &bound_parameter_strings, &bound_parameter_ints, &bound_parameter_which_binding_to_use](ColumnsInTempView::ColumnInTempView const & column_in_view)
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

		// Set the list of bound parameters, regardless of whether or not the SQL string was created
		int int_index = 0;
		int string_index = 0;
		char cindex[256];
		bool first_column_value = true;
		std::int64_t data_int64 = 0;
		std::string data_string;
		long double data_long = 0.0;
		bound_parameter_strings.clear();
		bound_parameter_ints.clear();
		bound_parameter_which_binding_to_use.clear();
		std::for_each(row_of_data.current_parameter_which_binding_to_use.cbegin(), row_of_data.current_parameter_which_binding_to_use.cend(), [this, &row_of_data, &data_int64, &data_string, &data_long, &first_column_value, &int_index, &string_index, &cindex, &sql_add_xr_row, &bound_parameter_strings, &bound_parameter_ints, &bound_parameter_which_binding_to_use](SQLExecutor::WHICH_BINDING const & the_binding)
		{

			if (failed)
			{
				return;
			}

			switch (the_binding)
			{

				case SQLExecutor::INT64:
					{
						data_int64 = row_of_data.current_parameter_ints[int_index];
						++int_index;
						bound_parameter_ints.push_back(data_int64);
						bound_parameter_which_binding_to_use.push_back(SQLExecutor::INT64);
					}
					break;

				case SQLExecutor::STRING:
					{
						data_string = row_of_data.current_parameter_strings[string_index];
						++string_index;
						bound_parameter_strings.push_back(data_string);
						bound_parameter_which_binding_to_use.push_back(SQLExecutor::STRING);
					}
					break;

				case SQLExecutor::NULL_BINDING:
					{
						bound_parameter_which_binding_to_use.push_back(SQLExecutor::NULL_BINDING);
					}
					break;

				default:
					{
						// Todo: Error message
						boost::format msg("Unknown data type binding in column when writing row to database.");
						SetFailureMessage(msg.str());
						failed = true;
						return; // from lambda
					}

			}

		});

		// The two new "merged" time range columns
		bound_parameter_ints.push_back(row_of_data.datetime_start);
		bound_parameter_which_binding_to_use.push_back(SQLExecutor::INT64);
		bound_parameter_ints.push_back(row_of_data.datetime_end);
		bound_parameter_which_binding_to_use.push_back(SQLExecutor::INT64);
		
		sql_strings.push_back(SQLExecutor(db, sql_add_xr_row, bound_parameter_strings, bound_parameter_ints, bound_parameter_which_binding_to_use, the_prepared_stmt, true));
		the_prepared_stmt = sql_strings.back().stmt;
		++current_rows_added;
		++current_rows_added_since_execution;

	});

}

OutputModel::OutputGenerator::SqlAndColumnSet OutputModel::OutputGenerator::CreateSortedTable(ColumnsInTempView const & final_xr_or_xrmfxr_columns, int const primary_group_number)
{

	char c[256];

	bool is_xrmfxr_table = false;
	if (primary_group_number == 0)
	{
		is_xrmfxr_table = true;
	}

	SqlAndColumnSet result = std::make_pair(std::vector<SQLExecutor>(), ColumnsInTempView());
	std::vector<SQLExecutor> & sql_strings = result.first;
	ColumnsInTempView & result_columns = result.second;

	result_columns = final_xr_or_xrmfxr_columns;
	result_columns.most_recent_sql_statement_executed__index = -1;

	std::string view_name;
	if (!is_xrmfxr_table)
	{
		view_name += "S";
	}
	else
	{
		view_name += "MFXRMF";
	}
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
	sql_create_final_primary_group_table += final_xr_or_xrmfxr_columns.view_name;

	bool first = true;

	WidgetInstanceIdentifier first_variable_group;

	if (highest_multiplicity_primary_uoa > 1)
	{

		// Determine how many columns there are corresponding to the DMU category with multiplicity greater than 1
		first = true;
		int number_primary_key_columns_in_dmu_category_with_multiplicity_greater_than_1 = 0;
		std::for_each(result_columns.columns_in_view.begin(), result_columns.columns_in_view.end(), [this, &first_variable_group, &first, &number_primary_key_columns_in_dmu_category_with_multiplicity_greater_than_1](ColumnsInTempView::ColumnInTempView & view_column)
		{
			if (first)
			{
				first_variable_group = view_column.variable_group_associated_with_current_inner_table;
			}
			first = false;

			if (!view_column.variable_group_associated_with_current_inner_table.IsEqual(WidgetInstanceIdentifier::EQUALITY_CHECK_TYPE__STRING_CODE, first_variable_group))
			{
				return;
			}

			if (view_column.column_type == ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__PRIMARY)
			{
				if (view_column.is_within_inner_table_corresponding_to_top_level_uoa)
				{
					if (view_column.total_multiplicity__of_current_dmu_category__within_uoa_corresponding_to_the_current_inner_tables_variable_group == highest_multiplicity_primary_uoa)
					{
						if (view_column.current_multiplicity__corresponding_to__current_inner_table___is_1_in_all_inner_tables_when_multiplicity_is_1_for_that_vg == 1)
						{
							++number_primary_key_columns_in_dmu_category_with_multiplicity_greater_than_1;
						}
					}
				}
			}
		});

		// Create the ORDER BY clause, taking the proper primary key columns that compose the DMU category with multiplicity greater than 1, in sequence
		first = true;
		for (int outer_dmu_multiplicity = 1; outer_dmu_multiplicity <= highest_multiplicity_primary_uoa; ++outer_dmu_multiplicity)
		{
			for (int inner_dmu_multiplicity = 0; inner_dmu_multiplicity < number_primary_key_columns_in_dmu_category_with_multiplicity_greater_than_1; ++inner_dmu_multiplicity)
			{
				std::for_each(result_columns.columns_in_view.begin(), result_columns.columns_in_view.end(), [this, &first_variable_group, &inner_dmu_multiplicity, &outer_dmu_multiplicity, &sql_create_final_primary_group_table, &first, &is_xrmfxr_table](ColumnsInTempView::ColumnInTempView & view_column)
				{
					if (!view_column.variable_group_associated_with_current_inner_table.IsEqual(WidgetInstanceIdentifier::EQUALITY_CHECK_TYPE__STRING_CODE, first_variable_group))
					{
						return;
					}
					if (view_column.column_type == ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__PRIMARY)
					{
						if (view_column.total_multiplicity__of_current_dmu_category__within_uoa_corresponding_to_the_current_inner_tables_variable_group == highest_multiplicity_primary_uoa)
						{
							if (view_column.primary_key_index__within_uoa_corresponding_to_variable_group_corresponding_to_current_inner_table__for_dmu_category == inner_dmu_multiplicity)
							{
								if (view_column.current_multiplicity__corresponding_to__current_inner_table___is_1_in_all_inner_tables_when_multiplicity_is_1_for_that_vg == outer_dmu_multiplicity)
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
	std::for_each(result_columns.columns_in_view.begin(), result_columns.columns_in_view.end(), [this, &first_variable_group, &sql_create_final_primary_group_table, &result_columns, &current_column, &first, &is_xrmfxr_table](ColumnsInTempView::ColumnInTempView & view_column)
	{
		if (is_xrmfxr_table)
		{
			if (!view_column.variable_group_associated_with_current_inner_table.IsEqual(WidgetInstanceIdentifier::EQUALITY_CHECK_TYPE__STRING_CODE, first_variable_group))
			{
				return;
			}
			if (view_column.current_multiplicity__of__current_inner_table__within__current_vg > 1)
			{
				return;
			}
		}
		else
		{
			if (current_column >= inner_table_no_multiplicities_column_count)
			{
				return;
			}
		}

		// Determine how many columns there are corresponding to the DMU category
		int number_primary_key_columns_in_dmu_category_with_multiplicity_of_1 = 0;
		int column_count_nested = 0;
		std::for_each(result_columns.columns_in_view.begin(), result_columns.columns_in_view.end(), [this, &first_variable_group, &is_xrmfxr_table, &view_column, &column_count_nested, &number_primary_key_columns_in_dmu_category_with_multiplicity_of_1, &sql_create_final_primary_group_table](ColumnsInTempView::ColumnInTempView & view_column_nested)
		{
			if (is_xrmfxr_table)
			{
				if (!view_column_nested.variable_group_associated_with_current_inner_table.IsEqual(WidgetInstanceIdentifier::EQUALITY_CHECK_TYPE__STRING_CODE, first_variable_group))
				{
					return;
				}
				if (view_column_nested.current_multiplicity__of__current_inner_table__within__current_vg > 1)
				{
					return;
				}
			}
			else
			{
				if (column_count_nested >= inner_table_no_multiplicities_column_count)
				{
					return;
				}
			}
			if (view_column_nested.column_type == ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__PRIMARY)
			{
				if (view_column_nested.primary_key_dmu_category_identifier.IsEqual(WidgetInstanceIdentifier::EQUALITY_CHECK_TYPE__STRING_CODE, view_column.primary_key_dmu_category_identifier))
				{
					if (view_column_nested.total_multiplicity__of_current_dmu_category__within_uoa_corresponding_to_the_current_inner_tables_variable_group == 1)
					{
						if (view_column.is_within_inner_table_corresponding_to_top_level_uoa)
						{
							++number_primary_key_columns_in_dmu_category_with_multiplicity_of_1;
						}
					}
				}
			}
			++column_count_nested;
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
	sql_create_final_primary_group_table += ", ";
	sql_create_final_primary_group_table += final_xr_or_xrmfxr_columns.columns_in_view[final_xr_or_xrmfxr_columns.columns_in_view.size()-2].column_name_in_temporary_table; // final merged datetime start column
	sql_create_final_primary_group_table += ", ";
	sql_create_final_primary_group_table += final_xr_or_xrmfxr_columns.columns_in_view[final_xr_or_xrmfxr_columns.columns_in_view.size()-1].column_name_in_temporary_table; // final merged datetime end column

	sql_strings.push_back(SQLExecutor(db, sql_create_final_primary_group_table));

	return result;

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
		boost::format msg("SQLite database error when iterating through rows: %1%");
		msg % sql_error;
		SetFailureMessage(msg.str());
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
		boost::format msg("SQLite database error when preparing SELECT * SQL statement for table %1%: %2% (The SQL query is \"%3%\")");
		msg % column_set.view_name % sql_error % sql;
		msg % sql_error;
		SetFailureMessage(msg.str());
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
			// Error already posted
			SetFailureMessage(sql_executor.error_message);
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
				boost::format msg("Unable to prepare SQL query \"%1%\": %2%");
				msg % sql % sql_error;
				error_message = msg.str();
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
						boost::format msg("Unable to prepare SQL query \"%1%\": %2%");
						msg % sql % sql_error;
						error_message = msg.str();
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
						boost::format msg("Unable to prepare SQL query \"%1%\": %2%");
						msg % sql % sql_error;
						error_message = msg.str();
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
					boost::format msg("Unexpected result when attempting to execute SQL query \"%1%\": %2%");
					msg % sql % sql_error;
					error_message = msg.str();
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
		boost::format msg("Unexpected result (row not returned) when attempting to step through result set of SQL query \"%1%\": %2%");
		msg % sql % sql_error;
		error_message = msg.str();
		failed = true;
		return false;
	}

	return true;

}

OutputModel::OutputGenerator::SqlAndColumnSet OutputModel::OutputGenerator::CreateInitialPrimaryXTable_OrCount(ColumnsInTempView const & primary_variable_group_raw_data_columns, int const primary_group_number, bool const count_only)
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
	// Start with the primary key columns.
	std::for_each(primary_variable_group_raw_data_columns.columns_in_view.cbegin(), primary_variable_group_raw_data_columns.columns_in_view.cend(), [&result_columns, &variables_selected](ColumnsInTempView::ColumnInTempView const & column_in_view)
	{
		if (column_in_view.column_type == ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__DATETIMESTART || column_in_view.column_type == ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__DATETIMEEND)
		{
			return; // Enforce that datetime columns appear last.
		}
		bool match = true;
		if (column_in_view.column_type == ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__SECONDARY)
		{
			return; // Enforce that primary key columns appear first.
		}
		if (match)
		{
			result_columns.columns_in_view.push_back(column_in_view);
		}
	});
	// Proceed to the secondary key columns.
	std::for_each(primary_variable_group_raw_data_columns.columns_in_view.cbegin(), primary_variable_group_raw_data_columns.columns_in_view.cend(), [&result_columns, &variables_selected](ColumnsInTempView::ColumnInTempView const & column_in_view)
	{
		if (column_in_view.column_type == ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__DATETIMESTART || column_in_view.column_type == ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__DATETIMEEND)
		{
			return; // Enforce that datetime columns appear last.
		}
		if (column_in_view.column_type != ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__SECONDARY)
		{
			return; // We are populating secondary columns now, so exit if this isn't one
		}
		bool match = false;
		std::for_each(variables_selected.cbegin(), variables_selected.cend(), [&column_in_view, &match](WidgetInstanceIdentifier const & variable_selected)
		{
			if (boost::iequals(column_in_view.column_name_in_original_data_table, *variable_selected.code))
			{
				match = true;
			}
		});
		if (match)
		{
			result_columns.columns_in_view.push_back(column_in_view);
		}
	});
	// Proceed, finally, to the datetime columns, if they exist.  (If they don't, they will be added via ALTER TABLE to the temporary table under construction.)
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

	if (!count_only)
	{
		sql_string = "CREATE TABLE ";
		sql_string += result_columns.view_name;
		sql_string += " AS ";
	}
	else
	{
		sql_strings.back().statement_type = SQLExecutor::RETURNS_ROWS;
	}
	sql_string += "SELECT ";
	if (count_only)
	{
		sql_string += "COUNT(*)";
	}
	else
	{
		first = true;
		std::for_each(result_columns.columns_in_view.begin(), result_columns.columns_in_view.end(), [&sql_string, &first](ColumnsInTempView::ColumnInTempView & new_column)
		{
			if (!first)
			{
				sql_string += ", ";
			}
			first = false;
			sql_string += new_column.column_name_in_temporary_table_no_uuid; // This is the original column name
			sql_string += " AS ";
			sql_string += new_column.column_name_in_temporary_table;
		});
	}

	sql_string += " FROM ";
	sql_string += result_columns.original_table_names[0];

	if (!primary_variable_group_raw_data_columns.has_no_datetime_columns_originally)
	{
		if (count_only)
		{
			sql_string += " WHERE CASE WHEN ";
			sql_string += result_columns.columns_in_view[result_columns.columns_in_view.size() - 2].column_name_in_temporary_table_no_uuid;
			sql_string += " = 0 AND ";
			sql_string += result_columns.columns_in_view[result_columns.columns_in_view.size() - 1].column_name_in_temporary_table_no_uuid;
			sql_string += " = 0 ";
			sql_string += " THEN 1 ";
			sql_string += " WHEN ";
			sql_string += result_columns.columns_in_view[result_columns.columns_in_view.size() - 2].column_name_in_temporary_table_no_uuid;
			sql_string += " < ";
			sql_string += boost::lexical_cast<std::string>(timerange_end);
			sql_string += " THEN ";
			sql_string += result_columns.columns_in_view[result_columns.columns_in_view.size() - 1].column_name_in_temporary_table_no_uuid;
			sql_string += " > ";
			sql_string += boost::lexical_cast<std::string>(timerange_start);
			sql_string += " WHEN ";
			sql_string += result_columns.columns_in_view[result_columns.columns_in_view.size() - 1].column_name_in_temporary_table_no_uuid;
			sql_string += " > ";
			sql_string += boost::lexical_cast<std::string>(timerange_start);
			sql_string += " THEN ";
			sql_string += result_columns.columns_in_view[result_columns.columns_in_view.size() - 2].column_name_in_temporary_table_no_uuid;
			sql_string += " < ";
			sql_string += boost::lexical_cast<std::string>(timerange_end);
			sql_string += " END";
		}
		else
		{
			sql_string += " WHERE CASE WHEN ";
			sql_string += result_columns.columns_in_view[result_columns.columns_in_view.size() - 2].column_name_in_temporary_table;
			sql_string += " = 0 AND ";
			sql_string += result_columns.columns_in_view[result_columns.columns_in_view.size() - 1].column_name_in_temporary_table;
			sql_string += " = 0 ";
			sql_string += " THEN 1 ";
			sql_string += " WHEN ";
			sql_string += result_columns.columns_in_view[result_columns.columns_in_view.size() - 2].column_name_in_temporary_table;
			sql_string += " < ";
			sql_string += boost::lexical_cast<std::string>(timerange_end);
			sql_string += " THEN ";
			sql_string += result_columns.columns_in_view[result_columns.columns_in_view.size() - 1].column_name_in_temporary_table;
			sql_string += " > ";
			sql_string += boost::lexical_cast<std::string>(timerange_start);
			sql_string += " WHEN ";
			sql_string += result_columns.columns_in_view[result_columns.columns_in_view.size() - 1].column_name_in_temporary_table;
			sql_string += " > ";
			sql_string += boost::lexical_cast<std::string>(timerange_start);
			sql_string += " THEN ";
			sql_string += result_columns.columns_in_view[result_columns.columns_in_view.size() - 2].column_name_in_temporary_table;
			sql_string += " < ";
			sql_string += boost::lexical_cast<std::string>(timerange_end);
			sql_string += " END";
		}
	}

	// Add the ORDER BY column/s
	if (!count_only && debug_ordering)
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

	if (!count_only)
	{
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

OutputModel::OutputGenerator::SqlAndColumnSet OutputModel::OutputGenerator::CreateInitialPrimaryMergeXRTable(ColumnsInTempView const & first_final_primary_variable_group_columns)
{

	SqlAndColumnSet result = std::make_pair(std::vector<SQLExecutor>(), ColumnsInTempView());
	std::vector<SQLExecutor> & sql_strings = result.first;
	ColumnsInTempView & result_columns = result.second;

	result_columns = first_final_primary_variable_group_columns;

	std::string view_name = "MF0";
	view_name += "_xr";
	view_name += "1";
	result_columns.view_name_no_uuid = view_name;
	view_name += "_";
	view_name += newUUID(true);
	result_columns.view_name = view_name;

	WidgetInstanceIdentifier variable_group;
	WidgetInstanceIdentifier uoa;

	bool first = true;
	int column_index = 0;
	std::for_each(result_columns.columns_in_view.begin(), result_columns.columns_in_view.end(), [&variable_group, &uoa, &column_index, &first](ColumnsInTempView::ColumnInTempView & new_column)
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
		++column_index;
	});

	sql_strings.push_back(SQLExecutor(db));
	std::string & sql_string = sql_strings.back().sql;

	sql_string = "CREATE TABLE ";
	sql_string += result_columns.view_name;
	sql_string += " AS SELECT ";
	first = true;
	int the_index = 0;
	std::for_each(result_columns.columns_in_view.begin(), result_columns.columns_in_view.end(), [&sql_string, &the_index, &first_final_primary_variable_group_columns, &first](ColumnsInTempView::ColumnInTempView & new_column)
	{
		if (!first)
		{
			sql_string += ", ";
		}
		first = false;
		sql_string += first_final_primary_variable_group_columns.columns_in_view[the_index].column_name_in_temporary_table; // This is the original column name
		sql_string += " AS ";
		sql_string += new_column.column_name_in_temporary_table;
		++the_index;
	});
	sql_string += " FROM ";
	sql_string += first_final_primary_variable_group_columns.view_name;


	// Add the "merged" time range columns

	std::string datetime_start_col_name_no_uuid = "DATETIME_ROW_START_MERGED_BETWEEN_FINALS";
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
	datetime_start_column.column_type = ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__DATETIMESTART_MERGED_BETWEEN_FINALS;
	datetime_start_column.variable_group_associated_with_current_inner_table = variable_group;
	datetime_start_column.uoa_associated_with_variable_group_associated_with_current_inner_table = uoa;
	datetime_start_column.column_name_in_original_data_table = "";
	datetime_start_column.inner_table_set_number = 0;
	datetime_start_column.is_within_inner_table_corresponding_to_top_level_uoa = false;

	std::string datetime_end_col_name_no_uuid = "DATETIME_ROW_END_MERGED_BETWEEN_FINALS";
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
	datetime_end_column.column_type = ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__DATETIMEEND_MERGED_BETWEEN_FINALS;
	datetime_end_column.variable_group_associated_with_current_inner_table = variable_group;
	datetime_end_column.uoa_associated_with_variable_group_associated_with_current_inner_table = uoa;
	datetime_end_column.column_name_in_original_data_table = "";
	datetime_end_column.inner_table_set_number = 0;
	datetime_end_column.is_within_inner_table_corresponding_to_top_level_uoa = false;


	// Set the "merged" time range columns to be equal to the original time range columns
	std::string sql_time_range;
	sql_time_range += "UPDATE OR FAIL ";
	sql_time_range += result_columns.view_name;
	sql_time_range += " SET ";
	sql_time_range += datetime_start_col_name;
	sql_time_range += " = ";
	sql_time_range += result_columns.columns_in_view[result_columns.columns_in_view.size()-4].column_name_in_temporary_table;
	sql_time_range += ", ";
	sql_time_range += datetime_end_col_name;
	sql_time_range += " = ";
	sql_time_range += result_columns.columns_in_view[result_columns.columns_in_view.size()-3].column_name_in_temporary_table;
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
	inner_table_no_multiplicities_column_count = 0;
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
	std::for_each(result_columns.columns_in_view.begin(), result_columns.columns_in_view.end(), [this, &first_full_table_column_count, &reached_first_datetime_start_merged_column, &reached_first_datetime_end_merged_column, &in_first_inner_table, &previous_column_names_first_table, &variable_group, &uoa, &first](ColumnsInTempView::ColumnInTempView & new_column)
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
			++inner_table_no_multiplicities_column_count;
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
	//
	// Start with the primary key columns.
	std::for_each(primary_variable_group_raw_data_columns.columns_in_view.cbegin(), primary_variable_group_raw_data_columns.columns_in_view.cend(), [&result_columns, &variables_selected, &second_table_column_count, &current_multiplicity](ColumnsInTempView::ColumnInTempView const & new_column_raw_data_table)
	{
		if (new_column_raw_data_table.column_type == ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__DATETIMESTART || new_column_raw_data_table.column_type == ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__DATETIMESTART_INTERNAL || new_column_raw_data_table.column_type == ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__DATETIMEEND || new_column_raw_data_table.column_type == ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__DATETIMEEND_INTERNAL)
		{
			return; // Add these columns last
		}

		bool match = true;

		if (new_column_raw_data_table.column_type == ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__SECONDARY)
		{
			return; // Enforce that primary key columns appear first.
		}

		if (match)
		{
			result_columns.columns_in_view.push_back(new_column_raw_data_table);
			ColumnsInTempView::ColumnInTempView & new_column = result_columns.columns_in_view.back();
			new_column.column_name_in_temporary_table = new_column.column_name_in_temporary_table_no_uuid;
			new_column.column_name_in_temporary_table += "_";
			new_column.column_name_in_temporary_table += newUUID(true);
			new_column.inner_table_set_number = 0;
			new_column.is_within_inner_table_corresponding_to_top_level_uoa = true;
			new_column.current_multiplicity__of__current_inner_table__within__current_vg = current_multiplicity;
			if (new_column.column_type == ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__PRIMARY)
			{
				if (new_column.total_multiplicity__of_current_dmu_category__within_uoa_corresponding_to_the_current_inner_tables_variable_group > 1)
				{
					new_column.current_multiplicity__corresponding_to__current_inner_table___is_1_in_all_inner_tables_when_multiplicity_is_1_for_that_vg = current_multiplicity; // update current multiplicity
					new_column.primary_key_index_within_total_kad_for_dmu_category = new_column.primary_key_index__within_uoa_corresponding_to_variable_group_corresponding_to_current_inner_table__for_dmu_category
						+ (current_multiplicity - 1) * new_column.total_k_count__within_uoa_corresponding_to_current_variable_group__for_current_dmu_category;
				}
			}
			++second_table_column_count;
		}
	});
	// Proceed to the secondary key columns.
	std::for_each(primary_variable_group_raw_data_columns.columns_in_view.cbegin(), primary_variable_group_raw_data_columns.columns_in_view.cend(), [&result_columns, &variables_selected, &second_table_column_count, &current_multiplicity](ColumnsInTempView::ColumnInTempView const & new_column_secondary)
	{
		if (new_column_secondary.column_type == ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__DATETIMESTART || new_column_secondary.column_type == ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__DATETIMESTART_INTERNAL || new_column_secondary.column_type == ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__DATETIMEEND || new_column_secondary.column_type == ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__DATETIMEEND_INTERNAL)
		{
			return; // Add these columns last
		}

		if (new_column_secondary.column_type != ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__SECONDARY)
		{
			return; // We are populating secondary columns now, so exit if this isn't one
		}

		bool match = false;
		std::for_each(variables_selected.cbegin(), variables_selected.cend(), [&new_column_secondary, &match](WidgetInstanceIdentifier const & variable_selected)
		{
			if (boost::iequals(new_column_secondary.column_name_in_original_data_table, *variable_selected.code))
			{
				match = true;
			}
		});

		if (match)
		{
			result_columns.columns_in_view.push_back(new_column_secondary);
			ColumnsInTempView::ColumnInTempView & new_column = result_columns.columns_in_view.back();
			new_column.column_name_in_temporary_table = new_column.column_name_in_temporary_table_no_uuid;
			new_column.column_name_in_temporary_table += "_";
			new_column.column_name_in_temporary_table += newUUID(true);
			new_column.inner_table_set_number = 0;
			new_column.is_within_inner_table_corresponding_to_top_level_uoa = true;
			new_column.current_multiplicity__of__current_inner_table__within__current_vg = current_multiplicity;
			if (new_column.column_type == ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__PRIMARY)
			{
				if (new_column.total_multiplicity__of_current_dmu_category__within_uoa_corresponding_to_the_current_inner_tables_variable_group > 1)
				{
					new_column.current_multiplicity__corresponding_to__current_inner_table___is_1_in_all_inner_tables_when_multiplicity_is_1_for_that_vg = current_multiplicity; // update current multiplicity
					new_column.primary_key_index_within_total_kad_for_dmu_category = new_column.primary_key_index__within_uoa_corresponding_to_variable_group_corresponding_to_current_inner_table__for_dmu_category
						+ (current_multiplicity - 1) * new_column.total_k_count__within_uoa_corresponding_to_current_variable_group__for_current_dmu_category;
				}
			}
			++second_table_column_count;
		}
	});
	// Proceed, finally, to the datetime columns, if they exist.  (If they don't, they will be added via ALTER TABLE to the temporary table under construction.)
	std::for_each(primary_variable_group_raw_data_columns.columns_in_view.cbegin(), primary_variable_group_raw_data_columns.columns_in_view.cend(), [&result_columns, &second_table_column_count](ColumnsInTempView::ColumnInTempView const & new_column_datetime)
	{
		if (new_column_datetime.column_type == ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__DATETIMESTART || new_column_datetime.column_type == ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__DATETIMESTART_INTERNAL)
		{
			result_columns.columns_in_view.push_back(new_column_datetime);
			ColumnsInTempView::ColumnInTempView & new_column = result_columns.columns_in_view.back();
			new_column.column_name_in_temporary_table = new_column.column_name_in_temporary_table_no_uuid;
			new_column.column_name_in_temporary_table += "_";
			new_column.column_name_in_temporary_table += newUUID(true);
			new_column.inner_table_set_number = 0;
			new_column.is_within_inner_table_corresponding_to_top_level_uoa = true;
			++second_table_column_count;
		}
	});
	std::for_each(primary_variable_group_raw_data_columns.columns_in_view.cbegin(), primary_variable_group_raw_data_columns.columns_in_view.cend(), [&result_columns, &second_table_column_count](ColumnsInTempView::ColumnInTempView const & new_column_datetime)
	{
		if (new_column_datetime.column_type == ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__DATETIMEEND || new_column_datetime.column_type == ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__DATETIMEEND_INTERNAL)
		{
			result_columns.columns_in_view.push_back(new_column_datetime);
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
	std::for_each(sequence.primary_key_sequence_info.cbegin(), sequence.primary_key_sequence_info.cend(), [this, &sql_string, &variable_group, &result_columns, &first_full_table_column_count, &second_table_column_count, &previous_column_names_first_table, &and_](PrimaryKeySequence::PrimaryKeySequenceEntry const & primary_key)
	{
		std::for_each(primary_key.variable_group_info_for_primary_keys.cbegin(), primary_key.variable_group_info_for_primary_keys.cend(), [this, &sql_string, &variable_group, &primary_key, &result_columns, &first_full_table_column_count, &second_table_column_count, &previous_column_names_first_table, &and_](PrimaryKeySequence::VariableGroup_PrimaryKey_Info const & primary_key_info)
		{
			if (primary_key_info.vg_identifier.IsEqual(WidgetInstanceIdentifier::EQUALITY_CHECK_TYPE__STRING_CODE, variable_group))
			{
				if (primary_key_info.total_multiplicity == 1)
				{
					// Only join on primary keys whose total multiplicity is 1
					int column_count = 0;
					std::for_each(result_columns.columns_in_view.cbegin(), result_columns.columns_in_view.cend(), [this, &sql_string, &first_full_table_column_count, &second_table_column_count, &column_count, &previous_column_names_first_table, &primary_key, &and_](ColumnsInTempView::ColumnInTempView const & new_column)
					{
						if (column_count < inner_table_no_multiplicities_column_count)
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
							if (view_column.current_multiplicity__corresponding_to__current_inner_table___is_1_in_all_inner_tables_when_multiplicity_is_1_for_that_vg == 1)
							{
								++number_primary_key_columns_in_dmu_category_with_multiplicity_greater_than_1;
							}
						}
					}
				}
			});
		}
	}

	if (!primary_variable_group_raw_data_columns.has_no_datetime_columns_originally)
	{
		sql_string += " WHERE CASE WHEN ";
		sql_string += result_columns.columns_in_view[result_columns.columns_in_view.size() - 2].column_name_in_temporary_table;
		sql_string += " = 0 AND ";
		sql_string += result_columns.columns_in_view[result_columns.columns_in_view.size() - 1].column_name_in_temporary_table;
		sql_string += " = 0 ";
		sql_string += " THEN 1 ";
		sql_string += " WHEN ";
		sql_string += result_columns.columns_in_view[result_columns.columns_in_view.size() - 2].column_name_in_temporary_table;
		sql_string += " < ";
		sql_string += boost::lexical_cast<std::string>(timerange_end);
		sql_string += " THEN ";
		sql_string += result_columns.columns_in_view[result_columns.columns_in_view.size() - 1].column_name_in_temporary_table;
		sql_string += " > ";
		sql_string += boost::lexical_cast<std::string>(timerange_start);
		sql_string += " WHEN ";
		sql_string += result_columns.columns_in_view[result_columns.columns_in_view.size() - 1].column_name_in_temporary_table;
		sql_string += " > ";
		sql_string += boost::lexical_cast<std::string>(timerange_start);
		sql_string += " THEN ";
		sql_string += result_columns.columns_in_view[result_columns.columns_in_view.size() - 2].column_name_in_temporary_table;
		sql_string += " < ";
		sql_string += boost::lexical_cast<std::string>(timerange_end);
		sql_string += " END";
	}

	// Add the ORDER BY column/s
	if (debug_ordering)
	{

		bool first = true;

		if (highest_multiplicity_primary_uoa > 1)
		{

			// Create the ORDER BY clause, taking the proper primary key columns that compose the DMU category with multiplicity greater than 1, in sequence
			for (int outer_dmu_multiplicity = 1; outer_dmu_multiplicity <= highest_multiplicity_primary_uoa; ++outer_dmu_multiplicity)
			{
				for (int inner_dmu_multiplicity = 0; inner_dmu_multiplicity < number_primary_key_columns_in_dmu_category_with_multiplicity_greater_than_1; ++inner_dmu_multiplicity)
				{
					std::for_each(result_columns.columns_in_view.begin(), result_columns.columns_in_view.end(), [this, &inner_dmu_multiplicity, &outer_dmu_multiplicity, &sql_string, &first](ColumnsInTempView::ColumnInTempView & view_column)
					{
						if (view_column.column_type == ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__PRIMARY)
						{
							if (view_column.total_multiplicity__of_current_dmu_category__within_uoa_corresponding_to_the_current_inner_tables_variable_group == highest_multiplicity_primary_uoa)
							{
								if (view_column.primary_key_index__within_uoa_corresponding_to_variable_group_corresponding_to_current_inner_table__for_dmu_category == inner_dmu_multiplicity)
								{
									if (view_column.current_multiplicity__corresponding_to__current_inner_table___is_1_in_all_inner_tables_when_multiplicity_is_1_for_that_vg == outer_dmu_multiplicity)
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
		// ... If there are no primary key DMU categories for this top-level UOA with multiplicity greater than 1,
		// then this section will order by all of this top-level's UOA primary key DMU categories.
		int current_column = 0;
		std::for_each(result_columns.columns_in_view.begin(), result_columns.columns_in_view.end(), [this, &sql_string, &result_columns, &current_column, &first](ColumnsInTempView::ColumnInTempView & view_column)
		{

			if (current_column >= inner_table_no_multiplicities_column_count)
			{
				return;
			}

			// Determine how many columns there are corresponding to the DMU category
			int number_primary_key_columns_in_dmu_category_with_multiplicity_of_1 = 0;
			int column_count_nested = 0;
			std::for_each(result_columns.columns_in_view.begin(), result_columns.columns_in_view.end(), [this, &view_column, &column_count_nested, &number_primary_key_columns_in_dmu_category_with_multiplicity_of_1, &sql_string](ColumnsInTempView::ColumnInTempView & view_column_nested)
			{
				if (column_count_nested >= inner_table_no_multiplicities_column_count)
				{
					return;
				}
				if (view_column_nested.column_type == ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__PRIMARY)
				{
					if (view_column_nested.primary_key_dmu_category_identifier.IsEqual(WidgetInstanceIdentifier::EQUALITY_CHECK_TYPE__STRING_CODE, view_column.primary_key_dmu_category_identifier))
					{
						if (view_column_nested.total_multiplicity__of_current_dmu_category__within_uoa_corresponding_to_the_current_inner_tables_variable_group == 1)
						{
							if (view_column.is_within_inner_table_corresponding_to_top_level_uoa)
							{
								++number_primary_key_columns_in_dmu_category_with_multiplicity_of_1;
							}
						}
					}
				}
				++column_count_nested;
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

bool OutputModel::OutputGenerator::CreateNewXRRow(bool & first_row_added, std::string const & datetime_start_col_name, std::string const & datetime_end_col_name, std::string const & xr_view_name, std::string & sql_add_xr_row, std::vector<std::string> & bound_parameter_strings, std::vector<std::int64_t> & bound_parameter_ints, std::vector<SQLExecutor::WHICH_BINDING> & bound_parameter_which_binding_to_use, std::int64_t const datetime_start, std::int64_t const datetime_end, ColumnsInTempView & previous_x_or_mergedfinalplusnewfinal_columns, ColumnsInTempView & current_xr_or_completemerge_columns, bool const include_previous_data, bool const include_current_data, XR_TABLE_CATEGORY const xr_table_category)
{

	if (include_previous_data == false && include_current_data == false)
	{
		return false;
	}

	bool do_not_check_time_range = false;
	if (datetime_start == 0 && datetime_end == 0)
	{
		do_not_check_time_range = true;
	}
	if (!do_not_check_time_range)
	{
		if (datetime_start >= timerange_end)
		{
			return false;
		}
		if (datetime_end <= timerange_start)
		{
			return false;
		}
	}

	if (first_row_added)
	{
		
		// Create SQL statement here, including placeholders for bound parameters

		sql_add_xr_row.clear();

		sql_add_xr_row += "INSERT OR FAIL INTO ";
		sql_add_xr_row += xr_view_name;
		sql_add_xr_row += "(";

		bool first_column_name = true;
		int the_index = 0;
		std::for_each(previous_x_or_mergedfinalplusnewfinal_columns.columns_in_view.cbegin(), previous_x_or_mergedfinalplusnewfinal_columns.columns_in_view.cend(), [&the_index, &current_xr_or_completemerge_columns, &first_column_name, &sql_add_xr_row, &bound_parameter_strings, &bound_parameter_ints, &bound_parameter_which_binding_to_use, &include_previous_data, &include_current_data](ColumnsInTempView::ColumnInTempView const & column_in_view)
		{
			
			if (!first_column_name)
			{
				sql_add_xr_row += ", ";
			}
			first_column_name = false;

			sql_add_xr_row += current_xr_or_completemerge_columns.columns_in_view[the_index].column_name_in_temporary_table;

			++the_index;

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
		std::for_each(previous_x_or_mergedfinalplusnewfinal_columns.columns_in_view.cbegin(), previous_x_or_mergedfinalplusnewfinal_columns.columns_in_view.cend(), [&first_column_value, &index, &cindex, &sql_add_xr_row, &bound_parameter_strings, &bound_parameter_ints, &bound_parameter_which_binding_to_use, &include_previous_data, &include_current_data](ColumnsInTempView::ColumnInTempView const & column_in_view)
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
		return false;
	}

	bool new_method = true;

	int highest_index_previous_table = (int)previous_x_or_mergedfinalplusnewfinal_columns.columns_in_view.size() - 1;
	bool found_highest_index = false;
	std::for_each(previous_x_or_mergedfinalplusnewfinal_columns.columns_in_view.crbegin(), previous_x_or_mergedfinalplusnewfinal_columns.columns_in_view.crend(), [&xr_table_category, &new_method, &highest_index_previous_table, &found_highest_index](ColumnsInTempView::ColumnInTempView const & column_in_view)
	{
		if (found_highest_index)
		{
			return;
		}

		// The following "if" condition handles primary groups, child groups, and final results from top-level primary groups
		if (new_method)
		{
			if (xr_table_category == OutputModel::OutputGenerator::CHILD_VARIABLE_GROUP)
			{
				if (column_in_view.column_type == ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__DATETIMEEND_MERGED || column_in_view.column_type == ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__DATETIMEEND_MERGED_BETWEEN_FINALS || column_in_view.column_type == ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__DATETIMEEND_CHILD_MERGE)
				{
					found_highest_index = true;
					return;
				}
			}
			else if (xr_table_category == OutputModel::OutputGenerator::FINAL_MERGE_OF_PRIMARY_VARIABLE_GROUP)
			{
				if (column_in_view.column_type == ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__DATETIMEEND_MERGED_BETWEEN_FINALS)
				{
					found_highest_index = true;
					return;
				}
			}
			else
			{
				if (column_in_view.column_type == ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__DATETIMEEND_MERGED || column_in_view.column_type == ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__DATETIMEEND_MERGED_BETWEEN_FINALS)
				{
					found_highest_index = true;
					return;
				}
			}
		}
		else
		{
			if (column_in_view.column_type == ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__DATETIMEEND_MERGED || column_in_view.column_type == ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__DATETIMEEND_MERGED_BETWEEN_FINALS)
			{
				found_highest_index = true;
				return;
			}
		}

		--highest_index_previous_table;
	});


	// The following block only applies to PRIMARY_VARIABLE_GROUP merges
	int number_columns_each_single_inner_table = 0;
	WidgetInstanceIdentifier first_variable_group;
	int the_column_index = 0;
	int first_inner_table_datetime_columns_count = 0;
	std::for_each(previous_x_or_mergedfinalplusnewfinal_columns.columns_in_view.cbegin(), previous_x_or_mergedfinalplusnewfinal_columns.columns_in_view.cend(), [&first_inner_table_datetime_columns_count, &the_column_index, &number_columns_each_single_inner_table, &first_variable_group, &highest_index_previous_table, &found_highest_index](ColumnsInTempView::ColumnInTempView const & column_in_view)
	{
		if (the_column_index == 0)
		{
			first_variable_group = column_in_view.variable_group_associated_with_current_inner_table;
		}
		if (!column_in_view.variable_group_associated_with_current_inner_table.IsEqual(WidgetInstanceIdentifier::EQUALITY_CHECK_TYPE__STRING_CODE, first_variable_group))
		{
			++the_column_index;
			return;
		}
		if (first_inner_table_datetime_columns_count < 4)
		{
			++number_columns_each_single_inner_table;
		}
		if (column_in_view.column_type == ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__DATETIMESTART || column_in_view.column_type == ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__DATETIMEEND || column_in_view.column_type == ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__DATETIMESTART_INTERNAL || column_in_view.column_type == ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__DATETIMEEND_INTERNAL || column_in_view.column_type == ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__DATETIMESTART_MERGED || column_in_view.column_type == ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__DATETIMEEND_MERGED)
		{
			++first_inner_table_datetime_columns_count;
		}
		++the_column_index;
	});


	// The following block only applies to PRIMARY_VARIABLE_GROUP merges
	bool swap_current_and_previous_and_set_previous_to_null = false;
	if (xr_table_category == OutputModel::OutputGenerator::PRIMARY_VARIABLE_GROUP)
	{
		if (include_current_data && !include_previous_data)
		{
			// place current data at beginning, since there is no previous data.
			// Because this is a primary variable group, the schema of all
			// inner tables is the same.
			swap_current_and_previous_and_set_previous_to_null = true;
		}
	}


	// Set the list of bound parameters, regardless of whether or not the SQL string was created
	int index = 0;
	char cindex[256];
	bool first_column_value = true;
	std::int64_t data_int64 = 0;
	std::string data_string;
	long double data_long = 0.0;
	bool do_not_include_this_data = false;
	int column_data_type = 0;
	bound_parameter_strings.clear();
	bound_parameter_ints.clear();
	bound_parameter_which_binding_to_use.clear();
	int number_nulls_to_add_at_end = 0;

	std::vector<ColumnSorter> inner_table_columns;

	int which_inner_table = 0;

	std::for_each(previous_x_or_mergedfinalplusnewfinal_columns.columns_in_view.cbegin(), previous_x_or_mergedfinalplusnewfinal_columns.columns_in_view.cend(), [this, &which_inner_table, &inner_table_columns, &swap_current_and_previous_and_set_previous_to_null, &number_nulls_to_add_at_end, &xr_table_category, &number_columns_each_single_inner_table, &highest_index_previous_table, &data_int64, &data_string, &data_long, &do_not_include_this_data, &column_data_type, &first_column_value, &index, &cindex, &sql_add_xr_row, &bound_parameter_strings, &bound_parameter_ints, &bound_parameter_which_binding_to_use, &include_previous_data, &include_current_data](ColumnsInTempView::ColumnInTempView const & column_in_view)
	{

		if (failed)
		{
			return;
		}

		// The following block only applies to PRIMARY_VARIABLE_GROUP merges
		if (index % number_columns_each_single_inner_table == 0)
		{
			if (index > 0)
			{
				++which_inner_table;
			}
			inner_table_columns.push_back(ColumnSorter());
		}


		// The following variables only apply to PRIMARY_VARIABLE_GROUP merges
		std::vector<std::string> & inner_table_string_set = inner_table_columns.back().strings;
		std::vector<std::int64_t> & inner_table_int_set = inner_table_columns.back().ints;
		std::vector<std::pair<OutputModel::OutputGenerator::SQLExecutor::WHICH_BINDING, int>> & inner_table_bindings = inner_table_columns.back().bindings;
		std::vector<std::pair<OutputModel::OutputGenerator::SQLExecutor::WHICH_BINDING, int>> & inner_table__primary_keys_with_multiplicity_greater_than_one__which_binding_to_use__set = inner_table_columns.back().bindings__primary_keys_with_multiplicity_greater_than_1;


		do_not_include_this_data = false;


		// The following "if" condition will only match for PRIMARY_VARIABLE_GROUP merges;
		// otherwise, the "else" condition will be active
		if (swap_current_and_previous_and_set_previous_to_null)
		{

			// Only applies to PRIMARY_VARIABLE_GROUP merges
			// In this scenario, only the new data appears on the row, so no sorting is involved.
			if (index >= number_columns_each_single_inner_table)
			{
				++number_nulls_to_add_at_end;
			}

		}

		else
		{

			// ... just populate the single data structures that hold all data across all inner tables
			// ... (including possible null data for the newly added columns)
			if (index <= highest_index_previous_table)
			{
				// No need to worry about inner_table stuff,
				// because that only applies when both current and previous data is being populated
				if (!include_previous_data)
				{
					if (xr_table_category == OutputModel::OutputGenerator::FINAL_MERGE_OF_PRIMARY_VARIABLE_GROUP)
					{
						if (column_in_view.column_type != ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__PRIMARY)
						{
							do_not_include_this_data = true;
							bound_parameter_which_binding_to_use.push_back(SQLExecutor::NULL_BINDING);
						}
					}
					else
					{
						do_not_include_this_data = true;
						bound_parameter_which_binding_to_use.push_back(SQLExecutor::NULL_BINDING);
					}
				}
			}
			else
			{
				// No need to worry about inner_table stuff,
				// because that only applies when both current and previous data is being populated
				if (!include_current_data)
				{
					if (xr_table_category == OutputModel::OutputGenerator::FINAL_MERGE_OF_PRIMARY_VARIABLE_GROUP)
					{
						if (column_in_view.column_type != ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__PRIMARY)
						{
							do_not_include_this_data = true;
							bound_parameter_which_binding_to_use.push_back(SQLExecutor::NULL_BINDING);
						}
					}
					else
					{
						do_not_include_this_data = true;
						bound_parameter_which_binding_to_use.push_back(SQLExecutor::NULL_BINDING);
					}
				}
			}


			// Ordering across multiple inner tables only occurs using the data in the following block,
			// and only applies for PRIMARY_VARIABLE_GROUP merges

			// The following if/else block only applies to PRIMARY_VARIABLE_GROUP merges
			// Now, also save the data into sets broken down by inner table
			if (index <= highest_index_previous_table)
			{
				if (!include_previous_data)
				{
					if (xr_table_category == OutputModel::OutputGenerator::FINAL_MERGE_OF_PRIMARY_VARIABLE_GROUP)
					{
						if (column_in_view.column_type != ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__PRIMARY)
						{
							do_not_include_this_data = true;

							// unused... confirm that this can be removed
							inner_table_bindings.push_back(std::make_pair(OutputModel::OutputGenerator::SQLExecutor::NULL_BINDING, 0));
							if (column_in_view.total_multiplicity__of_current_dmu_category__within_uoa_corresponding_to_the_current_inner_tables_variable_group > 1)
							{
								inner_table__primary_keys_with_multiplicity_greater_than_one__which_binding_to_use__set.push_back(std::make_pair(OutputModel::OutputGenerator::SQLExecutor::NULL_BINDING, 0));
							}
						}
					}
					else
					{
						do_not_include_this_data = true;

						// unused... confirm that this can be removed
						inner_table_bindings.push_back(std::make_pair(OutputModel::OutputGenerator::SQLExecutor::NULL_BINDING, 0));
						if (column_in_view.total_multiplicity__of_current_dmu_category__within_uoa_corresponding_to_the_current_inner_tables_variable_group > 1)
						{
							inner_table__primary_keys_with_multiplicity_greater_than_one__which_binding_to_use__set.push_back(std::make_pair(OutputModel::OutputGenerator::SQLExecutor::NULL_BINDING, 0));
						}
					}
				}
			}
			else
			{
				if (!include_current_data)
				{
					if (xr_table_category == OutputModel::OutputGenerator::FINAL_MERGE_OF_PRIMARY_VARIABLE_GROUP)
					{
						if (column_in_view.column_type != ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__PRIMARY)
						{
							do_not_include_this_data = true;

							// unused... confirm that this can be removed
							inner_table_bindings.push_back(std::make_pair(OutputModel::OutputGenerator::SQLExecutor::NULL_BINDING, 0));
							if (column_in_view.total_multiplicity__of_current_dmu_category__within_uoa_corresponding_to_the_current_inner_tables_variable_group > 1)
							{
								inner_table__primary_keys_with_multiplicity_greater_than_one__which_binding_to_use__set.push_back(std::make_pair(OutputModel::OutputGenerator::SQLExecutor::NULL_BINDING, 0));
							}
						}
					}
					else
					{
						do_not_include_this_data = true;

						// unused... confirm that this can be removed
						inner_table_bindings.push_back(std::make_pair(OutputModel::OutputGenerator::SQLExecutor::NULL_BINDING, 0));
						if (column_in_view.total_multiplicity__of_current_dmu_category__within_uoa_corresponding_to_the_current_inner_tables_variable_group > 1)
						{
							inner_table__primary_keys_with_multiplicity_greater_than_one__which_binding_to_use__set.push_back(std::make_pair(OutputModel::OutputGenerator::SQLExecutor::NULL_BINDING, 0));
						}
					}
				}
			}


		}

		if (!do_not_include_this_data)
		{

			column_data_type = sqlite3_column_type(stmt_result, index);
			switch (column_data_type)
			{

				case SQLITE_INTEGER:
					{

						data_int64 = sqlite3_column_int64(stmt_result, index);

						// ... just populate the single data structures that hold all data across all inner tables
						// ... (including possible null data for the newly added columns)
						if (!swap_current_and_previous_and_set_previous_to_null)
						{
							bound_parameter_ints.push_back(data_int64);
							bound_parameter_which_binding_to_use.push_back(SQLExecutor::INT64);
						}
						else
						{
							if (index > highest_index_previous_table)
							{
								bound_parameter_ints.push_back(data_int64);
								bound_parameter_which_binding_to_use.push_back(SQLExecutor::INT64);
							}
						}

						// The following if/else block only applies to PRIMARY_VARIABLE_GROUP merges
						// Now, also save the data into sets broken down by inner table
						if (!swap_current_and_previous_and_set_previous_to_null)
						{
							inner_table_int_set.push_back(data_int64);
							inner_table_bindings.push_back(std::make_pair(OutputModel::OutputGenerator::SQLExecutor::INT64, (int)inner_table_int_set.size() - 1));
							if (column_in_view.total_multiplicity__of_current_dmu_category__within_uoa_corresponding_to_the_current_inner_tables_variable_group > 1)
							{
								inner_table__primary_keys_with_multiplicity_greater_than_one__which_binding_to_use__set.push_back(std::make_pair(OutputModel::OutputGenerator::SQLExecutor::INT64, (int)inner_table_int_set.size() - 1));
							}
						}
						else
						{
							if (index > highest_index_previous_table)
							{
								// unused... confirm that this can be removed
								inner_table_int_set.push_back(data_int64);
								inner_table_bindings.push_back(std::make_pair(OutputModel::OutputGenerator::SQLExecutor::INT64, (int)inner_table_int_set.size() - 1));
								if (column_in_view.total_multiplicity__of_current_dmu_category__within_uoa_corresponding_to_the_current_inner_tables_variable_group > 1)
								{
									inner_table__primary_keys_with_multiplicity_greater_than_one__which_binding_to_use__set.push_back(std::make_pair(OutputModel::OutputGenerator::SQLExecutor::INT64, (int)inner_table_int_set.size() - 1));
								}
							}
						}

					}
					break;

				case SQLITE_FLOAT:
					{
						// Currently not implemented!!!!!!!  Just add new bound_paramenter_longs as argument to this function, and as member of SQLExecutor just like the other bound_parameter data members, to implement.
						data_long = sqlite3_column_double(stmt_result, index);
						// Todo: Error message
						boost::format msg("Floating point values are not yet supported.  (Error while creating new timerange-managed row.)");
						SetFailureMessage(msg.str());
						failed = true;
						return; // from lambda
					}
					break;

				case SQLITE_TEXT:
					{

						data_string = reinterpret_cast<char const *>(sqlite3_column_text(stmt_result, index));

						// ... just populate the single data structures that hold all data across all inner tables
						// ... (including possible null data for the newly added columns)
						if (!swap_current_and_previous_and_set_previous_to_null)
						{
							bound_parameter_strings.push_back(data_string);
							bound_parameter_which_binding_to_use.push_back(SQLExecutor::STRING);
						}
						else
						{
							if (index > highest_index_previous_table)
							{
								bound_parameter_strings.push_back(data_string);
								bound_parameter_which_binding_to_use.push_back(SQLExecutor::STRING);
							}
						}

						// The following if/else block only applies to PRIMARY_VARIABLE_GROUP merges
						// Now, also save the data into sets broken down by inner table
						if (!swap_current_and_previous_and_set_previous_to_null)
						{
							inner_table_string_set.push_back(data_string);
							inner_table_bindings.push_back(std::make_pair(OutputModel::OutputGenerator::SQLExecutor::STRING, (int)inner_table_string_set.size() - 1));
							if (column_in_view.total_multiplicity__of_current_dmu_category__within_uoa_corresponding_to_the_current_inner_tables_variable_group > 1)
							{
								inner_table__primary_keys_with_multiplicity_greater_than_one__which_binding_to_use__set.push_back(std::make_pair(OutputModel::OutputGenerator::SQLExecutor::STRING, (int)inner_table_string_set.size() - 1));
							}
						}
						else
						{
							if (index > highest_index_previous_table)
							{
								// unused... confirm that this can be removed
								inner_table_string_set.push_back(data_string);
								inner_table_bindings.push_back(std::make_pair(OutputModel::OutputGenerator::SQLExecutor::STRING, (int)inner_table_string_set.size() - 1));
								if (column_in_view.total_multiplicity__of_current_dmu_category__within_uoa_corresponding_to_the_current_inner_tables_variable_group > 1)
								{
									inner_table__primary_keys_with_multiplicity_greater_than_one__which_binding_to_use__set.push_back(std::make_pair(OutputModel::OutputGenerator::SQLExecutor::STRING, (int)inner_table_string_set.size() - 1));
								}
							}
						}

					}
					break;

				case SQLITE_BLOB:
					{
						// Todo: Error message
						boost::format msg("BLOBs are not supported.  (Error while creating new timerange-managed row.)");
						SetFailureMessage(msg.str());
						failed = true;
						return; // from lambda
					}
					break;

				case SQLITE_NULL:
					{

						// ... just populate the single data structures that hold all data across all inner tables
						// ... (including possible null data for the newly added columns)
						if (!swap_current_and_previous_and_set_previous_to_null)
						{
							bound_parameter_which_binding_to_use.push_back(SQLExecutor::NULL_BINDING);
						}
						else
						{
							if (index > highest_index_previous_table)
							{
								bound_parameter_which_binding_to_use.push_back(SQLExecutor::NULL_BINDING);
							}
						}

						// The following if/else block only applies to PRIMARY_VARIABLE_GROUP merges
						// Now, also save the data into sets broken down by inner table
						if (!swap_current_and_previous_and_set_previous_to_null)
						{
							inner_table_bindings.push_back(std::make_pair(OutputModel::OutputGenerator::SQLExecutor::NULL_BINDING, 0));
							if (column_in_view.total_multiplicity__of_current_dmu_category__within_uoa_corresponding_to_the_current_inner_tables_variable_group > 1)
							{
								inner_table__primary_keys_with_multiplicity_greater_than_one__which_binding_to_use__set.push_back(std::make_pair(OutputModel::OutputGenerator::SQLExecutor::NULL_BINDING, 0));
							}
						}
						else
						{
							if (index > highest_index_previous_table)
							{
								// unused... confirm that this can be removed
								inner_table_bindings.push_back(std::make_pair(OutputModel::OutputGenerator::SQLExecutor::NULL_BINDING, 0));
								if (column_in_view.total_multiplicity__of_current_dmu_category__within_uoa_corresponding_to_the_current_inner_tables_variable_group > 1)
								{
									inner_table__primary_keys_with_multiplicity_greater_than_one__which_binding_to_use__set.push_back(std::make_pair(OutputModel::OutputGenerator::SQLExecutor::NULL_BINDING, 0));
								}
							}
						}

					}
					break;

				default:
					{
						// Todo: Error message
						boost::format msg("Unknown data type in column.  (Error while creating new timerange-managed row.)");
						SetFailureMessage(msg.str());
						failed = true;
						return; // from lambda
					}

			}

		}

		++index;

	});

	// The following if/else block only applies to PRIMARY_VARIABLE_GROUP merges
	if (swap_current_and_previous_and_set_previous_to_null)
	{
		// The addition of 2 handles the fact that the new table being added
		// has no MERGED time range columns
		for (int n=0; n<number_nulls_to_add_at_end + 2; ++n)
		{
			bound_parameter_which_binding_to_use.push_back(SQLExecutor::NULL_BINDING);
		}
	}
	else
	{
		// In the previous method, there is nothing to do here -
		// the single bound_parameter data structures are filled.

		// But in the new method, we sort the data by inner table column set.
		bool new_method = true;
		if (new_method && xr_table_category == OutputModel::OutputGenerator::PRIMARY_VARIABLE_GROUP)
		{
			int number_columns_first_sets = (int)inner_table_columns[0].bindings.size();
			int number_columns_last_set = (int)inner_table_columns[inner_table_columns.size() - 1].bindings.size();
			int number_inner_column_sets = (int)inner_table_columns.size();
			// sanity check
			if (number_columns_first_sets != number_columns_last_set + 2)
			{
				// The final inner column set is missing the "merged" datetime columns
				boost::format msg("The number of columns in previous inner tables does not match the number of columns in the current inner table (not including timerange columns).  (Error while creating new timerange-managed row.)");
				SetFailureMessage(msg.str());
				failed = true;
				return false;
			}
			std::sort(inner_table_columns.begin(), inner_table_columns.end());
			bound_parameter_ints.clear();
			bound_parameter_strings.clear();
			bound_parameter_which_binding_to_use.clear();
			int current_inner_column_set = 0;
			std::for_each(inner_table_columns.cbegin(), inner_table_columns.cend(), [&current_inner_column_set, &number_inner_column_sets, &number_columns_first_sets, &bound_parameter_ints, &bound_parameter_strings, &bound_parameter_which_binding_to_use](ColumnSorter const & columns_in_single_inner_table)
			{
				int the_index = 0;
				std::for_each(columns_in_single_inner_table.bindings.cbegin(), columns_in_single_inner_table.bindings.cend(), [&columns_in_single_inner_table, &the_index, &bound_parameter_ints, &bound_parameter_strings, &bound_parameter_which_binding_to_use](std::pair<OutputModel::OutputGenerator::SQLExecutor::WHICH_BINDING, int> const & binding_info)
				{
					switch (binding_info.first)
					{
						case OutputModel::OutputGenerator::SQLExecutor::INT64:
							{
								bound_parameter_ints.push_back(columns_in_single_inner_table.ints[binding_info.second]);
								bound_parameter_which_binding_to_use.push_back(OutputModel::OutputGenerator::SQLExecutor::INT64);
							}
							break;
						case OutputModel::OutputGenerator::SQLExecutor::STRING:
							{
								bound_parameter_strings.push_back(columns_in_single_inner_table.strings[binding_info.second]);
								bound_parameter_which_binding_to_use.push_back(OutputModel::OutputGenerator::SQLExecutor::STRING);
							}
							break;
						case OutputModel::OutputGenerator::SQLExecutor::NULL_BINDING:
							{
								bound_parameter_which_binding_to_use.push_back(OutputModel::OutputGenerator::SQLExecutor::NULL_BINDING);
							}
							break;
					}
					++the_index;
				});
				if (current_inner_column_set < number_inner_column_sets - 1)
				{
					// The current (new) inner column set being constructed here is not the last
					if (columns_in_single_inner_table.bindings.size() == number_columns_first_sets)
					{
						// OK - the "merged" datetime columns have been added
					}
					else
					{
						// Must add them... just fill with 0; only the datetime column from the last set is used by the algorithm
						bound_parameter_ints.push_back(0);
						bound_parameter_which_binding_to_use.push_back(OutputModel::OutputGenerator::SQLExecutor::INT64);
						bound_parameter_ints.push_back(0);
						bound_parameter_which_binding_to_use.push_back(OutputModel::OutputGenerator::SQLExecutor::INT64);
					}
				}
				else
				{
					// The current (new) inner column set being constructed here is the last one
					if (columns_in_single_inner_table.bindings.size() == number_columns_first_sets)
					{
						// The "merged" datetime columns have been added, but they should not have been in this special case
						OutputModel::OutputGenerator::SQLExecutor::WHICH_BINDING binding_1 = bound_parameter_which_binding_to_use.back();
						bound_parameter_which_binding_to_use.pop_back();
						OutputModel::OutputGenerator::SQLExecutor::WHICH_BINDING binding_2 = bound_parameter_which_binding_to_use.back();
						bound_parameter_which_binding_to_use.pop_back();
						switch (binding_1)
						{
							case OutputModel::OutputGenerator::SQLExecutor::INT64:
								{
									bound_parameter_ints.pop_back();
								}
								break;
							case OutputModel::OutputGenerator::SQLExecutor::STRING:
								{
									bound_parameter_strings.pop_back();
								}
								break;
							case OutputModel::OutputGenerator::SQLExecutor::NULL_BINDING:
								{
								}
								break;
						}
						switch (binding_2)
						{
							case OutputModel::OutputGenerator::SQLExecutor::INT64:
								{
									bound_parameter_ints.pop_back();
								}
								break;
							case OutputModel::OutputGenerator::SQLExecutor::STRING:
								{
									bound_parameter_strings.pop_back();
								}
								break;
							case OutputModel::OutputGenerator::SQLExecutor::NULL_BINDING:
								{
								}
								break;
						}
					}
					else
					{
						// OK - the "merged" datetime columns have not been added
					}
				}
				++current_inner_column_set;
			});
		}
	}

	if (failed)
	{
		return false;
	}

	// The two new "merged" time range columns
	bound_parameter_ints.push_back(datetime_start);
	bound_parameter_which_binding_to_use.push_back(SQLExecutor::INT64);
	bound_parameter_ints.push_back(datetime_end);
	bound_parameter_which_binding_to_use.push_back(SQLExecutor::INT64);

	return true;

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

	WidgetInstanceIdentifier variable_group_child;
	WidgetInstanceIdentifier uoa_child;

	// These columns are from the previous XR temporary table, which is guaranteed to have all columns in place, including datetime columns.
	// Further, the "current_multiplicity" of these columns is guaranteed to be correct.
	// Also, the first columns always correspond to the primary variable group.
	bool first = true;
	bool in_first_inner_table = true;
	bool reached_first_datetime_start_merged_column = false;
	bool reached_first_datetime_end_merged_column = false;
	std::for_each(result_columns.columns_in_view.begin(), result_columns.columns_in_view.end(), [&first_full_table_column_count, &top_level_inner_table_column_count, &in_first_inner_table, &reached_first_datetime_start_merged_column, &reached_first_datetime_end_merged_column, &previous_column_names_first_table, &first](ColumnsInTempView::ColumnInTempView & new_column)
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
	});

	WidgetInstanceIdentifiers const & variables_selected = (*the_map)[*child_variable_group_raw_data_columns.variable_groups[0].identifier_parent][child_variable_group_raw_data_columns.variable_groups[0]];

	// These columns are from the new table (the raw child data table) being added.
	// Make column names for this temporary table unique (not the same as the column names from the previous table that is being copied)
	// which may or may not have datetime columns.
	// Further, the "current_multiplicity" of these columns is 1, and must be updated.
	//
	// Start with the primary key columns.
	first = true;
	std::for_each(child_variable_group_raw_data_columns.columns_in_view.cbegin(), child_variable_group_raw_data_columns.columns_in_view.cend(), [&first, &child_set_number, &variable_group_child, &uoa_child, &variables_selected, &result_columns, &second_table_column_count, &current_multiplicity](ColumnsInTempView::ColumnInTempView const & new_column_child)
	{
		if (new_column_child.column_type == ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__DATETIMESTART || new_column_child.column_type == ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__DATETIMESTART_INTERNAL || new_column_child.column_type == ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__DATETIMEEND || new_column_child.column_type == ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__DATETIMEEND_INTERNAL)
		{
			return; // Add these columns last
		}

		bool match = true;

		if (new_column_child.column_type == ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__SECONDARY)
		{
			return; // Enforce that primary key columns appear first.
		}

		if (match)
		{
			result_columns.columns_in_view.push_back(new_column_child);
			ColumnsInTempView::ColumnInTempView & new_column = result_columns.columns_in_view.back();
			new_column.column_name_in_temporary_table = new_column.column_name_in_temporary_table_no_uuid;
			new_column.column_name_in_temporary_table += "_";
			new_column.column_name_in_temporary_table += newUUID(true);
			new_column.inner_table_set_number = child_set_number;
			new_column.is_within_inner_table_corresponding_to_top_level_uoa = false;
			new_column.current_multiplicity__of__current_inner_table__within__current_vg = current_multiplicity;
			if (new_column.column_type == ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__PRIMARY)
			{
				if (new_column.total_multiplicity__of_current_dmu_category__within_uoa_corresponding_to_the_current_inner_tables_variable_group > 1)
				{
					new_column.current_multiplicity__corresponding_to__current_inner_table___is_1_in_all_inner_tables_when_multiplicity_is_1_for_that_vg = current_multiplicity; // update current multiplicity
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
	// Proceed to the secondary key columns.
	std::for_each(child_variable_group_raw_data_columns.columns_in_view.cbegin(), child_variable_group_raw_data_columns.columns_in_view.cend(), [&first, &child_set_number, &variable_group_child, &uoa_child, &variables_selected, &result_columns, &second_table_column_count, &current_multiplicity](ColumnsInTempView::ColumnInTempView const & new_column_secondary)
	{
		if (new_column_secondary.column_type == ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__DATETIMESTART || new_column_secondary.column_type == ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__DATETIMESTART_INTERNAL || new_column_secondary.column_type == ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__DATETIMEEND || new_column_secondary.column_type == ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__DATETIMEEND_INTERNAL)
		{
			return; // Add these columns last
		}

		if (new_column_secondary.column_type != ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__SECONDARY)
		{
			return; // We are populating secondary columns now, so exit if this isn't one
		}

		bool match = false;
		std::for_each(variables_selected.cbegin(), variables_selected.cend(), [&new_column_secondary, &match](WidgetInstanceIdentifier const & variable_selected)
		{
			if (boost::iequals(new_column_secondary.column_name_in_original_data_table, *variable_selected.code))
			{
				match = true;
			}
		});

		if (match)
		{
			result_columns.columns_in_view.push_back(new_column_secondary);
			ColumnsInTempView::ColumnInTempView & new_column = result_columns.columns_in_view.back();
			new_column.column_name_in_temporary_table = new_column.column_name_in_temporary_table_no_uuid;
			new_column.column_name_in_temporary_table += "_";
			new_column.column_name_in_temporary_table += newUUID(true);
			new_column.inner_table_set_number = child_set_number;
			new_column.is_within_inner_table_corresponding_to_top_level_uoa = false;
			new_column.current_multiplicity__of__current_inner_table__within__current_vg = current_multiplicity;
			if (new_column.column_type == ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__PRIMARY)
			{
				if (new_column.total_multiplicity__of_current_dmu_category__within_uoa_corresponding_to_the_current_inner_tables_variable_group > 1)
				{
					new_column.current_multiplicity__corresponding_to__current_inner_table___is_1_in_all_inner_tables_when_multiplicity_is_1_for_that_vg = current_multiplicity; // update current multiplicity
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
	// Proceed, finally, to the datetime columns, if they exist.  (If they don't, they will be added via ALTER TABLE to the temporary table under construction.)
	std::for_each(child_variable_group_raw_data_columns.columns_in_view.cbegin(), child_variable_group_raw_data_columns.columns_in_view.cend(), [&result_columns, &child_set_number, &second_table_column_count](ColumnsInTempView::ColumnInTempView const & new_column_datetime)
	{
		if (new_column_datetime.column_type == ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__DATETIMESTART || new_column_datetime.column_type == ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__DATETIMESTART_INTERNAL)
		{
			result_columns.columns_in_view.push_back(new_column_datetime);
			ColumnsInTempView::ColumnInTempView & new_column = result_columns.columns_in_view.back();
			new_column.column_name_in_temporary_table = new_column.column_name_in_temporary_table_no_uuid;
			new_column.column_name_in_temporary_table += "_";
			new_column.column_name_in_temporary_table += newUUID(true);
			new_column.inner_table_set_number = child_set_number;
			new_column.is_within_inner_table_corresponding_to_top_level_uoa = false;
			++second_table_column_count;
		}
	});
	std::for_each(child_variable_group_raw_data_columns.columns_in_view.cbegin(), child_variable_group_raw_data_columns.columns_in_view.cend(), [&result_columns, &child_set_number, &second_table_column_count](ColumnsInTempView::ColumnInTempView const & new_column_datetime)
	{
		if (new_column_datetime.column_type == ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__DATETIMEEND || new_column_datetime.column_type == ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__DATETIMEEND_INTERNAL)
		{
			result_columns.columns_in_view.push_back(new_column_datetime);
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
	sql_string += " t1 LEFT OUTER JOIN ";
	sql_string += child_variable_group_raw_data_columns.original_table_names[0];
	sql_string += " t2 ON ";
	bool and_ = false;
	std::for_each(sequence.primary_key_sequence_info.cbegin(), sequence.primary_key_sequence_info.cend(), [this, &current_multiplicity, &sql_string, &variable_group_child, &result_columns, &first_full_table_column_count, &top_level_inner_table_column_count, &second_table_column_count, &previous_column_names_first_table, &and_](PrimaryKeySequence::PrimaryKeySequenceEntry const & primary_key)
	{
		std::for_each(primary_key.variable_group_info_for_primary_keys.cbegin(), primary_key.variable_group_info_for_primary_keys.cend(), [this, &current_multiplicity, &sql_string, &variable_group_child, &primary_key, &result_columns, &first_full_table_column_count, &top_level_inner_table_column_count, &second_table_column_count, &previous_column_names_first_table, &and_](PrimaryKeySequence::VariableGroup_PrimaryKey_Info const & primary_key_info_this_variable_group)
		{
			if (primary_key_info_this_variable_group.vg_identifier.IsEqual(WidgetInstanceIdentifier::EQUALITY_CHECK_TYPE__STRING_CODE, variable_group_child))
			{
				if (primary_key_info_this_variable_group.current_multiplicity == current_multiplicity)
				{
					int column_count = 0;
					std::for_each(result_columns.columns_in_view.cbegin(), result_columns.columns_in_view.cend(), [this, &current_multiplicity, &sql_string, &primary_key_info_this_variable_group, &first_full_table_column_count, &top_level_inner_table_column_count, &second_table_column_count, &column_count, &previous_column_names_first_table, &primary_key, &and_](ColumnsInTempView::ColumnInTempView const & new_column)
					{

						// The following 2 "if" checks are redundant and do the same thing.
						// They are both here in order to help understand the use of the metadata.
						if (!new_column.is_within_inner_table_corresponding_to_top_level_uoa)
						{
							++column_count;
							return;
						}
						if (column_count >= highest_multiplicity_primary_uoa * top_level_inner_table_column_count)
						{
							++column_count;
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
											match_condition = (new_column.primary_key_index_within_total_kad_for_dmu_category >= 0 && (new_column.primary_key_index__within_uoa_corresponding_to_variable_group_corresponding_to_current_inner_table__for_dmu_category == primary_key_info_this_variable_group.sequence_number_within_dmu_category_for_this_variable_groups_uoa));
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

	if (!child_variable_group_raw_data_columns.has_no_datetime_columns_originally)
	{
		sql_string += " WHERE CASE WHEN ";
		sql_string += result_columns.columns_in_view[result_columns.columns_in_view.size() - 2].column_name_in_temporary_table;
		sql_string += " = 0 AND ";
		sql_string += result_columns.columns_in_view[result_columns.columns_in_view.size() - 1].column_name_in_temporary_table;
		sql_string += " = 0 ";
		sql_string += " THEN 1 ";
		sql_string += " WHEN ";
		sql_string += result_columns.columns_in_view[result_columns.columns_in_view.size() - 2].column_name_in_temporary_table;
		sql_string += " < ";
		sql_string += boost::lexical_cast<std::string>(timerange_end);
		sql_string += " THEN ";
		sql_string += result_columns.columns_in_view[result_columns.columns_in_view.size() - 1].column_name_in_temporary_table;
		sql_string += " > ";
		sql_string += boost::lexical_cast<std::string>(timerange_start);
		sql_string += " WHEN ";
		sql_string += result_columns.columns_in_view[result_columns.columns_in_view.size() - 1].column_name_in_temporary_table;
		sql_string += " > ";
		sql_string += boost::lexical_cast<std::string>(timerange_start);
		sql_string += " THEN ";
		sql_string += result_columns.columns_in_view[result_columns.columns_in_view.size() - 2].column_name_in_temporary_table;
		sql_string += " < ";
		sql_string += boost::lexical_cast<std::string>(timerange_end);
		sql_string += " END";
	}

	// For use in ORDER BY clause
	// Determine how many columns there are corresponding to the DMU category with multiplicity greater than 1
	int number_primary_key_columns_in_dmu_category_with_multiplicity_greater_than_1__for_top_level_uoa = 0;
	WidgetInstanceIdentifier first_top_level_vg;
	first = true;
	if (debug_ordering)
	{
		if (highest_multiplicity_primary_uoa > 1)
		{
			std::for_each(result_columns.columns_in_view.begin(), result_columns.columns_in_view.end(), [this, &first, &first_top_level_vg, &number_primary_key_columns_in_dmu_category_with_multiplicity_greater_than_1__for_top_level_uoa, &sql_string](ColumnsInTempView::ColumnInTempView & view_column)
			{
				if (first)
				{
					first_top_level_vg = view_column.variable_group_associated_with_current_inner_table;
				}
				first = false;
				if (view_column.column_type == ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__PRIMARY)
				{
					if (view_column.is_within_inner_table_corresponding_to_top_level_uoa)
					{
						if (view_column.total_multiplicity__of_current_dmu_category__within_uoa_corresponding_to_the_current_inner_tables_variable_group == highest_multiplicity_primary_uoa)
						{
							if (view_column.current_multiplicity__corresponding_to__current_inner_table___is_1_in_all_inner_tables_when_multiplicity_is_1_for_that_vg == 1)
							{
								if (view_column.variable_group_associated_with_current_inner_table.IsEqual(WidgetInstanceIdentifier::EQUALITY_CHECK_TYPE__STRING_CODE, first_top_level_vg))
								{
									++number_primary_key_columns_in_dmu_category_with_multiplicity_greater_than_1__for_top_level_uoa;
								}
							}
						}
					}
				}
			});
		}
	}

	// Add the ORDER BY column/s
	if (debug_ordering)
	{

		bool first = true;

		if (highest_multiplicity_primary_uoa > 1)
		{

			// Create the ORDER BY clause, taking the proper primary key columns that compose the DMU category with multiplicity greater than 1 FOR THE TOP-LEVEL UOA, in sequence
			for (int outer_dmu_multiplicity = 1; outer_dmu_multiplicity <= highest_multiplicity_primary_uoa; ++outer_dmu_multiplicity)
			{
				for (int inner_dmu_multiplicity = 0; inner_dmu_multiplicity < number_primary_key_columns_in_dmu_category_with_multiplicity_greater_than_1__for_top_level_uoa; ++inner_dmu_multiplicity)
				{
					std::for_each(result_columns.columns_in_view.begin(), result_columns.columns_in_view.end(), [this, &first_top_level_vg, &inner_dmu_multiplicity, &outer_dmu_multiplicity, &sql_string, &first](ColumnsInTempView::ColumnInTempView & view_column)
					{
						if (view_column.column_type == ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__PRIMARY)
						{
							if (view_column.total_multiplicity__of_current_dmu_category__within_uoa_corresponding_to_the_current_inner_tables_variable_group == highest_multiplicity_primary_uoa)
							{
								if (view_column.primary_key_index__within_uoa_corresponding_to_variable_group_corresponding_to_current_inner_table__for_dmu_category == inner_dmu_multiplicity)
								{
									if (view_column.current_multiplicity__corresponding_to__current_inner_table___is_1_in_all_inner_tables_when_multiplicity_is_1_for_that_vg == outer_dmu_multiplicity)
									{
										if (view_column.is_within_inner_table_corresponding_to_top_level_uoa)
										{
											if (view_column.variable_group_associated_with_current_inner_table.IsEqual(WidgetInstanceIdentifier::EQUALITY_CHECK_TYPE__STRING_CODE, first_top_level_vg))
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
						}
					});
				}
			}

		}

		// Now order by remaining primary key columns (with multiplicity 1 FOR THE TOP-LEVEL UOA)
		// ... If there are no primary key DMU categories for the top-level UOA with multiplicity greater than 1,
		// then this section will order by all of the top-level UOA's primary key DMU categories.
		int current_column = 0;
		std::for_each(result_columns.columns_in_view.begin(), result_columns.columns_in_view.end(), [this, &sql_string, &result_columns, &current_column, &top_level_inner_table_column_count, &first](ColumnsInTempView::ColumnInTempView & view_column)
		{
			if (current_column >= top_level_inner_table_column_count)
			{
				++current_column;
				return;
			}

			// Determine how many columns there are corresponding to this DMU category
			int number_primary_key_columns_in_dmu_category_with_multiplicity_of_only_1__for_top_level_uoa = 0;
			int column_count_nested = 0;
			std::for_each(result_columns.columns_in_view.begin(), result_columns.columns_in_view.end(), [this, &view_column, &column_count_nested, &top_level_inner_table_column_count, &number_primary_key_columns_in_dmu_category_with_multiplicity_of_only_1__for_top_level_uoa, &sql_string](ColumnsInTempView::ColumnInTempView & view_column_nested)
			{
				if (column_count_nested >= top_level_inner_table_column_count)
				{
					++column_count_nested;
					return;
				}
				if (view_column_nested.column_type == ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__PRIMARY)
				{
					if (view_column_nested.primary_key_dmu_category_identifier.IsEqual(WidgetInstanceIdentifier::EQUALITY_CHECK_TYPE__STRING_CODE, view_column.primary_key_dmu_category_identifier))
					{
						if (view_column_nested.total_multiplicity__of_current_dmu_category__within_uoa_corresponding_to_the_current_inner_tables_variable_group == 1)
						{
							if (view_column_nested.is_within_inner_table_corresponding_to_top_level_uoa)
							{
								++number_primary_key_columns_in_dmu_category_with_multiplicity_of_only_1__for_top_level_uoa;
							}
						}
					}
				}
				++column_count_nested;
			});

			if (view_column.column_type == ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__PRIMARY)
			{
				if (view_column.total_multiplicity__of_current_dmu_category__within_uoa_corresponding_to_the_current_inner_tables_variable_group == 1)
				{
					for (int inner_dmu_multiplicity = 0; inner_dmu_multiplicity < number_primary_key_columns_in_dmu_category_with_multiplicity_of_only_1__for_top_level_uoa; ++inner_dmu_multiplicity)
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

OutputModel::OutputGenerator::SqlAndColumnSet OutputModel::OutputGenerator::CreateXRTable(ColumnsInTempView & previous_x_or_final_columns_being_cleaned_over_timerange, int const current_multiplicity, int const primary_group_number, XR_TABLE_CATEGORY const xr_table_category, int const current_set_number, int const current_view_name_index, std::int64_t & rows_estimated)
{

	std::int64_t saved_initial_progress_bar_value = current_progress_value;

	char c[256];

	SqlAndColumnSet result = std::make_pair(std::vector<SQLExecutor>(), ColumnsInTempView());
	std::vector<SQLExecutor> & sql_strings = result.first;
	ColumnsInTempView & result_columns = result.second;

	result_columns = previous_x_or_final_columns_being_cleaned_over_timerange;
	result_columns.most_recent_sql_statement_executed__index = -1;

	std::string view_name;
	if (xr_table_category == OutputModel::OutputGenerator::CHILD_VARIABLE_GROUP)
	{
		view_name += "CV";
	}
	else if (xr_table_category == OutputModel::OutputGenerator::PRIMARY_VARIABLE_GROUP)
	{
		view_name += "V";
	}
	else if (xr_table_category == OutputModel::OutputGenerator::FINAL_MERGE_OF_PRIMARY_VARIABLE_GROUP)
	{
		view_name += "MF";
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

	std::vector<std::string> previous_table_column_names;
	
	// rename columns so they are different than in previous table
	std::for_each(result_columns.columns_in_view.begin(), result_columns.columns_in_view.end(), [&previous_table_column_names](ColumnsInTempView::ColumnInTempView & new_column)
	{
		previous_table_column_names.push_back(new_column.column_name_in_temporary_table);
		new_column.column_name_in_temporary_table = new_column.column_name_in_temporary_table_no_uuid;
		new_column.column_name_in_temporary_table += "_";
		new_column.column_name_in_temporary_table += newUUID(true);
	});


	std::string sql_create_empty_table;
	sql_create_empty_table += "CREATE TABLE ";
	sql_create_empty_table += result_columns.view_name;
	sql_create_empty_table += " AS SELECT ";

	bool first = true;
	int the_index = 0;
	std::for_each(result_columns.columns_in_view.begin(), result_columns.columns_in_view.end(), [&sql_create_empty_table, &the_index, &previous_table_column_names, &first](ColumnsInTempView::ColumnInTempView & new_column)
	{
		if (!first)
		{
			sql_create_empty_table += ", ";
		}
		first = false;
		sql_create_empty_table += previous_table_column_names[the_index];
		sql_create_empty_table += " AS ";
		sql_create_empty_table += new_column.column_name_in_temporary_table;
		++the_index;
	});

	sql_create_empty_table += " FROM ";
	sql_create_empty_table += previous_x_or_final_columns_being_cleaned_over_timerange.view_name;
	sql_create_empty_table += " WHERE 0";
	sql_strings.push_back(SQLExecutor(db, sql_create_empty_table));

	
	bool new_method = true;

	WidgetInstanceIdentifier variable_group;
	WidgetInstanceIdentifier uoa;

	// Pull from the simple datetime columns at the end of the child's X table, which is guaranteed to be in place
	variable_group = previous_x_or_final_columns_being_cleaned_over_timerange.columns_in_view[previous_x_or_final_columns_being_cleaned_over_timerange.columns_in_view.size()-1].variable_group_associated_with_current_inner_table;
	uoa = previous_x_or_final_columns_being_cleaned_over_timerange.columns_in_view[previous_x_or_final_columns_being_cleaned_over_timerange.columns_in_view.size()-1].uoa_associated_with_variable_group_associated_with_current_inner_table;

	std::string datetime_start_col_name_no_uuid;
	if (xr_table_category == OutputModel::OutputGenerator::PRIMARY_VARIABLE_GROUP || xr_table_category == OutputModel::OutputGenerator::CHILD_VARIABLE_GROUP)
	{
		if (new_method && xr_table_category == OutputModel::OutputGenerator::CHILD_VARIABLE_GROUP)
		{
			datetime_start_col_name_no_uuid += "DATETIME_ROW_START_CHILD_MERGE";
		}
		else
		{
			datetime_start_col_name_no_uuid += "DATETIME_ROW_START_MERGED";
		}
	}
	else if (xr_table_category == OutputModel::OutputGenerator::FINAL_MERGE_OF_PRIMARY_VARIABLE_GROUP)
	{
		datetime_start_col_name_no_uuid += "DATETIME_ROW_START_MERGED_BETWEEN_FINALS";
	}
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
	if (xr_table_category == OutputModel::OutputGenerator::PRIMARY_VARIABLE_GROUP || xr_table_category == OutputModel::OutputGenerator::CHILD_VARIABLE_GROUP)
	{
		if (new_method && xr_table_category == OutputModel::OutputGenerator::CHILD_VARIABLE_GROUP)
		{
			datetime_start_column.column_type = ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__DATETIMESTART_CHILD_MERGE;
		}
		else
		{
			datetime_start_column.column_type = ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__DATETIMESTART_MERGED;
		}
	}
	else if (xr_table_category == OutputModel::OutputGenerator::FINAL_MERGE_OF_PRIMARY_VARIABLE_GROUP)
	{
		datetime_start_column.column_type = ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__DATETIMESTART_MERGED_BETWEEN_FINALS;
	}
	datetime_start_column.variable_group_associated_with_current_inner_table = variable_group;
	datetime_start_column.uoa_associated_with_variable_group_associated_with_current_inner_table = uoa;
	datetime_start_column.column_name_in_original_data_table = "";
	datetime_start_column.inner_table_set_number = current_set_number;
	if (xr_table_category == OutputModel::OutputGenerator::PRIMARY_VARIABLE_GROUP)
	{
		datetime_start_column.is_within_inner_table_corresponding_to_top_level_uoa = true;
	}
	else if (xr_table_category == OutputModel::OutputGenerator::CHILD_VARIABLE_GROUP)
	{
		datetime_start_column.is_within_inner_table_corresponding_to_top_level_uoa = false;
	}
	else if (xr_table_category == OutputModel::OutputGenerator::FINAL_MERGE_OF_PRIMARY_VARIABLE_GROUP)
	{
		datetime_start_column.is_within_inner_table_corresponding_to_top_level_uoa = false;
	}

	std::string datetime_end_col_name_no_uuid;
	if (xr_table_category == OutputModel::OutputGenerator::PRIMARY_VARIABLE_GROUP || xr_table_category == OutputModel::OutputGenerator::CHILD_VARIABLE_GROUP)
	{
		if (new_method && xr_table_category == OutputModel::OutputGenerator::CHILD_VARIABLE_GROUP)
		{
			datetime_end_col_name_no_uuid += "DATETIME_ROW_END_CHILD_MERGE";
		}
		else
		{
			datetime_end_col_name_no_uuid += "DATETIME_ROW_END_MERGED";
		}
	}
	else if (xr_table_category == OutputModel::OutputGenerator::FINAL_MERGE_OF_PRIMARY_VARIABLE_GROUP)
	{
		datetime_end_col_name_no_uuid += "DATETIME_ROW_END_MERGED_BETWEEN_FINALS";
	}
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
	if (xr_table_category == OutputModel::OutputGenerator::PRIMARY_VARIABLE_GROUP || xr_table_category == OutputModel::OutputGenerator::CHILD_VARIABLE_GROUP)
	{
		if (new_method && xr_table_category == OutputModel::OutputGenerator::CHILD_VARIABLE_GROUP)
		{
			datetime_end_column.column_type = ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__DATETIMEEND_CHILD_MERGE;
		}
		else
		{
			datetime_end_column.column_type = ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__DATETIMEEND_MERGED;
		}
	}
	else if (xr_table_category == OutputModel::OutputGenerator::FINAL_MERGE_OF_PRIMARY_VARIABLE_GROUP)
	{
		datetime_end_column.column_type = ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__DATETIMEEND_MERGED_BETWEEN_FINALS;
	}
	datetime_end_column.variable_group_associated_with_current_inner_table = variable_group;
	datetime_end_column.uoa_associated_with_variable_group_associated_with_current_inner_table = uoa;
	datetime_end_column.column_name_in_original_data_table = "";
	datetime_end_column.inner_table_set_number = current_set_number;
	if (xr_table_category == OutputModel::OutputGenerator::PRIMARY_VARIABLE_GROUP)
	{
		datetime_end_column.is_within_inner_table_corresponding_to_top_level_uoa = true;
	}
	else if (xr_table_category == OutputModel::OutputGenerator::CHILD_VARIABLE_GROUP)
	{
		datetime_end_column.is_within_inner_table_corresponding_to_top_level_uoa = false;
	}
	else if (xr_table_category == OutputModel::OutputGenerator::FINAL_MERGE_OF_PRIMARY_VARIABLE_GROUP)
	{
		datetime_end_column.is_within_inner_table_corresponding_to_top_level_uoa = false;
	}


	int previous_datetime_start_column_index = -1;
	int previous_datetime_end_column_index = -1;
	int current_datetime_start_column_index = -1;
	int current_datetime_end_column_index = -1;
	int column_index = (int)previous_x_or_final_columns_being_cleaned_over_timerange.columns_in_view.size() - 1;
	std::for_each(previous_x_or_final_columns_being_cleaned_over_timerange.columns_in_view.crbegin(), previous_x_or_final_columns_being_cleaned_over_timerange.columns_in_view.crend(), [&new_method, &xr_table_category, &previous_datetime_start_column_index, &previous_datetime_end_column_index, &current_datetime_start_column_index, &current_datetime_end_column_index, &column_index](ColumnsInTempView::ColumnInTempView const & schema_column)
	{

		// The previous values are always located after the current values, but in arbitrary order,
		// so this check suffices to be certain that all 4 values have been obtained
		if (previous_datetime_start_column_index != -1 && previous_datetime_end_column_index != -1)
		{
			--column_index;
			return;
		}

		if (xr_table_category == OutputModel::OutputGenerator::PRIMARY_VARIABLE_GROUP || xr_table_category == OutputModel::OutputGenerator::CHILD_VARIABLE_GROUP)
		{

			if (new_method && xr_table_category == OutputModel::OutputGenerator::CHILD_VARIABLE_GROUP)
			{

				// COLUMN_TYPE__DATETIMESTART_MERGED can only be for the previous data
				if (schema_column.column_type == ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__DATETIMESTART_MERGED || schema_column.column_type == ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__DATETIMESTART_MERGED_BETWEEN_FINALS)
				{
					if (previous_datetime_start_column_index == -1)
					{
						previous_datetime_start_column_index = column_index;
					}
				}
				// COLUMN_TYPE__DATETIMEEND_MERGED can only be for the previous data
				else if (schema_column.column_type == ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__DATETIMEEND_MERGED || schema_column.column_type == ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__DATETIMEEND_MERGED_BETWEEN_FINALS)
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

			}
			else
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

			}

		}
		else if (xr_table_category == OutputModel::OutputGenerator::FINAL_MERGE_OF_PRIMARY_VARIABLE_GROUP)
		{

			// COLUMN_TYPE__DATETIMESTART_MERGED_BETWEEN_FINALS can only be for the previous data
			if (schema_column.column_type == ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__DATETIMESTART_MERGED_BETWEEN_FINALS)
			{
				if (previous_datetime_start_column_index == -1)
				{
					previous_datetime_start_column_index = column_index;
				}
			}
			// COLUMN_TYPE__DATETIMEEND_MERGED_BETWEEN_FINALS can only be for the previous data
			else if (schema_column.column_type == ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__DATETIMEEND_MERGED_BETWEEN_FINALS)
			{
				if (previous_datetime_end_column_index == -1)
				{
					previous_datetime_end_column_index = column_index;
				}
			}
			// COLUMN_TYPE__DATETIMESTART_MERGED_FINAL, when first seen, can only be for the current data
			else if (schema_column.column_type == ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__DATETIMESTART_MERGED_FINAL)
			{
				if (current_datetime_start_column_index == -1)
				{
					current_datetime_start_column_index = column_index;
				}
			}
			// COLUMN_TYPE__DATETIMEEND_MERGED_FINAL, when first seen, can only be for the current data
			else if (schema_column.column_type == ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__DATETIMEEND_MERGED_FINAL)
			{
				if (current_datetime_end_column_index == -1)
				{
					current_datetime_end_column_index = column_index;
				}
			}

		}

		--column_index;

	});

	ExecuteSQL(result); // Executes all SQL queries up to the current one

	if (failed)
	{
		return result;
	}


	ObtainData(previous_x_or_final_columns_being_cleaned_over_timerange);

	if (failed)
	{
		return result;
	}


	int const minimum_desired_rows_per_transaction = 256;

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

		int previous_data_type = sqlite3_column_type(stmt_result, previous_datetime_start_column_index);
		int current_data_type = sqlite3_column_type(stmt_result, current_datetime_start_column_index);
		bool previous_datetime_is_null = (previous_data_type == SQLITE_NULL);
		bool current_datetime_is_null = (current_data_type == SQLITE_NULL);

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

		bool added = false;

		if (previous_datetime_is_null && current_datetime_is_null)
		{
			// no data
			continue;
		}

		else if (previous_datetime_is_null)
		{

			// Add only current data, setting time range to that of the previous data
			added = CreateNewXRRow(first_row_added, datetime_start_col_name, datetime_end_col_name, result_columns.view_name, sql_add_xr_row, bound_parameter_strings, bound_parameter_ints, bound_parameter_which_binding_to_use, current_datetime_start, current_datetime_end, previous_x_or_final_columns_being_cleaned_over_timerange, result_columns, false, true, xr_table_category);
			if (failed)
			{
				break;
			}
			if (added)
			{
				sql_strings.push_back(SQLExecutor(db, sql_add_xr_row, bound_parameter_strings, bound_parameter_ints, bound_parameter_which_binding_to_use, the_prepared_stmt, true));
				the_prepared_stmt = sql_strings.back().stmt;
				++current_rows_added;
				++current_rows_added_since_execution;
			}

		}
		
		else if (current_datetime_is_null)
		{

			// Add only previous data, setting time range to that of the previous data
			added = CreateNewXRRow(first_row_added, datetime_start_col_name, datetime_end_col_name, result_columns.view_name, sql_add_xr_row, bound_parameter_strings, bound_parameter_ints, bound_parameter_which_binding_to_use, previous_datetime_start, previous_datetime_end, previous_x_or_final_columns_being_cleaned_over_timerange, result_columns, true, false, xr_table_category);
			if (failed)
			{
				break;
			}
			if (added)
			{
				sql_strings.push_back(SQLExecutor(db, sql_add_xr_row, bound_parameter_strings, bound_parameter_ints, bound_parameter_which_binding_to_use, the_prepared_stmt, true));
				the_prepared_stmt = sql_strings.back().stmt;
				++current_rows_added;
				++current_rows_added_since_execution;
			}

		}

		else if (previous_is_0 && current_is_0)
		{

			// Add row as-is, setting new time range columns to 0
			added = CreateNewXRRow(first_row_added, datetime_start_col_name, datetime_end_col_name, result_columns.view_name, sql_add_xr_row, bound_parameter_strings, bound_parameter_ints, bound_parameter_which_binding_to_use, 0, 0, previous_x_or_final_columns_being_cleaned_over_timerange, result_columns, true, true, xr_table_category);
			if (failed)
			{
				break;
			}
			if (added)
			{
				sql_strings.push_back(SQLExecutor(db, sql_add_xr_row, bound_parameter_strings, bound_parameter_ints, bound_parameter_which_binding_to_use, the_prepared_stmt, true));
				the_prepared_stmt = sql_strings.back().stmt;
				++current_rows_added;
				++current_rows_added_since_execution;
			}

		}

		else if (previous_is_0 && !current_is_0)
		{

			// Add row as-is, setting new time range columns to current time range values
			added = CreateNewXRRow(first_row_added, datetime_start_col_name, datetime_end_col_name, result_columns.view_name, sql_add_xr_row, bound_parameter_strings, bound_parameter_ints, bound_parameter_which_binding_to_use, current_datetime_start, current_datetime_end, previous_x_or_final_columns_being_cleaned_over_timerange, result_columns, true, true, xr_table_category);
			if (failed)
			{
				break;
			}
			if (added)
			{
				sql_strings.push_back(SQLExecutor(db, sql_add_xr_row, bound_parameter_strings, bound_parameter_ints, bound_parameter_which_binding_to_use, the_prepared_stmt, true));
				the_prepared_stmt = sql_strings.back().stmt;
				++current_rows_added;
				++current_rows_added_since_execution;
			}

		}

		else if (!previous_is_0 && current_is_0)
		{

			// Add row as-is, setting new time range columns to previous time range values
			added = CreateNewXRRow(first_row_added, datetime_start_col_name, datetime_end_col_name, result_columns.view_name, sql_add_xr_row, bound_parameter_strings, bound_parameter_ints, bound_parameter_which_binding_to_use, previous_datetime_start, previous_datetime_end, previous_x_or_final_columns_being_cleaned_over_timerange, result_columns, true, true, xr_table_category);
			if (failed)
			{
				break;
			}
			if (added)
			{
				sql_strings.push_back(SQLExecutor(db, sql_add_xr_row, bound_parameter_strings, bound_parameter_ints, bound_parameter_which_binding_to_use, the_prepared_stmt, true));
				the_prepared_stmt = sql_strings.back().stmt;
				++current_rows_added;
				++current_rows_added_since_execution;
			}

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

			if (xr_table_category == OutputModel::OutputGenerator::CHILD_VARIABLE_GROUP)
			{
				if (previous_is_lower)
				{
					previous__DO_include_lower_range_data__DO_include_upper_range_data = true;
					previous__DO_include_lower_range_data__DO_NOT_include_upper_range_data = true;
					previous__DO_NOT_include_lower_range_data__DO_include_upper_range_data = false;
					previous__DO_NOT_include_lower_range_data__DO_NOT_include_upper_range_data = false;
					current__DO_include_lower_range_data__DO_include_upper_range_data = true;
					current__DO_include_lower_range_data__DO_NOT_include_upper_range_data = false;
					current__DO_NOT_include_lower_range_data__DO_include_upper_range_data = false; // Even though current data exists, do not include it if there is no lower data
					current__DO_NOT_include_lower_range_data__DO_NOT_include_upper_range_data = false;
				}
				else
				{
					previous__DO_include_lower_range_data__DO_include_upper_range_data = true;
					previous__DO_include_lower_range_data__DO_NOT_include_upper_range_data = false;
					previous__DO_NOT_include_lower_range_data__DO_include_upper_range_data = true;
					previous__DO_NOT_include_lower_range_data__DO_NOT_include_upper_range_data = false;
					current__DO_include_lower_range_data__DO_include_upper_range_data = true;
					current__DO_include_lower_range_data__DO_NOT_include_upper_range_data = false; // Even though current data exists, do not include it if there is no lower data
					current__DO_NOT_include_lower_range_data__DO_include_upper_range_data = false;
					current__DO_NOT_include_lower_range_data__DO_NOT_include_upper_range_data = false;
				}
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
					added = CreateNewXRRow(first_row_added, datetime_start_col_name, datetime_end_col_name, result_columns.view_name, sql_add_xr_row, bound_parameter_strings, bound_parameter_ints, bound_parameter_which_binding_to_use, current_datetime_start, current_datetime_end, previous_x_or_final_columns_being_cleaned_over_timerange, result_columns, previous__DO_include_lower_range_data__DO_include_upper_range_data, current__DO_include_lower_range_data__DO_include_upper_range_data, xr_table_category);
					if (failed)
					{
						break;
					}
					if (added)
					{
						sql_strings.push_back(SQLExecutor(db, sql_add_xr_row, bound_parameter_strings, bound_parameter_ints, bound_parameter_which_binding_to_use, the_prepared_stmt, true));
						the_prepared_stmt = sql_strings.back().stmt;
						++current_rows_added;
						++current_rows_added_since_execution;
					}

				}
				else if (lower_range_end < upper_range_end)
				{

					// The upper range ends higher than the lower range

					// First, add a row that includes all data,
					// setting new time range columns to:
					// lower_range_start - lower_range_end
					added = CreateNewXRRow(first_row_added, datetime_start_col_name, datetime_end_col_name, result_columns.view_name, sql_add_xr_row, bound_parameter_strings, bound_parameter_ints, bound_parameter_which_binding_to_use, lower_range_start, lower_range_end, previous_x_or_final_columns_being_cleaned_over_timerange, result_columns, previous__DO_include_lower_range_data__DO_include_upper_range_data, current__DO_include_lower_range_data__DO_include_upper_range_data, xr_table_category);
					if (failed)
					{
						break;
					}
					if (added)
					{
						sql_strings.push_back(SQLExecutor(db, sql_add_xr_row,bound_parameter_strings, bound_parameter_ints, bound_parameter_which_binding_to_use, the_prepared_stmt, true));
						the_prepared_stmt = sql_strings.back().stmt;
						++current_rows_added;
						++current_rows_added_since_execution;
					}

					// Second, add a row that includes only the upper range's data,
					// setting new time range columns to:
					// lower_range_end - upper_range_end
					if (xr_table_category != OutputModel::OutputGenerator::CHILD_VARIABLE_GROUP || !previous_is_lower)
					{
						added = CreateNewXRRow(first_row_added, datetime_start_col_name, datetime_end_col_name, result_columns.view_name, sql_add_xr_row, bound_parameter_strings, bound_parameter_ints, bound_parameter_which_binding_to_use, lower_range_end, upper_range_end, previous_x_or_final_columns_being_cleaned_over_timerange, result_columns, previous__DO_NOT_include_lower_range_data__DO_include_upper_range_data, current__DO_NOT_include_lower_range_data__DO_include_upper_range_data, xr_table_category);
						if (failed)
						{
							break;
						}
						if (added)
						{
							sql_strings.push_back(SQLExecutor(db, sql_add_xr_row, bound_parameter_strings, bound_parameter_ints, bound_parameter_which_binding_to_use, the_prepared_stmt, true));
							the_prepared_stmt = sql_strings.back().stmt;
							++current_rows_added;
							++current_rows_added_since_execution;
						}
					}

				}
				else
				{

					// The lower range ends higher than the upper range

					// First, add a row that includes all data,
					// setting new time range columns to:
					// upper_range_start - upper_range_end
					added = CreateNewXRRow(first_row_added, datetime_start_col_name, datetime_end_col_name, result_columns.view_name, sql_add_xr_row, bound_parameter_strings, bound_parameter_ints, bound_parameter_which_binding_to_use, upper_range_start, upper_range_end, previous_x_or_final_columns_being_cleaned_over_timerange, result_columns, previous__DO_include_lower_range_data__DO_include_upper_range_data, current__DO_include_lower_range_data__DO_include_upper_range_data, xr_table_category);
					if (failed)
					{
						break;
					}
					if (added)
					{
						sql_strings.push_back(SQLExecutor(db, sql_add_xr_row, bound_parameter_strings, bound_parameter_ints, bound_parameter_which_binding_to_use, the_prepared_stmt, true));
						the_prepared_stmt = sql_strings.back().stmt;
						++current_rows_added;
						++current_rows_added_since_execution;
					}


					// Second, add a row that includes only the lower range's data,
					// setting new time range columns to:
					// upper_range_end - lower_range_end
					if (xr_table_category != OutputModel::OutputGenerator::CHILD_VARIABLE_GROUP || previous_is_lower)
					{
						added = CreateNewXRRow(first_row_added, datetime_start_col_name, datetime_end_col_name, result_columns.view_name, sql_add_xr_row, bound_parameter_strings, bound_parameter_ints, bound_parameter_which_binding_to_use, upper_range_end, lower_range_end, previous_x_or_final_columns_being_cleaned_over_timerange, result_columns, previous__DO_include_lower_range_data__DO_NOT_include_upper_range_data, current__DO_include_lower_range_data__DO_NOT_include_upper_range_data, xr_table_category);
						if (failed)
						{
							break;
						}
						if (added)
						{
							sql_strings.push_back(SQLExecutor(db, sql_add_xr_row, bound_parameter_strings, bound_parameter_ints, bound_parameter_which_binding_to_use, the_prepared_stmt, true));
							the_prepared_stmt = sql_strings.back().stmt;
							++current_rows_added;
							++current_rows_added_since_execution;
						}
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
					if (xr_table_category != OutputModel::OutputGenerator::CHILD_VARIABLE_GROUP || previous_is_lower)
					{
						added = CreateNewXRRow(first_row_added, datetime_start_col_name, datetime_end_col_name, result_columns.view_name, sql_add_xr_row, bound_parameter_strings, bound_parameter_ints, bound_parameter_which_binding_to_use, lower_range_start, lower_range_end, previous_x_or_final_columns_being_cleaned_over_timerange, result_columns, previous__DO_include_lower_range_data__DO_NOT_include_upper_range_data, current__DO_include_lower_range_data__DO_NOT_include_upper_range_data, xr_table_category);
						if (failed)
						{
							break;
						}
						if (added)
						{
							sql_strings.push_back(SQLExecutor(db, sql_add_xr_row, bound_parameter_strings, bound_parameter_ints, bound_parameter_which_binding_to_use, the_prepared_stmt, true));
							the_prepared_stmt = sql_strings.back().stmt;
							++current_rows_added;
							++current_rows_added_since_execution;
						}
					}

					// Second, add a row corresponding to the upper range,
					// setting new time range columns to:
					// upper_range_start - upper_range_end
					if (xr_table_category != OutputModel::OutputGenerator::CHILD_VARIABLE_GROUP || !previous_is_lower)
					{
						added = CreateNewXRRow(first_row_added, datetime_start_col_name, datetime_end_col_name, result_columns.view_name, sql_add_xr_row, bound_parameter_strings, bound_parameter_ints, bound_parameter_which_binding_to_use, upper_range_start, upper_range_end, previous_x_or_final_columns_being_cleaned_over_timerange, result_columns, previous__DO_NOT_include_lower_range_data__DO_include_upper_range_data, current__DO_NOT_include_lower_range_data__DO_include_upper_range_data, xr_table_category);
						if (failed)
						{
							break;
						}
						if (added)
						{
							sql_strings.push_back(SQLExecutor(db, sql_add_xr_row, bound_parameter_strings, bound_parameter_ints, bound_parameter_which_binding_to_use, the_prepared_stmt, true));
							the_prepared_stmt = sql_strings.back().stmt;
							++current_rows_added;
							++current_rows_added_since_execution;
						}
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
					if (xr_table_category != OutputModel::OutputGenerator::CHILD_VARIABLE_GROUP || previous_is_lower)
					{
						added = CreateNewXRRow(first_row_added, datetime_start_col_name, datetime_end_col_name, result_columns.view_name, sql_add_xr_row, bound_parameter_strings, bound_parameter_ints, bound_parameter_which_binding_to_use, lower_range_start, upper_range_start, previous_x_or_final_columns_being_cleaned_over_timerange, result_columns, previous__DO_include_lower_range_data__DO_NOT_include_upper_range_data, current__DO_include_lower_range_data__DO_NOT_include_upper_range_data, xr_table_category);
						if (failed)
						{
							break;
						}
						if (added)
						{
							sql_strings.push_back(SQLExecutor(db, sql_add_xr_row, bound_parameter_strings, bound_parameter_ints, bound_parameter_which_binding_to_use, the_prepared_stmt, true));
							the_prepared_stmt = sql_strings.back().stmt;
							++current_rows_added;
							++current_rows_added_since_execution;
						}
					}

					if (lower_range_end == upper_range_end)
					{

						// special case: The lower range and the upper range
						// end at the same time value

						// So second, add a row that covers the entire upper range
						// that includes all data,
						// therefore setting new time range columns to:
						// upper_range_start - upper_range_end
						added = CreateNewXRRow(first_row_added, datetime_start_col_name, datetime_end_col_name, result_columns.view_name, sql_add_xr_row, bound_parameter_strings, bound_parameter_ints, bound_parameter_which_binding_to_use, upper_range_start, upper_range_end, previous_x_or_final_columns_being_cleaned_over_timerange, result_columns, previous__DO_include_lower_range_data__DO_include_upper_range_data, current__DO_include_lower_range_data__DO_include_upper_range_data, xr_table_category);
						if (failed)
						{
							break;
						}
						if (added)
						{
							sql_strings.push_back(SQLExecutor(db, sql_add_xr_row, bound_parameter_strings, bound_parameter_ints, bound_parameter_which_binding_to_use, the_prepared_stmt, true));
							the_prepared_stmt = sql_strings.back().stmt;
							++current_rows_added;
							++current_rows_added_since_execution;
						}

					}
					else if (lower_range_end < upper_range_end)
					{

						// The upper range ends higher than the lower range

						// So second, add a row that includes all data,
						// setting new time range columns to:
						// upper_range_start - lower_range_end
						added = CreateNewXRRow(first_row_added, datetime_start_col_name, datetime_end_col_name, result_columns.view_name, sql_add_xr_row, bound_parameter_strings, bound_parameter_ints, bound_parameter_which_binding_to_use, upper_range_start, lower_range_end, previous_x_or_final_columns_being_cleaned_over_timerange, result_columns, previous__DO_include_lower_range_data__DO_include_upper_range_data, current__DO_include_lower_range_data__DO_include_upper_range_data, xr_table_category);
						if (failed)
						{
							break;
						}
						if (added)
						{
							sql_strings.push_back(SQLExecutor(db, sql_add_xr_row, bound_parameter_strings, bound_parameter_ints, bound_parameter_which_binding_to_use, the_prepared_stmt, true));
							the_prepared_stmt = sql_strings.back().stmt;
							++current_rows_added;
							++current_rows_added_since_execution;
						}


						// And third, add a row that includes only the upper range's data,
						// setting new time range columns to:
						// lower_range_end - upper_range_end
						if (xr_table_category != OutputModel::OutputGenerator::CHILD_VARIABLE_GROUP || !previous_is_lower)
						{
							added = CreateNewXRRow(first_row_added, datetime_start_col_name, datetime_end_col_name, result_columns.view_name, sql_add_xr_row, bound_parameter_strings, bound_parameter_ints, bound_parameter_which_binding_to_use, lower_range_end, upper_range_end, previous_x_or_final_columns_being_cleaned_over_timerange, result_columns, previous__DO_NOT_include_lower_range_data__DO_include_upper_range_data, current__DO_NOT_include_lower_range_data__DO_include_upper_range_data, xr_table_category);
							if (failed)
							{
								break;
							}
							if (added)
							{
								sql_strings.push_back(SQLExecutor(db, sql_add_xr_row, bound_parameter_strings, bound_parameter_ints, bound_parameter_which_binding_to_use, the_prepared_stmt, true));
								the_prepared_stmt = sql_strings.back().stmt;
								++current_rows_added;
								++current_rows_added_since_execution;
							}
						}


					}
					else
					{

						// The lower range ends higher than the upper range

						// So second, add a row that covers the entire upper range
						// that includes all data,
						// therefore setting new time range columns to:
						// upper_range_start - upper_range_end
						added = CreateNewXRRow(first_row_added, datetime_start_col_name, datetime_end_col_name, result_columns.view_name, sql_add_xr_row, bound_parameter_strings, bound_parameter_ints, bound_parameter_which_binding_to_use, upper_range_start, upper_range_end, previous_x_or_final_columns_being_cleaned_over_timerange, result_columns, previous__DO_include_lower_range_data__DO_include_upper_range_data, current__DO_include_lower_range_data__DO_include_upper_range_data, xr_table_category);
						if (failed)
						{
							break;
						}
						if (added)
						{
							sql_strings.push_back(SQLExecutor(db, sql_add_xr_row, bound_parameter_strings, bound_parameter_ints, bound_parameter_which_binding_to_use, the_prepared_stmt, true));
							the_prepared_stmt = sql_strings.back().stmt;
							++current_rows_added;
							++current_rows_added_since_execution;
						}


						// And third, add a row that includes only the lower range's data,
						// setting new time range columns to:
						// upper_range_end - lower_range_end
						if (xr_table_category != OutputModel::OutputGenerator::CHILD_VARIABLE_GROUP || previous_is_lower)
						{
							added = CreateNewXRRow(first_row_added, datetime_start_col_name, datetime_end_col_name, result_columns.view_name, sql_add_xr_row, bound_parameter_strings, bound_parameter_ints, bound_parameter_which_binding_to_use, upper_range_end, lower_range_end, previous_x_or_final_columns_being_cleaned_over_timerange, result_columns, previous__DO_include_lower_range_data__DO_NOT_include_upper_range_data, current__DO_include_lower_range_data__DO_NOT_include_upper_range_data, xr_table_category);
							if (failed)
							{
								break;
							}
							if (added)
							{
								sql_strings.push_back(SQLExecutor(db, sql_add_xr_row, bound_parameter_strings, bound_parameter_ints, bound_parameter_which_binding_to_use, the_prepared_stmt, true));
								the_prepared_stmt = sql_strings.back().stmt;
								++current_rows_added;
								++current_rows_added_since_execution;
							}
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
			CheckProgressUpdate(current_rows_added, rows_estimated, saved_initial_progress_bar_value);
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

	messager.UpdateProgressBarValue(1000);

	rows_estimated = current_rows_added;

	return result;

}

void OutputModel::OutputGenerator::Prepare()
{

	// If we ever switch to using the SQLite "temp" mechanism, utilize temp_dot
	//temp_dot = "temp.";
	temp_dot = "";

	input_model = &model->getInputModel();

	db = input_model->getDb();

	executor.db = db;

	initialized = true;

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
		boost::format msg("The number of datetime columns in the raw data tables in the database is not either 0 or 2 (table %1%)");
		msg % vg_data_table_name;
		SetFailureMessage(msg.str());
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

	std::for_each(variables_in_group_sorted.cbegin(), variables_in_group_sorted.cend(), [this, &vg_data_table_name, &dmu_counts_corresponding_to_top_level_uoa, &dmu_counts_corresponding_to_uoa_for_current_primary_or_child_variable_group, &is_primary, &columns_in_variable_group_view, &datetime_columns, &the_variable_group, &variables_in_group_primary_keys_metadata](WidgetInstanceIdentifier const & variable_group_set_member)
	{
		columns_in_variable_group_view.columns_in_view.push_back(ColumnsInTempView::ColumnInTempView());
		ColumnsInTempView::ColumnInTempView & column_in_variable_group_data_table = columns_in_variable_group_view.columns_in_view.back();

		std::string column_name_no_uuid = *variable_group_set_member.code;
		column_in_variable_group_data_table.column_name_in_temporary_table_no_uuid = column_name_no_uuid;

		std::string column_name = column_name_no_uuid;

		column_in_variable_group_data_table.column_name_in_temporary_table = column_name;

		column_in_variable_group_data_table.column_name_in_original_data_table = column_name_no_uuid;

		column_in_variable_group_data_table.variable_group_associated_with_current_inner_table = the_variable_group.first;

		column_in_variable_group_data_table.current_multiplicity__of__current_inner_table__within__current_vg = 1;

		if (!the_variable_group.first.identifier_parent)
		{
			boost::format msg("There is no unit of analysis that can be retrieved for table %1% while attempting to retrieve raw data from this table.");
			msg % vg_data_table_name;
			SetFailureMessage(msg.str());
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
								column_in_variable_group_data_table.current_multiplicity__corresponding_to__current_inner_table___is_1_in_all_inner_tables_when_multiplicity_is_1_for_that_vg = current_variable_group_primary_key_entry.current_multiplicity;
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
				boost::format msg("Unable to determine DMU counts for unit of analysis %1%.");
				msg % *uoa__to__dmu_counts__pair.first.code;
				SetFailureMessage(msg.str());
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
			boost::format msg("Overlapping top-level variable groups are not yet supported.");
			SetFailureMessage(msg.str());
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
			boost::format msg("Unknown evaluation of DMU counts for top-level variable groups.");
			SetFailureMessage(msg.str());
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
			boost::format msg("DMU code (or UUID) is unknown while validating units of analysis.");
			SetFailureMessage(msg.str());
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
			if (the_dmu_category.longhand)
			{
				boost::format msg("The choice of K in the spin control for DMU %1% (\"%2%\") (%3%) is too small to support the unit/s of analysis for the variables selected, with required minimum K-value %4% for unit of analysis %5%.");
				msg % *the_dmu_category.code % *the_dmu_category.longhand % kad_count_current_dmu_category % uoa_count_current_dmu_category % *biggest_counts[0].first.code;
				SetFailureMessage(msg.str());
			}
			else
			{
				boost::format msg("The choice of K in the spin control for DMU %1% (%2%) is too small to support the unit/s of analysis for the variables selected, with required minimum K-value %3% for unit of analysis %4%.");
				msg % *the_dmu_category.code % kad_count_current_dmu_category % uoa_count_current_dmu_category % *biggest_counts[0].first.code;
				SetFailureMessage(msg.str());
			}
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
		if (multiplicity * uoa_count_current_dmu_category != kad_count_current_dmu_category)
		{
			if (the_dmu_category.longhand)
			{
				boost::format msg("The choice of K in the spin control for DMU %1% (\"%2%\") (%3%) is not an even multiple of the minimun K-value for the unit of analysis %4% (%5%).");
				msg % *the_dmu_category.code % *the_dmu_category.longhand % kad_count_current_dmu_category % *biggest_counts[0].first.code % uoa_count_current_dmu_category;
				SetFailureMessage(msg.str());
			}
			else
			{
				boost::format msg("The choice of K in the spin control for DMU %1% (%2%) is not an even multiple of the minimun K-value for the unit of analysis %3% (%4%).");
				msg % *the_dmu_category.code % kad_count_current_dmu_category % *biggest_counts[0].first.code % uoa_count_current_dmu_category;
				SetFailureMessage(msg.str());
			}
			failed = true;
			return;
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
				boost::format msg("Only a single DMU category may have its K value in the spin control set to larger than the minimum.  Future versions of NewGene may allow this.");
				SetFailureMessage(msg.str());
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
						if (current_dmu_plus_count.first.longhand)
						{
							boost::format msg("For child variable groups, the K-value within the unit of analysis for DMU category %1% (\"%2%\") may only be greater than 1 if it matches the K-value within the UOA of the top-level variable group/s.");
							msg % *current_dmu_plus_count.first.code % *current_dmu_plus_count.first.longhand;
							SetFailureMessage(msg.str());
						}
						else
						{
							boost::format msg("For child variable groups, the K-value within the unit of analysis for DMU category %1% may only be greater than 1 if it matches the K-value within the UOA of the top-level variable group/s.");
							msg % *current_dmu_plus_count.first.code;
							SetFailureMessage(msg.str());
						}
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
							if (current_dmu_plus_count.first.longhand)
							{
								boost::format msg("For child variable groups, the K-value within the unit of analysis for DMU category %1% (\"%2%\") cannot currently be 1 if the K-value within the UOA of the top-level variable group/s is greater than 1 when that DMU category does not have multiplicity greater than 1 for the top-level variable group.");
								msg % *current_dmu_plus_count.first.code % *current_dmu_plus_count.first.longhand;
								SetFailureMessage(msg.str());
							}
							else
							{
								boost::format msg("For child variable groups, the K-value within the unit of analysis for DMU category %1% cannot currently be 1 if the K-value within the UOA of the top-level variable group/s is greater than 1 when that DMU category does not have multiplicity greater than 1 for the top-level variable group.");
								msg % *current_dmu_plus_count.first.code;
								SetFailureMessage(msg.str());
							}
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

			boost::format msg("For child variable group with unit of analysis %1%, the K-value for more than one DMU category is less than the corresponding K-values within the UOA of the top-level variable group/s.  This is currently not supported by NewGene.");
			msg % *uoa__to__dmu_counts__pair.first.code;
			SetFailureMessage(msg.str());
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
			boost::format msg("Unit of analysis %1% does not have a UUID.  (Error while populating metadata for units of analysis.)");
			msg % *uoa.code;
			SetFailureMessage(msg.str());
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
				boost::format msg("Unknown DMU identifier while obtaining metadata for child variable groups.");
				SetFailureMessage(msg.str());
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
				if (the_dmu_category_identifier.longhand)
				{
					boost::format msg("The choice of K in the spin control for DMU %1% (\"%2%\") (%3%) is too small to support the unit/s of analysis for the variables selected, with required minimum K-value %4% for unit of analysis %5%.");
					msg % *the_dmu_category_identifier.code % *the_dmu_category_identifier.longhand % k_spin_control_count__current_dmu_category % uoa_k_count__current_dmu_category % *uoa_identifier.code;
					SetFailureMessage(msg.str());
				}
				else
				{
					boost::format msg("The choice of K in the spin control for DMU %1% (%2%) is too small to support the unit/s of analysis for the variables selected, with required minimum K-value %3% for unit of analysis %4%.");
					msg % *the_dmu_category_identifier.code % k_spin_control_count__current_dmu_category % uoa_k_count__current_dmu_category % *uoa_identifier.code;
					SetFailureMessage(msg.str());
				}
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
			boost::format msg("Unknown DMU identifier while populating primary key sequence metadata.");
			SetFailureMessage(msg.str());
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
					boost::format msg("Unknown variable group identifier while populating primary key sequence metadata.");
					SetFailureMessage(msg.str());
					failed = true;
					return;
				}

				if (!the_variable_group.first.identifier_parent)
				{
					// Todo: error message
					boost::format msg("Unknown unit of analysis identifier while populating primary key sequence metadata.");
					SetFailureMessage(msg.str());
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
					boost::format msg("Global data tables - i.e., tables of raw data with no DMU category primary key - are not yet supported in NewGene.");
					SetFailureMessage(msg.str());
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
									boost::format msg("Unable to locate variable \"%1%\" (%2%) while populating primary key sequence metadata.");
									msg % *current_variable_group__current_dmu_primary_key_instance.longhand % *the_variable_group.first.uuid;
									SetFailureMessage(msg.str());
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
					boost::format msg("Unknown variable group identifier while populating primary key sequence metadata.");
					SetFailureMessage(msg.str());
					failed = true;
					return;
				}

				if (!the_variable_group.first.identifier_parent)
				{
					// Todo: error message
					boost::format msg("Unknown unit of analysis identifier while populating primary key sequence metadata.");
					SetFailureMessage(msg.str());
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
					boost::format msg("Global data tables - i.e., tables of raw data with no DMU category primary key - are not yet supported in NewGene.");
					SetFailureMessage(msg.str());
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
									boost::format msg("Unable to locate variable \"%1%\" (%2%) while populating primary key sequence metadata.");
									msg % *current_variable_group_current_dmu_primary_key.longhand % *the_variable_group.first.uuid;
									SetFailureMessage(msg.str());
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

void OutputModel::OutputGenerator::RemoveDuplicatesFromPrimaryKeyMatches( SqlAndColumnSet & result, std::deque<SavedRowData> &rows_to_sort, std::deque<SavedRowData> &incoming_rows_of_data, std::deque<SavedRowData> &outgoing_rows_of_data, std::deque<SavedRowData> &intermediate_rows_of_data, std::string datetime_start_col_name, std::string datetime_end_col_name, sqlite3_stmt * the_prepared_stmt, std::vector<SQLExecutor> & sql_strings, ColumnsInTempView &result_columns, ColumnsInTempView & sorted_result_columns, std::int64_t & current_rows_added, int & current_rows_added_since_execution, std::string sql_add_xr_row, bool first_row_added, std::vector<std::string> bound_parameter_strings, std::vector<std::int64_t> bound_parameter_ints, std::vector<SQLExecutor::WHICH_BINDING> bound_parameter_which_binding_to_use, int const minimum_desired_rows_per_transaction )
{
	SavedRowData current_row_of_data;

	// perform sort here of rows in rows_to_sort ONLY on time columns
	std::sort(rows_to_sort.begin(), rows_to_sort.end());

	while (!rows_to_sort.empty())
	{

		current_row_of_data = rows_to_sort.front();
		rows_to_sort.pop_front();

		// If we're starting fresh, just add the current row of input to incoming_rows_of_data
		// and proceed to the next row of input.
		if (incoming_rows_of_data.empty())
		{
			incoming_rows_of_data.push_back(current_row_of_data);
			continue;
		}

		// If the current row of input starts past
		// the end of any of the saved rows, then
		// there can be no overlap with these rows, and they are done.
		// Move them to outgoing_rows_of_data.
		while(!incoming_rows_of_data.empty())
		{
			SavedRowData & first_incoming_row = incoming_rows_of_data.front();
			if (first_incoming_row.datetime_end <= current_row_of_data.datetime_start)
			{
				outgoing_rows_of_data.push_back(first_incoming_row);
				incoming_rows_of_data.pop_front();
			}
			else
			{
				break;
			}
		}

		// There is guaranteed to either:
		// (1) be overlap of the current row of input with the first saved row,
		// (2) have the current row be completely beyond the end of all saved rows
		// (or, falling into the same category,
		// the current row's starting datetime is exactly equal to the ending datetime
		// of the last of the saved rows)
		bool current_row_complete = false;
		while (!current_row_complete)
		{
			if (incoming_rows_of_data.empty())
			{
				break;
			}
			SavedRowData & first_incoming_row = incoming_rows_of_data.front();
			current_row_complete = ProcessCurrentDataRowOverlapWithFrontSavedRow(first_incoming_row, current_row_of_data, intermediate_rows_of_data, current_rows_added);
			if (failed)
			{
				return;
			}
			incoming_rows_of_data.pop_front();
		}

		if (!current_row_complete)
		{
			intermediate_rows_of_data.push_back(current_row_of_data);
		}

		incoming_rows_of_data.insert(incoming_rows_of_data.cbegin(), intermediate_rows_of_data.cbegin(), intermediate_rows_of_data.cend());
		intermediate_rows_of_data.clear();

	}

	outgoing_rows_of_data.insert(outgoing_rows_of_data.cend(), incoming_rows_of_data.cbegin(), incoming_rows_of_data.cend());
	WriteRowsToFinalTable(outgoing_rows_of_data, datetime_start_col_name, datetime_end_col_name, the_prepared_stmt, sql_strings, db, result_columns.view_name, sorted_result_columns, current_rows_added, current_rows_added_since_execution, sql_add_xr_row, first_row_added, bound_parameter_strings, bound_parameter_ints, bound_parameter_which_binding_to_use);
	if (failed)
	{
		return;
	}
	incoming_rows_of_data.clear();
	outgoing_rows_of_data.clear();

	if (current_rows_added_since_execution >= minimum_desired_rows_per_transaction)
	{
		ExecuteSQL(result);
		EndTransaction();
		BeginNewTransaction();
		current_rows_added_since_execution = 0;
	}
}

bool OutputModel::OutputGenerator::ColumnSorter::operator<(OutputModel::OutputGenerator::ColumnSorter const & rhs) const
{
	int index = 0;
	bool answered = false;
	bool is_less_than = false;
	std::for_each(bindings__primary_keys_with_multiplicity_greater_than_1.cbegin(), bindings__primary_keys_with_multiplicity_greater_than_1.cend(), [this, &index, &rhs, &answered, &is_less_than](std::pair<SQLExecutor::WHICH_BINDING, int> const & lhs_binding)
	{
		if (answered)
		{
			return;
		}
		std::pair<SQLExecutor::WHICH_BINDING, int> rhs_binding = rhs.bindings__primary_keys_with_multiplicity_greater_than_1[index];
		std::int64_t rhs_value_int = 0;
		std::string rhs_value_string;
		switch (rhs_binding.first)
		{
			case OutputModel::OutputGenerator::SQLExecutor::INT64:
				{
					rhs_value_int = rhs.ints[rhs_binding.second];
				}
				break;
			case OutputModel::OutputGenerator::SQLExecutor::STRING:
				{
					rhs_value_string = rhs.strings[rhs_binding.second];
				}
				break;
			case OutputModel::OutputGenerator::SQLExecutor::NULL_BINDING:
				{

				}
				break;
		}
		switch (lhs_binding.first)
		{
			case OutputModel::OutputGenerator::SQLExecutor::INT64:
				{
					std::int64_t lhs_value_int = ints[lhs_binding.second];
					switch (rhs_binding.first)
					{
						case OutputModel::OutputGenerator::SQLExecutor::INT64:
							{
								if (lhs_value_int < rhs_value_int)
								{
									answered = true;
									is_less_than = true;
								}
								else if (lhs_value_int > rhs_value_int)
								{
									answered = true;
									is_less_than = false;
								}
								else
								{
									is_less_than = false;
								}
							}
							break;
						case OutputModel::OutputGenerator::SQLExecutor::STRING:
							{
								if (lhs_value_int < boost::lexical_cast<std::int64_t>(rhs_value_string))
								{
									answered = true;
									is_less_than = true;
								}
								else if (lhs_value_int > boost::lexical_cast<std::int64_t>(rhs_value_string))
								{
									answered = true;
									is_less_than = false;
								}
								else
								{
									is_less_than = false;
								}
							}
							break;
						case OutputModel::OutputGenerator::SQLExecutor::NULL_BINDING:
							{
								// NULL is greater than anything
								answered = true;
								is_less_than = true;
							}
							break;
					}
				}
				break;
			case OutputModel::OutputGenerator::SQLExecutor::STRING:
				{
					std::string lhs_value_string = strings[lhs_binding.second];
					switch (rhs_binding.first)
					{
						case OutputModel::OutputGenerator::SQLExecutor::INT64:
							{
								if (boost::lexical_cast<std::int64_t>(lhs_value_string) < rhs_value_int)
								{
									answered = true;
									is_less_than = true;
								}
								else if (boost::lexical_cast<std::int64_t>(lhs_value_string) > rhs_value_int)
								{
									answered = true;
									is_less_than = false;
								}
								else
								{
									is_less_than = false;
								}
							}
							break;
						case OutputModel::OutputGenerator::SQLExecutor::STRING:
							{
								if (lhs_value_string < rhs_value_string)
								{
									answered = true;
									is_less_than = true;
								}
								else if (lhs_value_string > rhs_value_string)
								{
									answered = true;
									is_less_than = false;
								}
								else
								{
									is_less_than = false;
								}
							}
							break;
						case OutputModel::OutputGenerator::SQLExecutor::NULL_BINDING:
							{
								// NULL is greater than anything
								answered = true;
								is_less_than = true;
							}
							break;
					}
				}
				break;
			case OutputModel::OutputGenerator::SQLExecutor::NULL_BINDING:
				{
					switch (rhs_binding.first)
					{
						case OutputModel::OutputGenerator::SQLExecutor::INT64:
							{
								// NULL is greater than anything
								answered = true;
								is_less_than = false;
							}
							break;
						case OutputModel::OutputGenerator::SQLExecutor::STRING:
							{
								// NULL is greater than anything
								answered = true;
								is_less_than = false;
							}
							break;
						case OutputModel::OutputGenerator::SQLExecutor::NULL_BINDING:
							{
								is_less_than = false;
							}
							break;
					}
				}
				break;
		}
		++index;
	});
	return is_less_than;
}

bool OutputModel::OutputGenerator::SavedRowData::operator<(SavedRowData const & rhs) const
{
	// special-purpose sort.
	// In the algorithm, the only time this sort is called is when
	// the SavedRowData instances being compared match on all primary key columns
	// (perhaps with NULLs).
	// This sort routine just sorts by the timestamps.

	if (datetime_start < rhs.datetime_start)
	{
		return true;
	}

	if (datetime_start > rhs.datetime_start)
	{
		return false;
	}

	if (datetime_end < rhs.datetime_end)
	{
		return true;
	}

	if (datetime_end > rhs.datetime_end)
	{
		return false;
	}

	return false;
}

void OutputModel::OutputGenerator::ClearTables(SqlAndColumnSets const & tables_to_clear)
{
	std::for_each(tables_to_clear.cbegin(), tables_to_clear.cend(), [this](SqlAndColumnSet const & table_to_clear)
	{
		// Drop tables only if the "make_table_permanent" flag is not set
		if (!table_to_clear.second.make_table_permanent)
		{
			ClearTable(table_to_clear);
		}
	});
}

void OutputModel::OutputGenerator::ClearTable(SqlAndColumnSet const & table_to_clear)
{
	// Drop tables only if the "make_table_permanent" flag is not set
	if (!table_to_clear.second.make_table_permanent)
	{
		std::string table_name_to_clear = table_to_clear.second.view_name;

		if (false)
		{
			// The method below is the established way to determine whether the table exists or not,
			// but is not necessary
			std::string sql_string;
			sql_string += "SELECT name FROM sqlite_master WHERE type='table' and name='";
			sql_string += table_name_to_clear;
			sql_string += "'";

			SQLExecutor table_remover(input_model->getDb(), sql_string);
			table_remover.statement_type = OutputModel::OutputGenerator::SQLExecutor::RETURNS_ROWS;
			table_remover.Execute();
			bool row_exists = table_remover.Step();

			// If there is a row in the result set, then the table exists
			if (!row_exists)
			{
				return;
			}
		}

		std::string sql_string = "DROP TABLE IF EXISTS ";
		sql_string += table_name_to_clear;

		SQLExecutor table_remover(input_model->getDb(), sql_string);
		table_remover.Execute();
	}
}

std::string OutputModel::OutputGenerator::CheckOutputFileExists()
{

	OutputProjectPathToKadOutputFile * setting_path_to_kad_output = nullptr;

	std::unique_ptr<BackendProjectOutputSetting> & path_to_kad_output = project.projectSettings().GetSetting(messager, OUTPUT_PROJECT_SETTINGS_BACKEND_NAMESPACE::PATH_TO_KAD_OUTPUT_FILE);
	bool bad = false;
	try
	{
		setting_path_to_kad_output = dynamic_cast<OutputProjectPathToKadOutputFile*>(path_to_kad_output.get());
	}
	catch (std::bad_cast &)
	{
		bad = true;
	}

	bool output_file_exists = boost::filesystem::exists(setting_path_to_kad_output->ToString());
	if (output_file_exists)
	{
		if (!overwrite_if_output_file_already_exists)
		{
			boost::format overwrite_msg("The file %1% does already exists.  Overwrite this file?");
			overwrite_msg % setting_path_to_kad_output->ToString();
			bool overwrite_file = messager.ShowQuestionMessageBox("Overwrite file?", overwrite_msg.str());
			if (!overwrite_file)
			{
				return std::string();
			}
			overwrite_if_output_file_already_exists = true;
		}
	}

	return setting_path_to_kad_output->ToString();

}

void OutputModel::OutputGenerator::SetFailureMessage(std::string const & failure_message_)
{
	failure_message = failure_message_;
	messager.AppendKadStatusText(failure_message);
	messager.UpdateStatusBarText(failure_message);
}

void OutputModel::OutputGenerator::UpdateProgressBarToNextStage()
{
	boost::format msg_1("Generating output to file %1%: Stage %2% of %3%");
	msg_1 % boost::filesystem::path(setting_path_to_kad_output).filename() % ++current_progress_stage % total_progress_stages;
	messager.UpdateStatusBarText(msg_1.str().c_str());

	boost::format msg_1("Stage %1% of %2%");
	msg_1 % ++current_progress_stage % total_progress_stages;
	messager.AppendKadStatusText(msg_1.str().c_str());

	messager.UpdateProgressBarValue(1000);
}

void OutputModel::OutputGenerator::CheckProgressUpdate(std::int64_t const current_rows_added_, std::int64_t const rows_estimate_, std::int64_t const starting_value_this_stage)
{
	std::int64_t fraction = current_rows_added_ / rows_estimate_;
	std::int64_t increment_value = fraction * progress_increment_per_stage;
	std::int64_t max_value_this_stage = starting_value_this_stage + progress_increment_per_stage;
	std::int64_t desired_current_value = starting_value_this_stage + increment_value;
	if (desired_current_value > current_progress_value)
	{
		if (desired_current_value <= max_value_this_stage)
		{
			// no-op - each stage resets to 0
		}
	}
	desired_current_value = fraction * 1000;
	if (desired_current_value > current_progress_value)
	{
		messager.UpdateProgressBarValue(desired_current_value);
	}
}
