#include "OutputModel.h"
#include "../Utilities/UUID.h"

#include "../Settings/OutputProjectSettings_list.h"
#include <cstdint>

#ifndef Q_MOC_RUN
#	include <boost/lexical_cast.hpp>
#	include <deque>
#	include <boost/filesystem.hpp>
#	include <boost/format.hpp>
#	include <boost/scope_exit.hpp>
#	include <boost/date_time/local_time/local_time.hpp>
#endif

#include <fstream>
#include <algorithm>

#include "OutputModelDdlSql.h"


// Table name SQL tokens
// (to search for quotes around table names)
// CREATE
// UPDATE
// INSERT
// JOIN
// FROM
// ALTER

// Field name SQL tokens
// (to search for back ticks around field names)
// ON
// WHERE
// SELECT
// SET
// ORDER
// GROUP
// AS



std::recursive_mutex OutputModel::OutputGenerator::is_generating_output_mutex;
std::atomic<bool> OutputModel::OutputGenerator::is_generating_output(false);
bool OutputModel::OutputGenerator::cancelled = false;

int OutputModel::OutputGenerator::SQLExecutor::number_statement_prepares = 0;
int OutputModel::OutputGenerator::SQLExecutor::number_statement_finalizes = 0;
int OutputModel::OutputGenerator::number_transaction_begins = 0;
int OutputModel::OutputGenerator::number_transaction_ends = 0;

std::set<std::vector<std::int64_t>> OutputModel::OutputGenerator::TimeRangeMapper_Ints::test_set;
std::set<std::vector<long double>> OutputModel::OutputGenerator::TimeRangeMapper_Floats::test_set;
std::set<std::vector<std::string>> OutputModel::OutputGenerator::TimeRangeMapper_Strings::test_set;

std::string OutputModel::GetCreationSQL()
{
	return OutputModelDDLSQL();
}

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
		t_general_options.Load(db, this, input_model.get());
	}

}

bool OutputModelImportTableFn(Importer * importer, Model_basemost * model_, ImportDefinition & import_definition, Table_basemost * table_, DataBlock const & table_block,
							  int const number_rows, long & linenum, long & badwritelines, std::vector<std::string> & errors)
{
	int number_errors_at_start = errors.size();
	std::string errorMsg;

	try
	{
		if (table_->table_model_type == Table_basemost::TABLE_MODEL_TYPE__OUTPUT_MODEL)
		{

			OutputModel * output_model = dynamic_cast<OutputModel *>(model_);

			if (!output_model)
			{
				boost::format msg("Bad output model in InputModelImportTableFn");
				errorMsg = msg.str();
				errors.push_back(errorMsg);
				errorMsg.clear();
				return false;
			}

			if (output_model->getDb() == nullptr)
			{
				boost::format msg("Bad output model db in InputModelImportTableFn");
				errors.push_back(errorMsg);
				errorMsg.clear();
				return false;
			}

			switch (importer->mode)
			{

				case Importer::INSERT_IN_BULK:
					{
						table_->ImportBlockBulk(output_model->getDb(), import_definition, output_model, &output_model->getInputModel(), table_block, number_rows, linenum, badwritelines, errors);
						int number_errors_now = errors.size();

						if (number_errors_now > number_errors_at_start)
						{
							return false;
						}
					}
					break;

				case Importer::INSERT_OR_UPDATE:
					{
						long numlinesupdated = 0;
						table_->ImportBlockUpdate(output_model->getDb(), import_definition, output_model, &output_model->getInputModel(), table_block, number_rows, linenum, badwritelines, numlinesupdated,
												  errors);
						int number_errors_now = errors.size();

						if (number_errors_now > number_errors_at_start)
						{
							return false;
						}
					}
					break;

				default:
					{
						boost::format msg("Incorrect import mode attempting to call block update function.");
						errorMsg = msg.str();
						errors.push_back(errorMsg);
						errorMsg.clear();
					}
					break;

			}

		}
		else
		{
			boost::format msg("Incorrect model type in block update function.");
			errorMsg = msg.str();
			errors.push_back(errorMsg);
			errorMsg.clear();
			return false;
		}
	}
	catch (std::bad_cast &)
	{
		boost::format msg("Unable to cast to output model in block update function.");
		errorMsg = msg.str();
		errors.push_back(errorMsg);
		errorMsg.clear();
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
	, at_least_one_variable_group_has_timerange(false)
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
	, delete_tables(true)
	, debug_ordering(false)
	, ms_elapsed(0)
	, current_number_rows_to_sort(0)
	, remove_self_kads(true)
	, merge_adjacent_rows_with_identical_data_on_secondary_keys(true)
	, random_sampling_number_rows(1)
	, random_sampling(false)
	, random_sampling_old(false)
	, top_level_vg_index(0)
	, K(0)
{
	//debug_ordering = true;
	//delete_tables = false;
	//merge_adjacent_rows_with_identical_data_on_secondary_keys = false;
	//random_sampling = true;
	//random_sampling_number_rows = 1000000;
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

	std::for_each(primary_variable_group_column_sets.begin(), primary_variable_group_column_sets.end(), [this](SqlAndColumnSets & sql_and_column_sets)
	{

		std::for_each(sql_and_column_sets.begin(), sql_and_column_sets.end(), [this](SqlAndColumnSet & sql_and_column_set)
		{

			std::for_each(sql_and_column_set.first.begin(), sql_and_column_set.first.end(), [this](SQLExecutor & sql_executor)
			{

				sql_executor.Empty();

			});

		});

	});

	std::for_each(primary_group_final_results.begin(), primary_group_final_results.end(), [this](SqlAndColumnSet & sql_and_column_set)
	{

		std::for_each(sql_and_column_set.first.begin(), sql_and_column_set.first.end(), [this](SQLExecutor & sql_executor)
		{

			sql_executor.Empty();

		});

	});

	std::for_each(intermediate_merging_of_primary_groups_column_sets.begin(), intermediate_merging_of_primary_groups_column_sets.end(), [this](SqlAndColumnSet & sql_and_column_set)
	{

		std::for_each(sql_and_column_set.first.begin(), sql_and_column_set.first.end(), [this](SQLExecutor & sql_executor)
		{

			sql_executor.Empty();

		});

	});

	std::for_each(all_merged_results_unformatted.first.begin(), all_merged_results_unformatted.first.end(), [this](SQLExecutor & sql_executor)
	{

		sql_executor.Empty();

	});

	std::for_each(child_merge_final_result.first.begin(), child_merge_final_result.first.end(), [this](SQLExecutor & sql_executor)
	{

		sql_executor.Empty();

	});

	std::for_each(final_result.first.begin(), final_result.first.end(), [this](SQLExecutor & sql_executor)
	{

		sql_executor.Empty();

	});

}

void OutputModel::OutputGenerator::GenerateOutput(DataChangeMessage & change_response)
{

	cancelled = false;
	done = false;

	{
		std::lock_guard<std::recursive_mutex> guard(is_generating_output_mutex);

		if (is_generating_output)
		{
			messager.ShowMessageBox("Another K-ad output generation operation is in progress.  Please wait for that operation to complete first.");
			return;
		}

		is_generating_output = true;
	}

	BOOST_SCOPE_EXIT(&is_generating_output, &is_generating_output_mutex, &messager)
	{
		is_generating_output = false;
		messager.SetPerformanceLabel("");
	} BOOST_SCOPE_EXIT_END

	messager.AppendKadStatusText("", nullptr); // This will clear the pane
	boost::posix_time::ptime now = boost::posix_time::second_clock::local_time();
	std::string time_start_formatted = boost::posix_time::to_simple_string(now);
	boost::format msg_start("NewGene K-ad generation: Starting run at %1%");
	msg_start % time_start_formatted;
	messager.AppendKadStatusText(msg_start.str(), nullptr);

	InputModel & input_model = model->getInputModel();

	bool delete_tables_ = delete_tables;
	BOOST_SCOPE_EXIT(&input_model, &delete_tables_)
	{
		// This is also done explicity at the end,
		// but it's better to include in both places,
		// the first (at end) so that user can benefit from status text,
		// and the second (here) in case of exit due to failure
		if (delete_tables_)
		{
			input_model.ClearRemnantTemporaryTables();
		}

		input_model.VacuumDatabase();
	} BOOST_SCOPE_EXIT_END

	Table_VARIABLES_SELECTED::UOA_To_Variables_Map the_map_ = model->t_variables_selected_identifiers.GetSelectedVariablesByUOA(model->getDb(), model, &input_model);
	the_map = &the_map_;

	if (the_map->size() == 0)
	{
		boost::format msg("No variables are selected for output.");
		SetFailureMessage(msg.str());
		failed = true;
		return;
	}

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

	// The time range controls are only accurate to 1 second
	if (timerange_end <= (timerange_start + 999))
	{
		boost::format msg("The ending value of the time range must be greater than the starting value.");
		SetFailureMessage(msg.str());
		failed = true;
		return;
	}

	setting_path_to_kad_output = CheckOutputFileExists();

	if (failed || CheckCancelled())
	{
		return;
	}

	if (setting_path_to_kad_output.empty())
	{
		return;
	}

	if (!boost::filesystem::path(setting_path_to_kad_output).has_extension())
	{
		setting_path_to_kad_output = boost::filesystem::path(setting_path_to_kad_output).replace_extension("csv").string();
	}

	debug_sql_path = setting_path_to_kad_output;
	debug_sql_path.replace_extension(".debugsql.txt");

	messager.AppendKadStatusText("Validating database...", nullptr);
	input_model.ClearRemnantTemporaryTables();

	BOOST_SCOPE_EXIT(this_)
	{
		if (this_->debug_sql_file.is_open())
		{
			this_->debug_sql_file.close();
		}
	} BOOST_SCOPE_EXIT_END

	debug_sql_file.open(debug_sql_path.string(), std::ios::out | std::ios::trunc);

	if (!debug_sql_file.is_open())
	{
		// for now - harmless
	}

	current_progress_stage = 0;
	boost::format msg_1("Generating output to file %1%");
	msg_1 % boost::filesystem::path(setting_path_to_kad_output).filename();
	messager.UpdateStatusBarText(msg_1.str().c_str(), this);

	messager.AppendKadStatusText("Beginning generation of K-ad output.", this);
	messager.AppendKadStatusText("Initializing...", this);

	Prepare();

	if (failed || CheckCancelled())
	{
		return;
	}

	messager.AppendKadStatusText("Preparing input data...", this);
	ObtainColumnInfoForRawDataTables();

	if (failed || CheckCancelled())
	{
		return;
	}

	// RANDOM_SAMPLING: Update DetermineNumberStages() to account for new random sampling algorithm
	DetermineNumberStages();

	if (failed || CheckCancelled())
	{
		return;
	}

	// RANDOM_SAMPLING: The work is all done here
	if (true) // if (random_sampling)
	{

		K = 0;
		random_sampling_schema = RandomSamplingBuildSchema(primary_variable_groups_column_info, secondary_variable_groups_column_info);
		if (failed || CheckCancelled()) return;

		AllWeightings allWeightings;

		primary_variable_group_column_sets.push_back(SqlAndColumnSets());
		SqlAndColumnSets & primary_group_column_sets = primary_variable_group_column_sets.back();

		SqlAndColumnSet selected_raw_data_table_schema = CreateTableOfSelectedVariablesFromRawData(primary_variable_groups_column_info[top_level_vg_index], top_level_vg_index);
		if (failed || CheckCancelled()) return;
		selected_raw_data_table_schema.second.most_recent_sql_statement_executed__index = -1;
		ExecuteSQL(selected_raw_data_table_schema);
		primary_group_column_sets.push_back(selected_raw_data_table_schema);

		std::vector<std::string> errorMessages;
		RandomSampling_ReadData_AddToTimeSlices(selected_raw_data_table_schema.second, top_level_vg_index, allWeightings, VARIABLE_GROUP_MERGE_MODE__PRIMARY, errorMessages);
		if (failed || CheckCancelled()) return;

		allWeightings.CalculateWeightings(K, AvgMsperUnit(primary_variable_groups_vector[top_level_vg_index].first.time_granularity));
		if (failed || CheckCancelled()) return;

		std::int64_t const samples = random_sampling_number_rows;

		// The following prepares all randomly-generated output rows
		allWeightings.PrepareRandomNumbers(samples);
		if (failed || CheckCancelled()) return;

		if (random_sampling)
		{
			allWeightings.PrepareRandomSamples(K);
		}
		else
		{
			allWeightings.PrepareFullSamples(K);
		}
		if (failed || CheckCancelled()) return;

		allWeightings.ResetBranchCaches(); // build leaf cache and empty child caches.
		if (failed || CheckCancelled()) return;

		if (false)
		{

			// This is only necessary for debugging
			// or further sorting/ordering/processing

			RandomSamplingCreateOutputTable();
			if (failed || CheckCancelled()) return;

			// *********************************************************************** //
			// The following function has incomplete logic, and should
			// be merged with RandomSamplingWriteResultsToFileOrScreen().
			// *********************************************************************** //
			RandomSamplingWriteToOutputTable(allWeightings, errorMessages);
			if (failed || CheckCancelled()) return;

		}

		final_result = random_sampling_schema;

		RandomSamplerFillDataForChildGroups(allWeightings);
		if (failed || CheckCancelled()) return;

		allWeightings.ResetBranchCaches(); // build leaf cache and empty child caches.
		if (failed || CheckCancelled()) return;

		messager.AppendKadStatusText("Writing results to disk...", this);
		messager.SetPerformanceLabel("Writing results to disk...");
		RandomSamplingWriteResultsToFileOrScreen(allWeightings);

	}
	else
	{

		messager.AppendKadStatusText("Looping through top-level variable groups...", this);
		LoopThroughPrimaryVariableGroups();

		if (failed || CheckCancelled())
		{
			return;
		}

		messager.AppendKadStatusText("Merging top-level variable groups...", this);
		MergeHighLevelGroupResults();

		if (failed || CheckCancelled())
		{
			return;
		}

		messager.AppendKadStatusText("Merging child variable groups...", this);
		MergeChildGroups();

		if (failed || CheckCancelled())
		{
			return;
		}

		messager.AppendKadStatusText("Construction of final K-ad results...", this);
		all_merged_results_unformatted = CreateKadResultSet(child_merge_final_result.second);

		messager.AppendKadStatusText("Formatting results...", this);
		messager.SetPerformanceLabel("Formatting results...");
		FormatResultsForOutput();

		if (failed || CheckCancelled())
		{
			return;
		}

		messager.AppendKadStatusText("Writing results to disk...", this);
		messager.SetPerformanceLabel("Writing results to disk...");
		WriteResultsToFileOrScreen();

		if (failed || CheckCancelled())
		{
			return;
		}

		ClearTable(final_result);

	}

	messager.AppendKadStatusText("Vacuuming and defragmenting database...", this);
	messager.SetPerformanceLabel("Vacuuming and defragmenting database...");

	if (delete_tables)
	{
		input_model.ClearRemnantTemporaryTables();
	}

	input_model.VacuumDatabase();

	messager.UpdateProgressBarValue(1000);

	boost::format msg_2("Output successfully generated (%1%)");
	msg_2 % boost::filesystem::path(setting_path_to_kad_output).filename();
	messager.UpdateStatusBarText(msg_2.str().c_str(), this);
	messager.AppendKadStatusText("Done.", this);

	boost::format msg1("Number transactions begun: %1%");
	msg1 % OutputModel::OutputGenerator::number_transaction_begins;
	boost::format msg2("Number transactions ended: %1%");
	msg2 % OutputModel::OutputGenerator::number_transaction_ends;
	boost::format msg3("Number statements prepared: %1%");
	msg3 % OutputModel::OutputGenerator::SQLExecutor::number_statement_prepares;
	boost::format msg4("Number statements finalized: %1%");
	msg4 % OutputModel::OutputGenerator::SQLExecutor::number_statement_finalizes;

	if (debug_sql_file.is_open())
	{
		debug_sql_file << msg1.str() << std::endl;
		debug_sql_file << msg2.str() << std::endl;
		debug_sql_file << msg3.str() << std::endl;
		debug_sql_file << msg4.str() << std::endl;
	}

	done = true;

}

void OutputModel::OutputGenerator::MergeChildGroups()
{

	if (secondary_variable_groups_column_info.empty())
	{
		child_merge_final_result = primary_group_merged_results;
		return;
	}

	SqlAndColumnSet x_table_result = primary_group_merged_results;
	SqlAndColumnSet xr_table_result = x_table_result;
	SqlAndColumnSet duplicates_removed = xr_table_result;

	// Child tables
	int current_child_view_name_index = 1;
	int child_set_number = 1;
	std::for_each(secondary_variable_groups_column_info.cbegin(),
				  secondary_variable_groups_column_info.cend(), [this, &duplicates_removed, &current_child_view_name_index, &child_set_number, &x_table_result, &xr_table_result](
					  ColumnsInTempView const & child_variable_group_raw_data_columns)
	{

		if (failed || CheckCancelled())
		{
			return;
		}

		boost::format msg_start("Child variable group \"%1%\"");
		msg_start % (child_variable_group_raw_data_columns.variable_groups[0].longhand ? *child_variable_group_raw_data_columns.variable_groups[0].longhand
					 : child_variable_group_raw_data_columns.variable_groups[0].code ? *child_variable_group_raw_data_columns.variable_groups[0].code : std::string());
		messager.AppendKadStatusText(msg_start.str(), this);

		int const the_child_multiplicity = child_uoas__which_multiplicity_is_greater_than_1[*(child_variable_group_raw_data_columns.variable_groups[0].identifier_parent)].second;

		for (int current_multiplicity = 1; current_multiplicity <= the_child_multiplicity; ++current_multiplicity)
		{

			std::int64_t previous_count = ObtainCount(duplicates_removed.second);
			std::int64_t current_count = ObtainCount(child_variable_group_raw_data_columns);

			if (child_variable_group_raw_data_columns.variable_groups[0].longhand)
			{
				boost::format msg("Joining multiplicity %1% (%2% rows) with previous merged data for \"%3%\" (%4% rows).");
				msg % current_multiplicity % current_count % *child_variable_group_raw_data_columns.variable_groups[0].longhand % previous_count;
				messager.SetPerformanceLabel(msg.str().c_str());
			}
			else
			{
				boost::format msg("Joining multiplicity %1% (%2% rows) with previous merged data for %3% (%4% rows).");
				msg % current_multiplicity % current_count % *child_variable_group_raw_data_columns.variable_groups[0].code % previous_count;
				messager.SetPerformanceLabel(msg.str().c_str());
			}

			std::int64_t number_of_rows_previous = previous_count;
			std::int64_t number_of_rows_new = current_count;

			boost::format msg_("Multiplicity %2% - Retrieving data for child group \"%1%\": merging %3% previous with %4% new rows");
			msg_ % (child_variable_group_raw_data_columns.variable_groups[0].longhand ? *child_variable_group_raw_data_columns.variable_groups[0].longhand
					: child_variable_group_raw_data_columns.variable_groups[0].code ? *child_variable_group_raw_data_columns.variable_groups[0].code : std::string())
			% current_multiplicity % number_of_rows_previous % number_of_rows_new;
			UpdateProgressBarToNextStage(msg_.str(), "");

			messager.SetPerformanceLabel("Performing a database join; please be patient...");
			x_table_result = CreateChildXTable(child_variable_group_raw_data_columns, duplicates_removed.second, current_multiplicity, 0, child_set_number, current_child_view_name_index);
			x_table_result.second.most_recent_sql_statement_executed__index = -1;
			ExecuteSQL(x_table_result);
			ClearTable(xr_table_result);
			merging_of_children_column_sets.push_back(x_table_result);

			if (failed || CheckCancelled())
			{
				return;
			}

			messager.SetPerformanceLabel("");

			std::int64_t number_of_rows = ObtainCount(x_table_result.second);

			boost::format msg_2("Multiplicity %2% - Splitting rows on time boundaries \"%1%\": %3% rows");
			msg_2 % (child_variable_group_raw_data_columns.variable_groups[0].longhand ? *child_variable_group_raw_data_columns.variable_groups[0].longhand
					 : child_variable_group_raw_data_columns.variable_groups[0].code ? *child_variable_group_raw_data_columns.variable_groups[0].code : std::string())
			% current_multiplicity % number_of_rows;
			UpdateProgressBarToNextStage(msg_2.str(), "");

			// DETAILED_COMMENTING: To see purpose of CreateXRTable(), search for DESCRIPTION_OF_XR_ALGORITHM
			xr_table_result = CreateXRTable(x_table_result.second, current_multiplicity, 0, OutputModel::OutputGenerator::CHILD_VARIABLE_GROUP, child_set_number,
											current_child_view_name_index);
			ClearTable(x_table_result);
			merging_of_children_column_sets.push_back(xr_table_result);

			if (failed || CheckCancelled())
			{
				return;
			}

			std::string sorting_rows_text("Sorting rows");
			std::string removing_duplicates_text("Removing duplicates");
			// DETAILED_COMMENTING: To see purpose of SortAndOrRemoveDuplicates(), search for DESCRIPTION_OF_SORT_AND_OR_REMOVE_DUPLICATES_ALGORITHM
			duplicates_removed = SortAndOrRemoveDuplicates(xr_table_result.second, child_variable_group_raw_data_columns.variable_groups[0], sorting_rows_text, removing_duplicates_text,
								 current_multiplicity, child_set_number, merging_of_children_column_sets, true, OutputModel::OutputGenerator::CHILD_VARIABLE_GROUP, true);

			if (failed || CheckCancelled())
			{
				return;
			}

			++current_child_view_name_index;

		}

		++child_set_number;

	});

	child_merge_final_result = duplicates_removed;

}

void OutputModel::OutputGenerator::WriteResultsToFileOrScreen()
{

	// The final results are stored in "final_result".
	// Do an "ObtainData()" on this result, loop through,
	// and write the output to a CSV file on disk.

	std::string setting_path_to_kad_output = CheckOutputFileExists();

	if (failed || CheckCancelled())
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

	// Write columns headers
	int column_index = 0;
	bool first = true;
	std::for_each(final_result.second.columns_in_view.begin(),
				  final_result.second.columns_in_view.end(), [this, &output_file, &first, &column_index](ColumnsInTempView::ColumnInTempView & unformatted_column)
	{
		++column_index;

		if (!first)
		{
			output_file << ",";
		}

		first = false;
		output_file << unformatted_column.column_name_in_temporary_table;
	});
	output_file << std::endl;


	{

		BOOST_SCOPE_EXIT(this_)
		{
			this_->CloseObtainData();
		} BOOST_SCOPE_EXIT_END

		ObtainData(final_result.second);

		if (failed || CheckCancelled())
		{
			return;
		}

		int column_data_type = 0;
		column_index = 0;
		first = true;
		std::int64_t data_int64 = 0;
		std::string data_string;
		long double data_long = 0.0;
		std::int64_t rows_written = 0;

		while (StepData())
		{

			if (failed || CheckCancelled())
			{
				break;
			}

			column_index = 0;
			first = true;
			std::map<WidgetInstanceIdentifier, bool> variable_group_appears_more_than_once;
			std::for_each(final_result.second.columns_in_view.begin(),
						  final_result.second.columns_in_view.end(), [this, &output_file, &data_int64, &data_string, &data_long, &first, &column_index, &column_data_type](
							  ColumnsInTempView::ColumnInTempView & unformatted_column)
			{

				if (failed || CheckCancelled())
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
							data_long = sqlite3_column_double(stmt_result, column_index);

							if (!first)
							{
								output_file << ",";
							}

							first = false;
							output_file << data_long;
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

			if (failed || CheckCancelled())
			{
				return;
			}

			output_file << std::endl;

			++rows_written;

		}

		if (failed || CheckCancelled())
		{
			return;
		}

		if (output_file.good())
		{
			output_file.close();
		}

		boost::format msg("%1% rows written to output.");
		msg % rows_written;
		messager.AppendKadStatusText(msg.str(), this);

	}

}

void OutputModel::OutputGenerator::FormatResultsForOutput()
{

	// Incoming:
	// Last two columns are:
	// COLUMN_TYPE__DATETIMESTART_MERGED_KAD_OUTPUT / COLUMN_TYPE__DATETIMEEND_MERGED_KAD_OUTPUT

	char c[256];

	final_result = std::make_pair(std::vector<SQLExecutor>(), ColumnsInTempView());
	std::vector<SQLExecutor> & sql_strings = final_result.first;
	ColumnsInTempView & result_columns = final_result.second;

	std::string view_name = "NGTEMP_KAD_Results";
	result_columns.view_name_no_uuid = view_name;
	view_name += "_";
	view_name += newUUID(true);
	result_columns.view_name = view_name;

	sql_strings.push_back(SQLExecutor(this, db));
	std::string & sql_string = sql_strings.back().sql;

	sql_string = "CREATE TABLE \"";
	sql_string += result_columns.view_name;
	sql_string += "\" AS SELECT ";

	WidgetInstanceIdentifier first_variable_group;

	// Display primary key columns

	// First, display primary key columns with multiplicity of 1

	bool first = true;
	int column_index = 0;
	bool reached_end_of_first_inner_table_not_including_terminating_datetime_columns = false;
	std::string datetimestart_timestamp_colname;
	std::string datetimeend_timestamp_colname;
	std::for_each(all_merged_results_unformatted.second.columns_in_view.begin(),
				  all_merged_results_unformatted.second.columns_in_view.end(), [&c, &datetimestart_timestamp_colname, &datetimeend_timestamp_colname, &reached_end_of_first_inner_table_not_including_terminating_datetime_columns, &sql_string, &first, &first_variable_group, &result_columns, &column_index](
					  ColumnsInTempView::ColumnInTempView & unformatted_column)
	{

		if (column_index == 0)
		{
			first_variable_group = unformatted_column.variable_group_associated_with_current_inner_table;
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
				if (unformatted_column.total_outer_multiplicity__in_total_kad__for_current_dmu_category__for_current_variable_group == 1)
				{
					++column_index;
					return; // only display primary key columns of multiplicity 1 once - from the first inner table of the first primary variable group
				}
			}
		}

		if (unformatted_column.column_type == ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__PRIMARY
			&& unformatted_column.total_outer_multiplicity__in_total_kad__for_current_dmu_category__for_current_variable_group > 1)
		{
			++column_index; // We are currently displaying keys of multiplicity 1
			return;
		}

		if (unformatted_column.column_type == ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__DATETIMESTART_MERGED_KAD_OUTPUT)
		{
			datetimestart_timestamp_colname = unformatted_column.column_name_in_temporary_table;
		}

		if (unformatted_column.column_type == ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__DATETIMEEND_MERGED_KAD_OUTPUT)
		{
			datetimeend_timestamp_colname = unformatted_column.column_name_in_temporary_table;
		}

		if (unformatted_column.column_type != ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__PRIMARY
			&& unformatted_column.column_type != ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__SECONDARY)
		{
			if (unformatted_column.column_type != ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__DATETIMESTART_TEXT
				&& unformatted_column.column_type != ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__DATETIMEEND_TEXT)
			{
				reached_end_of_first_inner_table_not_including_terminating_datetime_columns = true;
				++column_index;
				return; // only display a single pair of time range columns
			}
		}

		if (unformatted_column.column_type == ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__SECONDARY)
		{
			++column_index;
			return; // display secondary keys only after primary keys
		}

		switch (unformatted_column.column_type)
		{
			case ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__DATETIMESTART_TEXT:
			case ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__DATETIMEEND_TEXT:
				{
					// only display datetime columns once
					++column_index;
					return;
				}
				break;

			default:
				break;
		}

		result_columns.columns_in_view.push_back(unformatted_column);
		ColumnsInTempView::ColumnInTempView & formatted_column = result_columns.columns_in_view.back();

		formatted_column.column_name_in_temporary_table = formatted_column.column_name_in_original_data_table;

		if (unformatted_column.total_outer_multiplicity__in_total_kad__for_current_dmu_category__for_current_variable_group > 1)
		{
			formatted_column.column_name_in_temporary_table += "_";
			formatted_column.column_name_in_temporary_table += std::to_string(
						unformatted_column.current_multiplicity__corresponding_to__current_inner_table___is_1_in_all_inner_tables_when_multiplicity_is_1_for_that_dmu_category_for_that_vg);
		}

		formatted_column.column_name_in_temporary_table_no_uuid = formatted_column.column_name_in_temporary_table;

		if (!first)
		{
			sql_string += ", ";
		}

		first = false;

		if (unformatted_column.column_type == ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__PRIMARY
			&& unformatted_column.primary_key_should_be_treated_as_integer_____float_not_allowed_as_primary_key)
		{
			sql_string += "CAST (";
		}

		sql_string += "`";
		sql_string += unformatted_column.column_name_in_temporary_table;
		sql_string += "`";

		if (unformatted_column.column_type == ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__PRIMARY
			&& unformatted_column.primary_key_should_be_treated_as_integer_____float_not_allowed_as_primary_key)
		{
			sql_string += " AS INTEGER)";
		}

		sql_string += " AS ";
		sql_string += "`";
		sql_string += formatted_column.column_name_in_temporary_table;
		sql_string += "`";

		++column_index;

	});


	// Then, display primary key columns with multiplicity greater than 1

	column_index = 0;
	reached_end_of_first_inner_table_not_including_terminating_datetime_columns = false;
	std::for_each(all_merged_results_unformatted.second.columns_in_view.begin(),
				  all_merged_results_unformatted.second.columns_in_view.end(), [this, &c, &datetimestart_timestamp_colname, &datetimeend_timestamp_colname, &reached_end_of_first_inner_table_not_including_terminating_datetime_columns, &sql_string, &first, &first_variable_group, &result_columns, &column_index](
					  ColumnsInTempView::ColumnInTempView & unformatted_column)
	{

		if (column_index == 0)
		{
			first_variable_group = unformatted_column.variable_group_associated_with_current_inner_table;
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
				if (unformatted_column.total_outer_multiplicity__in_total_kad__for_current_dmu_category__for_current_variable_group == 1)
				{
					++column_index;
					return; // only display primary key columns of multiplicity 1 once - from the first inner table of the first primary variable group
				}
			}
		}

		if (unformatted_column.column_type == ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__PRIMARY
			&& unformatted_column.total_outer_multiplicity__in_total_kad__for_current_dmu_category__for_current_variable_group == 1)
		{
			++column_index; // We are currently displaying keys of multiplicity greater than 1
			return;
		}

		if (unformatted_column.column_type != ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__PRIMARY
			&& unformatted_column.column_type != ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__SECONDARY)
		{
			if (unformatted_column.column_type != ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__DATETIMESTART_TEXT
				&& unformatted_column.column_type != ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__DATETIMEEND_TEXT)
			{
				reached_end_of_first_inner_table_not_including_terminating_datetime_columns = true;
				++column_index;
				return; // only display a single pair of time range columns
			}
		}

		if (unformatted_column.column_type == ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__SECONDARY)
		{
			++column_index;
			return; // display secondary keys only after primary keys
		}

		if (!at_least_one_variable_group_has_timerange)
		{
			// If no variable groups selected by user have a datetime associated with them,
			// do not output datetime columns
			switch (unformatted_column.column_type)
			{
				case ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__DATETIMESTART_TEXT:
				case ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__DATETIMEEND_TEXT:
					{
						++column_index;
						return;
					}
					break;

				default:
					break;
			}
		}


		// ************************************************************************************************************ //
		// DN: DATETIME_ROW_START_TODO - remove the following block, so that COLUMN_TYPE__DATETIMESTART/END_TEXT
		// passes through and is accepted,
		// if ever setting option to display DATETIME_ROW_START/END columns as variable possibilities to end user
		// ************************************************************************************************************ //
		switch (unformatted_column.column_type)
		{
			case ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__DATETIMESTART_TEXT:
			case ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__DATETIMEEND_TEXT:
				{
					++column_index;
					return;
				}
				break;

			default:
				break;
		}


		result_columns.columns_in_view.push_back(unformatted_column);
		ColumnsInTempView::ColumnInTempView & formatted_column = result_columns.columns_in_view.back();

		formatted_column.column_name_in_temporary_table = formatted_column.column_name_in_original_data_table;

		if (unformatted_column.total_outer_multiplicity__in_total_kad__for_current_dmu_category__for_current_variable_group > 1)
		{
			formatted_column.column_name_in_temporary_table += "_";
			formatted_column.column_name_in_temporary_table += std::to_string(
						unformatted_column.current_multiplicity__corresponding_to__current_inner_table___is_1_in_all_inner_tables_when_multiplicity_is_1_for_that_dmu_category_for_that_vg);
		}

		formatted_column.column_name_in_temporary_table_no_uuid = formatted_column.column_name_in_temporary_table;

		switch (unformatted_column.column_type)
		{
			case ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__DATETIMESTART_TEXT:
				{
					formatted_column.column_name_in_temporary_table = "DATETIME_START_OUTPUTROW";
				}
				break;

			case ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__DATETIMEEND_TEXT:
				{
					formatted_column.column_name_in_temporary_table = "DATETIME_END_OUTPUTROW";
				}
				break;

			default:
				break;
		}

		if (!first)
		{
			sql_string += ", ";
		}

		first = false;

		if (unformatted_column.column_type == ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__PRIMARY
			&& unformatted_column.primary_key_should_be_treated_as_integer_____float_not_allowed_as_primary_key)
		{
			sql_string += "CAST (";
		}

		sql_string += "`";
		sql_string += unformatted_column.column_name_in_temporary_table;
		sql_string += "`";

		if (unformatted_column.column_type == ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__PRIMARY
			&& unformatted_column.primary_key_should_be_treated_as_integer_____float_not_allowed_as_primary_key)
		{
			sql_string += " AS INTEGER)";
		}

		sql_string += " AS ";
		sql_string += "`";
		sql_string += formatted_column.column_name_in_temporary_table;
		sql_string += "`";

		++column_index;

	});


	// Display secondary key columns

	// First, do a quick calculation and save which secondary keys appear more than once, and stash that away
	column_index = 0;
	std::map<WidgetInstanceIdentifier, bool> variable_group_appears_more_than_once;
	std::for_each(all_merged_results_unformatted.second.columns_in_view.begin(),
				  all_merged_results_unformatted.second.columns_in_view.end(), [&c, &variable_group_appears_more_than_once, &sql_string, &first, &first_variable_group, &result_columns, &column_index](
					  ColumnsInTempView::ColumnInTempView & unformatted_column)
	{

		if (unformatted_column.column_type != ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__SECONDARY)
		{
			++column_index;
			return;
		}

		// A little trick: The highest multiplicity always appears last for any variable group,
		// so if the variable group has a multiplicity greater than 1, its highest value which appears last will set the value to true
		variable_group_appears_more_than_once[unformatted_column.variable_group_associated_with_current_inner_table] =
			(unformatted_column.current_multiplicity__of__current_inner_table__within__current_vg_inner_table_set > 1);

		++column_index;

	});

	// now display the secondary key columns themselves

	// ... first, the secondary key columns from the primary variable groups
	column_index = 0;
	std::for_each(all_merged_results_unformatted.second.columns_in_view.begin(),
				  all_merged_results_unformatted.second.columns_in_view.end(), [&c, &variable_group_appears_more_than_once, &sql_string, &first, &first_variable_group, &result_columns, &column_index](
					  ColumnsInTempView::ColumnInTempView & unformatted_column)
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
			formatted_column.column_name_in_temporary_table += std::to_string(formatted_column.current_multiplicity__of__current_inner_table__within__current_vg_inner_table_set);
		}

		formatted_column.column_name_in_temporary_table_no_uuid = formatted_column.column_name_in_temporary_table;

		if (!first)
		{
			sql_string += ", ";
		}

		first = false;

		sql_string += "`";
		sql_string += unformatted_column.column_name_in_temporary_table;
		sql_string += "`";
		sql_string += " AS ";
		sql_string += "`";
		sql_string += formatted_column.column_name_in_temporary_table;
		sql_string += "`";

		++column_index;

	});

	// ... next, the secondary key columns from the child variable groups
	column_index = 0;
	std::for_each(all_merged_results_unformatted.second.columns_in_view.begin(),
				  all_merged_results_unformatted.second.columns_in_view.end(), [&c, &variable_group_appears_more_than_once, &sql_string, &first, &first_variable_group, &result_columns, &column_index](
					  ColumnsInTempView::ColumnInTempView & unformatted_column)
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
			formatted_column.column_name_in_temporary_table += std::to_string(formatted_column.current_multiplicity__of__current_inner_table__within__current_vg_inner_table_set);
		}

		formatted_column.column_name_in_temporary_table_no_uuid = formatted_column.column_name_in_temporary_table;

		if (!first)
		{
			sql_string += ", ";
		}

		first = false;

		sql_string += "`";
		sql_string += unformatted_column.column_name_in_temporary_table;
		sql_string += "`";
		sql_string += " AS ";
		sql_string += "`";
		sql_string += formatted_column.column_name_in_temporary_table;
		sql_string += "`";

		++column_index;

	});

	sql_string += " FROM \"";
	sql_string += all_merged_results_unformatted.second.view_name;
	sql_string += "\"";

	first = true;
	SortOrderByMultiplicityOnes(all_merged_results_unformatted.second, XR_TABLE_CATEGORY::KAD_RESULTS, first_variable_group, sql_string, first);

	if (first)
	{
		sql_string += " ORDER BY ";
	}
	else
	{
		sql_string += ", ";
	}

	first = false;
	sql_string += "`";
	sql_string += datetimestart_timestamp_colname;
	sql_string += "`";
	sql_string += ", ";
	sql_string += "`";
	sql_string += datetimeend_timestamp_colname;
	sql_string += "`";
	SortOrderByMultiplicityGreaterThanOnes(all_merged_results_unformatted.second, XR_TABLE_CATEGORY::KAD_RESULTS, first_variable_group, sql_string, first);

	final_result.second.most_recent_sql_statement_executed__index = -1;
	ExecuteSQL(final_result);
	ClearTable(all_merged_results_unformatted);

}

void OutputModel::OutputGenerator::MergeHighLevelGroupResults()
{

	if (primary_group_final_results.size() == 1)
	{
		primary_group_merged_results = primary_group_final_results[0];
		return;
	}

	SqlAndColumnSet intermediate_merge_of_top_level_primary_group_results = primary_group_final_results[0];
	intermediate_merge_of_top_level_primary_group_results.second.view_number = 1;
	intermediate_merging_of_primary_groups_column_sets.push_back(intermediate_merge_of_top_level_primary_group_results);

	std::int64_t number_of_rows_to_sort = ObtainCount(intermediate_merging_of_primary_groups_column_sets.back().second);
	current_number_rows_to_sort = number_of_rows_to_sort;

	boost::format msg_start("Variable group \"%1%\"");
	msg_start % (primary_group_final_results[0].second.variable_groups[0].longhand ? *primary_group_final_results[0].second.variable_groups[0].longhand
				 : primary_group_final_results[0].second.variable_groups[0].code ? *primary_group_final_results[0].second.variable_groups[0].code : std::string());
	messager.AppendKadStatusText(msg_start.str(), this);

	boost::format msg_("Retrieving final data for variable group %1%: %2% rows");
	msg_ % (primary_group_final_results[0].second.variable_groups[0].longhand ? *primary_group_final_results[0].second.variable_groups[0].longhand
			: primary_group_final_results[0].second.variable_groups[0].code ? *primary_group_final_results[0].second.variable_groups[0].code : std::string()) % number_of_rows_to_sort;
	UpdateProgressBarToNextStage(msg_.str(), std::string());

	// DETAILED_COMMENTING: Add details about CreateInitialPrimaryMergeXRTable
	SqlAndColumnSet xr_table_result = CreateInitialPrimaryMergeXRTable(intermediate_merging_of_primary_groups_column_sets.back().second);

	if (failed || CheckCancelled())
	{
		return;
	}

	xr_table_result.second.most_recent_sql_statement_executed__index = -1;
	ExecuteSQL(xr_table_result);
	ClearTable(intermediate_merging_of_primary_groups_column_sets.back());
	intermediate_merging_of_primary_groups_column_sets.push_back(xr_table_result);

	if (failed || CheckCancelled())
	{
		return;
	}

	// unused
	std::int64_t raw_rows_count = 0;
	std::int64_t rows_estimate = 0;

	SqlAndColumnSet duplicates_removed = intermediate_merging_of_primary_groups_column_sets.back();

	// The primary variable group data is stored in primary_group_final_results
	int count = 1;
	std::for_each(primary_group_final_results.cbegin(),
				  primary_group_final_results.cend(), [this, &duplicates_removed, &number_of_rows_to_sort, &rows_estimate, &raw_rows_count, &xr_table_result, &intermediate_merge_of_top_level_primary_group_results, &count](
					  SqlAndColumnSet const & primary_variable_group_final_result)
	{

		if (failed || CheckCancelled())
		{
			return;
		}

		if (count != 1)
		{

			boost::format msg_start("Variable group \"%1%\"");
			msg_start % (primary_variable_group_final_result.second.variable_groups[0].longhand ? *primary_variable_group_final_result.second.variable_groups[0].longhand
						 : primary_variable_group_final_result.second.variable_groups[0].code ? *primary_variable_group_final_result.second.variable_groups[0].code : std::string());
			messager.AppendKadStatusText(msg_start.str(), this);

			std::int64_t number_of_rows_previous = ObtainCount(duplicates_removed.second);
			std::int64_t number_of_rows_new = ObtainCount(primary_variable_group_final_result.second);

			if (primary_variable_group_final_result.second.variable_groups[0].longhand)
			{
				boost::format msg("Retrieving final data for variable group \"%1%\": merging %2% previous with %3% new rows");
				msg % *primary_variable_group_final_result.second.variable_groups[0].longhand % number_of_rows_previous % number_of_rows_new;
				messager.SetPerformanceLabel(msg.str().c_str());
			}
			else
			{
				boost::format msg("Retrieving final data for variable group \"%1%\": merging %2% previous with %3% new rows");
				msg % *primary_variable_group_final_result.second.variable_groups[0].code % number_of_rows_previous % number_of_rows_new;
				messager.SetPerformanceLabel(msg.str().c_str());
			}

			boost::format msg_("Retrieving final data for variable group \"%1%\": merging %2% previous with %3% new rows");
			msg_ % (primary_variable_group_final_result.second.variable_groups[0].longhand ? *primary_variable_group_final_result.second.variable_groups[0].longhand
					: primary_variable_group_final_result.second.variable_groups[0].code ? *primary_variable_group_final_result.second.variable_groups[0].code : std::string()) %
			number_of_rows_previous % number_of_rows_new;
			UpdateProgressBarToNextStage(msg_.str(), std::string());

			messager.SetPerformanceLabel("Performing a database join; please be patient...");
			// DETAILED_COMMENTING: Describe MergeIndividualTopLevelGroupIntoPrevious()
			intermediate_merge_of_top_level_primary_group_results = MergeIndividualTopLevelGroupIntoPrevious(primary_variable_group_final_result.second, duplicates_removed, count);

			if (failed || CheckCancelled())
			{
				return;
			}

			intermediate_merge_of_top_level_primary_group_results.second.most_recent_sql_statement_executed__index = -1;
			ExecuteSQL(intermediate_merge_of_top_level_primary_group_results);
			ClearTable(intermediate_merging_of_primary_groups_column_sets.back());
			ClearTable(primary_variable_group_final_result);
			messager.SetPerformanceLabel("");
			intermediate_merging_of_primary_groups_column_sets.push_back(intermediate_merge_of_top_level_primary_group_results);

			if (failed || CheckCancelled())
			{
				return;
			}

			std::int64_t number_of_rows = ObtainCount(intermediate_merge_of_top_level_primary_group_results.second);

			boost::format msg_2("Splitting rows on time boundaries, and removing redundant NULL rows, for merged variable group \"%1%\": %2% rows");
			msg_2 % (primary_variable_group_final_result.second.variable_groups[0].longhand ? *primary_variable_group_final_result.second.variable_groups[0].longhand
					 : primary_variable_group_final_result.second.variable_groups[0].code ? *primary_variable_group_final_result.second.variable_groups[0].code : std::string()) % number_of_rows;
			UpdateProgressBarToNextStage(msg_2.str(), std::string());

			// DETAILED_COMMENTING: To see purpose of CreateXRTable(), search for DESCRIPTION_OF_XR_ALGORITHM
			xr_table_result = CreateXRTable(intermediate_merge_of_top_level_primary_group_results.second, count, 0, OutputModel::OutputGenerator::FINAL_MERGE_OF_PRIMARY_VARIABLE_GROUP, count,
											count);
			ClearTable(intermediate_merging_of_primary_groups_column_sets.back());
			intermediate_merging_of_primary_groups_column_sets.push_back(xr_table_result);

			if (failed || CheckCancelled())
			{
				return;
			}

			std::string sorting_rows_text("Sorting rows");
			std::string removing_duplicates_text("Removing duplicates");
			// DETAILED_COMMENTING: To see purpose of SortAndOrRemoveDuplicates(), search for DESCRIPTION_OF_SORT_AND_OR_REMOVE_DUPLICATES_ALGORITHM
			duplicates_removed = SortAndOrRemoveDuplicates(xr_table_result.second, primary_variable_group_final_result.second.variable_groups[0], sorting_rows_text, removing_duplicates_text,
								 -1, count, intermediate_merging_of_primary_groups_column_sets, true, OutputModel::OutputGenerator::FINAL_MERGE_OF_PRIMARY_VARIABLE_GROUP, true);

			if (failed || CheckCancelled())
			{
				return;
			}

		}

		++count;
	});

	if (failed || CheckCancelled())
	{
		return;
	}

	// Last two columns:
	// DATETIMESTART__TIMERANGE_MERGED_BETWEEN_TOP_LEVEL_PRIMARY_VARIABLE_GROUPS / DATETIMEEND__TIMERANGE_MERGED_BETWEEN_TOP_LEVEL_PRIMARY_VARIABLE_GROUPS
	primary_group_merged_results = duplicates_removed;

}

OutputModel::OutputGenerator::SqlAndColumnSet OutputModel::OutputGenerator::MergeIndividualTopLevelGroupIntoPrevious(ColumnsInTempView const & primary_variable_group_final_result,
		OutputModel::OutputGenerator::SqlAndColumnSet & previous_merged_primary_variable_groups_table, int const count)
{

	SqlAndColumnSet result = std::make_pair(std::vector<SQLExecutor>(), ColumnsInTempView());
	std::vector<SQLExecutor> & sql_strings = result.first;
	ColumnsInTempView & result_columns = result.second;

	result_columns = previous_merged_primary_variable_groups_table.second;

	std::string view_name = "NGTEMP_MF";
	view_name += std::to_string(count);
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

	// These columns are from the previous temporary table
	bool first_datetime_already_reached = false;
	bool stop_incrementing_first_inner_table = false;
	std::for_each(result_columns.columns_in_view.begin(),
				  result_columns.columns_in_view.end(), [&first_datetime_already_reached, &stop_incrementing_first_inner_table, &lhs_primary_keys, &first_full_table_column_count, &number_columns__in__very_first_primary_variable_group__and__only_its_first_inner_table, &previous_column_names, &number_columns_very_first_primary_variable_group_including_multiplicities, &very_first_primary_variable_group](
					  ColumnsInTempView::ColumnInTempView & previous_column)
	{

		if (first_full_table_column_count == 0)
		{
			very_first_primary_variable_group = previous_column.variable_group_associated_with_current_inner_table;
		}

		previous_column_names.push_back(previous_column.column_name_in_temporary_table);
		previous_column.column_name_in_temporary_table = previous_column.column_name_in_temporary_table_no_uuid;
		previous_column.column_name_in_temporary_table += "_";
		previous_column.column_name_in_temporary_table += newUUID(true);

		if (previous_column.column_type == ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__DATETIMESTART
			|| previous_column.column_type == ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__DATETIMESTART_INTERNAL)
		{
			first_datetime_already_reached = true;
		}
		else
		{
			if (previous_column.column_type == ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__PRIMARY
				|| previous_column.column_type == ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__SECONDARY)
			{
				if (first_datetime_already_reached)
				{
					stop_incrementing_first_inner_table = true;
				}
			}
		}

		if (!stop_incrementing_first_inner_table)
		{
			++number_columns__in__very_first_primary_variable_group__and__only_its_first_inner_table;
		}

		if (previous_column.variable_group_associated_with_current_inner_table.IsEqual(WidgetInstanceIdentifier::EQUALITY_CHECK_TYPE__STRING_CODE, very_first_primary_variable_group))
		{
			++number_columns_very_first_primary_variable_group_including_multiplicities;

			if (previous_column.column_type == ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__PRIMARY)
			{
				if (previous_column.current_multiplicity__of__current_inner_table__within__current_vg_inner_table_set == 1)
				{
					lhs_primary_keys[previous_column.primary_key_dmu_category_identifier].first = 0;
					lhs_primary_keys[previous_column.primary_key_dmu_category_identifier].second.push_back((int)(previous_column_names.size()) - 1);
				}
				else
				{
					if (previous_column.total_outer_multiplicity__in_total_kad__for_current_dmu_category__for_current_variable_group > 1)
					{
						lhs_primary_keys[previous_column.primary_key_dmu_category_identifier].first = 0;
						lhs_primary_keys[previous_column.primary_key_dmu_category_identifier].second.push_back((int)(previous_column_names.size()) - 1);
					}
				}
			}
		}

		// Includes columns of the previous table -
		// i.e., all columns (including all datetime columns) of the table PRIOR to the new primary variable group being merged in
		++first_full_table_column_count;
	});


	int number_columns_very_last_primary_variable_group_including_multiplicities = 0; // corresponding to newly-being-added primary variable group (the last one)
	int number_columns__in__very_last_primary_variable_group__and__only_its_first_inner_table = 0;
	WidgetInstanceIdentifier very_last_primary_variable_group;
	int second_table_column_count = 0;

	// These columns are from the new table (the current primary variable group final results being merged in) being added.
	// This data has a single extra pair of datetime columns at the very end of the very last inner table.
	bool last_datetime_already_reached = false;
	bool stop_incrementing_last_inner_table = false;
	std::for_each(primary_variable_group_final_result.columns_in_view.cbegin(),
				  primary_variable_group_final_result.columns_in_view.cend(), [&last_datetime_already_reached, &stop_incrementing_last_inner_table, &rhs_primary_keys, &result_columns, &number_columns_very_last_primary_variable_group_including_multiplicities, &number_columns__in__very_last_primary_variable_group__and__only_its_first_inner_table, &very_last_primary_variable_group, &second_table_column_count, &previous_column_names, &count](
					  ColumnsInTempView::ColumnInTempView const & new_table_column)
	{

		if (second_table_column_count == 0)
		{
			very_last_primary_variable_group = new_table_column.variable_group_associated_with_current_inner_table;
		}

		previous_column_names.push_back(new_table_column.column_name_in_temporary_table);
		result_columns.columns_in_view.push_back(new_table_column);
		ColumnsInTempView::ColumnInTempView & new_column = result_columns.columns_in_view.back();
		new_column.column_name_in_temporary_table = new_column.column_name_in_temporary_table_no_uuid;
		new_column.column_name_in_temporary_table += "_";
		new_column.column_name_in_temporary_table += newUUID(true);

		if (new_column.column_type == ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__DATETIMESTART
			|| new_column.column_type == ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__DATETIMESTART_INTERNAL)
		{
			last_datetime_already_reached = true;
		}
		else
		{
			if (new_column.column_type == ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__PRIMARY
				|| new_column.column_type == ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__SECONDARY)
			{
				if (last_datetime_already_reached)
				{
					stop_incrementing_last_inner_table = true;
				}
			}
		}

		if (!stop_incrementing_last_inner_table)
		{
			++number_columns__in__very_last_primary_variable_group__and__only_its_first_inner_table;
		}

		if (new_table_column.variable_group_associated_with_current_inner_table.IsEqual(WidgetInstanceIdentifier::EQUALITY_CHECK_TYPE__STRING_CODE, very_last_primary_variable_group))
		{
			++number_columns_very_last_primary_variable_group_including_multiplicities;

			if (new_table_column.column_type == ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__PRIMARY)
			{
				if (new_table_column.current_multiplicity__of__current_inner_table__within__current_vg_inner_table_set == 1)
				{
					rhs_primary_keys[new_table_column.primary_key_dmu_category_identifier].first = 0;
					rhs_primary_keys[new_table_column.primary_key_dmu_category_identifier].second.push_back((int)(previous_column_names.size()) - 1);
				}
				else
				{
					if (new_table_column.total_outer_multiplicity__in_total_kad__for_current_dmu_category__for_current_variable_group > 1)
					{
						rhs_primary_keys[new_table_column.primary_key_dmu_category_identifier].first = 0;
						rhs_primary_keys[new_table_column.primary_key_dmu_category_identifier].second.push_back((int)(previous_column_names.size()) - 1);
					}
				}
			}
		}

		// after this for_each exits, this variable includes all datetime columns:
		// the 2 pairs for all inner tables,
		// and the additional single pair for this top-level primary variable group
		++second_table_column_count;
	});

	// sanity check
	if (number_columns_very_last_primary_variable_group_including_multiplicities % number_columns__in__very_last_primary_variable_group__and__only_its_first_inner_table != 0)
	{
		boost::format
		msg("The number of columns in the full set of inner tables for the last primary variable group is not an even multiple of the number of columns in its first inner table.");
		SetFailureMessage(msg.str());
		failed = true;
		return result;
	}


	std::string sql_select_left;
	std::string sql_select_right;
	bool first = true;
	int column_count = 0;
	std::for_each(result_columns.columns_in_view.begin(),
				  result_columns.columns_in_view.end(), [&lhs_primary_keys, &rhs_primary_keys, &number_columns_very_first_primary_variable_group_including_multiplicities, &number_columns_very_last_primary_variable_group_including_multiplicities, &sql_select_left, &sql_select_right, &first, &column_count, &first_full_table_column_count, &second_table_column_count, &previous_column_names](
					  ColumnsInTempView::ColumnInTempView & new_column)
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
				if (new_column.current_multiplicity__of__current_inner_table__within__current_vg_inner_table_set == 1)
				{
					handled = true;
				}
				else
				{
					if (new_column.total_outer_multiplicity__in_total_kad__for_current_dmu_category__for_current_variable_group > 1)
					{
						handled = true;
					}
				}

				if (handled)
				{
					sql_select_left += "CASE WHEN \"t1\".`";
					sql_select_left += previous_column_names[column_count];
					sql_select_left += "` IS NOT NULL THEN ";

					// Not legal for outer joins - but child table should already have proper column type
					if (false && new_column.column_type == ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__PRIMARY
						&& new_column.primary_key_should_be_treated_as_integer_____float_not_allowed_as_primary_key)
					{
						sql_select_left += "CAST (";
					}

					sql_select_left += "\"t1\".`";
					sql_select_left += previous_column_names[column_count];
					sql_select_left += "`";

					// Not legal for outer joins - but child table should already have proper column type
					if (false && new_column.column_type == ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__PRIMARY
						&& new_column.primary_key_should_be_treated_as_integer_____float_not_allowed_as_primary_key)
					{
						sql_select_left += " AS INTEGER)";
					}

					sql_select_left += " ELSE ";

					// Not legal for outer joins - but child table should already have proper column type
					if (false && new_column.column_type == ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__PRIMARY
						&& new_column.primary_key_should_be_treated_as_integer_____float_not_allowed_as_primary_key)
					{
						sql_select_left += "CAST (";
					}

					sql_select_left += "\"t2\".`";
					sql_select_left +=
						previous_column_names[rhs_primary_keys[new_column.primary_key_dmu_category_identifier].second[rhs_primary_keys[new_column.primary_key_dmu_category_identifier].first]];
					sql_select_left += "`";

					// Not legal for outer joins - but child table should already have proper column type
					if (false && new_column.column_type == ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__PRIMARY
						&& new_column.primary_key_should_be_treated_as_integer_____float_not_allowed_as_primary_key)
					{
						sql_select_left += " AS INTEGER)";
					}

					sql_select_left += " END AS `";
					sql_select_left += new_column.column_name_in_temporary_table;
					sql_select_left += "`";

					sql_select_right += "CASE WHEN \"t2\".`";
					sql_select_right += previous_column_names[column_count];
					sql_select_right += "` IS NOT NULL THEN ";

					// Not legal for outer joins - but child table should already have proper column type
					if (false && new_column.column_type == ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__PRIMARY
						&& new_column.primary_key_should_be_treated_as_integer_____float_not_allowed_as_primary_key)
					{
						sql_select_right += "CAST (";
					}

					sql_select_right += "\"t2\".`";
					sql_select_right += previous_column_names[column_count];
					sql_select_right += "`";

					// Not legal for outer joins - but child table should already have proper column type
					if (false && new_column.column_type == ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__PRIMARY
						&& new_column.primary_key_should_be_treated_as_integer_____float_not_allowed_as_primary_key)
					{
						sql_select_right += " AS INTEGER)";
					}

					sql_select_right += " ELSE ";

					// Not legal for outer joins - but child table should already have proper column type
					if (false && new_column.column_type == ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__PRIMARY
						&& new_column.primary_key_should_be_treated_as_integer_____float_not_allowed_as_primary_key)
					{
						sql_select_right += "CAST (";
					}

					sql_select_right += "\"t1\".`";
					sql_select_right +=
						previous_column_names[rhs_primary_keys[new_column.primary_key_dmu_category_identifier].second[rhs_primary_keys[new_column.primary_key_dmu_category_identifier].first++]];
					sql_select_right += "`";

					// Not legal for outer joins - but child table should already have proper column type
					if (false && new_column.column_type == ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__PRIMARY
						&& new_column.primary_key_should_be_treated_as_integer_____float_not_allowed_as_primary_key)
					{
						sql_select_right += " AS INTEGER)";
					}

					sql_select_right += " END AS `";
					sql_select_right += new_column.column_name_in_temporary_table;
					sql_select_right += "`";
				}
			}
		}
		else if (column_count >= first_full_table_column_count && column_count < first_full_table_column_count + number_columns_very_last_primary_variable_group_including_multiplicities)
		{
			if (new_column.column_type == ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__PRIMARY)
			{
				if (new_column.current_multiplicity__of__current_inner_table__within__current_vg_inner_table_set == 1)
				{
					handled = true;
				}
				else
				{
					if (new_column.total_outer_multiplicity__in_total_kad__for_current_dmu_category__for_current_variable_group > 1)
					{
						handled = true;
					}
				}

				if (handled)
				{
					sql_select_right += "CASE WHEN \"t1\".`";
					sql_select_right += previous_column_names[column_count];
					sql_select_right += "` IS NOT NULL THEN ";

					// Not legal for outer joins - but child table should already have proper column type
					if (false && new_column.column_type == ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__PRIMARY
						&& new_column.primary_key_should_be_treated_as_integer_____float_not_allowed_as_primary_key)
					{
						sql_select_right += "CAST (";
					}

					sql_select_right += "\"t1\".`";
					sql_select_right += previous_column_names[column_count];
					sql_select_right += "`";

					// Not legal for outer joins - but child table should already have proper column type
					if (false && new_column.column_type == ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__PRIMARY
						&& new_column.primary_key_should_be_treated_as_integer_____float_not_allowed_as_primary_key)
					{
						sql_select_right += " AS INTEGER)";
					}

					sql_select_right += " ELSE ";

					// Not legal for outer joins - but child table should already have proper column type
					if (false && new_column.column_type == ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__PRIMARY
						&& new_column.primary_key_should_be_treated_as_integer_____float_not_allowed_as_primary_key)
					{
						sql_select_right += "CAST (";
					}

					sql_select_right += "\"t2\".`";
					sql_select_right +=
						previous_column_names[lhs_primary_keys[new_column.primary_key_dmu_category_identifier].second[lhs_primary_keys[new_column.primary_key_dmu_category_identifier].first]];
					sql_select_right += "`";

					// Not legal for outer joins - but child table should already have proper column type
					if (false && new_column.column_type == ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__PRIMARY
						&& new_column.primary_key_should_be_treated_as_integer_____float_not_allowed_as_primary_key)
					{
						sql_select_right += " AS INTEGER)";
					}

					sql_select_right += " END AS `";
					sql_select_right += new_column.column_name_in_temporary_table;
					sql_select_right += "`";

					sql_select_left += "CASE WHEN \"t2\".`";
					sql_select_left += previous_column_names[column_count];
					sql_select_left += "` IS NOT NULL THEN ";

					// Not legal for outer joins - but child table should already have proper column type
					if (false && new_column.column_type == ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__PRIMARY
						&& new_column.primary_key_should_be_treated_as_integer_____float_not_allowed_as_primary_key)
					{
						sql_select_left += "CAST (";
					}

					sql_select_left += "\"t2\".`";
					sql_select_left += previous_column_names[column_count];
					sql_select_left += "`";

					// Not legal for outer joins - but child table should already have proper column type
					if (false && new_column.column_type == ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__PRIMARY
						&& new_column.primary_key_should_be_treated_as_integer_____float_not_allowed_as_primary_key)
					{
						sql_select_left += " AS INTEGER)";
					}

					sql_select_left += " ELSE ";

					// Not legal for outer joins - but child table should already have proper column type
					if (false && new_column.column_type == ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__PRIMARY
						&& new_column.primary_key_should_be_treated_as_integer_____float_not_allowed_as_primary_key)
					{
						sql_select_left += "CAST (";
					}

					sql_select_left += "\"t1\".`";
					sql_select_left +=
						previous_column_names[lhs_primary_keys[new_column.primary_key_dmu_category_identifier].second[lhs_primary_keys[new_column.primary_key_dmu_category_identifier].first++]];
					sql_select_left += "`";

					// Not legal for outer joins - but child table should already have proper column type
					if (false && new_column.column_type == ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__PRIMARY
						&& new_column.primary_key_should_be_treated_as_integer_____float_not_allowed_as_primary_key)
					{
						sql_select_left += " AS INTEGER)";
					}

					sql_select_left += " END AS `";
					sql_select_left += new_column.column_name_in_temporary_table;
					sql_select_left += "`";
				}
			}
		}

		if (!handled)
		{

			// Not legal for outer joins - but child table should already have proper column type
			if (false && new_column.column_type == ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__PRIMARY
				&& new_column.primary_key_should_be_treated_as_integer_____float_not_allowed_as_primary_key)
			{
				sql_select_left += "CAST (";
			}

			if (column_count < first_full_table_column_count)
			{
				sql_select_left  += "\"t1\".";
				sql_select_right += "\"t2\".";
			}
			else
			{
				sql_select_left  += "\"t2\".";
				sql_select_right += "\"t1\".";
			}

			sql_select_left += "`";
			sql_select_left += previous_column_names[column_count];
			sql_select_left += "`";

			// Not legal for outer joins - but child table should already have proper column type
			if (false && new_column.column_type == ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__PRIMARY
				&& new_column.primary_key_should_be_treated_as_integer_____float_not_allowed_as_primary_key)
			{
				sql_select_left += " AS INTEGER)";
			}

			sql_select_left += " AS ";
			sql_select_left += "`";
			sql_select_left += new_column.column_name_in_temporary_table;
			sql_select_left += "`";

			// Not legal for outer joins - but child table should already have proper column type
			if (false && new_column.column_type == ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__PRIMARY
				&& new_column.primary_key_should_be_treated_as_integer_____float_not_allowed_as_primary_key)
			{
				sql_select_right += "CAST (";
			}

			sql_select_right += "`";
			sql_select_right += previous_column_names[column_count];
			sql_select_right += "`";

			// Not legal for outer joins - but child table should already have proper column type
			if (false && new_column.column_type == ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__PRIMARY
				&& new_column.primary_key_should_be_treated_as_integer_____float_not_allowed_as_primary_key)
			{
				sql_select_right += " AS INTEGER)";
			}

			sql_select_right += " AS ";
			sql_select_right += "`";
			sql_select_right += new_column.column_name_in_temporary_table;
			sql_select_right += "`";
		}

		++column_count;

	});

	std::vector<std::string> join_column_names_lhs;
	std::vector<std::string> join_column_names_rhs;

	for (int current_outer_multiplicity_of_top_level_variable_group___same_as___current_inner_table_number_of_top_level_variable_group = 1;
		 current_outer_multiplicity_of_top_level_variable_group___same_as___current_inner_table_number_of_top_level_variable_group <= highest_multiplicity_primary_uoa;
		 ++current_outer_multiplicity_of_top_level_variable_group___same_as___current_inner_table_number_of_top_level_variable_group)
	{
		std::for_each(sequence.primary_key_sequence_info.cbegin(),
					  sequence.primary_key_sequence_info.cend(), [this, &number_columns__in__very_first_primary_variable_group__and__only_its_first_inner_table, &join_column_names_lhs, &join_column_names_rhs, &very_first_primary_variable_group, &very_last_primary_variable_group, &number_columns_very_first_primary_variable_group_including_multiplicities, &number_columns_very_last_primary_variable_group_including_multiplicities, &current_outer_multiplicity_of_top_level_variable_group___same_as___current_inner_table_number_of_top_level_variable_group, &result_columns, &first_full_table_column_count, &second_table_column_count, &previous_column_names](
						  PrimaryKeySequence::PrimaryKeySequenceEntry const & primary_key)
		{
			std::for_each(primary_key.variable_group_info_for_primary_keys.cbegin(),
						  primary_key.variable_group_info_for_primary_keys.cend(), [this, &number_columns__in__very_first_primary_variable_group__and__only_its_first_inner_table, &join_column_names_lhs, &join_column_names_rhs, &very_first_primary_variable_group, &very_last_primary_variable_group, &number_columns_very_first_primary_variable_group_including_multiplicities, &number_columns_very_last_primary_variable_group_including_multiplicities, &current_outer_multiplicity_of_top_level_variable_group___same_as___current_inner_table_number_of_top_level_variable_group, &primary_key, &result_columns, &first_full_table_column_count, &second_table_column_count, &previous_column_names](
							  PrimaryKeySequence::VariableGroup_PrimaryKey_Info const & primary_key_info_this_variable_group)
			{
				if (primary_key_info_this_variable_group.vg_identifier.IsEqual(WidgetInstanceIdentifier::EQUALITY_CHECK_TYPE__STRING_CODE, very_first_primary_variable_group)
					||
					primary_key_info_this_variable_group.vg_identifier.IsEqual(WidgetInstanceIdentifier::EQUALITY_CHECK_TYPE__STRING_CODE, very_last_primary_variable_group))
				{
					if (primary_key_info_this_variable_group.current_outer_multiplicity_of_this_primary_key__in_relation_to__the_uoa_corresponding_to_the_current_variable_group___same_as___current_inner_table_number_within_the_inner_table_set_corresponding_to_the_current_variable_group
						== current_outer_multiplicity_of_top_level_variable_group___same_as___current_inner_table_number_of_top_level_variable_group)
					{
						int column_count = 0;
						std::for_each(result_columns.columns_in_view.cbegin(),
									  result_columns.columns_in_view.cend(), [this, &number_columns__in__very_first_primary_variable_group__and__only_its_first_inner_table, &current_outer_multiplicity_of_top_level_variable_group___same_as___current_inner_table_number_of_top_level_variable_group, &join_column_names_lhs, &join_column_names_rhs, &very_first_primary_variable_group, &very_last_primary_variable_group, &number_columns_very_first_primary_variable_group_including_multiplicities, &number_columns_very_last_primary_variable_group_including_multiplicities, &primary_key_info_this_variable_group, &first_full_table_column_count, &second_table_column_count, &column_count, &previous_column_names, &primary_key](
										  ColumnsInTempView::ColumnInTempView const & new_column)
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
										if (primary_key_info_this_variable_group.total_outer_multiplicity__in_total_kad__for_current_dmu_category__for_current_variable_group == 1)
										{
											if (current_outer_multiplicity_of_top_level_variable_group___same_as___current_inner_table_number_of_top_level_variable_group == 1)
											{
												if (new_column.current_multiplicity__of__current_inner_table__within__current_vg_inner_table_set == 1)
												{
													match_condition = (new_column.primary_key_index_within_total_kad_for_dmu_category >= 0
																	   && (new_column.primary_key_index_within_total_kad_for_dmu_category == primary_key.sequence_number_within_dmu_category_spin_control));
												}
											}
										}
										// Join on primary keys with multiplicity greater than 1
										else
										{
											desired_inner_table_index = current_outer_multiplicity_of_top_level_variable_group___same_as___current_inner_table_number_of_top_level_variable_group - 1;
											match_condition = (new_column.primary_key_index_within_total_kad_for_dmu_category >= 0
															   && (new_column.primary_key_index__within_uoa_corresponding_to_variable_group_corresponding_to_current_inner_table__for_dmu_category ==
																   primary_key_info_this_variable_group.sequence_number_within_dmu_category_for_this_variable_groups_uoa));
										}

										if (column_count >= desired_inner_table_index * number_columns__in__very_first_primary_variable_group__and__only_its_first_inner_table
											&& column_count < (desired_inner_table_index + 1) * number_columns__in__very_first_primary_variable_group__and__only_its_first_inner_table)
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
										if (primary_key_info_this_variable_group.total_outer_multiplicity__in_total_kad__for_current_dmu_category__for_current_variable_group == 1)
										{
											if (current_outer_multiplicity_of_top_level_variable_group___same_as___current_inner_table_number_of_top_level_variable_group == 1)
											{
												if (new_column.current_multiplicity__of__current_inner_table__within__current_vg_inner_table_set == 1)
												{
													match_condition = (new_column.primary_key_index_within_total_kad_for_dmu_category >= 0
																	   && (new_column.primary_key_index_within_total_kad_for_dmu_category == primary_key.sequence_number_within_dmu_category_spin_control));
												}
											}
										}
										// Join on primary keys with multiplicity greater than 1
										else
										{
											desired_inner_table_index = current_outer_multiplicity_of_top_level_variable_group___same_as___current_inner_table_number_of_top_level_variable_group - 1;
											match_condition = (new_column.primary_key_index_within_total_kad_for_dmu_category >= 0
															   && (new_column.primary_key_index__within_uoa_corresponding_to_variable_group_corresponding_to_current_inner_table__for_dmu_category ==
																   primary_key_info_this_variable_group.sequence_number_within_dmu_category_for_this_variable_groups_uoa));
										}

										if (column_count >= first_full_table_column_count + (desired_inner_table_index * number_columns__in__very_first_primary_variable_group__and__only_its_first_inner_table)
											&& column_count < first_full_table_column_count + ((desired_inner_table_index + 1) * number_columns__in__very_first_primary_variable_group__and__only_its_first_inner_table))
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
	std::for_each(join_column_names_lhs.cbegin(), join_column_names_lhs.cend(), [&join_index, &sql_join_on_left, &sql_join_on_right, &sql_null_clause, &and_, &join_column_names_rhs](
					  std::string const & join_column_name_lhs)
	{
		if (and_)
		{
			sql_join_on_left  += " AND ";
			sql_join_on_right += " AND ";
			sql_null_clause   += " AND ";
		}

		and_ = true;

		sql_null_clause += "\"t2\".`";
		sql_null_clause += join_column_name_lhs;
		sql_null_clause += "` IS NULL";

		sql_join_on_left += "\"t1\".`";
		sql_join_on_left += join_column_name_lhs;
		sql_join_on_left += "` = \"t2\".`";
		sql_join_on_left += join_column_names_rhs[join_index];
		sql_join_on_left += "`";

		sql_join_on_right += "\"t1\".`";
		sql_join_on_right += join_column_names_rhs[join_index];
		sql_join_on_right += "` = \"t2\".`";
		sql_join_on_right += join_column_name_lhs;
		sql_join_on_right += "`";

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
			std::for_each(result_columns.columns_in_view.begin(),
						  result_columns.columns_in_view.end(), [this, &current_column_count, &number_columns__in__very_first_primary_variable_group__and__only_its_first_inner_table, &number_primary_key_columns_in_dmu_category_with_multiplicity_greater_than_1__for_top_level_uoa](
							  ColumnsInTempView::ColumnInTempView & view_column)
			{
				if (current_column_count < number_columns__in__very_first_primary_variable_group__and__only_its_first_inner_table)
				{
					if (view_column.column_type == ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__PRIMARY)
					{
						if (view_column.is_within_inner_table_corresponding_to_top_level_uoa)
						{
							if (view_column.total_outer_multiplicity__in_total_kad__for_current_dmu_category__for_current_variable_group == highest_multiplicity_primary_uoa)
							{
								if (view_column.current_multiplicity__corresponding_to__current_inner_table___is_1_in_all_inner_tables_when_multiplicity_is_1_for_that_dmu_category_for_that_vg == 1)
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

		// Order by remaining primary key columns (with multiplicity 1)
		// ... If there are no primary key DMU categories for this top-level UOA with multiplicity greater than 1,
		// then this section will order by all of this top-level's UOA primary key DMU categories.
		int current_column = 0;
		std::for_each(result_columns.columns_in_view.begin(),
					  result_columns.columns_in_view.end(), [this, &sql_order_by, &number_columns__in__very_first_primary_variable_group__and__only_its_first_inner_table, &result_columns, &current_column, &first](
						  ColumnsInTempView::ColumnInTempView & view_column)
		{

			if (current_column >= number_columns__in__very_first_primary_variable_group__and__only_its_first_inner_table)
			{
				return;
			}

			// Determine how many columns there are corresponding to the DMU category
			int number_primary_key_columns_in_dmu_category_with_multiplicity_of_1 = 0;
			int column_count_nested = 0;
			std::for_each(result_columns.columns_in_view.begin(),
						  result_columns.columns_in_view.end(), [this, &number_columns__in__very_first_primary_variable_group__and__only_its_first_inner_table, &view_column, &column_count_nested, &number_primary_key_columns_in_dmu_category_with_multiplicity_of_1](
							  ColumnsInTempView::ColumnInTempView & view_column_nested)
			{
				if (column_count_nested >= number_columns__in__very_first_primary_variable_group__and__only_its_first_inner_table)
				{
					return;
				}

				if (view_column_nested.column_type == ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__PRIMARY)
				{
					if (view_column_nested.primary_key_dmu_category_identifier.IsEqual(WidgetInstanceIdentifier::EQUALITY_CHECK_TYPE__STRING_CODE, view_column.primary_key_dmu_category_identifier))
					{
						if (view_column_nested.total_outer_multiplicity__in_total_kad__for_current_dmu_category__for_current_variable_group == 1)
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
				if (view_column.total_outer_multiplicity__in_total_kad__for_current_dmu_category__for_current_variable_group == 1)
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

								// Not legal for outer joins - but child table should already have proper column type
								if (false && view_column.primary_key_should_be_treated_as_integer_____float_not_allowed_as_primary_key)
								{
									sql_order_by += "CAST (";
								}

								sql_order_by += "`";
								sql_order_by += view_column.column_name_in_temporary_table;
								sql_order_by += "`";

								// Not legal for outer joins - but child table should already have proper column type
								if (false && view_column.primary_key_should_be_treated_as_integer_____float_not_allowed_as_primary_key)
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

		if (highest_multiplicity_primary_uoa > 1)
		{

			// Create the ORDER BY clause, taking the proper primary key columns that compose the DMU category with multiplicity greater than 1, in sequence
			for (int outer_dmu_multiplicity = 1; outer_dmu_multiplicity <= highest_multiplicity_primary_uoa; ++outer_dmu_multiplicity)
			{
				for (int inner_dmu_multiplicity = 0; inner_dmu_multiplicity < number_primary_key_columns_in_dmu_category_with_multiplicity_greater_than_1__for_top_level_uoa;
					 ++inner_dmu_multiplicity)
				{
					int current_column_count = 0;
					std::for_each(result_columns.columns_in_view.begin(),
								  result_columns.columns_in_view.end(), [this, &sql_order_by, &current_column_count, &number_columns_very_first_primary_variable_group_including_multiplicities, &number_columns_very_last_primary_variable_group_including_multiplicities, &inner_dmu_multiplicity, &outer_dmu_multiplicity, &first](
									  ColumnsInTempView::ColumnInTempView & view_column)
					{
						if (current_column_count < number_columns_very_first_primary_variable_group_including_multiplicities)
						{
							if (view_column.column_type == ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__PRIMARY)
							{
								if (view_column.total_outer_multiplicity__in_total_kad__for_current_dmu_category__for_current_variable_group == highest_multiplicity_primary_uoa)
								{
									if (view_column.primary_key_index__within_uoa_corresponding_to_variable_group_corresponding_to_current_inner_table__for_dmu_category == inner_dmu_multiplicity)
									{
										if (view_column.current_multiplicity__corresponding_to__current_inner_table___is_1_in_all_inner_tables_when_multiplicity_is_1_for_that_dmu_category_for_that_vg ==
											outer_dmu_multiplicity)
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

												// Not legal for outer joins - but child table should already have proper column type
												if (false && view_column.primary_key_should_be_treated_as_integer_____float_not_allowed_as_primary_key)
												{
													sql_order_by += "CAST (";
												}

												sql_order_by += "`";
												sql_order_by += view_column.column_name_in_temporary_table;
												sql_order_by += "`";

												// Not legal for outer joins - but child table should already have proper column type
												if (false && view_column.primary_key_should_be_treated_as_integer_____float_not_allowed_as_primary_key)
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

	}


	sql_strings.push_back(SQLExecutor(this, db));
	std::string & sql_string = sql_strings.back().sql;

	sql_string = "CREATE TABLE \"";
	sql_string += result_columns.view_name;
	sql_string += "\" AS SELECT ";
	sql_string += sql_select_left;

	sql_string += " FROM \"";
	sql_string += previous_merged_primary_variable_groups_table.second.view_name;
	sql_string += "\" t1 ";

	sql_string += "LEFT OUTER JOIN \"";
	sql_string += primary_variable_group_final_result.view_name;
	sql_string += "\" t2 ON ";

	sql_string += sql_join_on_left;

	// Left-side WHERE goes here for timerange limitation

	sql_string += " UNION ALL ";

	sql_string += " SELECT ";

	sql_string += sql_select_right;

	sql_string += " FROM \"";
	sql_string += primary_variable_group_final_result.view_name;
	sql_string += "\" t1 ";

	sql_string += "LEFT OUTER JOIN \"";
	sql_string += previous_merged_primary_variable_groups_table.second.view_name;
	sql_string += "\" t2 ON ";

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

	result_columns.current_block_datetime_column_types = primary_variable_group_final_result.current_block_datetime_column_types;
	result_columns.previous_block_datetime_column_types = previous_merged_primary_variable_groups_table.second.current_block_datetime_column_types;

	return result;

}

void OutputModel::OutputGenerator::DetermineNumberStages()
{

	return;
















	total_number_primary_rows = 0;
	total_progress_stages = 0;

	int primary_group_number = 1;
	std::for_each(primary_variable_groups_column_info.cbegin(),
				  primary_variable_groups_column_info.cend(), [this, &primary_group_number](ColumnsInTempView const & primary_variable_group_raw_data_columns)
	{

		if (failed || CheckCancelled())
		{
			return;
		}

		SqlAndColumnSet x_table_result = CreateInitialPrimaryXTable_OrCount(primary_variable_group_raw_data_columns, primary_group_number, true);

		if (failed || CheckCancelled())
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

		// Before loop is entered
		total_progress_stages += 3;

		// Inside loop, which starts at 2, not 1
		total_progress_stages += (4 * (highest_multiplicity_primary_uoa - 1));

		// If randomizing
		if (random_sampling_old)
		{
			total_progress_stages += (1 * (highest_multiplicity_primary_uoa - 1));
		}

		if (primary_variable_groups_column_info.size() > 1)
		{
			// Merging of primary groups: One each
			++total_progress_stages;

			if (primary_group_number > 1)
			{
				// plus an extra three for every group after the first
				total_progress_stages += 3;
			}
		}

		++primary_group_number;

	});

	int child_set_number = 1;
	std::for_each(secondary_variable_groups_column_info.cbegin(),
				  secondary_variable_groups_column_info.cend(), [this, &child_set_number](ColumnsInTempView const & child_variable_group_raw_data_columns)
	{

		if (failed || CheckCancelled())
		{
			return;
		}

		// Call the following function, even though this is a child variable group, just to get a count
		SqlAndColumnSet x_table_result = CreateInitialPrimaryXTable_OrCount(child_variable_group_raw_data_columns, child_set_number, true);

		if (failed || CheckCancelled())
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

		// Four stages per child group multiplicity
		total_progress_stages += (4 * the_child_multiplicity);

		++child_set_number;

	});

	// Two stages for construction of final K-ad results
	//total_progress_stages += 2;

}

void OutputModel::OutputGenerator::LoopThroughPrimaryVariableGroups()
{

	int primary_group_number = 1;
	std::for_each(primary_variable_groups_column_info.cbegin(),
				  primary_variable_groups_column_info.cend(), [this, &primary_group_number](ColumnsInTempView const & primary_variable_group_raw_data_columns)
	{
		if (failed || CheckCancelled())
		{
			return;
		}

		primary_variable_group_column_sets.push_back(SqlAndColumnSets());
		SqlAndColumnSets & primary_group_column_sets = primary_variable_group_column_sets.back();
		SqlAndColumnSet primary_group_final_result = ConstructFullOutputForSinglePrimaryGroup(primary_variable_group_raw_data_columns, primary_group_column_sets, primary_group_number);

		if (failed || CheckCancelled())
		{
			return;
		}

		// EVERY inner table, including the LAST, has three pairs at its end:
		// COLUMN_TYPE__DATETIMESTART / COLUMN_TYPE__DATETIMEEND ***OR*** COLUMN_TYPE__DATETIMESTART_INTERNAL / COLUMN_TYPE__DATETIMEEND_INTERNAL
		// COLUMN_TYPE__DATETIMESTART__PRIMARY_VG_INNER_TABLE_MERGE__BEFORE_DUPLICATES_REMOVED / COLUMN_TYPE__DATETIMEEND__PRIMARY_VG_INNER_TABLE_MERGE__BEFORE_DUPLICATES_REMOVED
		// COLUMN_TYPE__DATETIMESTART__PRIMARY_VG_INNER_TABLE_MERGE__AFTER_DUPLICATES_REMOVED / COLUMN_TYPE__DATETIMEEND__PRIMARY_VG_INNER_TABLE_MERGE__AFTER_DUPLICATES_REMOVED
		primary_group_final_results.push_back(primary_group_final_result);
		++primary_group_number;
	});

}

OutputModel::OutputGenerator::SqlAndColumnSet OutputModel::OutputGenerator::ConstructFullOutputForSinglePrimaryGroup(ColumnsInTempView const &
		primary_variable_group_raw_data_columns, SqlAndColumnSets & sql_and_column_sets, int const primary_group_number)
{

	std::int64_t raw_rows_count = total_number_incoming_rows[primary_variable_group_raw_data_columns.variable_groups[0]];
	std::int64_t rows_estimate = raw_rows_count;

	boost::format msg_start("Variable group \"%1%\"");
	msg_start % (primary_variable_group_raw_data_columns.variable_groups[0].longhand ? *primary_variable_group_raw_data_columns.variable_groups[0].longhand
				 : primary_variable_group_raw_data_columns.variable_groups[0].code ? *primary_variable_group_raw_data_columns.variable_groups[0].code : std::string());
	messager.AppendKadStatusText(msg_start.str(), this);

	boost::format msg_("Multiplicity 1 - Retrieving data for variable group \"%1%\"");
	msg_ % (primary_variable_group_raw_data_columns.variable_groups[0].longhand ? *primary_variable_group_raw_data_columns.variable_groups[0].longhand
			: primary_variable_group_raw_data_columns.variable_groups[0].code ? *primary_variable_group_raw_data_columns.variable_groups[0].code : std::string());
	UpdateProgressBarToNextStage(msg_.str(), std::string());

	// Just pull raw data, making sure only to pull rows that overlap the time range selected by the end user,
	// and ordering the rows first by primary keys, then by time range.
	// For random sampling, only multiplicity 1 primary keys are included in the sort.
	// For regular output, first multiplicity 1, then multiplicity > 1 are included.
	// In both cases, the time range (start then end) is included last in the sort.
	SqlAndColumnSet x_table_result = CreateInitialPrimaryXTable_OrCount(primary_variable_group_raw_data_columns, primary_group_number, false);
	x_table_result.second.most_recent_sql_statement_executed__index = -1;
	ExecuteSQL(x_table_result);
	sql_and_column_sets.push_back(x_table_result);

	if (failed || CheckCancelled())
	{
		return SqlAndColumnSet();
	}

	if (false)
	{

		// moved to another function called in a different way

	}
	else
	{

		// The only purpose of CreateInitialPrimaryXRTable() is to modify the schema
		// to add an additional pair of time range columns so that the schema is a proper "XR" schema.
		// Nothing more.
		SqlAndColumnSet xr_table_result = CreateInitialPrimaryXRTable(x_table_result.second, primary_group_number);
		xr_table_result.second.most_recent_sql_statement_executed__index = -1;
		ExecuteSQL(xr_table_result);
		ClearTables(sql_and_column_sets);
		sql_and_column_sets.push_back(xr_table_result);

		if (failed || CheckCancelled())
		{
			return SqlAndColumnSet();
		}

		std::int64_t previous_count = ObtainCount(xr_table_result.second);

		// The initial XR table is necessary to clean up the (currently) raw input data,
		// which may contain duplicates.  Perhaps it is also used to ensure the schema is correct for the next stage.
		inner_table_no_multiplicities__with_all_datetime_columns_included__column_count = static_cast<int>
				(xr_table_result.second.columns_in_view.size()); // This class-global variable must be set
		std::string sorting_results_text("Sorting results");
		std::string removing_duplicates_text("Removing duplicates");
		// DETAILED_COMMENTING: To see purpose of SortAndOrRemoveDuplicates(), search for DESCRIPTION_OF_SORT_AND_OR_REMOVE_DUPLICATES_ALGORITHM.
		// But it just does what it says: sorts the rows by primary key, and removes duplicates
		SqlAndColumnSet duplicates_removed = SortAndOrRemoveDuplicates(xr_table_result.second, primary_variable_group_raw_data_columns.variable_groups[0], sorting_results_text,
											 removing_duplicates_text, 1, primary_group_number, sql_and_column_sets, true, OutputModel::OutputGenerator::PRIMARY_VARIABLE_GROUP);

		if (failed || CheckCancelled())
		{
			return SqlAndColumnSet();
		}

		for (int current_multiplicity = 2; current_multiplicity <= highest_multiplicity_primary_uoa; ++current_multiplicity)
		{

			std::int64_t current_count = ObtainCount(primary_variable_group_raw_data_columns);

			if (primary_variable_group_raw_data_columns.variable_groups[0].longhand)
			{
				boost::format msg("Joining multiplicity %1% (%2% rows) with previous merged data for \"%3%\" (%4% rows).");
				msg % current_multiplicity % current_count % *primary_variable_group_raw_data_columns.variable_groups[0].longhand % previous_count;
				messager.SetPerformanceLabel(msg.str().c_str());
			}
			else
			{
				boost::format msg("Joining multiplicity %1% (%2% rows) with previous merged data for %3% (%4% rows).");
				msg % current_multiplicity % current_count % *primary_variable_group_raw_data_columns.variable_groups[0].code % previous_count;
				messager.SetPerformanceLabel(msg.str().c_str());
			}

			std::int64_t number_of_rows_previous = ObtainCount(duplicates_removed.second);
			std::int64_t number_of_rows_new = ObtainCount(primary_variable_group_raw_data_columns);

			boost::format msg_("Multiplicity %2% - Retrieving data for variable group \"%1%\": merging %3% previous with %4% new rows");
			msg_ % (primary_variable_group_raw_data_columns.variable_groups[0].longhand ? *primary_variable_group_raw_data_columns.variable_groups[0].longhand
					: primary_variable_group_raw_data_columns.variable_groups[0].code ? *primary_variable_group_raw_data_columns.variable_groups[0].code : std::string())
			% current_multiplicity % number_of_rows_previous % number_of_rows_new;
			UpdateProgressBarToNextStage(msg_.str(), std::string());
			rows_estimate *= raw_rows_count;




			// ******************************************************************************************************************* //
			// Join with a new multiplicity of data.
			// ******************************************************************************************************************* //
			messager.SetPerformanceLabel("Performing a database join; please be patient...");
			x_table_result = CreatePrimaryXTable(primary_variable_group_raw_data_columns, duplicates_removed.second, current_multiplicity, primary_group_number);
			x_table_result.second.most_recent_sql_statement_executed__index = -1;
			ExecuteSQL(x_table_result);
			messager.SetPerformanceLabel("");
			ClearTables(sql_and_column_sets);
			sql_and_column_sets.push_back(x_table_result);

			if (failed || CheckCancelled())
			{
				return SqlAndColumnSet();
			}




			std::int64_t number_of_rows_to_sort = ObtainCount(x_table_result.second);

			boost::format msg_3("Multiplicity %2% - Splitting rows on time boundaries and removing redundant NULL rows for \"%1%\": %3% rows");
			msg_3 % (primary_variable_group_raw_data_columns.variable_groups[0].longhand ? *primary_variable_group_raw_data_columns.variable_groups[0].longhand
					 : primary_variable_group_raw_data_columns.variable_groups[0].code ? *primary_variable_group_raw_data_columns.variable_groups[0].code : std::string())
			% current_multiplicity % number_of_rows_to_sort;
			UpdateProgressBarToNextStage(msg_3.str(), std::string());


			// ******************************************************************************************************************* //
			// DESCRIPTION_OF_XR_ALGORITHM
			// Merge identical rows, while keeping track of time ranges to handle them properly during the merge.
			// This stage is the most complex.
			// It takes advantage of the fact that the rows are both ordered within themselves
			//    (inner tables from left to right within each row are sorted -
			//     each inner table consisting of a SINGLE multiplicity of the DMU
			//     which has multiplicity greater than 1, along with its
			//     selected secondary key data)
			// and sorted within the table (rows are sorted in ascending order).
			//
			// The merging of rows also splits rows to handle the time range overlap of the new data being joined.
			//
			// Finally, this stage also tracks all rows with the new data being NULL due to either:
			// ... The reordering inside of individual rows of inner tables (from the last step)
			//    that may have caused (potentially double) rows to appear, one of which has NULL in the final inner table,
			// ... Or the merging/splitting of MULTIPLE rows together or apart in this stage due to time range handling.
			//
			// (Note that there are, therefore, TWO stages of the splitting and creation of multiple rows
			// due to time range handling: One resulting from the joining of new data and the splitting of individual rows
			// to handle time range overlap of each individual joined row (the prior stage),
			// and the other to handle time range overlap of adjacent joined rows (in this stage).)
			//
			// Finally, this stage carefully tracks all rows with NULL and NOT NULL in the final inner table along with their time ranges,
			// and when all rows with non-NULL in the final inner table that match on all other primary keys have completed processing,
			// the algorithm does a special step of processing where it checks each tracked NULL row (including time range)
			// against the non-NULL entries over the same time range that match on the keys, and if there is a match
			// found it EXCLUDES the NULL row from the result set; otherwise it INCLUDES it.
			// This is how NewGene handles displaying K-ads that have, in the final output,
			// a smaller maximum count than the K-value selected by the user
			// (i.e., K=6 but there are only 4 countries in the dispute over the time range).
			// (In the latter case, a row with all 4 countries
			// and 2 NULLs should appear; whereas if 5 or more countries are available over the same time range, a row
			// with 4 countries should NOT appear - such a scenario could arise if the new inner table being merged in
			// first adds a 5th country, but then this row gets split by time range such that the 5th country only
			// partially overlaps the previous 4 countries, so the row gets split into two rows, one of which has
			// the NULL in the fifth inner table - this row would then need to possibly be merged with ANOTHER
			// row over the same time range as the NULL row that DOES have a country in the 5th slot).
			// ******************************************************************************************************************* //

			xr_table_result = CreateXRTable(x_table_result.second, current_multiplicity, primary_group_number, OutputModel::OutputGenerator::PRIMARY_VARIABLE_GROUP, 0, current_multiplicity);
			ClearTables(sql_and_column_sets);
			sql_and_column_sets.push_back(xr_table_result);

			if (failed || CheckCancelled())
			{
				return SqlAndColumnSet();
			}

			previous_count = ObtainCount(xr_table_result.second);





			// ******************************************************************************************************************* //
			// DESCRIPTION_OF_SORT_AND_OR_REMOVE_DUPLICATES_ALGORITHM
			// The previous stage handled merging and splitting of rows to handle time range overlap and NULLs.
			// However, it leaves some duplicate rows in its wake and it does not place rows into its result set in sorted order.
			// Perform a final pass, this time once again sorting the rows in the table in ascending order,
			// and then proceeding to step over all rows without doing any merging or splitting, but simply removing duplicate rows
			// that it encounters.
			//
			// The final result of this stage is the final result for this multiplicity, in unformatted form
			// (i.e., with UUID's added to column names, and with extraneous columns present).
			// ******************************************************************************************************************* //
			std::string sorting_rows_text("Sorting rows");
			std::string removing_duplicates_text("Removing duplicates");
			// DETAILED_COMMENTING: To see purpose of SortAndOrRemoveDuplicates(), search for DESCRIPTION_OF_SORT_AND_OR_REMOVE_DUPLICATES_ALGORITHM
			duplicates_removed = SortAndOrRemoveDuplicates(xr_table_result.second, primary_variable_group_raw_data_columns.variable_groups[0], sorting_rows_text, removing_duplicates_text,
								 current_multiplicity, primary_group_number, sql_and_column_sets, true, OutputModel::OutputGenerator::PRIMARY_VARIABLE_GROUP, true);

			if (failed || CheckCancelled())
			{
				return SqlAndColumnSet();
			}

		}

		if (failed || CheckCancelled())
		{
			return SqlAndColumnSet();
		}

	}

	return sql_and_column_sets.back();

}

void OutputModel::OutputGenerator::SavedRowData::Clear()
{
	current_parameter_strings.clear();
	current_parameter_ints.clear();
	current_parameter_floats.clear();
	current_parameter_which_binding_to_use.clear();
	datetime_start = 0;
	datetime_end = 0;
	failed = false;
	indices_of_primary_key_columns.clear();
	is_index_a_primary_key.clear();
	indices_of_primary_key_columns_with_multiplicity_greater_than_1.clear();
	is_index_a_primary_key_with_outer_multiplicity_greater_than_1.clear();
	indices_of_primary_key_columns_with_multiplicity_equal_to_1.clear();
	number_of_columns__in_a_single_inner_table__for_the_columns_only_having_the_dmu_category_with_multiplicity_greater_than_one__but_this_info_is_present_for_all_primary_key_columns =
		0;
	number_of_columns_in_inner_table = 0;
	inner_table_number.clear();
	indices_of_all_columns_in_all_but_final_inner_table.clear();
	indices_of_all_columns_in_final_inner_table.clear();
	is_index_in_all_but_final_inner_table.clear();
	is_index_in_final_inner_table.clear();
	is_index_a_primary_key_in_not_the_final_inner_table.clear();
	is_index_a_primary_key_in_the_final_inner_table.clear();
	is_index_a_primary_key_with_outer_multiplicity_equal_to_1.clear();
	indices_of_all_primary_key_columns_in_all_but_final_inner_table.clear();
	indices_of_all_primary_key_columns_in_final_inner_table.clear();
	is_index_a_secondary_key.clear();
	indices_of_secondary_key_columns.clear();
	indices_of_all_columns.clear();
	number_of_multiplicities = 0;
}

void OutputModel::OutputGenerator::SavedRowData::PopulateFromCurrentRowInDatabase(ColumnsInTempView const & sorted_result_columns, sqlite3_stmt * stmt_result,
	XR_TABLE_CATEGORY const xr_table_category, bool const obtain_rowid)
{

	Clear();

	int datetime_start_column_index_of_possible_duplicate = (int)sorted_result_columns.columns_in_view.size() - 2;
	int datetime_end_column_index_of_possible_duplicate = (int)sorted_result_columns.columns_in_view.size() - 1;
	std::int64_t datetime_start_of_possible_duplicate = sqlite3_column_int64(stmt_result, datetime_start_column_index_of_possible_duplicate);
	std::int64_t datetime_end_of_possible_duplicate = sqlite3_column_int64(stmt_result, datetime_end_column_index_of_possible_duplicate);

	datetime_start = datetime_start_of_possible_duplicate;
	datetime_end = datetime_end_of_possible_duplicate;

	WidgetInstanceIdentifier first_variable_group;

	int current_column = 0;
	bool reached_first_dates = false;
	bool reached_second_dates = false;
	bool on_other_side_of_first_dates = false;
	bool on_other_side_of_second_dates = false;
	int reverse_index_to_final_relevant_date_column = (int)
			sorted_result_columns.columns_in_view.size(); // ensures that even if there is only one inner table, it will match as the final inner table
	std::for_each(sorted_result_columns.columns_in_view.crbegin(),
				  sorted_result_columns.columns_in_view.crend(), [this, &reverse_index_to_final_relevant_date_column, &sorted_result_columns, &reached_first_dates, &reached_second_dates, &on_other_side_of_first_dates, &on_other_side_of_second_dates, &first_variable_group, &current_column](
					  ColumnsInTempView::ColumnInTempView const & possible_duplicate_view_column)
	{
		if (possible_duplicate_view_column.column_type != ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__PRIMARY
			&& possible_duplicate_view_column.column_type != ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__SECONDARY)
		{
			reached_first_dates = true;

			if (on_other_side_of_first_dates && !reached_second_dates)
			{
				reverse_index_to_final_relevant_date_column = current_column;
				reached_second_dates = true;
			}
		}
		else
		{
			if (reached_first_dates)
			{
				on_other_side_of_first_dates = true;
			}

			if (reached_second_dates)
			{
				on_other_side_of_second_dates = true;
			}
		}

		++current_column;
	});

	int column_index_of_start_of_final_inner_table = (int)sorted_result_columns.columns_in_view.size() - reverse_index_to_final_relevant_date_column;

	int column_data_type = 0;
	std::int64_t data_int64 = 0;
	long double data_float = 0.0;
	std::string data_string;
	long double data_long = 0.0;
	current_column = 0;
	reached_first_dates = false;
	on_other_side_of_first_dates = false;
	bool not_first_variable_group = false;
	std::for_each(sorted_result_columns.columns_in_view.cbegin(),
				  sorted_result_columns.columns_in_view.cend(), [this, &xr_table_category, &not_first_variable_group, &column_index_of_start_of_final_inner_table, &sorted_result_columns, &reached_first_dates, &on_other_side_of_first_dates, &first_variable_group, &data_int64, &data_float, &data_string, &data_long, &stmt_result, &column_data_type, &current_column](
					  ColumnsInTempView::ColumnInTempView const & possible_duplicate_view_column)
	{

		inner_table_number.push_back(possible_duplicate_view_column.current_multiplicity__of__current_inner_table__within__current_vg_inner_table_set);

		if (possible_duplicate_view_column.total_outer_multiplicity__in_total_kad__for_current_dmu_category__for_current_variable_group > 1)
		{
			if (possible_duplicate_view_column.total_k_count__within_uoa_corresponding_to_current_variable_group__for_current_dmu_category >
				number_of_columns__in_a_single_inner_table__for_the_columns_only_having_the_dmu_category_with_multiplicity_greater_than_one__but_this_info_is_present_for_all_primary_key_columns)
			{
				number_of_columns__in_a_single_inner_table__for_the_columns_only_having_the_dmu_category_with_multiplicity_greater_than_one__but_this_info_is_present_for_all_primary_key_columns =
					possible_duplicate_view_column.total_k_count__within_uoa_corresponding_to_current_variable_group__for_current_dmu_category;
			}
		}

		if (possible_duplicate_view_column.current_multiplicity__of__current_inner_table__within__current_vg_inner_table_set > number_of_multiplicities)
		{
			number_of_multiplicities = possible_duplicate_view_column.current_multiplicity__of__current_inner_table__within__current_vg_inner_table_set;
		}

		if (failed || OutputModel::OutputGenerator::CheckCancelled())
		{
			return;
		}

		if (current_column == 0)
		{
			first_variable_group = possible_duplicate_view_column.variable_group_associated_with_current_inner_table;
		}

		if (possible_duplicate_view_column.column_type != ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__PRIMARY
			&& possible_duplicate_view_column.column_type != ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__SECONDARY)
		{
			reached_first_dates = true;
		}
		else
		{
			if (reached_first_dates)
			{
				if (!on_other_side_of_first_dates)
				{
					number_of_columns_in_inner_table = current_column;
				}

				on_other_side_of_first_dates = true;
			}
		}

		if (!not_first_variable_group)
		{
			if (sorted_result_columns.not_first_variable_group_column_index != -1)
			{
				if (current_column == sorted_result_columns.not_first_variable_group_column_index)
				{
					not_first_variable_group = true;
				}
			}
			else if (!possible_duplicate_view_column.variable_group_associated_with_current_inner_table.IsEqual(WidgetInstanceIdentifier::EQUALITY_CHECK_TYPE__STRING_CODE,
					 first_variable_group))
			{
				not_first_variable_group = true;
				sorted_result_columns.not_first_variable_group_column_index = current_column;
			}
		}

		{

			bool add_as_column_in_final_inner_table = false;
			bool add_as_column_in_all_but_final_inner_table = false;
			bool add_as_primary_key_column = false;
			bool add_as_primary_key_with_multiplicity_greater_than_1 = false;
			bool add_as_primary_key_with_multiplicity_equal_to_1 = false;
			bool add_as_primary_key_column_in_final_inner_table = false;
			bool add_as_primary_key_column_in_all_but_final_inner_table = false;
			bool add_as_secondary_key_column = false;

			if (current_column >= column_index_of_start_of_final_inner_table)
			{
				add_as_column_in_final_inner_table = true;
			}
			else
			{
				add_as_column_in_all_but_final_inner_table = true;
			}

			if (possible_duplicate_view_column.column_type == ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__PRIMARY)
			{
				if (possible_duplicate_view_column.total_outer_multiplicity__in_total_kad__for_current_dmu_category__for_current_variable_group == 1)
				{
					if (possible_duplicate_view_column.current_multiplicity__of__current_inner_table__within__current_vg_inner_table_set == 1)
					{
						add_as_primary_key_column = true;
					}
				}
				else
				{
					// Only those primary keys we wish to capture in inner tables beyond the first
					// have the value greater than 1.
					add_as_primary_key_column = true;
				}
			}

			if (possible_duplicate_view_column.column_type == ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__PRIMARY)
			{
				if (possible_duplicate_view_column.total_outer_multiplicity__in_total_kad__for_current_dmu_category__for_current_variable_group > 1)
				{
					add_as_primary_key_with_multiplicity_greater_than_1 = true;
				}
			}

			if (possible_duplicate_view_column.column_type == ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__PRIMARY)
			{
				if (possible_duplicate_view_column.total_outer_multiplicity__in_total_kad__for_current_dmu_category__for_current_variable_group == 1)
				{
					if (possible_duplicate_view_column.current_multiplicity__of__current_inner_table__within__current_vg_inner_table_set == 1)
					{
						add_as_primary_key_with_multiplicity_equal_to_1 = true;
					}
				}
			}

			if (possible_duplicate_view_column.column_type == ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__SECONDARY)
			{
				add_as_secondary_key_column = true;
			}

			if (add_as_primary_key_column && add_as_column_in_final_inner_table)
			{
				add_as_primary_key_column_in_final_inner_table = true;
			}

			if (add_as_primary_key_column && add_as_column_in_all_but_final_inner_table)
			{
				add_as_primary_key_column_in_all_but_final_inner_table = true;
			}


			if (add_as_column_in_final_inner_table && !not_first_variable_group)
			{
				is_index_in_final_inner_table.push_back(true);
			}
			else
			{
				is_index_in_final_inner_table.push_back(false);
			}

			if (add_as_column_in_all_but_final_inner_table && !not_first_variable_group)
			{
				is_index_in_all_but_final_inner_table.push_back(true);
			}
			else
			{
				is_index_in_all_but_final_inner_table.push_back(false);
			}

			if (add_as_primary_key_column && !not_first_variable_group)
			{
				is_index_a_primary_key.push_back(true);
			}
			else
			{
				is_index_a_primary_key.push_back(false);
			}

			if (add_as_secondary_key_column)
			{
				is_index_a_secondary_key.push_back(true);
			}
			else
			{
				is_index_a_secondary_key.push_back(false);
			}

			if (add_as_primary_key_with_multiplicity_greater_than_1 && !not_first_variable_group)
			{
				is_index_a_primary_key_with_outer_multiplicity_greater_than_1.push_back(true);
			}
			else
			{
				is_index_a_primary_key_with_outer_multiplicity_greater_than_1.push_back(false);
			}

			if (add_as_primary_key_with_multiplicity_equal_to_1 && !not_first_variable_group)
			{
				is_index_a_primary_key_with_outer_multiplicity_equal_to_1.push_back(true);
			}
			else
			{
				is_index_a_primary_key_with_outer_multiplicity_equal_to_1.push_back(false);
			}

			if (add_as_primary_key_column_in_final_inner_table && !not_first_variable_group)
			{
				is_index_a_primary_key_in_the_final_inner_table.push_back(true);
			}
			else
			{
				is_index_a_primary_key_in_the_final_inner_table.push_back(false);
			}

			if (add_as_primary_key_column_in_all_but_final_inner_table && !not_first_variable_group)
			{
				is_index_a_primary_key_in_not_the_final_inner_table.push_back(true);
			}
			else
			{
				is_index_a_primary_key_in_not_the_final_inner_table.push_back(false);
			}

		}

		bool add_as_secondary_datetime_column = possible_duplicate_view_column.originally_datetime;

		column_data_type = sqlite3_column_type(stmt_result, current_column);

		switch (column_data_type)
		{

			case SQLITE_INTEGER:
				{

					data_int64 = sqlite3_column_int64(stmt_result, current_column);

					std::pair<SQLExecutor::WHICH_BINDING, std::pair<int, int>> binding;

					// Special case!  Construction of final K-ad output, and this is a
					// datetime column that the user has selected as a secondary data column.
					if (xr_table_category == XR_TABLE_CATEGORY::KAD_RESULTS && add_as_secondary_datetime_column)
					{
						boost::posix_time::ptime time_t_epoch__1970(boost::gregorian::date(1970, 1, 1));
						boost::posix_time::ptime time_database = time_t_epoch__1970 + boost::posix_time::milliseconds(data_int64);
						std::string time_formatted = boost::posix_time::to_simple_string(time_database);
						current_parameter_strings.push_back(time_formatted);
						current_parameter_which_binding_to_use.push_back(SQLExecutor::STRING);
						binding = std::make_pair(SQLExecutor::STRING, std::make_pair((int)current_parameter_strings.size() - 1, current_column));
					}
					else
					{
						current_parameter_ints.push_back(data_int64);
						current_parameter_which_binding_to_use.push_back(SQLExecutor::INT64);
						binding = std::make_pair(SQLExecutor::INT64, std::make_pair((int)current_parameter_ints.size() - 1, current_column));
					}

					indices_of_all_columns.push_back(binding);

					if (is_index_in_all_but_final_inner_table[current_column])
					{
						indices_of_all_columns_in_all_but_final_inner_table.push_back(binding);
					}

					if (is_index_in_final_inner_table[current_column])
					{
						indices_of_all_columns_in_final_inner_table.push_back(binding);
					}

					if (is_index_a_primary_key_in_not_the_final_inner_table[current_column])
					{
						indices_of_all_primary_key_columns_in_all_but_final_inner_table.push_back(binding);
					}

					if (is_index_a_primary_key_in_the_final_inner_table[current_column])
					{
						indices_of_all_primary_key_columns_in_final_inner_table.push_back(binding);
					}

					if (is_index_a_primary_key[current_column])
					{
						indices_of_primary_key_columns.push_back(binding);
					}

					if (is_index_a_primary_key_with_outer_multiplicity_greater_than_1[current_column])
					{
						indices_of_primary_key_columns_with_multiplicity_greater_than_1.push_back(binding);
					}

					if (is_index_a_primary_key_with_outer_multiplicity_equal_to_1[current_column])
					{
						indices_of_primary_key_columns_with_multiplicity_equal_to_1.push_back(binding);
					}

					if (is_index_a_secondary_key[current_column])
					{
						indices_of_secondary_key_columns.push_back(binding);
					}

				}
				break;

			case SQLITE_FLOAT:
				{

					data_float = sqlite3_column_double(stmt_result, current_column);
					current_parameter_floats.push_back(data_float);
					current_parameter_which_binding_to_use.push_back(SQLExecutor::FLOAT);

					indices_of_all_columns.push_back(std::make_pair(SQLExecutor::FLOAT, std::make_pair((int)current_parameter_floats.size() - 1, current_column)));

					if (is_index_in_all_but_final_inner_table[current_column])
					{
						indices_of_all_columns_in_all_but_final_inner_table.push_back(std::make_pair(SQLExecutor::FLOAT, std::make_pair((int)current_parameter_floats.size() - 1, current_column)));
					}

					if (is_index_in_final_inner_table[current_column])
					{
						indices_of_all_columns_in_final_inner_table.push_back(std::make_pair(SQLExecutor::FLOAT, std::make_pair((int)current_parameter_floats.size() - 1, current_column)));
					}

					if (is_index_a_primary_key_in_not_the_final_inner_table[current_column])
					{
						indices_of_all_primary_key_columns_in_all_but_final_inner_table.push_back(std::make_pair(SQLExecutor::FLOAT, std::make_pair((int)current_parameter_floats.size() - 1,
								current_column)));
					}

					if (is_index_a_primary_key_in_the_final_inner_table[current_column])
					{
						indices_of_all_primary_key_columns_in_final_inner_table.push_back(std::make_pair(SQLExecutor::FLOAT, std::make_pair((int)current_parameter_floats.size() - 1, current_column)));
					}

					if (is_index_a_primary_key[current_column])
					{
						indices_of_primary_key_columns.push_back(std::make_pair(SQLExecutor::FLOAT, std::make_pair((int)current_parameter_floats.size() - 1, current_column)));
					}

					if (is_index_a_primary_key_with_outer_multiplicity_greater_than_1[current_column])
					{
						indices_of_primary_key_columns_with_multiplicity_greater_than_1.push_back(std::make_pair(OutputModel::OutputGenerator::SQLExecutor::FLOAT,
								std::make_pair((int)current_parameter_floats.size() - 1, current_column)));
					}

					if (is_index_a_primary_key_with_outer_multiplicity_equal_to_1[current_column])
					{
						indices_of_primary_key_columns_with_multiplicity_equal_to_1.push_back(std::make_pair(OutputModel::OutputGenerator::SQLExecutor::FLOAT,
								std::make_pair((int)current_parameter_floats.size() - 1, current_column)));
					}

					if (is_index_a_secondary_key[current_column])
					{
						indices_of_secondary_key_columns.push_back(std::make_pair(SQLExecutor::FLOAT, std::make_pair((int)current_parameter_floats.size() - 1, current_column)));
					}

				}
				break;

			case SQLITE_TEXT:
				{

					data_string = reinterpret_cast<char const *>(sqlite3_column_text(stmt_result, current_column));

					// Special case!  Construction of final K-ad output, so commas need to be removed
					if (xr_table_category == XR_TABLE_CATEGORY::KAD_RESULTS)
					{
						std::replace(data_string.begin(), data_string.end(), ',', ' ');
					}

					current_parameter_strings.push_back(data_string);
					current_parameter_which_binding_to_use.push_back(SQLExecutor::STRING);

					indices_of_all_columns.push_back(std::make_pair(SQLExecutor::STRING, std::make_pair((int)current_parameter_strings.size() - 1, current_column)));

					if (is_index_in_all_but_final_inner_table[current_column])
					{
						indices_of_all_columns_in_all_but_final_inner_table.push_back(std::make_pair(SQLExecutor::STRING, std::make_pair((int)current_parameter_strings.size() - 1, current_column)));
					}

					if (is_index_in_final_inner_table[current_column])
					{
						indices_of_all_columns_in_final_inner_table.push_back(std::make_pair(SQLExecutor::STRING, std::make_pair((int)current_parameter_strings.size() - 1, current_column)));
					}

					if (is_index_a_primary_key_in_not_the_final_inner_table[current_column])
					{
						indices_of_all_primary_key_columns_in_all_but_final_inner_table.push_back(std::make_pair(SQLExecutor::STRING, std::make_pair((int)current_parameter_strings.size() - 1,
								current_column)));
					}

					if (is_index_a_primary_key_in_the_final_inner_table[current_column])
					{
						indices_of_all_primary_key_columns_in_final_inner_table.push_back(std::make_pair(SQLExecutor::STRING, std::make_pair((int)current_parameter_strings.size() - 1, current_column)));
					}

					if (is_index_a_primary_key[current_column])
					{
						indices_of_primary_key_columns.push_back(std::make_pair(SQLExecutor::STRING, std::make_pair((int)current_parameter_strings.size() - 1, current_column)));
					}

					if (is_index_a_primary_key_with_outer_multiplicity_greater_than_1[current_column])
					{
						indices_of_primary_key_columns_with_multiplicity_greater_than_1.push_back(std::make_pair(OutputModel::OutputGenerator::SQLExecutor::STRING,
								std::make_pair((int)current_parameter_strings.size() - 1, current_column)));
					}

					if (is_index_a_primary_key_with_outer_multiplicity_equal_to_1[current_column])
					{
						indices_of_primary_key_columns_with_multiplicity_equal_to_1.push_back(std::make_pair(OutputModel::OutputGenerator::SQLExecutor::STRING,
								std::make_pair((int)current_parameter_strings.size() - 1, current_column)));
					}

					if (is_index_a_secondary_key[current_column])
					{
						indices_of_secondary_key_columns.push_back(std::make_pair(SQLExecutor::STRING, std::make_pair((int)current_parameter_strings.size() - 1, current_column)));
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

					indices_of_all_columns.push_back(std::make_pair(SQLExecutor::NULL_BINDING, std::make_pair(0, current_column)));

					if (is_index_in_all_but_final_inner_table[current_column])
					{
						indices_of_all_columns_in_all_but_final_inner_table.push_back(std::make_pair(OutputModel::OutputGenerator::SQLExecutor::NULL_BINDING, std::make_pair(0, current_column)));
					}

					if (is_index_in_final_inner_table[current_column])
					{
						indices_of_all_columns_in_final_inner_table.push_back(std::make_pair(OutputModel::OutputGenerator::SQLExecutor::NULL_BINDING, std::make_pair(0, current_column)));
					}

					if (is_index_a_primary_key_in_not_the_final_inner_table[current_column])
					{
						indices_of_all_primary_key_columns_in_all_but_final_inner_table.push_back(std::make_pair(SQLExecutor::NULL_BINDING, std::make_pair(0, current_column)));
					}

					if (is_index_a_primary_key_in_the_final_inner_table[current_column])
					{
						indices_of_all_primary_key_columns_in_final_inner_table.push_back(std::make_pair(SQLExecutor::NULL_BINDING, std::make_pair(0, current_column)));
					}

					if (is_index_a_primary_key[current_column])
					{
						indices_of_primary_key_columns.push_back(std::make_pair(SQLExecutor::NULL_BINDING, std::make_pair(0, current_column)));
					}

					if (is_index_a_primary_key_with_outer_multiplicity_greater_than_1[current_column])
					{
						indices_of_primary_key_columns_with_multiplicity_greater_than_1.push_back(std::make_pair(OutputModel::OutputGenerator::SQLExecutor::NULL_BINDING, std::make_pair(0,
								current_column)));
					}

					if (is_index_a_primary_key_with_outer_multiplicity_equal_to_1[current_column])
					{
						indices_of_primary_key_columns_with_multiplicity_equal_to_1.push_back(std::make_pair(OutputModel::OutputGenerator::SQLExecutor::NULL_BINDING, std::make_pair(0, current_column)));
					}

					if (is_index_a_secondary_key[current_column])
					{
						indices_of_secondary_key_columns.push_back(std::make_pair(SQLExecutor::NULL_BINDING, std::make_pair(0, current_column)));
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

	if (obtain_rowid)
	{
		data_int64 = sqlite3_column_int64(stmt_result, current_column);
		rowid = data_int64;
	}

	if (sorted_result_columns.not_first_variable_group_column_index == -1)
	{
		// ALL columns are in the first variable group - set this index one past the end
		sorted_result_columns.not_first_variable_group_column_index = (int)sorted_result_columns.columns_in_view.size();
	}

	if (number_of_columns_in_inner_table == 0)
	{
		number_of_columns_in_inner_table = (int)sorted_result_columns.columns_in_view.size();
	}

}

OutputModel::OutputGenerator::SqlAndColumnSet OutputModel::OutputGenerator::RemoveDuplicates_Or_OrderWithinRows(ColumnsInTempView const & previous_result_columns,
		int const primary_group_number, std::int64_t & current_rows_added, int const current_multiplicity, XR_TABLE_CATEGORY const xr_table_category,
		bool const consider_merging_timerange_adjacent_identical_rows)
{

	SqlAndColumnSet result = std::make_pair(std::vector<SQLExecutor>(), ColumnsInTempView());
	std::vector<SQLExecutor> & sql_strings = result.first;
	ColumnsInTempView & result_columns = result.second;

	result_columns = previous_result_columns;
	result_columns.most_recent_sql_statement_executed__index = -1;

	std::string view_name;

	switch (xr_table_category)
	{
		case OutputModel::OutputGenerator::PRIMARY_VARIABLE_GROUP:
			{
				view_name += "NGTEMP_DR";
			}
			break;

		case OutputModel::OutputGenerator::CHILD_VARIABLE_GROUP:
			{
				view_name += "NGTEMP_KAD";
			}
			break;

		case OutputModel::OutputGenerator::FINAL_MERGE_OF_PRIMARY_VARIABLE_GROUP:
			{
				view_name += "NGTEMP_FM";
			}
			break;

		case OutputModel::OutputGenerator::KAD_RESULTS:
			{
				view_name += "NGTEMP_KKA";
			}
			break;
	}

	view_name += std::to_string(primary_group_number);
	result_columns.view_name_no_uuid = view_name;

	if (current_multiplicity >= 0)
	{
		view_name += "_";
		view_name += std::to_string(current_multiplicity);
	}

	view_name += "_";
	view_name += newUUID(true);

	if (current_multiplicity >= 0)
	{
		view_name += "_a";
		view_name += std::to_string(primary_group_number);
	}

	result_columns.view_name = view_name;
	result_columns.view_number = 1;
	result_columns.has_no_datetime_columns = false;

	std::string sql_create_empty_table;
	sql_create_empty_table += "CREATE TABLE \"";
	sql_create_empty_table += result_columns.view_name;
	sql_create_empty_table += "\" AS SELECT * FROM \"";
	sql_create_empty_table += previous_result_columns.view_name;
	sql_create_empty_table += "\" WHERE 0";
	sql_strings.push_back(SQLExecutor(this, db, sql_create_empty_table));

	if (failed)
	{
		SetFailureMessage(sql_error);
		return result;
	}

	if (CheckCancelled())
	{
		return result;
	}


	// Add the "merged" time range columns

	// The variable group is that of the primary variable group for this final result set,
	// which is obtained from the first column
	WidgetInstanceIdentifier variable_group = previous_result_columns.columns_in_view[0].variable_group_associated_with_current_inner_table;
	WidgetInstanceIdentifier uoa = previous_result_columns.columns_in_view[0].uoa_associated_with_variable_group_associated_with_current_inner_table;

	std::string datetime_start_col_name;
	std::string datetime_end_col_name;
	std::string datetime_start_col_name_text;
	std::string datetime_end_col_name_text;

	if (xr_table_category == OutputModel::OutputGenerator::KAD_RESULTS)
	{
		// Final result - add two TEXT columns to store text representation of the datetime
		std::string datetime_start_col_name_no_uuid;
		datetime_start_col_name_no_uuid = "DATETIME_START_TEXT";
		datetime_start_col_name_text = datetime_start_col_name_no_uuid;
		datetime_start_col_name_text += "_";
		datetime_start_col_name_text += newUUID(true);

		std::string alter_string;
		alter_string += "ALTER TABLE \"";
		alter_string += result_columns.view_name;
		alter_string += "\" ADD COLUMN ";
		alter_string += datetime_start_col_name_text;
		alter_string += " TEXT";

		sql_strings.push_back(SQLExecutor(this, db, alter_string));

		if (failed)
		{
			SetFailureMessage(sql_error);
			return result;
		}

		if (CheckCancelled())
		{
			return result;
		}

		result_columns.columns_in_view.push_back(ColumnsInTempView::ColumnInTempView());
		ColumnsInTempView::ColumnInTempView & datetime_start_column = result_columns.columns_in_view.back();
		datetime_start_column.column_name_in_temporary_table = datetime_start_col_name_text;
		datetime_start_column.column_name_in_temporary_table_no_uuid = datetime_start_col_name_no_uuid;
		datetime_start_column.current_multiplicity__of__current_inner_table__within__current_vg_inner_table_set =
			previous_result_columns.columns_in_view.back().current_multiplicity__of__current_inner_table__within__current_vg_inner_table_set;
		datetime_start_column.number_inner_tables_in_set = previous_result_columns.columns_in_view.back().number_inner_tables_in_set;

		datetime_start_column.column_type = ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__DATETIMESTART_TEXT;
		datetime_start_column.variable_group_associated_with_current_inner_table =
			previous_result_columns.columns_in_view[previous_result_columns.columns_in_view.size() - 2].variable_group_associated_with_current_inner_table;
		datetime_start_column.uoa_associated_with_variable_group_associated_with_current_inner_table =
			*previous_result_columns.columns_in_view[previous_result_columns.columns_in_view.size() - 2].variable_group_associated_with_current_inner_table.identifier_parent;
		datetime_start_column.is_within_inner_table_corresponding_to_top_level_uoa = false;
		datetime_start_column.column_name_in_original_data_table = "";

		std::string datetime_end_col_name_no_uuid;
		datetime_end_col_name_no_uuid = "DATETIME_END_TEXT";
		datetime_end_col_name_text = datetime_end_col_name_no_uuid;
		datetime_end_col_name_text += "_";
		datetime_end_col_name_text += newUUID(true);

		alter_string.clear();
		alter_string += "ALTER TABLE \"";
		alter_string += result_columns.view_name;
		alter_string += "\" ADD COLUMN ";
		alter_string += datetime_end_col_name_text;
		alter_string += " TEXT";

		sql_strings.push_back(SQLExecutor(this, db, alter_string));

		if (failed)
		{
			SetFailureMessage(sql_error);
			return result;
		}

		if (CheckCancelled())
		{
			return result;
		}

		result_columns.columns_in_view.push_back(ColumnsInTempView::ColumnInTempView());
		ColumnsInTempView::ColumnInTempView & datetime_end_column = result_columns.columns_in_view.back();
		datetime_end_column.column_name_in_temporary_table = datetime_end_col_name_text;
		datetime_end_column.column_name_in_temporary_table_no_uuid = datetime_end_col_name_no_uuid;
		datetime_end_column.current_multiplicity__of__current_inner_table__within__current_vg_inner_table_set =
			previous_result_columns.columns_in_view.back().current_multiplicity__of__current_inner_table__within__current_vg_inner_table_set;
		datetime_end_column.number_inner_tables_in_set = previous_result_columns.columns_in_view.back().number_inner_tables_in_set;
		datetime_end_column.column_type = ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__DATETIMEEND_TEXT;
		datetime_end_column.variable_group_associated_with_current_inner_table =
			previous_result_columns.columns_in_view[previous_result_columns.columns_in_view.size() - 2].variable_group_associated_with_current_inner_table;
		datetime_end_column.uoa_associated_with_variable_group_associated_with_current_inner_table =
			*previous_result_columns.columns_in_view[previous_result_columns.columns_in_view.size() - 2].variable_group_associated_with_current_inner_table.identifier_parent;
		datetime_end_column.is_within_inner_table_corresponding_to_top_level_uoa = false;
		datetime_end_column.column_name_in_original_data_table = "";
	}

	std::string datetime_start_col_name_no_uuid;

	switch (xr_table_category)
	{
		case OutputModel::OutputGenerator::PRIMARY_VARIABLE_GROUP:
			{
				datetime_start_col_name_no_uuid = "DATETIMESTART__PRIMARY_VG_INNER_TABLE_MERGE__AFTER_DUPLICATES_REMOVED";
			}
			break;

		case OutputModel::OutputGenerator::CHILD_VARIABLE_GROUP:
			{
				datetime_start_col_name_no_uuid = "DATETIMESTART_PRE_MERGED_KAD_OUTPUT";
			}
			break;

		case OutputModel::OutputGenerator::FINAL_MERGE_OF_PRIMARY_VARIABLE_GROUP:
			{
				datetime_start_col_name_no_uuid = "DATETIMESTART__POST_TIMERANGE_MERGED_BETWEEN_TOP_LEVEL_PRIMARY_VARIABLE_GROUPS";
			}
			break;

		case OutputModel::OutputGenerator::KAD_RESULTS:
			{
				datetime_start_col_name_no_uuid = "DATETIMESTART_PRE_MERGED_KAD_OUTPUT";
			}
			break;
	}

	datetime_start_col_name = datetime_start_col_name_no_uuid;
	datetime_start_col_name += "_";
	datetime_start_col_name += newUUID(true);

	std::string alter_string;
	alter_string += "ALTER TABLE \"";
	alter_string += result_columns.view_name;
	alter_string += "\" ADD COLUMN ";
	alter_string += datetime_start_col_name;

	switch (xr_table_category)
	{
		case OutputModel::OutputGenerator::PRIMARY_VARIABLE_GROUP:
			{
				alter_string += " INTEGER DEFAULT 0";
			}
			break;

		case OutputModel::OutputGenerator::CHILD_VARIABLE_GROUP:
			{
				alter_string += " INTEGER DEFAULT 0";
			}
			break;

		case OutputModel::OutputGenerator::KAD_RESULTS:
			{
				alter_string += " INTEGER DEFAULT 0";
			}
			break;

		case OutputModel::OutputGenerator::FINAL_MERGE_OF_PRIMARY_VARIABLE_GROUP:
			{
				alter_string += " INTEGER DEFAULT 0";
			}
			break;
	}

	sql_strings.push_back(SQLExecutor(this, db, alter_string));

	if (failed)
	{
		SetFailureMessage(sql_error);
		return result;
	}

	if (CheckCancelled())
	{
		return result;
	}

	result_columns.columns_in_view.push_back(ColumnsInTempView::ColumnInTempView());
	ColumnsInTempView::ColumnInTempView & datetime_start_column = result_columns.columns_in_view.back();
	datetime_start_column.column_name_in_temporary_table = datetime_start_col_name;
	datetime_start_column.column_name_in_temporary_table_no_uuid = datetime_start_col_name_no_uuid;
	datetime_start_column.current_multiplicity__of__current_inner_table__within__current_vg_inner_table_set =
		previous_result_columns.columns_in_view.back().current_multiplicity__of__current_inner_table__within__current_vg_inner_table_set;
	datetime_start_column.number_inner_tables_in_set = previous_result_columns.columns_in_view.back().number_inner_tables_in_set;

	switch (xr_table_category)
	{
		case OutputModel::OutputGenerator::PRIMARY_VARIABLE_GROUP:
			{
				datetime_start_column.column_type = ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__DATETIMESTART__PRIMARY_VG_INNER_TABLE_MERGE__AFTER_DUPLICATES_REMOVED;
				datetime_start_column.variable_group_associated_with_current_inner_table = variable_group;
				datetime_start_column.uoa_associated_with_variable_group_associated_with_current_inner_table = uoa;
				datetime_start_column.is_within_inner_table_corresponding_to_top_level_uoa = true;
			}
			break;

		case OutputModel::OutputGenerator::CHILD_VARIABLE_GROUP:
			{
				datetime_start_column.column_type = ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__DATETIMESTART_PRE_MERGED_KAD_OUTPUT;
				datetime_start_column.variable_group_associated_with_current_inner_table =
					previous_result_columns.columns_in_view[previous_result_columns.columns_in_view.size() - 2].variable_group_associated_with_current_inner_table;
				datetime_start_column.uoa_associated_with_variable_group_associated_with_current_inner_table =
					*previous_result_columns.columns_in_view[previous_result_columns.columns_in_view.size() - 2].variable_group_associated_with_current_inner_table.identifier_parent;
				datetime_start_column.is_within_inner_table_corresponding_to_top_level_uoa = false;
			}
			break;

		case OutputModel::OutputGenerator::KAD_RESULTS:
			{
				datetime_start_column.column_type = ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__DATETIMESTART_MERGED_KAD_OUTPUT;
				datetime_start_column.variable_group_associated_with_current_inner_table =
					previous_result_columns.columns_in_view[previous_result_columns.columns_in_view.size() - 2].variable_group_associated_with_current_inner_table;
				datetime_start_column.uoa_associated_with_variable_group_associated_with_current_inner_table =
					*previous_result_columns.columns_in_view[previous_result_columns.columns_in_view.size() - 2].variable_group_associated_with_current_inner_table.identifier_parent;
				datetime_start_column.is_within_inner_table_corresponding_to_top_level_uoa = false;
			}
			break;

		case OutputModel::OutputGenerator::FINAL_MERGE_OF_PRIMARY_VARIABLE_GROUP:
			{
				datetime_start_column.column_type = ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__DATETIMESTART__POST_TIMERANGE_MERGED_BETWEEN_TOP_LEVEL_PRIMARY_VARIABLE_GROUPS;
				datetime_start_column.variable_group_associated_with_current_inner_table =
					previous_result_columns.columns_in_view[previous_result_columns.columns_in_view.size() - 2].variable_group_associated_with_current_inner_table;
				datetime_start_column.uoa_associated_with_variable_group_associated_with_current_inner_table =
					*previous_result_columns.columns_in_view[previous_result_columns.columns_in_view.size() - 2].variable_group_associated_with_current_inner_table.identifier_parent;
				datetime_start_column.is_within_inner_table_corresponding_to_top_level_uoa = true;
			}
			break;
	}

	datetime_start_column.column_name_in_original_data_table = "";

	std::string datetime_end_col_name_no_uuid;

	switch (xr_table_category)
	{
		case OutputModel::OutputGenerator::PRIMARY_VARIABLE_GROUP:
			{
				datetime_end_col_name_no_uuid = "DATETIMEEND__PRIMARY_VG_INNER_TABLE_MERGE__AFTER_DUPLICATES_REMOVED";
			}
			break;

		case OutputModel::OutputGenerator::CHILD_VARIABLE_GROUP:
			{
				datetime_end_col_name_no_uuid = "DATETIMEEND_PRE_MERGED_KAD_OUTPUT";
			}
			break;

		case OutputModel::OutputGenerator::KAD_RESULTS:
			{
				datetime_end_col_name_no_uuid = "DATETIMEEND_PRE_MERGED_KAD_OUTPUT";
			}
			break;

		case OutputModel::OutputGenerator::FINAL_MERGE_OF_PRIMARY_VARIABLE_GROUP:
			{
				datetime_end_col_name_no_uuid = "DATETIMEEND__POST_TIMERANGE_MERGED_BETWEEN_TOP_LEVEL_PRIMARY_VARIABLE_GROUPS";
			}
			break;
	}

	datetime_end_col_name = datetime_end_col_name_no_uuid;
	datetime_end_col_name += "_";
	datetime_end_col_name += newUUID(true);

	alter_string.clear();
	alter_string += "ALTER TABLE \"";
	alter_string += result_columns.view_name;
	alter_string += "\" ADD COLUMN ";
	alter_string += datetime_end_col_name;

	switch (xr_table_category)
	{
		case OutputModel::OutputGenerator::PRIMARY_VARIABLE_GROUP:
			{
				alter_string += " INTEGER DEFAULT 0";
			}
			break;

		case OutputModel::OutputGenerator::CHILD_VARIABLE_GROUP:
			{
				alter_string += " INTEGER DEFAULT 0";
			}
			break;

		case OutputModel::OutputGenerator::KAD_RESULTS:
			{
				alter_string += " INTEGER DEFAULT 0";
			}
			break;

		case OutputModel::OutputGenerator::FINAL_MERGE_OF_PRIMARY_VARIABLE_GROUP:
			{
				alter_string += " INTEGER DEFAULT 0";
			}
			break;
	}

	sql_strings.push_back(SQLExecutor(this, db, alter_string));

	if (failed)
	{
		SetFailureMessage(sql_error);
		return result;
	}

	if (CheckCancelled())
	{
		return result;
	}

	result_columns.columns_in_view.push_back(ColumnsInTempView::ColumnInTempView());
	ColumnsInTempView::ColumnInTempView & datetime_end_column = result_columns.columns_in_view.back();
	datetime_end_column.column_name_in_temporary_table = datetime_end_col_name;
	datetime_end_column.column_name_in_temporary_table_no_uuid = datetime_end_col_name_no_uuid;
	datetime_end_column.current_multiplicity__of__current_inner_table__within__current_vg_inner_table_set =
		previous_result_columns.columns_in_view.back().current_multiplicity__of__current_inner_table__within__current_vg_inner_table_set;
	datetime_end_column.number_inner_tables_in_set = previous_result_columns.columns_in_view.back().number_inner_tables_in_set;

	switch (xr_table_category)
	{
		case OutputModel::OutputGenerator::PRIMARY_VARIABLE_GROUP:
			{
				datetime_end_column.column_type = ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__DATETIMEEND__PRIMARY_VG_INNER_TABLE_MERGE__AFTER_DUPLICATES_REMOVED;
				datetime_end_column.variable_group_associated_with_current_inner_table = variable_group;
				datetime_end_column.uoa_associated_with_variable_group_associated_with_current_inner_table = uoa;
				datetime_end_column.is_within_inner_table_corresponding_to_top_level_uoa = true;
				result_columns.current_block_datetime_column_types = std::make_pair(
							ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__DATETIMESTART__PRIMARY_VG_INNER_TABLE_MERGE__AFTER_DUPLICATES_REMOVED,
							ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__DATETIMEEND__PRIMARY_VG_INNER_TABLE_MERGE__AFTER_DUPLICATES_REMOVED);
				result_columns.previous_block_datetime_column_types = result_columns.current_block_datetime_column_types;
			}
			break;

		case OutputModel::OutputGenerator::CHILD_VARIABLE_GROUP:
			{
				datetime_end_column.column_type = ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__DATETIMEEND_PRE_MERGED_KAD_OUTPUT;
				datetime_end_column.variable_group_associated_with_current_inner_table =
					previous_result_columns.columns_in_view[previous_result_columns.columns_in_view.size() - 2].variable_group_associated_with_current_inner_table;
				datetime_end_column.uoa_associated_with_variable_group_associated_with_current_inner_table =
					*previous_result_columns.columns_in_view[previous_result_columns.columns_in_view.size() - 2].variable_group_associated_with_current_inner_table.identifier_parent;
				datetime_end_column.is_within_inner_table_corresponding_to_top_level_uoa = false;
				result_columns.current_block_datetime_column_types = std::make_pair(ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__DATETIMESTART_PRE_MERGED_KAD_OUTPUT,
						ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__DATETIMEEND_PRE_MERGED_KAD_OUTPUT);
			}
			break;

		case OutputModel::OutputGenerator::KAD_RESULTS:
			{
				datetime_end_column.column_type = ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__DATETIMEEND_MERGED_KAD_OUTPUT;
				datetime_end_column.variable_group_associated_with_current_inner_table =
					previous_result_columns.columns_in_view[previous_result_columns.columns_in_view.size() - 2].variable_group_associated_with_current_inner_table;
				datetime_end_column.uoa_associated_with_variable_group_associated_with_current_inner_table =
					*previous_result_columns.columns_in_view[previous_result_columns.columns_in_view.size() - 2].variable_group_associated_with_current_inner_table.identifier_parent;
				datetime_end_column.is_within_inner_table_corresponding_to_top_level_uoa = false;
				result_columns.current_block_datetime_column_types = std::make_pair(ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__DATETIMESTART_MERGED_KAD_OUTPUT,
						ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__DATETIMEEND_MERGED_KAD_OUTPUT);
			}
			break;

		case OutputModel::OutputGenerator::FINAL_MERGE_OF_PRIMARY_VARIABLE_GROUP:
			{
				datetime_end_column.column_type = ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__DATETIMEEND__POST_TIMERANGE_MERGED_BETWEEN_TOP_LEVEL_PRIMARY_VARIABLE_GROUPS;
				datetime_end_column.variable_group_associated_with_current_inner_table =
					previous_result_columns.columns_in_view[previous_result_columns.columns_in_view.size() - 2].variable_group_associated_with_current_inner_table;
				datetime_end_column.uoa_associated_with_variable_group_associated_with_current_inner_table =
					*previous_result_columns.columns_in_view[previous_result_columns.columns_in_view.size() - 2].variable_group_associated_with_current_inner_table.identifier_parent;
				datetime_end_column.is_within_inner_table_corresponding_to_top_level_uoa = true;
				result_columns.current_block_datetime_column_types = std::make_pair(
							ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__DATETIMESTART__POST_TIMERANGE_MERGED_BETWEEN_TOP_LEVEL_PRIMARY_VARIABLE_GROUPS,
							ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__DATETIMEEND__POST_TIMERANGE_MERGED_BETWEEN_TOP_LEVEL_PRIMARY_VARIABLE_GROUPS);
			}
			break;
	}

	result_columns.previous_block_datetime_column_types = result_columns.current_block_datetime_column_types;
	datetime_end_column.column_name_in_original_data_table = "";

	ExecuteSQL(result); // Executes all SQL queries up to the current one

	if (failed || CheckCancelled())
	{
		return result;
	}


	int const minimum_desired_rows_per_transaction = 1024 * 16;
	current_rows_added = 0;
	std::int64_t current_rows_stepped = 0;
	std::int64_t current_rows_added_since_execution = 0;
	std::string sql_add_xr_row;
	bool first_row_added = true;
	std::vector<std::string> bound_parameter_strings;
	std::vector<std::int64_t> bound_parameter_ints;
	std::vector<long double> bound_parameter_floats;
	std::vector<SQLExecutor::WHICH_BINDING> bound_parameter_which_binding_to_use;


	sqlite3_stmt *& the_prepared_stmt = SQLExecutor::stmt_insert;
	std::shared_ptr<bool> statement_is_prepared(std::make_shared<bool>(false));
	SQLExecutor::stmt_insert = nullptr;
	sqlite3_stmt *& the_stmt__ = SQLExecutor::stmt_insert;
	BOOST_SCOPE_EXIT(&the_prepared_stmt, &statement_is_prepared, &the_stmt__)
	{
		if (the_prepared_stmt && *statement_is_prepared)
		{
			sqlite3_finalize(the_prepared_stmt);
			++SQLExecutor::number_statement_finalizes;
			the_prepared_stmt = nullptr;
			*statement_is_prepared = false;
		}

		the_stmt__ = nullptr;
	} BOOST_SCOPE_EXIT_END

	{

		BOOST_SCOPE_EXIT(this_)
		{
			this_->CloseObtainData();
		} BOOST_SCOPE_EXIT_END

		ObtainData(previous_result_columns);

		if (failed || CheckCancelled())
		{
			return result;
		}

		BeginNewTransaction();

		std::deque<SavedRowData> rows_to_sort;
		SavedRowData sorting_row_of_data;
		std::deque<SavedRowData> ordering_within_rows_data;

		bool start_fresh = true;

		int which_previous_row_index_to_test_against = 0;
		bool use_newest_row_index = false;

		while (StepData())
		{

			sorting_row_of_data.PopulateFromCurrentRowInDatabase(previous_result_columns, stmt_result, xr_table_category);

			failed = sorting_row_of_data.failed;

			if (failed)
			{
				SetFailureMessage(sorting_row_of_data.error_message);
				return result;
			}

			if (CheckCancelled())
			{
				return result;
			}

			if (start_fresh)
			{
				rows_to_sort.push_back(sorting_row_of_data);
				++current_rows_stepped;
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
			use_newest_row_index = false;
			bool primary_keys_match = TestPrimaryKeyMatch(sorting_row_of_data, rows_to_sort[which_previous_row_index_to_test_against], use_newest_row_index,
									  PRIMARY_KEY_MATCH_CONDITION::MATCH_ON_ALL_KEYS);

			if (failed || CheckCancelled())
			{
				break;
			}

			if (primary_keys_match)
			{
				rows_to_sort.push_back(sorting_row_of_data);

				if (use_newest_row_index)
				{
					which_previous_row_index_to_test_against = (int)rows_to_sort.size() - 1;
					use_newest_row_index = false;
				}
			}
			else
			{

				// ******************************************************************************************************************* //
				// Sort all rows that match on primary keys by time range,
				// and split them as necessary to handle time range overlap.
				// ******************************************************************************************************************* //

				use_newest_row_index = false;
				RemoveDuplicatesFromPrimaryKeyMatches(current_rows_stepped, result, rows_to_sort, datetime_start_col_name, datetime_end_col_name, statement_is_prepared, the_prepared_stmt,
													  sql_strings, result_columns, previous_result_columns, current_rows_added, current_rows_added_since_execution, sql_add_xr_row, first_row_added, bound_parameter_strings,
													  bound_parameter_ints, bound_parameter_floats, bound_parameter_which_binding_to_use, minimum_desired_rows_per_transaction, xr_table_category,
													  consider_merging_timerange_adjacent_identical_rows, datetime_start_col_name_text, datetime_end_col_name_text);
				which_previous_row_index_to_test_against = 0;

				if (failed || CheckCancelled())
				{
					return result;
				}

				rows_to_sort.push_back(sorting_row_of_data);
			}

			++current_rows_stepped;

			if (current_rows_stepped % 100 == 0 || current_rows_stepped == current_number_rows_to_sort)
			{
				UpdateProgressBarValue(messager, current_rows_stepped);
			}

		}

		if (!rows_to_sort.empty())
		{
			RemoveDuplicatesFromPrimaryKeyMatches(current_rows_stepped, result, rows_to_sort, datetime_start_col_name, datetime_end_col_name, statement_is_prepared, the_prepared_stmt,
												  sql_strings, result_columns, previous_result_columns, current_rows_added, current_rows_added_since_execution, sql_add_xr_row, first_row_added, bound_parameter_strings,
												  bound_parameter_ints, bound_parameter_floats, bound_parameter_which_binding_to_use, minimum_desired_rows_per_transaction, xr_table_category,
												  consider_merging_timerange_adjacent_identical_rows, datetime_start_col_name_text, datetime_end_col_name_text);
			which_previous_row_index_to_test_against = 0;

			if (failed || CheckCancelled())
			{
				return result;
			}
		}

		ExecuteSQL(result);
		messager.UpdateProgressBarValue(1000);
		boost::format msg("Processed %1% of %2% temporary rows this stage: performing transaction");
		msg % current_rows_stepped % current_number_rows_to_sort;
		messager.SetPerformanceLabel(msg.str());
		EndTransaction();

		if (failed || CheckCancelled())
		{
			return result;
		}

	}


	return result;

}

bool OutputModel::OutputGenerator::ProcessCurrentDataRowOverlapWithPreviousSavedRow(SavedRowData & first_incoming_row, SavedRowData & current_row_of_data,
		std::deque<SavedRowData> & intermediate_rows_of_data, XR_TABLE_CATEGORY const xr_table_category)
{

	if (current_row_of_data.datetime_start >= first_incoming_row.datetime_end)
	{
		boost::format
		msg("Time range error during merging of data tables: the starting datetime of the current row (%1%) is greater than or equal to the ending datetime of the previous row (%2%).");
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
			intermediate_rows_of_data.push_back(std::move(current_row_of_data));
			intermediate_rows_of_data.push_back(std::move(first_incoming_row));
			return true; // current_row_complete
		}
		else
		{
			// Rows overlap
			SavedRowData new_data_row = current_row_of_data;
			new_data_row.datetime_end = first_incoming_row.datetime_start;
			intermediate_rows_of_data.push_back(std::move(new_data_row));
			current_row_of_data.datetime_start = first_incoming_row.datetime_start;
		}
	}

	if (current_row_of_data.datetime_start > first_incoming_row.datetime_start)
	{
		if (first_incoming_row.datetime_end <= current_row_of_data.datetime_start)
		{
			// Rows do not overlap
			intermediate_rows_of_data.push_back(std::move(first_incoming_row));
			return false; // current_row is not complete
		}
		else
		{
			// Rows overlap
			SavedRowData new_data_row = first_incoming_row;
			new_data_row.datetime_end = current_row_of_data.datetime_start;
			intermediate_rows_of_data.push_back(std::move(new_data_row));
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
		MergeRows(saved_temp_merged_row, current_row_of_data, first_incoming_row, xr_table_category);

		if (failed || CheckCancelled())
		{
			return false;
		}

		saved_temp_merged_row.datetime_start = current_row_of_data.datetime_start;
		saved_temp_merged_row.datetime_end = current_row_of_data.datetime_end;
		intermediate_rows_of_data.push_back(std::move(saved_temp_merged_row));

		// then:
		SavedRowData new_data_row = std::move(first_incoming_row);
		new_data_row.datetime_start = current_row_of_data.datetime_end;
		intermediate_rows_of_data.push_back(std::move(new_data_row));

		return true; // current_row_complete
	}
	else if (current_row_of_data.datetime_end == first_incoming_row.datetime_end)
	{
		// merge from:
		// current_row_of_data.datetime_start to current_row_of_data.datetime_end
		MergeRows(saved_temp_merged_row, current_row_of_data, first_incoming_row, xr_table_category);

		if (failed || CheckCancelled())
		{
			return false;
		}

		saved_temp_merged_row.datetime_start = current_row_of_data.datetime_start;
		saved_temp_merged_row.datetime_end = current_row_of_data.datetime_end;
		intermediate_rows_of_data.push_back(saved_temp_merged_row);

		return true; // current_row_complete
	}
	else
	{
		// current_row_of_data.datetime_end > first_incoming_row.datetime_end

		// merge from:
		// first_incoming_row.datetime_start to first_incoming_row.datetime_end
		MergeRows(saved_temp_merged_row, current_row_of_data, first_incoming_row, xr_table_category);

		if (failed || CheckCancelled())
		{
			return false;
		}

		saved_temp_merged_row.datetime_start = first_incoming_row.datetime_start;
		saved_temp_merged_row.datetime_end = first_incoming_row.datetime_end;
		intermediate_rows_of_data.push_back(std::move(saved_temp_merged_row));

		current_row_of_data.datetime_start = first_incoming_row.datetime_end;

		return false; // current_row is not complete
	}
}

void OutputModel::OutputGenerator::MergeRows(SavedRowData & merged_data_row, SavedRowData const & current_row_of_data, SavedRowData const & previous_row_of_data,
		XR_TABLE_CATEGORY const xr_table_category)
{

	merged_data_row.Clear();

	int int_index_current = 0;
	int float_index_current = 0;
	int string_index_current = 0;
	int int_index_previous = 0;
	int float_index_previous = 0;
	int string_index_previous = 0;

	merged_data_row.number_of_columns__in_a_single_inner_table__for_the_columns_only_having_the_dmu_category_with_multiplicity_greater_than_one__but_this_info_is_present_for_all_primary_key_columns
		= current_row_of_data.number_of_columns__in_a_single_inner_table__for_the_columns_only_having_the_dmu_category_with_multiplicity_greater_than_one__but_this_info_is_present_for_all_primary_key_columns;
	merged_data_row.number_of_columns_in_inner_table = current_row_of_data.number_of_columns_in_inner_table;
	merged_data_row.number_of_multiplicities = current_row_of_data.number_of_multiplicities;
	merged_data_row.is_index_a_primary_key = current_row_of_data.is_index_a_primary_key;
	merged_data_row.is_index_a_primary_key_with_outer_multiplicity_greater_than_1 = current_row_of_data.is_index_a_primary_key_with_outer_multiplicity_greater_than_1;
	merged_data_row.is_index_a_primary_key_with_outer_multiplicity_equal_to_1 = current_row_of_data.is_index_a_primary_key_with_outer_multiplicity_equal_to_1;
	merged_data_row.is_index_in_final_inner_table = current_row_of_data.is_index_in_final_inner_table;
	merged_data_row.is_index_in_all_but_final_inner_table = current_row_of_data.is_index_in_all_but_final_inner_table;
	merged_data_row.is_index_a_primary_key_in_the_final_inner_table = current_row_of_data.is_index_a_primary_key_in_the_final_inner_table;
	merged_data_row.is_index_a_primary_key_in_not_the_final_inner_table = current_row_of_data.is_index_a_primary_key_in_not_the_final_inner_table;
	merged_data_row.is_index_a_secondary_key = current_row_of_data.is_index_a_secondary_key;

	std::set<std::vector<std::string>> saved_strings_vector;
	std::set<std::vector<std::int64_t>> saved_ints_vector;
	std::set<std::vector<long double>> saved_floats_vector;

	std::vector<std::string> inner_multiplicity_string_vector;
	std::vector<std::int64_t> inner_multiplicity_int_vector;
	std::vector<long double> inner_multiplicity_float_vector;

	int inner_multiplicity_current_index = 0;
	int inner_multiplicity_previous_index = 0;

	int current_index = 0;
	int previous_index = 0;




	// ********************************************************************************** //
	// Compare the following method to that used in CreateNewXRRow().
	//
	// In this method, we save the indices of the original location of the data
	// in the incoming SavedRowData objects (current_row__map_from__inner_multiplicity_string_vector__to__inner_table_number),
	// and simply use those indices when we need to retrieve the data
	// for a certain inner table (current_inner_table_index_offset).
	//
	// In CreateNewXRRow(), we copy the inner table data into a new, local cache
	// (actually, a vector of them),
	// and then when we need the data we reference the local cache vector by index.
	// ********************************************************************************** //




	// Populate a set of primary key groups - one group from each inner table, corresponding only to the DMU with multiplicity greater than 1.
	// We do this because NULL's in each group could offset the group as a whole, making it more difficult to test for a match.
	// So instead of testing for a match on these groups, just save the non-NULL ones into a set, and add the set later in sorted order.

	std::map<std::vector<std::int64_t>, int> current_row__map_from__inner_multiplicity_int_vector__to__inner_table_number;
	std::map<std::vector<std::int64_t>, int> previous_row__map_from__inner_multiplicity_int_vector__to__inner_table_number;
	std::map<std::vector<long double>, int> current_row__map_from__inner_multiplicity_float_vector__to__inner_table_number;
	std::map<std::vector<long double>, int> previous_row__map_from__inner_multiplicity_float_vector__to__inner_table_number;
	std::map<std::vector<std::string>, int> current_row__map_from__inner_multiplicity_string_vector__to__inner_table_number;
	std::map<std::vector<std::string>, int> previous_row__map_from__inner_multiplicity_string_vector__to__inner_table_number;

	//std::for_each(current_row_of_data.current_parameter_which_binding_to_use.cbegin(), current_row_of_data.current_parameter_which_binding_to_use.cend(), [&current_row__map_from__inner_multiplicity_string_vector__to__inner_table_number, &current_row__map_from__inner_multiplicity_int_vector__to__inner_table_number, &current_row__map_from__inner_multiplicity_float_vector__to__inner_table_number, &inner_multiplicity_current_index, &saved_strings_vector, &saved_ints_vector, &saved_floats_vector, &inner_multiplicity_string_vector, &inner_multiplicity_int_vector, &inner_multiplicity_float_vector, &current_row_of_data, &int_index_current, &float_index_current, &string_index_current, &current_index, &merged_data_row](SQLExecutor::WHICH_BINDING const & current_binding)
	for (std::vector<SQLExecutor::WHICH_BINDING>::const_iterator it = current_row_of_data.current_parameter_which_binding_to_use.cbegin();
		 it != current_row_of_data.current_parameter_which_binding_to_use.cend(); ++it)
	{

		SQLExecutor::WHICH_BINDING const & current_binding = *it;

		if (current_binding == SQLExecutor::NULL_BINDING)
		{

			// no-op - assume that if any primary key column within a single inner table is null for DMU category,
			// then so will the others
			// ... therefore, this is a NULL primary key group and we don't need to save it

		}
		else
		{
			switch (current_binding)
			{
				case SQLExecutor::INT64:
					{

						if (current_row_of_data.is_index_a_primary_key_with_outer_multiplicity_greater_than_1[current_index])
						{
							inner_multiplicity_int_vector.push_back(current_row_of_data.current_parameter_ints[int_index_current]);
						}

					}
					break;

				case SQLExecutor::FLOAT:
					{

						if (current_row_of_data.is_index_a_primary_key_with_outer_multiplicity_greater_than_1[current_index])
						{
							inner_multiplicity_float_vector.push_back(current_row_of_data.current_parameter_floats[float_index_current]);
						}

					}
					break;

				case SQLExecutor::STRING:
					{

						if (current_row_of_data.is_index_a_primary_key_with_outer_multiplicity_greater_than_1[current_index])
						{
							inner_multiplicity_string_vector.push_back(current_row_of_data.current_parameter_strings[string_index_current]);
						}

					}
					break;

				default:
					break;
			}
		}

		switch (current_binding)
		{
			case SQLExecutor::INT64:
				{
					++int_index_current;
				}
				break;

			case SQLExecutor::FLOAT:
				{
					++float_index_current;
				}
				break;

			case SQLExecutor::STRING:
				{
					++string_index_current;
				}
				break;

			default:
				break;
		}

		if (current_row_of_data.is_index_a_primary_key_with_outer_multiplicity_greater_than_1[current_index])
		{
			++inner_multiplicity_current_index;
		}

		++current_index;

		if (inner_multiplicity_current_index ==
			current_row_of_data.number_of_columns__in_a_single_inner_table__for_the_columns_only_having_the_dmu_category_with_multiplicity_greater_than_one__but_this_info_is_present_for_all_primary_key_columns)
		{
			inner_multiplicity_current_index = 0;

			if (!inner_multiplicity_string_vector.empty())
			{
				saved_strings_vector.insert(inner_multiplicity_string_vector);
				current_row__map_from__inner_multiplicity_string_vector__to__inner_table_number[inner_multiplicity_string_vector] = current_row_of_data.inner_table_number[current_index - 1];
			}

			if (!inner_multiplicity_int_vector.empty())
			{
				saved_ints_vector.insert(inner_multiplicity_int_vector);
				current_row__map_from__inner_multiplicity_int_vector__to__inner_table_number[inner_multiplicity_int_vector] = current_row_of_data.inner_table_number[current_index - 1];
			}

			if (!inner_multiplicity_float_vector.empty())
			{
				saved_floats_vector.insert(inner_multiplicity_float_vector);
				current_row__map_from__inner_multiplicity_float_vector__to__inner_table_number[inner_multiplicity_float_vector] = current_row_of_data.inner_table_number[current_index - 1];
			}

			inner_multiplicity_string_vector.clear();
			inner_multiplicity_int_vector.clear();
			inner_multiplicity_float_vector.clear();
		}

		//});
	}

	int_index_current = 0;
	float_index_current = 0;
	string_index_current = 0;
	int_index_previous = 0;
	float_index_previous = 0;
	string_index_previous = 0;

	inner_multiplicity_string_vector.clear();
	inner_multiplicity_int_vector.clear();
	inner_multiplicity_float_vector.clear();

	inner_multiplicity_current_index = 0;
	inner_multiplicity_previous_index = 0;

	current_index = 0;
	previous_index = 0;

	//std::for_each(previous_row_of_data.current_parameter_which_binding_to_use.cbegin(), previous_row_of_data.current_parameter_which_binding_to_use.cend(), [&previous_row__map_from__inner_multiplicity_string_vector__to__inner_table_number, &previous_row__map_from__inner_multiplicity_int_vector__to__inner_table_number, &previous_row__map_from__inner_multiplicity_float_vector__to__inner_table_number, &inner_multiplicity_previous_index, &saved_strings_vector, &saved_ints_vector, &saved_floats_vector, &inner_multiplicity_string_vector, &inner_multiplicity_int_vector, &inner_multiplicity_float_vector, &previous_row_of_data, &int_index_previous, &float_index_previous, &string_index_previous, &previous_index, &merged_data_row](SQLExecutor::WHICH_BINDING const & previous_binding)
	for (std::vector<SQLExecutor::WHICH_BINDING>::const_iterator it = previous_row_of_data.current_parameter_which_binding_to_use.cbegin();
		 it != previous_row_of_data.current_parameter_which_binding_to_use.cend(); ++it)
	{

		SQLExecutor::WHICH_BINDING const & previous_binding = *it;

		if (previous_binding == SQLExecutor::NULL_BINDING)
		{

			// no-op - assume that if any primary key column within a single inner table is null for DMU category,
			// then so will the others
			// ... therefore, this is a NULL primary key group and we don't need to save it

		}
		else
		{
			switch (previous_binding)
			{
				case SQLExecutor::INT64:
					{

						if (previous_row_of_data.is_index_a_primary_key_with_outer_multiplicity_greater_than_1[previous_index])
						{
							inner_multiplicity_int_vector.push_back(previous_row_of_data.current_parameter_ints[int_index_previous]);
						}

					}
					break;

				case SQLExecutor::FLOAT:
					{

						if (previous_row_of_data.is_index_a_primary_key_with_outer_multiplicity_greater_than_1[previous_index])
						{
							inner_multiplicity_float_vector.push_back(previous_row_of_data.current_parameter_floats[float_index_previous]);
						}

					}
					break;

				case SQLExecutor::STRING:
					{

						if (previous_row_of_data.is_index_a_primary_key_with_outer_multiplicity_greater_than_1[previous_index])
						{
							inner_multiplicity_string_vector.push_back(previous_row_of_data.current_parameter_strings[string_index_previous]);
						}

					}
					break;

				default:
					break;
			}
		}

		switch (previous_binding)
		{
			case SQLExecutor::INT64:
				{
					++int_index_previous;
				}
				break;

			case SQLExecutor::FLOAT:
				{
					++float_index_previous;
				}
				break;

			case SQLExecutor::STRING:
				{
					++string_index_previous;
				}
				break;

			default:
				break;
		}

		if (previous_row_of_data.is_index_a_primary_key_with_outer_multiplicity_greater_than_1[previous_index])
		{
			++inner_multiplicity_previous_index;
		}

		++previous_index;

		if (inner_multiplicity_previous_index ==
			previous_row_of_data.number_of_columns__in_a_single_inner_table__for_the_columns_only_having_the_dmu_category_with_multiplicity_greater_than_one__but_this_info_is_present_for_all_primary_key_columns)
		{
			inner_multiplicity_previous_index = 0;

			if (!inner_multiplicity_string_vector.empty())
			{
				saved_strings_vector.insert(inner_multiplicity_string_vector);
				previous_row__map_from__inner_multiplicity_string_vector__to__inner_table_number[inner_multiplicity_string_vector] = previous_row_of_data.inner_table_number[previous_index - 1];
			}

			if (!inner_multiplicity_int_vector.empty())
			{
				saved_ints_vector.insert(inner_multiplicity_int_vector);
				previous_row__map_from__inner_multiplicity_int_vector__to__inner_table_number[inner_multiplicity_int_vector] = previous_row_of_data.inner_table_number[previous_index - 1];
			}

			if (!inner_multiplicity_float_vector.empty())
			{
				saved_floats_vector.insert(inner_multiplicity_float_vector);
				previous_row__map_from__inner_multiplicity_float_vector__to__inner_table_number[inner_multiplicity_float_vector] = previous_row_of_data.inner_table_number[previous_index - 1];
			}

			inner_multiplicity_string_vector.clear();
			inner_multiplicity_int_vector.clear();
			inner_multiplicity_float_vector.clear();
		}

		//});
	}

	int_index_current = 0;
	float_index_current = 0;
	string_index_current = 0;
	int_index_previous = 0;
	float_index_previous = 0;
	string_index_previous = 0;


	bool use_strings = false;

	if (!saved_strings_vector.empty())
	{
		use_strings = true;
	}

	bool use_ints = false;

	if (!saved_ints_vector.empty())
	{
		use_ints = true;
	}

	bool use_floats = false;

	if (!saved_floats_vector.empty())
	{
		use_floats = true;
	}

	int count_of_types = 0;

	if (use_strings)
	{
		++count_of_types;
	}

	if (use_ints)
	{
		++count_of_types;
	}

	if (use_floats)
	{
		++count_of_types;
	}

	if (count_of_types > 1)
	{
		boost::format msg("Primary keys within the same DMU category are of different, or of NULL, types.");
		SetFailureMessage(msg.str());
		failed = true;
		return;
	}

	bool one_multiplicity = false;

	if (count_of_types == 0)
	{
		one_multiplicity = true;
	}

	inner_multiplicity_current_index = 0;
	current_index = 0;

	saved_strings_deque.clear();
	saved_ints_deque.clear();
	saved_floats_deque.clear();

	saved_strings_deque.insert(saved_strings_deque.begin(), saved_strings_vector.begin(), saved_strings_vector.end());
	saved_ints_deque.insert(saved_ints_deque.begin(), saved_ints_vector.begin(), saved_ints_vector.end());
	saved_floats_deque.insert(saved_floats_deque.begin(), saved_floats_vector.begin(), saved_floats_vector.end());

	//int most_recent_current_offset = -1;
	//int most_recent_previous_offset = -1;

	//int current__current_inner_table = -1;
	//int previous__current_inner_table = -1;

	int current_row_inner_table_data_to_use = -1;
	int previous_row_inner_table_data_to_use = -1;

	//std::for_each(current_row_of_data.current_parameter_which_binding_to_use.cbegin(), current_row_of_data.current_parameter_which_binding_to_use.cend(), [this, &one_multiplicity, &current_row_inner_table_data_to_use, &previous_row_inner_table_data_to_use, &current__current_inner_table, &previous__current_inner_table, &most_recent_current_offset, &most_recent_previous_offset, &xr_table_category, &current_row__map_from__inner_multiplicity_string_vector__to__inner_table_number, &current_row__map_from__inner_multiplicity_int_vector__to__inner_table_number, &current_row__map_from__inner_multiplicity_float_vector__to__inner_table_number, &previous_row__map_from__inner_multiplicity_string_vector__to__inner_table_number, &previous_row__map_from__inner_multiplicity_int_vector__to__inner_table_number, &previous_row__map_from__inner_multiplicity_float_vector__to__inner_table_number, &inner_multiplicity_int_vector, &inner_multiplicity_float_vector,  &inner_multiplicity_string_vector, &use_strings, &use_ints, &use_floats, &inner_multiplicity_current_index, &saved_strings_deque, &saved_ints_deque, &saved_floats_deque, &current_row_of_data, &int_index_current, &float_index_current, &string_index_current, &int_index_previous, &float_index_previous, &string_index_previous, &current_index, &previous_row_of_data, &merged_data_row](SQLExecutor::WHICH_BINDING const & current_binding)
	for (std::vector<SQLExecutor::WHICH_BINDING>::const_iterator it = current_row_of_data.current_parameter_which_binding_to_use.cbegin();
		 it != current_row_of_data.current_parameter_which_binding_to_use.cend(); ++it)
	{

		//SQLExecutor::WHICH_BINDING const & current_binding = *it;

		if (failed || CheckCancelled())
		{
			//return;
			break;;
		}

		if (current_index % current_row_of_data.number_of_columns_in_inner_table == 0)
		{
			current_row_inner_table_data_to_use = -1;

			if (use_ints)
			{
				if (!saved_ints_deque.empty())
				{
					inner_multiplicity_int_vector = saved_ints_deque.front();

					if (current_row__map_from__inner_multiplicity_int_vector__to__inner_table_number.find(inner_multiplicity_int_vector) !=
						current_row__map_from__inner_multiplicity_int_vector__to__inner_table_number.cend())
					{
						current_row_inner_table_data_to_use = current_row__map_from__inner_multiplicity_int_vector__to__inner_table_number[inner_multiplicity_int_vector];
					}
				}
			}

			if (use_floats)
			{
				if (!saved_floats_deque.empty())
				{
					inner_multiplicity_float_vector = saved_floats_deque.front();

					if (current_row__map_from__inner_multiplicity_float_vector__to__inner_table_number.find(inner_multiplicity_float_vector) !=
						current_row__map_from__inner_multiplicity_float_vector__to__inner_table_number.cend())
					{
						current_row_inner_table_data_to_use = current_row__map_from__inner_multiplicity_float_vector__to__inner_table_number[inner_multiplicity_float_vector];
					}
				}
			}

			if (use_strings)
			{
				if (!saved_strings_deque.empty())
				{
					inner_multiplicity_string_vector = saved_strings_deque.front();

					if (current_row__map_from__inner_multiplicity_string_vector__to__inner_table_number.find(inner_multiplicity_string_vector) !=
						current_row__map_from__inner_multiplicity_string_vector__to__inner_table_number.cend())
					{
						current_row_inner_table_data_to_use = current_row__map_from__inner_multiplicity_string_vector__to__inner_table_number[inner_multiplicity_string_vector];
					}
				}
			}
		}

		if (current_index % previous_row_of_data.number_of_columns_in_inner_table == 0)
		{
			previous_row_inner_table_data_to_use = -1;

			if (use_ints)
			{
				if (!saved_ints_deque.empty())
				{
					inner_multiplicity_int_vector = saved_ints_deque.front();

					if (previous_row__map_from__inner_multiplicity_int_vector__to__inner_table_number.find(inner_multiplicity_int_vector) !=
						previous_row__map_from__inner_multiplicity_int_vector__to__inner_table_number.cend())
					{
						previous_row_inner_table_data_to_use = previous_row__map_from__inner_multiplicity_int_vector__to__inner_table_number[inner_multiplicity_int_vector];
					}
				}
			}

			if (use_floats)
			{
				if (!saved_floats_deque.empty())
				{
					inner_multiplicity_float_vector = saved_floats_deque.front();

					if (previous_row__map_from__inner_multiplicity_float_vector__to__inner_table_number.find(inner_multiplicity_float_vector) !=
						previous_row__map_from__inner_multiplicity_float_vector__to__inner_table_number.cend())
					{
						previous_row_inner_table_data_to_use = previous_row__map_from__inner_multiplicity_float_vector__to__inner_table_number[inner_multiplicity_float_vector];
					}
				}
			}

			if (use_strings)
			{
				if (!saved_strings_deque.empty())
				{
					inner_multiplicity_string_vector = saved_strings_deque.front();

					if (previous_row__map_from__inner_multiplicity_string_vector__to__inner_table_number.find(inner_multiplicity_string_vector) !=
						previous_row__map_from__inner_multiplicity_string_vector__to__inner_table_number.cend())
					{
						previous_row_inner_table_data_to_use = previous_row__map_from__inner_multiplicity_string_vector__to__inner_table_number[inner_multiplicity_string_vector];
					}
				}
			}
		}

		bool use_nulls_current = false;

		if (!one_multiplicity && current_row_inner_table_data_to_use == -1)
		{
			use_nulls_current = true;
		}

		bool use_nulls_previous = false;

		if (!one_multiplicity && previous_row_inner_table_data_to_use == -1)
		{
			use_nulls_previous = true;
		}

		int current_inner_table_index_offset = 0;

		if (!one_multiplicity && !use_nulls_current)
		{
			int current_inner_table_being_walked_through = current_row_of_data.inner_table_number[current_index];
			current_inner_table_index_offset = (current_row_inner_table_data_to_use - current_inner_table_being_walked_through) * current_row_of_data.number_of_columns_in_inner_table;
		}

		int previous_inner_table_index_offset = 0;

		if (!one_multiplicity && !use_nulls_previous)
		{
			int previous_inner_table_being_walked_through = previous_row_of_data.inner_table_number[current_index];
			previous_inner_table_index_offset = (previous_row_inner_table_data_to_use - previous_inner_table_being_walked_through) * previous_row_of_data.number_of_columns_in_inner_table;
		}

		if (xr_table_category != XR_TABLE_CATEGORY::PRIMARY_VARIABLE_GROUP)
		{
			current_inner_table_index_offset = 0;
			previous_inner_table_index_offset = 0;
		}

		if (xr_table_category != XR_TABLE_CATEGORY::PRIMARY_VARIABLE_GROUP)
		{
			use_nulls_previous = false;
			use_nulls_current = false;
		}


		SQLExecutor::WHICH_BINDING current_row_binding = current_row_of_data.current_parameter_which_binding_to_use[current_index];
		SQLExecutor::WHICH_BINDING previous_row_binding = previous_row_of_data.current_parameter_which_binding_to_use[current_index];


		// It will step off the end, because the time range rows have not been appended yet.
		// These, however, will be populated later, so they don't need to be correct here.
		if (!use_nulls_current && current_index + current_inner_table_index_offset < (int)current_row_of_data.current_parameter_which_binding_to_use.size())
		{
			current_row_binding = current_row_of_data.current_parameter_which_binding_to_use[current_index + current_inner_table_index_offset];
		}

		// It will step off the end, because the time range rows have not been appended yet.
		// These, however, will be populated later, so they don't need to be correct here.
		if (!use_nulls_previous && current_index + previous_inner_table_index_offset < (int)previous_row_of_data.current_parameter_which_binding_to_use.size())
		{
			previous_row_binding = previous_row_of_data.current_parameter_which_binding_to_use[current_index + previous_inner_table_index_offset];
		}

		merged_data_row.inner_table_number.push_back(current_row_of_data.inner_table_number[current_index]);

		// Special case: handle the primary key group with multiplicity greater than one
		if (xr_table_category == XR_TABLE_CATEGORY::PRIMARY_VARIABLE_GROUP
			&&
			current_row_of_data.is_index_a_primary_key_with_outer_multiplicity_greater_than_1[current_index])
		{

			if (use_ints)
			{

				if (!saved_ints_deque.empty())
				{

					std::vector<std::int64_t> & the_ints = saved_ints_deque.front();
					merged_data_row.current_parameter_which_binding_to_use.push_back(SQLExecutor::INT64);
					merged_data_row.current_parameter_ints.push_back(the_ints[inner_multiplicity_current_index]);

					merged_data_row.indices_of_all_columns.push_back(std::make_pair(SQLExecutor::INT64, std::make_pair((int)merged_data_row.current_parameter_ints.size() - 1, current_index)));
					merged_data_row.indices_of_primary_key_columns.push_back(std::make_pair(SQLExecutor::INT64, std::make_pair((int)merged_data_row.current_parameter_ints.size() - 1, current_index)));
					merged_data_row.indices_of_primary_key_columns_with_multiplicity_greater_than_1.push_back(std::make_pair(SQLExecutor::INT64,
							std::make_pair((int)merged_data_row.current_parameter_ints.size() - 1, current_index)));

					if (merged_data_row.is_index_in_final_inner_table[current_index])
					{
						merged_data_row.indices_of_all_columns_in_final_inner_table.push_back(std::make_pair(SQLExecutor::INT64, std::make_pair((int)merged_data_row.current_parameter_ints.size() - 1,
								current_index)));
					}

					if (merged_data_row.is_index_in_all_but_final_inner_table[current_index])
					{
						merged_data_row.indices_of_all_columns_in_all_but_final_inner_table.push_back(std::make_pair(SQLExecutor::INT64,
								std::make_pair((int)merged_data_row.current_parameter_ints.size() - 1, current_index)));
					}

					if (merged_data_row.is_index_a_primary_key_in_the_final_inner_table[current_index])
					{
						merged_data_row.indices_of_all_primary_key_columns_in_final_inner_table.push_back(std::make_pair(SQLExecutor::INT64,
								std::make_pair((int)merged_data_row.current_parameter_ints.size() - 1, current_index)));
					}

					if (merged_data_row.is_index_a_primary_key_in_not_the_final_inner_table[current_index])
					{
						merged_data_row.indices_of_all_primary_key_columns_in_all_but_final_inner_table.push_back(std::make_pair(SQLExecutor::INT64,
								std::make_pair((int)merged_data_row.current_parameter_ints.size() - 1, current_index)));
					}

				}
				else
				{

					// No more.  Add a NULL group
					merged_data_row.current_parameter_which_binding_to_use.push_back(SQLExecutor::NULL_BINDING);

					merged_data_row.indices_of_all_columns.push_back(std::make_pair(SQLExecutor::NULL_BINDING, std::make_pair(0, current_index)));
					merged_data_row.indices_of_primary_key_columns.push_back(std::make_pair(SQLExecutor::NULL_BINDING, std::make_pair(0, current_index)));
					merged_data_row.indices_of_primary_key_columns_with_multiplicity_greater_than_1.push_back(std::make_pair(SQLExecutor::NULL_BINDING, std::make_pair(0, current_index)));

					if (merged_data_row.is_index_in_final_inner_table[current_index])
					{
						merged_data_row.indices_of_all_columns_in_final_inner_table.push_back(std::make_pair(SQLExecutor::NULL_BINDING, std::make_pair(0, current_index)));
					}

					if (merged_data_row.is_index_in_all_but_final_inner_table[current_index])
					{
						merged_data_row.indices_of_all_columns_in_all_but_final_inner_table.push_back(std::make_pair(SQLExecutor::NULL_BINDING, std::make_pair(0, current_index)));
					}

					if (merged_data_row.is_index_a_primary_key_in_the_final_inner_table[current_index])
					{
						merged_data_row.indices_of_all_primary_key_columns_in_final_inner_table.push_back(std::make_pair(SQLExecutor::NULL_BINDING, std::make_pair(0, current_index)));
					}

					if (merged_data_row.is_index_a_primary_key_in_not_the_final_inner_table[current_index])
					{
						merged_data_row.indices_of_all_primary_key_columns_in_all_but_final_inner_table.push_back(std::make_pair(SQLExecutor::NULL_BINDING, std::make_pair(0, current_index)));
					}

				}

			}
			else if (use_floats)
			{

				if (!saved_floats_deque.empty())
				{

					std::vector<long double> & the_floats = saved_floats_deque.front();
					merged_data_row.current_parameter_which_binding_to_use.push_back(SQLExecutor::FLOAT);
					merged_data_row.current_parameter_floats.push_back(the_floats[inner_multiplicity_current_index]);

					merged_data_row.indices_of_all_columns.push_back(std::make_pair(SQLExecutor::FLOAT, std::make_pair((int)merged_data_row.current_parameter_floats.size() - 1, current_index)));
					merged_data_row.indices_of_primary_key_columns.push_back(std::make_pair(SQLExecutor::FLOAT, std::make_pair((int)merged_data_row.current_parameter_floats.size() - 1,
							current_index)));
					merged_data_row.indices_of_primary_key_columns_with_multiplicity_greater_than_1.push_back(std::make_pair(SQLExecutor::FLOAT,
							std::make_pair((int)merged_data_row.current_parameter_floats.size() - 1, current_index)));

					if (merged_data_row.is_index_in_final_inner_table[current_index])
					{
						merged_data_row.indices_of_all_columns_in_final_inner_table.push_back(std::make_pair(SQLExecutor::FLOAT, std::make_pair((int)merged_data_row.current_parameter_floats.size() - 1,
								current_index)));
					}

					if (merged_data_row.is_index_in_all_but_final_inner_table[current_index])
					{
						merged_data_row.indices_of_all_columns_in_all_but_final_inner_table.push_back(std::make_pair(SQLExecutor::FLOAT,
								std::make_pair((int)merged_data_row.current_parameter_floats.size() - 1, current_index)));
					}

					if (merged_data_row.is_index_a_primary_key_in_the_final_inner_table[current_index])
					{
						merged_data_row.indices_of_all_primary_key_columns_in_final_inner_table.push_back(std::make_pair(SQLExecutor::FLOAT,
								std::make_pair((int)merged_data_row.current_parameter_floats.size() - 1, current_index)));
					}

					if (merged_data_row.is_index_a_primary_key_in_not_the_final_inner_table[current_index])
					{
						merged_data_row.indices_of_all_primary_key_columns_in_all_but_final_inner_table.push_back(std::make_pair(SQLExecutor::FLOAT,
								std::make_pair((int)merged_data_row.current_parameter_floats.size() - 1, current_index)));
					}

				}
				else
				{

					// No more.  Add a NULL group
					merged_data_row.current_parameter_which_binding_to_use.push_back(SQLExecutor::NULL_BINDING);

					merged_data_row.indices_of_all_columns.push_back(std::make_pair(SQLExecutor::NULL_BINDING, std::make_pair(0, current_index)));
					merged_data_row.indices_of_primary_key_columns.push_back(std::make_pair(SQLExecutor::NULL_BINDING, std::make_pair(0, current_index)));
					merged_data_row.indices_of_primary_key_columns_with_multiplicity_greater_than_1.push_back(std::make_pair(SQLExecutor::NULL_BINDING, std::make_pair(0, current_index)));

					if (merged_data_row.is_index_in_final_inner_table[current_index])
					{
						merged_data_row.indices_of_all_columns_in_final_inner_table.push_back(std::make_pair(SQLExecutor::NULL_BINDING, std::make_pair(0, current_index)));
					}

					if (merged_data_row.is_index_in_all_but_final_inner_table[current_index])
					{
						merged_data_row.indices_of_all_columns_in_all_but_final_inner_table.push_back(std::make_pair(SQLExecutor::NULL_BINDING, std::make_pair(0, current_index)));
					}

					if (merged_data_row.is_index_a_primary_key_in_the_final_inner_table[current_index])
					{
						merged_data_row.indices_of_all_primary_key_columns_in_final_inner_table.push_back(std::make_pair(SQLExecutor::NULL_BINDING, std::make_pair(0, current_index)));
					}

					if (merged_data_row.is_index_a_primary_key_in_not_the_final_inner_table[current_index])
					{
						merged_data_row.indices_of_all_primary_key_columns_in_all_but_final_inner_table.push_back(std::make_pair(SQLExecutor::NULL_BINDING, std::make_pair(0, current_index)));
					}

				}

			}
			else
			{

				if (!saved_strings_deque.empty())
				{

					std::vector<std::string> & the_strings = saved_strings_deque.front();
					merged_data_row.current_parameter_which_binding_to_use.push_back(SQLExecutor::STRING);
					merged_data_row.current_parameter_strings.push_back(the_strings[inner_multiplicity_current_index]);

					merged_data_row.indices_of_all_columns.push_back(std::make_pair(SQLExecutor::STRING, std::make_pair((int)merged_data_row.current_parameter_strings.size() - 1, current_index)));
					merged_data_row.indices_of_primary_key_columns.push_back(std::make_pair(SQLExecutor::STRING, std::make_pair((int)merged_data_row.current_parameter_strings.size() - 1,
							current_index)));
					merged_data_row.indices_of_primary_key_columns_with_multiplicity_greater_than_1.push_back(std::make_pair(SQLExecutor::STRING,
							std::make_pair((int)merged_data_row.current_parameter_strings.size() - 1, current_index)));

					if (merged_data_row.is_index_in_final_inner_table[current_index])
					{
						merged_data_row.indices_of_all_columns_in_final_inner_table.push_back(std::make_pair(SQLExecutor::STRING, std::make_pair((int)merged_data_row.current_parameter_strings.size() - 1,
								current_index)));
					}

					if (merged_data_row.is_index_in_all_but_final_inner_table[current_index])
					{
						merged_data_row.indices_of_all_columns_in_all_but_final_inner_table.push_back(std::make_pair(SQLExecutor::STRING,
								std::make_pair((int)merged_data_row.current_parameter_strings.size() - 1, current_index)));
					}

					if (merged_data_row.is_index_a_primary_key_in_the_final_inner_table[current_index])
					{
						merged_data_row.indices_of_all_primary_key_columns_in_final_inner_table.push_back(std::make_pair(SQLExecutor::STRING,
								std::make_pair((int)merged_data_row.current_parameter_strings.size() - 1, current_index)));
					}

					if (merged_data_row.is_index_a_primary_key_in_not_the_final_inner_table[current_index])
					{
						merged_data_row.indices_of_all_primary_key_columns_in_all_but_final_inner_table.push_back(std::make_pair(SQLExecutor::STRING,
								std::make_pair((int)merged_data_row.current_parameter_strings.size() - 1, current_index)));
					}

				}
				else
				{

					// No more.  Add a NULL group
					merged_data_row.current_parameter_which_binding_to_use.push_back(SQLExecutor::NULL_BINDING);

					merged_data_row.indices_of_all_columns.push_back(std::make_pair(SQLExecutor::NULL_BINDING, std::make_pair(0, current_index)));
					merged_data_row.indices_of_primary_key_columns.push_back(std::make_pair(SQLExecutor::NULL_BINDING, std::make_pair(0, current_index)));
					merged_data_row.indices_of_primary_key_columns_with_multiplicity_greater_than_1.push_back(std::make_pair(SQLExecutor::NULL_BINDING, std::make_pair(0, current_index)));

					if (merged_data_row.is_index_in_final_inner_table[current_index])
					{
						merged_data_row.indices_of_all_columns_in_final_inner_table.push_back(std::make_pair(SQLExecutor::NULL_BINDING, std::make_pair(0, current_index)));
					}

					if (merged_data_row.is_index_in_all_but_final_inner_table[current_index])
					{
						merged_data_row.indices_of_all_columns_in_all_but_final_inner_table.push_back(std::make_pair(SQLExecutor::NULL_BINDING, std::make_pair(0, current_index)));
					}

					if (merged_data_row.is_index_a_primary_key_in_the_final_inner_table[current_index])
					{
						merged_data_row.indices_of_all_primary_key_columns_in_final_inner_table.push_back(std::make_pair(SQLExecutor::NULL_BINDING, std::make_pair(0, current_index)));
					}

					if (merged_data_row.is_index_a_primary_key_in_not_the_final_inner_table[current_index])
					{
						merged_data_row.indices_of_all_primary_key_columns_in_all_but_final_inner_table.push_back(std::make_pair(SQLExecutor::NULL_BINDING, std::make_pair(0, current_index)));
					}

				}

			}

		}
		else
		{

			// ****************************************************************************************************************************** //
			// !use_nulls_current means the current inner table has data associated with the current stored multiplicity data being used.
			// !use_nulls_previous means the previous inner table has data associated with the current stored multiplicity data being used.
			// ****************************************************************************************************************************** //

			bool do_null = false;

			if (use_nulls_current && use_nulls_previous)
			{
				do_null = true;
			}

			if (!use_nulls_current && use_nulls_previous)
			{
				if (current_row_binding == SQLExecutor::NULL_BINDING)
				{
					do_null = true;
				}
			}

			if (use_nulls_current && !use_nulls_previous)
			{
				if (previous_row_binding == SQLExecutor::NULL_BINDING)
				{
					do_null = true;
				}
			}

			if (!use_nulls_current && !use_nulls_previous)
			{
				if (current_row_binding == SQLExecutor::NULL_BINDING
					&&
					previous_row_binding == SQLExecutor::NULL_BINDING)
				{
					do_null = true;
				}
			}

			bool use_current = false;
			bool use_previous = false;

			if (!do_null)
			{
				if (!use_nulls_current && !use_nulls_previous)
				{
					if (previous_row_binding == SQLExecutor::NULL_BINDING)
					{
						use_current = true;
					}
					else if (current_row_binding == SQLExecutor::NULL_BINDING)
					{
						use_previous = true;
					}
					else
					{
						use_current = true;
					}
				}

				if (use_nulls_current && !use_nulls_previous)
				{
					use_previous = true;
				}

				if (!use_nulls_current && use_nulls_previous)
				{
					use_current = true;
				}
			}

			if (use_current && use_previous)
			{
				use_previous = false;
			}

			if (!use_current && !use_previous)
			{
				do_null = true;
			}

			if (do_null)
			{

				merged_data_row.current_parameter_which_binding_to_use.push_back(SQLExecutor::NULL_BINDING);

				merged_data_row.indices_of_all_columns.push_back(std::make_pair(SQLExecutor::NULL_BINDING, std::make_pair(0, current_index)));

				if (merged_data_row.is_index_in_final_inner_table[current_index])
				{
					merged_data_row.indices_of_all_columns_in_final_inner_table.push_back(std::make_pair(SQLExecutor::NULL_BINDING, std::make_pair(0, current_index)));
				}

				if (merged_data_row.is_index_in_all_but_final_inner_table[current_index])
				{
					merged_data_row.indices_of_all_columns_in_all_but_final_inner_table.push_back(std::make_pair(SQLExecutor::NULL_BINDING, std::make_pair(0, current_index)));
				}

				if (merged_data_row.is_index_a_primary_key[current_index])
				{
					merged_data_row.indices_of_primary_key_columns.push_back(std::make_pair(OutputModel::OutputGenerator::SQLExecutor::NULL_BINDING, std::make_pair(0, current_index)));
				}

				if (merged_data_row.is_index_a_primary_key_with_outer_multiplicity_greater_than_1[current_index])
				{
					merged_data_row.indices_of_primary_key_columns_with_multiplicity_greater_than_1.push_back(std::make_pair(OutputModel::OutputGenerator::SQLExecutor::NULL_BINDING, std::make_pair(0,
							current_index)));
				}

				if (merged_data_row.is_index_a_primary_key_with_outer_multiplicity_equal_to_1[current_index])
				{
					merged_data_row.indices_of_primary_key_columns_with_multiplicity_equal_to_1.push_back(std::make_pair(OutputModel::OutputGenerator::SQLExecutor::NULL_BINDING, std::make_pair(0,
							current_index)));
				}

				if (merged_data_row.is_index_a_primary_key_in_the_final_inner_table[current_index])
				{
					merged_data_row.indices_of_all_primary_key_columns_in_final_inner_table.push_back(std::make_pair(SQLExecutor::NULL_BINDING, std::make_pair(0, current_index)));
				}

				if (merged_data_row.is_index_a_primary_key_in_not_the_final_inner_table[current_index])
				{
					merged_data_row.indices_of_all_primary_key_columns_in_all_but_final_inner_table.push_back(std::make_pair(SQLExecutor::NULL_BINDING, std::make_pair(0, current_index)));
				}

				if (merged_data_row.is_index_a_secondary_key[current_index])
				{
					merged_data_row.indices_of_secondary_key_columns.push_back(std::make_pair(OutputModel::OutputGenerator::SQLExecutor::NULL_BINDING, std::make_pair(0, current_index)));
				}

				if (merged_data_row.is_index_a_secondary_key[current_index])
				{
					merged_data_row.indices_of_secondary_key_columns.push_back(std::make_pair(OutputModel::OutputGenerator::SQLExecutor::NULL_BINDING, std::make_pair(0, current_index)));
				}

			}
			else
			{
				if (use_current)
				{
					switch (current_row_binding)
					{
						case SQLExecutor::INT64:
							{

								merged_data_row.current_parameter_which_binding_to_use.push_back(SQLExecutor::INT64);

								if (current_index + current_inner_table_index_offset < (int)current_row_of_data.indices_of_all_columns.size())
								{
									merged_data_row.current_parameter_ints.push_back(
										current_row_of_data.current_parameter_ints[current_row_of_data.indices_of_all_columns[current_index + current_inner_table_index_offset].second.first]);
								}
								else
								{
									// otherwise, the not-yet-appended datetime columns, which will be added and written to later - this data does not need to be correct
									merged_data_row.current_parameter_ints.push_back(current_row_of_data.current_parameter_ints[current_row_of_data.indices_of_all_columns[current_index].second.first]);
								}

								merged_data_row.indices_of_all_columns.push_back(std::make_pair(SQLExecutor::INT64, std::make_pair((int)merged_data_row.current_parameter_ints.size() - 1, current_index)));

								if (current_row_of_data.is_index_a_primary_key[current_index])
								{
									merged_data_row.indices_of_primary_key_columns.push_back(std::make_pair(SQLExecutor::INT64, std::make_pair((int)merged_data_row.current_parameter_ints.size() - 1, current_index)));
								}

								if (current_row_of_data.is_index_a_primary_key_with_outer_multiplicity_greater_than_1[current_index])
								{
									merged_data_row.indices_of_primary_key_columns_with_multiplicity_greater_than_1.push_back(std::make_pair(SQLExecutor::INT64,
											std::make_pair((int)merged_data_row.current_parameter_ints.size() - 1, current_index)));
								}

								if (current_row_of_data.is_index_a_primary_key_with_outer_multiplicity_equal_to_1[current_index])
								{
									merged_data_row.indices_of_primary_key_columns_with_multiplicity_equal_to_1.push_back(std::make_pair(SQLExecutor::INT64,
											std::make_pair((int)merged_data_row.current_parameter_ints.size() - 1, current_index)));
								}

								if (merged_data_row.is_index_in_final_inner_table[current_index])
								{
									merged_data_row.indices_of_all_columns_in_final_inner_table.push_back(std::make_pair(SQLExecutor::INT64, std::make_pair((int)merged_data_row.current_parameter_ints.size() - 1,
											current_index)));
								}

								if (merged_data_row.is_index_in_all_but_final_inner_table[current_index])
								{
									merged_data_row.indices_of_all_columns_in_all_but_final_inner_table.push_back(std::make_pair(SQLExecutor::INT64,
											std::make_pair((int)merged_data_row.current_parameter_ints.size() - 1, current_index)));
								}

								if (merged_data_row.is_index_a_primary_key_in_not_the_final_inner_table[current_index])
								{
									merged_data_row.indices_of_all_primary_key_columns_in_all_but_final_inner_table.push_back(std::make_pair(SQLExecutor::INT64,
											std::make_pair((int)merged_data_row.current_parameter_ints.size() - 1, current_index)));
								}

								if (merged_data_row.is_index_a_primary_key_in_the_final_inner_table[current_index])
								{
									merged_data_row.indices_of_all_primary_key_columns_in_final_inner_table.push_back(std::make_pair(SQLExecutor::INT64,
											std::make_pair((int)merged_data_row.current_parameter_ints.size() - 1, current_index)));
								}

								if (merged_data_row.is_index_a_secondary_key[current_index])
								{
									merged_data_row.indices_of_secondary_key_columns.push_back(std::make_pair(SQLExecutor::INT64, std::make_pair((int)merged_data_row.current_parameter_ints.size() - 1,
											current_index)));
								}

							}
							break;

						case SQLExecutor::FLOAT:
							{

								merged_data_row.current_parameter_which_binding_to_use.push_back(SQLExecutor::FLOAT);

								if (current_index + current_inner_table_index_offset < (int)current_row_of_data.indices_of_all_columns.size())
								{
									merged_data_row.current_parameter_floats.push_back(
										current_row_of_data.current_parameter_floats[current_row_of_data.indices_of_all_columns[current_index + current_inner_table_index_offset].second.first]);
								}
								else
								{
									// otherwise, the not-yet-appended datetime columns, which will be added and written to later - this data does not need to be correct
									merged_data_row.current_parameter_floats.push_back(current_row_of_data.current_parameter_floats[current_row_of_data.indices_of_all_columns[current_index].second.first]);
								}

								merged_data_row.indices_of_all_columns.push_back(std::make_pair(SQLExecutor::FLOAT, std::make_pair((int)merged_data_row.current_parameter_floats.size() - 1, current_index)));

								if (current_row_of_data.is_index_a_primary_key[current_index])
								{
									merged_data_row.indices_of_primary_key_columns.push_back(std::make_pair(SQLExecutor::FLOAT, std::make_pair((int)merged_data_row.current_parameter_floats.size() - 1,
											current_index)));
								}

								if (current_row_of_data.is_index_a_primary_key_with_outer_multiplicity_greater_than_1[current_index])
								{
									merged_data_row.indices_of_primary_key_columns_with_multiplicity_greater_than_1.push_back(std::make_pair(SQLExecutor::FLOAT,
											std::make_pair((int)merged_data_row.current_parameter_floats.size() - 1, current_index)));
								}

								if (current_row_of_data.is_index_a_primary_key_with_outer_multiplicity_equal_to_1[current_index])
								{
									merged_data_row.indices_of_primary_key_columns_with_multiplicity_equal_to_1.push_back(std::make_pair(SQLExecutor::FLOAT,
											std::make_pair((int)merged_data_row.current_parameter_floats.size() - 1, current_index)));
								}

								if (merged_data_row.is_index_in_final_inner_table[current_index])
								{
									merged_data_row.indices_of_all_columns_in_final_inner_table.push_back(std::make_pair(SQLExecutor::FLOAT, std::make_pair((int)merged_data_row.current_parameter_floats.size() - 1,
											current_index)));
								}

								if (merged_data_row.is_index_in_all_but_final_inner_table[current_index])
								{
									merged_data_row.indices_of_all_columns_in_all_but_final_inner_table.push_back(std::make_pair(SQLExecutor::FLOAT,
											std::make_pair((int)merged_data_row.current_parameter_floats.size() - 1, current_index)));
								}

								if (merged_data_row.is_index_a_primary_key_in_not_the_final_inner_table[current_index])
								{
									merged_data_row.indices_of_all_primary_key_columns_in_all_but_final_inner_table.push_back(std::make_pair(SQLExecutor::FLOAT,
											std::make_pair((int)merged_data_row.current_parameter_floats.size() - 1, current_index)));
								}

								if (merged_data_row.is_index_a_primary_key_in_the_final_inner_table[current_index])
								{
									merged_data_row.indices_of_all_primary_key_columns_in_final_inner_table.push_back(std::make_pair(SQLExecutor::FLOAT,
											std::make_pair((int)merged_data_row.current_parameter_floats.size() - 1, current_index)));
								}

								if (merged_data_row.is_index_a_secondary_key[current_index])
								{
									merged_data_row.indices_of_secondary_key_columns.push_back(std::make_pair(SQLExecutor::FLOAT, std::make_pair((int)merged_data_row.current_parameter_floats.size() - 1,
											current_index)));
								}

							}
							break;

						case SQLExecutor::STRING:
							{

								merged_data_row.current_parameter_which_binding_to_use.push_back(SQLExecutor::STRING);

								if (current_index + current_inner_table_index_offset < (int)current_row_of_data.indices_of_all_columns.size())
								{
									merged_data_row.current_parameter_strings.push_back(
										current_row_of_data.current_parameter_strings[current_row_of_data.indices_of_all_columns[current_index + current_inner_table_index_offset].second.first]);
								}
								else
								{
									// otherwise, the not-yet-appended datetime columns, which will be added and written to later - this data does not need to be correct
									merged_data_row.current_parameter_strings.push_back(current_row_of_data.current_parameter_strings[current_row_of_data.indices_of_all_columns[current_index].second.first]);
								}

								merged_data_row.indices_of_all_columns.push_back(std::make_pair(SQLExecutor::STRING, std::make_pair((int)merged_data_row.current_parameter_strings.size() - 1, current_index)));

								if (current_row_of_data.is_index_a_primary_key[current_index])
								{
									merged_data_row.indices_of_primary_key_columns.push_back(std::make_pair(SQLExecutor::STRING, std::make_pair((int)merged_data_row.current_parameter_strings.size() - 1,
											current_index)));
								}

								if (current_row_of_data.is_index_a_primary_key_with_outer_multiplicity_greater_than_1[current_index])
								{
									merged_data_row.indices_of_primary_key_columns_with_multiplicity_greater_than_1.push_back(std::make_pair(SQLExecutor::STRING,
											std::make_pair((int)merged_data_row.current_parameter_strings.size() - 1, current_index)));
								}

								if (current_row_of_data.is_index_a_primary_key_with_outer_multiplicity_equal_to_1[current_index])
								{
									merged_data_row.indices_of_primary_key_columns_with_multiplicity_equal_to_1.push_back(std::make_pair(SQLExecutor::STRING,
											std::make_pair((int)merged_data_row.current_parameter_strings.size() - 1, current_index)));
								}

								if (merged_data_row.is_index_in_final_inner_table[current_index])
								{
									merged_data_row.indices_of_all_columns_in_final_inner_table.push_back(std::make_pair(SQLExecutor::STRING, std::make_pair((int)merged_data_row.current_parameter_strings.size() - 1,
											current_index)));
								}

								if (merged_data_row.is_index_in_all_but_final_inner_table[current_index])
								{
									merged_data_row.indices_of_all_columns_in_all_but_final_inner_table.push_back(std::make_pair(SQLExecutor::STRING,
											std::make_pair((int)merged_data_row.current_parameter_strings.size() - 1, current_index)));
								}

								if (merged_data_row.is_index_a_primary_key_in_not_the_final_inner_table[current_index])
								{
									merged_data_row.indices_of_all_primary_key_columns_in_all_but_final_inner_table.push_back(std::make_pair(SQLExecutor::STRING,
											std::make_pair((int)merged_data_row.current_parameter_strings.size() - 1, current_index)));
								}

								if (merged_data_row.is_index_a_primary_key_in_the_final_inner_table[current_index])
								{
									merged_data_row.indices_of_all_primary_key_columns_in_final_inner_table.push_back(std::make_pair(SQLExecutor::STRING,
											std::make_pair((int)merged_data_row.current_parameter_strings.size() - 1, current_index)));
								}

								if (current_row_of_data.is_index_a_secondary_key[current_index])
								{
									merged_data_row.indices_of_secondary_key_columns.push_back(std::make_pair(SQLExecutor::STRING, std::make_pair((int)merged_data_row.current_parameter_strings.size() - 1,
											current_index)));
								}

							}
							break;

						default:
							break;
					}
				}
				else
				{
					switch (previous_row_binding)
					{
						case SQLExecutor::INT64:
							{

								merged_data_row.current_parameter_which_binding_to_use.push_back(SQLExecutor::INT64);

								if (current_index + previous_inner_table_index_offset < (int)previous_row_of_data.indices_of_all_columns.size())
								{
									merged_data_row.current_parameter_ints.push_back(
										previous_row_of_data.current_parameter_ints[previous_row_of_data.indices_of_all_columns[current_index + previous_inner_table_index_offset].second.first]);
								}
								else
								{
									// otherwise, the not-yet-appended datetime columns, which will be added and written to later - this data does not need to be correct
									merged_data_row.current_parameter_ints.push_back(previous_row_of_data.current_parameter_ints[previous_row_of_data.indices_of_all_columns[current_index].second.first]);
								}

								merged_data_row.indices_of_all_columns.push_back(std::make_pair(SQLExecutor::INT64, std::make_pair((int)merged_data_row.current_parameter_ints.size() - 1, current_index)));

								if (previous_row_of_data.is_index_a_primary_key[current_index])
								{
									merged_data_row.indices_of_primary_key_columns.push_back(std::make_pair(SQLExecutor::INT64, std::make_pair((int)merged_data_row.current_parameter_ints.size() - 1, current_index)));
								}

								if (previous_row_of_data.is_index_a_primary_key_with_outer_multiplicity_greater_than_1[current_index])
								{
									merged_data_row.indices_of_primary_key_columns_with_multiplicity_greater_than_1.push_back(std::make_pair(SQLExecutor::INT64,
											std::make_pair((int)merged_data_row.current_parameter_ints.size() - 1, current_index)));
								}

								if (previous_row_of_data.is_index_a_primary_key_with_outer_multiplicity_equal_to_1[current_index])
								{
									merged_data_row.indices_of_primary_key_columns_with_multiplicity_equal_to_1.push_back(std::make_pair(SQLExecutor::INT64,
											std::make_pair((int)merged_data_row.current_parameter_ints.size() - 1, current_index)));
								}

								if (merged_data_row.is_index_in_final_inner_table[current_index])
								{
									merged_data_row.indices_of_all_columns_in_final_inner_table.push_back(std::make_pair(SQLExecutor::INT64, std::make_pair((int)merged_data_row.current_parameter_ints.size() - 1,
											current_index)));
								}

								if (merged_data_row.is_index_in_all_but_final_inner_table[current_index])
								{
									merged_data_row.indices_of_all_columns_in_all_but_final_inner_table.push_back(std::make_pair(SQLExecutor::INT64,
											std::make_pair((int)merged_data_row.current_parameter_ints.size() - 1, current_index)));
								}

								if (merged_data_row.is_index_a_primary_key_in_not_the_final_inner_table[current_index])
								{
									merged_data_row.indices_of_all_primary_key_columns_in_all_but_final_inner_table.push_back(std::make_pair(SQLExecutor::INT64,
											std::make_pair((int)merged_data_row.current_parameter_ints.size() - 1, current_index)));
								}

								if (merged_data_row.is_index_a_primary_key_in_the_final_inner_table[current_index])
								{
									merged_data_row.indices_of_all_primary_key_columns_in_final_inner_table.push_back(std::make_pair(SQLExecutor::INT64,
											std::make_pair((int)merged_data_row.current_parameter_ints.size() - 1, current_index)));
								}

								if (merged_data_row.is_index_a_secondary_key[current_index])
								{
									merged_data_row.indices_of_secondary_key_columns.push_back(std::make_pair(SQLExecutor::INT64, std::make_pair((int)merged_data_row.current_parameter_ints.size() - 1,
											current_index)));
								}

							}
							break;

						case SQLExecutor::FLOAT:
							{

								merged_data_row.current_parameter_which_binding_to_use.push_back(SQLExecutor::FLOAT);

								if (current_index + previous_inner_table_index_offset < (int)previous_row_of_data.indices_of_all_columns.size())
								{
									merged_data_row.current_parameter_floats.push_back(
										previous_row_of_data.current_parameter_floats[previous_row_of_data.indices_of_all_columns[current_index + previous_inner_table_index_offset].second.first]);
								}
								else
								{
									// otherwise, the not-yet-appended datetime columns, which will be added and written to later - this data does not need to be correct
									merged_data_row.current_parameter_floats.push_back(previous_row_of_data.current_parameter_floats[previous_row_of_data.indices_of_all_columns[current_index].second.first]);
								}

								merged_data_row.indices_of_all_columns.push_back(std::make_pair(SQLExecutor::FLOAT, std::make_pair((int)merged_data_row.current_parameter_floats.size() - 1, current_index)));

								if (previous_row_of_data.is_index_a_primary_key[current_index])
								{
									merged_data_row.indices_of_primary_key_columns.push_back(std::make_pair(SQLExecutor::FLOAT, std::make_pair((int)merged_data_row.current_parameter_floats.size() - 1,
											current_index)));
								}

								if (previous_row_of_data.is_index_a_primary_key_with_outer_multiplicity_greater_than_1[current_index])
								{
									merged_data_row.indices_of_primary_key_columns_with_multiplicity_greater_than_1.push_back(std::make_pair(SQLExecutor::FLOAT,
											std::make_pair((int)merged_data_row.current_parameter_floats.size() - 1, current_index)));
								}

								if (previous_row_of_data.is_index_a_primary_key_with_outer_multiplicity_equal_to_1[current_index])
								{
									merged_data_row.indices_of_primary_key_columns_with_multiplicity_equal_to_1.push_back(std::make_pair(SQLExecutor::FLOAT,
											std::make_pair((int)merged_data_row.current_parameter_floats.size() - 1, current_index)));
								}

								if (merged_data_row.is_index_in_final_inner_table[current_index])
								{
									merged_data_row.indices_of_all_columns_in_final_inner_table.push_back(std::make_pair(SQLExecutor::FLOAT, std::make_pair((int)merged_data_row.current_parameter_floats.size() - 1,
											current_index)));
								}

								if (merged_data_row.is_index_in_all_but_final_inner_table[current_index])
								{
									merged_data_row.indices_of_all_columns_in_all_but_final_inner_table.push_back(std::make_pair(SQLExecutor::FLOAT,
											std::make_pair((int)merged_data_row.current_parameter_floats.size() - 1, current_index)));
								}

								if (merged_data_row.is_index_a_primary_key_in_not_the_final_inner_table[current_index])
								{
									merged_data_row.indices_of_all_primary_key_columns_in_all_but_final_inner_table.push_back(std::make_pair(SQLExecutor::FLOAT,
											std::make_pair((int)merged_data_row.current_parameter_floats.size() - 1, current_index)));
								}

								if (merged_data_row.is_index_a_primary_key_in_the_final_inner_table[current_index])
								{
									merged_data_row.indices_of_all_primary_key_columns_in_final_inner_table.push_back(std::make_pair(SQLExecutor::FLOAT,
											std::make_pair((int)merged_data_row.current_parameter_floats.size() - 1, current_index)));
								}

								if (merged_data_row.is_index_a_secondary_key[current_index])
								{
									merged_data_row.indices_of_secondary_key_columns.push_back(std::make_pair(SQLExecutor::FLOAT, std::make_pair((int)merged_data_row.current_parameter_floats.size() - 1,
											current_index)));
								}

							}
							break;

						case SQLExecutor::STRING:
							{

								merged_data_row.current_parameter_which_binding_to_use.push_back(SQLExecutor::STRING);

								if (current_index + previous_inner_table_index_offset < (int)previous_row_of_data.indices_of_all_columns.size())
								{
									merged_data_row.current_parameter_strings.push_back(
										previous_row_of_data.current_parameter_strings[previous_row_of_data.indices_of_all_columns[current_index + previous_inner_table_index_offset].second.first]);
								}
								else
								{
									// otherwise, the not-yet-appended datetime columns, which will be added and written to later - this data does not need to be correct
									merged_data_row.current_parameter_strings.push_back(previous_row_of_data.current_parameter_strings[previous_row_of_data.indices_of_all_columns[current_index].second.first]);
								}

								merged_data_row.indices_of_all_columns.push_back(std::make_pair(SQLExecutor::STRING, std::make_pair((int)merged_data_row.current_parameter_strings.size() - 1, current_index)));

								if (previous_row_of_data.is_index_a_primary_key[current_index])
								{
									merged_data_row.indices_of_primary_key_columns.push_back(std::make_pair(SQLExecutor::STRING, std::make_pair((int)merged_data_row.current_parameter_strings.size() - 1,
											current_index)));
								}

								if (previous_row_of_data.is_index_a_primary_key_with_outer_multiplicity_greater_than_1[current_index])
								{
									merged_data_row.indices_of_primary_key_columns_with_multiplicity_greater_than_1.push_back(std::make_pair(SQLExecutor::STRING,
											std::make_pair((int)merged_data_row.current_parameter_strings.size() - 1, current_index)));
								}

								if (previous_row_of_data.is_index_a_primary_key_with_outer_multiplicity_equal_to_1[current_index])
								{
									merged_data_row.indices_of_primary_key_columns_with_multiplicity_equal_to_1.push_back(std::make_pair(SQLExecutor::STRING,
											std::make_pair((int)merged_data_row.current_parameter_strings.size() - 1, current_index)));
								}

								if (merged_data_row.is_index_in_final_inner_table[current_index])
								{
									merged_data_row.indices_of_all_columns_in_final_inner_table.push_back(std::make_pair(SQLExecutor::STRING, std::make_pair((int)merged_data_row.current_parameter_strings.size() - 1,
											current_index)));
								}

								if (merged_data_row.is_index_in_all_but_final_inner_table[current_index])
								{
									merged_data_row.indices_of_all_columns_in_all_but_final_inner_table.push_back(std::make_pair(SQLExecutor::STRING,
											std::make_pair((int)merged_data_row.current_parameter_strings.size() - 1, current_index)));
								}

								if (merged_data_row.is_index_a_primary_key_in_not_the_final_inner_table[current_index])
								{
									merged_data_row.indices_of_all_primary_key_columns_in_all_but_final_inner_table.push_back(std::make_pair(SQLExecutor::STRING,
											std::make_pair((int)merged_data_row.current_parameter_strings.size() - 1, current_index)));
								}

								if (merged_data_row.is_index_a_primary_key_in_the_final_inner_table[current_index])
								{
									merged_data_row.indices_of_all_primary_key_columns_in_final_inner_table.push_back(std::make_pair(SQLExecutor::STRING,
											std::make_pair((int)merged_data_row.current_parameter_strings.size() - 1, current_index)));
								}

								if (merged_data_row.is_index_a_secondary_key[current_index])
								{
									merged_data_row.indices_of_secondary_key_columns.push_back(std::make_pair(SQLExecutor::STRING, std::make_pair((int)merged_data_row.current_parameter_strings.size() - 1,
											current_index)));
								}

							}
							break;

						default:
							break;
					}
				}
			}

		}


		switch (current_row_binding)
		{
			case SQLExecutor::INT64:
				{
					++int_index_current;
				}
				break;

			case SQLExecutor::FLOAT:
				{
					++float_index_current;
				}
				break;

			case SQLExecutor::STRING:
				{
					++string_index_current;
				}
				break;

			default:
				break;
		}

		switch (previous_row_binding)
		{
			case SQLExecutor::INT64:
				{
					++int_index_previous;
				}
				break;

			case SQLExecutor::FLOAT:
				{
					++float_index_previous;
				}
				break;

			case SQLExecutor::STRING:
				{
					++string_index_previous;
				}
				break;

			default:
				break;
		}

		if (current_row_of_data.is_index_a_primary_key_with_outer_multiplicity_greater_than_1[current_index])
		{
			++inner_multiplicity_current_index;
		}

		if (inner_multiplicity_current_index ==
			current_row_of_data.number_of_columns__in_a_single_inner_table__for_the_columns_only_having_the_dmu_category_with_multiplicity_greater_than_one__but_this_info_is_present_for_all_primary_key_columns)
		{
			if (use_ints)
			{
				if (!saved_ints_deque.empty())
				{
					saved_ints_deque.pop_front();
				}
			}

			if (use_floats)
			{
				if (!saved_floats_deque.empty())
				{
					saved_floats_deque.pop_front();
				}
			}

			if (use_strings)
			{
				if (!saved_strings_deque.empty())
				{
					saved_strings_deque.pop_front();
				}
			}

			inner_multiplicity_current_index = 0;
		}

		++current_index;

		//});
	}

}

bool OutputModel::OutputGenerator::TestPrimaryKeyMatch(SavedRowData const & current_row_of_data, SavedRowData const & previous_row_of_data, bool & use_newest_row_index,
		PRIMARY_KEY_MATCH_CONDITION const match_condition)
{

	// This is a special matching function that tests all the primary keys
	// (but if the last argument is true, it skips the primary keys in the last inner table)
	// ... and returns true (a match) if the primary keys from different inner tables
	// match, regardless of the order they appear in the inner tables.
	// Furthermore, inner tables with NULL primary keys are skipped during this check.
	// Alongside this skipping of NULL primary keys in the test,
	// a true value will be returned (successful match) if one of the rows being
	// tested has a SMALL number of non-NULL primary keys than the other row,
	// just so long as all of its non-NULL keys match at least one of the
	// keys in the other row.

	bool match_failed = false;
	int entry_number = 0;
	use_newest_row_index = false;

	// First test multiplicity = 1 columns, which must match exactly in sequence between the two rows being compared
	std::for_each(current_row_of_data.indices_of_primary_key_columns_with_multiplicity_equal_to_1.cbegin(),
				  current_row_of_data.indices_of_primary_key_columns_with_multiplicity_equal_to_1.cend(), [this, &entry_number, &current_row_of_data, &previous_row_of_data, &match_failed](
					  std::pair<SQLExecutor::WHICH_BINDING, std::pair<int, int>> const & current_info)
	{

		if (match_failed)
		{
			return;
		}

		SQLExecutor::WHICH_BINDING binding_current = current_info.first;
		int index_current__data_vectors = current_info.second.first;
		std::int64_t data_int_current = 0;
		long double data_float_current = 0.0;
		std::string data_string_current;
		bool data_null_current = false;

		SQLExecutor::WHICH_BINDING binding_previous = previous_row_of_data.indices_of_primary_key_columns_with_multiplicity_equal_to_1[entry_number].first;
		int index_previous__data_vectors = previous_row_of_data.indices_of_primary_key_columns_with_multiplicity_equal_to_1[entry_number].second.first;
		std::int64_t data_int_previous = 0;
		long double data_float_previous = 0.0;
		std::string data_string_previous;

		switch (binding_current)
		{

			case SQLExecutor::INT64:
				{

					data_int_current = current_row_of_data.current_parameter_ints[index_current__data_vectors];

					switch (binding_previous)
					{
						case SQLExecutor::INT64:
							{
								data_int_previous = previous_row_of_data.current_parameter_ints[index_previous__data_vectors];

								if (data_int_current != data_int_previous)
								{
									match_failed = true;
								}
							}
							break;

						case SQLExecutor::FLOAT:
							{
								data_float_previous = previous_row_of_data.current_parameter_floats[index_previous__data_vectors];

								if (boost::lexical_cast<long double>(data_int_current) != data_float_previous)
								{
									match_failed = true;
								}
							}
							break;

						case SQLExecutor::STRING:
							{
								data_string_previous = previous_row_of_data.current_parameter_strings[index_previous__data_vectors];

								if (data_int_current != boost::lexical_cast<std::int64_t>(data_string_previous))
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

						default:
							break;
					}

				}
				break;

			case SQLExecutor::FLOAT:
				{

					data_float_current = current_row_of_data.current_parameter_floats[index_current__data_vectors];

					switch (binding_previous)
					{
						case SQLExecutor::INT64:
							{
								data_int_previous = previous_row_of_data.current_parameter_ints[index_previous__data_vectors];

								if (data_float_current != boost::lexical_cast<long double>(data_int_previous))
								{
									match_failed = true;
								}
							}
							break;

						case SQLExecutor::FLOAT:
							{
								data_float_previous = previous_row_of_data.current_parameter_floats[index_previous__data_vectors];

								if (data_float_current != data_float_previous)
								{
									match_failed = true;
								}
							}
							break;

						case SQLExecutor::STRING:
							{
								data_string_previous = previous_row_of_data.current_parameter_strings[index_previous__data_vectors];

								if (data_float_current != boost::lexical_cast<long double>(data_string_previous))
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

						default:
							break;
					}

				}
				break;

			case SQLExecutor::STRING:
				{

					data_string_current = current_row_of_data.current_parameter_strings[index_current__data_vectors];

					switch (binding_previous)
					{
						case SQLExecutor::INT64:
							{
								data_int_previous = previous_row_of_data.current_parameter_ints[index_previous__data_vectors];

								if (boost::lexical_cast<std::int64_t>(data_string_current) != data_int_previous)
								{
									match_failed = true;
								}
							}
							break;

						case SQLExecutor::FLOAT:
							{
								data_float_previous = previous_row_of_data.current_parameter_floats[index_previous__data_vectors];

								if (boost::lexical_cast<long double>(data_string_current) != data_float_previous)
								{
									match_failed = true;
								}
							}
							break;

						case SQLExecutor::STRING:
							{
								data_string_previous = previous_row_of_data.current_parameter_strings[index_previous__data_vectors];

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

						default:
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

						case SQLExecutor::FLOAT:
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

						default:
							break;
					}

				}
				break;

			default:
				break;

		}

		++entry_number;

	});

	// If the match failed already, we know it doesn't match
	if (match_failed)
	{
		return false;
	}




	if (match_condition != PRIMARY_KEY_MATCH_CONDITION::MATCH_ON_ALL_MULTIPLICITY_1_KEYS)
	{

		// ************************************************************************************************** //
		// Special handling required to test the single DMU category with multiplicity greater than 1,
		// because there could be NULL values that offset what would otherwise be a match.
		// i.e.:
		// 2   200
		// 200
		// ... should match, because the NULL can be overwritten by the 2 if the 200 were in the second column
		// ************************************************************************************************** //

		std::map<std::vector<std::string>, int> saved_strings_previous__map_from_inner_primary_key_group__to__count;
		std::map<std::vector<std::int64_t>, int> saved_ints_previous__map_from_inner_primary_key_group__to__count;
		std::map<std::vector<long double>, int> saved_floats_previous__map_from_inner_primary_key_group__to__count;
		std::map<std::vector<std::string>, int> saved_strings_current__map_from_inner_primary_key_group__to__count;
		std::map<std::vector<std::int64_t>, int> saved_ints_current__map_from_inner_primary_key_group__to__count;
		std::map<std::vector<long double>, int> saved_floats_current__map_from_inner_primary_key_group__to__count;

		int inner_multiplicity_index = 0;
		int outer_multiplicity_number = 1;
		int the_index = 0;
		std::vector<std::string> inner_multiplicity_string_vector;
		std::vector<std::int64_t> inner_multiplicity_int_vector;
		std::vector<long double> inner_multiplicity_float_vector;
		int number_null_primary_key_groups_in_current_row = 0;
		bool current_row_current_inner_table_primary_key_group_is_null = false;
		std::for_each(current_row_of_data.indices_of_primary_key_columns_with_multiplicity_greater_than_1.cbegin(),
					  current_row_of_data.indices_of_primary_key_columns_with_multiplicity_greater_than_1.cend(), [this, &outer_multiplicity_number, &current_row_current_inner_table_primary_key_group_is_null, &number_null_primary_key_groups_in_current_row, &match_condition, &inner_multiplicity_int_vector, &inner_multiplicity_float_vector, &inner_multiplicity_string_vector, &the_index, &inner_multiplicity_index, &saved_strings_previous__map_from_inner_primary_key_group__to__count, &saved_ints_previous__map_from_inner_primary_key_group__to__count, &saved_floats_previous__map_from_inner_primary_key_group__to__count, &saved_strings_current__map_from_inner_primary_key_group__to__count, &saved_ints_current__map_from_inner_primary_key_group__to__count, &saved_floats_current__map_from_inner_primary_key_group__to__count, &current_row_of_data, &previous_row_of_data](
						  std::pair<SQLExecutor::WHICH_BINDING, std::pair<int, int>> const & current_info)
		{

			SQLExecutor::WHICH_BINDING binding = current_info.first;
			int index_in_data_vectors = current_info.second.first;
			std::int64_t data_int = 0;
			long double data_float = 0.0;
			std::string data_string;
			//bool data_null = false;

			switch (binding)
			{

				case SQLExecutor::INT64:
					{
						data_int = current_row_of_data.current_parameter_ints[index_in_data_vectors];
						inner_multiplicity_int_vector.push_back(data_int);
					}
					break;

				case SQLExecutor::FLOAT:
					{
						data_float = current_row_of_data.current_parameter_floats[index_in_data_vectors];
						inner_multiplicity_float_vector.push_back(data_float);
					}
					break;

				case SQLExecutor::STRING:
					{
						data_string = current_row_of_data.current_parameter_strings[index_in_data_vectors];
						inner_multiplicity_string_vector.push_back(data_string);
					}
					break;

				case SQLExecutor::NULL_BINDING:
					{
						if (!current_row_current_inner_table_primary_key_group_is_null)
						{
							++number_null_primary_key_groups_in_current_row;
							current_row_current_inner_table_primary_key_group_is_null = true;
						}
					}
					break;

				default:
					break;

			}

			++the_index;
			++inner_multiplicity_index;

			if (inner_multiplicity_index ==
				current_row_of_data.number_of_columns__in_a_single_inner_table__for_the_columns_only_having_the_dmu_category_with_multiplicity_greater_than_one__but_this_info_is_present_for_all_primary_key_columns)
			{
				if (!inner_multiplicity_int_vector.empty())
				{
					bool do_insert = false;

					if (match_condition == PRIMARY_KEY_MATCH_CONDITION::MATCH_ON_ALL_BUT_FINAL_INNER_TABLE)
					{
						if (outer_multiplicity_number < current_row_of_data.number_of_multiplicities)
						{
							do_insert = true;
						}
					}
					else if (match_condition == PRIMARY_KEY_MATCH_CONDITION::MATCH_ON_ALL_BUT_FINAL_TWO_INNER_TABLES)
					{
						if (outer_multiplicity_number < current_row_of_data.number_of_multiplicities - 1)
						{
							do_insert = true;
						}
					}
					else
					{
						do_insert = true;
					}

					if (do_insert)
					{
						if (saved_ints_current__map_from_inner_primary_key_group__to__count.find(inner_multiplicity_int_vector) != saved_ints_current__map_from_inner_primary_key_group__to__count.cend())
						{
							int current_count = saved_ints_current__map_from_inner_primary_key_group__to__count[inner_multiplicity_int_vector];
							++current_count;
							saved_ints_current__map_from_inner_primary_key_group__to__count[inner_multiplicity_int_vector] = current_count;
						}
						else
						{
							saved_ints_current__map_from_inner_primary_key_group__to__count[inner_multiplicity_int_vector] = 1;
						}
					}

					inner_multiplicity_int_vector.clear();
				}

				if (!inner_multiplicity_float_vector.empty())
				{
					bool do_insert = false;

					if (match_condition == PRIMARY_KEY_MATCH_CONDITION::MATCH_ON_ALL_BUT_FINAL_INNER_TABLE)
					{
						if (outer_multiplicity_number < current_row_of_data.number_of_multiplicities)
						{
							do_insert = true;
						}
					}
					else if (match_condition == PRIMARY_KEY_MATCH_CONDITION::MATCH_ON_ALL_BUT_FINAL_TWO_INNER_TABLES)
					{
						if (outer_multiplicity_number < current_row_of_data.number_of_multiplicities - 1)
						{
							do_insert = true;
						}
					}
					else
					{
						do_insert = true;
					}

					if (do_insert)
					{
						if (saved_floats_current__map_from_inner_primary_key_group__to__count.find(inner_multiplicity_float_vector) !=
							saved_floats_current__map_from_inner_primary_key_group__to__count.cend())
						{
							int current_count = saved_floats_current__map_from_inner_primary_key_group__to__count[inner_multiplicity_float_vector];
							++current_count;
							saved_floats_current__map_from_inner_primary_key_group__to__count[inner_multiplicity_float_vector] = current_count;
						}
						else
						{
							saved_floats_current__map_from_inner_primary_key_group__to__count[inner_multiplicity_float_vector] = 1;
						}
					}

					inner_multiplicity_float_vector.clear();
				}

				if (!inner_multiplicity_string_vector.empty())
				{
					bool do_insert = false;

					if (match_condition == PRIMARY_KEY_MATCH_CONDITION::MATCH_ON_ALL_BUT_FINAL_INNER_TABLE)
					{
						if (outer_multiplicity_number < current_row_of_data.number_of_multiplicities)
						{
							do_insert = true;
						}
					}
					else if (match_condition == PRIMARY_KEY_MATCH_CONDITION::MATCH_ON_ALL_BUT_FINAL_TWO_INNER_TABLES)
					{
						if (outer_multiplicity_number < current_row_of_data.number_of_multiplicities - 1)
						{
							do_insert = true;
						}
					}
					else
					{
						do_insert = true;
					}

					if (do_insert)
					{
						if (saved_strings_current__map_from_inner_primary_key_group__to__count.find(inner_multiplicity_string_vector) !=
							saved_strings_current__map_from_inner_primary_key_group__to__count.cend())
						{
							int current_count = saved_strings_current__map_from_inner_primary_key_group__to__count[inner_multiplicity_string_vector];
							++current_count;
							saved_strings_current__map_from_inner_primary_key_group__to__count[inner_multiplicity_string_vector] = current_count;
						}
						else
						{
							saved_strings_current__map_from_inner_primary_key_group__to__count[inner_multiplicity_string_vector] = 1;
						}
					}

					inner_multiplicity_string_vector.clear();
				}

				inner_multiplicity_index = 0;
				++outer_multiplicity_number;
				current_row_current_inner_table_primary_key_group_is_null = false;
			}

		});

		inner_multiplicity_string_vector.clear();
		inner_multiplicity_int_vector.clear();
		inner_multiplicity_float_vector.clear();
		inner_multiplicity_index = 0;
		outer_multiplicity_number = 1;
		int number_null_primary_key_groups_in_previous_row = 0;
		bool previous_row_current_inner_table_primary_key_group_is_null = false;
		the_index = 0;
		std::for_each(previous_row_of_data.indices_of_primary_key_columns_with_multiplicity_greater_than_1.cbegin(),
					  previous_row_of_data.indices_of_primary_key_columns_with_multiplicity_greater_than_1.cend(), [this, &outer_multiplicity_number, &previous_row_current_inner_table_primary_key_group_is_null, &number_null_primary_key_groups_in_previous_row, &match_condition, &inner_multiplicity_int_vector, &inner_multiplicity_float_vector, &inner_multiplicity_string_vector, &the_index, &inner_multiplicity_index, &saved_strings_previous__map_from_inner_primary_key_group__to__count, &saved_ints_previous__map_from_inner_primary_key_group__to__count, &saved_floats_previous__map_from_inner_primary_key_group__to__count, &saved_strings_current__map_from_inner_primary_key_group__to__count, &saved_ints_current__map_from_inner_primary_key_group__to__count, &saved_floats_current__map_from_inner_primary_key_group__to__count, &current_row_of_data, &previous_row_of_data](
						  std::pair<SQLExecutor::WHICH_BINDING, std::pair<int, int>> const & previous_info)
		{

			SQLExecutor::WHICH_BINDING binding = previous_info.first;
			int index_in_data_vectors = previous_info.second.first;
			std::int64_t data_int = 0;
			long double data_float = 0.0;
			std::string data_string;
			//bool data_null = false;

			switch (binding)
			{

				case SQLExecutor::INT64:
					{
						data_int = previous_row_of_data.current_parameter_ints[index_in_data_vectors];
						inner_multiplicity_int_vector.push_back(data_int);
					}
					break;

				case SQLExecutor::FLOAT:
					{
						data_float = previous_row_of_data.current_parameter_floats[index_in_data_vectors];
						inner_multiplicity_float_vector.push_back(data_float);
					}
					break;

				case SQLExecutor::STRING:
					{
						data_string = previous_row_of_data.current_parameter_strings[index_in_data_vectors];
						inner_multiplicity_string_vector.push_back(data_string);
					}
					break;

				case SQLExecutor::NULL_BINDING:
					{
						if (!previous_row_current_inner_table_primary_key_group_is_null)
						{
							++number_null_primary_key_groups_in_previous_row;
							previous_row_current_inner_table_primary_key_group_is_null = true;
						}
					}
					break;

				default:
					break;

			}

			++the_index;
			++inner_multiplicity_index;

			if (inner_multiplicity_index ==
				current_row_of_data.number_of_columns__in_a_single_inner_table__for_the_columns_only_having_the_dmu_category_with_multiplicity_greater_than_one__but_this_info_is_present_for_all_primary_key_columns)
			{
				if (!inner_multiplicity_int_vector.empty())
				{
					bool do_insert = false;

					if (match_condition == PRIMARY_KEY_MATCH_CONDITION::MATCH_ON_ALL_BUT_FINAL_INNER_TABLE)
					{
						if (outer_multiplicity_number < current_row_of_data.number_of_multiplicities)
						{
							do_insert = true;
						}
					}
					else if (match_condition == PRIMARY_KEY_MATCH_CONDITION::MATCH_ON_ALL_BUT_FINAL_TWO_INNER_TABLES)
					{
						if (outer_multiplicity_number < current_row_of_data.number_of_multiplicities - 1)
						{
							do_insert = true;
						}
					}
					else
					{
						do_insert = true;
					}

					if (do_insert)
					{
						if (saved_ints_previous__map_from_inner_primary_key_group__to__count.find(inner_multiplicity_int_vector) != saved_ints_previous__map_from_inner_primary_key_group__to__count.cend())
						{
							int current_count = saved_ints_previous__map_from_inner_primary_key_group__to__count[inner_multiplicity_int_vector];
							++current_count;
							saved_ints_previous__map_from_inner_primary_key_group__to__count[inner_multiplicity_int_vector] = current_count;
						}
						else
						{
							saved_ints_previous__map_from_inner_primary_key_group__to__count[inner_multiplicity_int_vector] = 1;
						}
					}

					inner_multiplicity_int_vector.clear();
				}

				if (!inner_multiplicity_float_vector.empty())
				{
					bool do_insert = false;

					if (match_condition == PRIMARY_KEY_MATCH_CONDITION::MATCH_ON_ALL_BUT_FINAL_INNER_TABLE)
					{
						if (outer_multiplicity_number < current_row_of_data.number_of_multiplicities)
						{
							do_insert = true;
						}
					}
					else if (match_condition == PRIMARY_KEY_MATCH_CONDITION::MATCH_ON_ALL_BUT_FINAL_TWO_INNER_TABLES)
					{
						if (outer_multiplicity_number < current_row_of_data.number_of_multiplicities - 1)
						{
							do_insert = true;
						}
					}
					else
					{
						do_insert = true;
					}

					if (do_insert)
					{
						if (saved_floats_previous__map_from_inner_primary_key_group__to__count.find(inner_multiplicity_float_vector) !=
							saved_floats_previous__map_from_inner_primary_key_group__to__count.cend())
						{
							int current_count = saved_floats_previous__map_from_inner_primary_key_group__to__count[inner_multiplicity_float_vector];
							++current_count;
							saved_floats_previous__map_from_inner_primary_key_group__to__count[inner_multiplicity_float_vector] = current_count;
						}
						else
						{
							saved_floats_previous__map_from_inner_primary_key_group__to__count[inner_multiplicity_float_vector] = 1;
						}
					}

					inner_multiplicity_float_vector.clear();
				}

				if (!inner_multiplicity_string_vector.empty())
				{
					bool do_insert = false;

					if (match_condition == PRIMARY_KEY_MATCH_CONDITION::MATCH_ON_ALL_BUT_FINAL_INNER_TABLE)
					{
						if (outer_multiplicity_number < current_row_of_data.number_of_multiplicities)
						{
							do_insert = true;
						}
					}
					else if (match_condition == PRIMARY_KEY_MATCH_CONDITION::MATCH_ON_ALL_BUT_FINAL_TWO_INNER_TABLES)
					{
						if (outer_multiplicity_number < current_row_of_data.number_of_multiplicities - 1)
						{
							do_insert = true;
						}
					}
					else
					{
						do_insert = true;
					}

					if (do_insert)
					{
						if (saved_strings_previous__map_from_inner_primary_key_group__to__count.find(inner_multiplicity_string_vector) !=
							saved_strings_previous__map_from_inner_primary_key_group__to__count.cend())
						{
							int current_count = saved_strings_previous__map_from_inner_primary_key_group__to__count[inner_multiplicity_string_vector];
							++current_count;
							saved_strings_previous__map_from_inner_primary_key_group__to__count[inner_multiplicity_string_vector] = current_count;
						}
						else
						{
							saved_strings_previous__map_from_inner_primary_key_group__to__count[inner_multiplicity_string_vector] = 1;
						}
					}

					inner_multiplicity_string_vector.clear();
				}

				inner_multiplicity_index = 0;
				++outer_multiplicity_number;
				previous_row_current_inner_table_primary_key_group_is_null = false;
			}

		});

		if (number_null_primary_key_groups_in_current_row < number_null_primary_key_groups_in_previous_row)
		{
			use_newest_row_index = true;
		}

		int count_current = 0;
		bool use_current_strings = false;
		bool use_current_ints = false;
		bool use_current_floats = false;

		if (saved_strings_current__map_from_inner_primary_key_group__to__count.size() > 0)
		{
			++count_current;
			use_current_strings = true;
		}

		if (saved_ints_current__map_from_inner_primary_key_group__to__count.size() > 0)
		{
			++count_current;
			use_current_ints = true;
		}

		if (saved_floats_current__map_from_inner_primary_key_group__to__count.size() > 0)
		{
			++count_current;
			use_current_floats = true;
		}

		if (count_current > 1)
		{
			return false;
		}

		int count_previous = 0;
		bool use_previous_strings = false;
		bool use_previous_ints = false;
		bool use_previous_floats = false;

		if (saved_strings_previous__map_from_inner_primary_key_group__to__count.size() > 0)
		{
			++count_previous;
			use_previous_strings = true;
		}

		if (saved_ints_previous__map_from_inner_primary_key_group__to__count.size() > 0)
		{
			++count_previous;
			use_previous_ints = true;
		}

		if (saved_floats_previous__map_from_inner_primary_key_group__to__count.size() > 0)
		{
			++count_previous;
			use_previous_floats = true;
		}

		if (count_previous > 1)
		{
			return false;
		}

		if (use_previous_strings && !use_current_strings)
		{
			return false;
		}

		if (use_previous_ints && !use_current_ints)
		{
			return false;
		}

		if (use_previous_floats && !use_current_floats)
		{
			return false;
		}

		// sets are automatically sorted
		if (saved_strings_current__map_from_inner_primary_key_group__to__count.size() > 0)
		{

			if (saved_strings_previous__map_from_inner_primary_key_group__to__count.size() == 0)
			{
				// One of the rows is all NULL in the primary keys with multiplicity greater than 1
				// This counts as a match, as the NULLs will be overwritten when the rows are merged
				return true;
			}

			bool primary_key_group_in_current__does_not_exist_in_previous = false;
			bool an_existing_primary_key_group_in_current__has_a_bigger_count_than__an_existing_primary_key_group_in_previous = false;
			int current_excess_count = 0;
			std::for_each(saved_strings_current__map_from_inner_primary_key_group__to__count.cbegin(),
						  saved_strings_current__map_from_inner_primary_key_group__to__count.cend(), [&current_excess_count, &an_existing_primary_key_group_in_current__has_a_bigger_count_than__an_existing_primary_key_group_in_previous, &primary_key_group_in_current__does_not_exist_in_previous, &saved_strings_previous__map_from_inner_primary_key_group__to__count](
							  std::pair<std::vector<std::string> const, int> const & inner_table_primary_key_group_info)
			{
				if (saved_strings_previous__map_from_inner_primary_key_group__to__count.find(inner_table_primary_key_group_info.first) ==
					saved_strings_previous__map_from_inner_primary_key_group__to__count.cend())
				{
					// Not found!
					primary_key_group_in_current__does_not_exist_in_previous = true;
					current_excess_count += inner_table_primary_key_group_info.second;
				}
				else
				{
					// Found!
					if (inner_table_primary_key_group_info.second > saved_strings_previous__map_from_inner_primary_key_group__to__count[inner_table_primary_key_group_info.first])
					{
						an_existing_primary_key_group_in_current__has_a_bigger_count_than__an_existing_primary_key_group_in_previous = true;
						current_excess_count += (inner_table_primary_key_group_info.second - saved_strings_previous__map_from_inner_primary_key_group__to__count[inner_table_primary_key_group_info.first]);
					}
				}
			});

			bool primary_key_group_in_previous__does_not_exist_in_current = false;
			bool an_existing_primary_key_group_in_previous__has_a_bigger_count_than__an_existing_primary_key_group_in_current = false;
			int previous_excess_count = 0;
			std::for_each(saved_strings_previous__map_from_inner_primary_key_group__to__count.cbegin(),
						  saved_strings_previous__map_from_inner_primary_key_group__to__count.cend(), [&previous_excess_count, &an_existing_primary_key_group_in_previous__has_a_bigger_count_than__an_existing_primary_key_group_in_current, &primary_key_group_in_previous__does_not_exist_in_current, &saved_strings_current__map_from_inner_primary_key_group__to__count](
							  std::pair<std::vector<std::string> const, int> const & inner_table_primary_key_group_info)
			{
				if (saved_strings_current__map_from_inner_primary_key_group__to__count.find(inner_table_primary_key_group_info.first) ==
					saved_strings_current__map_from_inner_primary_key_group__to__count.cend())
				{
					// Not found!
					primary_key_group_in_previous__does_not_exist_in_current = true;
					previous_excess_count += inner_table_primary_key_group_info.second;
				}
				else
				{
					// Found!
					if (inner_table_primary_key_group_info.second > saved_strings_current__map_from_inner_primary_key_group__to__count[inner_table_primary_key_group_info.first])
					{
						an_existing_primary_key_group_in_previous__has_a_bigger_count_than__an_existing_primary_key_group_in_current = true;
						previous_excess_count += (inner_table_primary_key_group_info.second - saved_strings_current__map_from_inner_primary_key_group__to__count[inner_table_primary_key_group_info.first]);
					}
				}
			});

			if (primary_key_group_in_current__does_not_exist_in_previous && primary_key_group_in_previous__does_not_exist_in_current)
			{
				return false;
			}

			if (an_existing_primary_key_group_in_current__has_a_bigger_count_than__an_existing_primary_key_group_in_previous
				&& an_existing_primary_key_group_in_previous__has_a_bigger_count_than__an_existing_primary_key_group_in_current)
			{
				return false;
			}

			if (current_excess_count > number_null_primary_key_groups_in_previous_row)
			{
				return false;
			}

			if (previous_excess_count > number_null_primary_key_groups_in_current_row)
			{
				return false;
			}

		}

		else if (saved_ints_current__map_from_inner_primary_key_group__to__count.size() > 0)
		{

			if (saved_ints_previous__map_from_inner_primary_key_group__to__count.size() == 0)
			{
				// One of the rows is all NULL in the primary keys with multiplicity greater than 1
				// This counts as a match, as the NULLs will be overwritten when the rows are merged
				return true;
			}

			bool primary_key_group_in_current__does_not_exist_in_previous = false;
			bool an_existing_primary_key_group_in_current__has_a_bigger_count_than__an_existing_primary_key_group_in_previous = false;
			int current_excess_count = 0;
			std::for_each(saved_ints_current__map_from_inner_primary_key_group__to__count.cbegin(),
						  saved_ints_current__map_from_inner_primary_key_group__to__count.cend(), [&current_excess_count, &an_existing_primary_key_group_in_current__has_a_bigger_count_than__an_existing_primary_key_group_in_previous, &primary_key_group_in_current__does_not_exist_in_previous, &saved_ints_previous__map_from_inner_primary_key_group__to__count](
							  std::pair<std::vector<std::int64_t> const, int> const & inner_table_primary_key_group_info)
			{
				if (saved_ints_previous__map_from_inner_primary_key_group__to__count.find(inner_table_primary_key_group_info.first) ==
					saved_ints_previous__map_from_inner_primary_key_group__to__count.cend())
				{
					// Not found!
					primary_key_group_in_current__does_not_exist_in_previous = true;
					current_excess_count += inner_table_primary_key_group_info.second;
				}
				else
				{
					// Found!
					if (inner_table_primary_key_group_info.second > saved_ints_previous__map_from_inner_primary_key_group__to__count[inner_table_primary_key_group_info.first])
					{
						an_existing_primary_key_group_in_current__has_a_bigger_count_than__an_existing_primary_key_group_in_previous = true;
						current_excess_count += (inner_table_primary_key_group_info.second - saved_ints_previous__map_from_inner_primary_key_group__to__count[inner_table_primary_key_group_info.first]);
					}
				}
			});

			bool primary_key_group_in_previous__does_not_exist_in_current = false;
			bool an_existing_primary_key_group_in_previous__has_a_bigger_count_than__an_existing_primary_key_group_in_current = false;
			int previous_excess_count = 0;
			std::for_each(saved_ints_previous__map_from_inner_primary_key_group__to__count.cbegin(),
						  saved_ints_previous__map_from_inner_primary_key_group__to__count.cend(), [&previous_excess_count, &an_existing_primary_key_group_in_previous__has_a_bigger_count_than__an_existing_primary_key_group_in_current, &primary_key_group_in_previous__does_not_exist_in_current, &saved_ints_current__map_from_inner_primary_key_group__to__count](
						  std::pair<std::vector<std::int64_t> const, int> const & inner_table_primary_key_group_info)
			{
				if (saved_ints_current__map_from_inner_primary_key_group__to__count.find(inner_table_primary_key_group_info.first) ==
					saved_ints_current__map_from_inner_primary_key_group__to__count.cend())
				{
					// Not found!
					primary_key_group_in_previous__does_not_exist_in_current = true;
					previous_excess_count += inner_table_primary_key_group_info.second;
				}
				else
				{
					// Found!
					if (inner_table_primary_key_group_info.second > saved_ints_current__map_from_inner_primary_key_group__to__count[inner_table_primary_key_group_info.first])
					{
						an_existing_primary_key_group_in_previous__has_a_bigger_count_than__an_existing_primary_key_group_in_current = true;
						previous_excess_count += (inner_table_primary_key_group_info.second - saved_ints_current__map_from_inner_primary_key_group__to__count[inner_table_primary_key_group_info.first]);
					}
				}
			});

			if (primary_key_group_in_current__does_not_exist_in_previous && primary_key_group_in_previous__does_not_exist_in_current)
			{
				return false;
			}

			if (an_existing_primary_key_group_in_current__has_a_bigger_count_than__an_existing_primary_key_group_in_previous
				&& an_existing_primary_key_group_in_previous__has_a_bigger_count_than__an_existing_primary_key_group_in_current)
			{
				return false;
			}

			if (current_excess_count > number_null_primary_key_groups_in_previous_row)
			{
				return false;
			}

			if (previous_excess_count > number_null_primary_key_groups_in_current_row)
			{
				return false;
			}

		}

		else if (saved_floats_current__map_from_inner_primary_key_group__to__count.size() > 0)
		{

			if (saved_floats_previous__map_from_inner_primary_key_group__to__count.size() == 0)
			{
				// One of the rows is all NULL in the primary keys with multiplicity greater than 1
				// This counts as a match, as the NULLs will be overwritten when the rows are merged
				return true;
			}

			bool primary_key_group_in_current__does_not_exist_in_previous = false;
			bool an_existing_primary_key_group_in_current__has_a_bigger_count_than__an_existing_primary_key_group_in_previous = false;
			int current_excess_count = 0;
			std::for_each(saved_floats_current__map_from_inner_primary_key_group__to__count.cbegin(),
						  saved_floats_current__map_from_inner_primary_key_group__to__count.cend(), [&current_excess_count, &an_existing_primary_key_group_in_current__has_a_bigger_count_than__an_existing_primary_key_group_in_previous, &primary_key_group_in_current__does_not_exist_in_previous, &saved_floats_previous__map_from_inner_primary_key_group__to__count](
						  std::pair<std::vector<long double> const, int> const & inner_table_primary_key_group_info)
			{
				if (saved_floats_previous__map_from_inner_primary_key_group__to__count.find(inner_table_primary_key_group_info.first) ==
					saved_floats_previous__map_from_inner_primary_key_group__to__count.cend())
				{
					// Not found!
					primary_key_group_in_current__does_not_exist_in_previous = true;
					current_excess_count += inner_table_primary_key_group_info.second;
				}
				else
				{
					// Found!
					if (inner_table_primary_key_group_info.second > saved_floats_previous__map_from_inner_primary_key_group__to__count[inner_table_primary_key_group_info.first])
					{
						an_existing_primary_key_group_in_current__has_a_bigger_count_than__an_existing_primary_key_group_in_previous = true;
						current_excess_count += (inner_table_primary_key_group_info.second - saved_floats_previous__map_from_inner_primary_key_group__to__count[inner_table_primary_key_group_info.first]);
					}
				}
			});

			bool primary_key_group_in_previous__does_not_exist_in_current = false;
			bool an_existing_primary_key_group_in_previous__has_a_bigger_count_than__an_existing_primary_key_group_in_current = false;
			int previous_excess_count = 0;
			std::for_each(saved_floats_previous__map_from_inner_primary_key_group__to__count.cbegin(),
						  saved_floats_previous__map_from_inner_primary_key_group__to__count.cend(), [&previous_excess_count, &an_existing_primary_key_group_in_previous__has_a_bigger_count_than__an_existing_primary_key_group_in_current, &primary_key_group_in_previous__does_not_exist_in_current, &saved_floats_current__map_from_inner_primary_key_group__to__count](
						  std::pair<std::vector<long double> const, int> const & inner_table_primary_key_group_info)
			{
				if (saved_floats_current__map_from_inner_primary_key_group__to__count.find(inner_table_primary_key_group_info.first) ==
					saved_floats_current__map_from_inner_primary_key_group__to__count.cend())
				{
					// Not found!
					primary_key_group_in_previous__does_not_exist_in_current = true;
					previous_excess_count += inner_table_primary_key_group_info.second;
				}
				else
				{
					// Found!
					if (inner_table_primary_key_group_info.second > saved_floats_current__map_from_inner_primary_key_group__to__count[inner_table_primary_key_group_info.first])
					{
						an_existing_primary_key_group_in_previous__has_a_bigger_count_than__an_existing_primary_key_group_in_current = true;
						previous_excess_count += (inner_table_primary_key_group_info.second - saved_floats_current__map_from_inner_primary_key_group__to__count[inner_table_primary_key_group_info.first]);
					}
				}
			});

			if (primary_key_group_in_current__does_not_exist_in_previous && primary_key_group_in_previous__does_not_exist_in_current)
			{
				return false;
			}

			if (an_existing_primary_key_group_in_current__has_a_bigger_count_than__an_existing_primary_key_group_in_previous
				&& an_existing_primary_key_group_in_previous__has_a_bigger_count_than__an_existing_primary_key_group_in_current)
			{
				return false;
			}

			if (current_excess_count > number_null_primary_key_groups_in_previous_row)
			{
				return false;
			}

			if (previous_excess_count > number_null_primary_key_groups_in_current_row)
			{
				return false;
			}

		}

		else
		{
			// all NULLs - this is a match
			// ... no-op
		}

	}

	return true;

}

void OutputModel::OutputGenerator::WriteRowsToFinalTable(std::deque<SavedRowData> & outgoing_rows_of_data, std::string const & datetime_start_col_name,
		std::string const & datetime_end_col_name, std::shared_ptr<bool> & statement_is_prepared, sqlite3_stmt *& the_prepared_stmt, std::vector<SQLExecutor> & sql_strings, sqlite3 * db,
		std::string & result_columns_view_name, ColumnsInTempView const & preliminary_sorted_top_level_variable_group_result_columns, std::int64_t & current_rows_added,
		std::int64_t & current_rows_added_since_execution, std::string & sql_add_xr_row, bool & first_row_added, std::vector<std::string> & bound_parameter_strings,
		std::vector<std::int64_t> & bound_parameter_ints, std::vector<long double> & bound_parameter_floats, std::vector<SQLExecutor::WHICH_BINDING> & bound_parameter_which_binding_to_use,
		XR_TABLE_CATEGORY const xr_table_category, bool const no_new_column_names, std::string const datetimestart_text_colname, std::string const datetimeend_text_colname)
{

	std::for_each(outgoing_rows_of_data.cbegin(),
				  outgoing_rows_of_data.cend(), [this, &datetimestart_text_colname, &datetimeend_text_colname, &no_new_column_names, &xr_table_category, &statement_is_prepared, &datetime_start_col_name, &datetime_end_col_name, &the_prepared_stmt, &sql_strings, &db, &result_columns_view_name, &preliminary_sorted_top_level_variable_group_result_columns, &current_rows_added, &current_rows_added_since_execution, &sql_add_xr_row, &first_row_added, &bound_parameter_strings, &bound_parameter_ints, &bound_parameter_floats, &bound_parameter_which_binding_to_use](
					  SavedRowData const & row_of_data)
	{

		if (failed || CheckCancelled())
		{
			return;
		}

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

			sql_add_xr_row += "INSERT OR FAIL INTO \"";
			sql_add_xr_row += result_columns_view_name;
			sql_add_xr_row += "\" (";

			bool first_column_name = true;
			std::for_each(preliminary_sorted_top_level_variable_group_result_columns.columns_in_view.cbegin(),
						  preliminary_sorted_top_level_variable_group_result_columns.columns_in_view.cend(), [&first_column_name, &sql_add_xr_row, &bound_parameter_strings, &bound_parameter_ints, &bound_parameter_which_binding_to_use](
							  ColumnsInTempView::ColumnInTempView const & column_in_view)
			{

				if (!first_column_name)
				{
					sql_add_xr_row += ", ";
				}

				first_column_name = false;

				sql_add_xr_row += "`";
				sql_add_xr_row += column_in_view.column_name_in_temporary_table;
				sql_add_xr_row += "`";

			});

			// special case: final output.  Two text column names form the final pair of columns except for the timestamp columns
			if (xr_table_category == OutputModel::OutputGenerator::KAD_RESULTS)
			{
				if (!datetimestart_text_colname.empty())
				{
					if (!first_column_name)
					{
						sql_add_xr_row += ", ";
					}

					first_column_name = false;
					sql_add_xr_row += "`";
					sql_add_xr_row += datetimestart_text_colname;
					sql_add_xr_row += "`";
					sql_add_xr_row += ", ";
					sql_add_xr_row += "`";
					sql_add_xr_row += datetimeend_text_colname;
					sql_add_xr_row += "`";
				}
			}

			if (!no_new_column_names)
			{
				// The two new "merged" time range columns
				if (!first_column_name)
				{
					sql_add_xr_row += ", ";
				}

				first_column_name = false;
				sql_add_xr_row += "`";
				sql_add_xr_row += datetime_start_col_name;
				sql_add_xr_row += "`";
				sql_add_xr_row += ", ";
				sql_add_xr_row += "`";
				sql_add_xr_row += datetime_end_col_name;
				sql_add_xr_row += "`";
			}

			sql_add_xr_row += ") VALUES (";

			int index = 1;
			char cindex[256];

			bool first_column_value = true;
			std::for_each(preliminary_sorted_top_level_variable_group_result_columns.columns_in_view.cbegin(),
						  preliminary_sorted_top_level_variable_group_result_columns.columns_in_view.cend(), [&first_column_value, &index, &cindex, &sql_add_xr_row, &bound_parameter_strings, &bound_parameter_ints, &bound_parameter_which_binding_to_use](
							  ColumnsInTempView::ColumnInTempView const & column_in_view)
			{

				if (!first_column_value)
				{
					sql_add_xr_row += ", ";
				}

				first_column_value = false;

				sql_add_xr_row += "?";
				sql_add_xr_row += std::to_string(index);
				++index;

			});

			if (xr_table_category == OutputModel::OutputGenerator::KAD_RESULTS)
			{
				if (!datetimestart_text_colname.empty())
				{
					// the two text column names
					if (!first_column_value)
					{
						sql_add_xr_row += ", ";
					}

					first_column_value = false;
					sql_add_xr_row += "?";
					sql_add_xr_row += std::to_string(index);
					++index;
				}

				sql_add_xr_row += ", ";
				sql_add_xr_row += "?";
				sql_add_xr_row += std::to_string(index);
				++index;
			}

			if (!no_new_column_names)
			{
				// The two new "merged" time range columns
				if (!first_column_value)
				{
					sql_add_xr_row += ", ";
				}

				first_column_value = false;
				sql_add_xr_row += "?";
				sql_add_xr_row += std::to_string(index);
				++index;
				sql_add_xr_row += ", ";
				sql_add_xr_row += "?";
				sql_add_xr_row += std::to_string(index);
				++index;
			}

			sql_add_xr_row += ")";

			first_row_added = false;

		}

		if (failed || CheckCancelled())
		{
			return;
		}

		// Set the list of bound parameters, regardless of whether or not the SQL string was created
		int int_index = 0;
		int float_index = 0;
		int string_index = 0;
		char cindex[256];
		bool first_column_value = true;
		std::int64_t data_int64 = 0;
		long double data_float = 0;
		std::string data_string;
		long double data_long = 0.0;
		bound_parameter_strings.clear();
		bound_parameter_ints.clear();
		bound_parameter_floats.clear();
		bound_parameter_which_binding_to_use.clear();
		std::for_each(row_of_data.current_parameter_which_binding_to_use.cbegin(),
					  row_of_data.current_parameter_which_binding_to_use.cend(), [this, &row_of_data, &data_int64, &data_float, &data_string, &data_long, &first_column_value, &int_index, &float_index, &string_index, &cindex, &sql_add_xr_row, &bound_parameter_strings, &bound_parameter_ints, &bound_parameter_floats, &bound_parameter_which_binding_to_use](
						  SQLExecutor::WHICH_BINDING const & the_binding)
		{

			if (failed || CheckCancelled())
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

				case SQLExecutor::FLOAT:
					{
						data_float = row_of_data.current_parameter_floats[float_index];
						++float_index;
						bound_parameter_floats.push_back(data_float);
						bound_parameter_which_binding_to_use.push_back(SQLExecutor::FLOAT);
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
						boost::format msg("Unknown data type binding in column when writing row to database.");
						SetFailureMessage(msg.str());
						failed = true;
						return; // from lambda
					}

			}

		});


		// The two new "merged" time range columns

		std::int64_t datetime_start = row_of_data.datetime_start;
		std::int64_t datetime_end = row_of_data.datetime_end;

		bool skip_check = false;

		if (row_of_data.datetime_start == 0 && row_of_data.datetime_end == 0)
		{
			skip_check = true;
		}

		if (!skip_check)
		{
			if (row_of_data.datetime_end > timerange_end)
			{
				datetime_end = timerange_end;
			}

			if (row_of_data.datetime_start < timerange_start)
			{
				datetime_start = timerange_start;
			}
		}

		switch (xr_table_category)
		{
			case OutputModel::OutputGenerator::PRIMARY_VARIABLE_GROUP:
				{
					bound_parameter_ints.push_back(datetime_start);
					bound_parameter_which_binding_to_use.push_back(SQLExecutor::INT64);
					bound_parameter_ints.push_back(datetime_end);
					bound_parameter_which_binding_to_use.push_back(SQLExecutor::INT64);
				}
				break;

			case OutputModel::OutputGenerator::CHILD_VARIABLE_GROUP:
				{
					bound_parameter_ints.push_back(datetime_start);
					bound_parameter_which_binding_to_use.push_back(SQLExecutor::INT64);
					bound_parameter_ints.push_back(datetime_end);
					bound_parameter_which_binding_to_use.push_back(SQLExecutor::INT64);
				}
				break;

			case OutputModel::OutputGenerator::KAD_RESULTS:
				{
					if (!datetimestart_text_colname.empty())
					{
						// Final results: The columns with text

						boost::posix_time::ptime time_t_epoch__1970(boost::gregorian::date(1970, 1, 1));

						boost::posix_time::ptime time_start_database = time_t_epoch__1970 + boost::posix_time::milliseconds(datetime_start);
						boost::posix_time::ptime time_end_database = time_t_epoch__1970 + boost::posix_time::milliseconds(datetime_end);

						std::string time_start_formatted = boost::posix_time::to_simple_string(time_start_database);
						std::string time_end_formatted = boost::posix_time::to_simple_string(time_end_database);

						bound_parameter_strings.push_back(time_start_formatted);
						bound_parameter_which_binding_to_use.push_back(SQLExecutor::STRING);
						bound_parameter_strings.push_back(time_end_formatted);
						bound_parameter_which_binding_to_use.push_back(SQLExecutor::STRING);
					}

					bound_parameter_ints.push_back(datetime_start);
					bound_parameter_which_binding_to_use.push_back(SQLExecutor::INT64);
					bound_parameter_ints.push_back(datetime_end);
					bound_parameter_which_binding_to_use.push_back(SQLExecutor::INT64);
				}
				break;

			case OutputModel::OutputGenerator::FINAL_MERGE_OF_PRIMARY_VARIABLE_GROUP:
				{
					bound_parameter_ints.push_back(datetime_start);
					bound_parameter_which_binding_to_use.push_back(SQLExecutor::INT64);
					bound_parameter_ints.push_back(datetime_end);
					bound_parameter_which_binding_to_use.push_back(SQLExecutor::INT64);
				}
				break;
		}

		SQLExecutor::Execute(false, SQLExecutor::DOES_NOT_RETURN_ROWS, this, db, sql_add_xr_row, bound_parameter_strings, bound_parameter_ints, bound_parameter_floats,
							 bound_parameter_which_binding_to_use, statement_is_prepared, the_prepared_stmt, true);

		if (failed)
		{
			SetFailureMessage(sql_error);
			return;
		}

		if (CheckCancelled())
		{
			return;
		}

		the_prepared_stmt = SQLExecutor::stmt_insert;
		++current_rows_added;
		++current_rows_added_since_execution;

	});

}

OutputModel::OutputGenerator::SqlAndColumnSet OutputModel::OutputGenerator::CreateSortedTable(ColumnsInTempView const & final_xr_or_xrmfxr_columns, int const primary_group_number,
		int const current_multiplicity, XR_TABLE_CATEGORY const xr_table_category)
{

	SqlAndColumnSet result = std::make_pair(std::vector<SQLExecutor>(), ColumnsInTempView());
	std::vector<SQLExecutor> & sql_strings = result.first;
	ColumnsInTempView & result_columns = result.second;

	result_columns = final_xr_or_xrmfxr_columns;
	result_columns.most_recent_sql_statement_executed__index = -1;

	std::string view_name;

	switch (xr_table_category)
	{
		case OutputModel::OutputGenerator::PRIMARY_VARIABLE_GROUP:
			{
				view_name += "NGTEMP_S";
			}
			break;

		case OutputModel::OutputGenerator::CHILD_VARIABLE_GROUP:
			{
				view_name += "NGTEMP_MFXRMF";
			}
			break;

		case OutputModel::OutputGenerator::FINAL_MERGE_OF_PRIMARY_VARIABLE_GROUP:
			{
				view_name += "NGTEMP_FMB";
			}
			break;

		case OutputModel::OutputGenerator::KAD_RESULTS:
			{
				view_name += "NGTEMP_KK";
			}
			break;
	}

	view_name += std::to_string(primary_group_number);

	if (current_multiplicity >= 0)
	{
		view_name += "_";
		view_name += std::to_string(current_multiplicity);
	}

	result_columns.view_name_no_uuid = view_name;
	view_name += "_";
	view_name += newUUID(true);
	result_columns.view_name = view_name;
	result_columns.view_number = 1;
	result_columns.has_no_datetime_columns = false;

	std::string sql_create_final_primary_group_table;
	sql_create_final_primary_group_table += "CREATE TABLE \"";
	sql_create_final_primary_group_table += result_columns.view_name;
	sql_create_final_primary_group_table += "\" AS SELECT * FROM \"";
	sql_create_final_primary_group_table += final_xr_or_xrmfxr_columns.view_name;
	sql_create_final_primary_group_table += "\"";

	WidgetInstanceIdentifier first_variable_group;

	first_variable_group = result_columns.columns_in_view.front().variable_group_associated_with_current_inner_table;

	bool first = true;
	SortOrderByMultiplicityOnes(result_columns, xr_table_category, first_variable_group, sql_create_final_primary_group_table, first);
	SortOrderByMultiplicityGreaterThanOnes(result_columns, xr_table_category, first_variable_group, sql_create_final_primary_group_table, first);

	// Finally, order by the time range columns
	sql_create_final_primary_group_table += ", ";
	sql_create_final_primary_group_table +=
		final_xr_or_xrmfxr_columns.columns_in_view[final_xr_or_xrmfxr_columns.columns_in_view.size() - 2].column_name_in_temporary_table; // final merged datetime start column
	sql_create_final_primary_group_table += ", ";
	sql_create_final_primary_group_table +=
		final_xr_or_xrmfxr_columns.columns_in_view[final_xr_or_xrmfxr_columns.columns_in_view.size() - 1].column_name_in_temporary_table; // final merged datetime end column

	sql_strings.push_back(SQLExecutor(this, db, sql_create_final_primary_group_table));

	if (failed)
	{
		SetFailureMessage(sql_error);
		return result;
	}

	if (CheckCancelled())
	{
		return result;
	}

	return result;

}

bool OutputModel::OutputGenerator::StepData()
{

	if (stmt_result == nullptr)
	{
		return false;
	}

	if (failed || CheckCancelled())
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

void OutputModel::OutputGenerator::ObtainData(ColumnsInTempView const & column_set, bool const obtain_rowid)
{

	CloseObtainData();

	std::string sql;

	if (obtain_rowid)
	{
		sql += "SELECT *, rowid FROM \"";
	}
	else
	{
		sql += "SELECT * FROM \"";
	}

	sql += column_set.view_name;
	sql += "\"";

	if (debug_sql_file.is_open())
	{
		debug_sql_file << sql << std::endl << std::endl;
	}

	sqlite3_prepare_v2(db, sql.c_str(), static_cast<int>(sql.size()) + 1, &stmt_result, NULL);

	if (stmt_result == NULL)
	{
		sql_error = sqlite3_errmsg(db);
		boost::format msg("SQLite database error when preparing SELECT * SQL statement for table %1%: %2% (The SQL query is \"%3%\")");
		msg % column_set.view_name % sql_error % sql;
		SetFailureMessage(msg.str());
		failed = true;
		return;
	}

	++SQLExecutor::number_statement_prepares;

}

void OutputModel::OutputGenerator::CloseObtainData()
{
	if (stmt_result)
	{
		sqlite3_finalize(stmt_result);
		++SQLExecutor::number_statement_finalizes;
		stmt_result = nullptr;
	}
}

std::int64_t OutputModel::OutputGenerator::ObtainCount(ColumnsInTempView const & column_set)
{

	BOOST_SCOPE_EXIT(this_)
	{
		this_->CloseObtainData();
	} BOOST_SCOPE_EXIT_END

	std::string sql;
	sql += "SELECT COUNT (*) FROM \"";
	sql += column_set.view_name;
	sql += "\"";

	sqlite3_prepare_v2(db, sql.c_str(), static_cast<int>(sql.size()) + 1, &stmt_result, NULL);

	if (stmt_result == NULL)
	{
		sql_error = sqlite3_errmsg(db);
		boost::format msg("SQLite database error when preparing SELECT * SQL statement for table %1%: %2% (The SQL query is \"%3%\")");
		msg % column_set.view_name % sql_error % sql;
		SetFailureMessage(msg.str());
		failed = true;
		return 0;
	}

	++SQLExecutor::number_statement_prepares;

	int step_result = 0;

	if ((step_result = sqlite3_step(stmt_result)) != SQLITE_ROW)
	{

		if (step_result == SQLITE_DONE)
		{
			return 0;
		}

		sql_error = sqlite3_errmsg(db);
		boost::format msg("Unexpected result (row not returned) when attempting to step through result set of SQL query \"%1%\": %2%");
		msg % sql % sql_error;
		SetFailureMessage(msg.str());
		failed = true;
		return 0;
	}

	std::int64_t data_int64 = sqlite3_column_int64(stmt_result, 0);

	if (stmt_result)
	{
		sqlite3_finalize(stmt_result);
		++SQLExecutor::number_statement_finalizes;
		stmt_result = nullptr;
	}

	return data_int64;

}

void OutputModel::OutputGenerator::BeginNewTransaction()
{
	++number_transaction_begins;
	executor.BeginTransaction();
}

void OutputModel::OutputGenerator::EndTransaction()
{
	++number_transaction_ends;
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
	std::for_each(sql_commands.begin() + (sql_and_column_set.second.most_recent_sql_statement_executed__index + 1),
				  sql_commands.end(), [this, &number_executed, &sql_and_column_set](SQLExecutor & sql_executor)
	{

		if (failed || CheckCancelled())
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

OutputModel::OutputGenerator::SQLExecutor::SQLExecutor(OutputModel::OutputGenerator * generator_, sqlite3 * db_)
	: statement_type(DOES_NOT_RETURN_ROWS)
	, generator(generator_)
	, db(db_)
	, stmt(nullptr)
	, failed(false)
	, statement_is_owned(true)
	, statement_is_prepared(std::make_shared<bool>(false))
	, statement_is_shared(false)
{

}

OutputModel::OutputGenerator::SQLExecutor::SQLExecutor(OutputModel::OutputGenerator * generator_, sqlite3 * db_, std::string const & sql_)
	: sql(sql_)
	, generator(generator_)
	, statement_type(DOES_NOT_RETURN_ROWS)
	, db(db_)
	, stmt(nullptr)
	, failed(false)
	, statement_is_owned(true)
	, statement_is_prepared(std::make_shared<bool>(false))
	, statement_is_shared(false)
{

}

sqlite3_stmt * OutputModel::OutputGenerator::SQLExecutor::stmt_insert = nullptr;

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
	this->bound_parameter_floats = rhs.bound_parameter_floats;
	this->bound_parameter_strings = rhs.bound_parameter_strings;
	this->bound_parameter_which_binding_to_use = rhs.bound_parameter_which_binding_to_use;
	this->db = rhs.db;
	this->failed = rhs.failed;
	this->sql = rhs.sql;
	this->sql_error = sql_error;
	this->statement_is_prepared = rhs.statement_is_prepared;
	this->statement_type = rhs.statement_type;
	this->stmt = rhs.stmt;
	this->statement_is_shared = rhs.statement_is_shared;
	this->generator = rhs.generator;
}

void OutputModel::OutputGenerator::SQLExecutor::CopyOwned(SQLExecutor & rhs)
{
	// The following line DOES use the default here
	this->statement_is_owned = rhs.statement_is_owned;
	rhs.statement_is_owned = false;

	this->bound_parameter_ints = rhs.bound_parameter_ints;
	this->bound_parameter_floats = rhs.bound_parameter_floats;
	this->bound_parameter_strings = rhs.bound_parameter_strings;
	this->bound_parameter_which_binding_to_use = rhs.bound_parameter_which_binding_to_use;
	this->db = rhs.db;
	this->failed = rhs.failed;
	this->sql = rhs.sql;
	this->sql_error = sql_error;
	this->statement_is_prepared = rhs.statement_is_prepared;
	this->statement_type = rhs.statement_type;
	this->stmt = rhs.stmt;
	this->statement_is_shared = rhs.statement_is_shared;
	this->generator = rhs.generator;
}

void OutputModel::OutputGenerator::SQLExecutor::Empty(bool const empty_sql)
{

	if (statement_is_owned && statement_is_prepared && *statement_is_prepared && stmt)
	{
		sqlite3_finalize(stmt);
		++number_statement_finalizes;
		*statement_is_prepared = false;
		stmt = nullptr;
	}

	if (empty_sql)
	{
		sql.clear();
		bound_parameter_strings.clear();
		bound_parameter_ints.clear();
		bound_parameter_floats.clear();
		bound_parameter_which_binding_to_use.clear();
	}

	if (statement_is_owned)
	{
		if (statement_is_prepared != nullptr)
		{
			*statement_is_prepared = false;
		}
	}

	failed = false;

}

void OutputModel::OutputGenerator::SQLExecutor::Execute()
{

	if (failed || CheckCancelled())
	{
		return;
	}

	switch (statement_type)
	{

		case DOES_NOT_RETURN_ROWS:
			{

				if (statement_is_owned && !(*statement_is_prepared))
				{
					if (generator && generator->debug_sql_file.is_open() && !boost::iequals(sql.substr(0, strlen("INSERT")), std::string("INSERT")))
					{
						generator->debug_sql_file << sql << std::endl << std::endl;
					}

					sqlite3_prepare_v2(db, sql.c_str(), static_cast<int>(sql.size()) + 1, &stmt, NULL);

					if (stmt == NULL)
					{
						sql_error = sqlite3_errmsg(db);
						boost::format msg("Unable to prepare SQL query \"%1%\": %2%");
						msg % sql % sql_error;
						error_message = msg.str();
						failed = true;
						return;
					}

					++number_statement_prepares;
					*statement_is_prepared = true;
				}

			}
			break;

		case RETURNS_ROWS:
			{

				if (statement_is_owned && !(*statement_is_prepared))
				{
					if (generator && generator->debug_sql_file.is_open() && !boost::iequals(sql.substr(0, strlen("INSERT")), std::string("INSERT")))
					{
						generator->debug_sql_file << sql << std::endl << std::endl;
					}

					sqlite3_prepare_v2(db, sql.c_str(), static_cast<int>(sql.size()) + 1, &stmt, NULL);

					if (stmt == NULL)
					{
						sql_error = sqlite3_errmsg(db);
						boost::format msg("Unable to prepare SQL query \"%1%\": %2%");
						msg % sql % sql_error;
						error_message = msg.str();
						failed = true;
						return;
					}

					++number_statement_prepares;
					*statement_is_prepared = true;
				}

			}
			break;

		default:
			break;

	}

	if (stmt == nullptr)
	{
		return;
	}

	DoExecute(db, generator, stmt, statement_type, bound_parameter_strings, bound_parameter_ints, bound_parameter_floats, bound_parameter_which_binding_to_use, statement_is_owned,
			  statement_is_prepared, statement_is_shared);

}

void OutputModel::OutputGenerator::SQLExecutor::Execute(bool statement_is_owned_, OutputModel::OutputGenerator::SQLExecutor::STATEMENT_TYPE statement_type_,
		OutputModel::OutputGenerator * generator_, sqlite3 * db_, std::string const & sql_, std::vector<std::string> const & bound_parameter_strings_,
		std::vector<std::int64_t> const & bound_parameter_ints_, std::vector<long double> const & bound_parameter_floats_,
		std::vector<WHICH_BINDING> & bound_parameter_which_binding_to_use_, std::shared_ptr<bool> & statement_is_prepared_, sqlite3_stmt *& stmt_to_use,
		bool const prepare_statement_if_null)
{
	if (prepare_statement_if_null && stmt_to_use == nullptr)
	{
		if (!(*statement_is_prepared_))
		{
			sqlite3_prepare_v2(db_, sql_.c_str(), static_cast<int>(sql_.size()) + 1, &stmt_to_use, NULL);

			if (stmt_to_use == NULL)
			{
				//sql_error = sqlite3_errmsg(db);
				std::string sql_error = sqlite3_errmsg(db_);
				boost::format msg("Unable to prepare SQL query \"%1%\": %2%");
				msg % sql_ % sql_error;
				//error_message = msg.str();
				std::string error_message = msg.str();
				generator_->failed = true;
				generator_->sql_error = error_message;
				//failed = true;
				return;
			}

			++number_statement_prepares;
			//statement_is_owned = true;
			*statement_is_prepared_ = true;
		}
	}

	DoExecute(db_, generator_, stmt_to_use, statement_type_, bound_parameter_strings_, bound_parameter_ints_, bound_parameter_floats_, bound_parameter_which_binding_to_use_,
			  statement_is_owned_, statement_is_prepared_, true);
}

void OutputModel::OutputGenerator::SQLExecutor::DoExecute(sqlite3 * db, OutputModel::OutputGenerator * generator, sqlite3_stmt *& stmt, SQLExecutor::STATEMENT_TYPE statement_type,
		std::vector<std::string> const & bound_parameter_strings, std::vector<std::int64_t> const & bound_parameter_ints, std::vector<long double> const & bound_parameter_floats,
		std::vector<WHICH_BINDING> & bound_parameter_which_binding_to_use, bool statement_is_owned, std::shared_ptr<bool> & statement_is_prepared, bool statement_is_shared)
{

	if (stmt == nullptr)
	{
		return;
	}

	if (bound_parameter_which_binding_to_use.size() > 0)
	{

		if (*statement_is_prepared)
		{
			sqlite3_reset(stmt); // OK even if the prepared statement has not been executed yet
		}

		int current_string_index = 0;
		int current_int64_index = 0;
		int current_float_index = 0;
		int current_index = 1;
		std::for_each(bound_parameter_which_binding_to_use.cbegin(),
					  bound_parameter_which_binding_to_use.cend(), [&stmt, &bound_parameter_strings, &bound_parameter_ints, &bound_parameter_floats, &bound_parameter_which_binding_to_use, &current_string_index, &current_int64_index, &current_float_index, &current_index](
						  WHICH_BINDING const & which_binding)
		{
			switch (which_binding)
			{

				case STRING:
					{
						std::string const & the_string = bound_parameter_strings[current_string_index];
						sqlite3_bind_text(stmt, current_index, the_string.c_str(), static_cast<int>(the_string.size()), SQLITE_STATIC);
						++current_string_index;
						++current_index;
					}
					break;

				case INT64:
					{
						std::int64_t the_int64 = bound_parameter_ints[current_int64_index];
						sqlite3_bind_int64(stmt, current_index, the_int64);
						++current_int64_index;
						++current_index;
					}
					break;

				case FLOAT:
					{
						long double the_float = bound_parameter_floats[current_float_index];
						sqlite3_bind_double(stmt, current_index, the_float);
						++current_float_index;
						++current_index;
					}
					break;

				case NULL_BINDING:
					{
						sqlite3_bind_null(stmt, current_index);
						++current_index;
					}
					break;

				default:
					break;

			}
		});

	}

	switch (statement_type)
	{

		case DOES_NOT_RETURN_ROWS:
			{

				int step_result = 0;

				if ((step_result = sqlite3_step(stmt)) != SQLITE_DONE)
				{
					std::string sql_error = sqlite3_errmsg(db);
					boost::format msg("Unexpected result when attempting to execute SQL query \"%1%\": %2%");
					//msg % sql % sql_error;
					generator->failed = true;
					generator->sql_error = msg.str();
					//failed = true;
					return;
				}

				if (stmt && statement_is_owned && !statement_is_shared)
				{
					sqlite3_finalize(stmt);
					++number_statement_finalizes;
					*statement_is_prepared = false;
					stmt = nullptr;
				}

			}
			break;

		case RETURNS_ROWS:
			{

				// no-op

			}
			break;

		default:
			break;

	}

}

bool OutputModel::OutputGenerator::SQLExecutor::Step()
{

	if (stmt == nullptr)
	{
		return false;
	}

	if (!(*statement_is_prepared))
	{
		return false;
	}

	if (failed || CheckCancelled())
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

OutputModel::OutputGenerator::SqlAndColumnSet OutputModel::OutputGenerator::CreateTableOfSelectedVariablesFromRawData(ColumnsInTempView const & variable_group_raw_data_columns, int const group_number)
{

	SqlAndColumnSet result = std::make_pair(std::vector<SQLExecutor>(), ColumnsInTempView());
	std::vector<SQLExecutor> & sql_strings = result.first;
	ColumnsInTempView & result_columns = result.second;

	// Note: The "columns_in_view" of result_columns will be CLEARED
	// and REFILLED below!
	result_columns = variable_group_raw_data_columns;

	result_columns.view_number = 1; // which set of secondary keys is this table - from 1 to K where K is the multiplicity.  This is the first, so set to 1.  (Regardless of which top-level primary variable group this is (currently multiple top-level primary variable groups is not supported).)
	result_columns.has_no_datetime_columns = false; // Only the actual permanent data table in the database can have this be set to true.  Here, we are doing a SELECT of the permanent data for the first time into a temporary table.
	std::string view_name;
	switch (variable_group_raw_data_columns.schema_type)
	{
		case ColumnsInTempView::SCHEMA_TYPE__RAW__SELECTED_VARIABLES_PRIMARY:
			view_name = "NGTEMP__RAW__SELECTED_VARIABLES_PRIMARY";
			break;
		case ColumnsInTempView::SCHEMA_TYPE__RAW__SELECTED_VARIABLES_TOP_LEVEL_NOT_PRIMARY:
			view_name = "NGTEMP__RAW__SELECTED_VARIABLES_TOP_LEVEL_NOT_PRIMARY";
			break;
		case ColumnsInTempView::SCHEMA_TYPE__RAW__SELECTED_VARIABLES_CHILD:
			view_name = "NGTEMP__RAW__SELECTED_VARIABLES_CHILD";
			break;
	}
	view_name += "_";
	view_name += boost::lexical_cast<std::string>(group_number);
	result_columns.view_name_no_uuid = view_name;
	view_name += "_";
	view_name += newUUID(true);
	result_columns.view_name = view_name;

	WidgetInstanceIdentifiers const & variables_selected =
		(*the_map)[*variable_group_raw_data_columns.variable_groups[0].identifier_parent][variable_group_raw_data_columns.variable_groups[0]];

	result_columns.columns_in_view.clear();

	// Create the schema columns from the raw data table into the temporary table.
	// Start with the primary key columns.
	std::for_each(variable_group_raw_data_columns.columns_in_view.cbegin(),
		variable_group_raw_data_columns.columns_in_view.cend(), [&result_columns, &variables_selected](ColumnsInTempView::ColumnInTempView const & column_in_view)
	{
		if (column_in_view.column_type == ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__DATETIMESTART
			|| column_in_view.column_type == ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__DATETIMEEND)
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

	switch (variable_group_raw_data_columns.schema_type)
	{
	case ColumnsInTempView::SCHEMA_TYPE__RAW__SELECTED_VARIABLES_PRIMARY:
	case ColumnsInTempView::SCHEMA_TYPE__RAW__SELECTED_VARIABLES_TOP_LEVEL_NOT_PRIMARY:
		top_level_number_secondary_columns[group_number] = 0;
		break;
	case ColumnsInTempView::SCHEMA_TYPE__RAW__SELECTED_VARIABLES_CHILD:
		child_number_secondary_columns[group_number] = 0;
		break;
	}

	// Proceed to the secondary key columns.
	std::for_each(variable_group_raw_data_columns.columns_in_view.cbegin(),
		variable_group_raw_data_columns.columns_in_view.cend(), [&](ColumnsInTempView::ColumnInTempView const & column_in_view)
	{
		bool make_secondary_datetime_column = false;

		if (column_in_view.column_type == ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__DATETIMESTART
			|| column_in_view.column_type == ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__DATETIMEEND)
		{
			// No!  If the user selects these columns, they should appear as regular secondary key columns.  Change the column type in this case to "secondary".
			//return; // Enforce that datetime columns appear last.
			make_secondary_datetime_column = true;
		}

		if (column_in_view.column_type != ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__SECONDARY)
		{
			// No!  If the user selects these columns, they should appear as regular secondary key columns.  Change the column type in this case to "secondary".
			if (!make_secondary_datetime_column)
			{
				return; // We are populating secondary columns now, so exit if this isn't one
			}
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
			
			switch (variable_group_raw_data_columns.schema_type)
			{
			case ColumnsInTempView::SCHEMA_TYPE__RAW__SELECTED_VARIABLES_PRIMARY:
			case ColumnsInTempView::SCHEMA_TYPE__RAW__SELECTED_VARIABLES_TOP_LEVEL_NOT_PRIMARY:
				++top_level_number_secondary_columns[group_number];
				break;
			case ColumnsInTempView::SCHEMA_TYPE__RAW__SELECTED_VARIABLES_CHILD:
				++child_number_secondary_columns[group_number];
				break;
			}


			result_columns.columns_in_view.push_back(column_in_view);

			if (make_secondary_datetime_column)
			{
				result_columns.columns_in_view.back().column_type = ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__SECONDARY;
				result_columns.columns_in_view.back().originally_datetime = true;
			}
		}
	});

	// Proceed, finally, to the datetime columns, if they exist.  (If they don't, they will be added via ALTER TABLE to the temporary table under construction.)
	std::for_each(variable_group_raw_data_columns.columns_in_view.cbegin(),
		variable_group_raw_data_columns.columns_in_view.cend(), [&result_columns](ColumnsInTempView::ColumnInTempView const & column_in_view)
	{
		// Now do the datetime_start column
		if (column_in_view.column_type == ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__DATETIMESTART)
		{
			result_columns.columns_in_view.push_back(column_in_view);
		}
	});
	std::for_each(variable_group_raw_data_columns.columns_in_view.cbegin(),
		variable_group_raw_data_columns.columns_in_view.cend(), [&result_columns](ColumnsInTempView::ColumnInTempView const & column_in_view)
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
	std::for_each(result_columns.columns_in_view.begin(), result_columns.columns_in_view.end(), [&first, &variable_group_saved, &uoa_saved](
		ColumnsInTempView::ColumnInTempView & new_column)
	{
		new_column.column_name_in_temporary_table = new_column.column_name_in_temporary_table_no_uuid;
		new_column.column_name_in_temporary_table += "_";
		new_column.column_name_in_temporary_table += newUUID(true);

		new_column.is_within_inner_table_corresponding_to_top_level_uoa = true;

		if (first)
		{
			first = false;
			variable_group_saved = new_column.variable_group_associated_with_current_inner_table;
			uoa_saved = new_column.uoa_associated_with_variable_group_associated_with_current_inner_table;
		}
	});

	sql_strings.push_back(SQLExecutor(this, db));
	std::string & sql_string = sql_strings.back().sql;

	sql_string = "CREATE TABLE \"";
	sql_string += result_columns.view_name;
	sql_string += "\" AS ";
	sql_string += "SELECT ";

	first = true;
	std::for_each(result_columns.columns_in_view.begin(), result_columns.columns_in_view.end(), [&sql_string, &first](ColumnsInTempView::ColumnInTempView & new_column)
	{
		if (!first)
		{
			sql_string += ", ";
		}

		first = false;

		if (new_column.column_type == ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__PRIMARY && new_column.primary_key_should_be_treated_as_integer_____float_not_allowed_as_primary_key)
		{
			sql_string += "CAST (";
		}

		sql_string += "`";
		sql_string += new_column.column_name_in_temporary_table_no_uuid; // This is the original column name
		sql_string += "`";

		if (new_column.column_type == ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__PRIMARY && new_column.primary_key_should_be_treated_as_integer_____float_not_allowed_as_primary_key)
		{
			sql_string += " AS INTEGER)";
		}

		sql_string += " AS ";
		sql_string += "`";
		sql_string += new_column.column_name_in_temporary_table;
		sql_string += "`";
	});

	sql_string += " FROM \"";
	sql_string += result_columns.original_table_names[0];
	sql_string += "\"";

	// Only include rows in raw data that overlap the time range selected
	// by the user, or that have no time range granularity (represented by 0
	// in the time range fields)
	if (!variable_group_raw_data_columns.has_no_datetime_columns_originally)
	{
		sql_string += " WHERE CASE WHEN `";
		sql_string += result_columns.columns_in_view[result_columns.columns_in_view.size() - 2].column_name_in_temporary_table;
		sql_string += "` = 0 AND `";
		sql_string += result_columns.columns_in_view[result_columns.columns_in_view.size() - 1].column_name_in_temporary_table;
		sql_string += "` = 0 ";
		sql_string += " THEN 1 ";
		sql_string += " WHEN `";
		sql_string += result_columns.columns_in_view[result_columns.columns_in_view.size() - 2].column_name_in_temporary_table;
		sql_string += "` < ";
		sql_string += boost::lexical_cast<std::string>(timerange_end);
		sql_string += " THEN `";
		sql_string += result_columns.columns_in_view[result_columns.columns_in_view.size() - 1].column_name_in_temporary_table;
		sql_string += "` > ";
		sql_string += boost::lexical_cast<std::string>(timerange_start);
		sql_string += " WHEN `";
		sql_string += result_columns.columns_in_view[result_columns.columns_in_view.size() - 1].column_name_in_temporary_table;
		sql_string += "` > ";
		sql_string += boost::lexical_cast<std::string>(timerange_start);
		sql_string += " THEN `";
		sql_string += result_columns.columns_in_view[result_columns.columns_in_view.size() - 2].column_name_in_temporary_table;
		sql_string += "` < ";
		sql_string += boost::lexical_cast<std::string>(timerange_end);
		sql_string += " ELSE 0";
		sql_string += " END";
	}

	// SQL to add the datetime columns, if they are not present in the raw data table (filled with 0)
	if (variable_group_raw_data_columns.has_no_datetime_columns_originally)
	{

		result_columns.current_block_datetime_column_types = std::make_pair(ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__DATETIMESTART_INTERNAL,
			ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__DATETIMEEND_INTERNAL);

		std::string datetime_start_col_name_no_uuid = Table_VariableGroupMetadata_DateTimeColumns::DefaultDatetimeStartColumnName;
		std::string datetime_start_col_name = datetime_start_col_name_no_uuid;
		datetime_start_col_name += "_";
		datetime_start_col_name += newUUID(true);

		std::string alter_string;
		alter_string += "ALTER TABLE \"";
		alter_string += result_columns.view_name;
		alter_string += "\" ADD COLUMN ";
		alter_string += datetime_start_col_name;
		alter_string += " INTEGER DEFAULT 0";
		sql_strings.push_back(SQLExecutor(this, db, alter_string));

		if (failed)
		{
			SetFailureMessage(sql_error);
			return result;
		}

		if (CheckCancelled())
		{
			return result;
		}

		result_columns.columns_in_view.push_back(ColumnsInTempView::ColumnInTempView());
		ColumnsInTempView::ColumnInTempView & datetime_start_column = result_columns.columns_in_view.back();
		datetime_start_column.column_name_in_temporary_table = datetime_start_col_name;
		datetime_start_column.column_name_in_temporary_table_no_uuid = datetime_start_col_name_no_uuid;
		datetime_start_column.column_type = ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__DATETIMESTART_INTERNAL;
		datetime_start_column.variable_group_associated_with_current_inner_table = variable_group_saved;
		datetime_start_column.uoa_associated_with_variable_group_associated_with_current_inner_table = uoa_saved;
		datetime_start_column.column_name_in_original_data_table = "";
		datetime_start_column.is_within_inner_table_corresponding_to_top_level_uoa = true;
		datetime_start_column.current_multiplicity__of__current_inner_table__within__current_vg_inner_table_set = 1;
		datetime_start_column.number_inner_tables_in_set = variable_group_raw_data_columns.columns_in_view.back().number_inner_tables_in_set;

		std::string datetime_end_col_name_no_uuid = Table_VariableGroupMetadata_DateTimeColumns::DefaultDatetimeEndColumnName;
		std::string datetime_end_col_name = datetime_end_col_name_no_uuid;
		datetime_end_col_name += "_";
		datetime_end_col_name += newUUID(true);

		alter_string.clear();
		alter_string += "ALTER TABLE \"";
		alter_string += result_columns.view_name;
		alter_string += "\" ADD COLUMN ";
		alter_string += datetime_end_col_name;
		alter_string += " INTEGER DEFAULT 0";
		sql_strings.push_back(SQLExecutor(this, db, alter_string));

		if (failed)
		{
			SetFailureMessage(sql_error);
			return result;
		}

		if (CheckCancelled())
		{
			return result;
		}

		result_columns.columns_in_view.push_back(ColumnsInTempView::ColumnInTempView());
		ColumnsInTempView::ColumnInTempView & datetime_end_column = result_columns.columns_in_view.back();
		datetime_end_column.column_name_in_temporary_table = datetime_end_col_name;
		datetime_end_column.column_name_in_temporary_table_no_uuid = datetime_end_col_name_no_uuid;
		datetime_end_column.column_type = ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__DATETIMEEND_INTERNAL;
		datetime_end_column.variable_group_associated_with_current_inner_table = variable_group_saved;
		datetime_end_column.uoa_associated_with_variable_group_associated_with_current_inner_table = uoa_saved;
		datetime_end_column.column_name_in_original_data_table = "";
		datetime_end_column.is_within_inner_table_corresponding_to_top_level_uoa = true;
		datetime_end_column.current_multiplicity__of__current_inner_table__within__current_vg_inner_table_set = 1;
		datetime_end_column.number_inner_tables_in_set = variable_group_raw_data_columns.columns_in_view.back().number_inner_tables_in_set;

	}
	else
	{
		result_columns.current_block_datetime_column_types = std::make_pair(ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__DATETIMESTART,
			ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__DATETIMEEND);
		at_least_one_variable_group_has_timerange = true;
	}

	result_columns.previous_block_datetime_column_types = result_columns.current_block_datetime_column_types;

	return result;

}

OutputModel::OutputGenerator::SqlAndColumnSet OutputModel::OutputGenerator::CreateInitialPrimaryXTable_OrCount(ColumnsInTempView const & primary_variable_group_raw_data_columns,
		int const primary_group_number, bool const count_only)
{

	SqlAndColumnSet result = std::make_pair(std::vector<SQLExecutor>(), ColumnsInTempView());
	std::vector<SQLExecutor> & sql_strings = result.first;
	ColumnsInTempView & result_columns = result.second;

	result_columns = primary_variable_group_raw_data_columns;

	result_columns.view_number = 1;
	result_columns.has_no_datetime_columns = false;
	std::string view_name = "NGTEMP_V";
	view_name += std::to_string(primary_group_number);
	view_name += "_x";
	view_name += "1";
	result_columns.view_name_no_uuid = view_name;
	view_name += "_";
	view_name += newUUID(true);
	result_columns.view_name = view_name;

	WidgetInstanceIdentifiers const & variables_selected =
		(*the_map)[*primary_variable_group_raw_data_columns.variable_groups[0].identifier_parent][primary_variable_group_raw_data_columns.variable_groups[0]];

	result_columns.columns_in_view.clear();

	// Add the columns from the raw data table into this initial temporary table.
	// Start with the primary key columns.
	std::for_each(primary_variable_group_raw_data_columns.columns_in_view.cbegin(),
				  primary_variable_group_raw_data_columns.columns_in_view.cend(), [&result_columns, &variables_selected](ColumnsInTempView::ColumnInTempView const & column_in_view)
	{
		if (column_in_view.column_type == ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__DATETIMESTART
			|| column_in_view.column_type == ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__DATETIMEEND)
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
	std::for_each(primary_variable_group_raw_data_columns.columns_in_view.cbegin(),
				  primary_variable_group_raw_data_columns.columns_in_view.cend(), [&result_columns, &variables_selected](ColumnsInTempView::ColumnInTempView const & column_in_view)
	{
		bool make_secondary_datetime_column = false;

		if (column_in_view.column_type == ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__DATETIMESTART
			|| column_in_view.column_type == ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__DATETIMEEND)
		{
			// No!  If the user selects these columns, they should appear as regular secondary key columns.  Change the column type in this case to "secondary".
			//return; // Enforce that datetime columns appear last.
			make_secondary_datetime_column = true;
		}

		if (column_in_view.column_type != ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__SECONDARY)
		{
			// No!  If the user selects these columns, they should appear as regular secondary key columns.  Change the column type in this case to "secondary".
			if (!make_secondary_datetime_column)
			{
				return; // We are populating secondary columns now, so exit if this isn't one
			}
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

			if (make_secondary_datetime_column)
			{
				result_columns.columns_in_view.back().column_type = ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__SECONDARY;
				result_columns.columns_in_view.back().originally_datetime = true;
			}
		}
	});

	// Proceed, finally, to the datetime columns, if they exist.  (If they don't, they will be added via ALTER TABLE to the temporary table under construction.)
	std::for_each(primary_variable_group_raw_data_columns.columns_in_view.cbegin(),
				  primary_variable_group_raw_data_columns.columns_in_view.cend(), [&result_columns](ColumnsInTempView::ColumnInTempView const & column_in_view)
	{
		// Now do the datetime_start column
		if (column_in_view.column_type == ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__DATETIMESTART)
		{
			result_columns.columns_in_view.push_back(column_in_view);
		}
	});
	std::for_each(primary_variable_group_raw_data_columns.columns_in_view.cbegin(),
				  primary_variable_group_raw_data_columns.columns_in_view.cend(), [&result_columns](ColumnsInTempView::ColumnInTempView const & column_in_view)
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
	std::for_each(result_columns.columns_in_view.begin(), result_columns.columns_in_view.end(), [&first, &variable_group_saved, &uoa_saved](
					  ColumnsInTempView::ColumnInTempView & new_column)
	{
		new_column.column_name_in_temporary_table = new_column.column_name_in_temporary_table_no_uuid;
		new_column.column_name_in_temporary_table += "_";
		new_column.column_name_in_temporary_table += newUUID(true);

		new_column.is_within_inner_table_corresponding_to_top_level_uoa = true;

		if (first)
		{
			first = false;
			variable_group_saved = new_column.variable_group_associated_with_current_inner_table;
			uoa_saved = new_column.uoa_associated_with_variable_group_associated_with_current_inner_table;
		}
	});

	sql_strings.push_back(SQLExecutor(this, db));
	std::string & sql_string = sql_strings.back().sql;

	if (!count_only)
	{
		sql_string = "CREATE TABLE \"";
		sql_string += result_columns.view_name;
		sql_string += "\" AS ";
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

			if (new_column.column_type == ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__PRIMARY && new_column.primary_key_should_be_treated_as_integer_____float_not_allowed_as_primary_key)
			{
				sql_string += "CAST (";
			}

			sql_string += "`";
			sql_string += new_column.column_name_in_temporary_table_no_uuid; // This is the original column name
			sql_string += "`";

			if (new_column.column_type == ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__PRIMARY && new_column.primary_key_should_be_treated_as_integer_____float_not_allowed_as_primary_key)
			{
				sql_string += " AS INTEGER)";
			}

			sql_string += " AS ";
			sql_string += "`";
			sql_string += new_column.column_name_in_temporary_table;
			sql_string += "`";
		});
	}

	sql_string += " FROM \"";
	sql_string += result_columns.original_table_names[0];
	sql_string += "\"";

	// Only include rows in raw data that overlap the time range selected
	// by the user, or that have no time range granularity (represented by 0
	// in the time range fields)
	if (!primary_variable_group_raw_data_columns.has_no_datetime_columns_originally)
	{
		if (count_only)
		{
			sql_string += " WHERE CASE WHEN `";
			sql_string += result_columns.columns_in_view[result_columns.columns_in_view.size() - 2].column_name_in_temporary_table_no_uuid;
			sql_string += "` = 0 AND `";
			sql_string += result_columns.columns_in_view[result_columns.columns_in_view.size() - 1].column_name_in_temporary_table_no_uuid;
			sql_string += "` = 0 ";
			sql_string += " THEN 1 ";
			sql_string += " WHEN `";
			sql_string += result_columns.columns_in_view[result_columns.columns_in_view.size() - 2].column_name_in_temporary_table_no_uuid;
			sql_string += "` < ";
			sql_string += boost::lexical_cast<std::string>(timerange_end);
			sql_string += " THEN `";
			sql_string += result_columns.columns_in_view[result_columns.columns_in_view.size() - 1].column_name_in_temporary_table_no_uuid;
			sql_string += "` > ";
			sql_string += boost::lexical_cast<std::string>(timerange_start);
			sql_string += " WHEN `";
			sql_string += result_columns.columns_in_view[result_columns.columns_in_view.size() - 1].column_name_in_temporary_table_no_uuid;
			sql_string += "` > ";
			sql_string += boost::lexical_cast<std::string>(timerange_start);
			sql_string += " THEN `";
			sql_string += result_columns.columns_in_view[result_columns.columns_in_view.size() - 2].column_name_in_temporary_table_no_uuid;
			sql_string += "` < ";
			sql_string += boost::lexical_cast<std::string>(timerange_end);
			sql_string += " ELSE 0";
			sql_string += " END";
		}
		else
		{
			sql_string += " WHERE CASE WHEN `";
			sql_string += result_columns.columns_in_view[result_columns.columns_in_view.size() - 2].column_name_in_temporary_table;
			sql_string += "` = 0 AND `";
			sql_string += result_columns.columns_in_view[result_columns.columns_in_view.size() - 1].column_name_in_temporary_table;
			sql_string += "` = 0 ";
			sql_string += " THEN 1 ";
			sql_string += " WHEN `";
			sql_string += result_columns.columns_in_view[result_columns.columns_in_view.size() - 2].column_name_in_temporary_table;
			sql_string += "` < ";
			sql_string += boost::lexical_cast<std::string>(timerange_end);
			sql_string += " THEN `";
			sql_string += result_columns.columns_in_view[result_columns.columns_in_view.size() - 1].column_name_in_temporary_table;
			sql_string += "` > ";
			sql_string += boost::lexical_cast<std::string>(timerange_start);
			sql_string += " WHEN `";
			sql_string += result_columns.columns_in_view[result_columns.columns_in_view.size() - 1].column_name_in_temporary_table;
			sql_string += "` > ";
			sql_string += boost::lexical_cast<std::string>(timerange_start);
			sql_string += " THEN `";
			sql_string += result_columns.columns_in_view[result_columns.columns_in_view.size() - 2].column_name_in_temporary_table;
			sql_string += "` < ";
			sql_string += boost::lexical_cast<std::string>(timerange_end);
			sql_string += " ELSE 0";
			sql_string += " END";
		}
	}

	// Add the ORDER BY column/s
	//if (!count_only && debug_ordering)
	if (true)
	{

		if (!count_only && true)
		{

			bool first = true;

			// First order by primary key columns (with multiplicity 1)
			std::for_each(result_columns.columns_in_view.begin(), result_columns.columns_in_view.end(), [this, &sql_string, &result_columns, &first](
							  ColumnsInTempView::ColumnInTempView & view_column)
			{
				// Determine how many columns there are corresponding to the DMU category
				int number_primary_key_columns_in_dmu_category_with_multiplicity_of_1 = 0;
				std::for_each(result_columns.columns_in_view.begin(),
							  result_columns.columns_in_view.end(), [this, &view_column, &number_primary_key_columns_in_dmu_category_with_multiplicity_of_1, &sql_string](
								  ColumnsInTempView::ColumnInTempView & view_column_)
				{
					if (view_column_.column_type == ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__PRIMARY)
					{
						if (view_column_.primary_key_dmu_category_identifier.IsEqual(WidgetInstanceIdentifier::EQUALITY_CHECK_TYPE__STRING_CODE, view_column.primary_key_dmu_category_identifier))
						{
							if (view_column_.total_outer_multiplicity__in_total_kad__for_current_dmu_category__for_current_variable_group == 1)
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
					if (view_column.total_outer_multiplicity__in_total_kad__for_current_dmu_category__for_current_variable_group == 1)
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

									if (view_column.primary_key_should_be_treated_as_integer_____float_not_allowed_as_primary_key)
									{
										sql_string += "CAST (";
									}

									sql_string += "`";
									sql_string += view_column.column_name_in_temporary_table;
									sql_string += "`";

									if (view_column.primary_key_should_be_treated_as_integer_____float_not_allowed_as_primary_key)
									{
										sql_string += " AS INTEGER)";
									}
								}
							}
						}
					}
				}
			});

			if (true)
			{

				// Then by columns with multiplicity greater than 1
				if (highest_multiplicity_primary_uoa > 1)
				{

					// Determine how many columns there are corresponding to the DMU category with multiplicity greater than 1
					int number_primary_key_columns_in_dmu_category_with_multiplicity_greater_than_1 = 0;
					std::for_each(result_columns.columns_in_view.begin(),
								  result_columns.columns_in_view.end(), [this, &number_primary_key_columns_in_dmu_category_with_multiplicity_greater_than_1, &sql_string](
									  ColumnsInTempView::ColumnInTempView & view_column)
					{
						if (view_column.column_type == ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__PRIMARY)
						{
							if (view_column.is_within_inner_table_corresponding_to_top_level_uoa)
							{
								if (view_column.total_outer_multiplicity__in_total_kad__for_current_dmu_category__for_current_variable_group == highest_multiplicity_primary_uoa)
								{
									++number_primary_key_columns_in_dmu_category_with_multiplicity_greater_than_1;
								}
							}
						}
					});

					// Create the ORDER BY clause, taking the proper primary key columns that compose the DMU category with multiplicity greater than 1, in sequence
					for (int inner_dmu_multiplicity = 0; inner_dmu_multiplicity < number_primary_key_columns_in_dmu_category_with_multiplicity_greater_than_1; ++inner_dmu_multiplicity)
					{
						std::for_each(result_columns.columns_in_view.begin(), result_columns.columns_in_view.end(), [this, &inner_dmu_multiplicity, &sql_string, &first](
										  ColumnsInTempView::ColumnInTempView & view_column)
						{
							if (view_column.column_type == ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__PRIMARY)
							{
								if (view_column.total_outer_multiplicity__in_total_kad__for_current_dmu_category__for_current_variable_group == highest_multiplicity_primary_uoa)
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

											if (view_column.primary_key_should_be_treated_as_integer_____float_not_allowed_as_primary_key)
											{
												sql_string += "CAST (";
											}

											sql_string += "`";
											sql_string += view_column.column_name_in_temporary_table;
											sql_string += "`";

											if (view_column.primary_key_should_be_treated_as_integer_____float_not_allowed_as_primary_key)
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

			}

			// Finally, order by the time range columns
			if (!primary_variable_group_raw_data_columns.has_no_datetime_columns_originally)
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
				sql_string += "`";
				sql_string += result_columns.columns_in_view[result_columns.columns_in_view.size() - 2].column_name_in_temporary_table_no_uuid; // final merged datetime start column
				sql_string += "`";
				sql_string += ", ";
				sql_string += "`";
				sql_string += result_columns.columns_in_view[result_columns.columns_in_view.size() - 1].column_name_in_temporary_table_no_uuid; // final merged datetime end column
				sql_string += "`";
			}

		}

	}

	if (!count_only)
	{
		// SQL to add the datetime columns, if they are not present in the raw data table (filled with 0)
		if (primary_variable_group_raw_data_columns.has_no_datetime_columns_originally)
		{

			result_columns.current_block_datetime_column_types = std::make_pair(ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__DATETIMESTART_INTERNAL,
					ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__DATETIMEEND_INTERNAL);

			std::string datetime_start_col_name_no_uuid = Table_VariableGroupMetadata_DateTimeColumns::DefaultDatetimeStartColumnName;
			std::string datetime_start_col_name = datetime_start_col_name_no_uuid;
			datetime_start_col_name += "_";
			datetime_start_col_name += newUUID(true);

			std::string alter_string;
			alter_string += "ALTER TABLE \"";
			alter_string += result_columns.view_name;
			alter_string += "\" ADD COLUMN ";
			alter_string += datetime_start_col_name;
			alter_string += " INTEGER DEFAULT 0";
			sql_strings.push_back(SQLExecutor(this, db, alter_string));

			if (failed)
			{
				SetFailureMessage(sql_error);
				return result;
			}

			if (CheckCancelled())
			{
				return result;
			}

			result_columns.columns_in_view.push_back(ColumnsInTempView::ColumnInTempView());
			ColumnsInTempView::ColumnInTempView & datetime_start_column = result_columns.columns_in_view.back();
			datetime_start_column.column_name_in_temporary_table = datetime_start_col_name;
			datetime_start_column.column_name_in_temporary_table_no_uuid = datetime_start_col_name_no_uuid;
			datetime_start_column.column_type = ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__DATETIMESTART_INTERNAL;
			datetime_start_column.variable_group_associated_with_current_inner_table = variable_group_saved;
			datetime_start_column.uoa_associated_with_variable_group_associated_with_current_inner_table = uoa_saved;
			datetime_start_column.column_name_in_original_data_table = "";
			datetime_start_column.is_within_inner_table_corresponding_to_top_level_uoa = true;
			datetime_start_column.current_multiplicity__of__current_inner_table__within__current_vg_inner_table_set = 1;
			datetime_start_column.number_inner_tables_in_set = primary_variable_group_raw_data_columns.columns_in_view.back().number_inner_tables_in_set;

			std::string datetime_end_col_name_no_uuid = Table_VariableGroupMetadata_DateTimeColumns::DefaultDatetimeEndColumnName;
			std::string datetime_end_col_name = datetime_end_col_name_no_uuid;
			datetime_end_col_name += "_";
			datetime_end_col_name += newUUID(true);

			alter_string.clear();
			alter_string += "ALTER TABLE \"";
			alter_string += result_columns.view_name;
			alter_string += "\" ADD COLUMN ";
			alter_string += datetime_end_col_name;
			alter_string += " INTEGER DEFAULT 0";
			sql_strings.push_back(SQLExecutor(this, db, alter_string));

			if (failed)
			{
				SetFailureMessage(sql_error);
				return result;
			}

			if (CheckCancelled())
			{
				return result;
			}

			result_columns.columns_in_view.push_back(ColumnsInTempView::ColumnInTempView());
			ColumnsInTempView::ColumnInTempView & datetime_end_column = result_columns.columns_in_view.back();
			datetime_end_column.column_name_in_temporary_table = datetime_end_col_name;
			datetime_end_column.column_name_in_temporary_table_no_uuid = datetime_end_col_name_no_uuid;
			datetime_end_column.column_type = ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__DATETIMEEND_INTERNAL;
			datetime_end_column.variable_group_associated_with_current_inner_table = variable_group_saved;
			datetime_end_column.uoa_associated_with_variable_group_associated_with_current_inner_table = uoa_saved;
			datetime_end_column.column_name_in_original_data_table = "";
			datetime_end_column.is_within_inner_table_corresponding_to_top_level_uoa = true;
			datetime_end_column.current_multiplicity__of__current_inner_table__within__current_vg_inner_table_set = 1;
			datetime_end_column.number_inner_tables_in_set = primary_variable_group_raw_data_columns.columns_in_view.back().number_inner_tables_in_set;

		}
		else
		{
			result_columns.current_block_datetime_column_types = std::make_pair(ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__DATETIMESTART,
					ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__DATETIMEEND);
			at_least_one_variable_group_has_timerange = true;
		}

		result_columns.previous_block_datetime_column_types = result_columns.current_block_datetime_column_types;
	}

	return result;

}

OutputModel::OutputGenerator::SqlAndColumnSet OutputModel::OutputGenerator::CreateInitialPrimaryXRTable(ColumnsInTempView const & primary_variable_group_x1_columns,
		int const primary_group_number)
{

	// ************************************************************************************************ //
	// This function doesn't do anything complicated because it is creating the INITIAL "XR" table.
	// ... So this function just copies the data from the initial "X" table, and adds
	// ... the required additional time range columns so that the schema becomes correct
	// ... for an "XR" table, as expected by the following stage of the algorithm.
	// ************************************************************************************************ //

	SqlAndColumnSet result = std::make_pair(std::vector<SQLExecutor>(), ColumnsInTempView());
	std::vector<SQLExecutor> & sql_strings = result.first;
	ColumnsInTempView & result_columns = result.second;

	result_columns = primary_variable_group_x1_columns;

	std::string view_name = "NGTEMP_V";
	view_name += std::to_string(primary_group_number);
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
	std::for_each(result_columns.columns_in_view.begin(),
				  result_columns.columns_in_view.end(), [&result_columns, &variable_group, &uoa, &x1_datetime_start_column_index, &x1_datetime_end_column_index, &column_index, &first](
					  ColumnsInTempView::ColumnInTempView & new_column)
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

		if (new_column.column_type == result_columns.previous_block_datetime_column_types.first)
		{
			x1_datetime_start_column_index = column_index;
		}

		if (new_column.column_type == result_columns.previous_block_datetime_column_types.second)
		{
			x1_datetime_end_column_index = column_index;
		}

		++column_index;
	});

	sql_strings.push_back(SQLExecutor(this, db));
	std::string & sql_string = sql_strings.back().sql;

	sql_string = "CREATE TABLE \"";
	sql_string += result_columns.view_name;
	sql_string += "\" AS SELECT ";
	first = true;
	int the_index = 0;
	std::for_each(result_columns.columns_in_view.begin(), result_columns.columns_in_view.end(), [&sql_string, &the_index, &primary_variable_group_x1_columns, &first](
					  ColumnsInTempView::ColumnInTempView & new_column)
	{
		if (!first)
		{
			sql_string += ", ";
		}

		first = false;

		if (new_column.column_type == ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__PRIMARY && new_column.primary_key_should_be_treated_as_integer_____float_not_allowed_as_primary_key)
		{
			sql_string += "CAST (";
		}

		sql_string += "`";
		sql_string += primary_variable_group_x1_columns.columns_in_view[the_index].column_name_in_temporary_table; // This is the original column name
		sql_string += "`";

		if (new_column.column_type == ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__PRIMARY && new_column.primary_key_should_be_treated_as_integer_____float_not_allowed_as_primary_key)
		{
			sql_string += " AS INTEGER)";
		}

		sql_string += " AS ";
		sql_string += "`";
		sql_string += new_column.column_name_in_temporary_table;
		sql_string += "`";
		++the_index;
	});
	sql_string += " FROM \"";
	sql_string += primary_variable_group_x1_columns.view_name;
	sql_string += "\"";


	// Add the "merged" time range columns

	std::string datetime_start_col_name_no_uuid = "DATETIMESTART__PRIMARY_VG_INNER_TABLE_MERGE__BEFORE_DUPLICATES_REMOVED";
	std::string datetime_start_col_name = datetime_start_col_name_no_uuid;
	datetime_start_col_name += "_";
	datetime_start_col_name += newUUID(true);

	std::string alter_string;
	alter_string += "ALTER TABLE \"";
	alter_string += result_columns.view_name;
	alter_string += "\" ADD COLUMN ";
	alter_string += datetime_start_col_name;
	alter_string += " INTEGER DEFAULT 0";
	sql_strings.push_back(SQLExecutor(this, db, alter_string));

	if (failed)
	{
		SetFailureMessage(sql_error);
		return result;
	}

	if (CheckCancelled())
	{
		return result;
	}

	result_columns.columns_in_view.push_back(ColumnsInTempView::ColumnInTempView());
	ColumnsInTempView::ColumnInTempView & datetime_start_column = result_columns.columns_in_view.back();
	datetime_start_column.column_name_in_temporary_table = datetime_start_col_name;
	datetime_start_column.column_name_in_temporary_table_no_uuid = datetime_start_col_name_no_uuid;
	datetime_start_column.column_type = ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__DATETIMESTART__PRIMARY_VG_INNER_TABLE_MERGE__BEFORE_DUPLICATES_REMOVED;
	datetime_start_column.variable_group_associated_with_current_inner_table = variable_group;
	datetime_start_column.uoa_associated_with_variable_group_associated_with_current_inner_table = uoa;
	datetime_start_column.column_name_in_original_data_table = "";
	datetime_start_column.is_within_inner_table_corresponding_to_top_level_uoa = true;
	datetime_start_column.current_multiplicity__of__current_inner_table__within__current_vg_inner_table_set = 1;
	datetime_start_column.number_inner_tables_in_set = primary_variable_group_x1_columns.columns_in_view.back().number_inner_tables_in_set;

	std::string datetime_end_col_name_no_uuid = "DATETIMEEND__PRIMARY_VG_INNER_TABLE_MERGE__BEFORE_DUPLICATES_REMOVED";
	std::string datetime_end_col_name = datetime_end_col_name_no_uuid;
	datetime_end_col_name += "_";
	datetime_end_col_name += newUUID(true);

	alter_string.clear();
	alter_string += "ALTER TABLE \"";
	alter_string += result_columns.view_name;
	alter_string += "\" ADD COLUMN ";
	alter_string += datetime_end_col_name;
	alter_string += " INTEGER DEFAULT 0";
	sql_strings.push_back(SQLExecutor(this, db, alter_string));

	if (failed)
	{
		SetFailureMessage(sql_error);
		return result;
	}

	if (CheckCancelled())
	{
		return result;
	}

	result_columns.columns_in_view.push_back(ColumnsInTempView::ColumnInTempView());
	ColumnsInTempView::ColumnInTempView & datetime_end_column = result_columns.columns_in_view.back();
	datetime_end_column.column_name_in_temporary_table = datetime_end_col_name;
	datetime_end_column.column_name_in_temporary_table_no_uuid = datetime_end_col_name_no_uuid;
	datetime_end_column.column_type = ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__DATETIMEEND__PRIMARY_VG_INNER_TABLE_MERGE__BEFORE_DUPLICATES_REMOVED;
	datetime_end_column.variable_group_associated_with_current_inner_table = variable_group;
	datetime_end_column.uoa_associated_with_variable_group_associated_with_current_inner_table = uoa;
	datetime_end_column.column_name_in_original_data_table = "";
	datetime_end_column.is_within_inner_table_corresponding_to_top_level_uoa = true;
	datetime_end_column.current_multiplicity__of__current_inner_table__within__current_vg_inner_table_set = 1;
	datetime_end_column.number_inner_tables_in_set = primary_variable_group_x1_columns.columns_in_view.back().number_inner_tables_in_set;


	// Set the "merged" time range columns to be equal to the original time range columns
	std::string sql_time_range;
	sql_time_range += "UPDATE OR FAIL \"";
	sql_time_range += result_columns.view_name;
	sql_time_range += "\" SET ";
	sql_time_range += "`";
	sql_time_range += datetime_start_col_name;
	sql_time_range += "`";
	sql_time_range += " = ";
	sql_time_range += result_columns.columns_in_view[x1_datetime_start_column_index].column_name_in_temporary_table;
	sql_time_range += ", ";
	sql_time_range += "`";
	sql_time_range += datetime_end_col_name;
	sql_time_range += "`";
	sql_time_range += " = ";
	sql_time_range += result_columns.columns_in_view[x1_datetime_end_column_index].column_name_in_temporary_table;
	sql_strings.push_back(SQLExecutor(this, db, sql_time_range));

	if (failed)
	{
		SetFailureMessage(sql_error);
		return result;
	}

	if (CheckCancelled())
	{
		return result;
	}

	result_columns.current_block_datetime_column_types = std::make_pair(
				ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__DATETIMESTART__PRIMARY_VG_INNER_TABLE_MERGE__BEFORE_DUPLICATES_REMOVED,
				ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__DATETIMEEND__PRIMARY_VG_INNER_TABLE_MERGE__BEFORE_DUPLICATES_REMOVED);
	result_columns.previous_block_datetime_column_types = result_columns.current_block_datetime_column_types;

	return result;

}

OutputModel::OutputGenerator::SqlAndColumnSet OutputModel::OutputGenerator::CreateInitialPrimaryMergeXRTable(ColumnsInTempView const & first_final_primary_variable_group_columns)
{

	SqlAndColumnSet result = std::make_pair(std::vector<SQLExecutor>(), ColumnsInTempView());
	std::vector<SQLExecutor> & sql_strings = result.first;
	ColumnsInTempView & result_columns = result.second;

	result_columns = first_final_primary_variable_group_columns;

	std::string view_name = "NGTEMP_MF0";
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
	std::for_each(result_columns.columns_in_view.begin(), result_columns.columns_in_view.end(), [&variable_group, &uoa, &column_index, &first](
					  ColumnsInTempView::ColumnInTempView & new_column)
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

	sql_strings.push_back(SQLExecutor(this, db));
	std::string & sql_string = sql_strings.back().sql;

	sql_string = "CREATE TABLE \"";
	sql_string += result_columns.view_name;
	sql_string += "\" AS SELECT ";
	first = true;
	int the_index = 0;
	std::for_each(result_columns.columns_in_view.begin(), result_columns.columns_in_view.end(), [&sql_string, &the_index, &first_final_primary_variable_group_columns, &first](
					  ColumnsInTempView::ColumnInTempView & new_column)
	{
		if (!first)
		{
			sql_string += ", ";
		}

		first = false;

		if (new_column.column_type == ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__PRIMARY && new_column.primary_key_should_be_treated_as_integer_____float_not_allowed_as_primary_key)
		{
			sql_string += "CAST (";
		}

		sql_string += "`";
		sql_string += first_final_primary_variable_group_columns.columns_in_view[the_index].column_name_in_temporary_table; // This is the original column name
		sql_string += "`";

		if (new_column.column_type == ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__PRIMARY && new_column.primary_key_should_be_treated_as_integer_____float_not_allowed_as_primary_key)
		{
			sql_string += " AS INTEGER)";
		}

		sql_string += " AS ";
		sql_string += "`";
		sql_string += new_column.column_name_in_temporary_table;
		sql_string += "`";
		++the_index;
	});
	sql_string += " FROM \"";
	sql_string += first_final_primary_variable_group_columns.view_name;
	sql_string += "\"";


	// Add the "merged" time range columns

	std::string datetime_start_col_name_no_uuid = "DATETIMESTART__POST_TIMERANGE_MERGED_BETWEEN_TOP_LEVEL_PRIMARY_VARIABLE_GROUPS";
	std::string datetime_start_col_name = datetime_start_col_name_no_uuid;
	datetime_start_col_name += "_";
	datetime_start_col_name += newUUID(true);

	std::string alter_string;
	alter_string += "ALTER TABLE \"";
	alter_string += result_columns.view_name;
	alter_string += "\" ADD COLUMN ";
	alter_string += datetime_start_col_name;
	alter_string += " INTEGER DEFAULT 0";
	sql_strings.push_back(SQLExecutor(this, db, alter_string));

	if (failed)
	{
		SetFailureMessage(sql_error);
		return result;
	}

	if (CheckCancelled())
	{
		return result;
	}

	result_columns.columns_in_view.push_back(ColumnsInTempView::ColumnInTempView());
	ColumnsInTempView::ColumnInTempView & datetime_start_column = result_columns.columns_in_view.back();
	datetime_start_column.column_name_in_temporary_table = datetime_start_col_name;
	datetime_start_column.column_name_in_temporary_table_no_uuid = datetime_start_col_name_no_uuid;
	datetime_start_column.column_type = ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__DATETIMESTART__POST_TIMERANGE_MERGED_BETWEEN_TOP_LEVEL_PRIMARY_VARIABLE_GROUPS;
	datetime_start_column.variable_group_associated_with_current_inner_table = variable_group;
	datetime_start_column.uoa_associated_with_variable_group_associated_with_current_inner_table = uoa;
	datetime_start_column.column_name_in_original_data_table = "";
	datetime_start_column.is_within_inner_table_corresponding_to_top_level_uoa = true;
	datetime_start_column.current_multiplicity__of__current_inner_table__within__current_vg_inner_table_set =
		first_final_primary_variable_group_columns.columns_in_view.back().current_multiplicity__of__current_inner_table__within__current_vg_inner_table_set;
	datetime_start_column.number_inner_tables_in_set = first_final_primary_variable_group_columns.columns_in_view.back().number_inner_tables_in_set;


	std::string datetime_end_col_name_no_uuid = "DATETIMEEND__POST_TIMERANGE_MERGED_BETWEEN_TOP_LEVEL_PRIMARY_VARIABLE_GROUPS";
	std::string datetime_end_col_name = datetime_end_col_name_no_uuid;
	datetime_end_col_name += "_";
	datetime_end_col_name += newUUID(true);

	alter_string.clear();
	alter_string += "ALTER TABLE \"";
	alter_string += result_columns.view_name;
	alter_string += "\" ADD COLUMN ";
	alter_string += datetime_end_col_name;
	alter_string += " INTEGER DEFAULT 0";
	sql_strings.push_back(SQLExecutor(this, db, alter_string));

	if (failed)
	{
		SetFailureMessage(sql_error);
		return result;
	}

	if (CheckCancelled())
	{
		return result;
	}

	result_columns.columns_in_view.push_back(ColumnsInTempView::ColumnInTempView());
	ColumnsInTempView::ColumnInTempView & datetime_end_column = result_columns.columns_in_view.back();
	datetime_end_column.column_name_in_temporary_table = datetime_end_col_name;
	datetime_end_column.column_name_in_temporary_table_no_uuid = datetime_end_col_name_no_uuid;
	datetime_end_column.column_type = ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__DATETIMEEND__POST_TIMERANGE_MERGED_BETWEEN_TOP_LEVEL_PRIMARY_VARIABLE_GROUPS;
	datetime_end_column.variable_group_associated_with_current_inner_table = variable_group;
	datetime_end_column.uoa_associated_with_variable_group_associated_with_current_inner_table = uoa;
	datetime_end_column.column_name_in_original_data_table = "";
	datetime_end_column.is_within_inner_table_corresponding_to_top_level_uoa = true;
	datetime_end_column.current_multiplicity__of__current_inner_table__within__current_vg_inner_table_set =
		first_final_primary_variable_group_columns.columns_in_view.back().current_multiplicity__of__current_inner_table__within__current_vg_inner_table_set;
	datetime_end_column.number_inner_tables_in_set = first_final_primary_variable_group_columns.columns_in_view.back().number_inner_tables_in_set;


	// Set the "merged" time range columns to be equal to the original time range columns
	std::string sql_time_range;
	sql_time_range += "UPDATE OR FAIL \"";
	sql_time_range += result_columns.view_name;
	sql_time_range += "\" SET ";
	sql_time_range += "`";
	sql_time_range += datetime_start_col_name;
	sql_time_range += "`";
	sql_time_range += " = ";
	sql_time_range += result_columns.columns_in_view[result_columns.columns_in_view.size() - 4].column_name_in_temporary_table;
	sql_time_range += ", ";
	sql_time_range += "`";
	sql_time_range += datetime_end_col_name;
	sql_time_range += "`";
	sql_time_range += " = ";
	sql_time_range += result_columns.columns_in_view[result_columns.columns_in_view.size() - 3].column_name_in_temporary_table;
	sql_strings.push_back(SQLExecutor(this, db, sql_time_range));

	if (failed)
	{
		SetFailureMessage(sql_error);
		return result;
	}

	if (CheckCancelled())
	{
		return result;
	}

	result_columns.current_block_datetime_column_types = std::make_pair(
				ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__DATETIMESTART__POST_TIMERANGE_MERGED_BETWEEN_TOP_LEVEL_PRIMARY_VARIABLE_GROUPS,
				ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__DATETIMEEND__POST_TIMERANGE_MERGED_BETWEEN_TOP_LEVEL_PRIMARY_VARIABLE_GROUPS);
	result_columns.previous_block_datetime_column_types = result_columns.current_block_datetime_column_types;

	return result;

}

OutputModel::OutputGenerator::SqlAndColumnSet OutputModel::OutputGenerator::CreatePrimaryXTable(ColumnsInTempView const & primary_variable_group_raw_data_columns,
		ColumnsInTempView const & previous_xr_columns, int const current_multiplicity, int const primary_group_number)
{

	SqlAndColumnSet result = std::make_pair(std::vector<SQLExecutor>(), ColumnsInTempView());
	std::vector<SQLExecutor> & sql_strings = result.first;
	ColumnsInTempView & result_columns = result.second;

	result_columns = previous_xr_columns;

	std::string view_name = "NGTEMP_V";
	view_name += std::to_string(primary_group_number);
	view_name += "_x";
	view_name += std::to_string(current_multiplicity);
	result_columns.view_name_no_uuid = view_name;
	view_name += "_";
	view_name += newUUID(true);
	result_columns.view_name = view_name;
	result_columns.view_number = current_multiplicity;
	result_columns.has_no_datetime_columns = false;

	int first_full_table_column_count = 0;
	inner_table_no_multiplicities__with_all_datetime_columns_included__column_count = 0;
	int second_table_column_count = 0;

	std::vector<std::string> previous_column_names_first_table;

	WidgetInstanceIdentifier variable_group;
	WidgetInstanceIdentifier uoa;

	// These columns are from the previous XR temporary table, which is guaranteed to have all columns in place, including datetime columns.
	// Further, the "current_multiplicity" of these columns is guaranteed to be correct.

	// First, calculate some indices.
	bool first = true;
	bool in_first_inner_table = true;
	bool reached_first_datetime_start_merged_column = false;
	bool reached_first_datetime_end_merged_column = false;
	std::for_each(result_columns.columns_in_view.begin(),
				  result_columns.columns_in_view.end(), [this, &first_full_table_column_count, &reached_first_datetime_start_merged_column, &reached_first_datetime_end_merged_column, &in_first_inner_table, &previous_column_names_first_table, &variable_group, &uoa, &first](
					  ColumnsInTempView::ColumnInTempView & new_column)
	{
		previous_column_names_first_table.push_back(new_column.column_name_in_temporary_table);
		new_column.column_name_in_temporary_table = new_column.column_name_in_temporary_table_no_uuid;
		new_column.column_name_in_temporary_table += "_";
		new_column.column_name_in_temporary_table += newUUID(true);
		++first_full_table_column_count;

		if (new_column.column_type == ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__DATETIMESTART__PRIMARY_VG_INNER_TABLE_MERGE__AFTER_DUPLICATES_REMOVED)
		{
			reached_first_datetime_start_merged_column = true;
		}

		if (new_column.column_type == ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__DATETIMEEND__PRIMARY_VG_INNER_TABLE_MERGE__AFTER_DUPLICATES_REMOVED)
		{
			reached_first_datetime_end_merged_column = true;
		}

		if (in_first_inner_table)
		{
			++inner_table_no_multiplicities__with_all_datetime_columns_included__column_count;
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

	WidgetInstanceIdentifiers const & variables_selected =
		(*the_map)[*primary_variable_group_raw_data_columns.variable_groups[0].identifier_parent][primary_variable_group_raw_data_columns.variable_groups[0]];

	// These columns are from the new table (the raw data table) being added.
	// Make column names for this temporary table unique (not the same as the column names from the previous table that is being copied)
	// These columns are from the original raw data table, which may or may not have datetime columns.
	// Further, the "current_multiplicity" of these columns is 1, and must be updated.
	//
	// Start with the primary key columns.
	std::for_each(primary_variable_group_raw_data_columns.columns_in_view.cbegin(),
				  primary_variable_group_raw_data_columns.columns_in_view.cend(), [&primary_variable_group_raw_data_columns, &result_columns, &variables_selected, &second_table_column_count, &current_multiplicity](
					  ColumnsInTempView::ColumnInTempView const & new_column_raw_data_table)
	{
		if (new_column_raw_data_table.column_type == ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__DATETIMESTART
			|| new_column_raw_data_table.column_type == ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__DATETIMESTART_INTERNAL
			|| new_column_raw_data_table.column_type == ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__DATETIMEEND
			|| new_column_raw_data_table.column_type == ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__DATETIMEEND_INTERNAL)
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
			new_column.is_within_inner_table_corresponding_to_top_level_uoa = true;
			new_column.current_multiplicity__of__current_inner_table__within__current_vg_inner_table_set = current_multiplicity;

			if (new_column.column_type == ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__PRIMARY)
			{
				if (new_column.total_outer_multiplicity__in_total_kad__for_current_dmu_category__for_current_variable_group > 1)
				{
					new_column.current_multiplicity__corresponding_to__current_inner_table___is_1_in_all_inner_tables_when_multiplicity_is_1_for_that_dmu_category_for_that_vg =
						current_multiplicity; // update current multiplicity
					new_column.primary_key_index_within_total_kad_for_dmu_category =
						new_column.primary_key_index__within_uoa_corresponding_to_variable_group_corresponding_to_current_inner_table__for_dmu_category
						+ (current_multiplicity - 1) * new_column.total_k_count__within_uoa_corresponding_to_current_variable_group__for_current_dmu_category;
				}
			}

			++second_table_column_count;
		}
	});

	// Proceed to the secondary key columns.
	std::for_each(primary_variable_group_raw_data_columns.columns_in_view.cbegin(),
				  primary_variable_group_raw_data_columns.columns_in_view.cend(), [&result_columns, &primary_variable_group_raw_data_columns, &variables_selected, &second_table_column_count, &current_multiplicity](
					  ColumnsInTempView::ColumnInTempView const & new_column_secondary)
	{

		bool make_secondary_datetime_column = false;

		if (new_column_secondary.column_type == ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__DATETIMESTART
			|| new_column_secondary.column_type == ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__DATETIMEEND)
		{
			// No!  If the user selects these columns, they should appear as regular secondary key columns.  Change the column type in this case to "secondary".
			//return; // Add these columns last
			make_secondary_datetime_column = true;
		}

		if (!make_secondary_datetime_column && new_column_secondary.column_type != ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__SECONDARY)
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
			new_column.is_within_inner_table_corresponding_to_top_level_uoa = true;
			new_column.current_multiplicity__of__current_inner_table__within__current_vg_inner_table_set = current_multiplicity;

			if (new_column.column_type == ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__PRIMARY)
			{
				if (new_column.total_outer_multiplicity__in_total_kad__for_current_dmu_category__for_current_variable_group > 1)
				{
					new_column.current_multiplicity__corresponding_to__current_inner_table___is_1_in_all_inner_tables_when_multiplicity_is_1_for_that_dmu_category_for_that_vg =
						current_multiplicity; // update current multiplicity
					new_column.primary_key_index_within_total_kad_for_dmu_category =
						new_column.primary_key_index__within_uoa_corresponding_to_variable_group_corresponding_to_current_inner_table__for_dmu_category
						+ (current_multiplicity - 1) * new_column.total_k_count__within_uoa_corresponding_to_current_variable_group__for_current_dmu_category;
				}
			}

			if (make_secondary_datetime_column)
			{
				new_column.column_type = ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__SECONDARY;
				new_column.originally_datetime = true;
			}

			++second_table_column_count;
		}
	});

	// Proceed, finally, to the datetime columns, if they exist.  (If they don't, they will be added via ALTER TABLE to the temporary table under construction.)
	int most_recent_current_inner_table_count = -1;
	std::for_each(primary_variable_group_raw_data_columns.columns_in_view.cbegin(),
				  primary_variable_group_raw_data_columns.columns_in_view.cend(), [&most_recent_current_inner_table_count, &primary_variable_group_raw_data_columns, &result_columns, &second_table_column_count](
					  ColumnsInTempView::ColumnInTempView const & new_column_datetime)
	{
		if (new_column_datetime.current_multiplicity__of__current_inner_table__within__current_vg_inner_table_set > 0)
		{
			most_recent_current_inner_table_count = new_column_datetime.current_multiplicity__of__current_inner_table__within__current_vg_inner_table_set;
		}

		if (new_column_datetime.column_type == ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__DATETIMESTART
			|| new_column_datetime.column_type == ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__DATETIMESTART_INTERNAL)
		{
			result_columns.columns_in_view.push_back(new_column_datetime);
			ColumnsInTempView::ColumnInTempView & new_column = result_columns.columns_in_view.back();
			new_column.column_name_in_temporary_table = new_column.column_name_in_temporary_table_no_uuid;
			new_column.column_name_in_temporary_table += "_";
			new_column.column_name_in_temporary_table += newUUID(true);
			new_column.is_within_inner_table_corresponding_to_top_level_uoa = true;
			new_column.current_multiplicity__of__current_inner_table__within__current_vg_inner_table_set = most_recent_current_inner_table_count;
			++second_table_column_count;
		}
	});

	most_recent_current_inner_table_count = -1;
	std::for_each(primary_variable_group_raw_data_columns.columns_in_view.cbegin(),
				  primary_variable_group_raw_data_columns.columns_in_view.cend(), [&most_recent_current_inner_table_count, &result_columns, &second_table_column_count](
					  ColumnsInTempView::ColumnInTempView const & new_column_datetime)
	{
		if (new_column_datetime.current_multiplicity__of__current_inner_table__within__current_vg_inner_table_set > 0)
		{
			most_recent_current_inner_table_count = new_column_datetime.current_multiplicity__of__current_inner_table__within__current_vg_inner_table_set;
		}

		if (new_column_datetime.column_type == ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__DATETIMEEND
			|| new_column_datetime.column_type == ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__DATETIMEEND_INTERNAL)
		{
			result_columns.columns_in_view.push_back(new_column_datetime);
			ColumnsInTempView::ColumnInTempView & new_column = result_columns.columns_in_view.back();
			new_column.column_name_in_temporary_table = new_column.column_name_in_temporary_table_no_uuid;
			new_column.column_name_in_temporary_table += "_";
			new_column.column_name_in_temporary_table += newUUID(true);
			new_column.is_within_inner_table_corresponding_to_top_level_uoa = true;
			new_column.current_multiplicity__of__current_inner_table__within__current_vg_inner_table_set = most_recent_current_inner_table_count;
			++second_table_column_count;
		}
	});

	sql_strings.push_back(SQLExecutor(this, db));
	std::string & sql_string = sql_strings.back().sql;

	sql_string = "CREATE TABLE \"";
	sql_string += result_columns.view_name;
	sql_string += "\" AS SELECT ";
	first = true;
	int column_count = 0;
	std::for_each(result_columns.columns_in_view.begin(),
				  result_columns.columns_in_view.end(), [&sql_string, &first, &column_count, &first_full_table_column_count, &second_table_column_count, &previous_column_names_first_table](
					  ColumnsInTempView::ColumnInTempView & new_column)
	{
		if (!first)
		{
			sql_string += ", ";
		}

		first = false;

		if (new_column.column_type == ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__PRIMARY && new_column.primary_key_should_be_treated_as_integer_____float_not_allowed_as_primary_key)
		{
			sql_string += "CAST (";
		}

		if (column_count < first_full_table_column_count)
		{
			sql_string += "\"t1\".";
			sql_string += "`";
			sql_string += previous_column_names_first_table[column_count];
			sql_string += "`";
		}
		else
		{
			sql_string += "\"t2\".";
			sql_string += "`";
			sql_string += new_column.column_name_in_temporary_table_no_uuid; // This is the original column name
			sql_string += "`";
		}

		if (new_column.column_type == ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__PRIMARY && new_column.primary_key_should_be_treated_as_integer_____float_not_allowed_as_primary_key)
		{
			sql_string += " AS INTEGER)";
		}

		sql_string += " AS ";
		sql_string += "`";
		sql_string += new_column.column_name_in_temporary_table;
		sql_string += "`";
		++column_count;
	});
	sql_string += " FROM \"";
	sql_string += previous_xr_columns.view_name;
	sql_string += "\" t1 LEFT OUTER JOIN \"";
	sql_string += primary_variable_group_raw_data_columns.original_table_names[0];
	sql_string += "\" t2 ";
	bool and_ = false;




	// ********************************************************************************************* //
	// See corresponding block in CreateChildXTable() for helpful comments
	// ********************************************************************************************* //

	std::vector<int> column_indices_primary_keys_multiplicity_greater_than_1_to_check;
	std::vector<int> column_indices_final_previous_inner_table_primary_keys_multiplicity_greater_than_1;
	std::vector<int> column_indices_current_inner_table_primary_keys_multiplicity_greater_than_1;
	int biggest_multiplicity_previous_table = 0;
	std::for_each(sequence.primary_key_sequence_info.cbegin(),
				  sequence.primary_key_sequence_info.cend(), [this, &column_indices_current_inner_table_primary_keys_multiplicity_greater_than_1, &biggest_multiplicity_previous_table, &column_indices_final_previous_inner_table_primary_keys_multiplicity_greater_than_1, &column_indices_primary_keys_multiplicity_greater_than_1_to_check, &previous_xr_columns, &sql_string, &variable_group, &result_columns, &first_full_table_column_count, &second_table_column_count, &previous_column_names_first_table, &and_](
					  PrimaryKeySequence::PrimaryKeySequenceEntry const & primary_key)
	{
		std::for_each(primary_key.variable_group_info_for_primary_keys.cbegin(),
					  primary_key.variable_group_info_for_primary_keys.cend(), [this, &column_indices_current_inner_table_primary_keys_multiplicity_greater_than_1, &biggest_multiplicity_previous_table, &column_indices_final_previous_inner_table_primary_keys_multiplicity_greater_than_1, &column_indices_primary_keys_multiplicity_greater_than_1_to_check, &previous_xr_columns, &sql_string, &variable_group, &primary_key, &result_columns, &first_full_table_column_count, &second_table_column_count, &previous_column_names_first_table, &and_](
						  PrimaryKeySequence::VariableGroup_PrimaryKey_Info const & primary_key_info)
		{
			if (primary_key_info.vg_identifier.IsEqual(WidgetInstanceIdentifier::EQUALITY_CHECK_TYPE__STRING_CODE, variable_group))
			{
				// Only join on primary keys whose total multiplicity is 1
				if (primary_key_info.total_outer_multiplicity__in_total_kad__for_current_dmu_category__for_current_variable_group == 1)
				{
					int column_count = 0;
					std::for_each(result_columns.columns_in_view.cbegin(),
								  result_columns.columns_in_view.cend(), [this, &sql_string, &first_full_table_column_count, &second_table_column_count, &column_count, &previous_column_names_first_table, &primary_key, &and_](
									  ColumnsInTempView::ColumnInTempView const & new_column)
					{
						if (column_count < inner_table_no_multiplicities__with_all_datetime_columns_included__column_count)
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
									else
									{
										sql_string += " ON ";
									}

									and_ = true;
									sql_string += "\"t1\".`";
									sql_string += previous_column_names_first_table[column_count];
									sql_string += "` = \"t2\".`";
									sql_string += new_column.column_name_in_original_data_table;
									sql_string += "`";
								}
							}
						}

						++column_count;
					});
				}
				else
				{
					// save indices of multiplicity > 1 keys for the join, below
					int column_count = 0;
					std::for_each(result_columns.columns_in_view.cbegin(),
								  result_columns.columns_in_view.cend(), [this, &primary_key_info, &column_indices_current_inner_table_primary_keys_multiplicity_greater_than_1, &biggest_multiplicity_previous_table, &column_indices_final_previous_inner_table_primary_keys_multiplicity_greater_than_1, &column_indices_primary_keys_multiplicity_greater_than_1_to_check, &previous_xr_columns, &sql_string, &first_full_table_column_count, &second_table_column_count, &column_count, &previous_column_names_first_table, &primary_key, &and_](
									  ColumnsInTempView::ColumnInTempView const & new_column)
					{
						if (new_column.current_multiplicity__corresponding_to__current_inner_table___is_1_in_all_inner_tables_when_multiplicity_is_1_for_that_dmu_category_for_that_vg ==
							primary_key_info.current_outer_multiplicity_of_this_primary_key__in_relation_to__the_uoa_corresponding_to_the_current_variable_group___same_as___current_inner_table_number_within_the_inner_table_set_corresponding_to_the_current_variable_group)
						{
							if (column_count < (int)previous_xr_columns.columns_in_view.size())
							{
								if (new_column.primary_key_dmu_category_identifier.IsEqual(WidgetInstanceIdentifier::EQUALITY_CHECK_TYPE__STRING_CODE, primary_key.dmu_category))
								{
									if (new_column.primary_key_index_within_primary_uoa_for_dmu_category == primary_key.sequence_number_within_dmu_category_primary_uoa)
									{
										column_indices_primary_keys_multiplicity_greater_than_1_to_check.push_back(column_count);

										if (new_column.current_multiplicity__corresponding_to__current_inner_table___is_1_in_all_inner_tables_when_multiplicity_is_1_for_that_dmu_category_for_that_vg >
											biggest_multiplicity_previous_table)
										{
											biggest_multiplicity_previous_table =
												new_column.current_multiplicity__corresponding_to__current_inner_table___is_1_in_all_inner_tables_when_multiplicity_is_1_for_that_dmu_category_for_that_vg;
											column_indices_final_previous_inner_table_primary_keys_multiplicity_greater_than_1.clear();
										}

										column_indices_final_previous_inner_table_primary_keys_multiplicity_greater_than_1.push_back(column_count);
									}
								}
							}
							else
							{
								if (new_column.primary_key_dmu_category_identifier.IsEqual(WidgetInstanceIdentifier::EQUALITY_CHECK_TYPE__STRING_CODE, primary_key.dmu_category))
								{
									if (new_column.primary_key_index_within_primary_uoa_for_dmu_category == primary_key.sequence_number_within_dmu_category_primary_uoa)
									{
										column_indices_current_inner_table_primary_keys_multiplicity_greater_than_1.push_back(column_count);
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

	// For use in both the WHERE and ORDER BY clauses
	// Determine how many columns there are corresponding to the DMU category with multiplicity greater than 1
	int number_primary_key_columns_in_dmu_category_with_multiplicity_greater_than_1 = 0;

	if (debug_ordering)
	{
		if (highest_multiplicity_primary_uoa > 1)
		{
			std::for_each(result_columns.columns_in_view.begin(),
						  result_columns.columns_in_view.end(), [this, &number_primary_key_columns_in_dmu_category_with_multiplicity_greater_than_1, &sql_string](
							  ColumnsInTempView::ColumnInTempView & view_column)
			{
				if (view_column.column_type == ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__PRIMARY)
				{
					if (view_column.is_within_inner_table_corresponding_to_top_level_uoa)
					{
						if (view_column.total_outer_multiplicity__in_total_kad__for_current_dmu_category__for_current_variable_group == highest_multiplicity_primary_uoa)
						{
							if (view_column.current_multiplicity__corresponding_to__current_inner_table___is_1_in_all_inner_tables_when_multiplicity_is_1_for_that_dmu_category_for_that_vg == 1)
							{
								++number_primary_key_columns_in_dmu_category_with_multiplicity_greater_than_1;
							}
						}
					}
				}
			});
		}
	}

	if (true)
	{
		if (!primary_variable_group_raw_data_columns.has_no_datetime_columns_originally)
		{
			// ************************************************************************************************** //
			// Basic timerange check against timerange selected by end user
			// ************************************************************************************************** //
			if (and_)
			{
				sql_string += " AND ";
			}
			else
			{
				sql_string += " ON ";
			}

			and_ = true;

			sql_string += " CASE WHEN `";
			sql_string += result_columns.columns_in_view[result_columns.columns_in_view.size() - 2].column_name_in_temporary_table;
			sql_string += "` = 0 AND `";
			sql_string += result_columns.columns_in_view[result_columns.columns_in_view.size() - 1].column_name_in_temporary_table;
			sql_string += "` = 0 ";
			sql_string += " THEN 1 ";
			sql_string += " WHEN `";
			sql_string += result_columns.columns_in_view[result_columns.columns_in_view.size() - 2].column_name_in_temporary_table;
			sql_string += "` < ";
			sql_string += boost::lexical_cast<std::string>(timerange_end);
			sql_string += " THEN `";
			sql_string += result_columns.columns_in_view[result_columns.columns_in_view.size() - 1].column_name_in_temporary_table;
			sql_string += "` > ";
			sql_string += boost::lexical_cast<std::string>(timerange_start);
			sql_string += " WHEN `";
			sql_string += result_columns.columns_in_view[result_columns.columns_in_view.size() - 1].column_name_in_temporary_table;
			sql_string += "` > ";
			sql_string += boost::lexical_cast<std::string>(timerange_start);
			sql_string += " THEN `";
			sql_string += result_columns.columns_in_view[result_columns.columns_in_view.size() - 2].column_name_in_temporary_table;
			sql_string += "` < ";
			sql_string += boost::lexical_cast<std::string>(timerange_end);
			sql_string += " ELSE 0";
			sql_string += " END";





			// ************************************************************************************************** //
			// Timerange check against timerange in previous column blocks
			// (if it fails here, it means the new data was already included
			// in a previous stage in a previous inner table)
			// ************************************************************************************************** //
			if (and_)
			{
				sql_string += " AND ";
			}
			else
			{
				sql_string += " ON ";
			}

			and_ = true;

			sql_string += " CASE WHEN `";
			sql_string += result_columns.columns_in_view[result_columns.columns_in_view.size() - 2].column_name_in_temporary_table;
			sql_string += "` = 0 AND `";
			sql_string += result_columns.columns_in_view[result_columns.columns_in_view.size() - 1].column_name_in_temporary_table;
			sql_string += "` = 0 ";
			sql_string += " THEN 1 ";
			sql_string += " WHEN `";
			sql_string += result_columns.columns_in_view[result_columns.columns_in_view.size() - 2].column_name_in_temporary_table;
			sql_string += "` < ";
			sql_string += boost::lexical_cast<std::string>(timerange_end);
			sql_string += " THEN `";
			sql_string += result_columns.columns_in_view[result_columns.columns_in_view.size() - 1].column_name_in_temporary_table;
			sql_string += "` > ";
			sql_string += boost::lexical_cast<std::string>(timerange_start);
			sql_string += " WHEN `";
			sql_string += result_columns.columns_in_view[result_columns.columns_in_view.size() - 1].column_name_in_temporary_table;
			sql_string += "` > ";
			sql_string += boost::lexical_cast<std::string>(timerange_start);
			sql_string += " THEN `";
			sql_string += result_columns.columns_in_view[result_columns.columns_in_view.size() - 2].column_name_in_temporary_table;
			sql_string += "` < ";
			sql_string += boost::lexical_cast<std::string>(timerange_end);
			sql_string += " ELSE 0";
			sql_string += " END";



			// ************************************************************************************************** //
			// Now, the test to make sure that if one or more previous multiplicity > 1 columns is NULL,
			// that this row gets automatically included (because it's an OUTER join) even though new data
			// does not match; OR test that the new data is greater than the previous final piece of data
			// (in the multiplicity > 1 columns) to maintain orderedness within rows
			// ************************************************************************************************** //
			if (and_)
			{
				sql_string += " AND ";
			}
			else
			{
				sql_string += " ON ";
			}

			and_ = true;

			sql_string += " CASE WHEN ";
			bool second_and = false;
			std::for_each(column_indices_primary_keys_multiplicity_greater_than_1_to_check.cbegin(),
						  column_indices_primary_keys_multiplicity_greater_than_1_to_check.cend(), [&](int const & column_index_test)
			{
				if (second_and)
				{
					sql_string += " OR ";
				}

				second_and = true;
				sql_string += "\"t1\".`";
				sql_string += previous_column_names_first_table[column_index_test];
				sql_string += "` IS NULL ";
			});
			sql_string += " THEN 0 ELSE ";
			second_and = false;
			int current_inner_multiplicity_count = 0;
			sql_string += " CASE ";
			std::for_each(column_indices_final_previous_inner_table_primary_keys_multiplicity_greater_than_1.cbegin(),
						  column_indices_final_previous_inner_table_primary_keys_multiplicity_greater_than_1.cend(), [&](int const & column_index_test)
			{
				// ************************************************************************************************** //
				// ************************************************************************************************** //
				//
				// TODO! CAST AS NUMERIC IF NEEDED
				//
				// ************************************************************************************************** //
				// ************************************************************************************************** //
				sql_string += " WHEN \"t1\".`";
				sql_string += previous_column_names_first_table[column_index_test];
				sql_string += "` < ";
				sql_string += " \"t2\".`";
				sql_string +=
					result_columns.columns_in_view[column_indices_current_inner_table_primary_keys_multiplicity_greater_than_1[current_inner_multiplicity_count]].column_name_in_original_data_table;
				sql_string += "` THEN 1 ";
				++current_inner_multiplicity_count;
				// ************************************************************************************************** //
				// ************************************************************************************************** //
				//
				// TODO! CAST AS NUMERIC IF NEEDED
				//
				// ************************************************************************************************** //
				// ************************************************************************************************** //
			});
			sql_string += " ELSE 0 END END";
		}
	}
	else  // moved into join, so else clause never reached
	{
		if (!primary_variable_group_raw_data_columns.has_no_datetime_columns_originally)
		{
			sql_string += " WHERE CASE WHEN `";
			sql_string += result_columns.columns_in_view[result_columns.columns_in_view.size() - 2].column_name_in_temporary_table;
			sql_string += "` = 0 AND `";
			sql_string += result_columns.columns_in_view[result_columns.columns_in_view.size() - 1].column_name_in_temporary_table;
			sql_string += "` = 0 ";
			sql_string += " THEN 1 ";
			sql_string += " WHEN `";
			sql_string += result_columns.columns_in_view[result_columns.columns_in_view.size() - 2].column_name_in_temporary_table;
			sql_string += "` < ";
			sql_string += boost::lexical_cast<std::string>(timerange_end);
			sql_string += " THEN `";
			sql_string += result_columns.columns_in_view[result_columns.columns_in_view.size() - 1].column_name_in_temporary_table;
			sql_string += "` > ";
			sql_string += boost::lexical_cast<std::string>(timerange_start);
			sql_string += " WHEN `";
			sql_string += result_columns.columns_in_view[result_columns.columns_in_view.size() - 1].column_name_in_temporary_table;
			sql_string += "` > ";
			sql_string += boost::lexical_cast<std::string>(timerange_start);
			sql_string += " THEN `";
			sql_string += result_columns.columns_in_view[result_columns.columns_in_view.size() - 2].column_name_in_temporary_table;
			sql_string += "` < ";
			sql_string += boost::lexical_cast<std::string>(timerange_end);
			sql_string += " ELSE 0";
			sql_string += " END";
		}
	}

	// Add the ORDER BY column/s
	if (debug_ordering)
		//if (true)
	{

		bool first = true;

		// Now order by remaining primary key columns (with multiplicity 1)
		// ... If there are no primary key DMU categories for this top-level UOA with multiplicity greater than 1,
		// then this section will order by all of this top-level's UOA primary key DMU categories.
		int current_column = 0;
		std::for_each(result_columns.columns_in_view.begin(), result_columns.columns_in_view.end(), [this, &sql_string, &result_columns, &current_column, &first](
						  ColumnsInTempView::ColumnInTempView & view_column)
		{

			if (current_column >= inner_table_no_multiplicities__with_all_datetime_columns_included__column_count)
			{
				return;
			}

			// Determine how many columns there are corresponding to the DMU category
			int number_primary_key_columns_in_dmu_category_with_multiplicity_of_1 = 0;
			int column_count_nested = 0;
			std::for_each(result_columns.columns_in_view.begin(),
						  result_columns.columns_in_view.end(), [this, &view_column, &column_count_nested, &number_primary_key_columns_in_dmu_category_with_multiplicity_of_1, &sql_string](
							  ColumnsInTempView::ColumnInTempView & view_column_nested)
			{
				if (column_count_nested >= inner_table_no_multiplicities__with_all_datetime_columns_included__column_count)
				{
					return;
				}

				if (view_column_nested.column_type == ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__PRIMARY)
				{
					if (view_column_nested.primary_key_dmu_category_identifier.IsEqual(WidgetInstanceIdentifier::EQUALITY_CHECK_TYPE__STRING_CODE, view_column.primary_key_dmu_category_identifier))
					{
						if (view_column_nested.total_outer_multiplicity__in_total_kad__for_current_dmu_category__for_current_variable_group == 1)
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
				if (view_column.total_outer_multiplicity__in_total_kad__for_current_dmu_category__for_current_variable_group == 1)
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

								if (view_column.primary_key_should_be_treated_as_integer_____float_not_allowed_as_primary_key)
								{
									sql_string += "CAST (";
								}

								sql_string += "`";
								sql_string += view_column.column_name_in_temporary_table;
								sql_string += "`";

								if (view_column.primary_key_should_be_treated_as_integer_____float_not_allowed_as_primary_key)
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

		if (highest_multiplicity_primary_uoa > 1)
		{

			// Create the ORDER BY clause, taking the proper primary key columns that compose the DMU category with multiplicity greater than 1, in sequence
			for (int outer_dmu_multiplicity = 1; outer_dmu_multiplicity <= highest_multiplicity_primary_uoa; ++outer_dmu_multiplicity)
			{
				for (int inner_dmu_multiplicity = 0; inner_dmu_multiplicity < number_primary_key_columns_in_dmu_category_with_multiplicity_greater_than_1; ++inner_dmu_multiplicity)
				{
					std::for_each(result_columns.columns_in_view.begin(), result_columns.columns_in_view.end(), [this, &inner_dmu_multiplicity, &outer_dmu_multiplicity, &sql_string, &first](
									  ColumnsInTempView::ColumnInTempView & view_column)
					{
						if (view_column.column_type == ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__PRIMARY)
						{
							if (view_column.total_outer_multiplicity__in_total_kad__for_current_dmu_category__for_current_variable_group == highest_multiplicity_primary_uoa)
							{
								if (view_column.primary_key_index__within_uoa_corresponding_to_variable_group_corresponding_to_current_inner_table__for_dmu_category == inner_dmu_multiplicity)
								{
									if (view_column.current_multiplicity__corresponding_to__current_inner_table___is_1_in_all_inner_tables_when_multiplicity_is_1_for_that_dmu_category_for_that_vg ==
										outer_dmu_multiplicity)
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

											if (view_column.primary_key_should_be_treated_as_integer_____float_not_allowed_as_primary_key)
											{
												sql_string += "CAST (";
											}

											sql_string += "`";
											sql_string += view_column.column_name_in_temporary_table;
											sql_string += "`";

											if (view_column.primary_key_should_be_treated_as_integer_____float_not_allowed_as_primary_key)
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

	}

	// SQL to add the datetime columns, if they are not present in the raw data table (filled with 0)
	if (primary_variable_group_raw_data_columns.has_no_datetime_columns_originally)
	{
		std::string datetime_start_col_name_no_uuid = Table_VariableGroupMetadata_DateTimeColumns::DefaultDatetimeStartColumnName;
		std::string datetime_start_col_name = datetime_start_col_name_no_uuid;
		datetime_start_col_name += "_";
		datetime_start_col_name += newUUID(true);

		std::string alter_string;
		alter_string += "ALTER TABLE \"";
		alter_string += result_columns.view_name;
		alter_string += "\" ADD COLUMN ";
		alter_string += datetime_start_col_name;
		alter_string += " INTEGER DEFAULT 0";
		sql_strings.push_back(SQLExecutor(this, db, alter_string));

		if (failed)
		{
			SetFailureMessage(sql_error);
			return result;
		}

		if (CheckCancelled())
		{
			return result;
		}

		result_columns.columns_in_view.push_back(ColumnsInTempView::ColumnInTempView());
		ColumnsInTempView::ColumnInTempView & datetime_start_column = result_columns.columns_in_view.back();
		datetime_start_column.column_name_in_temporary_table = datetime_start_col_name;
		datetime_start_column.column_name_in_temporary_table_no_uuid = datetime_start_col_name_no_uuid;
		datetime_start_column.column_type = ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__DATETIMESTART_INTERNAL;
		datetime_start_column.variable_group_associated_with_current_inner_table = variable_group;
		datetime_start_column.uoa_associated_with_variable_group_associated_with_current_inner_table = uoa;
		datetime_start_column.column_name_in_original_data_table = "";
		datetime_start_column.is_within_inner_table_corresponding_to_top_level_uoa = true;
		datetime_start_column.current_multiplicity__of__current_inner_table__within__current_vg_inner_table_set =
			primary_variable_group_raw_data_columns.columns_in_view.back().current_multiplicity__of__current_inner_table__within__current_vg_inner_table_set;
		datetime_start_column.number_inner_tables_in_set = primary_variable_group_raw_data_columns.columns_in_view.back().number_inner_tables_in_set;

		std::string datetime_end_col_name_no_uuid = Table_VariableGroupMetadata_DateTimeColumns::DefaultDatetimeEndColumnName;
		std::string datetime_end_col_name = datetime_end_col_name_no_uuid;
		datetime_end_col_name += "_";
		datetime_end_col_name += newUUID(true);

		alter_string.clear();
		alter_string += "ALTER TABLE \"";
		alter_string += result_columns.view_name;
		alter_string += "\" ADD COLUMN ";
		alter_string += datetime_end_col_name;
		alter_string += " INTEGER DEFAULT 0";
		sql_strings.push_back(SQLExecutor(this, db, alter_string));

		if (failed)
		{
			SetFailureMessage(sql_error);
			return result;
		}

		if (CheckCancelled())
		{
			return result;
		}

		result_columns.columns_in_view.push_back(ColumnsInTempView::ColumnInTempView());
		ColumnsInTempView::ColumnInTempView & datetime_end_column = result_columns.columns_in_view.back();
		datetime_end_column.column_name_in_temporary_table = datetime_end_col_name;
		datetime_end_column.column_name_in_temporary_table_no_uuid = datetime_end_col_name_no_uuid;
		datetime_end_column.column_type = ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__DATETIMEEND_INTERNAL;
		datetime_end_column.variable_group_associated_with_current_inner_table = variable_group;
		datetime_end_column.uoa_associated_with_variable_group_associated_with_current_inner_table = uoa;
		datetime_end_column.column_name_in_original_data_table = "";
		datetime_end_column.is_within_inner_table_corresponding_to_top_level_uoa = true;
		datetime_end_column.current_multiplicity__of__current_inner_table__within__current_vg_inner_table_set =
			primary_variable_group_raw_data_columns.columns_in_view.back().current_multiplicity__of__current_inner_table__within__current_vg_inner_table_set;
		datetime_end_column.number_inner_tables_in_set = primary_variable_group_raw_data_columns.columns_in_view.back().number_inner_tables_in_set;

		result_columns.current_block_datetime_column_types = std::make_pair(ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__DATETIMESTART_INTERNAL,
				ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__DATETIMEEND_INTERNAL);
	}
	else
	{
		result_columns.current_block_datetime_column_types = std::make_pair(ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__DATETIMESTART,
				ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__DATETIMEEND);
	}

	return result;

}

bool OutputModel::OutputGenerator::CreateNewXRRow(SavedRowData const & current_row_of_data, bool & first_row_added, std::string const & datetime_start_col_name,
		std::string const & datetime_end_col_name, std::string const & xr_view_name, std::string & sql_add_xr_row, std::vector<std::string> & bound_parameter_strings,
		std::vector<std::int64_t> & bound_parameter_ints, std::vector<long double> & bound_parameter_floats, std::vector<SQLExecutor::WHICH_BINDING> & bound_parameter_which_binding_to_use,
		std::int64_t const datetime_start, std::int64_t const datetime_end, ColumnsInTempView const & previous_x_or_mergedfinalplusnewfinal_columns,
		ColumnsInTempView & current_xr_or_completemerge_columns, bool const include_previous_data, bool const include_current_data, XR_TABLE_CATEGORY const xr_table_category,
		bool const sort_only, bool const no_new_column_names)
{

	// ********************************************************************************** //
	//
	// Compare the following method to that used in MergeRows().
	//
	// In this method, we copy the inner table data into a new, local cache
	// (actually, a vector of them),
	// and then when we need the data we reference the local cache vector by index.
	//
	// In MergeRows(), we save the indices of the original location of the data
	// in the incoming SavedRowData objects (current_row__map_from__inner_multiplicity_string_vector__to__inner_table_number),
	// and simply use those indices when we need to retrieve the data
	// for a certain inner table (current_inner_table_index_offset).
	//
	// ********************************************************************************** //

	if (include_previous_data == false && include_current_data == false)
	{
		return false;
	}

	if (xr_table_category == XR_TABLE_CATEGORY::PRIMARY_VARIABLE_GROUP)
	{
		if (include_previous_data == false && include_current_data == true)
		{
			return false;
		}
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

		sql_add_xr_row += "INSERT OR FAIL INTO \"";
		sql_add_xr_row += xr_view_name;
		sql_add_xr_row += "\" (";

		bool first_column_name = true;
		int the_index = 0;
		std::for_each(previous_x_or_mergedfinalplusnewfinal_columns.columns_in_view.cbegin(),
					  previous_x_or_mergedfinalplusnewfinal_columns.columns_in_view.cend(), [&the_index, &current_xr_or_completemerge_columns, &first_column_name, &sql_add_xr_row, &bound_parameter_strings, &bound_parameter_ints, &bound_parameter_which_binding_to_use, &include_previous_data, &include_current_data](
						  ColumnsInTempView::ColumnInTempView const & column_in_view)
		{

			if (!first_column_name)
			{
				sql_add_xr_row += ", ";
			}

			first_column_name = false;

			sql_add_xr_row += "`";
			sql_add_xr_row += current_xr_or_completemerge_columns.columns_in_view[the_index].column_name_in_temporary_table;
			sql_add_xr_row += "`";

			++the_index;

		});

		if (!no_new_column_names)
		{
			// The two new "merged" time range columns
			if (!first_column_name)
			{
				sql_add_xr_row += ", ";
			}

			first_column_name = false;
			sql_add_xr_row += "`";
			sql_add_xr_row += datetime_start_col_name;
			sql_add_xr_row += "`";
			sql_add_xr_row += ", ";
			sql_add_xr_row += "`";
			sql_add_xr_row += datetime_end_col_name;
			sql_add_xr_row += "`";
		}

		sql_add_xr_row += ") VALUES (";

		int index = 1;
		char cindex[256];

		bool first_column_value = true;
		std::for_each(previous_x_or_mergedfinalplusnewfinal_columns.columns_in_view.cbegin(),
					  previous_x_or_mergedfinalplusnewfinal_columns.columns_in_view.cend(), [&first_column_value, &index, &cindex, &sql_add_xr_row, &bound_parameter_strings, &bound_parameter_ints, &bound_parameter_which_binding_to_use, &include_previous_data, &include_current_data](
						  ColumnsInTempView::ColumnInTempView const & column_in_view)
		{

			if (!first_column_value)
			{
				sql_add_xr_row += ", ";
			}

			first_column_value = false;

			sql_add_xr_row += "?";
			sql_add_xr_row += std::to_string(index);
			++index;

		});

		if (!no_new_column_names)
		{
			// The two new "merged" time range columns
			if (!first_column_value)
			{
				sql_add_xr_row += ", ";
			}

			first_column_value = false;
			sql_add_xr_row += "?";
			sql_add_xr_row += std::to_string(index);
			++index;
			sql_add_xr_row += ", ";
			sql_add_xr_row += "?";
			sql_add_xr_row += std::to_string(index);
			++index;
		}

		sql_add_xr_row += ")";

		first_row_added = false;

	}

	if (failed || CheckCancelled())
	{
		return false;
	}

	//int highest_index_previous_table = (int)previous_x_or_mergedfinalplusnewfinal_columns.columns_in_view.size() - 1;
	//bool found_highest_index = false;
	//std::for_each(previous_x_or_mergedfinalplusnewfinal_columns.columns_in_view.crbegin(), previous_x_or_mergedfinalplusnewfinal_columns.columns_in_view.crend(), [&xr_table_category, &highest_index_previous_table, &found_highest_index](ColumnsInTempView::ColumnInTempView const & column_in_view)
	//{
	//	if (found_highest_index)
	//	{
	//		return;
	//	}

	//	if (xr_table_category == OutputModel::OutputGenerator::CHILD_VARIABLE_GROUP)
	//	{
	//		if (column_in_view.column_type == ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__DATETIMEEND__POST_TIMERANGE_MERGED_BETWEEN_TOP_LEVEL_PRIMARY_VARIABLE_GROUPS
	//		 || column_in_view.column_type == ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__DATETIMEEND_CHILD_MERGE)
	//		{
	//			found_highest_index = true;
	//			return;
	//		}
	//	}
	//	else if (xr_table_category == OutputModel::OutputGenerator::FINAL_MERGE_OF_PRIMARY_VARIABLE_GROUP)
	//	{
	//		if (column_in_view.column_type == ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__DATETIMEEND__POST_TIMERANGE_MERGED_BETWEEN_TOP_LEVEL_PRIMARY_VARIABLE_GROUPS)
	//		{
	//			found_highest_index = true;
	//			return;
	//		}
	//	}
	//	else
	//	{
	//		if (column_in_view.column_type == ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__DATETIMEEND__PRIMARY_VG_INNER_TABLE_MERGE__AFTER_DUPLICATES_REMOVED
	//		 || column_in_view.column_type == ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__DATETIMEEND__TIMERANGE_MERGED_BETWEEN_TOP_LEVEL_PRIMARY_VARIABLE_GROUPS)
	//		{
	//			found_highest_index = true;
	//			return;
	//		}
	//	}

	//	--highest_index_previous_table;
	//});


	//int index_of_previous_datetime_start_column = highest_index_previous_table - 1;
	//int index_of_previous_datetime_end_column = highest_index_previous_table;
	//int index_of_current_datetime_start_column = (int)previous_x_or_mergedfinalplusnewfinal_columns.columns_in_view.size() - 2;
	//int index_of_current_datetime_end_column = (int)previous_x_or_mergedfinalplusnewfinal_columns.columns_in_view.size() - 1;;
	int index_of_previous_datetime_start_column = -1;
	int index_of_previous_datetime_end_column = -1;
	int index_of_current_datetime_start_column = -1;
	int index_of_current_datetime_end_column = -1;

	FindDatetimeIndices(previous_x_or_mergedfinalplusnewfinal_columns, index_of_previous_datetime_start_column, index_of_previous_datetime_end_column,
						index_of_current_datetime_start_column, index_of_current_datetime_end_column, xr_table_category);

	int highest_index_previous_table = index_of_previous_datetime_end_column;

	std::int64_t saved_previous_datetime_start = 0;
	std::int64_t saved_previous_datetime_end = 0;
	std::int64_t saved_current_datetime_start = 0;
	std::int64_t saved_current_datetime_end = 0;

	if (current_row_of_data.indices_of_all_columns[index_of_previous_datetime_start_column].first != SQLExecutor::NULL_BINDING)
	{
		saved_previous_datetime_start = current_row_of_data.current_parameter_ints[current_row_of_data.indices_of_all_columns[index_of_previous_datetime_start_column].second.first];
	}

	if (current_row_of_data.indices_of_all_columns[index_of_previous_datetime_end_column].first != SQLExecutor::NULL_BINDING)
	{
		saved_previous_datetime_end = current_row_of_data.current_parameter_ints[current_row_of_data.indices_of_all_columns[index_of_previous_datetime_end_column].second.first];
	}

	if (current_row_of_data.indices_of_all_columns[index_of_current_datetime_start_column].first != SQLExecutor::NULL_BINDING)
	{
		saved_current_datetime_start = current_row_of_data.current_parameter_ints[current_row_of_data.indices_of_all_columns[index_of_current_datetime_start_column].second.first];
	}

	if (current_row_of_data.indices_of_all_columns[index_of_current_datetime_end_column].first != SQLExecutor::NULL_BINDING)
	{
		saved_current_datetime_end = current_row_of_data.current_parameter_ints[current_row_of_data.indices_of_all_columns[index_of_current_datetime_end_column].second.first];
	}

	// The following block only applies to PRIMARY_VARIABLE_GROUP merges
	int number_columns_each_single_inner_table = 0;
	int the_column_index = 0;
	bool first_datetime_column_already_reached = false;
	bool stop_incrementing_single_inner_table_column_count = false;
	std::for_each(previous_x_or_mergedfinalplusnewfinal_columns.columns_in_view.cbegin(),
				  previous_x_or_mergedfinalplusnewfinal_columns.columns_in_view.cend(), [&index_of_previous_datetime_start_column, &index_of_previous_datetime_end_column, &index_of_current_datetime_start_column, &index_of_current_datetime_end_column, &first_datetime_column_already_reached, &stop_incrementing_single_inner_table_column_count, &the_column_index, &number_columns_each_single_inner_table, &highest_index_previous_table](
					  ColumnsInTempView::ColumnInTempView const & column_in_view)
	{

		if (stop_incrementing_single_inner_table_column_count)
		{
			return;
		}

		if (column_in_view.column_type != ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__PRIMARY
			&& column_in_view.column_type != ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__SECONDARY)
		{
			first_datetime_column_already_reached = true;
		}
		else
		{
			if (first_datetime_column_already_reached)
			{
				stop_incrementing_single_inner_table_column_count = true;
			}
		}

		if (!stop_incrementing_single_inner_table_column_count)
		{
			++number_columns_each_single_inner_table;
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
	long double data_float = 0.0;
	bool do_not_include_this_data = false;
	SQLExecutor::WHICH_BINDING column_data_type = SQLExecutor::UNKNOWN_BINDING;
	bound_parameter_strings.clear();
	bound_parameter_ints.clear();
	bound_parameter_floats.clear();
	bound_parameter_which_binding_to_use.clear();
	int number_nulls_to_add_at_end = 0;

	std::vector<ColumnSorter> inner_table_columns;

	int which_inner_table = 0;

	std::for_each(previous_x_or_mergedfinalplusnewfinal_columns.columns_in_view.cbegin(),
				  previous_x_or_mergedfinalplusnewfinal_columns.columns_in_view.cend(), [this, &current_row_of_data, &which_inner_table, &inner_table_columns, &swap_current_and_previous_and_set_previous_to_null, &number_nulls_to_add_at_end, &xr_table_category, &number_columns_each_single_inner_table, &highest_index_previous_table, &data_int64, &data_float, &data_string, &do_not_include_this_data, &column_data_type, &first_column_value, &index, &cindex, &sql_add_xr_row, &bound_parameter_strings, &bound_parameter_ints, &bound_parameter_floats, &bound_parameter_which_binding_to_use, &include_previous_data, &include_current_data](
					  ColumnsInTempView::ColumnInTempView const & column_in_view)
	{

		if (failed || CheckCancelled())
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
		std::vector<long double> & inner_table_float_set = inner_table_columns.back().floats;
		std::vector<std::pair<OutputModel::OutputGenerator::SQLExecutor::WHICH_BINDING, int>> & inner_table_bindings = inner_table_columns.back().bindings;
		std::vector<std::pair<OutputModel::OutputGenerator::SQLExecutor::WHICH_BINDING, int>> & inner_table__primary_keys_with_multiplicity_greater_than_one__which_binding_to_use__set =
					inner_table_columns.back().bindings__primary_keys_with_multiplicity_greater_than_1;
		std::vector<std::pair<OutputModel::OutputGenerator::SQLExecutor::WHICH_BINDING, int>> & inner_table__all_primary_keys__which_binding_to_use__set =
					inner_table_columns.back().bindings__all_primary_keys;


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
					inner_table_bindings.push_back(std::make_pair(OutputModel::OutputGenerator::SQLExecutor::NULL_BINDING, 0));

					if (column_in_view.total_outer_multiplicity__in_total_kad__for_current_dmu_category__for_current_variable_group > 1)
					{
						inner_table__primary_keys_with_multiplicity_greater_than_one__which_binding_to_use__set.push_back(std::make_pair(OutputModel::OutputGenerator::SQLExecutor::NULL_BINDING, 0));
					}

					if (column_in_view.column_type == ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__PRIMARY)
					{
						inner_table__all_primary_keys__which_binding_to_use__set.push_back(std::make_pair(OutputModel::OutputGenerator::SQLExecutor::NULL_BINDING, 0));
					}
				}
			}
			else
			{
				if (!include_current_data)
				{
					inner_table_bindings.push_back(std::make_pair(OutputModel::OutputGenerator::SQLExecutor::NULL_BINDING, 0));

					if (column_in_view.total_outer_multiplicity__in_total_kad__for_current_dmu_category__for_current_variable_group > 1)
					{
						inner_table__primary_keys_with_multiplicity_greater_than_one__which_binding_to_use__set.push_back(std::make_pair(OutputModel::OutputGenerator::SQLExecutor::NULL_BINDING, 0));
					}

					if (column_in_view.column_type == ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__PRIMARY)
					{
						inner_table__all_primary_keys__which_binding_to_use__set.push_back(std::make_pair(OutputModel::OutputGenerator::SQLExecutor::NULL_BINDING, 0));
					}
				}
			}


		}

		if (!do_not_include_this_data)
		{

			column_data_type = current_row_of_data.indices_of_all_columns[index].first;

			switch (column_data_type)
			{

				case SQLExecutor::INT64:
					{

						data_int64 = current_row_of_data.current_parameter_ints[current_row_of_data.indices_of_all_columns[index].second.first];

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

							if (column_in_view.total_outer_multiplicity__in_total_kad__for_current_dmu_category__for_current_variable_group > 1)
							{
								inner_table__primary_keys_with_multiplicity_greater_than_one__which_binding_to_use__set.push_back(std::make_pair(OutputModel::OutputGenerator::SQLExecutor::INT64,
										(int)inner_table_int_set.size() - 1));
							}

							if (column_in_view.column_type == ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__PRIMARY)
							{
								inner_table__all_primary_keys__which_binding_to_use__set.push_back(std::make_pair(OutputModel::OutputGenerator::SQLExecutor::INT64, (int)inner_table_int_set.size() - 1));
							}
						}
						else
						{
							if (index > highest_index_previous_table)
							{
								inner_table_int_set.push_back(data_int64);
								inner_table_bindings.push_back(std::make_pair(OutputModel::OutputGenerator::SQLExecutor::INT64, (int)inner_table_int_set.size() - 1));

								if (column_in_view.total_outer_multiplicity__in_total_kad__for_current_dmu_category__for_current_variable_group > 1)
								{
									inner_table__primary_keys_with_multiplicity_greater_than_one__which_binding_to_use__set.push_back(std::make_pair(OutputModel::OutputGenerator::SQLExecutor::INT64,
											(int)inner_table_int_set.size() - 1));
								}

								if (column_in_view.column_type == ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__PRIMARY)
								{
									inner_table__all_primary_keys__which_binding_to_use__set.push_back(std::make_pair(OutputModel::OutputGenerator::SQLExecutor::INT64, (int)inner_table_int_set.size() - 1));
								}
							}
						}

					}
					break;

				case SQLExecutor::FLOAT:
					{

						data_float = current_row_of_data.current_parameter_floats[current_row_of_data.indices_of_all_columns[index].second.first];

						// ... just populate the single data structures that hold all data across all inner tables
						// ... (including possible null data for the newly added columns)
						if (!swap_current_and_previous_and_set_previous_to_null)
						{
							bound_parameter_floats.push_back(data_float);
							bound_parameter_which_binding_to_use.push_back(SQLExecutor::FLOAT);
						}
						else
						{
							if (index > highest_index_previous_table)
							{
								bound_parameter_floats.push_back(data_float);
								bound_parameter_which_binding_to_use.push_back(SQLExecutor::FLOAT);
							}
						}

						// The following if/else block only applies to PRIMARY_VARIABLE_GROUP merges
						// Now, also save the data into sets broken down by inner table
						if (!swap_current_and_previous_and_set_previous_to_null)
						{
							inner_table_float_set.push_back(data_float);
							inner_table_bindings.push_back(std::make_pair(OutputModel::OutputGenerator::SQLExecutor::FLOAT, (int)inner_table_float_set.size() - 1));

							if (column_in_view.total_outer_multiplicity__in_total_kad__for_current_dmu_category__for_current_variable_group > 1)
							{
								inner_table__primary_keys_with_multiplicity_greater_than_one__which_binding_to_use__set.push_back(std::make_pair(OutputModel::OutputGenerator::SQLExecutor::FLOAT,
										(int)inner_table_float_set.size() - 1));
							}

							if (column_in_view.column_type == ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__PRIMARY)
							{
								inner_table__all_primary_keys__which_binding_to_use__set.push_back(std::make_pair(OutputModel::OutputGenerator::SQLExecutor::FLOAT, (int)inner_table_float_set.size() - 1));
							}
						}
						else
						{
							if (index > highest_index_previous_table)
							{
								inner_table_float_set.push_back(data_float);
								inner_table_bindings.push_back(std::make_pair(OutputModel::OutputGenerator::SQLExecutor::FLOAT, (int)inner_table_float_set.size() - 1));

								if (column_in_view.total_outer_multiplicity__in_total_kad__for_current_dmu_category__for_current_variable_group > 1)
								{
									inner_table__primary_keys_with_multiplicity_greater_than_one__which_binding_to_use__set.push_back(std::make_pair(OutputModel::OutputGenerator::SQLExecutor::FLOAT,
											(int)inner_table_float_set.size() - 1));
								}

								if (column_in_view.column_type == ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__PRIMARY)
								{
									inner_table__all_primary_keys__which_binding_to_use__set.push_back(std::make_pair(OutputModel::OutputGenerator::SQLExecutor::FLOAT, (int)inner_table_float_set.size() - 1));
								}
							}
						}

					}
					break;

				case SQLExecutor::STRING:
					{

						data_string = current_row_of_data.current_parameter_strings[current_row_of_data.indices_of_all_columns[index].second.first];

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

							if (column_in_view.total_outer_multiplicity__in_total_kad__for_current_dmu_category__for_current_variable_group > 1)
							{
								inner_table__primary_keys_with_multiplicity_greater_than_one__which_binding_to_use__set.push_back(std::make_pair(OutputModel::OutputGenerator::SQLExecutor::STRING,
										(int)inner_table_string_set.size() - 1));
							}

							if (column_in_view.column_type == ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__PRIMARY)
							{
								inner_table__all_primary_keys__which_binding_to_use__set.push_back(std::make_pair(OutputModel::OutputGenerator::SQLExecutor::STRING, (int)inner_table_string_set.size() - 1));
							}
						}
						else
						{
							if (index > highest_index_previous_table)
							{
								inner_table_string_set.push_back(data_string);
								inner_table_bindings.push_back(std::make_pair(OutputModel::OutputGenerator::SQLExecutor::STRING, (int)inner_table_string_set.size() - 1));

								if (column_in_view.total_outer_multiplicity__in_total_kad__for_current_dmu_category__for_current_variable_group > 1)
								{
									inner_table__primary_keys_with_multiplicity_greater_than_one__which_binding_to_use__set.push_back(std::make_pair(OutputModel::OutputGenerator::SQLExecutor::STRING,
											(int)inner_table_string_set.size() - 1));
								}

								if (column_in_view.column_type == ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__PRIMARY)
								{
									inner_table__all_primary_keys__which_binding_to_use__set.push_back(std::make_pair(OutputModel::OutputGenerator::SQLExecutor::STRING, (int)inner_table_string_set.size() - 1));
								}
							}
						}

					}
					break;

				case SQLExecutor::NULL_BINDING:
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

							if (column_in_view.total_outer_multiplicity__in_total_kad__for_current_dmu_category__for_current_variable_group > 1)
							{
								inner_table__primary_keys_with_multiplicity_greater_than_one__which_binding_to_use__set.push_back(std::make_pair(OutputModel::OutputGenerator::SQLExecutor::NULL_BINDING, 0));
							}

							if (column_in_view.column_type == ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__PRIMARY)
							{
								inner_table__all_primary_keys__which_binding_to_use__set.push_back(std::make_pair(OutputModel::OutputGenerator::SQLExecutor::NULL_BINDING, 0));
							}
						}
						else
						{
							if (index > highest_index_previous_table)
							{
								// unused... confirm that this can be removed
								inner_table_bindings.push_back(std::make_pair(OutputModel::OutputGenerator::SQLExecutor::NULL_BINDING, 0));

								if (column_in_view.total_outer_multiplicity__in_total_kad__for_current_dmu_category__for_current_variable_group > 1)
								{
									inner_table__primary_keys_with_multiplicity_greater_than_one__which_binding_to_use__set.push_back(std::make_pair(OutputModel::OutputGenerator::SQLExecutor::NULL_BINDING, 0));
								}

								if (column_in_view.column_type == ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__PRIMARY)
								{
									inner_table__all_primary_keys__which_binding_to_use__set.push_back(std::make_pair(OutputModel::OutputGenerator::SQLExecutor::NULL_BINDING, 0));
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

	// ************************************************************************* //
	// The following if/else block only applies to PRIMARY_VARIABLE_GROUP merges
	// ************************************************************************* //
	if (swap_current_and_previous_and_set_previous_to_null)
	{
		// ************************************************************************* //
		// Only applies to PRIMARY_VARIABLE_GROUP merges
		// ************************************************************************* //

		// The addition of 2 handles the fact that the new table being added
		// has no MERGED time range columns
		for (int n = 0; n < number_nulls_to_add_at_end + 2; ++n)
		{
			bound_parameter_which_binding_to_use.push_back(SQLExecutor::NULL_BINDING);
		}
	}
	else
	{
		// ************************************************************************* //
		// Only applies to PRIMARY_VARIABLE_GROUP merges
		// ************************************************************************* //

		// Sort the data by inner table column set.
		if (xr_table_category == OutputModel::OutputGenerator::PRIMARY_VARIABLE_GROUP)
		{
			int number_columns_first_sets = (int)inner_table_columns[0].bindings.size();
			int number_columns_last_set = (int)inner_table_columns[inner_table_columns.size() - 1].bindings.size();
			int number_inner_column_sets = (int)inner_table_columns.size();
			std::sort(inner_table_columns.begin(), inner_table_columns.end());

			// First, check if there are self-kads that should be skipped
			if (remove_self_kads)
			{

				bool continue_checking = true;

				while (continue_checking)
				{

					ColumnSorter previous_columns;
					int current_column_set = 0;
					bool do_any_adjacent_pairs_of_inner_tables_have_the_same_primary_keys = false;
					std::for_each(inner_table_columns.begin(),
								  inner_table_columns.end(), [&do_any_adjacent_pairs_of_inner_tables_have_the_same_primary_keys, &current_column_set, &previous_columns, &number_inner_column_sets, &number_columns_first_sets, &number_columns_last_set](
									  ColumnSorter & new_columns_to_test)
					{
						if (current_column_set == 0)
						{
							previous_columns = new_columns_to_test;
						}
						else
						{
							int testing_index = 0;
							bool inner_tables_have_different_primary_keys = false;
							std::for_each(previous_columns.bindings__all_primary_keys.cbegin(),
										  previous_columns.bindings__all_primary_keys.cend(), [&inner_tables_have_different_primary_keys, &testing_index, &previous_columns, &new_columns_to_test](
											  std::pair<SQLExecutor::WHICH_BINDING, int> const & binding)
							{

								std::pair<SQLExecutor::WHICH_BINDING, int> const & test_binding = new_columns_to_test.bindings__all_primary_keys[testing_index];

								switch (binding.first)
								{

									case OutputModel::OutputGenerator::SQLExecutor::INT64:
										{

											std::int64_t binding_value_int = previous_columns.ints[binding.second];

											switch (test_binding.first)
											{

												case OutputModel::OutputGenerator::SQLExecutor::INT64:
													{

														std::int64_t test_binding_value_int = new_columns_to_test.ints[test_binding.second];

														if (binding_value_int != test_binding_value_int)
														{
															inner_tables_have_different_primary_keys = true;
														}

													}
													break;

												case OutputModel::OutputGenerator::SQLExecutor::FLOAT:
													{

														long double test_binding_value_float = new_columns_to_test.floats[test_binding.second];

														if (boost::lexical_cast<long double>(binding_value_int) != test_binding_value_float)
														{
															inner_tables_have_different_primary_keys = true;
														}

													}
													break;

												case OutputModel::OutputGenerator::SQLExecutor::STRING:
													{

														std::string test_binding_value_string = new_columns_to_test.strings[test_binding.second];

														if (binding_value_int != boost::lexical_cast<std::int64_t>(test_binding_value_string))
														{
															inner_tables_have_different_primary_keys = true;
														}

													}
													break;

												case OutputModel::OutputGenerator::SQLExecutor::NULL_BINDING:
													{

														inner_tables_have_different_primary_keys = true;

													}
													break;

												default:
													break;

											}
										}
										break;

									case OutputModel::OutputGenerator::SQLExecutor::FLOAT:
										{

											long double binding_value_float = previous_columns.floats[binding.second];

											switch (test_binding.first)
											{

												case OutputModel::OutputGenerator::SQLExecutor::INT64:
													{

														std::int64_t test_binding_value_int = new_columns_to_test.ints[test_binding.second];

														if (binding_value_float != boost::lexical_cast<long double>(test_binding_value_int))
														{
															inner_tables_have_different_primary_keys = true;
														}

													}
													break;

												case OutputModel::OutputGenerator::SQLExecutor::FLOAT:
													{

														long double test_binding_value_float = new_columns_to_test.floats[test_binding.second];

														if (binding_value_float != test_binding_value_float)
														{
															inner_tables_have_different_primary_keys = true;
														}

													}
													break;

												case OutputModel::OutputGenerator::SQLExecutor::STRING:
													{

														std::string test_binding_value_string = new_columns_to_test.strings[test_binding.second];

														if (binding_value_float != boost::lexical_cast<long double>(test_binding_value_string))
														{
															inner_tables_have_different_primary_keys = true;
														}

													}
													break;

												case OutputModel::OutputGenerator::SQLExecutor::NULL_BINDING:
													{

														inner_tables_have_different_primary_keys = true;

													}
													break;

												default:
													break;

											}
										}
										break;

									case OutputModel::OutputGenerator::SQLExecutor::STRING:
										{

											std::string binding_value_string = previous_columns.strings[binding.second];

											switch (test_binding.first)
											{

												case OutputModel::OutputGenerator::SQLExecutor::INT64:
													{

														std::int64_t test_binding_value_int = new_columns_to_test.ints[test_binding.second];

														if (boost::lexical_cast<std::int64_t>(binding_value_string) != test_binding_value_int)
														{
															inner_tables_have_different_primary_keys = true;
														}

													}
													break;

												case OutputModel::OutputGenerator::SQLExecutor::FLOAT:
													{

														long double test_binding_value_float = new_columns_to_test.floats[test_binding.second];

														if (boost::lexical_cast<long double>(binding_value_string) != test_binding_value_float)
														{
															inner_tables_have_different_primary_keys = true;
														}

													}
													break;

												case OutputModel::OutputGenerator::SQLExecutor::STRING:
													{

														std::string test_binding_value_string = new_columns_to_test.strings[test_binding.second];

														if (!boost::iequals(binding_value_string, test_binding_value_string))
														{

															inner_tables_have_different_primary_keys = true;

														}

													}
													break;

												case OutputModel::OutputGenerator::SQLExecutor::NULL_BINDING:
													{

														inner_tables_have_different_primary_keys = true;

													}
													break;

												default:
													break;

											}
										}
										break;

									case OutputModel::OutputGenerator::SQLExecutor::NULL_BINDING:
										{

											switch (test_binding.first)
											{

												case OutputModel::OutputGenerator::SQLExecutor::INT64:
													{

														inner_tables_have_different_primary_keys = true;

													}
													break;

												case OutputModel::OutputGenerator::SQLExecutor::FLOAT:
													{

														inner_tables_have_different_primary_keys = true;

													}
													break;

												case OutputModel::OutputGenerator::SQLExecutor::STRING:
													{

														inner_tables_have_different_primary_keys = true;

													}
													break;

												case OutputModel::OutputGenerator::SQLExecutor::NULL_BINDING:
													{

														// All NULL inner tables are considered different from other all NULL inner tables
														// ... This avoids an infinite loop that would result from continuously setting an all NULL
														// ... inner table to all NULL because it continues to match a previous all NULL inner table
														inner_tables_have_different_primary_keys = true;

													}
													break;

												default:
													break;

											}
										}
										break;

									default:
										break;
								}

								++testing_index;

							});

							if (!inner_tables_have_different_primary_keys)
							{
								// Self-Kad found!  Remove it.

								// set the new inner table to all NULL.  It will be sorted to the right later.
								std::for_each(new_columns_to_test.bindings.begin(), new_columns_to_test.bindings.end(), [](std::pair<SQLExecutor::WHICH_BINDING, int> & binding)
								{
									binding.first = OutputModel::OutputGenerator::SQLExecutor::NULL_BINDING;
									binding.second = 0;
								});
								std::for_each(new_columns_to_test.bindings__all_primary_keys.begin(), new_columns_to_test.bindings__all_primary_keys.end(), [](std::pair<SQLExecutor::WHICH_BINDING, int> & binding)
								{
									binding.first = OutputModel::OutputGenerator::SQLExecutor::NULL_BINDING;
									binding.second = 0;
								});
								std::for_each(new_columns_to_test.bindings__primary_keys_with_multiplicity_greater_than_1.begin(),
											  new_columns_to_test.bindings__primary_keys_with_multiplicity_greater_than_1.end(), [](std::pair<SQLExecutor::WHICH_BINDING, int> & binding)
								{
									binding.first = OutputModel::OutputGenerator::SQLExecutor::NULL_BINDING;
									binding.second = 0;
								});
								new_columns_to_test.ints.clear();
								new_columns_to_test.floats.clear();
								new_columns_to_test.strings.clear();
								do_any_adjacent_pairs_of_inner_tables_have_the_same_primary_keys = true;
							}
							else
							{
								previous_columns = new_columns_to_test;
							}

						}

						++current_column_set;
					});

					if (do_any_adjacent_pairs_of_inner_tables_have_the_same_primary_keys)
					{
						std::sort(inner_table_columns.begin(), inner_table_columns.end());
					}
					else
					{
						continue_checking = false;
					}

				}

			}

			bound_parameter_ints.clear();
			bound_parameter_floats.clear();
			bound_parameter_strings.clear();
			bound_parameter_which_binding_to_use.clear();
			int current_inner_column_set = 0;
			int total_index = 0;
			std::for_each(inner_table_columns.cbegin(),
						  inner_table_columns.cend(), [&index_of_previous_datetime_start_column, &index_of_previous_datetime_end_column, &index_of_current_datetime_start_column, &index_of_current_datetime_end_column, &saved_previous_datetime_start, &saved_previous_datetime_end, &saved_current_datetime_start, &saved_current_datetime_end, &total_index, &current_inner_column_set, &number_inner_column_sets, &number_columns_first_sets, &number_columns_last_set, &bound_parameter_ints, &bound_parameter_floats, &bound_parameter_strings, &bound_parameter_which_binding_to_use](
							  ColumnSorter const & columns_in_single_inner_table)
			{
				int the_index = 0;
				std::for_each(columns_in_single_inner_table.bindings.cbegin(),
							  columns_in_single_inner_table.bindings.cend(), [&index_of_previous_datetime_start_column, &index_of_previous_datetime_end_column, &index_of_current_datetime_start_column, &index_of_current_datetime_end_column, &saved_previous_datetime_start, &saved_previous_datetime_end, &saved_current_datetime_start, &saved_current_datetime_end, &total_index, &columns_in_single_inner_table, &the_index, &bound_parameter_ints, &bound_parameter_floats, &bound_parameter_strings, &bound_parameter_which_binding_to_use](
								  std::pair<OutputModel::OutputGenerator::SQLExecutor::WHICH_BINDING, int> const & binding_info)
				{
					bool handled = false;

					if (total_index == index_of_current_datetime_start_column)
					{
						bound_parameter_ints.push_back(saved_current_datetime_start);
						bound_parameter_which_binding_to_use.push_back(OutputModel::OutputGenerator::SQLExecutor::INT64);
						handled = true;
					}

					if (total_index == index_of_current_datetime_end_column)
					{
						bound_parameter_ints.push_back(saved_current_datetime_end);
						bound_parameter_which_binding_to_use.push_back(OutputModel::OutputGenerator::SQLExecutor::INT64);
						handled = true;
					}

					if (total_index == index_of_previous_datetime_start_column)
					{
						bound_parameter_ints.push_back(saved_previous_datetime_start);
						bound_parameter_which_binding_to_use.push_back(OutputModel::OutputGenerator::SQLExecutor::INT64);
						handled = true;
					}

					if (total_index == index_of_previous_datetime_end_column)
					{
						bound_parameter_ints.push_back(saved_previous_datetime_end);
						bound_parameter_which_binding_to_use.push_back(OutputModel::OutputGenerator::SQLExecutor::INT64);
						handled = true;
					}

					if (!handled)
					{
						switch (binding_info.first)
						{
							case OutputModel::OutputGenerator::SQLExecutor::INT64:
								{
									bound_parameter_ints.push_back(columns_in_single_inner_table.ints[binding_info.second]);
									bound_parameter_which_binding_to_use.push_back(OutputModel::OutputGenerator::SQLExecutor::INT64);
								}
								break;

							case OutputModel::OutputGenerator::SQLExecutor::FLOAT:
								{
									bound_parameter_floats.push_back(columns_in_single_inner_table.floats[binding_info.second]);
									bound_parameter_which_binding_to_use.push_back(OutputModel::OutputGenerator::SQLExecutor::FLOAT);
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

							default:
								break;
						}
					}

					++the_index;
					++total_index;
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
						// Must add the datetime columns...
						// Just push the previous values from the current last two columns into the new columns.
						int number_to_add = number_columns_first_sets - static_cast<int>(columns_in_single_inner_table.bindings.size());

						for (int n = 0; n < number_to_add; ++n)
						{
							bound_parameter_ints.push_back(0);
							bound_parameter_which_binding_to_use.push_back(OutputModel::OutputGenerator::SQLExecutor::INT64);
							++total_index;
						}

						// Now populate with the previous values
						if (current_inner_column_set == number_inner_column_sets - 2)
						{
							// This is the exact second to last inner table.
							// Add the saved datetime columns.
							bound_parameter_ints[bound_parameter_ints.size() - 2] = saved_previous_datetime_start;
							bound_parameter_ints[bound_parameter_ints.size() - 1] = saved_previous_datetime_end;
						}
						else
						{
							bound_parameter_ints[bound_parameter_ints.size() - 2] = bound_parameter_ints[bound_parameter_ints.size() - 2 - number_to_add];
							bound_parameter_ints[bound_parameter_ints.size() - 1] = bound_parameter_ints[bound_parameter_ints.size() - 1 - number_to_add];
						}
					}
				}
				else
				{
					// The current (new) inner column set being constructed here is the last one
					if (columns_in_single_inner_table.bindings.size() == number_columns_first_sets)
					{

						// The "merged" datetime columns have been added, but they should not have been in this special case.

						// Note that the saved current columns are already in place.

						int number_to_remove = static_cast<int>(columns_in_single_inner_table.bindings.size()) - number_columns_last_set;

						for (int n = 0; n < number_to_remove; ++n)
						{
							OutputModel::OutputGenerator::SQLExecutor::WHICH_BINDING binding = bound_parameter_which_binding_to_use.back();
							bound_parameter_which_binding_to_use.pop_back();

							switch (binding)
							{
								case OutputModel::OutputGenerator::SQLExecutor::INT64:
									{
										bound_parameter_ints.pop_back();
									}
									break;

								case OutputModel::OutputGenerator::SQLExecutor::FLOAT:
									{
										bound_parameter_floats.pop_back();
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

								default:
									break;
							}
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

	if (failed || CheckCancelled())
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

OutputModel::OutputGenerator::SqlAndColumnSet OutputModel::OutputGenerator::CreateChildXTable(ColumnsInTempView const & child_variable_group_raw_data_columns,
		ColumnsInTempView const & previous_xr_columns, int const
		current_outer_multiplicity_of_child_table___same_as___current_inner_table_number_within_the_inner_table_set_for_the_current_child_variable_group, int const primary_group_number,
		int const child_set_number, int const current_child_view_name_index)
{

	SqlAndColumnSet result = std::make_pair(std::vector<SQLExecutor>(), ColumnsInTempView());
	std::vector<SQLExecutor> & sql_strings = result.first;
	ColumnsInTempView & result_columns = result.second;

	result_columns = previous_xr_columns;

	std::string view_name = "NGTEMP_CV";
	view_name += std::to_string(primary_group_number);
	view_name += "_x";
	view_name += std::to_string(current_child_view_name_index);
	result_columns.view_name_no_uuid = view_name;
	view_name += "_";
	view_name += newUUID(true);
	result_columns.view_name = view_name;
	result_columns.view_number = current_outer_multiplicity_of_child_table___same_as___current_inner_table_number_within_the_inner_table_set_for_the_current_child_variable_group;
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

	// First, calculate some indices, as well as modify the previous table's column names as they appear in the new schema
	bool first = true;
	bool not_yet_reached_any_datetime_columns = true;
	bool reached_second_inner_table = false;
	std::for_each(result_columns.columns_in_view.begin(),
				  result_columns.columns_in_view.end(), [&not_yet_reached_any_datetime_columns, &reached_second_inner_table, &first_full_table_column_count, &top_level_inner_table_column_count, &previous_column_names_first_table, &first](
					  ColumnsInTempView::ColumnInTempView & new_column)
	{
		previous_column_names_first_table.push_back(new_column.column_name_in_temporary_table);
		new_column.column_name_in_temporary_table = new_column.column_name_in_temporary_table_no_uuid;
		new_column.column_name_in_temporary_table += "_";
		new_column.column_name_in_temporary_table += newUUID(true);
		++first_full_table_column_count;

		if (new_column.column_type != ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__PRIMARY
			&& new_column.column_type != ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__SECONDARY)
		{
			// This is a datetime column
			not_yet_reached_any_datetime_columns = false;
		}
		else
		{
			// This is not a datetime column
			if (!not_yet_reached_any_datetime_columns)
			{
				reached_second_inner_table = true;
			}
		}

		if (!reached_second_inner_table)
		{
			// The number of columns in each top-level primary variable group, including datetime columns.
			// Used to determine which top-level variable group we're in.
			++top_level_inner_table_column_count;
		}
	});

	WidgetInstanceIdentifiers const & variables_selected =
		(*the_map)[*child_variable_group_raw_data_columns.variable_groups[0].identifier_parent][child_variable_group_raw_data_columns.variable_groups[0]];

	// These columns are from the new table (the raw child data table) being added.
	// Make column names for this temporary table unique (not the same as the column names from the previous table that is being copied)
	// which may or may not have datetime columns.
	// Further, the "current_multiplicity" of these columns is 1, and must be updated.
	//
	// Start with the primary key columns.
	first = true;
	std::for_each(child_variable_group_raw_data_columns.columns_in_view.cbegin(),
				  child_variable_group_raw_data_columns.columns_in_view.cend(), [&first, &child_set_number, &variable_group_child, &uoa_child, &variables_selected, &result_columns, &second_table_column_count, &current_outer_multiplicity_of_child_table___same_as___current_inner_table_number_within_the_inner_table_set_for_the_current_child_variable_group](
					  ColumnsInTempView::ColumnInTempView const & new_column_child)
	{
		if (new_column_child.column_type == ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__DATETIMESTART
			|| new_column_child.column_type == ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__DATETIMESTART_INTERNAL
			|| new_column_child.column_type == ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__DATETIMEEND
			|| new_column_child.column_type == ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__DATETIMEEND_INTERNAL)
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
			new_column.is_within_inner_table_corresponding_to_top_level_uoa = false;
			new_column.current_multiplicity__of__current_inner_table__within__current_vg_inner_table_set =
				current_outer_multiplicity_of_child_table___same_as___current_inner_table_number_within_the_inner_table_set_for_the_current_child_variable_group;

			if (new_column.column_type == ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__PRIMARY)
			{
				if (new_column.total_outer_multiplicity__in_total_kad__for_current_dmu_category__for_current_variable_group > 1)
				{
					new_column.current_multiplicity__corresponding_to__current_inner_table___is_1_in_all_inner_tables_when_multiplicity_is_1_for_that_dmu_category_for_that_vg =
						current_outer_multiplicity_of_child_table___same_as___current_inner_table_number_within_the_inner_table_set_for_the_current_child_variable_group; // update current multiplicity

					if (new_column.total_k_count__within_uoa_corresponding_to_current_variable_group__for_current_dmu_category <
						new_column.total_k_count__within_uoa_corresponding_to_top_level_variable_group__for_current_dmu_category)
					{
						new_column.primary_key_index_within_total_kad_for_dmu_category =
							current_outer_multiplicity_of_child_table___same_as___current_inner_table_number_within_the_inner_table_set_for_the_current_child_variable_group;
					}
					else
					{
						// must have: new_column.total_k_count__within_uoa_corresponding_to_current_variable_group__for_current_dmu_category
						//         == new_column.total_k_count__within_uoa_corresponding_to_top_level_variable_group__for_current_dmu_category
						new_column.primary_key_index_within_total_kad_for_dmu_category =
							new_column.primary_key_index__within_uoa_corresponding_to_variable_group_corresponding_to_current_inner_table__for_dmu_category
							+ (current_outer_multiplicity_of_child_table___same_as___current_inner_table_number_within_the_inner_table_set_for_the_current_child_variable_group - 1) *
							new_column.total_k_count__within_uoa_corresponding_to_current_variable_group__for_current_dmu_category;
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
	std::for_each(child_variable_group_raw_data_columns.columns_in_view.cbegin(),
				  child_variable_group_raw_data_columns.columns_in_view.cend(), [&first, &child_set_number, &variable_group_child, &uoa_child, &variables_selected, &result_columns, &second_table_column_count, &current_outer_multiplicity_of_child_table___same_as___current_inner_table_number_within_the_inner_table_set_for_the_current_child_variable_group](
					  ColumnsInTempView::ColumnInTempView const & new_column_secondary)
	{
		bool make_secondary_datetime_column = false;

		if (new_column_secondary.column_type == ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__DATETIMESTART
			|| new_column_secondary.column_type == ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__DATETIMEEND)
		{
			// No!  If the user selectes these columns, they should appear as regular secondary key columns.  Change the column type in this case to "secondary".
			//return; // Add these columns last
			make_secondary_datetime_column = true;
		}

		if (!make_secondary_datetime_column && new_column_secondary.column_type != ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__SECONDARY)
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
			new_column.is_within_inner_table_corresponding_to_top_level_uoa = false;
			new_column.current_multiplicity__of__current_inner_table__within__current_vg_inner_table_set =
				current_outer_multiplicity_of_child_table___same_as___current_inner_table_number_within_the_inner_table_set_for_the_current_child_variable_group;

			if (new_column.column_type == ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__PRIMARY)
			{
				if (new_column.total_outer_multiplicity__in_total_kad__for_current_dmu_category__for_current_variable_group > 1)
				{
					new_column.current_multiplicity__corresponding_to__current_inner_table___is_1_in_all_inner_tables_when_multiplicity_is_1_for_that_dmu_category_for_that_vg =
						current_outer_multiplicity_of_child_table___same_as___current_inner_table_number_within_the_inner_table_set_for_the_current_child_variable_group; // update current multiplicity

					if (new_column.total_k_count__within_uoa_corresponding_to_current_variable_group__for_current_dmu_category <
						new_column.total_k_count__within_uoa_corresponding_to_top_level_variable_group__for_current_dmu_category)
					{
						new_column.primary_key_index_within_total_kad_for_dmu_category =
							current_outer_multiplicity_of_child_table___same_as___current_inner_table_number_within_the_inner_table_set_for_the_current_child_variable_group;
					}
					else
					{
						// must have: new_column.total_k_count__within_uoa_corresponding_to_current_variable_group__for_current_dmu_category
						//         == new_column.total_k_count__within_uoa_corresponding_to_top_level_variable_group__for_current_dmu_category
						new_column.primary_key_index_within_total_kad_for_dmu_category =
							new_column.primary_key_index__within_uoa_corresponding_to_variable_group_corresponding_to_current_inner_table__for_dmu_category
							+ (current_outer_multiplicity_of_child_table___same_as___current_inner_table_number_within_the_inner_table_set_for_the_current_child_variable_group - 1) *
							new_column.total_k_count__within_uoa_corresponding_to_current_variable_group__for_current_dmu_category;
					}
				}
			}

			if (make_secondary_datetime_column)
			{
				new_column.column_type = ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__SECONDARY;
				new_column.originally_datetime = true;
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
	std::for_each(child_variable_group_raw_data_columns.columns_in_view.cbegin(),
				  child_variable_group_raw_data_columns.columns_in_view.cend(), [&result_columns, &child_set_number, &second_table_column_count](
					  ColumnsInTempView::ColumnInTempView const & new_column_datetime)
	{
		if (new_column_datetime.column_type == ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__DATETIMESTART
			|| new_column_datetime.column_type == ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__DATETIMESTART_INTERNAL)
		{
			result_columns.columns_in_view.push_back(new_column_datetime);
			ColumnsInTempView::ColumnInTempView & new_column = result_columns.columns_in_view.back();
			new_column.column_name_in_temporary_table = new_column.column_name_in_temporary_table_no_uuid;
			new_column.column_name_in_temporary_table += "_";
			new_column.column_name_in_temporary_table += newUUID(true);
			new_column.is_within_inner_table_corresponding_to_top_level_uoa = false;
			++second_table_column_count;
		}
	});
	std::for_each(child_variable_group_raw_data_columns.columns_in_view.cbegin(),
				  child_variable_group_raw_data_columns.columns_in_view.cend(), [&result_columns, &child_set_number, &second_table_column_count](
					  ColumnsInTempView::ColumnInTempView const & new_column_datetime)
	{
		if (new_column_datetime.column_type == ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__DATETIMEEND
			|| new_column_datetime.column_type == ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__DATETIMEEND_INTERNAL)
		{
			result_columns.columns_in_view.push_back(new_column_datetime);
			ColumnsInTempView::ColumnInTempView & new_column = result_columns.columns_in_view.back();
			new_column.column_name_in_temporary_table = new_column.column_name_in_temporary_table_no_uuid;
			new_column.column_name_in_temporary_table += "_";
			new_column.column_name_in_temporary_table += newUUID(true);
			new_column.is_within_inner_table_corresponding_to_top_level_uoa = false;
			++second_table_column_count;
		}
	});

	sql_strings.push_back(SQLExecutor(this, db));
	std::string & sql_string = sql_strings.back().sql;

	sql_string = "CREATE TABLE \"";
	sql_string += result_columns.view_name;
	sql_string += "\" AS SELECT ";
	first = true;
	int column_count = 0;
	std::for_each(result_columns.columns_in_view.begin(),
				  result_columns.columns_in_view.end(), [&sql_string, &first, &column_count, &first_full_table_column_count, &second_table_column_count, &previous_column_names_first_table](
					  ColumnsInTempView::ColumnInTempView & new_column)
	{
		if (!first)
		{
			sql_string += ", ";
		}

		first = false;

		// Not legal for outer joins - but child table should already have proper column type
		if (false && new_column.column_type == ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__PRIMARY
			&& new_column.primary_key_should_be_treated_as_integer_____float_not_allowed_as_primary_key)
		{
			sql_string += "CAST (";
		}

		if (column_count < first_full_table_column_count)
		{
			sql_string += "\"t1\".";
			sql_string += "`";
			sql_string += previous_column_names_first_table[column_count];
			sql_string += "`";
		}
		else
		{
			sql_string += "\"t2\".";
			sql_string += "`";
			sql_string += new_column.column_name_in_temporary_table_no_uuid; // This is the original column name
			sql_string += "`";
		}

		// Not legal for outer joins - but child table should already have proper column type
		if (false && new_column.column_type == ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__PRIMARY
			&& new_column.primary_key_should_be_treated_as_integer_____float_not_allowed_as_primary_key)
		{
			sql_string += " AS INTEGER)";
		}

		sql_string += " AS ";
		sql_string += "`";
		sql_string += new_column.column_name_in_temporary_table;
		sql_string += "`";
		++column_count;
	});
	sql_string += " FROM \"";
	sql_string += previous_xr_columns.view_name;
	sql_string += "\" t1 LEFT OUTER JOIN \"";
	sql_string += child_variable_group_raw_data_columns.original_table_names[0];
	sql_string += "\" t2 ON ";
	bool and_ = false;
	std::for_each(sequence.primary_key_sequence_info.cbegin(),
				  sequence.primary_key_sequence_info.cend(), [this, &current_outer_multiplicity_of_child_table___same_as___current_inner_table_number_within_the_inner_table_set_for_the_current_child_variable_group, &sql_string, &variable_group_child, &result_columns, &first_full_table_column_count, &top_level_inner_table_column_count, &second_table_column_count, &previous_column_names_first_table, &and_](
					  PrimaryKeySequence::PrimaryKeySequenceEntry const & primary_key)
	{
		std::for_each(primary_key.variable_group_info_for_primary_keys.cbegin(),
					  primary_key.variable_group_info_for_primary_keys.cend(), [this, &current_outer_multiplicity_of_child_table___same_as___current_inner_table_number_within_the_inner_table_set_for_the_current_child_variable_group, &sql_string, &variable_group_child, &primary_key, &result_columns, &first_full_table_column_count, &top_level_inner_table_column_count, &second_table_column_count, &previous_column_names_first_table, &and_](
						  PrimaryKeySequence::VariableGroup_PrimaryKey_Info const & primary_key_info_this_variable_group)
		{

			if (primary_key_info_this_variable_group.vg_identifier.IsEqual(WidgetInstanceIdentifier::EQUALITY_CHECK_TYPE__STRING_CODE, variable_group_child))
			{

				// Now we have the primary key information for OUR child variable group...
				// for an arbitrary primary key out of the FULL K-AD.

				// But we don't know which of this full sequence of primary keys we have

				// So narrow it down

				if (

					// Match against a primary key that states itself to be in the same inner table number as we are building an inner table for
					primary_key_info_this_variable_group.current_outer_multiplicity_of_this_primary_key__in_relation_to__the_uoa_corresponding_to_the_current_variable_group___same_as___current_inner_table_number_within_the_inner_table_set_corresponding_to_the_current_variable_group
					== current_outer_multiplicity_of_child_table___same_as___current_inner_table_number_within_the_inner_table_set_for_the_current_child_variable_group


					// Or, for those primary keys that appear identically in all inner tables,
					// i.e. have multiplicity of 1 in this VG's UOA,
					// the primary key will state itself to belong to the FIRST inner table, yet ALL child tables have the same value
					// as the first inner table for this primary key,
					// ... so always match against these primary keys.
					|| (primary_key_info_this_variable_group.total_outer_multiplicity__in_total_kad__for_current_dmu_category__for_current_variable_group == 1

						// redundant, but safe check - these primary keys only appear once in the list of primary keys, and they therefore all have a value of 1
						&& primary_key_info_this_variable_group.current_outer_multiplicity_of_this_primary_key__in_relation_to__the_uoa_corresponding_to_the_current_variable_group___same_as___current_inner_table_number_within_the_inner_table_set_corresponding_to_the_current_variable_group
						== 1)

				)
				{
					int column_count = 0;
					std::for_each(result_columns.columns_in_view.cbegin(),
								  result_columns.columns_in_view.cend(), [this, &current_outer_multiplicity_of_child_table___same_as___current_inner_table_number_within_the_inner_table_set_for_the_current_child_variable_group, &sql_string, &primary_key_info_this_variable_group, &first_full_table_column_count, &top_level_inner_table_column_count, &second_table_column_count, &column_count, &previous_column_names_first_table, &primary_key, &and_](
									  ColumnsInTempView::ColumnInTempView const & new_column)
					{

						if (!new_column.is_within_inner_table_corresponding_to_top_level_uoa)
						{
							++column_count;
							return;
						}

						if (new_column.primary_key_dmu_category_identifier.IsEqual(WidgetInstanceIdentifier::EQUALITY_CHECK_TYPE__STRING_CODE, primary_key.dmu_category))
						{
							int desired_inner_table_index = 0;
							bool match_condition = false;

							// First, join on primary keys whose total multiplicity in this child variable group (against the full K-ad spin count) is 1.
							// All the keys are in the first inner table (of the top-level group).
							if (primary_key_info_this_variable_group.total_outer_multiplicity__in_total_kad__for_current_dmu_category__for_current_variable_group == 1)
							{
								if (primary_key_info_this_variable_group.current_outer_multiplicity_of_this_primary_key__in_relation_to__the_uoa_corresponding_to_the_current_variable_group___same_as___current_inner_table_number_within_the_inner_table_set_corresponding_to_the_current_variable_group
									== 1)
								{
									match_condition = (new_column.primary_key_index_within_total_kad_for_dmu_category >= 0
													   &&
													   (new_column.primary_key_index_within_total_kad_for_dmu_category == primary_key.sequence_number_within_dmu_category_spin_control));
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
									if (primary_key_info_this_variable_group.current_outer_multiplicity_of_this_primary_key__in_relation_to__the_uoa_corresponding_to_the_current_variable_group___same_as___current_inner_table_number_within_the_inner_table_set_corresponding_to_the_current_variable_group
										== current_outer_multiplicity_of_child_table___same_as___current_inner_table_number_within_the_inner_table_set_for_the_current_child_variable_group)
									{
										match_condition = (new_column.primary_key_index_within_total_kad_for_dmu_category >= 0
														   && (new_column.primary_key_index_within_total_kad_for_dmu_category == primary_key.sequence_number_within_dmu_category_spin_control));
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
										if (current_outer_multiplicity_of_child_table___same_as___current_inner_table_number_within_the_inner_table_set_for_the_current_child_variable_group == 1)
										{
											// Same match condition as above.
											match_condition = (new_column.primary_key_index_within_total_kad_for_dmu_category >= 0
															   && (new_column.primary_key_index_within_total_kad_for_dmu_category == primary_key.sequence_number_within_dmu_category_spin_control));
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
										if (primary_key_info_this_variable_group.total_number_columns_for_dmu_category__internal_to_uoa_corresponding_to_this_variable_group <
											primary_key.total_k_count_within_high_level_variable_group_uoa_for_this_dmu_category)
										{
											desired_inner_table_index = (current_outer_multiplicity_of_child_table___same_as___current_inner_table_number_within_the_inner_table_set_for_the_current_child_variable_group - 1) /
																		primary_key.total_k_count_within_high_level_variable_group_uoa_for_this_dmu_category;
											match_condition = (current_outer_multiplicity_of_child_table___same_as___current_inner_table_number_within_the_inner_table_set_for_the_current_child_variable_group - 1 ==
															   new_column.primary_key_index_within_total_kad_for_dmu_category);
										}

										// ... Case 4: The K-value for the *UOA* of the child group for this DMU category
										// ... matches the K-value for the *UOA* of the primary groups for this DMU category.
										// ... Therefore, we need to iterate through every inner table,
										// ... but inside each inner table, there is only one match for the child
										// ... that includes all columns in that table for this DMU category.
										else
										{
											desired_inner_table_index = current_outer_multiplicity_of_child_table___same_as___current_inner_table_number_within_the_inner_table_set_for_the_current_child_variable_group - 1;
											match_condition = (new_column.primary_key_index_within_total_kad_for_dmu_category >= 0
															   && (new_column.primary_key_index__within_uoa_corresponding_to_variable_group_corresponding_to_current_inner_table__for_dmu_category ==
																   primary_key_info_this_variable_group.sequence_number_within_dmu_category_for_this_variable_groups_uoa));
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
									sql_string += "\"t1\".`";
									sql_string += previous_column_names_first_table[column_count];
									sql_string += "` = \"t2\".`";
									sql_string += primary_key_info_this_variable_group.table_column_name;
									sql_string += "`";
								}
							}

						}

						++column_count;
					});
				}
			}
		
		});
	});

	if (true)
	{

		if (true)
		{
			if (!child_variable_group_raw_data_columns.has_no_datetime_columns_originally)
			{
				if (and_)
				{
					sql_string += " AND ";
				}

				and_ = true;

				sql_string += " CASE WHEN `";
				sql_string += result_columns.columns_in_view[result_columns.columns_in_view.size() - 2].column_name_in_temporary_table;
				sql_string += "` = 0 AND `";
				sql_string += result_columns.columns_in_view[result_columns.columns_in_view.size() - 1].column_name_in_temporary_table;
				sql_string += "` = 0 ";
				sql_string += " THEN 1 ";
				sql_string += " WHEN `";
				sql_string += result_columns.columns_in_view[result_columns.columns_in_view.size() - 2].column_name_in_temporary_table;
				sql_string += "` < ";
				sql_string += boost::lexical_cast<std::string>(timerange_end); // the cast isn't really necessary, because std::string knows how to append a number
				sql_string += " THEN `";
				sql_string += result_columns.columns_in_view[result_columns.columns_in_view.size() - 1].column_name_in_temporary_table;
				sql_string += "` > ";
				sql_string += boost::lexical_cast<std::string>(timerange_start); // the cast isn't really necessary, because std::string knows how to append a number
				sql_string += " WHEN `";
				sql_string += result_columns.columns_in_view[result_columns.columns_in_view.size() - 1].column_name_in_temporary_table;
				sql_string += "` > ";
				sql_string += boost::lexical_cast<std::string>(timerange_start); // the cast isn't really necessary, because std::string knows how to append a number
				sql_string += " THEN `";
				sql_string += result_columns.columns_in_view[result_columns.columns_in_view.size() - 2].column_name_in_temporary_table;
				sql_string += "` < ";
				sql_string += boost::lexical_cast<std::string>(timerange_end); // the cast isn't really necessary, because std::string knows how to append a number
				sql_string += " ELSE 0";
				sql_string += " END";

				if (and_)
				{
					sql_string += " AND ";
				}

				and_ = true;

				sql_string += " CASE WHEN `";
				sql_string += result_columns.columns_in_view[result_columns.columns_in_view.size() - 2].column_name_in_temporary_table;
				sql_string += "` = 0 AND `";
				sql_string += result_columns.columns_in_view[result_columns.columns_in_view.size() - 1].column_name_in_temporary_table;
				sql_string += "` = 0 ";
				sql_string += " THEN 1 ";
				sql_string += " WHEN `";
				sql_string += result_columns.columns_in_view[result_columns.columns_in_view.size() - 2].column_name_in_temporary_table;
				sql_string += "` < `";
				sql_string += result_columns.columns_in_view[previous_xr_columns.columns_in_view.size() - 1].column_name_in_temporary_table; // end date of previous columns
				sql_string += "` THEN `";
				sql_string += result_columns.columns_in_view[result_columns.columns_in_view.size() - 1].column_name_in_temporary_table;
				sql_string += "` > `";
				sql_string += result_columns.columns_in_view[previous_xr_columns.columns_in_view.size() - 2].column_name_in_temporary_table; // start date of previous columns
				sql_string += "` WHEN `";
				sql_string += result_columns.columns_in_view[result_columns.columns_in_view.size() - 1].column_name_in_temporary_table;
				sql_string += "` > `";
				sql_string += result_columns.columns_in_view[previous_xr_columns.columns_in_view.size() - 2].column_name_in_temporary_table; // start date of previous columns
				sql_string += "` THEN `";
				sql_string += result_columns.columns_in_view[result_columns.columns_in_view.size() - 2].column_name_in_temporary_table;
				sql_string += "` < `";
				sql_string += result_columns.columns_in_view[previous_xr_columns.columns_in_view.size() - 1].column_name_in_temporary_table; // end date of previous columns
				sql_string += "` ELSE 0";
				sql_string += " END";
			}
		}

	}

	// No!  No WHERE clause for time range when merging child groups!
	// This has the effect of causing valid rows to not appear in the result set
	// (i.e., the child data should just be NULL, rather than the entire row be missing).
	// The later "XR" algorithm that loops through all rows evaluating the time range
	// condition for both the previous, and the new, data, will take care of
	// child data that does not match the user's selection of time range.
	//
	// Instead, put this functionality into the JOIN ON clause, in the above "if" block.
	else
	{
		if (!child_variable_group_raw_data_columns.has_no_datetime_columns_originally)
		{
			sql_string += " WHERE CASE WHEN `";
			sql_string += result_columns.columns_in_view[result_columns.columns_in_view.size() - 2].column_name_in_temporary_table;
			sql_string += "` = 0 AND `";
			sql_string += result_columns.columns_in_view[result_columns.columns_in_view.size() - 1].column_name_in_temporary_table;
			sql_string += "` = 0 ";
			sql_string += " THEN 1 ";
			sql_string += " WHEN `";
			sql_string += result_columns.columns_in_view[result_columns.columns_in_view.size() - 2].column_name_in_temporary_table;
			sql_string += "` < ";
			sql_string += boost::lexical_cast<std::string>(timerange_end);
			sql_string += " THEN `";
			sql_string += result_columns.columns_in_view[result_columns.columns_in_view.size() - 1].column_name_in_temporary_table;
			sql_string += "` > ";
			sql_string += boost::lexical_cast<std::string>(timerange_start);
			sql_string += " WHEN `";
			sql_string += result_columns.columns_in_view[result_columns.columns_in_view.size() - 1].column_name_in_temporary_table;
			sql_string += "` > ";
			sql_string += boost::lexical_cast<std::string>(timerange_start);
			sql_string += " THEN `";
			sql_string += result_columns.columns_in_view[result_columns.columns_in_view.size() - 2].column_name_in_temporary_table;
			sql_string += "` < ";
			sql_string += boost::lexical_cast<std::string>(timerange_end);
			sql_string += " ELSE 0";
			sql_string += " END";
		}
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
			std::for_each(result_columns.columns_in_view.begin(),
						  result_columns.columns_in_view.end(), [this, &first, &first_top_level_vg, &number_primary_key_columns_in_dmu_category_with_multiplicity_greater_than_1__for_top_level_uoa, &sql_string](
							  ColumnsInTempView::ColumnInTempView & view_column)
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
						if (view_column.total_outer_multiplicity__in_total_kad__for_current_dmu_category__for_current_variable_group == highest_multiplicity_primary_uoa)
						{
							if (view_column.current_multiplicity__corresponding_to__current_inner_table___is_1_in_all_inner_tables_when_multiplicity_is_1_for_that_dmu_category_for_that_vg == 1)
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
				for (int inner_dmu_multiplicity = 0; inner_dmu_multiplicity < number_primary_key_columns_in_dmu_category_with_multiplicity_greater_than_1__for_top_level_uoa;
					 ++inner_dmu_multiplicity)
				{
					std::for_each(result_columns.columns_in_view.begin(),
								  result_columns.columns_in_view.end(), [this, &first_top_level_vg, &inner_dmu_multiplicity, &outer_dmu_multiplicity, &sql_string, &first](
									  ColumnsInTempView::ColumnInTempView & view_column)
					{
						if (view_column.column_type == ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__PRIMARY)
						{
							if (view_column.total_outer_multiplicity__in_total_kad__for_current_dmu_category__for_current_variable_group == highest_multiplicity_primary_uoa)
							{
								if (view_column.primary_key_index__within_uoa_corresponding_to_variable_group_corresponding_to_current_inner_table__for_dmu_category == inner_dmu_multiplicity)
								{
									if (view_column.current_multiplicity__corresponding_to__current_inner_table___is_1_in_all_inner_tables_when_multiplicity_is_1_for_that_dmu_category_for_that_vg ==
										outer_dmu_multiplicity)
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

												if (view_column.primary_key_should_be_treated_as_integer_____float_not_allowed_as_primary_key)
												{
													sql_string += "CAST (";
												}

												sql_string += "`";
												sql_string += view_column.column_name_in_temporary_table;
												sql_string += "`";

												if (view_column.primary_key_should_be_treated_as_integer_____float_not_allowed_as_primary_key)
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
		std::for_each(result_columns.columns_in_view.begin(),
					  result_columns.columns_in_view.end(), [this, &sql_string, &result_columns, &current_column, &top_level_inner_table_column_count, &first](
						  ColumnsInTempView::ColumnInTempView & view_column)
		{
			if (current_column >= top_level_inner_table_column_count)
			{
				++current_column;
				return;
			}

			// Determine how many columns there are corresponding to this DMU category
			int number_primary_key_columns_in_dmu_category_with_multiplicity_of_only_1__for_top_level_uoa = 0;
			int column_count_nested = 0;
			std::for_each(result_columns.columns_in_view.begin(),
						  result_columns.columns_in_view.end(), [this, &view_column, &column_count_nested, &top_level_inner_table_column_count, &number_primary_key_columns_in_dmu_category_with_multiplicity_of_only_1__for_top_level_uoa, &sql_string](
							  ColumnsInTempView::ColumnInTempView & view_column_nested)
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
						if (view_column_nested.total_outer_multiplicity__in_total_kad__for_current_dmu_category__for_current_variable_group == 1)
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
				if (view_column.total_outer_multiplicity__in_total_kad__for_current_dmu_category__for_current_variable_group == 1)
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

								if (view_column.primary_key_should_be_treated_as_integer_____float_not_allowed_as_primary_key)
								{
									sql_string += "CAST (";
								}

								sql_string += "`";
								sql_string += view_column.column_name_in_temporary_table;
								sql_string += "`";

								if (view_column.primary_key_should_be_treated_as_integer_____float_not_allowed_as_primary_key)
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
		std::string datetime_start_col_name_no_uuid = Table_VariableGroupMetadata_DateTimeColumns::DefaultDatetimeStartColumnName;
		std::string datetime_start_col_name = datetime_start_col_name_no_uuid;
		datetime_start_col_name += "_";
		datetime_start_col_name += newUUID(true);

		std::string alter_string;
		alter_string += "ALTER TABLE \"";
		alter_string += result_columns.view_name;
		alter_string += "\" ADD COLUMN ";
		alter_string += datetime_start_col_name;
		alter_string += " INTEGER DEFAULT 0";
		sql_strings.push_back(SQLExecutor(this, db, alter_string));

		if (failed)
		{
			SetFailureMessage(sql_error);
			return result;
		}

		if (CheckCancelled())
		{
			return result;
		}

		result_columns.columns_in_view.push_back(ColumnsInTempView::ColumnInTempView());
		ColumnsInTempView::ColumnInTempView & datetime_start_column = result_columns.columns_in_view.back();
		datetime_start_column.column_name_in_temporary_table = datetime_start_col_name;
		datetime_start_column.column_name_in_temporary_table_no_uuid = datetime_start_col_name_no_uuid;
		datetime_start_column.column_type = ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__DATETIMESTART_INTERNAL;
		datetime_start_column.variable_group_associated_with_current_inner_table = variable_group_child;
		datetime_start_column.uoa_associated_with_variable_group_associated_with_current_inner_table = uoa_child;
		datetime_start_column.column_name_in_original_data_table = "";
		datetime_start_column.is_within_inner_table_corresponding_to_top_level_uoa = false;
		datetime_start_column.current_multiplicity__of__current_inner_table__within__current_vg_inner_table_set =
			child_variable_group_raw_data_columns.columns_in_view.back().current_multiplicity__of__current_inner_table__within__current_vg_inner_table_set;
		datetime_start_column.number_inner_tables_in_set = child_variable_group_raw_data_columns.columns_in_view.back().number_inner_tables_in_set;

		std::string datetime_end_col_name_no_uuid = Table_VariableGroupMetadata_DateTimeColumns::DefaultDatetimeEndColumnName;
		std::string datetime_end_col_name = datetime_end_col_name_no_uuid;
		datetime_end_col_name += "_";
		datetime_end_col_name += newUUID(true);

		alter_string.clear();
		alter_string += "ALTER TABLE \"";
		alter_string += result_columns.view_name;
		alter_string += "\" ADD COLUMN ";
		alter_string += datetime_end_col_name;
		alter_string += " INTEGER DEFAULT 0";
		sql_strings.push_back(SQLExecutor(this, db, alter_string));

		if (failed)
		{
			SetFailureMessage(sql_error);
			return result;
		}

		if (CheckCancelled())
		{
			return result;
		}

		result_columns.columns_in_view.push_back(ColumnsInTempView::ColumnInTempView());
		ColumnsInTempView::ColumnInTempView & datetime_end_column = result_columns.columns_in_view.back();
		datetime_end_column.column_name_in_temporary_table = datetime_end_col_name;
		datetime_end_column.column_name_in_temporary_table_no_uuid = datetime_end_col_name_no_uuid;
		datetime_end_column.column_type = ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__DATETIMEEND_INTERNAL;
		datetime_end_column.variable_group_associated_with_current_inner_table = variable_group_child;
		datetime_end_column.uoa_associated_with_variable_group_associated_with_current_inner_table = uoa_child;
		datetime_end_column.column_name_in_original_data_table = "";
		datetime_end_column.is_within_inner_table_corresponding_to_top_level_uoa = false;
		datetime_end_column.current_multiplicity__of__current_inner_table__within__current_vg_inner_table_set =
			child_variable_group_raw_data_columns.columns_in_view.back().current_multiplicity__of__current_inner_table__within__current_vg_inner_table_set;
		datetime_end_column.number_inner_tables_in_set = child_variable_group_raw_data_columns.columns_in_view.back().number_inner_tables_in_set;

		result_columns.current_block_datetime_column_types = std::make_pair(ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__DATETIMESTART_INTERNAL,
				ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__DATETIMEEND_INTERNAL);
	}
	else
	{
		at_least_one_variable_group_has_timerange = true;
		result_columns.current_block_datetime_column_types = std::make_pair(ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__DATETIMESTART,
				ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__DATETIMEEND);
	}

	return result;

}

// DETAILED_COMMENTING: To see purpose of CreateXRTable(), search for DESCRIPTION_OF_XR_ALGORITHM
OutputModel::OutputGenerator::SqlAndColumnSet OutputModel::OutputGenerator::CreateXRTable(ColumnsInTempView const &
		previous_full_table__each_row_containing_two_sets_of_data_being_cleaned_against_one_another, int const current_multiplicity, int const primary_group_number,
		XR_TABLE_CATEGORY const xr_table_category, int const current_set_number, int const current_view_name_index)
{

	std::int64_t number_of_rows_to_sort = ObtainCount(previous_full_table__each_row_containing_two_sets_of_data_being_cleaned_against_one_another);
	current_number_rows_to_sort = number_of_rows_to_sort;

	SqlAndColumnSet result = std::make_pair(std::vector<SQLExecutor>(), ColumnsInTempView());
	std::vector<SQLExecutor> & sql_strings = result.first;
	ColumnsInTempView & result_columns = result.second;

	result_columns = previous_full_table__each_row_containing_two_sets_of_data_being_cleaned_against_one_another;
	result_columns.most_recent_sql_statement_executed__index = -1;

	std::string view_name;

	switch (xr_table_category)
	{
		case OutputModel::OutputGenerator::PRIMARY_VARIABLE_GROUP:
			{
				view_name += "NGTEMP_V";
			}
			break;

		case OutputModel::OutputGenerator::CHILD_VARIABLE_GROUP:
			{
				view_name += "NGTEMP_CV";
			}
			break;

		case OutputModel::OutputGenerator::FINAL_MERGE_OF_PRIMARY_VARIABLE_GROUP:
			{
				view_name += "NGTEMP_MF";
			}
			break;

		default:
			break;
	}

	view_name += std::to_string(primary_group_number);
	view_name += "_xr";
	view_name += std::to_string(current_view_name_index);
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
	sql_create_empty_table += "CREATE TABLE \"";
	sql_create_empty_table += result_columns.view_name;
	sql_create_empty_table += "\" AS SELECT ";

	bool first = true;
	int the_index = 0;
	std::for_each(result_columns.columns_in_view.begin(), result_columns.columns_in_view.end(), [&sql_create_empty_table, &the_index, &previous_table_column_names, &first](
					  ColumnsInTempView::ColumnInTempView & new_column)
	{
		if (!first)
		{
			sql_create_empty_table += ", ";
		}

		first = false;

		if (new_column.column_type == ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__PRIMARY && new_column.primary_key_should_be_treated_as_integer_____float_not_allowed_as_primary_key)
		{
			sql_create_empty_table += "CAST(";
		}

		sql_create_empty_table += "`";
		sql_create_empty_table += previous_table_column_names[the_index];
		sql_create_empty_table += "`";

		if (new_column.column_type == ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__PRIMARY && new_column.primary_key_should_be_treated_as_integer_____float_not_allowed_as_primary_key)
		{
			sql_create_empty_table += " AS INTEGER)";
		}

		sql_create_empty_table += " AS ";
		sql_create_empty_table += "`";
		sql_create_empty_table += new_column.column_name_in_temporary_table;
		sql_create_empty_table += "`";
		++the_index;
	});

	sql_create_empty_table += " FROM \"";
	sql_create_empty_table += previous_full_table__each_row_containing_two_sets_of_data_being_cleaned_against_one_another.view_name;
	sql_create_empty_table += "\" WHERE 0";
	sql_strings.push_back(SQLExecutor(this, db, sql_create_empty_table));

	if (failed)
	{
		SetFailureMessage(sql_error);
		return result;
	}

	if (CheckCancelled())
	{
		return result;
	}


	// Pull from the simple datetime columns at the end of the previous X table, which are guaranteed to be in place
	WidgetInstanceIdentifier variable_group =
		previous_full_table__each_row_containing_two_sets_of_data_being_cleaned_against_one_another.columns_in_view[previous_full_table__each_row_containing_two_sets_of_data_being_cleaned_against_one_another.columns_in_view.size() - 1].variable_group_associated_with_current_inner_table;
	WidgetInstanceIdentifier uoa =
		previous_full_table__each_row_containing_two_sets_of_data_being_cleaned_against_one_another.columns_in_view[previous_full_table__each_row_containing_two_sets_of_data_being_cleaned_against_one_another.columns_in_view.size() - 1].uoa_associated_with_variable_group_associated_with_current_inner_table;


	std::string datetime_start_col_name_no_uuid;

	switch (xr_table_category)
	{
		case OutputModel::OutputGenerator::PRIMARY_VARIABLE_GROUP:
			{
				datetime_start_col_name_no_uuid += "DATETIMESTART__PRIMARY_VG_INNER_TABLE_MERGE__BEFORE_DUPLICATES_REMOVED";
			}
			break;

		case OutputModel::OutputGenerator::CHILD_VARIABLE_GROUP:
			{
				datetime_start_col_name_no_uuid += "DATETIME_ROW_START_CHILD_MERGE";
			}
			break;

		case OutputModel::OutputGenerator::FINAL_MERGE_OF_PRIMARY_VARIABLE_GROUP:
			{
				datetime_start_col_name_no_uuid += "DATETIMESTART__TIMERANGE_MERGED_BETWEEN_TOP_LEVEL_PRIMARY_VARIABLE_GROUPS";
			}
			break;

		default:
			break;
	}

	std::string datetime_start_col_name = datetime_start_col_name_no_uuid;
	datetime_start_col_name += "_";
	datetime_start_col_name += newUUID(true);

	std::string alter_string;
	alter_string += "ALTER TABLE \"";
	alter_string += result_columns.view_name;
	alter_string += "\" ADD COLUMN ";
	alter_string += datetime_start_col_name;
	alter_string += " INTEGER DEFAULT 0";
	sql_strings.push_back(SQLExecutor(this, db, alter_string));

	if (failed)
	{
		SetFailureMessage(sql_error);
		return result;
	}

	if (CheckCancelled())
	{
		return result;
	}

	result_columns.columns_in_view.push_back(ColumnsInTempView::ColumnInTempView());
	ColumnsInTempView::ColumnInTempView & datetime_start_column = result_columns.columns_in_view.back();
	datetime_start_column.column_name_in_temporary_table = datetime_start_col_name;
	datetime_start_column.column_name_in_temporary_table_no_uuid = datetime_start_col_name_no_uuid;
	datetime_start_column.current_multiplicity__of__current_inner_table__within__current_vg_inner_table_set =
		previous_full_table__each_row_containing_two_sets_of_data_being_cleaned_against_one_another.columns_in_view.back().current_multiplicity__of__current_inner_table__within__current_vg_inner_table_set;
	datetime_start_column.number_inner_tables_in_set =
		previous_full_table__each_row_containing_two_sets_of_data_being_cleaned_against_one_another.columns_in_view.back().number_inner_tables_in_set;

	switch (xr_table_category)
	{
		case OutputModel::OutputGenerator::PRIMARY_VARIABLE_GROUP:
			{
				datetime_start_column.column_type = ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__DATETIMESTART__PRIMARY_VG_INNER_TABLE_MERGE__BEFORE_DUPLICATES_REMOVED;
			}
			break;

		case OutputModel::OutputGenerator::CHILD_VARIABLE_GROUP:
			{
				datetime_start_column.column_type = ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__DATETIMESTART_CHILD_MERGE;
			}
			break;

		case OutputModel::OutputGenerator::FINAL_MERGE_OF_PRIMARY_VARIABLE_GROUP:
			{
				datetime_start_column.column_type = ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__DATETIMESTART__TIMERANGE_MERGED_BETWEEN_TOP_LEVEL_PRIMARY_VARIABLE_GROUPS;
			}
			break;

		default:
			break;
	}

	datetime_start_column.variable_group_associated_with_current_inner_table = variable_group;
	datetime_start_column.uoa_associated_with_variable_group_associated_with_current_inner_table = uoa;
	datetime_start_column.column_name_in_original_data_table = "";

	switch (xr_table_category)
	{
		case OutputModel::OutputGenerator::PRIMARY_VARIABLE_GROUP:
			{
				datetime_start_column.is_within_inner_table_corresponding_to_top_level_uoa = true;
			}
			break;

		case OutputModel::OutputGenerator::CHILD_VARIABLE_GROUP:
			{
				datetime_start_column.is_within_inner_table_corresponding_to_top_level_uoa = false;
			}
			break;

		case OutputModel::OutputGenerator::FINAL_MERGE_OF_PRIMARY_VARIABLE_GROUP:
			{
				datetime_start_column.is_within_inner_table_corresponding_to_top_level_uoa = true;
			}
			break;

		default:
			break;
	}

	std::string datetime_end_col_name_no_uuid;

	switch (xr_table_category)
	{
		case OutputModel::OutputGenerator::PRIMARY_VARIABLE_GROUP:
			{
				datetime_end_col_name_no_uuid += "DATETIMEEND__PRIMARY_VG_INNER_TABLE_MERGE__BEFORE_DUPLICATES_REMOVED";
			}
			break;

		case OutputModel::OutputGenerator::CHILD_VARIABLE_GROUP:
			{
				datetime_end_col_name_no_uuid += "DATETIME_ROW_END_CHILD_MERGE";
			}
			break;

		case OutputModel::OutputGenerator::FINAL_MERGE_OF_PRIMARY_VARIABLE_GROUP:
			{
				datetime_end_col_name_no_uuid += "DATETIMEEND__TIMERANGE_MERGED_BETWEEN_TOP_LEVEL_PRIMARY_VARIABLE_GROUPS";
			}
			break;

		default:
			break;
	}

	std::string datetime_end_col_name = datetime_end_col_name_no_uuid;
	datetime_end_col_name += "_";
	datetime_end_col_name += newUUID(true);

	alter_string.clear();
	alter_string += "ALTER TABLE \"";
	alter_string += result_columns.view_name;
	alter_string += "\" ADD COLUMN ";
	alter_string += datetime_end_col_name;
	alter_string += " INTEGER DEFAULT 0";
	sql_strings.push_back(SQLExecutor(this, db, alter_string));

	if (failed)
	{
		SetFailureMessage(sql_error);
		return result;
	}

	if (CheckCancelled())
	{
		return result;
	}

	result_columns.columns_in_view.push_back(ColumnsInTempView::ColumnInTempView());
	ColumnsInTempView::ColumnInTempView & datetime_end_column = result_columns.columns_in_view.back();
	datetime_end_column.column_name_in_temporary_table = datetime_end_col_name;
	datetime_end_column.column_name_in_temporary_table_no_uuid = datetime_end_col_name_no_uuid;
	datetime_end_column.current_multiplicity__of__current_inner_table__within__current_vg_inner_table_set =
		previous_full_table__each_row_containing_two_sets_of_data_being_cleaned_against_one_another.columns_in_view.back().current_multiplicity__of__current_inner_table__within__current_vg_inner_table_set;
	datetime_end_column.number_inner_tables_in_set =
		previous_full_table__each_row_containing_two_sets_of_data_being_cleaned_against_one_another.columns_in_view.back().number_inner_tables_in_set;

	switch (xr_table_category)
	{
		case OutputModel::OutputGenerator::PRIMARY_VARIABLE_GROUP:
			{
				datetime_end_column.column_type = ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__DATETIMEEND__PRIMARY_VG_INNER_TABLE_MERGE__BEFORE_DUPLICATES_REMOVED;
				result_columns.current_block_datetime_column_types = std::make_pair(
							ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__DATETIMESTART__PRIMARY_VG_INNER_TABLE_MERGE__BEFORE_DUPLICATES_REMOVED,
							ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__DATETIMEEND__PRIMARY_VG_INNER_TABLE_MERGE__BEFORE_DUPLICATES_REMOVED);
				result_columns.previous_block_datetime_column_types = result_columns.current_block_datetime_column_types;
			}
			break;

		case OutputModel::OutputGenerator::CHILD_VARIABLE_GROUP:
			{
				datetime_end_column.column_type = ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__DATETIMEEND_CHILD_MERGE;
				result_columns.current_block_datetime_column_types = std::make_pair(ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__DATETIMESTART_CHILD_MERGE,
						ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__DATETIMEEND_CHILD_MERGE);
				result_columns.previous_block_datetime_column_types = result_columns.current_block_datetime_column_types;
			}
			break;

		case OutputModel::OutputGenerator::FINAL_MERGE_OF_PRIMARY_VARIABLE_GROUP:
			{
				datetime_end_column.column_type = ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__DATETIMEEND__TIMERANGE_MERGED_BETWEEN_TOP_LEVEL_PRIMARY_VARIABLE_GROUPS;
				result_columns.current_block_datetime_column_types = std::make_pair(
							ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__DATETIMESTART__TIMERANGE_MERGED_BETWEEN_TOP_LEVEL_PRIMARY_VARIABLE_GROUPS,
							ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__DATETIMEEND__TIMERANGE_MERGED_BETWEEN_TOP_LEVEL_PRIMARY_VARIABLE_GROUPS);
				result_columns.previous_block_datetime_column_types = result_columns.current_block_datetime_column_types;
			}
			break;

		default:
			break;
	}

	datetime_end_column.variable_group_associated_with_current_inner_table = variable_group;
	datetime_end_column.uoa_associated_with_variable_group_associated_with_current_inner_table = uoa;
	datetime_end_column.column_name_in_original_data_table = "";

	switch (xr_table_category)
	{
		case OutputModel::OutputGenerator::PRIMARY_VARIABLE_GROUP:
			{
				datetime_end_column.is_within_inner_table_corresponding_to_top_level_uoa = true;
			}
			break;

		case OutputModel::OutputGenerator::CHILD_VARIABLE_GROUP:
			{
				datetime_end_column.is_within_inner_table_corresponding_to_top_level_uoa = false;
			}
			break;

		case OutputModel::OutputGenerator::FINAL_MERGE_OF_PRIMARY_VARIABLE_GROUP:
			{
				datetime_end_column.is_within_inner_table_corresponding_to_top_level_uoa = true;
			}
			break;

		default:
			break;
	}


	// ***************************************************************************************************************************************** //
	// Locate the indices of the datetime columns corresponding to the currently-being-added new inner table
	// (its columns are at the end, and the final two columns are always its datetime-start and datetime-end columns)
	// ... and corresponding to the previous "row" being merged into - which corresponds to all the columns at the left
	// (and the final two of THESE columns - which the algorithm below searches for - are ITS datetime-start and datetime-end columns).
	// ***************************************************************************************************************************************** //

	int previous_datetime_start_column_index = -1;
	int previous_datetime_end_column_index = -1;
	int current_datetime_start_column_index = -1;
	int current_datetime_end_column_index = -1;
	FindDatetimeIndices(previous_full_table__each_row_containing_two_sets_of_data_being_cleaned_against_one_another, previous_datetime_start_column_index,
						previous_datetime_end_column_index, current_datetime_start_column_index, current_datetime_end_column_index, xr_table_category);

	ExecuteSQL(result); // Executes all SQL queries up to the current one

	if (failed || CheckCancelled())
	{
		return result;
	}



	// ***************************************************************************************************************************************** //
	// DESCRIPTION_OF_XR_ALGORITHM
	// Now step through every row to merge the first set of columns (at the left of each row)
	// with the new set of columns (at the right of each row).
	// Specifically check for TIME RANGE OVERLAP - that is the entire purpose of this loop.
	// If the time ranges do not match, then create two new separate rows,
	// one containing only the data from the left columns (with NULL for the remaining columns),
	// and one containing only the data from the right columns (with NULL for the remaining columns).
	// Further, if the left columns are NULL, then swap the position of the right columns to place them at the left,
	// since all column sets being merged together have the same schema (albeit the columns at the left may have
	// more than one block of columns with this schema - i.e., multiple "inner tables".
	// For overlapping or invalid time range overlaps, handle those cases appropriately, adding between 0 and 3 new rows.
	// ***************************************************************************************************************************************** //

	{

		BOOST_SCOPE_EXIT(this_)
		{
			this_->CloseObtainData();
		} BOOST_SCOPE_EXIT_END

		ObtainData(previous_full_table__each_row_containing_two_sets_of_data_being_cleaned_against_one_another);

		if (failed || CheckCancelled())
		{
			return result;
		}


		int const minimum_desired_rows_per_transaction = 1024 * 16;

		std::int64_t current_rows_added = 0;
		std::int64_t current_rows_stepped = 0;
		std::int64_t current_rows_added_since_execution = 0;
		std::string sql_add_xr_row;
		bool first_row_added = true;
		std::vector<std::string> bound_parameter_strings;
		std::vector<std::int64_t> bound_parameter_ints;
		std::vector<long double> bound_parameter_floats;
		std::vector<SQLExecutor::WHICH_BINDING> bound_parameter_which_binding_to_use;

		sqlite3_stmt *& the_prepared_stmt = SQLExecutor::stmt_insert;
		std::shared_ptr<bool> statement_is_prepared(std::make_shared<bool>(false));
		SQLExecutor::stmt_insert = nullptr;
		sqlite3_stmt *& the_stmt__ = SQLExecutor::stmt_insert;
		BOOST_SCOPE_EXIT(&the_prepared_stmt, &statement_is_prepared, &the_stmt__)
		{
			if (the_prepared_stmt && *statement_is_prepared)
			{
				sqlite3_finalize(the_prepared_stmt);
				++SQLExecutor::number_statement_finalizes;
				the_prepared_stmt = nullptr;
				*statement_is_prepared = false;
			}

			the_stmt__ = nullptr;
		} BOOST_SCOPE_EXIT_END

		BeginNewTransaction();

		// To deal with multiple rows with the same ID that are being merged in, but some with proper date range and some without,
		// add a little cache loop here...
		std::vector<TimeRangeSorter> rows_to_check_for_duplicates_in_newly_joined_primary_key_columns;
		SavedRowData current_row_of_data;
		bool use_newest_row_index = false;
		int which_previous_row_index_to_test_against = 0;
		std::vector<std::tuple<bool, bool, std::pair<std::int64_t, std::int64_t>>> row_inserts_info;
		std::int64_t datetime_range_start = 0;
		std::int64_t datetime_range_end = 0;
		bool include_current_data = false;
		bool include_previous_data = false;

		TimeRangesForIndividualGroup_IntKeys multiplicity_one_time_ranges__intkeys;
		TimeRangesForIndividualGroup_FloatKeys multiplicity_one_time_ranges__floatkeys;
		TimeRangesForIndividualGroup_StringKeys multiplicity_one_time_ranges__stringkeys;
		std::vector<SavedRowData> saved_rows_with_null_in_final_inner_table;

		while (StepData())
		{

			current_row_of_data.PopulateFromCurrentRowInDatabase(previous_full_table__each_row_containing_two_sets_of_data_being_cleaned_against_one_another, stmt_result, xr_table_category);

			if (failed || CheckCancelled())
			{
				break;
			}

			if (xr_table_category == XR_TABLE_CATEGORY::PRIMARY_VARIABLE_GROUP)
			{

				bool any_primary_key_groups_are_null = false;
				std::for_each(current_row_of_data.indices_of_all_primary_key_columns_in_all_but_final_inner_table.cbegin(),
							  current_row_of_data.indices_of_all_primary_key_columns_in_all_but_final_inner_table.cend(), [&any_primary_key_groups_are_null](
								  std::pair<SQLExecutor::WHICH_BINDING, std::pair<int, int>> const & binding_info)
				{
					if (binding_info.first == SQLExecutor::NULL_BINDING)
					{
						any_primary_key_groups_are_null = true;
					}
				});

				if (any_primary_key_groups_are_null)
				{

					// ******************************************************************************************************************* //
					// a previous iteration of the multiplicity merge was unable to find a match, so we won't be able to, either
					// ******************************************************************************************************************* //


					// ... use the datetime read from the database as the new datetime

					current_row_of_data.SetFinalInnerTableToNull(true);
					std::int64_t datetime_start = current_row_of_data.datetime_start;
					std::int64_t datetime_end = current_row_of_data.datetime_end;
					bool skip_check = false;

					if (datetime_start == 0 && datetime_end == 0)
					{
						skip_check = true;
					}

					if (!skip_check)
					{
						if (datetime_start < timerange_start)
						{
							datetime_start = timerange_start;
						}

						if (datetime_end > timerange_end)
						{
							datetime_end = timerange_end;
						}
					}

					bool added = CreateNewXRRow(current_row_of_data, first_row_added, datetime_start_col_name, datetime_end_col_name, result_columns.view_name, sql_add_xr_row, bound_parameter_strings,
												bound_parameter_ints, bound_parameter_floats, bound_parameter_which_binding_to_use, datetime_start, datetime_end,
												previous_full_table__each_row_containing_two_sets_of_data_being_cleaned_against_one_another, result_columns, true, true, xr_table_category, false, false);

					if (failed || CheckCancelled())
					{
						break;
					}

					if (added)
					{
						SQLExecutor::Execute(false, SQLExecutor::DOES_NOT_RETURN_ROWS, this, db, sql_add_xr_row, bound_parameter_strings, bound_parameter_ints, bound_parameter_floats,
											 bound_parameter_which_binding_to_use, statement_is_prepared, the_prepared_stmt, true);

						if (failed)
						{
							SetFailureMessage(sql_error);
							return result;
						}

						if (CheckCancelled())
						{
							return result;
						}

						the_prepared_stmt = SQLExecutor::stmt_insert;
						++current_rows_added;
						++current_rows_added_since_execution;
					}

					++current_rows_stepped;
					continue;

				}

				if (rows_to_check_for_duplicates_in_newly_joined_primary_key_columns.empty())
				{
					rows_to_check_for_duplicates_in_newly_joined_primary_key_columns.push_back(TimeRangeSorter(current_row_of_data));
					++current_rows_stepped;
					continue;
				}

				// Final argument to the following function ensures that the new data being appended will be skipped in the primary key check
				// ... this way, we can gather up all rows that match only on the previous data, and do some pre-processing
				// to eliminate some edge-case redundant rows,
				// and to help optimize the algorithm by clearing out some known duplicates here rather than waiting for a later stage
				// to clear them out.
				// See comments inside the function.
				use_newest_row_index = false;
				bool primary_keys_match = TestPrimaryKeyMatch(current_row_of_data,
										  rows_to_check_for_duplicates_in_newly_joined_primary_key_columns[which_previous_row_index_to_test_against].the_data_row_to_be_sorted__with_guaranteed_primary_key_match_on_all_but_last_inner_table,
										  use_newest_row_index,
										  // Note: important 4th parameter: See note above
										  PRIMARY_KEY_MATCH_CONDITION::MATCH_ON_ALL_BUT_FINAL_INNER_TABLE);

				if (failed || CheckCancelled())
				{
					break;
				}

				if (primary_keys_match)
				{
					rows_to_check_for_duplicates_in_newly_joined_primary_key_columns.push_back(TimeRangeSorter(current_row_of_data));

					if (use_newest_row_index)
					{
						which_previous_row_index_to_test_against = (int)rows_to_check_for_duplicates_in_newly_joined_primary_key_columns.size() - 1;
						use_newest_row_index = false;
					}

					++current_rows_stepped;

					if (current_rows_stepped % 100 == 0 || current_rows_stepped == current_number_rows_to_sort)
					{
						UpdateProgressBarValue(messager, current_rows_stepped);
					}

					continue;
				}
				else
				{

					// ******************************************************************************************************************* //
					// Special processing: reorder inner tables in sorted primary key order,
					// and split rows to handle the time range overlap of the new data being joined.
					// ******************************************************************************************************************* //

					// The previous rows (not the current) match on all primary keys except those in the final inner table.
					// Note that the matching function (TestPrimaryKeyMatch(), above) does not care if there are some
					// extra NULL primary keys (and fewer non-NULL primary keys) in all but the final inner table
					// when performing the comparison; unlike the operator<() in the TimeRangeSorter
					// used in the sort operation, below.
					//
					// Therefore, the only possible difference in the rows in this vector
					// are the date range, and the primary key of the final inner table.

					use_newest_row_index = false;
					which_previous_row_index_to_test_against = 0;

					// This function takes advantage of the operator<() (comparison operator) of the TimeRangeSorter class
					// to sort the elements in the array first by primary keys in all but the final inner table
					//     (in this case, rows are considered NOT to match if there are a different number of
					//      non-NULL primary keys in all inner tables but the last)
					std::sort(rows_to_check_for_duplicates_in_newly_joined_primary_key_columns.begin(), rows_to_check_for_duplicates_in_newly_joined_primary_key_columns.end());

					HandleCompletionOfProcessingOfNormalizedGroupOfMatchingRowsInXRalgorithm(saved_rows_with_null_in_final_inner_table, multiplicity_one_time_ranges__intkeys,
							multiplicity_one_time_ranges__floatkeys, multiplicity_one_time_ranges__stringkeys, rows_to_check_for_duplicates_in_newly_joined_primary_key_columns,
							previous_datetime_start_column_index, current_datetime_start_column_index, previous_datetime_end_column_index, current_datetime_end_column_index, xr_table_category, sql_strings,
							the_prepared_stmt, statement_is_prepared, current_rows_added, current_rows_added_since_execution, first_row_added, datetime_start_col_name, datetime_end_col_name, result_columns,
							sql_add_xr_row, bound_parameter_strings, bound_parameter_ints, bound_parameter_floats, bound_parameter_which_binding_to_use, datetime_range_start, datetime_range_end,
							previous_full_table__each_row_containing_two_sets_of_data_being_cleaned_against_one_another);

					if (failed || CheckCancelled())
					{
						break;
					}

					// Also check if the multiplicity 1 keys have changed
					bool primary_keys_match_on_multiplicity_1 = TestPrimaryKeyMatch(current_row_of_data,
							rows_to_check_for_duplicates_in_newly_joined_primary_key_columns[which_previous_row_index_to_test_against].the_data_row_to_be_sorted__with_guaranteed_primary_key_match_on_all_but_last_inner_table,
							use_newest_row_index,
							// Note: important 4th parameter: See note above
							PRIMARY_KEY_MATCH_CONDITION::MATCH_ON_ALL_MULTIPLICITY_1_KEYS);
					//PRIMARY_KEY_MATCH_CONDITION::MATCH_ON_ALL_BUT_FINAL_TWO_INNER_TABLES);

					rows_to_check_for_duplicates_in_newly_joined_primary_key_columns.clear();
					rows_to_check_for_duplicates_in_newly_joined_primary_key_columns.push_back(TimeRangeSorter(current_row_of_data));

					bool perform_null_checks = false;

					if (!primary_keys_match_on_multiplicity_1)
					{
						perform_null_checks = true;
					}

					// The following cannot be made to work as long as there is the possibility of multiple rows with the same key (except for time range)
					if (false)
					{
						if (!saved_rows_with_null_in_final_inner_table.empty())
						{
							SavedRowData const & test_highest_saved_row_with_a_null_primary_key = saved_rows_with_null_in_final_inner_table.back();

							// See comments at bottom of following function for some hints about what is happening here
							bool previous_row_with_nulls_cannot_be_matched_by_any_following_rows = test_highest_saved_row_with_a_null_primary_key.TestLessEqual(current_row_of_data, *this);

							if (previous_row_with_nulls_cannot_be_matched_by_any_following_rows)
							{
								perform_null_checks = true;
							}
						}
						else
						{
							perform_null_checks = true;
						}
					}

					if (perform_null_checks)
					{
						EliminateRedundantNullsInFinalInnerTable(saved_rows_with_null_in_final_inner_table, multiplicity_one_time_ranges__intkeys, multiplicity_one_time_ranges__floatkeys,
								multiplicity_one_time_ranges__stringkeys);

						if (failed || CheckCancelled())
						{
							return result;
						}

						AddRowsToXRTable(saved_rows_with_null_in_final_inner_table, sql_strings, the_prepared_stmt, statement_is_prepared, current_rows_added, current_rows_added_since_execution,
										 first_row_added, datetime_start_col_name, datetime_end_col_name, result_columns, sql_add_xr_row, bound_parameter_strings, bound_parameter_ints, bound_parameter_floats,
										 bound_parameter_which_binding_to_use, datetime_range_start, datetime_range_end, previous_full_table__each_row_containing_two_sets_of_data_being_cleaned_against_one_another);

						if (failed || CheckCancelled())
						{
							return result;
						}

						multiplicity_one_time_ranges__intkeys.clear();
						multiplicity_one_time_ranges__floatkeys.clear();
						multiplicity_one_time_ranges__stringkeys.clear();
						saved_rows_with_null_in_final_inner_table.clear();

						if (failed || CheckCancelled())
						{
							return result;
						}
					}

					//ExecuteSQL(result);

					if (failed || CheckCancelled())
					{
						break;
					}

					if (current_rows_added_since_execution >= minimum_desired_rows_per_transaction)
					{
						boost::format msg("Processed %1% of %2% temporary rows this stage: performing transaction");
						msg % current_rows_stepped % current_number_rows_to_sort;
						messager.SetPerformanceLabel(msg.str());
						EndTransaction();
						BeginNewTransaction();
						current_rows_added_since_execution = 0;
					}

					++current_rows_stepped;

					if (current_rows_stepped % 100 == 0 || current_rows_stepped == current_number_rows_to_sort)
					{
						UpdateProgressBarValue(messager, current_rows_stepped);
					}

					continue;

				}

			}
			else
			{

				row_inserts_info.clear();
				PopulateSplitRowInfo_FromCurrentMergingColumns(row_inserts_info, previous_datetime_start_column_index, current_datetime_start_column_index, previous_datetime_end_column_index,
						current_datetime_end_column_index, current_row_of_data, xr_table_category);
				std::for_each(row_inserts_info.cbegin(),
							  row_inserts_info.cend(), [this, &sql_strings, &current_rows_added, &current_rows_added_since_execution, &statement_is_prepared, &the_prepared_stmt, &current_row_of_data, &first_row_added, &include_current_data, &include_previous_data, &datetime_start_col_name, &datetime_end_col_name, &datetime_range_start, &datetime_range_end, &result_columns, &sql_add_xr_row, &bound_parameter_strings, &bound_parameter_ints, &bound_parameter_floats, &bound_parameter_which_binding_to_use, &previous_full_table__each_row_containing_two_sets_of_data_being_cleaned_against_one_another, &xr_table_category](
								  std::tuple<bool, bool, std::pair<std::int64_t, std::int64_t>> const & row_insert_info)
				{

					if (failed || CheckCancelled())
					{
						return;
					}

					bool added = false;
					datetime_range_start = std::get<2>(row_insert_info).first;
					datetime_range_end = std::get<2>(row_insert_info).second;
					include_current_data = std::get<0>(row_insert_info);
					include_previous_data = std::get<1>(row_insert_info);
					bool skip_check = false;

					if (datetime_range_start == 0 && datetime_range_end == 0)
					{
						skip_check = true;
					}

					if (!skip_check)
					{
						if (datetime_range_start < timerange_start)
						{
							datetime_range_start = timerange_start;
						}

						if (datetime_range_end > timerange_end)
						{
							datetime_range_end = timerange_end;
						}
					}

					added = CreateNewXRRow(current_row_of_data, first_row_added, datetime_start_col_name, datetime_end_col_name, result_columns.view_name, sql_add_xr_row, bound_parameter_strings,
										   bound_parameter_ints, bound_parameter_floats, bound_parameter_which_binding_to_use, datetime_range_start, datetime_range_end,
										   previous_full_table__each_row_containing_two_sets_of_data_being_cleaned_against_one_another, result_columns, include_previous_data, include_current_data, xr_table_category, false,
										   false);

					if (failed || CheckCancelled())
					{
						return;
					}

					if (added)
					{
						SQLExecutor::Execute(false, SQLExecutor::DOES_NOT_RETURN_ROWS, this, db, sql_add_xr_row, bound_parameter_strings, bound_parameter_ints, bound_parameter_floats,
											 bound_parameter_which_binding_to_use, statement_is_prepared, SQLExecutor::stmt_insert, true);

						if (failed)
						{
							SetFailureMessage(sql_error);
							return;
						}

						if (CheckCancelled())
						{
							return;
						}

						the_prepared_stmt = SQLExecutor::stmt_insert;
						++current_rows_added;
						++current_rows_added_since_execution;
					}

				});

			}

			if (failed || CheckCancelled())
			{
				return result;
			}

			if (failed || CheckCancelled())
			{
				return result;
			}

			if (current_rows_added_since_execution >= minimum_desired_rows_per_transaction)
			{
				boost::format msg("Processed %1% of %2% temporary rows this stage: performing transaction");
				msg % current_rows_stepped % current_number_rows_to_sort;
				messager.SetPerformanceLabel(msg.str());
				EndTransaction();
				BeginNewTransaction();
				current_rows_added_since_execution = 0;
			}

			++current_rows_stepped;

			if (current_rows_stepped % 100 == 0 || current_rows_stepped == current_number_rows_to_sort)
			{
				UpdateProgressBarValue(messager, current_rows_stepped);
			}

		}

		if (failed || CheckCancelled())
		{
			return result;
		}

		if (xr_table_category == XR_TABLE_CATEGORY::PRIMARY_VARIABLE_GROUP)
		{
			HandleCompletionOfProcessingOfNormalizedGroupOfMatchingRowsInXRalgorithm(saved_rows_with_null_in_final_inner_table, multiplicity_one_time_ranges__intkeys,
					multiplicity_one_time_ranges__floatkeys, multiplicity_one_time_ranges__stringkeys, rows_to_check_for_duplicates_in_newly_joined_primary_key_columns,
					previous_datetime_start_column_index, current_datetime_start_column_index, previous_datetime_end_column_index, current_datetime_end_column_index, xr_table_category, sql_strings,
					the_prepared_stmt, statement_is_prepared, current_rows_added, current_rows_added_since_execution, first_row_added, datetime_start_col_name, datetime_end_col_name, result_columns,
					sql_add_xr_row, bound_parameter_strings, bound_parameter_ints, bound_parameter_floats, bound_parameter_which_binding_to_use, datetime_range_start, datetime_range_end,
					previous_full_table__each_row_containing_two_sets_of_data_being_cleaned_against_one_another);

			if (failed || CheckCancelled())
			{
				return result;
			}

			EliminateRedundantNullsInFinalInnerTable(saved_rows_with_null_in_final_inner_table, multiplicity_one_time_ranges__intkeys, multiplicity_one_time_ranges__floatkeys,
					multiplicity_one_time_ranges__stringkeys);

			if (failed || CheckCancelled())
			{
				return result;
			}

			AddRowsToXRTable(saved_rows_with_null_in_final_inner_table, sql_strings, the_prepared_stmt, statement_is_prepared, current_rows_added, current_rows_added_since_execution,
							 first_row_added, datetime_start_col_name, datetime_end_col_name, result_columns, sql_add_xr_row, bound_parameter_strings, bound_parameter_ints, bound_parameter_floats,
							 bound_parameter_which_binding_to_use, datetime_range_start, datetime_range_end, previous_full_table__each_row_containing_two_sets_of_data_being_cleaned_against_one_another);

			if (failed || CheckCancelled())
			{
				return result;
			}

			multiplicity_one_time_ranges__intkeys.clear();
			multiplicity_one_time_ranges__floatkeys.clear();
			multiplicity_one_time_ranges__stringkeys.clear();
			saved_rows_with_null_in_final_inner_table.clear();
			rows_to_check_for_duplicates_in_newly_joined_primary_key_columns.clear();

			if (failed || CheckCancelled())
			{
				return result;
			}
		}

		//ExecuteSQL(result);

		messager.UpdateProgressBarValue(1000);
		boost::format msg("Processed %1% of %2% temporary rows this stage: performing transaction");
		msg % current_rows_stepped % current_number_rows_to_sort;
		messager.SetPerformanceLabel(msg.str());
		EndTransaction();

		if (failed || CheckCancelled())
		{
			return result;
		}

	}

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

	tables_deleted.clear();

	initialized = true;

	std::pair<bool, std::int64_t> info_random_sampling = model->t_general_options.getRandomSamplingInfo(model->getDb());
	random_sampling = info_random_sampling.first;
	random_sampling_number_rows = info_random_sampling.second;

	if (random_sampling && (random_sampling_number_rows <= 0))
	{
		boost::format msg("You have selected random sampling, but the number of desired rows to generate is 0.");
		SetFailureMessage(msg.str());
		failed = true;
	}

	PopulateUOAs();

	if (failed || CheckCancelled())
	{
		return;
	}

	PopulateDMUCounts();

	if (failed || CheckCancelled())
	{
		return;
	}

	ValidateUOAs();

	if (failed || CheckCancelled())
	{
		return;
	}

	DetermineChildMultiplicitiesGreaterThanOne();

	if (failed || CheckCancelled())
	{
		return;
	}

	PopulateVariableGroups();

	top_level_vg_index = 0;
	if (primary_variable_groups_vector.size() > 1)
	{
		if (true) // if (random_sampling)
		{
			if (true)
			{
				std::vector<WidgetInstanceIdentifier> variableGroupOptions;
				std::for_each(primary_variable_groups_vector.cbegin(), primary_variable_groups_vector.cend(), [&](std::pair<WidgetInstanceIdentifier, WidgetInstanceIdentifiers> const & vg_to_selected)
				{
					variableGroupOptions.push_back(vg_to_selected.first);
				});
				boost::format msgTitle("Select top-level variable group");
				boost::format msgQuestion("There are multiple variable groups with the same set of unit of analysis fields that might be used as the \"primary\" variable group for the run.  Please select one to use as the primary variable group for this run:");

				// 0-based
				top_level_vg_index = static_cast<size_t>(messager.ShowOptionMessageBox(msgTitle.str(), msgQuestion.str(), variableGroupOptions));

				if (top_level_vg_index == -1)
				{
					// user cancelled
					failed = true;
					return;
				}
				//top_level_vg_index = 0;
			}
			else
			{
				boost::format msg("Multiple top-level variable groups are not currently supported.");
				SetFailureMessage(msg.str());
				failed = true;
			}
		}
		else
		{
			boost::format
				msg("Because you have multiple top-level variable groups selected, it is possible that some rows over the same time range will contain data for one (or more) but not all of these variable groups, and not the others.  However, a *subset* of data in the other variable groups might have valid data over this time range.  In this case, that valid data will appear as NULL in the output for this run.  This is probably minor, but to prevent this, perform a run with only one top-level variable group selected.");
			bool do_continue = messager.ShowQuestionMessageBox(std::string("Continue?"), msg.str());

			if (!do_continue)
			{
				cancelled = true;
				return;
			}
		}
	}

	if (failed || CheckCancelled())
	{
		return;
	}

	PopulatePrimaryKeySequenceInfo();

	if (failed || CheckCancelled())
	{
		return;
	}

}

void OutputModel::OutputGenerator::ObtainColumnInfoForRawDataTables()
{

	int primary_view_count = 0;
	int primary_or_secondary_view_index = 0;
	std::for_each(primary_variable_groups_vector.cbegin(),
				  primary_variable_groups_vector.cend(), [&](std::pair<WidgetInstanceIdentifier, WidgetInstanceIdentifiers> const & the_primary_variable_group)
	{
		PopulateColumnsFromRawDataTable(the_primary_variable_group, primary_view_count, primary_variable_groups_column_info, true, primary_or_secondary_view_index);
		++primary_or_secondary_view_index;
	});

	int secondary_view_count = 0;
	primary_or_secondary_view_index = 0;
	std::for_each(secondary_variable_groups_vector.cbegin(),
				  secondary_variable_groups_vector.cend(), [&](std::pair<WidgetInstanceIdentifier, WidgetInstanceIdentifiers> const & the_secondary_variable_group)
	{
		PopulateColumnsFromRawDataTable(the_secondary_variable_group, secondary_view_count, secondary_variable_groups_column_info, false, primary_or_secondary_view_index);
		++primary_or_secondary_view_index;
	});

}

void OutputModel::OutputGenerator::PopulateColumnsFromRawDataTable(std::pair<WidgetInstanceIdentifier, WidgetInstanceIdentifiers> const & the_variable_group, int view_count,
	std::vector<ColumnsInTempView> & variable_groups_column_info, bool const & is_primary, int const primary_or_secondary_view_index)
{

	// ************************************************************************************************ //
	// the_variable_group:
	// A pair: VG identifier -> Variables in this group selected by the user - but the latter is UNUSED.
	// Note that even though only the variables selected by the user appear as the second member
	// of the pair, that nonetheless in this function we bypass this data structure
	// and retrieve the *full* set of columns from the vg_set_member table
	// via the VG identifier.
	// 
	// Truly! The purpose of this function is to populate the THIRD incoming argument
	// with ALL of the fields in the table, not just those selected by the user.
	// ************************************************************************************************ //

	// Convert column metadata into a far more useful form for construction of K-adic output

	if (failed || CheckCancelled())
	{
		return;
	}

	std::string vg_code = *the_variable_group.first.code;
	std::string vg_data_table_name = Table_VariableGroupData::TableNameFromVGCode(vg_code);

	variable_groups_column_info.push_back(ColumnsInTempView());
	ColumnsInTempView & columns_in_variable_group_view = variable_groups_column_info.back();

	if (is_primary)
	{
		if (primary_or_secondary_view_index == top_level_vg_index)
		{
			columns_in_variable_group_view.schema_type = ColumnsInTempView::SCHEMA_TYPE__RAW__SELECTED_VARIABLES_PRIMARY;
		}
		else
		{
			columns_in_variable_group_view.schema_type = ColumnsInTempView::SCHEMA_TYPE__RAW__SELECTED_VARIABLES_TOP_LEVEL_NOT_PRIMARY;
		}
	}
	else
	{
		columns_in_variable_group_view.schema_type = ColumnsInTempView::SCHEMA_TYPE__RAW__SELECTED_VARIABLES_CHILD;
	}



	columns_in_variable_group_view.view_number = view_count;
	std::string view_name;
	view_name = vg_data_table_name;
	columns_in_variable_group_view.view_name_no_uuid = view_name;
	columns_in_variable_group_view.view_name = view_name;

	columns_in_variable_group_view.original_table_names.push_back(vg_data_table_name);
	columns_in_variable_group_view.variable_group_codes.push_back(*the_variable_group.first.code);
	columns_in_variable_group_view.variable_group_longhand_names.push_back(*the_variable_group.first.longhand);
	columns_in_variable_group_view.variable_groups.push_back(the_variable_group.first);

	WidgetInstanceIdentifiers variables_in_group = input_model->t_vgp_setmembers.getIdentifiers(*the_variable_group.first.uuid);
	WidgetInstanceIdentifiers variables_in_group_primary_keys_metadata = input_model->t_vgp_data_metadata__primary_keys.getIdentifiers(vg_data_table_name);

	std::set<WidgetInstanceIdentifier> variables_in_group_sorted;
	std::for_each(variables_in_group.cbegin(), variables_in_group.cend(), [&variables_in_group_sorted](WidgetInstanceIdentifier const & variable_group_set_member)
	{
		variables_in_group_sorted.insert(variable_group_set_member);
	});

	WidgetInstanceIdentifiers datetime_columns = input_model->t_vgp_data_metadata__datetime_columns.getIdentifiers(vg_data_table_name);

	if (datetime_columns.size() > 0 && datetime_columns.size() != 2)
	{
		boost::format msg("The number of datetime columns in the raw data tables in the database is not either 0 or 2 (table %1%)");
		msg % vg_data_table_name;
		SetFailureMessage(msg.str());
		failed = true;
		return;
	}

	Table_UOA_Identifier::DMU_Counts dmu_counts_corresponding_to_uoa_for_current_primary_or_child_variable_group;
	std::for_each(biggest_counts.cbegin(), biggest_counts.cend(), [&dmu_counts_corresponding_to_uoa_for_current_primary_or_child_variable_group, &the_variable_group](
					  std::pair<WidgetInstanceIdentifier, Table_UOA_Identifier::DMU_Counts> const & uoa_plus_dmu_counts)
	{
		if (uoa_plus_dmu_counts.first.IsEqual(WidgetInstanceIdentifier::EQUALITY_CHECK_TYPE__STRING_CODE, *the_variable_group.first.identifier_parent))
		{
			dmu_counts_corresponding_to_uoa_for_current_primary_or_child_variable_group = uoa_plus_dmu_counts.second;
		}
	});
	std::for_each(child_counts.cbegin(), child_counts.cend(), [&dmu_counts_corresponding_to_uoa_for_current_primary_or_child_variable_group, &the_variable_group](
					  std::pair<WidgetInstanceIdentifier, Table_UOA_Identifier::DMU_Counts> const & uoa_plus_dmu_counts)
	{
		if (uoa_plus_dmu_counts.first.IsEqual(WidgetInstanceIdentifier::EQUALITY_CHECK_TYPE__STRING_CODE, *the_variable_group.first.identifier_parent))
		{
			dmu_counts_corresponding_to_uoa_for_current_primary_or_child_variable_group = uoa_plus_dmu_counts.second;
		}
	});

	Table_UOA_Identifier::DMU_Counts & dmu_counts_corresponding_to_top_level_uoa = biggest_counts[0].second;

	columns_in_variable_group_view.has_no_datetime_columns = false;
	columns_in_variable_group_view.has_no_datetime_columns_originally = false;

	if (datetime_columns.empty())
	{
		columns_in_variable_group_view.has_no_datetime_columns = true;
		columns_in_variable_group_view.has_no_datetime_columns_originally = true;
	}
	else
	{
		columns_in_variable_group_view.current_block_datetime_column_types = std::make_pair(ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__DATETIMESTART,
				ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__DATETIMEEND);
		columns_in_variable_group_view.previous_block_datetime_column_types = columns_in_variable_group_view.current_block_datetime_column_types;
	}



	std::for_each(variables_in_group_sorted.cbegin(),
				  variables_in_group_sorted.cend(), [this, &vg_data_table_name, &dmu_counts_corresponding_to_top_level_uoa, &dmu_counts_corresponding_to_uoa_for_current_primary_or_child_variable_group, &is_primary, &columns_in_variable_group_view, &datetime_columns, &the_variable_group, &variables_in_group_primary_keys_metadata](
					  WidgetInstanceIdentifier const & variable_group_set_member)
	{
		columns_in_variable_group_view.columns_in_view.push_back(ColumnsInTempView::ColumnInTempView());
		ColumnsInTempView::ColumnInTempView & column_in_variable_group_data_table = columns_in_variable_group_view.columns_in_view.back();

		std::string column_name_no_uuid = *variable_group_set_member.code;
		column_in_variable_group_data_table.column_name_in_temporary_table_no_uuid = column_name_no_uuid;

		std::string column_name = column_name_no_uuid;

		column_in_variable_group_data_table.column_name_in_temporary_table = column_name;

		column_in_variable_group_data_table.column_name_in_original_data_table = column_name_no_uuid;

		column_in_variable_group_data_table.variable_group_associated_with_current_inner_table = the_variable_group.first;

		column_in_variable_group_data_table.current_multiplicity__of__current_inner_table__within__current_vg_inner_table_set = 1;

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
		std::for_each(variables_in_group_primary_keys_metadata.cbegin(),
					  variables_in_group_primary_keys_metadata.cend(), [&column_in_variable_group_data_table, &primary_key_field](WidgetInstanceIdentifier const & primary_key_in_variable_group_metadata)
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


		// Populate primary key column data, for those columns that are primary keys.
		// The logic is a bit tricky, but works... See comments above, and below, for context.

		int number_inner_tables = 0;
		std::for_each(sequence.primary_key_sequence_info.cbegin(),
					  sequence.primary_key_sequence_info.cend(), [this, &number_inner_tables, &dmu_counts_corresponding_to_top_level_uoa, &dmu_counts_corresponding_to_uoa_for_current_primary_or_child_variable_group, &the_variable_group, &column_in_variable_group_data_table, &variables_in_group_primary_keys_metadata](
						  PrimaryKeySequence::PrimaryKeySequenceEntry const & primary_key_entry__output__including_multiplicities)
		{

			int k_count__corresponding_to_top_level_uoa__and_current_dmu_category;
			std::for_each(dmu_counts_corresponding_to_top_level_uoa.cbegin(),
						  dmu_counts_corresponding_to_top_level_uoa.cend(), [&k_count__corresponding_to_top_level_uoa__and_current_dmu_category, &primary_key_entry__output__including_multiplicities](
							  Table_UOA_Identifier::DMU_Plus_Count const & dmu_plus_count)
			{
				if (dmu_plus_count.first.IsEqual(WidgetInstanceIdentifier::EQUALITY_CHECK_TYPE__STRING_CODE, primary_key_entry__output__including_multiplicities.dmu_category))
				{
					k_count__corresponding_to_top_level_uoa__and_current_dmu_category = dmu_plus_count.second;
				}
			});

			// Determine those columns in the RAW DATA TABLE
			//    (only includes one instance of columns with multiplicity > 1)
			// which correspond to primary keys, and if so, WHICH of the primary keys
			// in the FULL SEQUENCE they correspond to (includes multiplicity > 1,
			//     so single columns in the raw data table match to multiple columns in the full sequence)

			// Loop through all variable groups that are associated with this column in the final output sequence of columns.
			// I.e., if the primary UOA has 4 CTY columns and the user sets K=8 for CTY for the output,
			// and a child variable group UOA has 2 CTY columns for (necessarily the same) 8 output columns,
			// then each CTY column corresponds to *two* variable groups -
			// in this case, the single top-level (primary) variable group, and the child variable group.
			// In the first case (the top-level variable group with a UOA with 4 CTY columns),
			// the first four CTY columns in the output are the first inner table, and the second four are the second.
			// In the second case (the child variable group with a UOA with 2 CTY columns),
			// the first two CTY columns in the output are the first inner table, the second two
			// are the second inner table, the third two are the third inner table,
			// and the fourth two are the fourth inner table.
			// Note that in this example, the 3rd and 4th columns in the final output
			// correspond both to the first inner table for the top-level primary variable group,
			// and the second inner table for the child variable group -
			// all in one and the same columns.
			// You can see that each column therefore corresponds to possibly *multiple* variable groups,
			// and correspondingly for each such VG one and the same column can be a member of a different inner table.
			std::for_each(primary_key_entry__output__including_multiplicities.variable_group_info_for_primary_keys.cbegin(),
						  primary_key_entry__output__including_multiplicities.variable_group_info_for_primary_keys.cend(), [this, &number_inner_tables, &k_count__corresponding_to_top_level_uoa__and_current_dmu_category, &dmu_counts_corresponding_to_top_level_uoa, &dmu_counts_corresponding_to_uoa_for_current_primary_or_child_variable_group, &the_variable_group, &column_in_variable_group_data_table, &primary_key_entry__output__including_multiplicities, &variables_in_group_primary_keys_metadata](
							  PrimaryKeySequence::VariableGroup_PrimaryKey_Info const & current_variable_group_primary_key_entry)
			{
				if (current_variable_group_primary_key_entry.vg_identifier.IsEqual(WidgetInstanceIdentifier::EQUALITY_CHECK_TYPE__STRING_CODE, the_variable_group.first))
				{
					if (!current_variable_group_primary_key_entry.column_name.empty())
					{
						if (boost::iequals(current_variable_group_primary_key_entry.table_column_name, column_in_variable_group_data_table.column_name_in_original_data_table))
						{

							// If we're here, we're a PRIMARY KEY -
							// not a secondary key

							bool matched = false;

							if (current_variable_group_primary_key_entry.current_outer_multiplicity_of_this_primary_key__in_relation_to__the_uoa_corresponding_to_the_current_variable_group___same_as___current_inner_table_number_within_the_inner_table_set_corresponding_to_the_current_variable_group
								== 1)
							{
								// For the raw data table, there is only one instance of the primary keys associated with multiplicity greater than 1.
								// But the total primary key sequence ("sequence") stores the columns of the OUTPUT, which contains all multiplicities.
								// So use the first occurrence of the primary keys (current_multiplicity == 1) to obtain the information.

								matched = true;
							}


							// DN removed Wednesday, Mar 26, 2014 after careful review.
							// Only affects child variable groups, and is simply wrong.
							// Note that for smaller child variable groups, it will match multiple times,
							// and the last hit will overwrite others and persist.

							//else
							//{
							//	// deal with child tables that have a smaller number of columns in this DMU category
							//	// than the primary variable groups do.
							//	if (current_variable_group_primary_key_entry.total_number_columns_for_dmu_category__internal_to_uoa_corresponding_to_this_variable_group
							//		<
							//		current_variable_group_primary_key_entry.total_number_columns_for_dmu_category__internal_to_the_uoa_corresponding_to_primary_uoa_for_the_same_dmu_category)
							//	{

							//		// matching the above condition means we're a child variable group
							//		// and that the current column in this child variable group
							//		// is a DMU that has a smaller count in the child than in the primary variable group

							//		if (current_variable_group_primary_key_entry.current_outer_multiplicity_of_this_primary_key__in_relation_to__the_uoa_corresponding_to_the_current_variable_group___same_as___current_inner_table_number_within_the_inner_table_set_corresponding_to_the_current_variable_group
							//			<= current_variable_group_primary_key_entry.total_number_columns_for_dmu_category__internal_to_the_uoa_corresponding_to_primary_uoa_for_the_same_dmu_category)
							//		{
							//			// The current column corresponds to the first inner table of the top-level variable groups,
							//			// though it corresponds to the second or greater inner table of a current child variable group.
							//			// i.e., this is the second or following call to this function corresponding to a second or
							//			// higher multiplicity of a child variable group.

							//			matched = true;
							//		}
							//	}
							//}

							if (matched)
							{

								// The various loops have left us in the following state:
								// This block will be hit once, and only once, for every primary key.
								// It will never be hit for secondary keys.
								// "primary_key_entry__output__including_multiplicities"
								// and
								// "current_variable_group_primary_key_entry"
								// will both correspond to the LEFTMOST (i.e., first)
								// occurrence of the primary key in the total K-spin-count selected by the user,
								// even though THIS block is hit (once) for EVERY occurrence of the primary key
								// in the total K-spin-count selected by the user.
								// Due to the "column name" check, above, the current
								// "column_in_variable_group_data_table" variable will properly match
								// the sequence number it's supposed to be within that total K-spin-count.

								column_in_variable_group_data_table.primary_key_dmu_category_identifier = primary_key_entry__output__including_multiplicities.dmu_category;
								column_in_variable_group_data_table.primary_key_index_within_total_kad_for_dmu_category =
									primary_key_entry__output__including_multiplicities.sequence_number_within_dmu_category_spin_control;
								column_in_variable_group_data_table.primary_key_index__within_uoa_corresponding_to_variable_group_corresponding_to_current_inner_table__for_dmu_category =
									current_variable_group_primary_key_entry.sequence_number_within_dmu_category_for_this_variable_groups_uoa;
								column_in_variable_group_data_table.primary_key_index_within_primary_uoa_for_dmu_category =
									primary_key_entry__output__including_multiplicities.sequence_number_within_dmu_category_primary_uoa;
								column_in_variable_group_data_table.current_multiplicity__corresponding_to__current_inner_table___is_1_in_all_inner_tables_when_multiplicity_is_1_for_that_dmu_category_for_that_vg
									= current_variable_group_primary_key_entry.current_outer_multiplicity_of_this_primary_key__in_relation_to__the_uoa_corresponding_to_the_current_variable_group___same_as___current_inner_table_number_within_the_inner_table_set_corresponding_to_the_current_variable_group;
								column_in_variable_group_data_table.total_outer_multiplicity__in_total_kad__for_current_dmu_category__for_current_variable_group =
									current_variable_group_primary_key_entry.total_outer_multiplicity__in_total_kad__for_current_dmu_category__for_current_variable_group;
								column_in_variable_group_data_table.total_k_count__within_uoa_corresponding_to_top_level_variable_group__for_current_dmu_category =
									k_count__corresponding_to_top_level_uoa__and_current_dmu_category;

								std::for_each(dmu_counts_corresponding_to_uoa_for_current_primary_or_child_variable_group.cbegin(),
											  dmu_counts_corresponding_to_uoa_for_current_primary_or_child_variable_group.cend(), [this, &number_inner_tables, &column_in_variable_group_data_table, &primary_key_entry__output__including_multiplicities](
												  Table_UOA_Identifier::DMU_Plus_Count const & dmu_count)
								{
									if (dmu_count.first.IsEqual(WidgetInstanceIdentifier::EQUALITY_CHECK_TYPE__STRING_CODE, primary_key_entry__output__including_multiplicities.dmu_category))
									{
										column_in_variable_group_data_table.total_k_count__within_uoa_corresponding_to_current_variable_group__for_current_dmu_category = dmu_count.second;
										WidgetInstanceIdentifier_Int_Pair Kad_Data = model->t_kad_count.getIdentifier(*primary_key_entry__output__including_multiplicities.dmu_category.uuid);
										column_in_variable_group_data_table.total_k_spin_count_across_multiplicities_for_dmu_category = Kad_Data.second;

										int test_total_number_inner_tables = Kad_Data.second / dmu_count.second;

										// We hit this block for DIFFERENT DMU's.
										// Only one of them has multiplicity > 1.
										// That one wins.
										if (test_total_number_inner_tables > number_inner_tables)
										{
											number_inner_tables = test_total_number_inner_tables;
										}
									}
								});

								// Now determine if this primary key field should be treated as numeric for sorting and ordering
								std::for_each(variables_in_group_primary_keys_metadata.cbegin(),
											  variables_in_group_primary_keys_metadata.cend(), [&column_in_variable_group_data_table, &primary_key_entry__output__including_multiplicities, &current_variable_group_primary_key_entry](
												  WidgetInstanceIdentifier const & primary_key_in_variable_group_metadata)
								{
									if (boost::iequals(current_variable_group_primary_key_entry.table_column_name, *primary_key_in_variable_group_metadata.longhand))
									{
										if (primary_key_in_variable_group_metadata.flags == "n")
										{
											column_in_variable_group_data_table.primary_key_should_be_treated_as_integer_____float_not_allowed_as_primary_key = true;
										}
									}
								});
							}

						}
					}
				}
			});
		});

		column_in_variable_group_data_table.number_inner_tables_in_set = number_inner_tables;

	});

}

void OutputModel::OutputGenerator::PopulateDMUCounts()
{

	bool first = true;
	std::for_each(UOAs.cbegin(), UOAs.cend(), [this, &first](std::pair<WidgetInstanceIdentifier, Table_UOA_Identifier::DMU_Counts> const & uoa__to__dmu_counts__pair)
	{

		if (failed || CheckCancelled())
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
		std::for_each(current_dmu_counts.cbegin(), current_dmu_counts.cend(), [this, &current_is_bigger, &current_is_smaller, &current_is_same, &current](
						  Table_UOA_Identifier::DMU_Plus_Count const & current_dmu_plus_count)
		{
			bool matched_current_dmu = false;
			// Looking at the first entry in biggest_counts is the same as looking at any other entry
			// in terms of the DMU counts
			std::for_each(biggest_counts[0].second.cbegin(),
						  biggest_counts[0].second.cend(), [&matched_current_dmu, &current_dmu_plus_count, &current_is_bigger, &current_is_smaller, &current_is_same, &current](
							  Table_UOA_Identifier::DMU_Plus_Count const & k_count_for_primary_uoa_for_given_dmu_category__info)
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
		std::for_each(biggest_counts[0].second.cbegin(), biggest_counts[0].second.cend(), [&current_dmu_counts, &current_is_bigger, &current_is_smaller, &current_is_same](
						  Table_UOA_Identifier::DMU_Plus_Count const & k_count_for_primary_uoa_for_given_dmu_category__info)
		{
			bool matched_biggest_dmu = false;
			std::for_each(current_dmu_counts.cbegin(),
						  current_dmu_counts.cend(), [&matched_biggest_dmu, &k_count_for_primary_uoa_for_given_dmu_category__info, &current_is_bigger, &current_is_smaller, &current_is_same](
							  Table_UOA_Identifier::DMU_Plus_Count const & current_dmu_plus_count)
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

	outer_multiplicities_primary_uoa___ie___if_there_are_3_cols_for_a_single_dmu_in_the_primary_uoa__and_K_is_12__then__this_value_is_4_for_that_DMU____note_this_is_greater_than_1_for_only_1_DMU_in_the_primary_UOA.clear();
	highest_multiplicity_primary_uoa = 0;
	highest_multiplicity_primary_uoa_dmu_string_code.clear();

	// Looking at the first entry in biggest_counts is the same as looking at any other entry
	// in terms of the DMU counts
	std::for_each(biggest_counts[0].second.cbegin(), biggest_counts[0].second.cend(), [this](Table_UOA_Identifier::DMU_Plus_Count const &
				  k_count_for_primary_uoa_for_given_dmu_category__info)
	{
		if (failed || CheckCancelled())
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
				if (biggest_counts[0].first.longhand)
				{
					boost::format
					msg("The choice of K in the spin control for DMU %1% (%2%) (%3%) is too small to support the unit/s of analysis for the variables selected, with required minimum K-value %4% for unit of analysis \"%5%\".");
					msg % *the_dmu_category.code % *the_dmu_category.longhand % kad_count_current_dmu_category % uoa_count_current_dmu_category % *biggest_counts[0].first.longhand;
					SetFailureMessage(msg.str());
				}
				else
				{
					boost::format
					msg("The choice of K in the spin control for DMU %1% (%2%) (%3%) is too small to support the unit/s of analysis for the variables selected, with required minimum K-value %4% for unit of analysis %5%.");
					msg % *the_dmu_category.code % *the_dmu_category.longhand % kad_count_current_dmu_category % uoa_count_current_dmu_category % *biggest_counts[0].first.code;
					SetFailureMessage(msg.str());
				}
			}
			else
			{
				if (biggest_counts[0].first.longhand)
				{
					boost::format
					msg("The choice of K in the spin control for DMU %1% (%2%) is too small to support the unit/s of analysis for the variables selected, with required minimum K-value %3% for unit of analysis \"%4%\".");
					msg % *the_dmu_category.code % kad_count_current_dmu_category % uoa_count_current_dmu_category % *biggest_counts[0].first.longhand;
					SetFailureMessage(msg.str());
				}
				else
				{
					boost::format
					msg("The choice of K in the spin control for DMU %1% (%2%) is too small to support the unit/s of analysis for the variables selected, with required minimum K-value %3% for unit of analysis %4%.");
					msg % *the_dmu_category.code % kad_count_current_dmu_category % uoa_count_current_dmu_category % *biggest_counts[0].first.code;
					SetFailureMessage(msg.str());
				}
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
				if (biggest_counts[0].first.longhand)
				{
					boost::format msg("The choice of K in the spin control for DMU %1% (%2%) (%3%) is not an even multiple of the minimum K-value for the unit of analysis %4% (%5%).");
					msg % *the_dmu_category.code % *the_dmu_category.longhand % kad_count_current_dmu_category % *biggest_counts[0].first.longhand % uoa_count_current_dmu_category;
					SetFailureMessage(msg.str());
				}
				else
				{
					boost::format msg("The choice of K in the spin control for DMU %1% (%2%) (%3%) is not an even multiple of the minimum K-value for the unit of analysis %4% (%5%).");
					msg % *the_dmu_category.code % *the_dmu_category.longhand % kad_count_current_dmu_category % *biggest_counts[0].first.code % uoa_count_current_dmu_category;
					SetFailureMessage(msg.str());
				}
			}
			else
			{
				if (biggest_counts[0].first.longhand)
				{
					boost::format msg("The choice of K in the spin control for DMU %1% (%2%) is not an even multiple of the minimum K-value for the unit of analysis %3% (%4%).");
					msg % *the_dmu_category.code % kad_count_current_dmu_category % *biggest_counts[0].first.longhand % uoa_count_current_dmu_category;
					SetFailureMessage(msg.str());
				}
				else
				{
					boost::format msg("The choice of K in the spin control for DMU %1% (%2%) is not an even multiple of the minimum K-value for the unit of analysis %3% (%4%).");
					msg % *the_dmu_category.code % kad_count_current_dmu_category % *biggest_counts[0].first.code % uoa_count_current_dmu_category;
					SetFailureMessage(msg.str());
				}
			}

			failed = true;
			return;
		}

		outer_multiplicities_primary_uoa___ie___if_there_are_3_cols_for_a_single_dmu_in_the_primary_uoa__and_K_is_12__then__this_value_is_4_for_that_DMU____note_this_is_greater_than_1_for_only_1_DMU_in_the_primary_UOA.push_back(multiplicity);

		if (multiplicity > highest_multiplicity_primary_uoa)
		{
			highest_multiplicity_primary_uoa = multiplicity;
			highest_multiplicity_primary_uoa_dmu_string_code = *the_dmu_category.code;
		}
	});

	if (failed || CheckCancelled())
	{
		// Todo: Error message
		return;
	}

	// check that only 1 primary UOA's DMU category has multiplicity > 1 (for now)
	any_primary_dmu_has_multiplicity_greater_than_1 = false;
	which_primary_index_has_multiplicity_greater_than_1 = -1;

	int current_index = 0;
	std::for_each(outer_multiplicities_primary_uoa___ie___if_there_are_3_cols_for_a_single_dmu_in_the_primary_uoa__and_K_is_12__then__this_value_is_4_for_that_DMU____note_this_is_greater_than_1_for_only_1_DMU_in_the_primary_UOA.cbegin(), outer_multiplicities_primary_uoa___ie___if_there_are_3_cols_for_a_single_dmu_in_the_primary_uoa__and_K_is_12__then__this_value_is_4_for_that_DMU____note_this_is_greater_than_1_for_only_1_DMU_in_the_primary_UOA.cend(), [this, &current_index](int const & test_multiplicity)
	{
		if (failed || CheckCancelled())
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

	if (failed || CheckCancelled())
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
		if (failed || CheckCancelled())
		{
			return; // from lambda
		}

		int primary_dmu_categories_for_which_child_has_less = 0;
		Table_UOA_Identifier::DMU_Counts const & current_dmu_counts = uoa__to__dmu_counts__pair.second;
		std::for_each(current_dmu_counts.cbegin(), current_dmu_counts.cend(), [this, &primary_dmu_categories_for_which_child_has_less](
						  Table_UOA_Identifier::DMU_Plus_Count const & current_dmu_plus_count)
		{
			if (failed || CheckCancelled())
			{
				return; // from lambda
			}

			if (current_dmu_plus_count.second == 0)
			{
				// just fine
				return; // from lambda
			}

			std::for_each(biggest_counts[0].second.cbegin(), biggest_counts[0].second.cend(), [this, &current_dmu_plus_count, &primary_dmu_categories_for_which_child_has_less](
							  Table_UOA_Identifier::DMU_Plus_Count const & k_count_for_primary_uoa_for_given_dmu_category__info)
			{
				if (failed || CheckCancelled())
				{
					return; // from lambda
				}

				if (current_dmu_plus_count.first.IsEqual(WidgetInstanceIdentifier::EQUALITY_CHECK_TYPE__STRING_CODE, k_count_for_primary_uoa_for_given_dmu_category__info.first))
				{
					if (current_dmu_plus_count.second ==
						k_count_for_primary_uoa_for_given_dmu_category__info.second) // biggest_dmu_plus_count.second is the K-value of the unit of analysis, not the K-value chosen by the user in the spin control
					{
						if (boost::iequals(*current_dmu_plus_count.first.code, highest_multiplicity_primary_uoa_dmu_string_code))
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
						}

						return; // from lambda
					}

					if (current_dmu_plus_count.second > 1)
					{
						// Todo: error message
						// Invalid child UOA for this output
						if (current_dmu_plus_count.first.longhand)
						{
							boost::format
							msg("For child variable groups, the K-value within the unit of analysis for DMU category %1% (%2%) may only be greater than 1 if it matches the K-value within the UOA of the top-level variable group/s.");
							msg % *current_dmu_plus_count.first.code % *current_dmu_plus_count.first.longhand;
							SetFailureMessage(msg.str());
						}
						else
						{
							boost::format
							msg("For child variable groups, the K-value within the unit of analysis for DMU category %1% may only be greater than 1 if it matches the K-value within the UOA of the top-level variable group/s.");
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
								boost::format
								msg("For child variable groups, the K-value within the unit of analysis for DMU category %1% (%2%) cannot currently be 1 if the K-value within the UOA of the top-level variable group/s is greater than 1 when that DMU category does not have multiplicity greater than 1 for the top-level variable group.");
								msg % *current_dmu_plus_count.first.code % *current_dmu_plus_count.first.longhand;
								SetFailureMessage(msg.str());
							}
							else
							{
								boost::format
								msg("For child variable groups, the K-value within the unit of analysis for DMU category %1% cannot currently be 1 if the K-value within the UOA of the top-level variable group/s is greater than 1 when that DMU category does not have multiplicity greater than 1 for the top-level variable group.");
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

			boost::format
			msg("For child variable group with unit of analysis %1%, the K-value for more than one DMU category is less than the corresponding K-values within the UOA of the top-level variable group/s.  This is currently not supported by NewGene.");
			msg % *uoa__to__dmu_counts__pair.first.code;
			SetFailureMessage(msg.str());
			failed = true;
			return; // from lambda
		}

	});

}

void OutputModel::OutputGenerator::PopulateUOAs()
{

	std::for_each(the_map->cbegin(), the_map->cend(), [this](std::pair < /* UOA identifier */
				  WidgetInstanceIdentifier,
				  /* map: VG identifier => List of variables */
				  Table_VARIABLES_SELECTED::VariableGroup_To_VariableSelections_Map >
				  const & uoa__to__variable_groups__pair)
	{
		if (failed || CheckCancelled())
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

		if (failed || CheckCancelled())
		{
			return; // from lambda
		}

		WidgetInstanceIdentifier uoa_identifier = child_uoa__dmu_counts__pair.first;
		bool first = true;
		std::for_each(child_uoa__dmu_counts__pair.second.cbegin(),
					  child_uoa__dmu_counts__pair.second.cend(), [this, &uoa_identifier, &first](Table_UOA_Identifier::DMU_Plus_Count const & dmu_category)
		{

			if (failed || CheckCancelled())
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
					if (uoa_identifier.longhand)
					{
						boost::format
						msg("The choice of K in the spin control for DMU %1% (%2%) (%3%) is too small to support the unit/s of analysis for the variables selected, with required minimum K-value %4% for unit of analysis \"%5%\".");
						msg % *the_dmu_category_identifier.code % *the_dmu_category_identifier.longhand % k_spin_control_count__current_dmu_category % uoa_k_count__current_dmu_category %
						*uoa_identifier.longhand;
						SetFailureMessage(msg.str());
					}
					else
					{
						boost::format
						msg("The choice of K in the spin control for DMU %1% (%2%) (%3%) is too small to support the unit/s of analysis for the variables selected, with required minimum K-value %4% for unit of analysis %5%.");
						msg % *the_dmu_category_identifier.code % *the_dmu_category_identifier.longhand % k_spin_control_count__current_dmu_category % uoa_k_count__current_dmu_category %
						*uoa_identifier.code;
						SetFailureMessage(msg.str());
					}
				}
				else
				{
					if (uoa_identifier.longhand)
					{
						boost::format
						msg("The choice of K in the spin control for DMU %1% (%2%) is too small to support the unit/s of analysis for the variables selected, with required minimum K-value %3% for unit of analysis \"%4%\".");
						msg % *the_dmu_category_identifier.code % k_spin_control_count__current_dmu_category % uoa_k_count__current_dmu_category % *uoa_identifier.longhand;
						SetFailureMessage(msg.str());
					}
					else
					{
						boost::format
						msg("The choice of K in the spin control for DMU %1% (%2%) is too small to support the unit/s of analysis for the variables selected, with required minimum K-value %3% for unit of analysis %4%.");
						msg % *the_dmu_category_identifier.code % k_spin_control_count__current_dmu_category % uoa_k_count__current_dmu_category % *uoa_identifier.code;
						SetFailureMessage(msg.str());
					}
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
	std::for_each(biggest_counts[0].second.cbegin(), biggest_counts[0].second.cend(), [this, &overall_primary_key_sequence_number](
					  Table_UOA_Identifier::DMU_Plus_Count const & k_count_for_primary_uoa_for_given_dmu_category__info)
	{
		if (failed || CheckCancelled())
		{
			return;
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

		for (int k_sequence_number_count_for_given_dmu_category_out_of_total_spin_count_for_that_dmu_category = 0;
			 k_sequence_number_count_for_given_dmu_category_out_of_total_spin_count_for_that_dmu_category < total_spin_control_k_count_for_given_dmu_category;
			 ++k_sequence_number_count_for_given_dmu_category_out_of_total_spin_count_for_that_dmu_category)
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

			std::map<WidgetInstanceIdentifier, int> map__dmu_category__total_outer_multiplicity_of_dmu_category_in_primary_uoa_corresponding_to_top_level_variable_group;

			int view_count = 0;
			std::for_each(primary_variable_groups_vector.cbegin(),
						  primary_variable_groups_vector.cend(), [this, &the_dmu_category, &k_sequence_number_count_for_given_dmu_category_out_of_total_spin_count_for_that_dmu_category, &k_count_for_primary_uoa_for_given_dmu_category, &total_spin_control_k_count_for_given_dmu_category, &current_primary_key_sequence, &variable_group_info_for_primary_keys, &map__dmu_category__total_outer_multiplicity_of_dmu_category_in_primary_uoa_corresponding_to_top_level_variable_group](
							  std::pair<WidgetInstanceIdentifier, WidgetInstanceIdentifiers> const & the_variable_group)
			{
				if (failed || CheckCancelled())
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

				int total_inner_number_columns_in_given_dmu_category_for_uoa_corresponding_to_current_variable_group = 0;
				std::for_each(dmu_category_metadata__for_current_primary_or_child_uoa.cbegin(),
							  dmu_category_metadata__for_current_primary_or_child_uoa.cend(), [this, the_dmu_category, &total_inner_number_columns_in_given_dmu_category_for_uoa_corresponding_to_current_variable_group, &k_sequence_number_count_for_given_dmu_category_out_of_total_spin_count_for_that_dmu_category, &k_count_for_primary_uoa_for_given_dmu_category, &total_spin_control_k_count_for_given_dmu_category, &current_primary_key_sequence, &variable_group_info_for_primary_keys](
								  WidgetInstanceIdentifier const & current_primary_or_child_variable_group__current_dmu_category__primary_key_instance)
				{
					if (current_primary_or_child_variable_group__current_dmu_category__primary_key_instance.IsEqual(WidgetInstanceIdentifier::EQUALITY_CHECK_TYPE__STRING_CODE, the_dmu_category))
					{
						++total_inner_number_columns_in_given_dmu_category_for_uoa_corresponding_to_current_variable_group;
					}
				});

				int outer_multiplicity__within_current_dmu_category__compared_to_total_spin_count__for_that_dmu_category__for_the_current_primary_or_child_uoa = 0;

				if (total_inner_number_columns_in_given_dmu_category_for_uoa_corresponding_to_current_variable_group > 0)
				{
					outer_multiplicity__within_current_dmu_category__compared_to_total_spin_count__for_that_dmu_category__for_the_current_primary_or_child_uoa = 1;
					int test_kad_count = total_inner_number_columns_in_given_dmu_category_for_uoa_corresponding_to_current_variable_group;

					while (test_kad_count <= total_spin_control_k_count_for_given_dmu_category)
					{
						test_kad_count += total_inner_number_columns_in_given_dmu_category_for_uoa_corresponding_to_current_variable_group;

						if (test_kad_count <= total_spin_control_k_count_for_given_dmu_category)
						{
							++outer_multiplicity__within_current_dmu_category__compared_to_total_spin_count__for_that_dmu_category__for_the_current_primary_or_child_uoa;
						}
					}
				}

				// The current iteration is through primary variable groups
				current_primary_key_sequence.total_outer_multiplicity__for_the_current_dmu_category__corresponding_to_the_uoa_corresponding_to_top_level_variable_group =
					outer_multiplicity__within_current_dmu_category__compared_to_total_spin_count__for_that_dmu_category__for_the_current_primary_or_child_uoa;

				map__dmu_category__total_outer_multiplicity_of_dmu_category_in_primary_uoa_corresponding_to_top_level_variable_group[the_dmu_category] =
					outer_multiplicity__within_current_dmu_category__compared_to_total_spin_count__for_that_dmu_category__for_the_current_primary_or_child_uoa;

				variable_group_info_for_primary_keys.push_back(PrimaryKeySequence::VariableGroup_PrimaryKey_Info());
				PrimaryKeySequence::VariableGroup_PrimaryKey_Info & current_variable_group_current_primary_key_info = variable_group_info_for_primary_keys.back();
				current_variable_group_current_primary_key_info.vg_identifier = the_variable_group.first;
				current_variable_group_current_primary_key_info.is_primary_column_selected = false;
				current_variable_group_current_primary_key_info.associated_uoa_identifier = current_uoa_identifier;

				int outer_sequence_number__current_variable_group__current_primary_key_dmu_category = 0;

				for (int m = 0; m < outer_multiplicity__within_current_dmu_category__compared_to_total_spin_count__for_that_dmu_category__for_the_current_primary_or_child_uoa; ++m)
				{

					// Loop through the exact number of occurrences of the DMU category in the unit of analysis.
					// These correspond to the INNER multiplicity within a SINGLE inner table.
					// Only append info for the single instance that matches the sequence number of interest.
					int inner_sequence_number__current_variable_group__current_primary_key_dmu_category = 0;
					std::for_each(dmu_category_metadata__for_current_primary_or_child_uoa.cbegin(),
								  dmu_category_metadata__for_current_primary_or_child_uoa.cend(), [this, &m, &outer_multiplicity__within_current_dmu_category__compared_to_total_spin_count__for_that_dmu_category__for_the_current_primary_or_child_uoa, &current_variable_group_current_primary_key_info, &the_variable_group, &outer_sequence_number__current_variable_group__current_primary_key_dmu_category, &the_dmu_category, &inner_sequence_number__current_variable_group__current_primary_key_dmu_category, &k_sequence_number_count_for_given_dmu_category_out_of_total_spin_count_for_that_dmu_category, &k_count_for_primary_uoa_for_given_dmu_category, &total_spin_control_k_count_for_given_dmu_category, &current_primary_key_sequence, &variable_group_info_for_primary_keys](
									  WidgetInstanceIdentifier const & current_variable_group__current_dmu_primary_key_instance)
					{
						if (failed || CheckCancelled())
						{
							return; // from lambda
						}

						if (current_variable_group__current_dmu_primary_key_instance.IsEqual(WidgetInstanceIdentifier::EQUALITY_CHECK_TYPE__STRING_CODE, the_dmu_category))
						{
							if (k_sequence_number_count_for_given_dmu_category_out_of_total_spin_count_for_that_dmu_category == outer_sequence_number__current_variable_group__current_primary_key_dmu_category)
							{
								// In the WidgetInstanceIdentifier, the CODE is set to the DMU category code,
								// the LONGHAND is set to the column name corresponding to this DMU in the variable group data table,
								// and the SEQUENCE NUMBER is set to the sequence number of the primary key in this variable group.
								current_variable_group_current_primary_key_info.table_column_name = *current_variable_group__current_dmu_primary_key_instance.longhand;
								current_variable_group_current_primary_key_info.sequence_number_within_dmu_category_for_this_variable_groups_uoa =
									inner_sequence_number__current_variable_group__current_primary_key_dmu_category;
								current_variable_group_current_primary_key_info.current_outer_multiplicity_of_this_primary_key__in_relation_to__the_uoa_corresponding_to_the_current_variable_group___same_as___current_inner_table_number_within_the_inner_table_set_corresponding_to_the_current_variable_group
									= m + 1;
								current_variable_group_current_primary_key_info.total_outer_multiplicity__in_total_kad__for_current_dmu_category__for_current_variable_group =
									outer_multiplicity__within_current_dmu_category__compared_to_total_spin_count__for_that_dmu_category__for_the_current_primary_or_child_uoa;
								current_variable_group_current_primary_key_info.total_outer_multiplicity__for_the_current_dmu_category__corresponding_to_the_uoa_corresponding_to_top_level_variable_group =
									current_primary_key_sequence.total_outer_multiplicity__for_the_current_dmu_category__corresponding_to_the_uoa_corresponding_to_top_level_variable_group;
								current_variable_group_current_primary_key_info.total_number_columns_for_dmu_category__internal_to_the_uoa_corresponding_to_primary_uoa_for_the_same_dmu_category =
									k_count_for_primary_uoa_for_given_dmu_category;

								// We are currently iterating through primary variable groups, so this is easy
								current_variable_group_current_primary_key_info.total_number_columns_for_dmu_category__internal_to_uoa_corresponding_to_this_variable_group =
									k_count_for_primary_uoa_for_given_dmu_category;

								std::string this_variable_group__this_primary_key__unique_name;
								this_variable_group__this_primary_key__unique_name += *current_variable_group__current_dmu_primary_key_instance.longhand;
								current_variable_group_current_primary_key_info.column_name_no_uuid = this_variable_group__this_primary_key__unique_name;

								if (outer_multiplicity__within_current_dmu_category__compared_to_total_spin_count__for_that_dmu_category__for_the_current_primary_or_child_uoa > 1)
								{
									this_variable_group__this_primary_key__unique_name += "_";
									this_variable_group__this_primary_key__unique_name += std::to_string(m + 1);
								}

								this_variable_group__this_primary_key__unique_name += "_";
								this_variable_group__this_primary_key__unique_name += newUUID(true);
								current_variable_group_current_primary_key_info.column_name = this_variable_group__this_primary_key__unique_name;
								WidgetInstanceIdentifier vg_setmember_identifier;
								bool found_variable_group_set_member_identifier = input_model->t_vgp_setmembers.getIdentifierFromStringCodeAndParentUUID(
											*current_variable_group__current_dmu_primary_key_instance.longhand, *the_variable_group.first.uuid, vg_setmember_identifier);

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
								std::for_each(the_variable_group.second.cbegin(), the_variable_group.second.cend(), [&found, &vg_setmember_identifier](WidgetInstanceIdentifier const &
											  selected_variable_identifier)
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

							++inner_sequence_number__current_variable_group__current_primary_key_dmu_category;
							++outer_sequence_number__current_variable_group__current_primary_key_dmu_category;
						}
					});
				}

				if (failed || CheckCancelled())
				{
					return; // from lambda
				}
			});

			if (failed || CheckCancelled())
			{
				return; // from lambda
			}

			view_count = 0;
			std::for_each(secondary_variable_groups_vector.cbegin(),
						  secondary_variable_groups_vector.cend(), [this, &the_dmu_category, &k_sequence_number_count_for_given_dmu_category_out_of_total_spin_count_for_that_dmu_category, &k_count_for_primary_uoa_for_given_dmu_category, &total_spin_control_k_count_for_given_dmu_category, &current_primary_key_sequence, &variable_group_info_for_primary_keys, &map__dmu_category__total_outer_multiplicity_of_dmu_category_in_primary_uoa_corresponding_to_top_level_variable_group](
							  std::pair<WidgetInstanceIdentifier, WidgetInstanceIdentifiers> const & the_variable_group)
			{
				if (failed || CheckCancelled())
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
				// NOT the total K-spin-control count; JUST the value of K for this particular variable group's UOA.
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
				std::for_each(dmu_category_metadata__for_current_primary_or_child_uoa.cbegin(),
							  dmu_category_metadata__for_current_primary_or_child_uoa.cend(), [this, the_dmu_category, &total_number_columns_in_given_dmu_category_for_uoa_corresponding_to_current_variable_group, &k_sequence_number_count_for_given_dmu_category_out_of_total_spin_count_for_that_dmu_category, &k_count_for_primary_uoa_for_given_dmu_category, &total_spin_control_k_count_for_given_dmu_category, &current_primary_key_sequence, &variable_group_info_for_primary_keys](
								  WidgetInstanceIdentifier const & current_primary_or_child_variable_group__current_dmu_category__primary_key_instance)
				{
					if (current_primary_or_child_variable_group__current_dmu_category__primary_key_instance.IsEqual(WidgetInstanceIdentifier::EQUALITY_CHECK_TYPE__STRING_CODE, the_dmu_category))
					{
						++total_number_columns_in_given_dmu_category_for_uoa_corresponding_to_current_variable_group;
					}
				});

				int outer_multiplicity__within_current_dmu_category__compared_to_total_spin_count__for_that_dmu_category__for_the_current_primary_or_child_uoa = 0;

				if (total_number_columns_in_given_dmu_category_for_uoa_corresponding_to_current_variable_group > 0)
				{
					outer_multiplicity__within_current_dmu_category__compared_to_total_spin_count__for_that_dmu_category__for_the_current_primary_or_child_uoa = 1;
					int test_kad_count = total_number_columns_in_given_dmu_category_for_uoa_corresponding_to_current_variable_group;

					while (test_kad_count <= total_spin_control_k_count_for_given_dmu_category)
					{
						test_kad_count += total_number_columns_in_given_dmu_category_for_uoa_corresponding_to_current_variable_group;

						if (test_kad_count <= total_spin_control_k_count_for_given_dmu_category)
						{
							++outer_multiplicity__within_current_dmu_category__compared_to_total_spin_count__for_that_dmu_category__for_the_current_primary_or_child_uoa;
						}
					}
				}

				variable_group_info_for_primary_keys.push_back(PrimaryKeySequence::VariableGroup_PrimaryKey_Info());
				PrimaryKeySequence::VariableGroup_PrimaryKey_Info & current_variable_group_current_primary_key_info = variable_group_info_for_primary_keys.back();
				current_variable_group_current_primary_key_info.vg_identifier = the_variable_group.first;
				current_variable_group_current_primary_key_info.is_primary_column_selected = false;
				current_variable_group_current_primary_key_info.associated_uoa_identifier = current_uoa_identifier;
				current_variable_group_current_primary_key_info.total_outer_multiplicity__for_the_current_dmu_category__corresponding_to_the_uoa_corresponding_to_top_level_variable_group =
					map__dmu_category__total_outer_multiplicity_of_dmu_category_in_primary_uoa_corresponding_to_top_level_variable_group[the_dmu_category];



				// *************************************************************************************************************** //
				// We currently have a specific primary key, of a given DMU category,
				// out of the full set of primary keys, including the full K-ad spin counts
				// (represented by the variable:
				// "current_primary_key_sequence",
				//  ... and the variable we are here populating, called:
				// "current_variable_group_current_primary_key_info"
				// - corresponding to the current child variable group).
				//
				// However, we don't yet know which primary key, within the DMU category,
				// that we are dealing with.
				// It could be in any child inner table, and it could have any sequence number within that inner table.
				//
				// So figure out which one we are dealing with.
				// *************************************************************************************************************** //


				// *************************************************************************************************************** //
				// First, iterate through the outer multiplicity (i.e., iterate through the inner tables)
				// *************************************************************************************************************** //

				int outer_sequence_number__current_variable_group__current_primary_key_dmu_category = 0;

				for (int m = 0; m < outer_multiplicity__within_current_dmu_category__compared_to_total_spin_count__for_that_dmu_category__for_the_current_primary_or_child_uoa; ++m)
				{

					// *************************************************************************************************************** //
					// Then, iterate through the inner multiplicity (i.e., iterate through the columns within a single inner table).
					//
					// The inner multiplicity is simply given by the number of rows in the primary key metadata table
					// for the given variable group, for the given DMU category.
					// *************************************************************************************************************** //

					int inner_sequence_number__current_variable_group__current_primary_key_dmu_category = 0;
					std::for_each(dmu_category_metadata__for_current_primary_or_child_uoa.cbegin(),
								  dmu_category_metadata__for_current_primary_or_child_uoa.cend(), [this, &m, &outer_multiplicity__within_current_dmu_category__compared_to_total_spin_count__for_that_dmu_category__for_the_current_primary_or_child_uoa, &current_variable_group_current_primary_key_info, &the_variable_group, &outer_sequence_number__current_variable_group__current_primary_key_dmu_category, &the_dmu_category, &inner_sequence_number__current_variable_group__current_primary_key_dmu_category, &k_sequence_number_count_for_given_dmu_category_out_of_total_spin_count_for_that_dmu_category, &k_count_for_primary_uoa_for_given_dmu_category, &total_spin_control_k_count_for_given_dmu_category, &current_primary_key_sequence, &variable_group_info_for_primary_keys, &total_number_columns_in_given_dmu_category_for_uoa_corresponding_to_current_variable_group](
									  WidgetInstanceIdentifier const & current_variable_group_current_dmu_primary_key)
					{

						if (failed || CheckCancelled())
						{
							return; // from lambda
						}

						if (current_variable_group_current_dmu_primary_key.IsEqual(WidgetInstanceIdentifier::EQUALITY_CHECK_TYPE__STRING_CODE, the_dmu_category))
						{

							// *************************************************************************************************************** //
							// Now see if the TOTAL sequence number of this primary key within the TOTAL K-ad spin control count
							// matches the incoming "current_primary_key_sequence" primary key
							// *************************************************************************************************************** //

							if (k_sequence_number_count_for_given_dmu_category_out_of_total_spin_count_for_that_dmu_category == outer_sequence_number__current_variable_group__current_primary_key_dmu_category)
							{
								// In the WidgetInstanceIdentifier, the CODE is set to the DMU category code,
								// the LONGHAND is set to the column name corresponding to this DMU in the variable group data table,
								// and the SEQUENCE NUMBER is set to the sequence number of the primary key in this variable group.
								current_variable_group_current_primary_key_info.table_column_name = *current_variable_group_current_dmu_primary_key.longhand;
								current_variable_group_current_primary_key_info.sequence_number_within_dmu_category_for_this_variable_groups_uoa =
									inner_sequence_number__current_variable_group__current_primary_key_dmu_category;
								current_variable_group_current_primary_key_info.current_outer_multiplicity_of_this_primary_key__in_relation_to__the_uoa_corresponding_to_the_current_variable_group___same_as___current_inner_table_number_within_the_inner_table_set_corresponding_to_the_current_variable_group
									= m + 1;
								current_variable_group_current_primary_key_info.total_outer_multiplicity__in_total_kad__for_current_dmu_category__for_current_variable_group =
									outer_multiplicity__within_current_dmu_category__compared_to_total_spin_count__for_that_dmu_category__for_the_current_primary_or_child_uoa;
								current_variable_group_current_primary_key_info.total_outer_multiplicity__for_the_current_dmu_category__corresponding_to_the_uoa_corresponding_to_top_level_variable_group =
									current_primary_key_sequence.total_outer_multiplicity__for_the_current_dmu_category__corresponding_to_the_uoa_corresponding_to_top_level_variable_group;
								current_variable_group_current_primary_key_info.total_number_columns_for_dmu_category__internal_to_the_uoa_corresponding_to_primary_uoa_for_the_same_dmu_category =
									k_count_for_primary_uoa_for_given_dmu_category;
								current_variable_group_current_primary_key_info.total_number_columns_for_dmu_category__internal_to_uoa_corresponding_to_this_variable_group =
									total_number_columns_in_given_dmu_category_for_uoa_corresponding_to_current_variable_group;

								std::string this_variable_group__this_primary_key__unique_name;
								this_variable_group__this_primary_key__unique_name += *current_variable_group_current_dmu_primary_key.longhand;
								current_variable_group_current_primary_key_info.column_name_no_uuid = this_variable_group__this_primary_key__unique_name;

								if (outer_multiplicity__within_current_dmu_category__compared_to_total_spin_count__for_that_dmu_category__for_the_current_primary_or_child_uoa > 1)
								{
									this_variable_group__this_primary_key__unique_name += "_";
									this_variable_group__this_primary_key__unique_name += std::to_string(m + 1);
								}

								this_variable_group__this_primary_key__unique_name += "_";
								this_variable_group__this_primary_key__unique_name += newUUID(true);
								current_variable_group_current_primary_key_info.column_name = this_variable_group__this_primary_key__unique_name;
								WidgetInstanceIdentifier vg_setmember_identifier;
								bool found_variable_group_set_member_identifier = input_model->t_vgp_setmembers.getIdentifierFromStringCodeAndParentUUID(*current_variable_group_current_dmu_primary_key.longhand,
										*the_variable_group.first.uuid, vg_setmember_identifier);

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
								std::for_each(the_variable_group.second.cbegin(), the_variable_group.second.cend(), [&found, &vg_setmember_identifier](WidgetInstanceIdentifier const &
											  selected_variable_identifier)
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

							++inner_sequence_number__current_variable_group__current_primary_key_dmu_category;
							++outer_sequence_number__current_variable_group__current_primary_key_dmu_category;
						}
					});
				}

				if (failed || CheckCancelled())
				{
					return; // from lambda
				}
			});

			++overall_primary_key_sequence_number;
			++k_sequence_number_count_for_given_dmu_category_out_of_k_count_for_primary_uoa_for_that_dmu_category;
		}

	});
}

void OutputModel::OutputGenerator::RemoveDuplicatesFromPrimaryKeyMatches(std::int64_t const & current_rows_stepped, SqlAndColumnSet & result,
		std::deque<SavedRowData> & rows_to_sort, std::string datetime_start_col_name, std::string datetime_end_col_name, std::shared_ptr<bool> & statement_is_prepared,
		sqlite3_stmt *& the_prepared_stmt, std::vector<SQLExecutor> & sql_strings, ColumnsInTempView & result_columns, ColumnsInTempView const & sorted_result_columns,
		std::int64_t & current_rows_added, std::int64_t & current_rows_added_since_execution, std::string & sql_add_xr_row, bool & first_row_added,
		std::vector<std::string> & bound_parameter_strings, std::vector<std::int64_t> & bound_parameter_ints, std::vector<long double> & bound_parameter_floats,
		std::vector<SQLExecutor::WHICH_BINDING> & bound_parameter_which_binding_to_use, std::int64_t const minimum_desired_rows_per_transaction, XR_TABLE_CATEGORY const xr_table_category,
		bool const consider_merging_timerange_adjacent_identical_rows, std::string const datetimestart_colname_text, std::string const datetimeend_colname_text)
{

	// perform sort here of rows in rows_to_sort ONLY on time columns
	std::sort(rows_to_sort.begin(), rows_to_sort.end());

	std::deque<SavedRowData> outgoing_rows_of_data;
	HandleSetOfRowsThatMatchOnPrimaryKeys(result_columns, rows_to_sort, outgoing_rows_of_data, xr_table_category, consider_merging_timerange_adjacent_identical_rows);

	if (failed || CheckCancelled())
	{
		return;
	}

	WriteRowsToFinalTable(outgoing_rows_of_data, datetime_start_col_name, datetime_end_col_name, statement_is_prepared, the_prepared_stmt, sql_strings, db, result_columns.view_name,
						  sorted_result_columns, current_rows_added, current_rows_added_since_execution, sql_add_xr_row, first_row_added, bound_parameter_strings, bound_parameter_ints,
						  bound_parameter_floats, bound_parameter_which_binding_to_use, xr_table_category, false, datetimestart_colname_text, datetimeend_colname_text);

	if (failed || CheckCancelled())
	{
		return;
	}

	outgoing_rows_of_data.clear();

	ExecuteSQL(result);

	if (current_rows_added_since_execution >= minimum_desired_rows_per_transaction)
	{
		boost::format msg("Processed %1% of %2% temporary rows this stage: performing transaction");
		msg % current_rows_stepped % current_number_rows_to_sort;
		messager.SetPerformanceLabel(msg.str());
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
	std::for_each(bindings__primary_keys_with_multiplicity_greater_than_1.cbegin(),
				  bindings__primary_keys_with_multiplicity_greater_than_1.cend(), [this, &index, &rhs, &answered, &is_less_than](std::pair<SQLExecutor::WHICH_BINDING, int> const & lhs_binding)
	{
		if (answered)
		{
			return;
		}

		std::pair<SQLExecutor::WHICH_BINDING, int> rhs_binding = rhs.bindings__primary_keys_with_multiplicity_greater_than_1[index];
		std::int64_t rhs_value_int = 0;
		long double rhs_value_float = 0.0;
		std::string rhs_value_string;

		switch (rhs_binding.first)
		{
			case OutputModel::OutputGenerator::SQLExecutor::INT64:
				{
					rhs_value_int = rhs.ints[rhs_binding.second];
				}
				break;

			case OutputModel::OutputGenerator::SQLExecutor::FLOAT:
				{
					rhs_value_float = rhs.floats[rhs_binding.second];
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

			default:
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

						case OutputModel::OutputGenerator::SQLExecutor::FLOAT:
							{
								if (boost::lexical_cast<long double>(lhs_value_int) < rhs_value_float)
								{
									answered = true;
									is_less_than = true;
								}
								else if (boost::lexical_cast<long double>(lhs_value_int) > rhs_value_float)
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

						default:
							break;
					}
				}
				break;

			case OutputModel::OutputGenerator::SQLExecutor::FLOAT:
				{
					long double lhs_value_float = floats[lhs_binding.second];

					switch (rhs_binding.first)
					{
						case OutputModel::OutputGenerator::SQLExecutor::INT64:
							{
								if (lhs_value_float < boost::lexical_cast<long double>(rhs_value_int))
								{
									answered = true;
									is_less_than = true;
								}
								else if (lhs_value_float > boost::lexical_cast<long double>(rhs_value_int))
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

						case OutputModel::OutputGenerator::SQLExecutor::FLOAT:
							{
								if (lhs_value_float < rhs_value_float)
								{
									answered = true;
									is_less_than = true;
								}
								else if (lhs_value_float > rhs_value_float)
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
								if (lhs_value_float < boost::lexical_cast<long double>(rhs_value_string))
								{
									answered = true;
									is_less_than = true;
								}
								else if (lhs_value_float > boost::lexical_cast<long double>(rhs_value_string))
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

						default:
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

						case OutputModel::OutputGenerator::SQLExecutor::FLOAT:
							{
								if (boost::lexical_cast<long double>(lhs_value_string) < rhs_value_float)
								{
									answered = true;
									is_less_than = true;
								}
								else if (boost::lexical_cast<long double>(lhs_value_string) > rhs_value_float)
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

						default:
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

						case OutputModel::OutputGenerator::SQLExecutor::FLOAT:
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

						default:
							break;
					}
				}
				break;

			default:
				break;
		}

		++index;
	});
	return is_less_than;
}

bool OutputModel::OutputGenerator::SavedRowData::TestLessEqual(SavedRowData const & rhs, OutputModel::OutputGenerator & generator) const
{
	generator.test_strings.clear();
	generator.test_ints.clear();
	generator.test_floats.clear();
	generator.rhs_test_strings.clear();
	generator.rhs_test_ints.clear();
	generator.rhs_test_floats.clear();

	bool i_am_less_than = false;

	int column_index = 0;
	int current_inner_table = 0;
	bool is_any_rhs_inner_table_higher_than_my_biggest = false;
	bool something_in_me_does_not_match_rhs = false;

	for (auto it = indices_of_primary_key_columns_with_multiplicity_greater_than_1.cbegin(); it != indices_of_primary_key_columns_with_multiplicity_greater_than_1.cend(); ++it)
	{

		auto binding_info = *it;
		SQLExecutor::WHICH_BINDING const binding = binding_info.first;

		switch (binding)
		{
			case SQLExecutor::INT64:
				{
					generator.test_ints.push_back(current_parameter_ints[binding_info.second.first]);
				}
				break;

			case SQLExecutor::STRING:
				{
					generator.test_strings.push_back(current_parameter_strings[binding_info.second.first]);
				}
				break;

			case SQLExecutor::FLOAT:
				{
					generator.test_floats.push_back(current_parameter_floats[binding_info.second.first]);
				}
				break;

			case SQLExecutor::NULL_BINDING:
				{
					// If we are still not bigger than the rhs row,
					// then we're not bigger.
					return true;
				}
				break;

			default:
				break;
		}

		++column_index;

		bool test_now = false;
		auto test_it = it;
		bool my_last_inner_table = false;

		if (++test_it == indices_of_primary_key_columns_with_multiplicity_greater_than_1.cend())
		{
			test_now = true;
			my_last_inner_table = true;
		}
		else
		{
			auto binding_info_test = *test_it;
			int current_index_test = binding_info_test.second.second;

			if (inner_table_number[current_index_test] > current_inner_table)
			{
				test_now = true;
			}
		}

		if (test_now)
		{
			// We have the next of our own inner tables... test it

			bool match_any_rhs_inner_tables = false;


			int rhs_column_index = 0;
			int rhs_current_inner_table = 0;

			for (auto rhs_it = rhs.indices_of_primary_key_columns_with_multiplicity_greater_than_1.cbegin();
				 rhs_it != rhs.indices_of_primary_key_columns_with_multiplicity_greater_than_1.cend(); ++rhs_it)
			{
				auto rhs_binding_info = *rhs_it;
				SQLExecutor::WHICH_BINDING const rhs_binding = rhs_binding_info.first;

				switch (rhs_binding)
				{
					case SQLExecutor::INT64:
						{
							generator.rhs_test_ints.push_back(rhs.current_parameter_ints[rhs_binding_info.second.first]);
						}
						break;

					case SQLExecutor::STRING:
						{
							generator.rhs_test_strings.push_back(rhs.current_parameter_strings[rhs_binding_info.second.first]);
						}
						break;

					case SQLExecutor::FLOAT:
						{
							generator.rhs_test_floats.push_back(rhs.current_parameter_floats[rhs_binding_info.second.first]);
						}
						break;

					case SQLExecutor::NULL_BINDING:
						{
							// If we are still not bigger than the rhs row,
							// then we're not bigger.
							return true;
						}
						break;

					default:
						break;
				}

				++rhs_column_index;

				bool rhs_test_now = false;
				auto rhs_test_it = rhs_it;

				if (++rhs_test_it == rhs.indices_of_primary_key_columns_with_multiplicity_greater_than_1.cend())
				{
					rhs_test_now = true;
				}
				else
				{
					auto rhs_binding_info_test = *rhs_test_it;
					int rhs_current_index_test = rhs_binding_info_test.second.second;

					if (rhs.inner_table_number[rhs_current_index_test] > rhs_current_inner_table)
					{
						rhs_test_now = true;
					}
				}

				if (rhs_test_now)
				{
					// We have the next of the rhs's inner tables... test it against our current inner table

					if (!generator.test_ints.empty())
					{
						bool no_match = false;

						if (generator.test_ints.size() == generator.rhs_test_ints.size())
						{
							int the_size = (int)generator.test_ints.size();
							bool ignore_higher_check = false;

							for (int n = 0; n < the_size; ++n)
							{
								if (!ignore_higher_check)
								{
									if (generator.test_ints[n] < generator.rhs_test_ints[n])
									{
										if (my_last_inner_table)
										{
											is_any_rhs_inner_table_higher_than_my_biggest = true;
										}
									}
								}

								if (generator.test_ints[n] != generator.rhs_test_ints[n])
								{
									no_match = true;
									ignore_higher_check = true;
								}
							}
						}

						if (!no_match)
						{
							match_any_rhs_inner_tables = true;
						}
					}
					else if (!generator.test_strings.empty())
					{
						bool no_match = false;

						if (generator.test_strings.size() == generator.rhs_test_strings.size())
						{
							int the_size = (int)generator.test_strings.size();
							bool ignore_higher_check = false;

							for (int n = 0; n < the_size; ++n)
							{
								if (!ignore_higher_check)
								{
									if (generator.test_strings[n] < generator.rhs_test_strings[n])
									{
										if (my_last_inner_table)
										{
											is_any_rhs_inner_table_higher_than_my_biggest = true;
										}
									}
								}

								if (generator.test_strings[n] != generator.rhs_test_strings[n])
								{
									no_match = true;
									ignore_higher_check = true;
								}
							}
						}

						if (!no_match)
						{
							match_any_rhs_inner_tables = true;
						}
					}
					else if (!generator.test_floats.empty())
					{
						bool no_match = false;

						if (generator.test_floats.size() == generator.rhs_test_floats.size())
						{
							int the_size = (int)generator.test_floats.size();
							bool ignore_higher_check = false;

							for (int n = 0; n < the_size; ++n)
							{
								if (!ignore_higher_check)
								{
									if (generator.test_floats[n] < generator.rhs_test_floats[n])
									{
										if (my_last_inner_table)
										{
											is_any_rhs_inner_table_higher_than_my_biggest = true;
										}
									}
								}

								if (generator.test_floats[n] != generator.rhs_test_floats[n])
								{
									no_match = true;
									ignore_higher_check = true;
								}
							}
						}

						if (!no_match)
						{
							match_any_rhs_inner_tables = true;
						}
					}

					generator.rhs_test_strings.clear();
					generator.rhs_test_ints.clear();
					generator.rhs_test_floats.clear();
					++rhs_current_inner_table;
				}
			}

			if (!match_any_rhs_inner_tables)
			{
				something_in_me_does_not_match_rhs = true;
			}

			generator.test_strings.clear();
			generator.test_ints.clear();
			generator.test_floats.clear();
			++current_inner_table;
		}
	}

	// The whole point of this function is the following test.
	// For example, given lhs and rhs as follows:
	// 3 4 5
	// 2 3 4
	// The test should fail because a following row could bring in a 5 on the rhs (column counts are ignored - so this function thinks it's possible even if there aren't enough columns to hold it)
	// Whereas
	// 3 4 5
	// 2 3 6
	// ... should pass because there is no way for the 5 to appear in a following row, since rows are sorted and there's already a 6 present

	if (something_in_me_does_not_match_rhs && is_any_rhs_inner_table_higher_than_my_biggest)
	{
		i_am_less_than = true;
	}

	return i_am_less_than;
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

bool OutputModel::OutputGenerator::TimeRangeSorter::operator<(TimeRangeSorter const & rhs) const
{

	// the whole point of this time range sorter is to skip the final inner table
	// (so that the algorithm that splits XR rows can compare all rows that match
	// on every primary key group BUT the last one in order to determine
	// whether to include a row that has NULL for the final primary key group)...
	// the algorithm should NOT include a row if there is at least ONE
	// other row that gets included, which matches on all but the final primary key group.

	// This behavior can be overridden by setting the "DoCompareFinalInnerTable" flag.

	// Also, this function can either ignore the time range, or include it,
	// depending on the "ShouldReturnEqual_EvenIf_TimeRangesAreDifferent" flag.

	// If we're here, we already know that, aside from the sequence and NULLs,
	// we match on the primary key groups from all but the last inner table

	// (1) Test equality of the primary keys from all inner tables but the last
	//     (assume that if the COUNT of inner tables, not counting the last,
	//      that have non-NULL primary key fields, is sufficient to detect a match:
	//      the code assumes that a previous test using "TestPrimaryKeyMatch()")
	//      has been performed).
	//      Unlike TestPrimaryKeyMatch(), this function returns FALSE (no match)
	//      if the number of non-NULL primary keys present all but the last inner table
	//      is different (even if those that are present completely overlap).
	// (2) Test equality of the final inner table's primary keys
	// (3) Test equality of the time range

	// The number of non-NULL primary key groups dominates the decision on sort order.

	// First sort by multiplicity-1 primary keys.
	int the_index = 0;
	bool is_determined = false;
	bool is_less_than = false;
	std::for_each(the_data_row_to_be_sorted__with_guaranteed_primary_key_match_on_all_but_last_inner_table.indices_of_primary_key_columns_with_multiplicity_equal_to_1.cbegin(),
				  the_data_row_to_be_sorted__with_guaranteed_primary_key_match_on_all_but_last_inner_table.indices_of_primary_key_columns_with_multiplicity_equal_to_1.cend(), [this, &is_determined, &is_less_than, &rhs, &the_index](
					  std::pair<SQLExecutor::WHICH_BINDING, std::pair<int, int>> const & current_info)
	{

		if (is_determined)
		{
			return;
		}

		SQLExecutor::WHICH_BINDING binding = current_info.first;
		SQLExecutor::WHICH_BINDING rhs_binding =
			rhs.the_data_row_to_be_sorted__with_guaranteed_primary_key_match_on_all_but_last_inner_table.indices_of_all_columns[current_info.second.second].first;

		if (binding == SQLExecutor::NULL_BINDING && rhs_binding == SQLExecutor::NULL_BINDING)
		{
			// undetermined
			return;
		}

		if (binding == SQLExecutor::NULL_BINDING && rhs_binding != SQLExecutor::NULL_BINDING)
		{
			// NULL is less
			is_less_than = true;
			is_determined = true;
			return;
		}

		if (binding != SQLExecutor::NULL_BINDING && rhs_binding == SQLExecutor::NULL_BINDING)
		{
			// NULL is less
			is_less_than = false;
			is_determined = true;
			return;
		}

		switch (binding)
		{

			case SQLExecutor::INT64:
				{

					std::int64_t data_int64 = the_data_row_to_be_sorted__with_guaranteed_primary_key_match_on_all_but_last_inner_table.current_parameter_ints[current_info.second.first];

					switch (rhs_binding)
					{

						case SQLExecutor::INT64:
							{

								std::int64_t data_int64_rhs =
									rhs.the_data_row_to_be_sorted__with_guaranteed_primary_key_match_on_all_but_last_inner_table.current_parameter_ints[rhs.the_data_row_to_be_sorted__with_guaranteed_primary_key_match_on_all_but_last_inner_table.indices_of_all_columns[current_info.second.second].second.first];

								if (data_int64 < data_int64_rhs)
								{
									is_less_than = true;
									is_determined = true;;
								}
								else if (data_int64 > data_int64_rhs)
								{
									is_less_than = false;
									is_determined = true;
								}

							}
							break;

						case SQLExecutor::FLOAT:
							{

								long double data_float_rhs =
									rhs.the_data_row_to_be_sorted__with_guaranteed_primary_key_match_on_all_but_last_inner_table.current_parameter_floats[rhs.the_data_row_to_be_sorted__with_guaranteed_primary_key_match_on_all_but_last_inner_table.indices_of_all_columns[current_info.second.second].second.first];
								long double data_converted = boost::lexical_cast<long double>(data_int64);

								if (data_converted < data_float_rhs)
								{
									is_less_than = true;
									is_determined = true;;
								}
								else if (data_converted > data_float_rhs)
								{
									is_less_than = false;
									is_determined = true;
								}

							}
							break;

						case SQLExecutor::STRING:
							{

								std::string data_string_rhs =
									rhs.the_data_row_to_be_sorted__with_guaranteed_primary_key_match_on_all_but_last_inner_table.current_parameter_strings[rhs.the_data_row_to_be_sorted__with_guaranteed_primary_key_match_on_all_but_last_inner_table.indices_of_all_columns[current_info.second.second].second.first];
								std::int64_t data_rhs_converted = boost::lexical_cast<std::int64_t>(data_string_rhs);

								if (data_int64 < data_rhs_converted)
								{
									is_less_than = true;
									is_determined = true;;
								}
								else if (data_int64 > data_rhs_converted)
								{
									is_less_than = false;
									is_determined = true;
								}

							}
							break;

						default:
							break;

					}

				}
				break;

			case SQLExecutor::FLOAT:
				{

					long double data_float = the_data_row_to_be_sorted__with_guaranteed_primary_key_match_on_all_but_last_inner_table.current_parameter_floats[current_info.second.first];

					switch (rhs_binding)
					{

						case SQLExecutor::INT64:
							{

								std::int64_t data_int64_rhs =
									rhs.the_data_row_to_be_sorted__with_guaranteed_primary_key_match_on_all_but_last_inner_table.current_parameter_ints[rhs.the_data_row_to_be_sorted__with_guaranteed_primary_key_match_on_all_but_last_inner_table.indices_of_all_columns[current_info.second.second].second.first];
								long double data_rhs_converted = boost::lexical_cast<long double>(data_int64_rhs);

								if (data_float < data_rhs_converted)
								{
									is_less_than = true;
									is_determined = true;;
								}
								else if (data_float > data_rhs_converted)
								{
									is_less_than = false;
									is_determined = true;
								}

							}
							break;

						case SQLExecutor::FLOAT:
							{

								long double data_float_rhs =
									rhs.the_data_row_to_be_sorted__with_guaranteed_primary_key_match_on_all_but_last_inner_table.current_parameter_floats[rhs.the_data_row_to_be_sorted__with_guaranteed_primary_key_match_on_all_but_last_inner_table.indices_of_all_columns[current_info.second.second].second.first];

								if (data_float < data_float_rhs)
								{
									is_less_than = true;
									is_determined = true;;
								}
								else if (data_float > data_float_rhs)
								{
									is_less_than = false;
									is_determined = true;
								}

							}
							break;

						case SQLExecutor::STRING:
							{

								std::string data_string_rhs =
									rhs.the_data_row_to_be_sorted__with_guaranteed_primary_key_match_on_all_but_last_inner_table.current_parameter_strings[rhs.the_data_row_to_be_sorted__with_guaranteed_primary_key_match_on_all_but_last_inner_table.indices_of_all_columns[current_info.second.second].second.first];
								long double data_rhs_converted = boost::lexical_cast<long double>(data_string_rhs);

								if (data_float < data_rhs_converted)
								{
									is_less_than = true;
									is_determined = true;;
								}
								else if (data_float > data_rhs_converted)
								{
									is_less_than = false;
									is_determined = true;
								}

							}
							break;

						default:
							break;

					}

				}
				break;

			case SQLExecutor::STRING:
				{

					std::string data_string = the_data_row_to_be_sorted__with_guaranteed_primary_key_match_on_all_but_last_inner_table.current_parameter_strings[current_info.second.first];

					switch (rhs_binding)
					{

						case SQLExecutor::INT64:
							{

								std::int64_t data_int64_rhs =
									rhs.the_data_row_to_be_sorted__with_guaranteed_primary_key_match_on_all_but_last_inner_table.current_parameter_ints[rhs.the_data_row_to_be_sorted__with_guaranteed_primary_key_match_on_all_but_last_inner_table.indices_of_all_columns[current_info.second.second].second.first];
								std::int64_t data_converted = boost::lexical_cast<std::int64_t>(data_string);

								if (data_converted < data_int64_rhs)
								{
									is_less_than = true;
									is_determined = true;;
								}
								else if (data_converted > data_int64_rhs)
								{
									is_less_than = false;
									is_determined = true;
								}

							}
							break;

						case SQLExecutor::FLOAT:
							{

								long double data_float_rhs =
									rhs.the_data_row_to_be_sorted__with_guaranteed_primary_key_match_on_all_but_last_inner_table.current_parameter_floats[rhs.the_data_row_to_be_sorted__with_guaranteed_primary_key_match_on_all_but_last_inner_table.indices_of_all_columns[current_info.second.second].second.first];
								long double data_converted = boost::lexical_cast<long double>(data_string);

								if (data_converted < data_float_rhs)
								{
									is_less_than = true;
									is_determined = true;;
								}
								else if (data_converted > data_float_rhs)
								{
									is_less_than = false;
									is_determined = true;
								}

							}
							break;

						case SQLExecutor::STRING:
							{

								std::string data_string_rhs =
									rhs.the_data_row_to_be_sorted__with_guaranteed_primary_key_match_on_all_but_last_inner_table.current_parameter_strings[rhs.the_data_row_to_be_sorted__with_guaranteed_primary_key_match_on_all_but_last_inner_table.indices_of_all_columns[current_info.second.second].second.first];

								if (data_string < data_string_rhs)
								{
									is_less_than = true;
									is_determined = true;;
								}
								else if (data_string > data_string_rhs)
								{
									is_less_than = false;
									is_determined = true;
								}

							}
							break;

						default:
							break;

					}

				}
				break;

			default:
				break;

		}

		++the_index;
	});

	if (is_determined)
	{
		return is_less_than;
	}

	the_index = 0;
	int current_inner_multiplicity = 0;
	int number_null_primary_key_groups_in_current_row = 0;
	bool current_row_current_inner_table_primary_key_group_is_null = false;
	std::for_each(the_data_row_to_be_sorted__with_guaranteed_primary_key_match_on_all_but_last_inner_table.indices_of_primary_key_columns_with_multiplicity_greater_than_1.cbegin(),
				  the_data_row_to_be_sorted__with_guaranteed_primary_key_match_on_all_but_last_inner_table.indices_of_primary_key_columns_with_multiplicity_greater_than_1.cend(), [this, &current_row_current_inner_table_primary_key_group_is_null, &number_null_primary_key_groups_in_current_row, &the_index, &current_inner_multiplicity](
					  std::pair<SQLExecutor::WHICH_BINDING, std::pair<int, int>> const & current_info)
	{

		if (!DoCompareFinalInnerTable
			&& the_data_row_to_be_sorted__with_guaranteed_primary_key_match_on_all_but_last_inner_table.is_index_a_primary_key_in_the_final_inner_table[current_info.second.second])
		{
			// See note above: The whole point of this time range sorter
			// is to skip the primary key group in the last inner table.
			return;
		}

		SQLExecutor::WHICH_BINDING binding = current_info.first;

		if (binding == SQLExecutor::NULL_BINDING)
		{
			if (!current_row_current_inner_table_primary_key_group_is_null)
			{
				++number_null_primary_key_groups_in_current_row;
				current_row_current_inner_table_primary_key_group_is_null = true;
			}
		}

		++the_index;
		++current_inner_multiplicity;

		if (current_inner_multiplicity ==
			the_data_row_to_be_sorted__with_guaranteed_primary_key_match_on_all_but_last_inner_table.number_of_columns__in_a_single_inner_table__for_the_columns_only_having_the_dmu_category_with_multiplicity_greater_than_one__but_this_info_is_present_for_all_primary_key_columns)
		{
			current_inner_multiplicity = 0;
			current_row_current_inner_table_primary_key_group_is_null = false;
		}

	});


	the_index = 0;
	current_inner_multiplicity = 0;
	int number_null_primary_key_groups_in_current_row_rhs = 0;
	current_row_current_inner_table_primary_key_group_is_null = false;
	std::for_each(rhs.the_data_row_to_be_sorted__with_guaranteed_primary_key_match_on_all_but_last_inner_table.indices_of_primary_key_columns_with_multiplicity_greater_than_1.cbegin(),
				  rhs.the_data_row_to_be_sorted__with_guaranteed_primary_key_match_on_all_but_last_inner_table.indices_of_primary_key_columns_with_multiplicity_greater_than_1.cend(), [this, &rhs, &current_row_current_inner_table_primary_key_group_is_null, &number_null_primary_key_groups_in_current_row_rhs, &the_index, &current_inner_multiplicity](
					  std::pair<SQLExecutor::WHICH_BINDING, std::pair<int, int>> const & current_info)
	{

		if (!DoCompareFinalInnerTable
			&& rhs.the_data_row_to_be_sorted__with_guaranteed_primary_key_match_on_all_but_last_inner_table.is_index_a_primary_key_in_the_final_inner_table[current_info.second.second])
		{
			// See note above: The whole point of this time range sorter
			// is to skip the primary key group in the last inner table.
			return;
		}

		SQLExecutor::WHICH_BINDING binding = current_info.first;

		if (binding == SQLExecutor::NULL_BINDING)
		{
			if (!current_row_current_inner_table_primary_key_group_is_null)
			{
				++number_null_primary_key_groups_in_current_row_rhs;
				current_row_current_inner_table_primary_key_group_is_null = true;
			}
		}

		++the_index;
		++current_inner_multiplicity;

		if (current_inner_multiplicity ==
			rhs.the_data_row_to_be_sorted__with_guaranteed_primary_key_match_on_all_but_last_inner_table.number_of_columns__in_a_single_inner_table__for_the_columns_only_having_the_dmu_category_with_multiplicity_greater_than_one__but_this_info_is_present_for_all_primary_key_columns)
		{
			current_inner_multiplicity = 0;
			current_row_current_inner_table_primary_key_group_is_null = false;
		}

	});


	if (number_null_primary_key_groups_in_current_row > number_null_primary_key_groups_in_current_row_rhs)
	{
		// We have more NULLs than the RHS row, so we should appear first
		return true;
	}

	if (number_null_primary_key_groups_in_current_row < number_null_primary_key_groups_in_current_row_rhs)
	{
		// We have less NULLs than the RHS row, so we should appear after it
		return false;
	}


	// The value of the primary key group in the final inner table then attempts to determine the decision on sort order.

	if (DoCompareFinalInnerTable)
	{

		is_determined = false;
		is_less_than = false;
		int index = 0;
		std::for_each(the_data_row_to_be_sorted__with_guaranteed_primary_key_match_on_all_but_last_inner_table.indices_of_all_primary_key_columns_in_final_inner_table.cbegin(),
					  the_data_row_to_be_sorted__with_guaranteed_primary_key_match_on_all_but_last_inner_table.indices_of_all_primary_key_columns_in_final_inner_table.cend(),
					  [this, &rhs, &is_determined, &is_less_than, &index](std::pair<SQLExecutor::WHICH_BINDING, std::pair<int, int>> const & binding_info)
		{

			if (is_determined)
			{
				return;
			}

			std::pair<SQLExecutor::WHICH_BINDING, std::pair<int, int>> const & binding_info_rhs =
						rhs.the_data_row_to_be_sorted__with_guaranteed_primary_key_match_on_all_but_last_inner_table.indices_of_all_primary_key_columns_in_final_inner_table[index];

			if (binding_info.first == SQLExecutor::NULL_BINDING && binding_info_rhs.first == SQLExecutor::NULL_BINDING)
			{
				// undetermined - move on to time range
				return;
			}

			if (binding_info.first == SQLExecutor::NULL_BINDING && !binding_info_rhs.first == SQLExecutor::NULL_BINDING)
			{
				// determined - one primary key group in the final inner table is NULL, and the other not.
				// Unlike for the primary key groups in all but the final inner table, where NULLs "match",
				// for the FINAL inner table, NULLs do NOT match.
				is_less_than = true;
				is_determined = true;
				return;
			}

			if (!binding_info.first == SQLExecutor::NULL_BINDING && binding_info_rhs.first == SQLExecutor::NULL_BINDING)
			{
				// determined - one primary key group in the final inner table is NULL, and the other not.
				// Unlike for the primary key groups in all but the final inner table, where NULLs "match",
				// for the FINAL inner table, NULLs do NOT match.
				is_less_than = false;
				is_determined = true;
				return;
			}

			// if we're here, both incoming rows have non-NULL primary key fields in the final inner table

			switch (binding_info.first)
			{

				case SQLExecutor::INT64:
					{

						std::int64_t data_int64 = the_data_row_to_be_sorted__with_guaranteed_primary_key_match_on_all_but_last_inner_table.current_parameter_ints[binding_info.second.first];

						switch (binding_info_rhs.first)
						{

							case SQLExecutor::INT64:
								{

									std::int64_t data_int64_rhs = rhs.the_data_row_to_be_sorted__with_guaranteed_primary_key_match_on_all_but_last_inner_table.current_parameter_ints[binding_info_rhs.second.first];

									if (data_int64 < data_int64_rhs)
									{
										is_less_than = true;
										is_determined = true;;
									}
									else if (data_int64 > data_int64_rhs)
									{
										is_less_than = false;
										is_determined = true;
									}

								}
								break;

							case SQLExecutor::FLOAT:
								{

									long double data_float_rhs = rhs.the_data_row_to_be_sorted__with_guaranteed_primary_key_match_on_all_but_last_inner_table.current_parameter_floats[binding_info_rhs.second.first];
									long double data_converted = boost::lexical_cast<long double>(data_int64);

									if (data_converted < data_float_rhs)
									{
										is_less_than = true;
										is_determined = true;;
									}
									else if (data_converted > data_float_rhs)
									{
										is_less_than = false;
										is_determined = true;
									}

								}
								break;

							case SQLExecutor::STRING:
								{

									std::string data_string_rhs = rhs.the_data_row_to_be_sorted__with_guaranteed_primary_key_match_on_all_but_last_inner_table.current_parameter_strings[binding_info_rhs.second.first];
									std::int64_t data_rhs_converted = boost::lexical_cast<std::int64_t>(data_string_rhs);

									if (data_int64 < data_rhs_converted)
									{
										is_less_than = true;
										is_determined = true;;
									}
									else if (data_int64 > data_rhs_converted)
									{
										is_less_than = false;
										is_determined = true;
									}

								}
								break;

							default:
								break;

						}

					}
					break;

				case SQLExecutor::FLOAT:
					{

						long double data_float = the_data_row_to_be_sorted__with_guaranteed_primary_key_match_on_all_but_last_inner_table.current_parameter_floats[binding_info.second.first];

						switch (binding_info_rhs.first)
						{

							case SQLExecutor::INT64:
								{

									std::int64_t data_int64_rhs = rhs.the_data_row_to_be_sorted__with_guaranteed_primary_key_match_on_all_but_last_inner_table.current_parameter_ints[binding_info_rhs.second.first];
									long double data_rhs_converted = boost::lexical_cast<long double>(data_int64_rhs);

									if (data_float < data_rhs_converted)
									{
										is_less_than = true;
										is_determined = true;;
									}
									else if (data_float > data_rhs_converted)
									{
										is_less_than = false;
										is_determined = true;
									}

								}
								break;

							case SQLExecutor::FLOAT:
								{

									long double data_float_rhs = rhs.the_data_row_to_be_sorted__with_guaranteed_primary_key_match_on_all_but_last_inner_table.current_parameter_floats[binding_info_rhs.second.first];

									if (data_float < data_float_rhs)
									{
										is_less_than = true;
										is_determined = true;;
									}
									else if (data_float > data_float_rhs)
									{
										is_less_than = false;
										is_determined = true;
									}

								}
								break;

							case SQLExecutor::STRING:
								{

									std::string data_string_rhs = rhs.the_data_row_to_be_sorted__with_guaranteed_primary_key_match_on_all_but_last_inner_table.current_parameter_strings[binding_info_rhs.second.first];
									long double data_rhs_converted = boost::lexical_cast<long double>(data_string_rhs);

									if (data_float < data_rhs_converted)
									{
										is_less_than = true;
										is_determined = true;;
									}
									else if (data_float > data_rhs_converted)
									{
										is_less_than = false;
										is_determined = true;
									}

								}
								break;

							default:
								break;

						}

					}
					break;

				case SQLExecutor::STRING:
					{

						std::string data_string = the_data_row_to_be_sorted__with_guaranteed_primary_key_match_on_all_but_last_inner_table.current_parameter_strings[binding_info.second.first];

						switch (binding_info_rhs.first)
						{

							case SQLExecutor::INT64:
								{

									std::int64_t data_int64_rhs = rhs.the_data_row_to_be_sorted__with_guaranteed_primary_key_match_on_all_but_last_inner_table.current_parameter_ints[binding_info_rhs.second.first];
									std::int64_t data_converted = boost::lexical_cast<std::int64_t>(data_string);

									if (data_converted < data_int64_rhs)
									{
										is_less_than = true;
										is_determined = true;;
									}
									else if (data_converted > data_int64_rhs)
									{
										is_less_than = false;
										is_determined = true;
									}

								}
								break;

							case SQLExecutor::FLOAT:
								{

									long double data_float_rhs = rhs.the_data_row_to_be_sorted__with_guaranteed_primary_key_match_on_all_but_last_inner_table.current_parameter_floats[binding_info_rhs.second.first];
									long double data_converted = boost::lexical_cast<long double>(data_string);

									if (data_converted < data_float_rhs)
									{
										is_less_than = true;
										is_determined = true;;
									}
									else if (data_converted > data_float_rhs)
									{
										is_less_than = false;
										is_determined = true;
									}

								}
								break;

							case SQLExecutor::STRING:
								{

									std::string data_string_rhs = rhs.the_data_row_to_be_sorted__with_guaranteed_primary_key_match_on_all_but_last_inner_table.current_parameter_strings[binding_info_rhs.second.first];

									if (data_string < data_string_rhs)
									{
										is_less_than = true;
										is_determined = true;;
									}
									else if (data_string > data_string_rhs)
									{
										is_less_than = false;
										is_determined = true;
									}

								}
								break;

							default:
								break;

						}

					}
					break;

				default:
					break;

			}

			++index;

		});

		if (is_determined)
		{
			return is_less_than;
		}

	}

	if (!ShouldReturnEqual_EvenIf_TimeRangesAreDifferent)
	{

		// If the final inner column primary keys also match, then proceed with evaluating the time range to determine the sort.

		if (the_data_row_to_be_sorted__with_guaranteed_primary_key_match_on_all_but_last_inner_table.datetime_start <
			rhs.the_data_row_to_be_sorted__with_guaranteed_primary_key_match_on_all_but_last_inner_table.datetime_start)
		{
			return true;
		}

		if (the_data_row_to_be_sorted__with_guaranteed_primary_key_match_on_all_but_last_inner_table.datetime_start >
			rhs.the_data_row_to_be_sorted__with_guaranteed_primary_key_match_on_all_but_last_inner_table.datetime_start)
		{
			return false;
		}

		if (the_data_row_to_be_sorted__with_guaranteed_primary_key_match_on_all_but_last_inner_table.datetime_end <
			rhs.the_data_row_to_be_sorted__with_guaranteed_primary_key_match_on_all_but_last_inner_table.datetime_end)
		{
			return true;
		}

		if (the_data_row_to_be_sorted__with_guaranteed_primary_key_match_on_all_but_last_inner_table.datetime_end >
			rhs.the_data_row_to_be_sorted__with_guaranteed_primary_key_match_on_all_but_last_inner_table.datetime_end)
		{
			return false;
		}

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
	if (delete_tables)
	{
		if (!table_to_clear.second.make_table_permanent)
		{
			std::string table_name_to_clear = table_to_clear.second.view_name;

			if (tables_deleted.find(table_name_to_clear) == tables_deleted.cend())
			{
				std::string sql_string = "DROP TABLE IF EXISTS ";
				sql_string += table_name_to_clear;

				SQLExecutor table_remover(this, input_model->getDb(), sql_string);
				table_remover.Execute();

				if (!table_remover.failed)
				{
					tables_deleted.insert(table_name_to_clear);
				}
			}
		}
	}
}

std::string OutputModel::OutputGenerator::CheckOutputFileExists()
{

	OutputProjectPathToKadOutputFile * setting_path_to_kad_output = nullptr;

	std::unique_ptr<BackendProjectOutputSetting> path_to_kad_output = project.projectSettings().GetSetting(messager,
			OUTPUT_PROJECT_SETTINGS_BACKEND_NAMESPACE::PATH_TO_KAD_OUTPUT_FILE);
	bool bad = false;

	try
	{
		setting_path_to_kad_output = dynamic_cast<OutputProjectPathToKadOutputFile *>(path_to_kad_output.get());
	}
	catch (std::bad_cast &)
	{
		bad = true;
	}

	if (!bad)
	{
		bool output_file_exists = boost::filesystem::exists(setting_path_to_kad_output->ToString());

		if (output_file_exists)
		{
			if (!overwrite_if_output_file_already_exists)
			{
				boost::format overwrite_msg("The file %1% already exists.  Overwrite this file?");
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

	return "";
}

void OutputModel::OutputGenerator::SetFailureMessage(std::string const & failure_message_)
{
	failure_message = failure_message_;
	std::string report_failure_message = "Failed: ";
	report_failure_message += failure_message;
	messager.AppendKadStatusText(report_failure_message, this);
	messager.UpdateStatusBarText(report_failure_message, nullptr);
}

void OutputModel::OutputGenerator::UpdateProgressBarToNextStage(std::string const helper_text_first_choice, std::string helper_text_second_choice)
{
	++current_progress_stage;

	boost::format msg_1("Generating output to file %1%: Stage %2% of %3%");
	msg_1 % boost::filesystem::path(setting_path_to_kad_output).filename() % current_progress_stage % total_progress_stages;
	messager.UpdateStatusBarText(msg_1.str().c_str(), this);

	if (helper_text_first_choice.size() > 0)
	{
		boost::format msg_2("Stage %1% of %2% - %3%");
		msg_2 % current_progress_stage % total_progress_stages % helper_text_first_choice;
		messager.AppendKadStatusText(msg_2.str().c_str(), this);
	}
	else if (helper_text_second_choice.size())
	{
		boost::format msg_2("Stage %1% of %2% - %3%");
		msg_2 % current_progress_stage % total_progress_stages % helper_text_second_choice;
		messager.AppendKadStatusText(msg_2.str().c_str(), this);
	}
	else
	{
		boost::format msg_2("Stage %1% of %2%");
		msg_2 % current_progress_stage % total_progress_stages;
		messager.AppendKadStatusText(msg_2.str().c_str(), this);
	}

	messager.UpdateProgressBarValue(0);

	messager.SetPerformanceLabel("");

}

void OutputModel::OutputGenerator::UpdateProgressBarValue(Messager & messager, std::int64_t const current_rows_stepped)
{

	boost::format msg("Processed %1% of %2% temporary rows this stage.");
	msg % current_rows_stepped % current_number_rows_to_sort;
	messager.SetPerformanceLabel(msg.str());

	int current_progress_bar_value = (int)(((long double)(current_rows_stepped) / (long double)(current_number_rows_to_sort)) * 1000.0);

	if (current_progress_bar_value > 1000)
	{
		current_progress_bar_value = 1000;
	}

	messager.UpdateProgressBarValue(current_progress_bar_value);

}

// DETAILED_COMMENTING: To see purpose of SortAndOrRemoveDuplicates(), search for DESCRIPTION_OF_SORT_AND_OR_REMOVE_DUPLICATES_ALGORITHM
OutputModel::OutputGenerator::SqlAndColumnSet OutputModel::OutputGenerator::SortAndOrRemoveDuplicates(ColumnsInTempView const & column_set,
		WidgetInstanceIdentifier const & variable_group, std::string & msg_sort_preface, std::string & msg_remove_duplicates_preface, int const current_multiplicity,
		int const primary_group_number, SqlAndColumnSets & sql_and_column_sets, bool const do_clear_table, XR_TABLE_CATEGORY const xr_table_category,
		bool const consider_merging_timerange_adjacent_identical_rows)
{

	std::int64_t number_of_rows_to_sort = ObtainCount(column_set);
	current_number_rows_to_sort = number_of_rows_to_sort;

	if (variable_group.code)
	{
		if (variable_group.longhand)
		{
			if (current_multiplicity >= 0)
			{
				boost::format msg("Multiplicity %2% - %4% for \"%1%\": %3% rows");
				msg % *variable_group.longhand % current_multiplicity % number_of_rows_to_sort % msg_sort_preface;
				UpdateProgressBarToNextStage(msg.str(), std::string());
			}
			else
			{
				boost::format msg("%3% for \"%1%\": %2% rows");
				msg % *variable_group.longhand % number_of_rows_to_sort % msg_sort_preface;
				UpdateProgressBarToNextStage(msg.str(), std::string());
			}
		}
		else
		{
			if (current_multiplicity >= 0)
			{
				boost::format msg("Multiplicity %2% - %4% for %1%: %3% rows");
				msg % *variable_group.code % current_multiplicity % number_of_rows_to_sort % msg_sort_preface;
				UpdateProgressBarToNextStage(msg.str(), std::string());
			}
			else
			{
				boost::format msg("%3% for %1%: %2% rows");
				msg % *variable_group.code % number_of_rows_to_sort % msg_sort_preface;
				UpdateProgressBarToNextStage(msg.str(), std::string());
			}
		}
	}
	else
	{
		boost::format msg("%2%: %1% rows");
		msg % number_of_rows_to_sort % msg_sort_preface;
		UpdateProgressBarToNextStage(msg.str(), std::string());
	}

	// Columns do not change!!!!!!!!!!!!!!  Just the order of rows
	messager.SetPerformanceLabel("Sorting a temporary table; please be patient...");
	SqlAndColumnSet intermediate_sorted_top_level_variable_group_result = CreateSortedTable(column_set, primary_group_number, current_multiplicity, xr_table_category);

	if (failed || CheckCancelled())
	{
		return SqlAndColumnSet();
	}

	intermediate_sorted_top_level_variable_group_result.second.most_recent_sql_statement_executed__index = -1;

	ExecuteSQL(intermediate_sorted_top_level_variable_group_result);

	if (do_clear_table)
	{
		ClearTables(sql_and_column_sets);
	}

	messager.SetPerformanceLabel("");
	sql_and_column_sets.push_back(intermediate_sorted_top_level_variable_group_result);

	if (variable_group.code)
	{
		if (variable_group.longhand)
		{
			if (current_multiplicity >= 0)
			{
				boost::format msg("Multiplicity %2% - %4% for \"%1%\": %3% rows");
				msg % *variable_group.longhand % current_multiplicity % number_of_rows_to_sort % msg_remove_duplicates_preface;
				UpdateProgressBarToNextStage(msg.str(), std::string());
			}
			else
			{
				boost::format msg("%3% for \"%1%\": %2% rows");
				msg % *variable_group.longhand % number_of_rows_to_sort % msg_remove_duplicates_preface;
				UpdateProgressBarToNextStage(msg.str(), std::string());
			}
		}
		else
		{
			if (current_multiplicity >= 0)
			{
				boost::format msg("Multiplicity %2% - %4% for %1%: %3% rows");
				msg % *variable_group.code % current_multiplicity % number_of_rows_to_sort % msg_remove_duplicates_preface;
				UpdateProgressBarToNextStage(msg.str(), std::string());
			}
			else
			{
				boost::format msg("%3% for %1%: %2% rows");
				msg % *variable_group.code % number_of_rows_to_sort % msg_remove_duplicates_preface;
				UpdateProgressBarToNextStage(msg.str(), std::string());
			}
		}
	}
	else
	{
		boost::format msg_2("%2%: %1% rows");
		msg_2 % number_of_rows_to_sort % msg_remove_duplicates_preface;
		UpdateProgressBarToNextStage(msg_2.str(), std::string());
	}

	std::int64_t rows_added = 0;
	SqlAndColumnSet duplicates_removed_top_level_variable_group_result = RemoveDuplicates_Or_OrderWithinRows(intermediate_sorted_top_level_variable_group_result.second,
			primary_group_number, rows_added, current_multiplicity, xr_table_category, consider_merging_timerange_adjacent_identical_rows);

	if (failed || CheckCancelled())
	{
		return SqlAndColumnSet();
	}

	ClearTables(sql_and_column_sets);
	sql_and_column_sets.push_back(duplicates_removed_top_level_variable_group_result);

	if (random_sampling_old && xr_table_category == XR_TABLE_CATEGORY::PRIMARY_VARIABLE_GROUP)
	{
		duplicates_removed_top_level_variable_group_result = Randomize(duplicates_removed_top_level_variable_group_result.second, variable_group, current_multiplicity,
				primary_group_number, sql_and_column_sets);

		if (failed || CheckCancelled())
		{
			return SqlAndColumnSet();
		}
	}

	return duplicates_removed_top_level_variable_group_result;

}

OutputModel::OutputGenerator::SqlAndColumnSet OutputModel::OutputGenerator::Randomize(ColumnsInTempView const & columns, WidgetInstanceIdentifier const & variable_group,
		int const current_multiplicity, int const primary_group_number, SqlAndColumnSets & sql_and_column_sets)
{
	std::int64_t number_rows = ObtainCount(columns);
	boost::format msg("Multiplicity %1% - Randomizing to select a maximum of %2% rows from %3% to pass to the next stage...");
	msg % current_multiplicity % random_sampling_number_rows % number_rows;
	UpdateProgressBarToNextStage(msg.str(), std::string());
	messager.SetPerformanceLabel("Randomizing...");

	SqlAndColumnSet result = std::make_pair(std::vector<SQLExecutor>(), ColumnsInTempView());
	std::vector<SQLExecutor> & sql_strings = result.first;
	ColumnsInTempView & result_columns = result.second;

	result_columns = columns;
	result_columns.most_recent_sql_statement_executed__index = -1;
	std::string view_name = "NGTEMP_RAND";
	view_name += std::to_string(primary_group_number);

	if (current_multiplicity >= 0)
	{
		view_name += "_";
		view_name += std::to_string(current_multiplicity);
	}

	result_columns.view_name_no_uuid = view_name;
	view_name += "_";
	view_name += newUUID(true);
	result_columns.view_name = view_name;

	std::string sql_string;

	sql_string += "CREATE TABLE \"";
	sql_string += result_columns.view_name;
	sql_string += "\" AS SELECT * FROM ";
	sql_string += "(SELECT * FROM \"";
	sql_string += columns.view_name;
	sql_string += "\" ORDER BY RANDOM() LIMIT ";
	sql_string += boost::lexical_cast<std::string>(random_sampling_number_rows);
	sql_string += ") ";

	bool first = true;
	SortOrderByMultiplicityOnes(result_columns, XR_TABLE_CATEGORY::PRIMARY_VARIABLE_GROUP, variable_group, sql_string, first);
	SortOrderByMultiplicityGreaterThanOnes(result_columns, XR_TABLE_CATEGORY::PRIMARY_VARIABLE_GROUP, variable_group, sql_string, first);

	// Finally, order by the time range columns
	if (!first)
	{
		sql_string += ", ";
	}
	else
	{
		sql_string += " ORDER BY ";
	}

	first = false;
	sql_string += "`";
	sql_string += result_columns.columns_in_view[columns.columns_in_view.size() - 2].column_name_in_temporary_table; // final merged datetime start column
	sql_string += "`";
	sql_string += ", ";
	sql_string += "`";
	sql_string += result_columns.columns_in_view[columns.columns_in_view.size() - 1].column_name_in_temporary_table; // final merged datetime end column
	sql_string += "`";

	sql_strings.push_back(SQLExecutor(this, db, sql_string));

	if (failed)
	{
		SetFailureMessage(sql_error);
		return result;
	}

	if (CheckCancelled())
	{
		return result;
	}

	result_columns.most_recent_sql_statement_executed__index = -1;

	ExecuteSQL(result);

	if (failed || CheckCancelled())
	{
		return result;
	}

	ClearTables(sql_and_column_sets);
	messager.SetPerformanceLabel("");
	sql_and_column_sets.push_back(result);

	return result;
}

OutputModel::OutputGenerator::SqlAndColumnSet OutputModel::OutputGenerator::CreateKadResultSet(ColumnsInTempView const & column_set)
{

	//std::int64_t rows_added = 0;

	SqlAndColumnSet result = std::make_pair(std::vector<SQLExecutor>(), ColumnsInTempView());
	std::vector<SQLExecutor> & sql_strings = result.first;
	ColumnsInTempView & result_columns = result.second;

	result_columns = column_set;
	result_columns.most_recent_sql_statement_executed__index = -1;
	std::string view_name = "NGTEMP_KKA";
	result_columns.view_name_no_uuid = view_name;
	view_name += "_";
	view_name += newUUID(true);
	result_columns.view_name = view_name;
	result_columns.view_number = 1;
	result_columns.has_no_datetime_columns = false;

	std::string sql_create_empty_table;
	sql_create_empty_table += "CREATE TABLE \"";
	sql_create_empty_table += result_columns.view_name;
	sql_create_empty_table += "\" AS SELECT * FROM \"";
	sql_create_empty_table += column_set.view_name;
	sql_create_empty_table += "\" WHERE 0";
	sql_strings.push_back(SQLExecutor(this, db, sql_create_empty_table));

	if (failed)
	{
		SetFailureMessage(sql_error);
		return result;
	}

	if (CheckCancelled())
	{
		return result;
	}


	// Add the "merged" time range columns

	// The variable group is that of the primary variable group for this final result set,
	// which is obtained from the first column
	WidgetInstanceIdentifier variable_group = column_set.columns_in_view[0].variable_group_associated_with_current_inner_table;
	WidgetInstanceIdentifier uoa = column_set.columns_in_view[0].uoa_associated_with_variable_group_associated_with_current_inner_table;

	std::string datetime_start_col_name;
	std::string datetime_end_col_name;
	std::string datetime_start_col_name_text;
	std::string datetime_end_col_name_text;

	{
		// Final result - add two TEXT columns to store text representation of the datetime
		std::string datetime_start_col_name_no_uuid;
		datetime_start_col_name_no_uuid = "DATETIME_START_TEXT";
		datetime_start_col_name_text = datetime_start_col_name_no_uuid;
		datetime_start_col_name_text += "_";
		datetime_start_col_name_text += newUUID(true);

		std::string alter_string;
		alter_string += "ALTER TABLE \"";
		alter_string += result_columns.view_name;
		alter_string += "\" ADD COLUMN ";
		alter_string += datetime_start_col_name_text;
		alter_string += " TEXT";

		sql_strings.push_back(SQLExecutor(this, db, alter_string));

		if (failed)
		{
			SetFailureMessage(sql_error);
			return result;
		}

		if (CheckCancelled())
		{
			return result;
		}

		result_columns.columns_in_view.push_back(ColumnsInTempView::ColumnInTempView());
		ColumnsInTempView::ColumnInTempView & datetime_start_column = result_columns.columns_in_view.back();
		datetime_start_column.column_name_in_temporary_table = datetime_start_col_name_text;
		datetime_start_column.column_name_in_temporary_table_no_uuid = datetime_start_col_name_no_uuid;
		datetime_start_column.current_multiplicity__of__current_inner_table__within__current_vg_inner_table_set =
			column_set.columns_in_view.back().current_multiplicity__of__current_inner_table__within__current_vg_inner_table_set;
		datetime_start_column.number_inner_tables_in_set = column_set.columns_in_view.back().number_inner_tables_in_set;

		datetime_start_column.column_type = ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__DATETIMESTART_TEXT;
		datetime_start_column.variable_group_associated_with_current_inner_table =
			column_set.columns_in_view[column_set.columns_in_view.size() - 2].variable_group_associated_with_current_inner_table;
		datetime_start_column.uoa_associated_with_variable_group_associated_with_current_inner_table =
			*column_set.columns_in_view[column_set.columns_in_view.size() - 2].variable_group_associated_with_current_inner_table.identifier_parent;
		datetime_start_column.is_within_inner_table_corresponding_to_top_level_uoa = false;
		datetime_start_column.column_name_in_original_data_table = "";

		std::string datetime_end_col_name_no_uuid;
		datetime_end_col_name_no_uuid = "DATETIME_END_TEXT";
		datetime_end_col_name_text = datetime_end_col_name_no_uuid;
		datetime_end_col_name_text += "_";
		datetime_end_col_name_text += newUUID(true);

		alter_string.clear();
		alter_string += "ALTER TABLE \"";
		alter_string += result_columns.view_name;
		alter_string += "\" ADD COLUMN ";
		alter_string += datetime_end_col_name_text;
		alter_string += " TEXT";

		sql_strings.push_back(SQLExecutor(this, db, alter_string));

		if (failed)
		{
			SetFailureMessage(sql_error);
			return result;
		}

		if (CheckCancelled())
		{
			return result;
		}

		result_columns.columns_in_view.push_back(ColumnsInTempView::ColumnInTempView());
		ColumnsInTempView::ColumnInTempView & datetime_end_column = result_columns.columns_in_view.back();
		datetime_end_column.column_name_in_temporary_table = datetime_end_col_name_text;
		datetime_end_column.column_name_in_temporary_table_no_uuid = datetime_end_col_name_no_uuid;
		datetime_end_column.current_multiplicity__of__current_inner_table__within__current_vg_inner_table_set =
			column_set.columns_in_view.back().current_multiplicity__of__current_inner_table__within__current_vg_inner_table_set;
		datetime_end_column.number_inner_tables_in_set = column_set.columns_in_view.back().number_inner_tables_in_set;
		datetime_end_column.column_type = ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__DATETIMEEND_TEXT;
		datetime_end_column.variable_group_associated_with_current_inner_table =
			column_set.columns_in_view[column_set.columns_in_view.size() - 2].variable_group_associated_with_current_inner_table;
		datetime_end_column.uoa_associated_with_variable_group_associated_with_current_inner_table =
			*column_set.columns_in_view[column_set.columns_in_view.size() - 2].variable_group_associated_with_current_inner_table.identifier_parent;
		datetime_end_column.is_within_inner_table_corresponding_to_top_level_uoa = false;
		datetime_end_column.column_name_in_original_data_table = "";
	}

	std::string datetime_start_col_name_no_uuid;
	datetime_start_col_name_no_uuid = "DATETIMESTART_PRE_MERGED_KAD_OUTPUT";
	datetime_start_col_name = datetime_start_col_name_no_uuid;
	datetime_start_col_name += "_";
	datetime_start_col_name += newUUID(true);

	std::string alter_string;
	alter_string += "ALTER TABLE \"";
	alter_string += result_columns.view_name;
	alter_string += "\" ADD COLUMN ";
	alter_string += datetime_start_col_name;
	alter_string += " INTEGER DEFAULT 0";

	sql_strings.push_back(SQLExecutor(this, db, alter_string));

	if (failed)
	{
		SetFailureMessage(sql_error);
		return result;
	}

	if (CheckCancelled())
	{
		return result;
	}

	result_columns.columns_in_view.push_back(ColumnsInTempView::ColumnInTempView());
	ColumnsInTempView::ColumnInTempView & datetime_start_column = result_columns.columns_in_view.back();
	datetime_start_column.column_name_in_temporary_table = datetime_start_col_name;
	datetime_start_column.column_name_in_temporary_table_no_uuid = datetime_start_col_name_no_uuid;
	datetime_start_column.current_multiplicity__of__current_inner_table__within__current_vg_inner_table_set =
		column_set.columns_in_view.back().current_multiplicity__of__current_inner_table__within__current_vg_inner_table_set;
	datetime_start_column.number_inner_tables_in_set = column_set.columns_in_view.back().number_inner_tables_in_set;
	datetime_start_column.column_type = ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__DATETIMESTART_MERGED_KAD_OUTPUT;
	datetime_start_column.variable_group_associated_with_current_inner_table =
		column_set.columns_in_view[column_set.columns_in_view.size() - 2].variable_group_associated_with_current_inner_table;
	datetime_start_column.uoa_associated_with_variable_group_associated_with_current_inner_table =
		*column_set.columns_in_view[column_set.columns_in_view.size() - 2].variable_group_associated_with_current_inner_table.identifier_parent;
	datetime_start_column.is_within_inner_table_corresponding_to_top_level_uoa = false;
	datetime_start_column.column_name_in_original_data_table = "";

	std::string datetime_end_col_name_no_uuid;
	datetime_end_col_name_no_uuid = "DATETIMEEND_PRE_MERGED_KAD_OUTPUT";
	datetime_end_col_name = datetime_end_col_name_no_uuid;
	datetime_end_col_name += "_";
	datetime_end_col_name += newUUID(true);

	alter_string.clear();
	alter_string += "ALTER TABLE \"";
	alter_string += result_columns.view_name;
	alter_string += "\" ADD COLUMN ";
	alter_string += datetime_end_col_name;
	alter_string += " INTEGER DEFAULT 0";

	sql_strings.push_back(SQLExecutor(this, db, alter_string));

	if (failed)
	{
		SetFailureMessage(sql_error);
		return result;
	}

	if (CheckCancelled())
	{
		return result;
	}

	result_columns.columns_in_view.push_back(ColumnsInTempView::ColumnInTempView());
	ColumnsInTempView::ColumnInTempView & datetime_end_column = result_columns.columns_in_view.back();
	datetime_end_column.column_name_in_temporary_table = datetime_end_col_name;
	datetime_end_column.column_name_in_temporary_table_no_uuid = datetime_end_col_name_no_uuid;
	datetime_end_column.current_multiplicity__of__current_inner_table__within__current_vg_inner_table_set =
		column_set.columns_in_view.back().current_multiplicity__of__current_inner_table__within__current_vg_inner_table_set;
	datetime_end_column.number_inner_tables_in_set = column_set.columns_in_view.back().number_inner_tables_in_set;
	datetime_end_column.column_type = ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__DATETIMEEND_MERGED_KAD_OUTPUT;
	datetime_end_column.variable_group_associated_with_current_inner_table =
		column_set.columns_in_view[column_set.columns_in_view.size() - 2].variable_group_associated_with_current_inner_table;
	datetime_end_column.uoa_associated_with_variable_group_associated_with_current_inner_table =
		*column_set.columns_in_view[column_set.columns_in_view.size() - 2].variable_group_associated_with_current_inner_table.identifier_parent;
	datetime_end_column.is_within_inner_table_corresponding_to_top_level_uoa = false;
	result_columns.current_block_datetime_column_types = std::make_pair(ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__DATETIMESTART_MERGED_KAD_OUTPUT,
			ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__DATETIMEEND_MERGED_KAD_OUTPUT);
	result_columns.previous_block_datetime_column_types = result_columns.current_block_datetime_column_types;
	datetime_end_column.column_name_in_original_data_table = "";

	ExecuteSQL(result); // Executes all SQL queries up to the current one

	if (failed || CheckCancelled())
	{
		return result;
	}


	//int const minimum_desired_rows_per_transaction = 1024 * 16;
	std::int64_t current_rows_added = 0;
	//std::int64_t current_rows_stepped = 0;
	std::int64_t current_rows_added_since_execution = 0;
	std::string sql_add_xr_row;
	bool first_row_added = true;
	std::vector<std::string> bound_parameter_strings;
	std::vector<std::int64_t> bound_parameter_ints;
	std::vector<long double> bound_parameter_floats;
	std::vector<SQLExecutor::WHICH_BINDING> bound_parameter_which_binding_to_use;


	sqlite3_stmt *& the_prepared_stmt = SQLExecutor::stmt_insert;
	std::shared_ptr<bool> statement_is_prepared(std::make_shared<bool>(false));
	SQLExecutor::stmt_insert = nullptr;
	sqlite3_stmt *& the_stmt__ = SQLExecutor::stmt_insert;
	BOOST_SCOPE_EXIT(&the_prepared_stmt, &statement_is_prepared, &the_stmt__)
	{
		if (the_prepared_stmt && *statement_is_prepared)
		{
			sqlite3_finalize(the_prepared_stmt);
			++SQLExecutor::number_statement_finalizes;
			the_prepared_stmt = nullptr;
			*statement_is_prepared = false;
		}

		the_stmt__ = nullptr;
	} BOOST_SCOPE_EXIT_END

	{

		BOOST_SCOPE_EXIT(this_)
		{
			this_->CloseObtainData();
		} BOOST_SCOPE_EXIT_END

		ObtainData(column_set);

		if (failed || CheckCancelled())
		{
			return result;
		}

		BeginNewTransaction();

		std::deque<SavedRowData> rows_to_sort;
		SavedRowData sorting_row_of_data;
		std::deque<SavedRowData> ordering_within_rows_data;

		//bool start_fresh = true;

		//int which_previous_row_index_to_test_against = 0;
		//bool use_newest_row_index = false;

		std::deque<OutputModel::OutputGenerator::SavedRowData> one_row;

		while (StepData())
		{

			one_row.clear();
			sorting_row_of_data.PopulateFromCurrentRowInDatabase(column_set, stmt_result, OutputModel::OutputGenerator::KAD_RESULTS);
			one_row.push_back(std::move(sorting_row_of_data));

			failed = sorting_row_of_data.failed;

			if (failed)
			{
				SetFailureMessage(sorting_row_of_data.error_message);
				return result;
			}

			if (CheckCancelled())
			{
				return result;
			}

			WriteRowsToFinalTable(one_row, datetime_start_col_name, datetime_end_col_name, statement_is_prepared, the_prepared_stmt, sql_strings, db, result_columns.view_name, result_columns,
								  current_rows_added, current_rows_added_since_execution, sql_add_xr_row, first_row_added, bound_parameter_strings, bound_parameter_ints, bound_parameter_floats,
								  bound_parameter_which_binding_to_use, OutputModel::OutputGenerator::KAD_RESULTS, false, datetime_start_col_name_text, datetime_end_col_name_text);

			if (failed || CheckCancelled())
			{
				return SqlAndColumnSet();
			}
		}

		if (failed || CheckCancelled())
		{
			return SqlAndColumnSet();
		}

	}

	if (failed || CheckCancelled())
	{
		return SqlAndColumnSet();
	}

	return result;

}

void OutputModel::OutputGenerator::SortOrderByMultiplicityOnes(ColumnsInTempView const & result_columns, XR_TABLE_CATEGORY const xr_table_category,
		WidgetInstanceIdentifier const & first_variable_group, std::string & sql_create_final_primary_group_table, bool & first)
{
	int current_column = 0;
	std::for_each(result_columns.columns_in_view.cbegin(),
				  result_columns.columns_in_view.cend(), [this, &xr_table_category, &first_variable_group, &sql_create_final_primary_group_table, &result_columns, &current_column, &first](
					  ColumnsInTempView::ColumnInTempView const & view_column)
	{
		switch (xr_table_category)
		{
			case OutputModel::OutputGenerator::PRIMARY_VARIABLE_GROUP:
				{
					if (current_column >= inner_table_no_multiplicities__with_all_datetime_columns_included__column_count)
					{
						return;
					}
				}
				break;

			default:
				{
					if (!view_column.variable_group_associated_with_current_inner_table.IsEqual(WidgetInstanceIdentifier::EQUALITY_CHECK_TYPE__STRING_CODE, first_variable_group))
					{
						return;
					}

					if (view_column.current_multiplicity__of__current_inner_table__within__current_vg_inner_table_set > 1)
					{
						return;
					}
				}
				break;
		}

		// Determine how many columns there are corresponding to the DMU category
		int number_primary_key_columns_in_dmu_category_with_multiplicity_of_1 = 0;
		int column_count_nested = 0;
		std::for_each(result_columns.columns_in_view.cbegin(),
					  result_columns.columns_in_view.cend(), [this, &xr_table_category, &first_variable_group, &view_column, &column_count_nested, &number_primary_key_columns_in_dmu_category_with_multiplicity_of_1, &sql_create_final_primary_group_table](
						  ColumnsInTempView::ColumnInTempView const & view_column_nested)
		{
			switch (xr_table_category)
			{
				case OutputModel::OutputGenerator::PRIMARY_VARIABLE_GROUP:
					{
						if (column_count_nested >= inner_table_no_multiplicities__with_all_datetime_columns_included__column_count)
						{
							return;
						}
					}
					break;

				case OutputModel::OutputGenerator::CHILD_VARIABLE_GROUP:
					{
						if (!view_column_nested.variable_group_associated_with_current_inner_table.IsEqual(WidgetInstanceIdentifier::EQUALITY_CHECK_TYPE__STRING_CODE, first_variable_group))
						{
							return;
						}

						if (view_column_nested.current_multiplicity__of__current_inner_table__within__current_vg_inner_table_set > 1)
						{
							return;
						}
					}
					break;

				case OutputModel::OutputGenerator::FINAL_MERGE_OF_PRIMARY_VARIABLE_GROUP:
					{
					}
					break;

				default:
					break;
			}

			if (view_column_nested.column_type == ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__PRIMARY)
			{
				if (view_column_nested.primary_key_dmu_category_identifier.IsEqual(WidgetInstanceIdentifier::EQUALITY_CHECK_TYPE__STRING_CODE, view_column.primary_key_dmu_category_identifier))
				{
					if (view_column_nested.total_outer_multiplicity__in_total_kad__for_current_dmu_category__for_current_variable_group == 1)
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
			if (view_column.total_outer_multiplicity__in_total_kad__for_current_dmu_category__for_current_variable_group == 1)
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

							if (view_column.primary_key_should_be_treated_as_integer_____float_not_allowed_as_primary_key)
							{
								sql_create_final_primary_group_table += "CAST (";
							}

							sql_create_final_primary_group_table += "`";
							sql_create_final_primary_group_table += view_column.column_name_in_temporary_table;
							sql_create_final_primary_group_table += "`";

							if (view_column.primary_key_should_be_treated_as_integer_____float_not_allowed_as_primary_key)
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
}

void OutputModel::OutputGenerator::SortOrderByMultiplicityGreaterThanOnes(ColumnsInTempView const & result_columns, XR_TABLE_CATEGORY const xr_table_category,
		WidgetInstanceIdentifier const & first_variable_group, std::string & sql_create_final_primary_group_table, bool & first)
{
	if (highest_multiplicity_primary_uoa > 1)
	{

		// Determine how many columns there are corresponding to the DMU category with multiplicity greater than 1
		int number_primary_key_columns_in_dmu_category_with_multiplicity_greater_than_1 = 0;
		std::for_each(result_columns.columns_in_view.cbegin(),
					  result_columns.columns_in_view.cend(), [this, &first_variable_group, &first, &number_primary_key_columns_in_dmu_category_with_multiplicity_greater_than_1](
						  ColumnsInTempView::ColumnInTempView const & view_column)
		{
			if (!view_column.variable_group_associated_with_current_inner_table.IsEqual(WidgetInstanceIdentifier::EQUALITY_CHECK_TYPE__STRING_CODE, first_variable_group))
			{
				return;
			}

			if (view_column.column_type == ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__PRIMARY)
			{
				if (view_column.is_within_inner_table_corresponding_to_top_level_uoa)
				{
					if (view_column.total_outer_multiplicity__in_total_kad__for_current_dmu_category__for_current_variable_group == highest_multiplicity_primary_uoa)
					{
						if (view_column.current_multiplicity__corresponding_to__current_inner_table___is_1_in_all_inner_tables_when_multiplicity_is_1_for_that_dmu_category_for_that_vg == 1)
						{
							++number_primary_key_columns_in_dmu_category_with_multiplicity_greater_than_1;
						}
					}
				}
			}
		});

		// Create the ORDER BY clause, taking the proper primary key columns that compose the DMU category with multiplicity greater than 1, in sequence
		for (int outer_dmu_multiplicity = 1; outer_dmu_multiplicity <= highest_multiplicity_primary_uoa; ++outer_dmu_multiplicity)
		{
			for (int inner_dmu_multiplicity = 0; inner_dmu_multiplicity < number_primary_key_columns_in_dmu_category_with_multiplicity_greater_than_1; ++inner_dmu_multiplicity)
			{
				std::for_each(result_columns.columns_in_view.cbegin(),
							  result_columns.columns_in_view.cend(), [this, &first_variable_group, &inner_dmu_multiplicity, &outer_dmu_multiplicity, &sql_create_final_primary_group_table, &first](
								  ColumnsInTempView::ColumnInTempView const & view_column)
				{
					if (!view_column.variable_group_associated_with_current_inner_table.IsEqual(WidgetInstanceIdentifier::EQUALITY_CHECK_TYPE__STRING_CODE, first_variable_group))
					{
						return;
					}

					if (view_column.column_type == ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__PRIMARY)
					{
						if (view_column.total_outer_multiplicity__in_total_kad__for_current_dmu_category__for_current_variable_group == highest_multiplicity_primary_uoa)
						{
							if (view_column.primary_key_index__within_uoa_corresponding_to_variable_group_corresponding_to_current_inner_table__for_dmu_category == inner_dmu_multiplicity)
							{
								if (view_column.current_multiplicity__corresponding_to__current_inner_table___is_1_in_all_inner_tables_when_multiplicity_is_1_for_that_dmu_category_for_that_vg ==
									outer_dmu_multiplicity)
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

										if (view_column.primary_key_should_be_treated_as_integer_____float_not_allowed_as_primary_key)
										{
											sql_create_final_primary_group_table += "CAST (";
										}

										sql_create_final_primary_group_table += "`";
										sql_create_final_primary_group_table += view_column.column_name_in_temporary_table;
										sql_create_final_primary_group_table += "`";

										if (view_column.primary_key_should_be_treated_as_integer_____float_not_allowed_as_primary_key)
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
}

void OutputModel::OutputGenerator::PopulateSplitRowInfo_FromCurrentMergingColumns(std::vector<std::tuple<bool, bool, std::pair<std::int64_t, std::int64_t>>> & rows_to_insert_info,
		int const previous_datetime_start_column_index, int const current_datetime_start_column_index, int const previous_datetime_end_column_index,
		int const current_datetime_end_column_index, SavedRowData const & current_row_of_data, XR_TABLE_CATEGORY const xr_table_category)
{

	SQLExecutor::WHICH_BINDING previous_data_type = current_row_of_data.indices_of_all_columns[previous_datetime_start_column_index].first;

	SQLExecutor::WHICH_BINDING current_data_type = current_row_of_data.indices_of_all_columns[current_datetime_start_column_index].first;

	bool previous_datetime_is_null = (previous_data_type == SQLExecutor::NULL_BINDING);
	bool current_datetime_is_null = (current_data_type == SQLExecutor::NULL_BINDING);

	std::int64_t previous_datetime_start = 0;
	std::int64_t previous_datetime_end = 0;
	std::int64_t current_datetime_start = 0;
	std::int64_t current_datetime_end = 0;

	if (!previous_datetime_is_null)
	{
		previous_datetime_start = current_row_of_data.current_parameter_ints[current_row_of_data.indices_of_all_columns[previous_datetime_start_column_index].second.first];
		previous_datetime_end = current_row_of_data.current_parameter_ints[current_row_of_data.indices_of_all_columns[previous_datetime_end_column_index].second.first];
	}

	if (!current_datetime_is_null)
	{
		current_datetime_start = current_row_of_data.current_parameter_ints[current_row_of_data.indices_of_all_columns[current_datetime_start_column_index].second.first];
		current_datetime_end = current_row_of_data.current_parameter_ints[current_row_of_data.indices_of_all_columns[current_datetime_end_column_index].second.first];
	}

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

	//bool added = false;

	bool include_current_data = false;
	bool include_previous_data = false;
	std::int64_t start_datetime_to_use = 0;
	std::int64_t end_datetime_to_use = 0;

	if (previous_datetime_is_null && current_datetime_is_null)
	{
		// no data
		return;
	}

	else if (previous_datetime_is_null)
	{

		// Add only current data, setting time range to that of the current data
		include_current_data = true;
		include_previous_data = false;
		start_datetime_to_use = current_datetime_start;
		end_datetime_to_use = current_datetime_end;
		rows_to_insert_info.push_back(std::make_tuple(include_current_data, include_previous_data, std::make_pair(start_datetime_to_use, end_datetime_to_use)));

	}

	else if (current_datetime_is_null)
	{

		// Add only previous data, setting time range to that of the previous data
		include_current_data = false;
		include_previous_data = true;
		start_datetime_to_use = previous_datetime_start;
		end_datetime_to_use = previous_datetime_end;
		rows_to_insert_info.push_back(std::make_tuple(include_current_data, include_previous_data, std::make_pair(start_datetime_to_use, end_datetime_to_use)));

	}

	else if (previous_is_0 && current_is_0)
	{

		// Add row as-is, setting new time range columns to 0
		include_current_data = true;
		include_previous_data = true;
		start_datetime_to_use = 0;
		end_datetime_to_use = 0;
		rows_to_insert_info.push_back(std::make_tuple(include_current_data, include_previous_data, std::make_pair(start_datetime_to_use, end_datetime_to_use)));

	}

	else if (previous_is_0 && !current_is_0)
	{

		if (current_datetime_start < timerange_end && current_datetime_end > timerange_start)
		{
			// First, add row as-is, setting new time range columns to current time range values
			include_current_data = true;
			include_previous_data = true;
			start_datetime_to_use = current_datetime_start;
			end_datetime_to_use = current_datetime_end;

			if (start_datetime_to_use < timerange_start)
			{
				start_datetime_to_use = timerange_start;
			}

			if (end_datetime_to_use > timerange_end)
			{
				end_datetime_to_use = timerange_end;
			}

			rows_to_insert_info.push_back(std::make_tuple(include_current_data, include_previous_data, std::make_pair(start_datetime_to_use, end_datetime_to_use)));

			// Next, add a row that fills in from the user's choice of starting timerange to the starting datetime of the current row, with data from the previous row
			if (current_datetime_start > timerange_start && current_datetime_start < timerange_end)
			{
				include_current_data = false;
				include_previous_data = true;
				start_datetime_to_use = timerange_start;
				end_datetime_to_use = current_datetime_start;
				rows_to_insert_info.push_back(std::make_tuple(include_current_data, include_previous_data, std::make_pair(start_datetime_to_use, end_datetime_to_use)));
			}

			// Next, add a row that fills in from the ending datetime of the current row to the user's choice of ending timerange, with data from the previous row
			if (timerange_end > current_datetime_end && current_datetime_end > timerange_start)
			{
				include_current_data = false;
				include_previous_data = true;
				start_datetime_to_use = current_datetime_end;
				end_datetime_to_use = timerange_end;
				rows_to_insert_info.push_back(std::make_tuple(include_current_data, include_previous_data, std::make_pair(start_datetime_to_use, end_datetime_to_use)));
			}

		}
		else
		{
			// special-case - the new column is out of range, yet we still want the old data
			include_current_data = false;
			include_previous_data = true;
			start_datetime_to_use = 0;
			end_datetime_to_use = 0;
			rows_to_insert_info.push_back(std::make_tuple(include_current_data, include_previous_data, std::make_pair(start_datetime_to_use, end_datetime_to_use)));
		}

	}

	else if (!previous_is_0 && current_is_0)
	{

		// Note: Unlike above "else if" block,
		// in this one, it is guaranteed that the previous data
		// will be within the user's choice of time range
		//    (because that was handled in the previous multiplicity),
		// so we can skip all those checks in this block.

		// First, add row as-is, setting new time range columns to previous time range values
		include_current_data = true;
		include_previous_data = true;
		start_datetime_to_use = previous_datetime_start;
		end_datetime_to_use = previous_datetime_end;
		rows_to_insert_info.push_back(std::make_tuple(include_current_data, include_previous_data, std::make_pair(start_datetime_to_use, end_datetime_to_use)));

		// Unlike the above "else if" block, where we always include the previous data
		// over the full timerange selected by the user, whether or not the current data overlaps,
		// in this block we must NOT include the CURRENT data if it does not overlap
		// the previous data if this is a child merge.
		// (But for primary inner tables, and top-level primary merges, we want all the data over the full time range.)
		if (xr_table_category != XR_TABLE_CATEGORY::CHILD_VARIABLE_GROUP)
		{
			// Next, add a row that fills in from the user's choice of starting timerange to the starting datetime of the previous row, with data from the current row
			if (previous_datetime_start > timerange_start && previous_datetime_start < timerange_end)
			{
				include_current_data = true;
				include_previous_data = false;
				start_datetime_to_use = timerange_start;
				end_datetime_to_use = previous_datetime_start;
				rows_to_insert_info.push_back(std::make_tuple(include_current_data, include_previous_data, std::make_pair(start_datetime_to_use, end_datetime_to_use)));
			}

			// Next, add a row that fills in from the ending datetime of the current row to the user's choice of ending timerange, with data from the previous row
			if (timerange_end > previous_datetime_end && previous_datetime_end > timerange_start)
			{
				include_current_data = true;
				include_previous_data = false;
				start_datetime_to_use = previous_datetime_end;
				end_datetime_to_use = timerange_end;
				rows_to_insert_info.push_back(std::make_tuple(include_current_data, include_previous_data, std::make_pair(start_datetime_to_use, end_datetime_to_use)));
			}
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
			return;
		}
		else if (current_datetime_start >= current_datetime_end)
		{
			// invalid current time range values
			return;
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
				include_current_data = current__DO_include_lower_range_data__DO_include_upper_range_data;
				include_previous_data = previous__DO_include_lower_range_data__DO_include_upper_range_data;
				start_datetime_to_use = current_datetime_start;
				end_datetime_to_use = current_datetime_end;
				rows_to_insert_info.push_back(std::make_tuple(include_current_data, include_previous_data, std::make_pair(start_datetime_to_use, end_datetime_to_use)));

			}
			else if (lower_range_end < upper_range_end)
			{

				// The upper range ends higher than the lower range

				// First, add a row that includes all data,
				// setting new time range columns to:
				// lower_range_start - lower_range_end
				include_current_data = current__DO_include_lower_range_data__DO_include_upper_range_data;
				include_previous_data = previous__DO_include_lower_range_data__DO_include_upper_range_data;
				start_datetime_to_use = lower_range_start;
				end_datetime_to_use = lower_range_end;
				rows_to_insert_info.push_back(std::make_tuple(include_current_data, include_previous_data, std::make_pair(start_datetime_to_use, end_datetime_to_use)));


				// Second, add a row that includes only the upper range's data,
				// setting new time range columns to:
				// lower_range_end - upper_range_end
				if (xr_table_category != OutputModel::OutputGenerator::CHILD_VARIABLE_GROUP || !previous_is_lower)
				{

					include_current_data = current__DO_NOT_include_lower_range_data__DO_include_upper_range_data;
					include_previous_data = previous__DO_NOT_include_lower_range_data__DO_include_upper_range_data;
					start_datetime_to_use = lower_range_end;
					end_datetime_to_use = upper_range_end;
					rows_to_insert_info.push_back(std::make_tuple(include_current_data, include_previous_data, std::make_pair(start_datetime_to_use, end_datetime_to_use)));

				}

			}
			else
			{

				// The lower range ends higher than the upper range

				// First, add a row that includes all data,
				// setting new time range columns to:
				// upper_range_start - upper_range_end
				include_current_data = current__DO_include_lower_range_data__DO_include_upper_range_data;
				include_previous_data = previous__DO_include_lower_range_data__DO_include_upper_range_data;
				start_datetime_to_use = upper_range_start;
				end_datetime_to_use = upper_range_end;
				rows_to_insert_info.push_back(std::make_tuple(include_current_data, include_previous_data, std::make_pair(start_datetime_to_use, end_datetime_to_use)));


				// Second, add a row that includes only the lower range's data,
				// setting new time range columns to:
				// upper_range_end - lower_range_end
				if (xr_table_category != OutputModel::OutputGenerator::CHILD_VARIABLE_GROUP || previous_is_lower)
				{

					include_current_data = current__DO_include_lower_range_data__DO_NOT_include_upper_range_data;
					include_previous_data = previous__DO_include_lower_range_data__DO_NOT_include_upper_range_data;
					start_datetime_to_use = upper_range_end;
					end_datetime_to_use = lower_range_end;
					rows_to_insert_info.push_back(std::make_tuple(include_current_data, include_previous_data, std::make_pair(start_datetime_to_use, end_datetime_to_use)));

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

					include_current_data = current__DO_include_lower_range_data__DO_NOT_include_upper_range_data;
					include_previous_data = previous__DO_include_lower_range_data__DO_NOT_include_upper_range_data;
					start_datetime_to_use = lower_range_start;
					end_datetime_to_use = lower_range_end;
					rows_to_insert_info.push_back(std::make_tuple(include_current_data, include_previous_data, std::make_pair(start_datetime_to_use, end_datetime_to_use)));

				}

				// Second, add a row corresponding to the upper range,
				// setting new time range columns to:
				// upper_range_start - upper_range_end
				if (xr_table_category != OutputModel::OutputGenerator::CHILD_VARIABLE_GROUP || !previous_is_lower)
				{

					include_current_data = current__DO_NOT_include_lower_range_data__DO_include_upper_range_data;
					include_previous_data = previous__DO_NOT_include_lower_range_data__DO_include_upper_range_data;
					start_datetime_to_use = upper_range_start;
					end_datetime_to_use = upper_range_end;
					rows_to_insert_info.push_back(std::make_tuple(include_current_data, include_previous_data, std::make_pair(start_datetime_to_use, end_datetime_to_use)));

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

					include_current_data = current__DO_include_lower_range_data__DO_NOT_include_upper_range_data;
					include_previous_data = previous__DO_include_lower_range_data__DO_NOT_include_upper_range_data;
					start_datetime_to_use = lower_range_start;
					end_datetime_to_use = upper_range_start;
					rows_to_insert_info.push_back(std::make_tuple(include_current_data, include_previous_data, std::make_pair(start_datetime_to_use, end_datetime_to_use)));

				}

				if (lower_range_end == upper_range_end)
				{

					// special case: The lower range and the upper range
					// end at the same time value

					// So second, add a row that covers the entire upper range
					// that includes all data,
					// therefore setting new time range columns to:
					// upper_range_start - upper_range_end

					include_current_data = current__DO_include_lower_range_data__DO_include_upper_range_data;
					include_previous_data = previous__DO_include_lower_range_data__DO_include_upper_range_data;
					start_datetime_to_use = upper_range_start;
					end_datetime_to_use = upper_range_end;
					rows_to_insert_info.push_back(std::make_tuple(include_current_data, include_previous_data, std::make_pair(start_datetime_to_use, end_datetime_to_use)));

				}
				else if (lower_range_end < upper_range_end)
				{

					// The upper range ends higher than the lower range

					// So second, add a row that includes all data,
					// setting new time range columns to:
					// upper_range_start - lower_range_end

					include_current_data = current__DO_include_lower_range_data__DO_include_upper_range_data;
					include_previous_data = previous__DO_include_lower_range_data__DO_include_upper_range_data;
					start_datetime_to_use = upper_range_start;
					end_datetime_to_use = lower_range_end;
					rows_to_insert_info.push_back(std::make_tuple(include_current_data, include_previous_data, std::make_pair(start_datetime_to_use, end_datetime_to_use)));

					// And third, add a row that includes only the upper range's data,
					// setting new time range columns to:
					// lower_range_end - upper_range_end

					include_current_data = current__DO_NOT_include_lower_range_data__DO_include_upper_range_data;
					include_previous_data = previous__DO_NOT_include_lower_range_data__DO_include_upper_range_data;
					start_datetime_to_use = lower_range_end;
					end_datetime_to_use = upper_range_end;
					rows_to_insert_info.push_back(std::make_tuple(include_current_data, include_previous_data, std::make_pair(start_datetime_to_use, end_datetime_to_use)));

				}
				else
				{

					// The lower range ends higher than the upper range

					// So second, add a row that covers the entire upper range
					// that includes all data,
					// therefore setting new time range columns to:
					// upper_range_start - upper_range_end

					include_current_data = current__DO_include_lower_range_data__DO_include_upper_range_data;
					include_previous_data = previous__DO_include_lower_range_data__DO_include_upper_range_data;
					start_datetime_to_use = upper_range_start;
					end_datetime_to_use = upper_range_end;
					rows_to_insert_info.push_back(std::make_tuple(include_current_data, include_previous_data, std::make_pair(start_datetime_to_use, end_datetime_to_use)));

					// And third, add a row that includes only the lower range's data,
					// setting new time range columns to:
					// upper_range_end - lower_range_end
					if (xr_table_category != OutputModel::OutputGenerator::CHILD_VARIABLE_GROUP || previous_is_lower)
					{

						include_current_data = current__DO_include_lower_range_data__DO_NOT_include_upper_range_data;
						include_previous_data = previous__DO_include_lower_range_data__DO_NOT_include_upper_range_data;
						start_datetime_to_use = upper_range_end;
						end_datetime_to_use = lower_range_end;
						rows_to_insert_info.push_back(std::make_tuple(include_current_data, include_previous_data, std::make_pair(start_datetime_to_use, end_datetime_to_use)));

					}

				}

			}

		}

	}

}

void OutputModel::OutputGenerator::Process_RowsToCheckForDuplicates_ThatMatchOnAllButFinalInnerTable_ExceptForNullCount_InXRalgorithm(
	std::vector<SavedRowData> & saved_rows_with_null_in_final_inner_table, TimeRangesForIndividualGroup_IntKeys & group_time_ranges__intkeys,
	TimeRangesForIndividualGroup_FloatKeys & group_time_ranges__floatkeys, TimeRangesForIndividualGroup_StringKeys & group_time_ranges__stringkeys,
	ColumnsInTempView const & previous_full_table__each_row_containing_two_sets_of_data_being_cleaned_against_one_another, std::vector<SavedRowData> & outgoing_rows_of_data,
	std::vector<TimeRangeSorter> const & rows_to_check_for_duplicates_in_newly_joined_primary_key_columns, int const previous_datetime_start_column_index,
	int const current_datetime_start_column_index, int const previous_datetime_end_column_index, int const current_datetime_end_column_index, XR_TABLE_CATEGORY const xr_table_category)
{

	// All incoming rows match on all primary keys except those from the final inner table.
	// However, some of these matching rows might have NULLs for some of the "matching" primary keys.

	// Actually, an earlier stage prohibits the latter condition, but this function is nonetheless
	// redundantly designed to support it in case that changes in the future.

	// Also, the timerange of the data associated with the final inner table for each row
	// has not yet been compared against the time range associated with the block of
	// inner tables that does not include the last.


	std::vector<TimeRangeSorter> rows_to_check;

	std::vector<SavedRowData> saved_rows_with_multiple_nulls;

	std::vector<std::tuple<bool, bool, std::pair<std::int64_t, std::int64_t>>> row_inserts_info;
	std::int64_t datetime_range_start = 0;
	std::int64_t datetime_range_end;
	bool include_current_data = false;
	bool include_previous_data = false;

	//std::for_each(rows_to_check_for_duplicates_in_newly_joined_primary_key_columns.cbegin(), rows_to_check_for_duplicates_in_newly_joined_primary_key_columns.cend(), [this, &saved_rows_with_multiple_nulls, &rows_to_check, &previous_datetime_start_column_index, &current_datetime_start_column_index, &previous_datetime_end_column_index, &current_datetime_end_column_index, &row_inserts_info, &datetime_range_start, &datetime_range_end, &include_current_data, &include_previous_data, &xr_table_category](TimeRangeSorter const & row)
	for (std::vector<TimeRangeSorter>::const_iterator it = rows_to_check_for_duplicates_in_newly_joined_primary_key_columns.cbegin();
		 it != rows_to_check_for_duplicates_in_newly_joined_primary_key_columns.cend(); ++it)
	{

		TimeRangeSorter const & row = *it;

		if (failed || CheckCancelled())
		{
			break;
		}

		// Require that ALL inner tables (not including the last) have data.
		// The reason is that if not all of them have data,
		// it implies that an earlier multiplicity was unable to find any
		// data over the given time range to merge in, and if so,
		// we will not be able to, either.
		// And we must remove these rows for the following steps of this function to work.
		// See comments below for the specific steps that require
		// all inner tables except the last to have data.

		bool null_found_in_inner_table_prior_to_the_last_inner_table = false;
		std::for_each(row.GetSavedRowData().indices_of_all_primary_key_columns_in_all_but_final_inner_table.cbegin(),
					  row.GetSavedRowData().indices_of_all_primary_key_columns_in_all_but_final_inner_table.cend(), [&null_found_in_inner_table_prior_to_the_last_inner_table](
						  std::pair<SQLExecutor::WHICH_BINDING, std::pair<int, int>> const & binding_info)
		{
			if (binding_info.first == SQLExecutor::NULL_BINDING)
			{
				null_found_in_inner_table_prior_to_the_last_inner_table = true;
			}
		});

		if (null_found_in_inner_table_prior_to_the_last_inner_table)
		{
			// Nothing to do.
			// A previous iteration of the multiplicity algorithm
			// was unable to find a matching value here -
			// and so we won't be able to, either.
			// Just save the row as-is without further processing.
			saved_rows_with_multiple_nulls.push_back(row.GetSavedRowData());
			continue;
		}

		// Split an individual incoming row into 0, 1, 2, or 3 new rows,
		// based on the overlap of the time range of the previous data (contained in all inner tables except the last - i.e., the columns to the left)
		// with the new data (in the rightmost columns, the final inner table)
		// What comes out is metadata describing the rows that should be created - what their time ranges should be,
		// and whether to include the previous (left) data, and/or the current (right) data.
		//
		// Note that the rows possibly STILL won't match on the data in the final inner table.
		row_inserts_info.clear();
		PopulateSplitRowInfo_FromCurrentMergingColumns(row_inserts_info, previous_datetime_start_column_index, current_datetime_start_column_index, previous_datetime_end_column_index,
				current_datetime_end_column_index, row.GetSavedRowData(), xr_table_category);

		// Throw out redundant rows.
		//
		// The following block will EXCLUDE rows that only include the data from the final inner table,
		// but not the previous data,
		// because this data is guaranteed to be in place from the initial primary table.
		std::for_each(row_inserts_info.cbegin(),
					  row_inserts_info.cend(), [this, &datetime_range_start, &datetime_range_end, &include_current_data, &include_previous_data, &row, &rows_to_check](
						  std::tuple<bool, bool, std::pair<std::int64_t, std::int64_t>> const & row_insert_info)
					  //for(std::vector<std::tuple<bool, bool, std::pair<std::int64_t, std::int64_t>>>::const_iterator it = row_inserts_info.cbegin(); it != row_inserts_info.cend(); ++it)
		{

			//std::tuple<bool, bool, std::pair<std::int64_t, std::int64_t>> const & row_insert_info = *it;

			if (failed || CheckCancelled())
			{
				//break;
				return;
			}

			//bool added = false;
			datetime_range_start = std::get<2>(row_insert_info).first;
			datetime_range_end = std::get<2>(row_insert_info).second;
			include_current_data = std::get<0>(row_insert_info);
			include_previous_data = std::get<1>(row_insert_info);

			if (!include_previous_data)
			{
				// Skip rows that only include data from the final inner table.
				// The reason is that the INITIAL inner table, which is pulled
				// straight from the raw data, is guaranteed to also include
				// this data by itself, so it doesn't need to be re-added here.
				//continue;
				return;
			}

			rows_to_check.push_back(row);
			SavedRowData & current_row = rows_to_check.back().the_data_row_to_be_sorted__with_guaranteed_primary_key_match_on_all_but_last_inner_table;
			current_row.datetime_start = datetime_range_start;
			current_row.datetime_end = datetime_range_end;
			current_row.error_message.clear();

			if (include_current_data)
			{
				// data has already been copied into the row, and no data needs to be removed.
				//continue;
				return;
			}

			// If we're here, we need to remove the data from the final inner table from the row,
			// because its datetime range is out of range.
			current_row.SetFinalInnerTableToNull(false);

		});
		//}

		if (failed || CheckCancelled())
		{
			break;
		}

		//});
	}

	if (failed || CheckCancelled())
	{
		return;
	}

	// The rows are now properly *individually* split by time range.

	// Order the inner sets of each row
	std::vector<std::string> bound_parameter_strings;
	std::vector<std::int64_t> bound_parameter_ints;
	std::vector<long double> bound_parameter_floats;
	std::vector<SQLExecutor::WHICH_BINDING> bound_parameter_which_binding_to_use;
	bool at_least_one_row_is_bad = false;

	//std::for_each(rows_to_check.begin(), rows_to_check.end(), [this, &at_least_one_row_is_bad, &bound_parameter_strings, &bound_parameter_ints, &bound_parameter_floats, &bound_parameter_which_binding_to_use, &previous_full_table__each_row_containing_two_sets_of_data_being_cleaned_against_one_another, &xr_table_category](TimeRangeSorter & row)
	for (std::vector<TimeRangeSorter>::iterator it = rows_to_check.begin(); it != rows_to_check.end(); ++it)
	{
		TimeRangeSorter & row = *it;

		std::int64_t datetime_start = row.GetSavedRowData().datetime_start;
		std::int64_t datetime_end = row.GetSavedRowData().datetime_end;
		bool skip_check = false;

		if (datetime_start == 0 && datetime_end == 0)
		{
			skip_check = true;
		}

		if (!skip_check)
		{
			if (datetime_start < timerange_start)
			{
				datetime_start = timerange_start;
			}

			if (datetime_end > timerange_end)
			{
				datetime_end = timerange_end;
			}
		}

		bool dummy = false;
		std::string dummystr;
		ColumnsInTempView dummycols;

		// The function returns the inner table parameters, ordered properly
		bound_parameter_ints.clear();
		bound_parameter_floats.clear();
		bound_parameter_strings.clear();
		bound_parameter_which_binding_to_use.clear();
		bool added = CreateNewXRRow(row.GetSavedRowData(), dummy, "", "", "", dummystr, bound_parameter_strings, bound_parameter_ints, bound_parameter_floats,
									bound_parameter_which_binding_to_use, datetime_start, datetime_end, previous_full_table__each_row_containing_two_sets_of_data_being_cleaned_against_one_another, dummycols, true,
									true, xr_table_category, false, false);

		if (failed || CheckCancelled())
		{
			break;
		}

		if (added)
		{
			// Remove the last two bindings, which are the new timerange columns - we don't have them yet, and we don't need them

			bound_parameter_ints.pop_back();
			bound_parameter_ints.pop_back();
			bound_parameter_which_binding_to_use.pop_back();
			bound_parameter_which_binding_to_use.pop_back();

			// Set the new inner table order
			row.GetSavedRowData().SwapBindings(bound_parameter_strings, bound_parameter_ints, bound_parameter_floats, bound_parameter_which_binding_to_use);

			row.GetSavedRowData().SetLast2DateTimeColumns(datetime_start, datetime_end);
		}
		else
		{
			row.bad_row = true;
			at_least_one_row_is_bad = true;
		}

		//});
	}

	if (failed || CheckCancelled())
	{
		return;
	}

	if (at_least_one_row_is_bad)
	{
		std::vector<TimeRangeSorter> new_vector;

		//std::for_each(rows_to_check.begin(), rows_to_check.end(), [this, &new_vector](TimeRangeSorter & row)
		for (std::vector<TimeRangeSorter>::iterator it = rows_to_check.begin(); it != rows_to_check.end(); ++it)
		{
			TimeRangeSorter & row = *it;

			if (!row.bad_row)
			{
				new_vector.push_back(row);
			}

			//});
		}

		rows_to_check.swap(new_vector);
	}

	// Separate the incoming rows into groups:
	// Each group has the same number of non-NULL primary key groups from all inner tables except the last,
	// and the same values for these primary key groups.
	// The last inner table primary key group might or might not be NULL,
	// and does not have to match.
	std::map<TimeRangeSorter, std::deque<TimeRangeSorter>> rowgroups_separated_into_primarykey_sets;

	// Set a flag to modify the way equality is tested for when element is inserted into the map
	// in the next loop (using operator[], which internally calls TimeRangeSorter::operator<() for equality comparison)
	// when retrieving deque's from the map, there should be one deque for every
	// set of rows that match on all primary key fields EXCEPT the last.
	// They will include all time ranges within this group.
	//std::for_each(rows_to_check.begin(), rows_to_check.end(), [](TimeRangeSorter & row)
	for (std::vector<TimeRangeSorter>::iterator it = rows_to_check.begin(); it != rows_to_check.end(); ++it)
	{
		TimeRangeSorter & row = *it;
		row.ShouldReturnEqual_EvenIf_TimeRangesAreDifferent = true;
		//});
	}

	// Now perform the actual binning of the rows into separate deques,
	// with each deque matching on all but the final inner table (including NULL count),
	// in arbitrary time range order.
	//std::for_each(rows_to_check.cbegin(), rows_to_check.cend(), [&rowgroups_separated_into_primarykey_sets](TimeRangeSorter const & row)
	for (std::vector<TimeRangeSorter>::const_iterator it = rows_to_check.cbegin(); it != rows_to_check.cend(); ++it)
	{
		TimeRangeSorter const & row = *it;
		rowgroups_separated_into_primarykey_sets[row].push_back(row);
		//});
	}

	// Modify the flags so that the sort in the next loop DOES include the time range in the sort.
	//std::for_each(rowgroups_separated_into_primarykey_sets.begin(), rowgroups_separated_into_primarykey_sets.end(), [](std::pair<TimeRangeSorter const, std::deque<TimeRangeSorter>> & row_group)
	for (std::map<TimeRangeSorter, std::deque<TimeRangeSorter>>::iterator it = rowgroups_separated_into_primarykey_sets.begin(); it != rowgroups_separated_into_primarykey_sets.end();
		 ++it)
	{
		std::pair<TimeRangeSorter const, std::deque<TimeRangeSorter>> & row_group = *it;
		std::for_each(row_group.second.begin(), row_group.second.end(), [](TimeRangeSorter & row)
		{
			row.ShouldReturnEqual_EvenIf_TimeRangesAreDifferent = false;
		});
		//});
	}

	// Sort the row groups, this time including time range in the sort.
	// The result will be that each deque will have all rows matching on primary key groups (including the number of NULL's)
	// EXCEPT the final inner table's primary key group.
	// These rows inside each deque will be sorted according to time range.
	//std::for_each(rowgroups_separated_into_primarykey_sets.begin(), rowgroups_separated_into_primarykey_sets.end(), [](std::pair<TimeRangeSorter const, std::deque<TimeRangeSorter>> & row_group)
	for (std::map<TimeRangeSorter, std::deque<TimeRangeSorter>>::iterator it = rowgroups_separated_into_primarykey_sets.begin(); it != rowgroups_separated_into_primarykey_sets.end();
		 ++it)
	{
		std::pair<TimeRangeSorter const, std::deque<TimeRangeSorter>> & row_group = *it;
		std::sort(row_group.second.begin(), row_group.second.end());
		//});
	}

	// The task now is to check to see if there is even a single row in each group (deque)
	// that has non-NULL in the final inner table.
	// If so, it dominates and we can remove all entries that have NULL in the final inner table.
	// If not, we must include all entries (even though they have NULLs in the final inner table).
	// However, the time range must be taken into consideration.

	//std::for_each(rowgroups_separated_into_primarykey_sets.begin(), rowgroups_separated_into_primarykey_sets.end(), [this, &previous_full_table__each_row_containing_two_sets_of_data_being_cleaned_against_one_another, &outgoing_rows_of_data, &saved_rows_with_null_in_final_inner_table, &xr_table_category](std::pair<TimeRangeSorter const, std::deque<TimeRangeSorter>> & row_group)
	for (std::map<TimeRangeSorter, std::deque<TimeRangeSorter>>::iterator it = rowgroups_separated_into_primarykey_sets.begin(); it != rowgroups_separated_into_primarykey_sets.end();
		 ++it)
	{

		std::pair<TimeRangeSorter const, std::deque<TimeRangeSorter>> & row_group = *it;

		// Process rows according to time range.
		// The incoming rows are sorted according to time range (first on start datetime, then on end datetime)

		if (failed || CheckCancelled())
		{
			break;
		}

		// TEST the rows only!!!!!!
		// Do not actually use this data!!!
		//
		// The following function internally calls MergeRows()
		// for all OVERLAPPING time ranges, and in this way eliminates
		// rows that are empty in the final inner table when there
		// is some other row in the same time range that is not empty in the final inner table
		//outgoing_rows_of_data.push_back(std::deque<SavedRowData>());
		std::deque<SavedRowData> outgoing_test_rows;
		HandleSetOfRowsThatMatchOnPrimaryKeys(previous_full_table__each_row_containing_two_sets_of_data_being_cleaned_against_one_another, row_group.second, outgoing_test_rows,
											  xr_table_category);

		if (failed || CheckCancelled())
		{
			break;
		}

		// test_rows returns with two categories of rows: those for which
		// the merging algorithm was unable to find data for the final inner table.
		// in that time range, and those that do have data in the final inner table.
		//
		// THE LATTER DATA IS GARBAGE, because the merging algorithm chooses (arbitrarily)
		// the data to appear if there are an excess number of inner table sets in the merge.
		// However, all we need right now is to know which time ranges have no data in the final
		// inner table.

		std::for_each(outgoing_test_rows.cbegin(), outgoing_test_rows.cend(), [&saved_rows_with_null_in_final_inner_table](const SavedRowData & test_row)
		{
			if (test_row.indices_of_all_primary_key_columns_in_final_inner_table[0].first == SQLExecutor::NULL_BINDING)
			{
				// The time range in this row represents a time range for which no data could be found
				// for the final inner table.  It must be saved.
				saved_rows_with_null_in_final_inner_table.push_back(test_row);
			}
		});

		//});
	}


	if (failed || CheckCancelled())
	{
		return;
	}

	// ******************************************************************************* //
	// Part two: Rerun the entire algorithm,
	// this time grouping rows into deque's INCLUDING
	// the final inner table primary key group.
	// ******************************************************************************* //


	// Separate the incoming rows into groups:
	// Each group has a non-NULL value in EVERY inner table,
	// including the last
	rowgroups_separated_into_primarykey_sets.clear();

	// This time, bucket the rows into groups INCLUDING the final inner table's primary key group.
	//std::for_each(rows_to_check.begin(), rows_to_check.end(), [](TimeRangeSorter & row)
	for (std::vector<TimeRangeSorter>::iterator it = rows_to_check.begin(); it != rows_to_check.end(); ++it)
	{
		TimeRangeSorter & row = *it;
		row.ShouldReturnEqual_EvenIf_TimeRangesAreDifferent = true;
		row.DoCompareFinalInnerTable = true;
		//});
	}

	// Now perform the actual binning of the rows into separate deques,
	// this time with each group guaranteed to have ALL inner tables
	// with non-NULL data, and matching on all inner table primary key groups.
	//std::for_each(rows_to_check.begin(), rows_to_check.end(), [&rowgroups_separated_into_primarykey_sets](TimeRangeSorter & row)
	for (std::vector<TimeRangeSorter>::iterator it = rows_to_check.begin(); it != rows_to_check.end(); ++it)
	{
		TimeRangeSorter & row = *it;
		rowgroups_separated_into_primarykey_sets[row].push_back(row);
		//});
	}

	// Modify the flags so that the sort in the next loop DOES include the time range in the sort.
	//std::for_each(rowgroups_separated_into_primarykey_sets.begin(), rowgroups_separated_into_primarykey_sets.end(), [](std::pair<TimeRangeSorter const, std::deque<TimeRangeSorter>> & row_group)
	for (std::map<TimeRangeSorter, std::deque<TimeRangeSorter>>::iterator it = rowgroups_separated_into_primarykey_sets.begin(); it != rowgroups_separated_into_primarykey_sets.end();
		 ++it)
	{
		std::pair<TimeRangeSorter const, std::deque<TimeRangeSorter>> & row_group = *it;
		std::for_each(row_group.second.begin(), row_group.second.end(), [](TimeRangeSorter & row)
		{
			row.ShouldReturnEqual_EvenIf_TimeRangesAreDifferent = false;
		});
		//});
	}

	// Sort the row groups, this time including time range in the sort.
	// The result will be that each deque will have all rows matching on primary key groups (including the number of NULL's)
	// INCLUDING the final inner table's primary key group.
	// These rows inside each deque will be sorted according to time range.
	//std::for_each(rowgroups_separated_into_primarykey_sets.begin(), rowgroups_separated_into_primarykey_sets.end(), [](std::pair<TimeRangeSorter const, std::deque<TimeRangeSorter>> & row_group)
	for (std::map<TimeRangeSorter, std::deque<TimeRangeSorter>>::iterator it = rowgroups_separated_into_primarykey_sets.begin(); it != rowgroups_separated_into_primarykey_sets.end();
		 ++it)
	{
		std::pair<TimeRangeSorter const, std::deque<TimeRangeSorter>> & row_group = *it;
		std::sort(row_group.second.begin(), row_group.second.end());
		//});
	}


	// Same as before, except this time it's not a test...
	// Unlike the above equivalent-looking loop,
	// each group handled in the following loop has the SAME primary key group
	// in the final inner table.
	// So there won't be garbage produced during the merge.

	std::vector<SavedRowData> saved_complete_rows;

	//std::for_each(rowgroups_separated_into_primarykey_sets.begin(), rowgroups_separated_into_primarykey_sets.end(), [this, &previous_full_table__each_row_containing_two_sets_of_data_being_cleaned_against_one_another, &group_time_ranges__intkeys, &group_time_ranges__floatkeys, &group_time_ranges__stringkeys, &outgoing_rows_of_data, &saved_complete_rows, &xr_table_category](std::pair<TimeRangeSorter const, std::deque<TimeRangeSorter>> & row_group)
	for (std::map<TimeRangeSorter, std::deque<TimeRangeSorter>>::iterator it = rowgroups_separated_into_primarykey_sets.begin(); it != rowgroups_separated_into_primarykey_sets.end();
		 ++it)
	{

		std::pair<TimeRangeSorter const, std::deque<TimeRangeSorter>> & row_group = *it;

		// Process rows according to time range.
		// The incoming rows are sorted according to time range (first on start datetime, then on end datetime)

		if (failed || CheckCancelled())
		{
			break;
		}

		// This time it's real data that comes out!
		// (Unlike the above equivalent-looking loop.)
		//
		// The following function internally calls MergeRows()
		// but this time all primary keys match, so the results
		// will be a legitimate merging/splitting of rows into proper time ranges.
		std::deque<SavedRowData> outgoing_real_rows;
		HandleSetOfRowsThatMatchOnPrimaryKeys(previous_full_table__each_row_containing_two_sets_of_data_being_cleaned_against_one_another, row_group.second, outgoing_real_rows,
											  xr_table_category);

		if (failed || CheckCancelled())
		{
			break;
		}

		// real_rows returns with two categories of rows, just as test_rows
		// in the above equivalent-looking loop did:
		// those for which
		// the merging algorithm was unable to find data for the final inner table.
		// in that time range, and those that do have data in the final inner table.
		//
		// However, unlike in the above loop, this time we want to keep only
		// the rows that have data in the final inner table,
		// because we've already saved the rows that don't (from the above loop).

		//std::for_each(outgoing_real_rows.cbegin(), outgoing_real_rows.cend(), [&saved_complete_rows, &group_time_ranges__intkeys, &group_time_ranges__floatkeys, &group_time_ranges__stringkeys](const SavedRowData & test_row)
		for (std::deque<SavedRowData>::const_iterator it = outgoing_real_rows.cbegin(); it != outgoing_real_rows.cend(); ++it)
		{
			const SavedRowData & test_row = *it;

			if (test_row.indices_of_all_primary_key_columns_in_final_inner_table[0].first != SQLExecutor::NULL_BINDING)
			{
				// The time range in this row represents a time range for which data is present
				// in the final inner table.  It must be saved.
				saved_complete_rows.push_back(test_row);

				if (test_row.indices_of_primary_key_columns_with_multiplicity_greater_than_1[0].first == SQLExecutor::INT64)
				{
					// Tricky use of map!  We only insert into map when no inner tables are NULL.
					//
					// Our map comparison will behave oddly if this is violated
					// because the comparison will return equality for inner sets that have a smaller number of elements
					// when compared against a full set.  This is how, later, we test to see
					// if a row with a NULL should be included.
					// But never insert an element with a smaller number of inner tables!
					// This could overwrite the map entry!!!
					//
					// Guaranteed to have no NULL primary key groups, so the following map insertion
					// (at this phase) will never return a match with a group that has a smaller
					// number of inner tables.  Later, when the "saved_rows_with_null_in_final_inner_table"
					// data structure is processed, there *will* be checks into the map against
					// a smaller number of (non-NULL) inner tables - but not yet.
					std::set<std::vector<std::int64_t>> inner_table_primary_key_groups;
					test_row.ReturnAllNonNullPrimaryKeyGroups(inner_table_primary_key_groups);
					TimeRanges & time_ranges = group_time_ranges__intkeys[TimeRangeMapper_Ints(inner_table_primary_key_groups)];
					time_ranges.append(test_row.datetime_start, test_row.datetime_end);
				}
				else if (test_row.indices_of_primary_key_columns_with_multiplicity_greater_than_1[0].first == SQLExecutor::FLOAT)
				{
					std::set<std::vector<long double>> inner_table_primary_key_groups;
					test_row.ReturnAllNonNullPrimaryKeyGroups(inner_table_primary_key_groups);
					TimeRanges & time_ranges = group_time_ranges__floatkeys[TimeRangeMapper_Floats(inner_table_primary_key_groups)];
					time_ranges.append(test_row.datetime_start, test_row.datetime_end);
				}
				else
				{
					std::set<std::vector<std::string>> inner_table_primary_key_groups;
					test_row.ReturnAllNonNullPrimaryKeyGroups(inner_table_primary_key_groups);
					TimeRanges & time_ranges = group_time_ranges__stringkeys[TimeRangeMapper_Strings(inner_table_primary_key_groups)];
					time_ranges.append(test_row.datetime_start, test_row.datetime_end);
				}
			}

			//});
		}

		if (failed || CheckCancelled())
		{
			break;
		}

		//});
	}

	if (failed || CheckCancelled())
	{
		return;
	}

	// We have the final results.
	//
	// Rows that arrived from the previous multiplicity stage with NULL's for some primary key inner tables,
	// even before being merged with the current multiplicity, are saved in "saved_rows_with_multiple_nulls".
	//
	// Rows with legitimate NULLs in the final inner table are saved in "saved_rows_with_null_in_final_inner_table".
	// ... These are processed by the outer loop, not here.
	//
	// Rows with all inner tables populated with data are saved in "saved_complete_rows".

	outgoing_rows_of_data.insert(outgoing_rows_of_data.begin(), saved_rows_with_multiple_nulls.cbegin(), saved_rows_with_multiple_nulls.cend());
	outgoing_rows_of_data.insert(outgoing_rows_of_data.begin(), saved_complete_rows.cbegin(), saved_complete_rows.cend());

}

void OutputModel::OutputGenerator::HandleCompletionOfProcessingOfNormalizedGroupOfMatchingRowsInXRalgorithm(std::vector<SavedRowData> & saved_rows_with_null_in_final_inner_table,
		TimeRangesForIndividualGroup_IntKeys & group_time_ranges__intkeys, TimeRangesForIndividualGroup_FloatKeys & group_time_ranges__floatkeys,
		TimeRangesForIndividualGroup_StringKeys & group_time_ranges__stringkeys, std::vector<TimeRangeSorter> const & rows_to_check_for_duplicates_in_newly_joined_primary_key_columns,
		int const previous_datetime_start_column_index, int const current_datetime_start_column_index, int const previous_datetime_end_column_index,
		int const current_datetime_end_column_index, XR_TABLE_CATEGORY const xr_table_category, std::vector<SQLExecutor> & sql_strings, sqlite3_stmt *& the_prepared_stmt,
		std::shared_ptr<bool> & statement_is_prepared, std::int64_t & current_rows_added, std::int64_t & current_rows_added_since_execution, bool & first_row_added,
		std::string const & datetime_start_col_name, std::string const & datetime_end_col_name, ColumnsInTempView & result_columns, std::string & sql_add_xr_row,
		std::vector<std::string> & bound_parameter_strings, std::vector<std::int64_t> & bound_parameter_ints, std::vector<long double> & bound_parameter_floats,
		std::vector<SQLExecutor::WHICH_BINDING> & bound_parameter_which_binding_to_use, std::int64_t & datetime_range_start, std::int64_t & datetime_range_end,
		ColumnsInTempView const & previous_full_table__each_row_containing_two_sets_of_data_being_cleaned_against_one_another)
{

	if (rows_to_check_for_duplicates_in_newly_joined_primary_key_columns.empty())
	{
		return;
	}

	std::vector<SavedRowData> outgoing_rows_of_data;
	Process_RowsToCheckForDuplicates_ThatMatchOnAllButFinalInnerTable_ExceptForNullCount_InXRalgorithm(saved_rows_with_null_in_final_inner_table, group_time_ranges__intkeys,
			group_time_ranges__floatkeys, group_time_ranges__stringkeys, previous_full_table__each_row_containing_two_sets_of_data_being_cleaned_against_one_another, outgoing_rows_of_data,
			rows_to_check_for_duplicates_in_newly_joined_primary_key_columns, previous_datetime_start_column_index, current_datetime_start_column_index, previous_datetime_end_column_index,
			current_datetime_end_column_index, xr_table_category);
	AddRowsToXRTable(outgoing_rows_of_data, sql_strings, the_prepared_stmt, statement_is_prepared, current_rows_added, current_rows_added_since_execution, first_row_added,
					 datetime_start_col_name, datetime_end_col_name, result_columns, sql_add_xr_row, bound_parameter_strings, bound_parameter_ints, bound_parameter_floats,
					 bound_parameter_which_binding_to_use, datetime_range_start, datetime_range_end, previous_full_table__each_row_containing_two_sets_of_data_being_cleaned_against_one_another);

	if (failed || CheckCancelled())
	{
		return;
	}

}

void OutputModel::OutputGenerator::AddRowsToXRTable(std::vector<SavedRowData> & outgoing_rows_of_data, std::vector<SQLExecutor> & sql_strings, sqlite3_stmt *& the_prepared_stmt,
		std::shared_ptr<bool> & statement_is_prepared, std::int64_t & current_rows_added, std::int64_t & current_rows_added_since_execution, bool & first_row_added,
		std::string const & datetime_start_col_name, std::string const & datetime_end_col_name, ColumnsInTempView & result_columns, std::string & sql_add_xr_row,
		std::vector<std::string> & bound_parameter_strings, std::vector<std::int64_t> & bound_parameter_ints, std::vector<long double> & bound_parameter_floats,
		std::vector<SQLExecutor::WHICH_BINDING> & bound_parameter_which_binding_to_use, std::int64_t & datetime_range_start, std::int64_t & datetime_range_end,
		ColumnsInTempView const & previous_full_table__each_row_containing_two_sets_of_data_being_cleaned_against_one_another)
{

	std::for_each(outgoing_rows_of_data.cbegin(),
				  outgoing_rows_of_data.cend(), [this, &sql_strings, &the_prepared_stmt, &statement_is_prepared, &current_rows_added, &current_rows_added_since_execution, &first_row_added, &datetime_start_col_name, &datetime_end_col_name, &result_columns, &sql_add_xr_row, &bound_parameter_strings, &bound_parameter_ints, &bound_parameter_floats, &bound_parameter_which_binding_to_use, &datetime_range_start, &datetime_range_end, &previous_full_table__each_row_containing_two_sets_of_data_being_cleaned_against_one_another](
					  SavedRowData const & new_row_to_write_to_database)
	{
		if (failed || CheckCancelled())
		{
			return;
		}

		// The previous function already set both the datetime columns and the data in the previous/current inner tables (including NULLs where necessary),
		// so just tell the following function to use all data, and to use the existing date and time
		std::int64_t datetime_start = new_row_to_write_to_database.datetime_start;
		std::int64_t datetime_end = new_row_to_write_to_database.datetime_end;
		bool skip_check = false;

		if (datetime_start == 0 && datetime_end == 0)
		{
			skip_check = true;
		}

		if (!skip_check)
		{
			if (datetime_start < timerange_start)
			{
				datetime_start = timerange_start;
			}

			if (datetime_end > timerange_end)
			{
				datetime_end = timerange_end;
			}
		}

		bool added = CreateNewXRRow(new_row_to_write_to_database, first_row_added, datetime_start_col_name, datetime_end_col_name, result_columns.view_name, sql_add_xr_row,
									bound_parameter_strings, bound_parameter_ints, bound_parameter_floats, bound_parameter_which_binding_to_use, datetime_start, datetime_end,
									previous_full_table__each_row_containing_two_sets_of_data_being_cleaned_against_one_another, result_columns, true, true, XR_TABLE_CATEGORY::PRIMARY_VARIABLE_GROUP, false, false);

		if (failed || CheckCancelled())
		{
			return;
		}

		if (added)
		{
			SQLExecutor::Execute(false, SQLExecutor::DOES_NOT_RETURN_ROWS, this, db, sql_add_xr_row, bound_parameter_strings, bound_parameter_ints, bound_parameter_floats,
								 bound_parameter_which_binding_to_use, statement_is_prepared, the_prepared_stmt, true);

			if (failed)
			{
				SetFailureMessage(sql_error);
				return;
			}

			if (CheckCancelled())
			{
				return;
			}

			the_prepared_stmt = SQLExecutor::stmt_insert;
			++current_rows_added;
			++current_rows_added_since_execution;
		}
	});

}

void OutputModel::OutputGenerator::SavedRowData::SwapBindings(std::vector<std::string> const & new_strings,
		std::vector<std::int64_t> const & new_ints,
		std::vector<long double> const & new_floats,
		std::vector<SQLExecutor::WHICH_BINDING> const & new_bindings,
		bool enforce_all_datetimes,
		std::int64_t const startdate_current,
		std::int64_t const enddate_current,
		std::int64_t const startdate_previous,
		std::int64_t const enddate_previous,
		int const current_datetime_start_column_index,
		int const current_datetime_end_column_index,
		int const previous_datetime_start_column_index,
		int const previous_datetime_end_column_index)
{

	std::vector<std::pair<SQLExecutor::WHICH_BINDING, std::pair<int, int>>> new_indices;
	int string_index = 0;
	int int_index = 0;
	int float_index = 0;
	int column_index = 0;
	std::for_each(new_bindings.cbegin(), new_bindings.cend(), [this, &column_index, &string_index, &int_index, &float_index, &new_indices](SQLExecutor::WHICH_BINDING const binding)
	{
		switch (binding)
		{
			case SQLExecutor::INT64:
				{
					new_indices.push_back(std::make_pair(binding, std::make_pair(int_index, column_index)));
					++int_index;
				}
				break;

			case SQLExecutor::FLOAT:
				{
					new_indices.push_back(std::make_pair(binding, std::make_pair(float_index, column_index)));
					++float_index;
				}
				break;

			case SQLExecutor::STRING:
				{
					new_indices.push_back(std::make_pair(binding, std::make_pair(string_index, column_index)));
					++string_index;
				}
				break;

			case SQLExecutor::NULL_BINDING:
				{
					new_indices.push_back(std::make_pair(binding, std::make_pair(0, column_index)));
				}
				break;

			default:
				break;
		}

		++column_index;
	});

	SwapBindings(new_strings,
				 new_ints,
				 new_floats,
				 new_indices,
				 enforce_all_datetimes,
				 startdate_current,
				 enddate_current,
				 startdate_previous,
				 enddate_previous,
				 current_datetime_start_column_index,
				 current_datetime_end_column_index,
				 previous_datetime_start_column_index,
				 previous_datetime_end_column_index);

}

void OutputModel::OutputGenerator::SavedRowData::SwapBindings(std::vector<std::string> const & new_strings,
		std::vector<std::int64_t> const & new_ints,
		std::vector<long double> const & new_floats,
		std::vector < std::pair < SQLExecutor::WHICH_BINDING,
		std::pair<int, int >>> & new_indices,
		bool enforce_all_datetimes,
		std::int64_t const startdate_current,
		std::int64_t const enddate_current,
		std::int64_t const startdate_previous,
		std::int64_t const enddate_previous,
		int const current_datetime_start_column_index,
		int const current_datetime_end_column_index,
		int const previous_datetime_start_column_index,
		int const previous_datetime_end_column_index)
{

	indices_of_all_columns = new_indices;

	current_parameter_ints.clear();
	current_parameter_floats.clear();
	current_parameter_strings.clear();
	current_parameter_which_binding_to_use.clear();

	indices_of_all_columns_in_final_inner_table.clear();
	indices_of_all_columns_in_all_but_final_inner_table.clear();
	indices_of_primary_key_columns.clear();
	indices_of_primary_key_columns_with_multiplicity_greater_than_1.clear();
	indices_of_primary_key_columns_with_multiplicity_equal_to_1.clear();
	indices_of_all_primary_key_columns_in_final_inner_table.clear();
	indices_of_all_primary_key_columns_in_all_but_final_inner_table.clear();
	indices_of_secondary_key_columns.clear();

	int column_index = 0;
	int current_int_index = 0;
	int current_float_index = 0;
	int current_string_index = 0;
	int current_int_index_new = 0;
	int current_float_index_new = 0;
	int current_string_index_new = 0;
	std::for_each(indices_of_all_columns.begin(),
				  indices_of_all_columns.end(), [this, &current_int_index_new, &current_float_index_new, &current_string_index_new, &new_ints, &new_floats, &new_strings, &enforce_all_datetimes, &startdate_current, &enddate_current, &startdate_previous, &enddate_previous, &previous_datetime_start_column_index, &previous_datetime_end_column_index, &current_datetime_start_column_index, &current_datetime_end_column_index, &column_index, &current_int_index, &current_float_index, &current_string_index](
					  std::pair<SQLExecutor::WHICH_BINDING, std::pair<int, int>> & binding)
	{

		std::pair<SQLExecutor::WHICH_BINDING, std::pair<int, int>> old_binding = binding;

		bool handled = false;

		if (enforce_all_datetimes)
		{
			if (column_index == current_datetime_start_column_index)
			{
				binding.first = SQLExecutor::INT64;
				current_parameter_ints.push_back(startdate_current);
				binding.second.first = current_int_index_new;
				binding.second.second = column_index;
				handled = true;
			}

			if (column_index == current_datetime_end_column_index)
			{
				binding.first = SQLExecutor::INT64;
				current_parameter_ints.push_back(enddate_current);
				binding.second.first = current_int_index_new;
				binding.second.second = column_index;
				handled = true;
			}

			if (column_index == previous_datetime_start_column_index)
			{
				binding.first = SQLExecutor::INT64;
				current_parameter_ints.push_back(startdate_previous);
				binding.second.first = current_int_index_new;
				binding.second.second = column_index;
				handled = true;
			}

			if (column_index == previous_datetime_end_column_index)
			{
				binding.first = SQLExecutor::INT64;
				current_parameter_ints.push_back(enddate_previous);
				binding.second.first = current_int_index_new;
				binding.second.second = column_index;
				handled = true;
			}
		}

		if (!handled)
		{
			switch (binding.first)
			{
				case SQLExecutor::INT64:
					{
						current_parameter_ints.push_back(new_ints[current_int_index]);
					}
					break;

				case SQLExecutor::FLOAT:
					{
						current_parameter_floats.push_back(new_floats[current_float_index]);
					}
					break;

				case SQLExecutor::STRING:
					{
						current_parameter_strings.push_back(new_strings[current_string_index]);
					}
					break;

				case SQLExecutor::NULL_BINDING:
					{

					}
					break;

				default:
					break;
			}
		}

		current_parameter_which_binding_to_use.push_back(binding.first);

		std::pair<SQLExecutor::WHICH_BINDING, std::pair<int, int>> current_int_binding_to_add = std::make_pair(binding.first, std::make_pair(current_int_index_new, column_index));
		std::pair<SQLExecutor::WHICH_BINDING, std::pair<int, int>> current_float_binding_to_add = std::make_pair(binding.first, std::make_pair(current_float_index_new, column_index));
		std::pair<SQLExecutor::WHICH_BINDING, std::pair<int, int>> current_string_binding_to_add = std::make_pair(binding.first, std::make_pair(current_string_index_new, column_index));
		//std::pair<SQLExecutor::WHICH_BINDING, std::pair<int, int>> current_null_binding_to_add = std::make_pair(binding.first, std::make_pair(0, column_index));

		AddBinding(is_index_in_final_inner_table, indices_of_all_columns_in_final_inner_table, binding.first, column_index, current_int_binding_to_add, current_float_binding_to_add,
				   current_string_binding_to_add);
		AddBinding(is_index_in_all_but_final_inner_table, indices_of_all_columns_in_all_but_final_inner_table, binding.first, column_index, current_int_binding_to_add,
				   current_float_binding_to_add, current_string_binding_to_add);
		AddBinding(is_index_a_primary_key, indices_of_primary_key_columns, binding.first, column_index, current_int_binding_to_add, current_float_binding_to_add,
				   current_string_binding_to_add);
		AddBinding(is_index_a_primary_key_with_outer_multiplicity_greater_than_1, indices_of_primary_key_columns_with_multiplicity_greater_than_1, binding.first, column_index,
				   current_int_binding_to_add, current_float_binding_to_add, current_string_binding_to_add);
		AddBinding(is_index_a_primary_key_with_outer_multiplicity_equal_to_1, indices_of_primary_key_columns_with_multiplicity_equal_to_1, binding.first, column_index,
				   current_int_binding_to_add, current_float_binding_to_add, current_string_binding_to_add);
		AddBinding(is_index_a_primary_key_in_the_final_inner_table, indices_of_all_primary_key_columns_in_final_inner_table, binding.first, column_index, current_int_binding_to_add,
				   current_float_binding_to_add, current_string_binding_to_add);
		AddBinding(is_index_a_primary_key_in_not_the_final_inner_table, indices_of_all_primary_key_columns_in_all_but_final_inner_table, binding.first, column_index,
				   current_int_binding_to_add, current_float_binding_to_add, current_string_binding_to_add);
		AddBinding(is_index_a_secondary_key, indices_of_secondary_key_columns, binding.first, column_index, current_int_binding_to_add, current_float_binding_to_add,
				   current_string_binding_to_add);

		switch (old_binding.first)
		{
			case SQLExecutor::INT64:
				{
					++current_int_index;
				}
				break;

			case SQLExecutor::FLOAT:
				{
					++current_float_index;
				}
				break;

			case SQLExecutor::STRING:
				{
					++current_string_index;
				}
				break;

			case SQLExecutor::NULL_BINDING:
				{

				}
				break;

			default:
				break;
		}

		switch (binding.first)
		{
			case SQLExecutor::INT64:
				{
					++current_int_index_new;
				}
				break;

			case SQLExecutor::FLOAT:
				{
					++current_float_index_new;
				}
				break;

			case SQLExecutor::STRING:
				{
					++current_string_index_new;
				}
				break;

			case SQLExecutor::NULL_BINDING:
				{

				}
				break;

			default:
				break;
		}

		++column_index;
	});
}

void OutputModel::OutputGenerator::SavedRowData::AddBinding(std::vector<bool> const & binding_test,
		std::vector<std::pair<SQLExecutor::WHICH_BINDING, std::pair<int, int>>> & bindings, SQLExecutor::WHICH_BINDING binding_type, int const binding_index,
		std::pair<SQLExecutor::WHICH_BINDING, std::pair<int, int>> const & potential_current_int_binding_to_add,
		std::pair<SQLExecutor::WHICH_BINDING, std::pair<int, int>> const & potential_current_float_binding_to_add,
		std::pair<SQLExecutor::WHICH_BINDING, std::pair<int, int>> const & potential_current_string_binding_to_add)
{
	if (binding_test[binding_index])
	{
		switch (binding_type)
		{
			case SQLExecutor::INT64:
				{
					bindings.push_back(potential_current_int_binding_to_add);
				}
				break;

			case SQLExecutor::FLOAT:
				{
					bindings.push_back(potential_current_float_binding_to_add);
				}
				break;

			case SQLExecutor::STRING:
				{
					bindings.push_back(potential_current_string_binding_to_add);
				}
				break;

			case SQLExecutor::NULL_BINDING:
				{
					bindings.push_back(std::make_pair(SQLExecutor::NULL_BINDING, std::make_pair(0, binding_index)));
				}
				break;

			default:
				break;
		}
	}
}

void OutputModel::OutputGenerator::SavedRowData::SetFinalInnerTableToNull(bool const set_datetime_to_previous_block)
{
	int column_index = 0;
	int final_inner_table_column_index = 0;
	int final_inner_table_primary_key_column_index = 0;
	int primary_key_column_index = 0;
	int primary_key_with_multiplicity_1_index = 0;
	int primary_key_with_multiplicity_greater_than_1_index = 0;
	int secondary_key_column_index = 0;
	std::for_each(indices_of_all_columns.begin(),
				  indices_of_all_columns.end(), [this, &primary_key_with_multiplicity_greater_than_1_index, &primary_key_with_multiplicity_1_index, &primary_key_column_index, &secondary_key_column_index, &final_inner_table_primary_key_column_index, &final_inner_table_column_index, &column_index](
					  std::pair<SQLExecutor::WHICH_BINDING, std::pair<int, int>> & binding_info)
	{

		if (is_index_in_final_inner_table[column_index])
		{

			binding_info.first = SQLExecutor::NULL_BINDING;
			binding_info.second.first = 0;

			switch (binding_info.first)
			{
				case SQLExecutor::INT64:
					{
						current_parameter_ints.pop_back();
					}
					break;

				case SQLExecutor::FLOAT:
					{
						current_parameter_floats.pop_back();
					}
					break;

				case SQLExecutor::STRING:
					{
						current_parameter_strings.pop_back();
					}

				default:
					break;
			}

			current_parameter_which_binding_to_use[column_index] = SQLExecutor::NULL_BINDING;
			indices_of_all_columns_in_final_inner_table[final_inner_table_column_index].first = SQLExecutor::NULL_BINDING;
			indices_of_all_columns_in_final_inner_table[final_inner_table_column_index].second.first = 0;

			if (is_index_a_primary_key_in_the_final_inner_table[column_index])
			{
				indices_of_all_primary_key_columns_in_final_inner_table[final_inner_table_primary_key_column_index].first = SQLExecutor::NULL_BINDING;
				indices_of_all_primary_key_columns_in_final_inner_table[final_inner_table_primary_key_column_index].second.first = 0;
				++final_inner_table_primary_key_column_index;
			}

			if (is_index_a_primary_key[column_index])
			{
				indices_of_primary_key_columns[primary_key_column_index].first = SQLExecutor::NULL_BINDING;
				indices_of_primary_key_columns[primary_key_column_index].second.first = 0;
				++primary_key_column_index;
			}

			if (is_index_a_primary_key_with_outer_multiplicity_equal_to_1[column_index])
			{
				indices_of_primary_key_columns_with_multiplicity_equal_to_1[primary_key_with_multiplicity_1_index].first = SQLExecutor::NULL_BINDING;
				indices_of_primary_key_columns_with_multiplicity_equal_to_1[primary_key_with_multiplicity_1_index].second.first = 0;
				++primary_key_with_multiplicity_1_index;
			}

			if (is_index_a_primary_key_with_outer_multiplicity_greater_than_1[column_index])
			{
				indices_of_primary_key_columns_with_multiplicity_greater_than_1[primary_key_with_multiplicity_greater_than_1_index].first = SQLExecutor::NULL_BINDING;
				indices_of_primary_key_columns_with_multiplicity_greater_than_1[primary_key_with_multiplicity_greater_than_1_index].second.first = 0;
				++primary_key_with_multiplicity_greater_than_1_index;
			}

			if (is_index_a_secondary_key[column_index])
			{
				indices_of_secondary_key_columns[secondary_key_column_index].first = SQLExecutor::NULL_BINDING;
				indices_of_secondary_key_columns[secondary_key_column_index].second.first = 0;
				++secondary_key_column_index;
			}

			++final_inner_table_column_index;

		}

		++column_index;

	});

	// With final inner table set to NULL, the previous block's final datetime columns
	// are guaranteed to be the last two ints
	if (set_datetime_to_previous_block)
	{
		if (current_parameter_ints.size() > 1)
		{
			datetime_start = current_parameter_ints[(int)current_parameter_ints.size() - 2];
			datetime_end = current_parameter_ints[(int)current_parameter_ints.size() - 1];
		}
	}

}

void OutputModel::OutputGenerator::EliminateRedundantNullsInFinalInnerTable(std::vector<SavedRowData> & saved_rows_with_null_in_final_inner_table,
		TimeRangesForIndividualGroup_IntKeys const & group_time_ranges__intkeys, TimeRangesForIndividualGroup_FloatKeys const & group_time_ranges__floatkeys,
		TimeRangesForIndividualGroup_StringKeys const & group_time_ranges__stringkeys)
{

	if (saved_rows_with_null_in_final_inner_table.empty())
	{
		return;
	}

	std::vector<SavedRowData> outgoing_rows;

	// Set progress feedback for the following time-consuming loop.
	std::int64_t number_null_rows = saved_rows_with_null_in_final_inner_table.size();
	std::int64_t number_map_entries = (std::int64_t)group_time_ranges__intkeys.size();
	std::int64_t total_number_null_checks = number_null_rows * number_map_entries;
	std::int64_t current_null_check_count = 0;
	std::int64_t status_increment_check = 1000;
	boost::format msg("Processing potential NULLs: 0 / %1%");
	msg % total_number_null_checks;
	this->messager.SetPerformanceLabel(msg.str());

	std::for_each(saved_rows_with_null_in_final_inner_table.begin(),
				  saved_rows_with_null_in_final_inner_table.end(), [this, &current_null_check_count, &total_number_null_checks, &status_increment_check, &outgoing_rows, &group_time_ranges__intkeys, &group_time_ranges__floatkeys, &group_time_ranges__stringkeys](
					  SavedRowData & saved_row_data_with_null_at_end)
	{

		if (failed || CheckCancelled())
		{
			return;
		}

		bool use_ints = false;
		bool use_floats = false;

		if (saved_row_data_with_null_at_end.indices_of_primary_key_columns_with_multiplicity_greater_than_1[0].first == SQLExecutor::NULL_BINDING)
		{
			return;
		}

		if (saved_row_data_with_null_at_end.indices_of_primary_key_columns_with_multiplicity_greater_than_1[0].first == SQLExecutor::INT64)
		{
			use_ints = true;
		}

		if (saved_row_data_with_null_at_end.indices_of_primary_key_columns_with_multiplicity_greater_than_1[0].first == SQLExecutor::FLOAT)
		{
			use_floats = true;
		}

		if (use_ints)
		{

			std::set<std::vector<std::int64_t>> inner_table_primary_key_groups;
			saved_row_data_with_null_at_end.ReturnAllNonNullPrimaryKeyGroups(inner_table_primary_key_groups);

			if (group_time_ranges__intkeys.find(inner_table_primary_key_groups) == group_time_ranges__intkeys.cend())
			{
				// No other row exists (with all inner tables populated, including the last)
				// with an overlapping primary key group set as the current one has.
				// So we definitely want to keep this one, despite the fact that it has a NULL at the end.
				outgoing_rows.push_back(saved_row_data_with_null_at_end);
				return;
			}

			// There is overlap with some other row that is able to populate every inner table.
			// Therefore, where we overlap time ranges with such rows, we must not appear in the output;
			// where we don't overlap, we must appear in the output.

			// Populate a new TimeRanges object to track all time ranges for which we overlap with other sets.

			TimeRanges my_time_ranges;
			my_time_ranges.append(saved_row_data_with_null_at_end.datetime_start, saved_row_data_with_null_at_end.datetime_end);

			// This part is nasty and dangerously time-consuming.
			// However, the algorithm demands it.
			// We have no choice but to iterate through the map to pull out matches.
			//std::for_each(group_time_ranges__intkeys.cbegin(), group_time_ranges__intkeys.cend(), [this, &done, &current_null_check_count, &total_number_null_checks, &status_increment_check, &my_time_ranges, &inner_table_primary_key_groups](std::pair<TimeRangeMapper_Ints, TimeRanges> const & map_info)
			for (TimeRangesForIndividualGroup_IntKeys::const_iterator it = group_time_ranges__intkeys.cbegin(); it != group_time_ranges__intkeys.cend(); ++it)
			{
				std::pair<TimeRangeMapper_Ints, TimeRanges> const & map_info = *it;

				if (failed || CheckCancelled())
				{
					break;
				}

				if (map_info.first == inner_table_primary_key_groups)
				{
					TimeRanges const & time_range_ = map_info.second;
					my_time_ranges.subtract(time_range_);

					if (my_time_ranges.empty())
					{
						break;
					}
				}

				if (current_null_check_count % status_increment_check == 0)
				{
					boost::format msg("Processing potential NULLs: %1% / %2%");
					msg % current_null_check_count % total_number_null_checks;
					this->messager.SetPerformanceLabel(msg.str());
				}

				++current_null_check_count;
				//});
			}

			if (failed || CheckCancelled())
			{
				return;
			}

			// ******************************************************************************************************* //
			// Whatever time ranges are left require rows over that time range, even though we have a NULL at the end.
			// ******************************************************************************************************* //

			std::for_each(my_time_ranges.ranges.cbegin(), my_time_ranges.ranges.cend(), [&saved_row_data_with_null_at_end, &outgoing_rows](std::pair<std::int64_t, std::int64_t> const & range)
			{
				outgoing_rows.push_back(saved_row_data_with_null_at_end);
				SavedRowData & new_null_row = outgoing_rows.back();
				new_null_row.datetime_start = range.first;
				new_null_row.datetime_end = range.second;
			});

		}
		else if (use_floats)
		{

			std::set<std::vector<long double>> inner_table_primary_key_groups;
			saved_row_data_with_null_at_end.ReturnAllNonNullPrimaryKeyGroups(inner_table_primary_key_groups);

			if (group_time_ranges__floatkeys.find(inner_table_primary_key_groups) == group_time_ranges__floatkeys.cend())
			{
				// No other row exists (with all inner tables populated, including the last)
				// with an overlapping primary key group set as the current one has.
				// So we definitely want to keep this one, despite the fact that it has a NULL at the end.
				outgoing_rows.push_back(saved_row_data_with_null_at_end);
				return;
			}

			// There is overlap with some other row that is able to populate every inner table.
			// Therefore, where we overlap time ranges with such rows, we must not appear in the output;
			// where we don't overlap, we must appear in the output.

			// Populate a new TimeRanges object to track all time ranges for which we overlap with other sets.

			TimeRanges my_time_ranges;
			my_time_ranges.append(saved_row_data_with_null_at_end.datetime_start, saved_row_data_with_null_at_end.datetime_end);

			// This part is nasty and dangerously time-consuming.
			// However, the algorithm demands it.
			// We have no choice but to iterate through the map to pull out matches.
			//std::for_each(group_time_ranges__floatkeys.cbegin(), group_time_ranges__floatkeys.cend(), [this, &done, &current_null_check_count, &total_number_null_checks, &status_increment_check, &my_time_ranges, &inner_table_primary_key_groups](std::pair<TimeRangeMapper_Floats, TimeRanges> const & map_info)
			for (TimeRangesForIndividualGroup_FloatKeys::const_iterator it = group_time_ranges__floatkeys.cbegin(); it != group_time_ranges__floatkeys.cend(); ++it)
			{
				std::pair<TimeRangeMapper_Floats, TimeRanges> const & map_info = *it;

				if (failed || CheckCancelled())
				{
					break;
				}

				if (map_info.first == inner_table_primary_key_groups)
				{
					TimeRanges const & time_range_ = map_info.second;
					my_time_ranges.subtract(time_range_);

					if (my_time_ranges.empty())
					{
						break;
					}
				}

				if (current_null_check_count % status_increment_check == 0)
				{
					boost::format msg("Processing potential NULLs: %1% / %2%");
					msg % current_null_check_count % total_number_null_checks;
					this->messager.SetPerformanceLabel(msg.str());
				}

				++current_null_check_count;
				//});
			}


			if (failed || CheckCancelled())
			{
				return;
			}

			// ******************************************************************************************************* //
			// Whatever time ranges are left require rows over that time range, even though we have a NULL at the end.
			// ******************************************************************************************************* //

			std::for_each(my_time_ranges.ranges.cbegin(), my_time_ranges.ranges.cend(), [&saved_row_data_with_null_at_end, &outgoing_rows](std::pair<std::int64_t, std::int64_t> const & range)
			{
				outgoing_rows.push_back(saved_row_data_with_null_at_end);
				SavedRowData & new_null_row = outgoing_rows.back();
				new_null_row.datetime_start = range.first;
				new_null_row.datetime_end = range.second;
			});

		}
		else
		{

			std::set<std::vector<std::string>> inner_table_primary_key_groups;
			saved_row_data_with_null_at_end.ReturnAllNonNullPrimaryKeyGroups(inner_table_primary_key_groups);

			if (group_time_ranges__stringkeys.find(inner_table_primary_key_groups) == group_time_ranges__stringkeys.cend())
			{
				// No other row exists (with all inner tables populated, including the last)
				// with an overlapping primary key group set as the current one has.
				// So we definitely want to keep this one, despite the fact that it has a NULL at the end.
				outgoing_rows.push_back(saved_row_data_with_null_at_end);
				return;
			}

			// There is overlap with some other row that is able to populate every inner table.
			// Therefore, where we overlap time ranges with such rows, we must not appear in the output;
			// where we don't overlap, we must appear in the output.

			// Populate a new TimeRanges object to track all time ranges for which we overlap with other sets.

			TimeRanges my_time_ranges;
			my_time_ranges.append(saved_row_data_with_null_at_end.datetime_start, saved_row_data_with_null_at_end.datetime_end);

			// This part is nasty and dangerously time-consuming.
			// However, the algorithm demands it.
			// We have no choice but to iterate through the map to pull out matches.
			//std::for_each(group_time_ranges__stringkeys.cbegin(), group_time_ranges__stringkeys.cend(), [this, &done, &current_null_check_count, &total_number_null_checks, &status_increment_check, &my_time_ranges, &inner_table_primary_key_groups](std::pair<TimeRangeMapper_Strings, TimeRanges> const & map_info)
			for (TimeRangesForIndividualGroup_StringKeys::const_iterator it = group_time_ranges__stringkeys.cbegin(); it != group_time_ranges__stringkeys.cend(); ++it)
			{
				std::pair<TimeRangeMapper_Strings, TimeRanges> const & map_info = *it;

				if (failed || CheckCancelled())
				{
					break;
				}

				if (map_info.first == inner_table_primary_key_groups)
				{
					TimeRanges const & time_range_ = map_info.second;
					my_time_ranges.subtract(time_range_);

					if (my_time_ranges.empty())
					{
						break;
					}
				}

				if (current_null_check_count % status_increment_check == 0)
				{
					boost::format msg("Processing potential NULLs: %1% / %2%");
					msg % current_null_check_count % total_number_null_checks;
					this->messager.SetPerformanceLabel(msg.str());
				}

				++current_null_check_count;
				//});
			}


			if (failed || CheckCancelled())
			{
				return;
			}

			// ******************************************************************************************************* //
			// Whatever time ranges are left require rows over that time range, even though we have a NULL at the end.
			// ******************************************************************************************************* //

			std::for_each(my_time_ranges.ranges.cbegin(), my_time_ranges.ranges.cend(), [&saved_row_data_with_null_at_end, &outgoing_rows](std::pair<std::int64_t, std::int64_t> const & range)
			{
				outgoing_rows.push_back(saved_row_data_with_null_at_end);
				SavedRowData & new_null_row = outgoing_rows.back();
				new_null_row.datetime_start = range.first;
				new_null_row.datetime_end = range.second;
			});

		}

	});

	this->messager.SetPerformanceLabel(std::string());

	saved_rows_with_null_in_final_inner_table.swap(outgoing_rows);

}

void OutputModel::OutputGenerator::SavedRowData::ReturnAllNonNullPrimaryKeyGroups(std::set<std::vector<std::int64_t>> & inner_table_primary_key_groups) const
{
	if (indices_of_primary_key_columns_with_multiplicity_greater_than_1.empty())
	{
		return;
	}

	if (indices_of_primary_key_columns_with_multiplicity_greater_than_1[0].first != SQLExecutor::INT64)
	{
		return;
	}

	int column_index = 0;
	int inner_multiplicity_index = 0;
	std::vector<std::int64_t> inner_table_primary_key_group;
	std::for_each(indices_of_primary_key_columns_with_multiplicity_greater_than_1.cbegin(),
				  indices_of_primary_key_columns_with_multiplicity_greater_than_1.cend(), [this, &inner_table_primary_key_group, &inner_table_primary_key_groups, &column_index, &inner_multiplicity_index](
					  std::pair<SQLExecutor::WHICH_BINDING, std::pair<int, int>> const & binding)
	{

		int index_in_data_vector = binding.second.first;

		if (binding.first != SQLExecutor::NULL_BINDING)
		{
			inner_table_primary_key_group.push_back(current_parameter_ints[index_in_data_vector]);
		}

		++column_index;
		++inner_multiplicity_index;

		if (inner_multiplicity_index ==
			number_of_columns__in_a_single_inner_table__for_the_columns_only_having_the_dmu_category_with_multiplicity_greater_than_one__but_this_info_is_present_for_all_primary_key_columns)
		{
			if (!inner_table_primary_key_group.empty())
			{
				inner_table_primary_key_groups.insert(inner_table_primary_key_group);
				inner_table_primary_key_group.clear();
			}

			inner_multiplicity_index = 0;
		}

	});
}

void OutputModel::OutputGenerator::SavedRowData::ReturnAllNonNullPrimaryKeyGroups(std::set<std::vector<long double>> & inner_table_primary_key_groups) const
{
	if (indices_of_primary_key_columns_with_multiplicity_greater_than_1.empty())
	{
		return;
	}

	if (indices_of_primary_key_columns_with_multiplicity_greater_than_1[0].first != SQLExecutor::FLOAT)
	{
		return;
	}

	int column_index = 0;
	int inner_multiplicity_index = 0;
	std::vector<long double> inner_table_primary_key_group;
	std::for_each(indices_of_primary_key_columns_with_multiplicity_greater_than_1.cbegin(),
				  indices_of_primary_key_columns_with_multiplicity_greater_than_1.cend(), [this, &inner_table_primary_key_group, &inner_table_primary_key_groups, &column_index, &inner_multiplicity_index](
					  std::pair<SQLExecutor::WHICH_BINDING, std::pair<int, int>> const & binding)
	{

		int index_in_data_vector = binding.second.first;

		if (binding.first != SQLExecutor::NULL_BINDING)
		{
			inner_table_primary_key_group.push_back(current_parameter_floats[index_in_data_vector]);
		}

		++column_index;
		++inner_multiplicity_index;

		if (inner_multiplicity_index ==
			number_of_columns__in_a_single_inner_table__for_the_columns_only_having_the_dmu_category_with_multiplicity_greater_than_one__but_this_info_is_present_for_all_primary_key_columns)
		{
			if (!inner_table_primary_key_group.empty())
			{
				inner_table_primary_key_groups.insert(inner_table_primary_key_group);
				inner_table_primary_key_group.clear();
			}

			inner_multiplicity_index = 0;
		}

	});
}

void OutputModel::OutputGenerator::SavedRowData::ReturnAllNonNullPrimaryKeyGroups(std::set<std::vector<std::string>> & inner_table_primary_key_groups) const
{
	if (indices_of_primary_key_columns_with_multiplicity_greater_than_1.empty())
	{
		return;
	}

	if (indices_of_primary_key_columns_with_multiplicity_greater_than_1[0].first != SQLExecutor::STRING)
	{
		return;
	}

	int column_index = 0;
	int inner_multiplicity_index = 0;
	std::vector<std::string> inner_table_primary_key_group;
	std::for_each(indices_of_primary_key_columns_with_multiplicity_greater_than_1.cbegin(),
				  indices_of_primary_key_columns_with_multiplicity_greater_than_1.cend(), [this, &inner_table_primary_key_group, &inner_table_primary_key_groups, &column_index, &inner_multiplicity_index](
					  std::pair<SQLExecutor::WHICH_BINDING, std::pair<int, int>> const & binding)
	{

		int index_in_data_vector = binding.second.first;

		if (binding.first != SQLExecutor::NULL_BINDING)
		{
			inner_table_primary_key_group.push_back(current_parameter_strings[index_in_data_vector]);
		}

		++column_index;
		++inner_multiplicity_index;

		if (inner_multiplicity_index ==
			number_of_columns__in_a_single_inner_table__for_the_columns_only_having_the_dmu_category_with_multiplicity_greater_than_one__but_this_info_is_present_for_all_primary_key_columns)
		{
			if (!inner_table_primary_key_group.empty())
			{
				inner_table_primary_key_groups.insert(inner_table_primary_key_group);
				inner_table_primary_key_group.clear();
			}

			inner_multiplicity_index = 0;
		}

	});
}

void OutputModel::OutputGenerator::TimeRanges::append(std::int64_t const datetime_start, std::int64_t const datetime_end)
{

	if (ranges.empty())
	{
		ranges.push_back(std::make_pair(datetime_start, datetime_end));
		return;
	}

	std::list<std::pair<std::int64_t, std::int64_t>>::iterator me = ranges.begin();

	while (me != ranges.end())
	{

		std::pair<std::int64_t, std::int64_t> & range_ = *me;
		std::int64_t & _start = range_.first;
		std::int64_t & _end = range_.second;

		if (datetime_start < _start)
		{

			if (datetime_end < _start)
			{
				ranges.insert(ranges.begin(), std::make_pair(datetime_start, datetime_end));
				break;
			}

			if (datetime_end <= _end)
			{
				_start = datetime_start;
				break;
			}

			std::list<std::pair<std::int64_t, std::int64_t>>::iterator peek_ = me;
			++peek_;

			if (peek_ != ranges.end())
			{

				if (datetime_end >= peek_->first)
				{
					std::list<std::pair<std::int64_t, std::int64_t>>::iterator old_me = me;
					me = peek_;
					ranges.erase(old_me);
					me->first = datetime_start;
					continue;
				}

			}

			me->first = datetime_start;
			me->second = datetime_end;
			break;

		}

		else if (datetime_start < _end)
		{

			if (datetime_end <= _end)
			{
				// nothing to do
				break;
			}

			std::list<std::pair<std::int64_t, std::int64_t>>::iterator peek_ = me;
			++peek_;

			if (peek_ != ranges.end())
			{

				if (datetime_end >= peek_->first)
				{
					std::list<std::pair<std::int64_t, std::int64_t>>::iterator old_me = me;
					std::int64_t _old_start = _start;
					me = peek_;
					ranges.erase(old_me);
					me->first = _old_start;
					continue;
				}

			}

			me->second = datetime_end;
			break;

		}

		else if (datetime_start == _end)
		{

			std::list<std::pair<std::int64_t, std::int64_t>>::iterator peek_ = me;
			++peek_;

			if (peek_ != ranges.end())
			{

				if (datetime_end >= peek_->first)
				{
					std::list<std::pair<std::int64_t, std::int64_t>>::iterator old_me = me;
					std::int64_t _old_start = _start;
					me = peek_;
					ranges.erase(old_me);
					me->first = _old_start;
					continue;
				}

			}

			me->second = datetime_end;
			break;

		}

		else
		{

			// We start past the previous one

			std::list<std::pair<std::int64_t, std::int64_t>>::iterator peek_ = me;
			++peek_;

			if (peek_ != ranges.end())
			{
				me = peek_;
				continue;
			}

			// ... and there's none higher than us, so we're a brand-new one at the end
			ranges.insert(ranges.begin(), std::make_pair(datetime_start, datetime_end));
			break;

		}

	}

}

bool OutputModel::OutputGenerator::TimeRanges::empty()
{
	if (ranges.empty())
	{
		return true;
	}

	return false;
}

void OutputModel::OutputGenerator::TimeRanges::subtract(TimeRanges const & rhs)
{

	if (ranges.empty())
	{
		return;
	}

	if (rhs.ranges.empty())
	{
		return;
	}

	std::list<std::pair<std::int64_t, std::int64_t>>::iterator me = ranges.begin();
	std::list<std::pair<std::int64_t, std::int64_t>>::const_iterator them = rhs.ranges.cbegin();

	while (me != ranges.end() && them != rhs.ranges.cend())
	{
		if (me->first < them->first)
		{
			if (me->second <= them->first)
			{
				++me;
				continue;
			}
			else if (me->second <= them->second)
			{
				me->second = them->first;
				++me;
				continue;
			}
			else
			{
				std::int64_t _old_first = me->first;
				me->first = them->second;
				ranges.insert(ranges.begin(), std::make_pair(_old_first, them->first));
				++them;
				continue;
			}
		}
		else if (me->first < them->second)
		{
			if (me->second < them->second)
			{
				++me;
				ranges.erase(ranges.begin());
				continue;
			}
			else if (me->second == them->second)
			{
				++me;
				++them;
				ranges.erase(ranges.begin());
				continue;
			}
			else
			{
				me->first = them->second;
				++them;
				continue;
			}
		}
		else
		{
			++them;
			continue;
		}
	}

}

bool OutputModel::OutputGenerator::TimeRangeMapper_Ints::operator<(TimeRangeMapper_Ints const & rhs) const
{

	// ************************************************************ //
	// Called by the standard library map functions
	// ************************************************************ //

	int size_me = (int)sets.size();
	int size_you = (int)rhs.sets.size();
	int max_size = std::max(size_me, size_you);

	//std::set<std::vector<std::int64_t>> test_set = sets;
	test_set = sets;
	test_set.insert(rhs.sets.cbegin(), rhs.sets.cend());

	if (test_set.size() == max_size)
	{
		// equal!  Return false
		// (Even if the rhs set has less elements - that is the whole point of this class)
		return false;
	}

	return sets < rhs.sets;

}

bool OutputModel::OutputGenerator::TimeRangeMapper_Floats::operator<(TimeRangeMapper_Floats const & rhs) const
{

	// ************************************************************ //
	// Called by the standard library map functions
	// ************************************************************ //

	int size_me = (int)sets.size();
	int size_you = (int)rhs.sets.size();
	int max_size = std::max(size_me, size_you);

	//std::set<std::vector<long double>> test_set = sets;
	test_set = sets;
	test_set.insert(rhs.sets.cbegin(), rhs.sets.cend());

	if (test_set.size() == max_size)
	{
		// equal!  Return false
		// (Even if the rhs set has less elements - that is the whole point of this class)
		return false;
	}

	return sets < rhs.sets;

}

bool OutputModel::OutputGenerator::TimeRangeMapper_Strings::operator<(TimeRangeMapper_Strings const & rhs) const
{

	// ************************************************************ //
	// Called by the standard library map functions
	// ************************************************************ //

	int size_me = (int)sets.size();
	int size_you = (int)rhs.sets.size();
	int max_size = std::max(size_me, size_you);

	//std::set<std::vector<std::string>> test_set = sets;
	test_set = sets;
	test_set.insert(rhs.sets.cbegin(), rhs.sets.cend());

	if (test_set.size() == max_size)
	{
		// equal!  Return false
		// (Even if the rhs set has less elements - that is the whole point of this class)
		return false;
	}

	return sets < rhs.sets;

}

bool OutputModel::OutputGenerator::TimeRangeMapper_Ints::operator==(TimeRangeMapper_Ints const & rhs) const
{

	// ************************************************************ //
	// For internal calls only - not called by the standard library
	// ************************************************************ //

	//std::set<std::vector<std::int64_t>> test_set = sets;
	test_set = sets;
	test_set.insert(rhs.sets.cbegin(), rhs.sets.cend());

	if (sets.size() == test_set.size())
	{
		// equal!
		return true;
	}

	return sets == rhs.sets;

}

bool OutputModel::OutputGenerator::TimeRangeMapper_Floats::operator==(TimeRangeMapper_Floats const & rhs) const
{

	// ************************************************************ //
	// For internal calls only - not called by the standard library
	// ************************************************************ //

	//std::set<std::vector<long double>> test_set = sets;
	test_set = sets;
	test_set.insert(rhs.sets.cbegin(), rhs.sets.cend());

	if (sets.size() == test_set.size())
	{
		// equal!
		return true;
	}

	return sets == rhs.sets;

}

bool OutputModel::OutputGenerator::TimeRangeMapper_Strings::operator==(TimeRangeMapper_Strings const & rhs) const
{

	// ************************************************************ //
	// For internal calls only - not called by the standard library
	// ************************************************************ //

	//std::set<std::vector<std::string>> test_set = sets;
	test_set = sets;
	test_set.insert(rhs.sets.cbegin(), rhs.sets.cend());

	if (sets.size() == test_set.size())
	{
		// equal!
		return true;
	}

	return sets == rhs.sets;

}

bool OutputModel::OutputGenerator::TimeRangeMapper_Ints::operator==(std::set<std::vector<std::int64_t>> const & rhs) const
{

	// ************************************************************ //
	// For internal calls only - not called by the standard library
	// ************************************************************ //

	//std::set<std::vector<std::int64_t>> test_set = sets;
	test_set = sets;
	test_set.insert(rhs.cbegin(), rhs.cend());

	if (sets.size() == test_set.size())
	{
		// equal!
		return true;
	}

	return sets == rhs;

}

bool OutputModel::OutputGenerator::TimeRangeMapper_Floats::operator==(std::set<std::vector<long double>> const & rhs) const
{

	// ************************************************************ //
	// For internal calls only - not called by the standard library
	// ************************************************************ //

	//std::set<std::vector<long double>> test_set = sets;
	test_set = sets;
	test_set.insert(rhs.cbegin(), rhs.cend());

	if (sets.size() == test_set.size())
	{
		// equal!
		return true;
	}

	return sets == rhs;

}

bool OutputModel::OutputGenerator::TimeRangeMapper_Strings::operator==(std::set<std::vector<std::string>> const & rhs) const
{

	// ************************************************************ //
	// For internal calls only - not called by the standard library
	// ************************************************************ //

	//std::set<std::vector<std::string>> test_set = sets;
	test_set = sets;
	test_set.insert(rhs.cbegin(), rhs.cend());

	if (sets.size() == test_set.size())
	{
		// equal!
		return true;
	}

	return sets == rhs;

}

OutputModel::OutputGenerator::TimeRangeMapper_Ints::TimeRangeMapper_Ints(std::set<std::vector<std::int64_t>> const & rhs)
	: sets(rhs)
{

}

OutputModel::OutputGenerator::TimeRangeMapper_Floats::TimeRangeMapper_Floats(std::set<std::vector<long double>> const & rhs)
	: sets(rhs)
{

}

OutputModel::OutputGenerator::TimeRangeMapper_Strings::TimeRangeMapper_Strings(std::set<std::vector<std::string>> const & rhs)
	: sets(rhs)
{

}

OutputModel::OutputGenerator::TimeRangeMapper_Ints::TimeRangeMapper_Ints(TimeRangeMapper_Ints const & rhs)
	: sets(rhs.sets)
{

}

OutputModel::OutputGenerator::TimeRangeMapper_Floats::TimeRangeMapper_Floats(TimeRangeMapper_Floats const & rhs)
	: sets(rhs.sets)
{

}

OutputModel::OutputGenerator::TimeRangeMapper_Strings::TimeRangeMapper_Strings(TimeRangeMapper_Strings const & rhs)
	: sets(rhs.sets)
{

}

void OutputModel::OutputGenerator::SavedRowData::SetLast2DateTimeColumns(std::int64_t const start_datetime_to_set, std::int64_t const end_datetime_to_set)
{

	int number_columns = (int)current_parameter_which_binding_to_use.size();

	if (current_parameter_which_binding_to_use[number_columns - 2] == SQLExecutor::NULL_BINDING)
	{
		// The swapping of inner tables may have swapped nulls into the timerange columns

		int number_columns_final_inner_table = (int)indices_of_all_columns_in_final_inner_table.size();

		current_parameter_which_binding_to_use[number_columns - 2] = SQLExecutor::INT64;
		current_parameter_ints.push_back(start_datetime_to_set);
		indices_of_all_columns[number_columns - 2].first = SQLExecutor::INT64;
		indices_of_all_columns[number_columns - 2].second.first = (int)current_parameter_ints.size() - 1;
		indices_of_all_columns[number_columns - 2].second.second = number_columns - 2;
		indices_of_all_columns_in_final_inner_table[number_columns_final_inner_table - 2].first = SQLExecutor::INT64;
		indices_of_all_columns_in_final_inner_table[number_columns_final_inner_table - 2].second.first = (int)current_parameter_ints.size() - 1;
		indices_of_all_columns_in_final_inner_table[number_columns_final_inner_table - 2].second.second = number_columns - 2;
		datetime_start = start_datetime_to_set;

		current_parameter_which_binding_to_use[number_columns - 1] = SQLExecutor::INT64;
		current_parameter_ints.push_back(end_datetime_to_set);
		indices_of_all_columns[number_columns - 1].first = SQLExecutor::INT64;
		indices_of_all_columns[number_columns - 1].second.first = (int)current_parameter_ints.size() - 1;
		indices_of_all_columns[number_columns - 1].second.second = number_columns - 1;
		indices_of_all_columns_in_final_inner_table[number_columns_final_inner_table - 1].first = SQLExecutor::INT64;
		indices_of_all_columns_in_final_inner_table[number_columns_final_inner_table - 1].second.first = (int)current_parameter_ints.size() - 1;
		indices_of_all_columns_in_final_inner_table[number_columns_final_inner_table - 1].second.second = number_columns - 1;
		datetime_end = end_datetime_to_set;
	}
	else
	{
		int size_of_int_vector = (int)GetSavedRowData().current_parameter_ints.size();
		GetSavedRowData().current_parameter_ints[size_of_int_vector - 2] = start_datetime_to_set;
		GetSavedRowData().current_parameter_ints[size_of_int_vector - 1] = end_datetime_to_set;
	}

}

void OutputModel::OutputGenerator::FindDatetimeIndices(ColumnsInTempView const & columns, int & previous_datetime_start_column_index, int & previous_datetime_end_column_index,
		int & current_datetime_start_column_index, int & current_datetime_end_column_index, XR_TABLE_CATEGORY const xr_table_category)
{

	previous_datetime_start_column_index = -1;
	previous_datetime_end_column_index = -1;
	current_datetime_start_column_index = -1;
	current_datetime_end_column_index = -1;
	int column_index = (int)columns.columns_in_view.size() - 1;
	std::for_each(columns.columns_in_view.crbegin(),
				  columns.columns_in_view.crend(), [&columns, &xr_table_category, &previous_datetime_start_column_index, &previous_datetime_end_column_index, &current_datetime_start_column_index, &current_datetime_end_column_index, &column_index](
					  ColumnsInTempView::ColumnInTempView const & schema_column)
	{

		// The previous values are always located after the current values, but in arbitrary order,
		// so this check suffices to be certain that all 4 values have been obtained
		if (previous_datetime_start_column_index != -1 && previous_datetime_end_column_index != -1)
		{
			--column_index;
			return;
		}

		// The following works even if the previous and the current datetime column types are the same,
		// because it first sets the current, and only then the previous
		bool set_ = false;

		if (!set_ && schema_column.column_type == columns.current_block_datetime_column_types.first)
		{
			if (current_datetime_start_column_index == -1)
			{
				current_datetime_start_column_index = column_index;
				set_ = true;
			}
		}

		if (!set_ && schema_column.column_type == columns.current_block_datetime_column_types.second)
		{
			if (current_datetime_end_column_index == -1)
			{
				current_datetime_end_column_index = column_index;
				set_ = true;
			}
		}

		if (!set_ && schema_column.column_type == columns.previous_block_datetime_column_types.first)
		{
			if (previous_datetime_start_column_index == -1)
			{
				previous_datetime_start_column_index = column_index;
				set_ = true;
			}
		}

		if (!set_ && schema_column.column_type == columns.previous_block_datetime_column_types.second)
		{
			if (previous_datetime_end_column_index == -1)
			{
				previous_datetime_end_column_index = column_index;
				set_ = true;
			}
		}

		if (false)
		{
			switch (xr_table_category)
			{
				case OutputModel::OutputGenerator::PRIMARY_VARIABLE_GROUP:
					{
						//if (schema_column.column_type == ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__DATETIMESTART__PRIMARY_VG_INNER_TABLE_MERGE__AFTER_DUPLICATES_REMOVED)
						if (schema_column.column_type == columns.previous_block_datetime_column_types.first)
						{
							//if (current_datetime_start_column_index == -1)
							//{
							//	current_datetime_start_column_index = column_index;
							//} else
							if (previous_datetime_start_column_index == -1)
							{
								previous_datetime_start_column_index = column_index;
							}
						}
						//else if (schema_column.column_type == ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__DATETIMEEND__PRIMARY_VG_INNER_TABLE_MERGE__AFTER_DUPLICATES_REMOVED)
						else if (schema_column.column_type == columns.previous_block_datetime_column_types.second)
						{
							//if (current_datetime_end_column_index == -1)
							//{
							//	current_datetime_end_column_index = column_index;
							//} else
							if (previous_datetime_end_column_index == -1)
							{
								previous_datetime_end_column_index = column_index;
							}
						}
						//else if (schema_column.column_type == ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__DATETIMESTART || schema_column.column_type == ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__DATETIMESTART_INTERNAL)
						else if (schema_column.column_type == columns.current_block_datetime_column_types.first)
						{
							if (current_datetime_start_column_index == -1)
							{
								current_datetime_start_column_index = column_index;
							}
						}
						//else if (schema_column.column_type == ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__DATETIMEEND || schema_column.column_type == ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__DATETIMEEND_INTERNAL)
						else if (schema_column.column_type == columns.current_block_datetime_column_types.second)
						{
							if (current_datetime_end_column_index == -1)
							{
								current_datetime_end_column_index = column_index;
							}
						}
					}
					break;

				case OutputModel::OutputGenerator::CHILD_VARIABLE_GROUP:
					{
						if (schema_column.column_type == columns.previous_block_datetime_column_types.first)
						{
							if (previous_datetime_start_column_index == -1)
							{
								previous_datetime_start_column_index = column_index;
							}
						}
						else if (schema_column.column_type == columns.previous_block_datetime_column_types.second)
						{
							if (previous_datetime_end_column_index == -1)
							{
								previous_datetime_end_column_index = column_index;
							}
						}
						else if (schema_column.column_type == columns.current_block_datetime_column_types.first)
						{
							if (current_datetime_start_column_index == -1)
							{
								current_datetime_start_column_index = column_index;
							}
						}
						else if (schema_column.column_type == columns.current_block_datetime_column_types.second)
						{
							if (current_datetime_end_column_index == -1)
							{
								current_datetime_end_column_index = column_index;
							}
						}

						//if (    schema_column.column_type == ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__DATETIMESTART__POST_TIMERANGE_MERGED_BETWEEN_TOP_LEVEL_PRIMARY_VARIABLE_GROUPS
						//	||  schema_column.column_type == ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__DATETIMESTART__PRIMARY_VG_INNER_TABLE_MERGE__AFTER_DUPLICATES_REMOVED
						//	||  schema_column.column_type == ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__DATETIMESTART_PRE_MERGED_KAD_OUTPUT)
						//{
						//	if (previous_datetime_start_column_index == -1)
						//	{
						//		previous_datetime_start_column_index = column_index;
						//	}
						//}
						//else if (schema_column.column_type == ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__DATETIMEEND__POST_TIMERANGE_MERGED_BETWEEN_TOP_LEVEL_PRIMARY_VARIABLE_GROUPS
						//	||   schema_column.column_type == ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__DATETIMEEND__PRIMARY_VG_INNER_TABLE_MERGE__AFTER_DUPLICATES_REMOVED
						//	||   schema_column.column_type == ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__DATETIMEEND_PRE_MERGED_KAD_OUTPUT)
						//{
						//	if ( previous_datetime_end_column_index == -1)
						//	{
						//		previous_datetime_end_column_index = column_index;
						//	}
						//}
						//else if (schema_column.column_type == ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__DATETIMESTART
						//	||   schema_column.column_type == ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__DATETIMESTART_INTERNAL)
						//{
						//	if ( current_datetime_start_column_index == -1)
						//	{
						//		current_datetime_start_column_index = column_index;
						//	}
						//}
						//else if (schema_column.column_type == ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__DATETIMEEND
						//	||   schema_column.column_type == ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__DATETIMEEND_INTERNAL)
						//{
						//	if ( current_datetime_end_column_index == -1)
						//	{
						//		current_datetime_end_column_index = column_index;
						//	}
						//}

					}
					break;

				case OutputModel::OutputGenerator::FINAL_MERGE_OF_PRIMARY_VARIABLE_GROUP:
					{
						if (schema_column.column_type == ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__DATETIMESTART__POST_TIMERANGE_MERGED_BETWEEN_TOP_LEVEL_PRIMARY_VARIABLE_GROUPS)
						{
							if (previous_datetime_start_column_index == -1)
							{
								previous_datetime_start_column_index = column_index;
							}
						}
						else if (schema_column.column_type == ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__DATETIMEEND__POST_TIMERANGE_MERGED_BETWEEN_TOP_LEVEL_PRIMARY_VARIABLE_GROUPS)
						{
							if (previous_datetime_end_column_index == -1)
							{
								previous_datetime_end_column_index = column_index;
							}
						}
						else if (schema_column.column_type == ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__DATETIMESTART__PRIMARY_VG_INNER_TABLE_MERGE__AFTER_DUPLICATES_REMOVED)
						{
							if (current_datetime_start_column_index == -1)
							{
								current_datetime_start_column_index = column_index;
							}
						}
						else if (schema_column.column_type == ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__DATETIMEEND__PRIMARY_VG_INNER_TABLE_MERGE__AFTER_DUPLICATES_REMOVED)
						{
							if (current_datetime_end_column_index == -1)
							{
								current_datetime_end_column_index = column_index;
							}
						}
					}
					break;

				default:
					break;
			}
		}

		--column_index;

	});

}

bool OutputModel::OutputGenerator::CheckForIdenticalData(ColumnsInTempView const & columns, SavedRowData const & previous_row, SavedRowData const & current_row)
{

	bool anything_is_different = false;
	int column_index = 0;

	std::for_each(columns.columns_in_view.cbegin(), columns.columns_in_view.cend(), [&column_index, &anything_is_different, &previous_row, &current_row](
					  ColumnsInTempView::ColumnInTempView const & column)
	{

		if (anything_is_different)
		{
			return;
		}

		if (column.column_type == ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__PRIMARY
			|| column.column_type == ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__SECONDARY)
		{

			std::pair<SQLExecutor::WHICH_BINDING, std::pair<int, int>> const & previous_binding = previous_row.indices_of_all_columns[column_index];
			std::pair<SQLExecutor::WHICH_BINDING, std::pair<int, int>> const & current_binding = current_row.indices_of_all_columns[column_index];

			switch (current_binding.first)
			{

				case SQLExecutor::INT64:
					{

						std::int64_t const & current_int = current_row.current_parameter_ints[current_binding.second.first];

						switch (previous_binding.first)
						{

							case SQLExecutor::INT64:
								{

									std::int64_t const & previous_int = previous_row.current_parameter_ints[previous_binding.second.first];

									if (previous_int != current_int)
									{
										anything_is_different = true;
									}

								}
								break;

							case SQLExecutor::FLOAT:
								{

									long double const & previous_float = previous_row.current_parameter_floats[previous_binding.second.first];

									if (previous_float != boost::lexical_cast<long double>(current_int))
									{
										anything_is_different = true;
									}

								}
								break;

							case SQLExecutor::STRING:
								{

									std::string const & previous_string = previous_row.current_parameter_strings[previous_binding.second.first];

									if (boost::lexical_cast<std::int64_t>(previous_string) != current_int)
									{
										anything_is_different = true;
									}

								}
								break;

							case SQLExecutor::NULL_BINDING:
								{

									// one row has data and the other doesn't
									anything_is_different = true;

								}
								break;

							default:
								break;

						}


					}
					break;

				case SQLExecutor::FLOAT:
					{

						long double const & current_float = current_row.current_parameter_floats[current_binding.second.first];

						switch (previous_binding.first)
						{

							case SQLExecutor::INT64:
								{

									std::int64_t const & previous_int = previous_row.current_parameter_ints[previous_binding.second.first];

									if (boost::lexical_cast<long double>(previous_int) != current_float)
									{
										anything_is_different = true;
									}

								}
								break;

							case SQLExecutor::FLOAT:
								{

									long double const & previous_float = previous_row.current_parameter_floats[previous_binding.second.first];

									if (previous_float != current_float)
									{
										anything_is_different = true;
									}

								}
								break;

							case SQLExecutor::STRING:
								{

									std::string const & previous_string = previous_row.current_parameter_strings[previous_binding.second.first];

									if (boost::lexical_cast<long double>(previous_string) != current_float)
									{
										anything_is_different = true;
									}

								}
								break;

							case SQLExecutor::NULL_BINDING:
								{

									// one row has data and the other doesn't
									anything_is_different = true;

								}
								break;

							default:
								break;

						}


					}
					break;

				case SQLExecutor::STRING:
					{

						std::string const & current_string = current_row.current_parameter_strings[current_binding.second.first];

						switch (previous_binding.first)
						{

							case SQLExecutor::INT64:
								{

									std::int64_t const & previous_int = previous_row.current_parameter_ints[previous_binding.second.first];

									if (previous_int != boost::lexical_cast<std::int64_t>(current_string))
									{
										anything_is_different = true;
									}

								}
								break;

							case SQLExecutor::FLOAT:
								{

									long double const & previous_float = previous_row.current_parameter_floats[previous_binding.second.first];

									if (previous_float != boost::lexical_cast<long double>(current_string))
									{
										anything_is_different = true;
									}

								}
								break;

							case SQLExecutor::STRING:
								{

									std::string const & previous_string = previous_row.current_parameter_strings[previous_binding.second.first];

									if (previous_string != current_string)
									{
										anything_is_different = true;
									}

								}
								break;

							case SQLExecutor::NULL_BINDING:
								{

									// one row has data and the other doesn't
									anything_is_different = true;

								}
								break;

							default:
								break;

						}

					}
					break;

				case SQLExecutor::NULL_BINDING:
					{

						switch (previous_binding.first)
						{

							case SQLExecutor::INT64:
								{

									// one row has data and the other doesn't
									anything_is_different = true;

								}
								break;

							case SQLExecutor::FLOAT:
								{

									// one row has data and the other doesn't
									anything_is_different = true;

								}
								break;

							case SQLExecutor::STRING:
								{

									// one row has data and the other doesn't
									anything_is_different = true;

								}
								break;

							case SQLExecutor::NULL_BINDING:
								{

									// neither row has data here - they are the same on this column

								}
								break;

							default:
								break;

						}

					}
					break;

				default:
					break;

			}

		}

		++column_index;

	});

	return !anything_is_different;

}

void OutputModel::OutputGenerator::RandomSampling_ReadData_AddToTimeSlices(ColumnsInTempView const & variable_group_selected_columns_schema, int const variable_group_number,
	AllWeightings & allWeightings, VARIABLE_GROUP_MERGE_MODE const merge_mode, std::vector<std::string> & errorMessages,
	std::vector<ChildToPrimaryMapping> mappings_from_child_branch_to_primary,
	std::vector<ChildToPrimaryMapping> mappings_from_child_leaf_to_primary)
{

	{

		std::int64_t current_rows_stepped = 0;
		sqlite3_stmt *& the_prepared_stmt = SQLExecutor::stmt_insert;
		std::shared_ptr<bool> statement_is_prepared(std::make_shared<bool>(false));
		SQLExecutor::stmt_insert = nullptr;
		sqlite3_stmt *& the_stmt__ = SQLExecutor::stmt_insert;
		BOOST_SCOPE_EXIT(&the_prepared_stmt, &statement_is_prepared, &the_stmt__)
		{
			if (the_prepared_stmt && *statement_is_prepared)
			{
				sqlite3_finalize(the_prepared_stmt);
				++SQLExecutor::number_statement_finalizes;
				the_prepared_stmt = nullptr;
				*statement_is_prepared = false;
			}

			the_stmt__ = nullptr;
		} BOOST_SCOPE_EXIT_END

		{

			BOOST_SCOPE_EXIT(this_)
			{
				this_->CloseObtainData();
			} BOOST_SCOPE_EXIT_END

			ObtainData(variable_group_selected_columns_schema, true);

			if (failed || CheckCancelled())
			{
				return;
			}

			//BeginNewTransaction();

			SavedRowData sorting_row_of_data;

			while (StepData())
			{

				if (CheckCancelled())
				{
					return;
				}

				sorting_row_of_data.PopulateFromCurrentRowInDatabase(variable_group_selected_columns_schema, stmt_result, XR_TABLE_CATEGORY::RANDOMIZE, true);

				// Construct branch and leaf

				failed = sorting_row_of_data.failed;

				if (failed)
				{
					SetFailureMessage(sorting_row_of_data.error_message);
					return;
				}

				// Construct Leaf
				DMUInstanceDataVector dmus_leaf;
				bool bad = false;
				std::for_each(sorting_row_of_data.indices_of_primary_key_columns_with_multiplicity_greater_than_1.cbegin(),
					sorting_row_of_data.indices_of_primary_key_columns_with_multiplicity_greater_than_1.cend(), [&](std::pair<SQLExecutor::WHICH_BINDING, std::pair<int, int>> const & binding_info)
				{

					if (bad)
					{
						return;
					}

					SQLExecutor::WHICH_BINDING binding = binding_info.first;
					std::pair<int, int> const & indices = binding_info.second;
					int const index_in_bound_vector = indices.first;
					int const column_number = indices.second;

					switch (binding)
					{

						case SQLExecutor::INT64:
							{
								dmus_leaf.push_back(sorting_row_of_data.current_parameter_ints[index_in_bound_vector]);
							}
							break;

						case SQLExecutor::FLOAT:
							{
								dmus_leaf.push_back(static_cast<double>(sorting_row_of_data.current_parameter_floats[index_in_bound_vector]));
							}
							break;

						case SQLExecutor::STRING:
							{
								dmus_leaf.push_back(sorting_row_of_data.current_parameter_strings[index_in_bound_vector]);
							}
							break;

						default:
							{
								// No throw... TODO: log error
								//boost::format msg("Data cannot be NULL in a primary key field.");
								//throw NewGeneException() << newgene_error_description(msg.str());
								bad = true;
								return;
							}
							break;

					}

				});

				// Construct Branch
				DMUInstanceDataVector dmus_branch;
				std::for_each(sorting_row_of_data.indices_of_primary_key_columns_with_multiplicity_equal_to_1.cbegin(),
					sorting_row_of_data.indices_of_primary_key_columns_with_multiplicity_equal_to_1.cend(), [&](std::pair<SQLExecutor::WHICH_BINDING, std::pair<int, int>> const & binding_info)
				{

					if (bad)
					{
						return;
					}

					SQLExecutor::WHICH_BINDING binding = binding_info.first;
					std::pair<int, int> const & indices = binding_info.second;
					int const index_in_bound_vector = indices.first;
					int const column_number = indices.second;

					switch (binding)
					{

						case SQLExecutor::INT64:
							{
								dmus_branch.push_back(sorting_row_of_data.current_parameter_ints[index_in_bound_vector]);
							}
							break;

						case SQLExecutor::FLOAT:
							{
								dmus_branch.push_back(static_cast<double>(sorting_row_of_data.current_parameter_floats[index_in_bound_vector]));
							}
							break;

						case SQLExecutor::STRING:
							{
								dmus_branch.push_back(sorting_row_of_data.current_parameter_strings[index_in_bound_vector]);
							}
							break;

						default:
							{
								// No throw... TODO: log error
								//boost::format msg("Data cannot be NULL in a primary key field.");
								//throw NewGeneException() << newgene_error_description(msg.str());
								bad = true;
								return;
							}
							break;

					}

				});

				// Store secondary key data for use in constructing final output.
				// Note that this is stored in a cache.
				// In the future, this cache can be enhanced to become an intelligent LIFO memory/disk cache
				// to support huge input datasets.
				DMUInstanceDataVector secondary_data;
				std::for_each(sorting_row_of_data.indices_of_secondary_key_columns.cbegin(),
							  sorting_row_of_data.indices_of_secondary_key_columns.cend(), [&](std::pair<SQLExecutor::WHICH_BINDING, std::pair<int, int>> const & binding_info)
				{

					if (bad)
					{
						return;
					}

					SQLExecutor::WHICH_BINDING binding = binding_info.first;
					std::pair<int, int> const & indices = binding_info.second;
					int const index_in_bound_vector = indices.first;
					int const column_number = indices.second;

					switch (binding)
					{

						case SQLExecutor::INT64:
							{
								secondary_data.push_back(sorting_row_of_data.current_parameter_ints[index_in_bound_vector]);
							}
							break;

						case SQLExecutor::FLOAT:
							{
								secondary_data.push_back(static_cast<double>(sorting_row_of_data.current_parameter_floats[index_in_bound_vector]));
							}
							break;

						case SQLExecutor::STRING:
							{
								secondary_data.push_back(sorting_row_of_data.current_parameter_strings[index_in_bound_vector]);
							}
							break;

						default:
							{
								// No throw... TODO: log error
								//boost::format msg("Data cannot be NULL in a primary key field.");
								//throw NewGeneException() << newgene_error_description(msg.str());
								bad = true;
								return;
							}
							break;

					}

				});

				if (!bad)
				{

					try
					{
						
						switch (merge_mode)
						{

							case VARIABLE_GROUP_MERGE_MODE__PRIMARY:
							{

								Leaf leaf(dmus_leaf, sorting_row_of_data.rowid);
								Branch branch(dmus_branch);

								bool added = allWeightings.HandleBranchAndLeaf(branch, std::make_pair(TimeSlice(sorting_row_of_data.datetime_start, sorting_row_of_data.datetime_end), leaf), variable_group_number, merge_mode);

								if (added)
								{
									// Add the secondary data for this primary variable group to the cache
									allWeightings.dataCache[sorting_row_of_data.rowid] = secondary_data;
								}

							}
							break;

							case VARIABLE_GROUP_MERGE_MODE__TOP_LEVEL:
							{

								Leaf leaf(dmus_leaf);
								Branch branch(dmus_branch);

								// Set the secondary data index into the above cache for this non-primary top-level variable group
								// so that it can be set in the corresponding leaf already present for the branch
								leaf.other_top_level_indices_into_raw_data[variable_group_number] = sorting_row_of_data.rowid;

								// Add the secondary data for this non-primary top-level variable group to the cache
								allWeightings.otherTopLevelCache[variable_group_number][sorting_row_of_data.rowid] = secondary_data;

								bool added = allWeightings.HandleBranchAndLeaf(branch, std::make_pair(TimeSlice(sorting_row_of_data.datetime_start, sorting_row_of_data.datetime_end), leaf), variable_group_number, merge_mode);

							}
							break;

							case VARIABLE_GROUP_MERGE_MODE__CHILD:
							{

								// pack the child data index into the main leaf for use in the function called below - because this leaf is TEMPORARY
								// (this data will be unpacked from the temporary leaf and put into the proper place in the function called below)
								Leaf leaf(dmus_leaf, sorting_row_of_data.rowid);
								Branch branch(dmus_branch);

								// ************************************************************************************************** //
								// Important!
								// This is a *CHILD* merge,
								// so all LEAVES in the primary variable group have already been added to all branches.
								// No new leaves will be added to any branches here.
								// Therefore, even though 'HandleBranchAndLeaf()' is called,
								// which might slice the time slices, each such slice will not add any new primary leaves
								// and the previous set of cached leaves will be persisted in the time slice copies.
								// ************************************************************************************************** //
								bool added = allWeightings.HandleBranchAndLeaf(branch, std::make_pair(TimeSlice(sorting_row_of_data.datetime_start, sorting_row_of_data.datetime_end), leaf), variable_group_number, merge_mode, mappings_from_child_branch_to_primary, mappings_from_child_leaf_to_primary);

								if (added)
								{
									// Add the secondary data for this child variable group to the cache
									allWeightings.childCache[variable_group_number][sorting_row_of_data.rowid] = secondary_data;
								}

							}
							break;

							default:
							{

							}
							break;

						}

					}
					catch (boost::exception & e)
					{
						bad = true;
						if (std::string const * error_desc = boost::get_error_info<newgene_error_description>(e))
						{
							boost::format msg("Error: %1%");
							errorMessages.push_back(error_desc->c_str());
						}
					}

				}

				++current_rows_stepped;

				if (current_rows_stepped % 100 == 0 || current_rows_stepped == current_number_rows_to_sort)
				{
					//UpdateProgressBarValue(messager, current_rows_stepped);
				}

				if (bad)
				{
					continue;
				}

			}

			//messager.UpdateProgressBarValue(1000);
			//boost::format msg("Processed %1% of %2% temporary rows this stage: performing transaction");
			//msg % current_rows_stepped % current_number_rows_to_sort;
			//messager.SetPerformanceLabel(msg.str());

		}

	}

}

OutputModel::OutputGenerator::SqlAndColumnSet OutputModel::OutputGenerator::RandomSamplingBuildSchema(std::vector<ColumnsInTempView> const & primary_variable_groups_raw_data_columns, std::vector<ColumnsInTempView> const & secondary_variable_groups_column_info)
{


	// **************************************************************************************** //
	// Initialize schema
	// **************************************************************************************** //

	SqlAndColumnSet result = std::make_pair(std::vector<SQLExecutor>(), ColumnsInTempView());
	std::vector<SQLExecutor> & sql_strings = result.first;
	ColumnsInTempView & result_columns = result.second;

	result_columns.most_recent_sql_statement_executed__index = -1;

	std::string view_name = "NGTEMP_RANDOM_SAMPLING";
	result_columns.view_name_no_uuid = view_name;

	view_name += "_";
	view_name += newUUID(true);

	result_columns.view_name = view_name;
	result_columns.view_number = 1;
	result_columns.has_no_datetime_columns = false;


	ColumnsInTempView const & primary_variable_group_raw_data_columns = primary_variable_groups_raw_data_columns[top_level_vg_index];

	// First, calculate some indices.
	int highest_multiplicity = 1;
	std::for_each(primary_variable_group_raw_data_columns.columns_in_view.cbegin(),
				  primary_variable_group_raw_data_columns.columns_in_view.cend(), [&](ColumnsInTempView::ColumnInTempView const & raw_data_table_column)
	{

		if (raw_data_table_column.total_outer_multiplicity__in_total_kad__for_current_dmu_category__for_current_variable_group > highest_multiplicity)
		{
			highest_multiplicity = raw_data_table_column.total_outer_multiplicity__in_total_kad__for_current_dmu_category__for_current_variable_group;
		}

	});

	K = highest_multiplicity;


	// **************************************************************************************** //
	// Start with the primary key columns of multiplicity 1.
	// **************************************************************************************** //

	std::for_each(primary_variable_group_raw_data_columns.columns_in_view.cbegin(),
		primary_variable_group_raw_data_columns.columns_in_view.cend(), [&](
		ColumnsInTempView::ColumnInTempView const & raw_data_table_column)
	{

		if (raw_data_table_column.column_type == ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__DATETIMESTART
			|| raw_data_table_column.column_type == ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__DATETIMESTART_INTERNAL
			|| raw_data_table_column.column_type == ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__DATETIMEEND
			|| raw_data_table_column.column_type == ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__DATETIMEEND_INTERNAL)
		{
			return; // Add these columns last, if selected by user
		}

		if (raw_data_table_column.column_type == ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__SECONDARY)
		{
			return; // Enforce that primary key columns appear first.
		}

		if (raw_data_table_column.column_type == ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__PRIMARY)
		{

			if (raw_data_table_column.total_outer_multiplicity__in_total_kad__for_current_dmu_category__for_current_variable_group > 1)
			{
				return; // Add the multiplicity = 1 primary keys first
			}

			result_columns.columns_in_view.push_back(raw_data_table_column);
			ColumnsInTempView::ColumnInTempView & new_column = result_columns.columns_in_view.back();
			new_column.column_name_in_temporary_table = new_column.column_name_in_temporary_table_no_uuid;
			new_column.column_name_in_temporary_table += "_";
			new_column.column_name_in_temporary_table += newUUID(true);
			new_column.is_within_inner_table_corresponding_to_top_level_uoa = true;
			new_column.current_multiplicity__of__current_inner_table__within__current_vg_inner_table_set = 1;
			new_column.current_multiplicity__corresponding_to__current_inner_table___is_1_in_all_inner_tables_when_multiplicity_is_1_for_that_dmu_category_for_that_vg = 1;

		}

	});

	// **************************************************************************************** //
	// Proceed with the primary key columns of multiplicity greater than 1.
	// **************************************************************************************** //

	for (int current_multiplicity = 1; current_multiplicity <= highest_multiplicity; ++current_multiplicity)
	{

		std::for_each(primary_variable_group_raw_data_columns.columns_in_view.cbegin(),
			primary_variable_group_raw_data_columns.columns_in_view.cend(), [&](
			ColumnsInTempView::ColumnInTempView const & raw_data_table_column)
		{
			if (raw_data_table_column.column_type == ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__DATETIMESTART
				|| raw_data_table_column.column_type == ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__DATETIMESTART_INTERNAL
				|| raw_data_table_column.column_type == ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__DATETIMEEND
				|| raw_data_table_column.column_type == ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__DATETIMEEND_INTERNAL)
			{
				return; // Add these columns last, if selected by user
			}

			if (raw_data_table_column.column_type == ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__SECONDARY)
			{
				return; // Enforce that primary key columns appear first.
			}

			if (raw_data_table_column.column_type == ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__PRIMARY)
			{

				if (raw_data_table_column.total_outer_multiplicity__in_total_kad__for_current_dmu_category__for_current_variable_group == 1)
				{
					return; // Add the multiplicity = 1 primary keys first
				}

				result_columns.columns_in_view.push_back(raw_data_table_column);
				ColumnsInTempView::ColumnInTempView & new_column = result_columns.columns_in_view.back();
				new_column.column_name_in_temporary_table = new_column.column_name_in_temporary_table_no_uuid;
				new_column.column_name_in_temporary_table += "_";
				new_column.column_name_in_temporary_table += newUUID(true);
				new_column.is_within_inner_table_corresponding_to_top_level_uoa = true;
				new_column.current_multiplicity__of__current_inner_table__within__current_vg_inner_table_set = current_multiplicity;
				new_column.current_multiplicity__corresponding_to__current_inner_table___is_1_in_all_inner_tables_when_multiplicity_is_1_for_that_dmu_category_for_that_vg = current_multiplicity;
				new_column.primary_key_index_within_total_kad_for_dmu_category =
					new_column.primary_key_index__within_uoa_corresponding_to_variable_group_corresponding_to_current_inner_table__for_dmu_category
					+ (current_multiplicity - 1) * new_column.total_k_count__within_uoa_corresponding_to_current_variable_group__for_current_dmu_category;

			}

		});

	}




	// **************************************************************************************** //
	// For the primary top-level variable group, construct secondary data columns
	// **************************************************************************************** //

	{

		ColumnsInTempView const & primary_variable_group_raw_data_columns = primary_variable_groups_raw_data_columns[top_level_vg_index];

		WidgetInstanceIdentifiers const & variables_selected =
			(*the_map)[*primary_variable_group_raw_data_columns.variable_groups[0].identifier_parent][primary_variable_group_raw_data_columns.variable_groups[0]];

		// Proceed to the secondary key columns.
		for (int current_multiplicity = 1; current_multiplicity <= highest_multiplicity; ++current_multiplicity)
		{

			std::for_each(primary_variable_group_raw_data_columns.columns_in_view.cbegin(),
				primary_variable_group_raw_data_columns.columns_in_view.cend(), [&](ColumnsInTempView::ColumnInTempView const & raw_data_table_column)
			{

				bool make_secondary_datetime_column = false;

				if (raw_data_table_column.column_type == ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__DATETIMESTART
					|| raw_data_table_column.column_type == ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__DATETIMEEND)
				{
					// Do not return!  If the user selects these columns, they should appear as regular secondary key columns.
					make_secondary_datetime_column = true;
				}

				if (!make_secondary_datetime_column && raw_data_table_column.column_type != ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__SECONDARY)
				{
					return; // We are populating secondary columns now, so exit if this isn't one
				}

				bool match = false;
				std::for_each(variables_selected.cbegin(), variables_selected.cend(), [&](WidgetInstanceIdentifier const & variable_selected)
				{
					if (boost::iequals(raw_data_table_column.column_name_in_original_data_table, *variable_selected.code))
					{
						match = true;
					}
				});

				if (match)
				{
					result_columns.columns_in_view.push_back(raw_data_table_column);
					ColumnsInTempView::ColumnInTempView & new_column = result_columns.columns_in_view.back();
					new_column.column_name_in_temporary_table = new_column.column_name_in_temporary_table_no_uuid;
					new_column.column_name_in_temporary_table += "_";
					new_column.column_name_in_temporary_table += newUUID(true);
					new_column.is_within_inner_table_corresponding_to_top_level_uoa = true;
					new_column.current_multiplicity__of__current_inner_table__within__current_vg_inner_table_set = current_multiplicity;
					new_column.current_multiplicity__corresponding_to__current_inner_table___is_1_in_all_inner_tables_when_multiplicity_is_1_for_that_dmu_category_for_that_vg = current_multiplicity;
					new_column.total_outer_multiplicity__in_total_kad__for_current_dmu_category__for_current_variable_group = highest_multiplicity;

					if (make_secondary_datetime_column)
					{
						new_column.column_type = ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__SECONDARY;
						new_column.originally_datetime = true;
					}

				}

			});

		}

	}



	// **************************************************************************************** //
	// For the non-primary top-level variable groups, construct secondary data columns
	// **************************************************************************************** //

	int primary_group_number = 0;
	std::for_each(primary_variable_groups_raw_data_columns.cbegin(), primary_variable_groups_raw_data_columns.cend(), [&](ColumnsInTempView const & primary_variable_group_raw_data_columns)
	{

		if (primary_group_number == top_level_vg_index)
		{
			return; // already handled - the primary top-level variable group's secondary data always goes first
		}

		WidgetInstanceIdentifiers const & variables_selected =
			(*the_map)[*primary_variable_group_raw_data_columns.variable_groups[0].identifier_parent][primary_variable_group_raw_data_columns.variable_groups[0]];

		// Proceed to the secondary key columns.
		for (int current_multiplicity = 1; current_multiplicity <= highest_multiplicity; ++current_multiplicity)
		{

			std::for_each(primary_variable_group_raw_data_columns.columns_in_view.cbegin(),
				primary_variable_group_raw_data_columns.columns_in_view.cend(), [&](ColumnsInTempView::ColumnInTempView const & raw_data_table_column)
			{

				bool make_secondary_datetime_column = false;

				if (raw_data_table_column.column_type == ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__DATETIMESTART
					|| raw_data_table_column.column_type == ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__DATETIMEEND)
				{
					// Do not return!  If the user selects these columns, they should appear as regular secondary key columns.
					make_secondary_datetime_column = true;
				}

				if (!make_secondary_datetime_column && raw_data_table_column.column_type != ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__SECONDARY)
				{
					return; // We are populating secondary columns now, so exit if this isn't one
				}

				bool match = false;
				std::for_each(variables_selected.cbegin(), variables_selected.cend(), [&](WidgetInstanceIdentifier const & variable_selected)
				{
					if (boost::iequals(raw_data_table_column.column_name_in_original_data_table, *variable_selected.code))
					{
						match = true;
					}
				});

				if (match)
				{
					result_columns.columns_in_view.push_back(raw_data_table_column);
					ColumnsInTempView::ColumnInTempView & new_column = result_columns.columns_in_view.back();
					new_column.column_name_in_temporary_table = new_column.column_name_in_temporary_table_no_uuid;
					new_column.column_name_in_temporary_table += "_";
					new_column.column_name_in_temporary_table += newUUID(true);
					new_column.is_within_inner_table_corresponding_to_top_level_uoa = true;
					new_column.current_multiplicity__of__current_inner_table__within__current_vg_inner_table_set = current_multiplicity;
					new_column.current_multiplicity__corresponding_to__current_inner_table___is_1_in_all_inner_tables_when_multiplicity_is_1_for_that_dmu_category_for_that_vg = current_multiplicity;
					new_column.total_outer_multiplicity__in_total_kad__for_current_dmu_category__for_current_variable_group = highest_multiplicity;

					if (make_secondary_datetime_column)
					{
						new_column.column_type = ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__SECONDARY;
						new_column.originally_datetime = true;
					}

				}

			});

		}

		++primary_group_number;

	});



	// **************************************************************************************** //
	// Child variable groups - secondary columns
	// **************************************************************************************** //

	std::for_each(secondary_variable_groups_column_info.cbegin(),
		secondary_variable_groups_column_info.cend(), [&](ColumnsInTempView const & child_variable_group_raw_data_columns)
	{

		if (failed || CheckCancelled())
		{
			return;
		}

		int const the_child_multiplicity = child_uoas__which_multiplicity_is_greater_than_1[*(child_variable_group_raw_data_columns.variable_groups[0].identifier_parent)].second;

		for (int current_multiplicity = 1; current_multiplicity <= the_child_multiplicity; ++current_multiplicity)
		{

			WidgetInstanceIdentifiers const & variables_selected =
				(*the_map)[*child_variable_group_raw_data_columns.variable_groups[0].identifier_parent][child_variable_group_raw_data_columns.variable_groups[0]];

			std::for_each(child_variable_group_raw_data_columns.columns_in_view.cbegin(),
				child_variable_group_raw_data_columns.columns_in_view.cend(), [&](ColumnsInTempView::ColumnInTempView const & new_column_secondary)
			{
				bool make_secondary_datetime_column = false;

				if (new_column_secondary.column_type == ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__DATETIMESTART
					|| new_column_secondary.column_type == ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__DATETIMEEND)
				{
					// Do not return!  If the user selects these columns, they should appear as regular secondary key columns.
					make_secondary_datetime_column = true;
				}

				if (!make_secondary_datetime_column && new_column_secondary.column_type != ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__SECONDARY)
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
					new_column.is_within_inner_table_corresponding_to_top_level_uoa = false;
					new_column.current_multiplicity__of__current_inner_table__within__current_vg_inner_table_set = current_multiplicity;
					new_column.total_outer_multiplicity__in_total_kad__for_current_dmu_category__for_current_variable_group = the_child_multiplicity;

					if (make_secondary_datetime_column)
					{
						new_column.column_type = ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__SECONDARY;
						new_column.originally_datetime = true;
					}

				}
			});

			if (failed || CheckCancelled())
			{
				return;
			}

		}

	});


	// **************************************************************************************** //
	// Finalize schema
	// ... by adding the "time slice" time range columns
	// **************************************************************************************** //

	std::string datetime_start_col_name;
	std::string datetime_end_col_name;
	std::string datetime_start_col_name_text;
	std::string datetime_end_col_name_text;

	std::string datetime_start_col_name_no_uuid = "DATETIMESTART__TIME_SLICE";
	datetime_start_col_name = datetime_start_col_name_no_uuid;
	datetime_start_col_name += "_";
	datetime_start_col_name += newUUID(true);

	result_columns.columns_in_view.push_back(ColumnsInTempView::ColumnInTempView());
	ColumnsInTempView::ColumnInTempView & datetime_start_column = result_columns.columns_in_view.back();
	datetime_start_column.column_name_in_temporary_table = datetime_start_col_name;
	datetime_start_column.column_name_in_temporary_table_no_uuid = datetime_start_col_name_no_uuid;
	datetime_start_column.current_multiplicity__of__current_inner_table__within__current_vg_inner_table_set = highest_multiplicity;
	datetime_start_column.number_inner_tables_in_set = highest_multiplicity;
	datetime_start_column.column_type = ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__DATETIMESTART__TIME_SLICE;
	datetime_start_column.variable_group_associated_with_current_inner_table = WidgetInstanceIdentifier();
	datetime_start_column.uoa_associated_with_variable_group_associated_with_current_inner_table = WidgetInstanceIdentifier();
	datetime_start_column.is_within_inner_table_corresponding_to_top_level_uoa = true;
	datetime_start_column.column_name_in_original_data_table = "";

	std::string datetime_end_col_name_no_uuid = "DATETIMEEND__TIME_SLICE";
	datetime_end_col_name = datetime_end_col_name_no_uuid;
	datetime_end_col_name += "_";
	datetime_end_col_name += newUUID(true);

	result_columns.columns_in_view.push_back(ColumnsInTempView::ColumnInTempView());
	ColumnsInTempView::ColumnInTempView & datetime_end_column = result_columns.columns_in_view.back();
	datetime_end_column.column_name_in_temporary_table = datetime_end_col_name;
	datetime_end_column.column_name_in_temporary_table_no_uuid = datetime_end_col_name_no_uuid;
	datetime_end_column.current_multiplicity__of__current_inner_table__within__current_vg_inner_table_set = highest_multiplicity;
	datetime_end_column.number_inner_tables_in_set = highest_multiplicity;
	datetime_end_column.column_type = ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__DATETIMESTART__TIME_SLICE;
	datetime_end_column.variable_group_associated_with_current_inner_table = WidgetInstanceIdentifier();
	datetime_end_column.uoa_associated_with_variable_group_associated_with_current_inner_table = WidgetInstanceIdentifier();
	datetime_end_column.is_within_inner_table_corresponding_to_top_level_uoa = true;
	datetime_end_column.column_name_in_original_data_table = "";

	result_columns.current_block_datetime_column_types = std::make_pair(
		ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__DATETIMESTART__TIME_SLICE,
		ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__DATETIMESTART__TIME_SLICE);
	result_columns.previous_block_datetime_column_types = result_columns.current_block_datetime_column_types;

	ExecuteSQL(result); // Executes all SQL queries up to the current one

	if (failed || CheckCancelled())
	{
		return result;
	}

	return result;

}

void OutputModel::OutputGenerator::RandomSamplingCreateOutputTable()
{

	std::vector<SQLExecutor> & sql_strings = random_sampling_schema.first;
	ColumnsInTempView & random_sampling_columns = random_sampling_schema.second;
	random_sampling_columns.most_recent_sql_statement_executed__index = -1;

	std::string sql_create_empty_table;
	sql_create_empty_table += "CREATE TABLE \"";
	sql_create_empty_table += random_sampling_columns.view_name;
	sql_create_empty_table += "\"";
	sql_create_empty_table += " (";

	bool first = true;
	std::for_each(random_sampling_columns.columns_in_view.cbegin(),
				  random_sampling_columns.columns_in_view.cend(), [&](
					  ColumnsInTempView::ColumnInTempView const & random_sampling_column)
	{

		if (!first)
		{
			sql_create_empty_table += ", ";
		}
		first = false;

		sql_create_empty_table += random_sampling_column.column_name_in_temporary_table;

	});

	sql_create_empty_table += " )";

	sql_strings.push_back(SQLExecutor(this, db, sql_create_empty_table));
	ExecuteSQL(random_sampling_schema);

	if (failed)
	{
		SetFailureMessage(sql_error);
		return;
	}

	if (CheckCancelled())
	{
		return;
	}

}

void OutputModel::OutputGenerator::RandomSamplingWriteToOutputTable(AllWeightings & allWeightings, std::vector<std::string> & errorMessages)
{

	ColumnsInTempView const & random_sampling_columns = random_sampling_schema.second;

	int const    minimum_desired_rows_per_transaction = 1024 * 16;
	std::int64_t current_rows_in_error = 0;
	std::int64_t current_rows_stepped = 0;

	BeginNewTransaction();

	BOOST_SCOPE_EXIT(this_, &allWeightings)
	{
		// Transaction is for efficiency;
		// rollback not yet enabled
		if (allWeightings.insert_random_sample_stmt != nullptr)
		{
			sqlite3_finalize(allWeightings.insert_random_sample_stmt);
			allWeightings.insert_random_sample_stmt = nullptr;
		}
		this_->EndTransaction();
	} BOOST_SCOPE_EXIT_END

	PrepareInsertStatement(allWeightings.insert_random_sample_stmt, random_sampling_columns);

	std::for_each(allWeightings.timeSlices.begin(), allWeightings.timeSlices.end(), [&](std::pair<TimeSlice const, VariableGroupTimeSliceData> & timeSliceData)
	{

		if (failed || CheckCancelled())
		{
			return;
		}

		TimeSlice const & timeSlice = timeSliceData.first;
		VariableGroupTimeSliceData const & variableGroupTimeSliceData = timeSliceData.second;

		VariableGroupBranchesAndLeavesVector const & variableGroupBranchesAndLeavesVector = variableGroupTimeSliceData.branches_and_leaves;

		// For now, assume only one variable group
		if (variableGroupBranchesAndLeavesVector.size() > 1)
		{
			boost::format msg("Only one top-level variable group is currently supported for the random and full sampler in ConsolidateHits().");
			throw NewGeneException() << newgene_error_description(msg.str());
		}

		VariableGroupBranchesAndLeaves const & variableGroupBranchesAndLeaves = variableGroupBranchesAndLeavesVector[0];
		BranchesAndLeaves const & branchesAndLeaves = variableGroupBranchesAndLeaves.branches_and_leaves;

		std::for_each(branchesAndLeaves.cbegin(), branchesAndLeaves.cend(), [&](std::pair<Branch const, Leaves> const & branchAndLeaves)
		{

			if (failed || CheckCancelled())
			{
				return;
			}

			Branch const & branch = branchAndLeaves.first;
			Leaves const & leaves = branchAndLeaves.second;

			std::for_each(branch.hits.cbegin(), branch.hits.cend(), [&](std::pair<boost::multiprecision::cpp_int const, std::set<BranchOutputRow>> const & time_unit_and_rows)
			{

				if (failed || CheckCancelled())
				{
					return;
				}

				std::set<BranchOutputRow> const & outputRows = time_unit_and_rows.second;

				std::for_each(outputRows.cbegin(), outputRows.cend(), [&](BranchOutputRow const & outputRow)
				{










					// ******************************** //

					// INCOMPLETE LOGIC!!!!
					// Merge with RandomSamplingWriteResultsToFileOrScreen()!

					// ******************************** //






























					// We have a row to output

					if (failed || CheckCancelled())
					{
						return;
					}

					int bindIndex = 1;

					// The branch represents the primary keys of multiplicity 1
					std::for_each(branch.primary_keys.cbegin(), branch.primary_keys.cend(), [&](DMUInstanceData const & data)
					{
						BindTermToInsertStatement(allWeightings.insert_random_sample_stmt, data, bindIndex++);
					});

					std::vector<std::int64_t> secondary_key_row_indices;
					std::map<int, std::vector<std::int64_t>> other_top_level_secondary_row_indices;

					// The leaves represent the primary keys of multiplicity > 1
					std::for_each(outputRow.primary_leaves_cache.cbegin(), outputRow.primary_leaves_cache.cend(), [&](int const & leafIndex)
					{
						Leaf & leaf = branch.leaves_cache[leafIndex];
						std::for_each(leaf.primary_keys.cbegin(), leaf.primary_keys.cend(), [&](DMUInstanceData const & data)
						{
							BindTermToInsertStatement(allWeightings.insert_random_sample_stmt, data, bindIndex++);
						});
					});

					// This is the data for the primary top-level variable group
					// secondary keys (i.e., for the dependent data)
					std::for_each(outputRow.primary_leaves_cache.cbegin(), outputRow.primary_leaves_cache.cend(), [&](int const & leafIndex)
					{
						Leaf & leaf = branch.leaves_cache[leafIndex];
						if (leaf.index_into_raw_data > 0)
						{
							SecondaryInstanceDataVector const & secondary_data_vector = allWeightings.dataCache[leaf.index_into_raw_data];
							std::for_each(secondary_data_vector.cbegin(), secondary_data_vector.cend(), [&](SecondaryInstanceData const & data)
							{
								BindTermToInsertStatement(allWeightings.insert_random_sample_stmt, data, bindIndex++);
							});
						}
						else
						{
							// no data available.  Import should always place blanks, so this should never happen.
							boost::format msg("Logic error: Missing primary variable group data (there isn't even blank data).");
							throw NewGeneException() << newgene_error_description(msg.str());
						}
					});

					// Then, the non-primary top-level variable group secondary data.
					// This info is stored in the leaf also.
					//
					// Show secondary data grouped by variable group,
					// then by multiplicity.
					// There might be multiple fields for each variable group and within each multiplicity,
					// ... so the logic is a bit non-trivial to get the display order right.
					int numberTopLevelGroups = static_cast<int>(primary_variable_groups_vector.size());
					for (int vgNumber = 0; vgNumber < numberTopLevelGroups; ++vgNumber)
					{
						for (int multiplicity = 0; multiplicity < K; ++multiplicity)
						{
							bool matched = false;

							// Find the variable group and leaf that is desired
							int testMultiplicity = 0;
							std::for_each(outputRow.primary_leaves_cache.cbegin(), outputRow.primary_leaves_cache.cend(), [&](int const & leafIndex)
							{
								if (testMultiplicity != multiplicity)
								{
									++testMultiplicity;
									return;
								}
								Leaf & leaf = branch.leaves_cache[leafIndex];
								std::for_each(leaf.other_top_level_indices_into_raw_data.cbegin(), leaf.other_top_level_indices_into_raw_data.cend(), [&](std::pair<int const, std::int64_t> const & top_level_vg_and_data_index)
								{
									int const vg_number = top_level_vg_and_data_index.first;
									if (vg_number == vgNumber)
									{

										// We now have both the multiplicity and the variable group that is desired
										// for the correct sequence of output data
										matched = true;

										// *********************************************************************** //
										// If we have ANY secondary data, we have ALL ROWS
										// because the schema controls this and the data is pulled from the DB,
										// which has empty columns rather than missing columns
										// *********************************************************************** //
										std::int64_t const & data_index = top_level_vg_and_data_index.second;
										DataCache & data_cache = allWeightings.otherTopLevelCache[vg_number];
										SecondaryInstanceDataVector const & secondary_data_vector = data_cache[data_index];
										std::for_each(secondary_data_vector.cbegin(), secondary_data_vector.cend(), [&](SecondaryInstanceData const & data)
										{
											BindTermToInsertStatement(allWeightings.insert_random_sample_stmt, data, bindIndex++);
										});

									}
								});
								++testMultiplicity;
							});

							if (!matched)
							{
								if (vgNumber != top_level_vg_index)
								{
									// Missing a variable group.  Fill with blanks.
									int numberSecondaries = top_level_number_secondary_columns[vgNumber];
									for (int n = 0; n < numberSecondaries; ++n)
									{
										BindTermToInsertStatement(allWeightings.insert_random_sample_stmt, InstanceData(std::string()), bindIndex++);
									}
								}
							}
						}
					}

					// Then, the child variable group secondary data.
					// This info is stored in the output row itself.
					//
					// Show secondary data grouped by variable group,
					// then by multiplicity.
					// There might be multiple fields for each variable group and within each multiplicity,
					// ... so the logic is a bit non-trivial to get the display order right.
					int numberChildGroups = static_cast<int>(secondary_variable_groups_vector.size());
					for (int vgNumber = 0; vgNumber < numberChildGroups; ++vgNumber)
					{
						int const the_child_multiplicity = child_uoas__which_multiplicity_is_greater_than_1[*(secondary_variable_groups_vector[vgNumber].first.identifier_parent)].second;
						for (int multiplicity = 0; multiplicity < the_child_multiplicity; ++multiplicity)
						{
							bool matched = false;
							std::for_each(outputRow.child_indices_into_raw_data.cbegin(), outputRow.child_indices_into_raw_data.cend(), [&](std::pair<int const, std::map<int, std::int64_t>> const & leaf_index_mappings)
							{
								int const vg_number = leaf_index_mappings.first;
								if (vg_number != vgNumber)
								{
									return;
								}
								std::map<int, std::int64_t> const & leaf_number_to_data_index = leaf_index_mappings.second;
								std::for_each(leaf_number_to_data_index.cbegin(), leaf_number_to_data_index.cend(), [&](std::pair<int const, std::int64_t> const & leaf_index_mapping)
								{

									int const leaf_number = leaf_index_mapping.first;
									if (leaf_number != multiplicity)
									{
										return;
									}

									if (leaf_number < 0)
									{
										return;
									}

									// This is the desired variable group and multiplicity
									matched = true;

									std::int64_t const & data_index = leaf_index_mapping.second;
									DataCache & data_cache = allWeightings.childCache[vg_number];
									SecondaryInstanceDataVector & secondary_data_vector = data_cache[data_index];
									std::for_each(secondary_data_vector.cbegin(), secondary_data_vector.cend(), [&](SecondaryInstanceData const & data)
									{
										BindTermToInsertStatement(allWeightings.insert_random_sample_stmt, data, bindIndex++);
									});

								});
							});
							if (!matched)
							{
								int numberSecondaries = child_number_secondary_columns[vgNumber];
								for (int n = 0; n < numberSecondaries; ++n)
								{
									BindTermToInsertStatement(allWeightings.insert_random_sample_stmt, InstanceData(std::string()), bindIndex++);
								}
							}
						}
					}

					int step_result = 0;

					// Execute the insert of the row into the table
					if ((step_result = sqlite3_step(allWeightings.insert_random_sample_stmt)) != SQLITE_DONE)
					{
						std::string sql_error = sqlite3_errmsg(db);
						boost::format msg("Unable to execute prepared insert query to insert a new random sample: %1%");
						msg % sql_error;
						errorMessages.push_back(msg.str());
						++current_rows_in_error;
					}

					// Prepare statement for next row
					sqlite3_clear_bindings(allWeightings.insert_random_sample_stmt);
					sqlite3_reset(allWeightings.insert_random_sample_stmt);

					++current_rows_stepped;
					if (current_rows_stepped % minimum_desired_rows_per_transaction == 0)
					{
						EndTransaction();
						BeginNewTransaction();
					}

				});

			});

		});

	});

	EndTransaction();

}

void OutputModel::OutputGenerator::PrepareInsertStatement(sqlite3_stmt *& insert_random_sample_stmt, ColumnsInTempView const & random_sampling_columns)
{

	if (insert_random_sample_stmt == nullptr)
	{
		std::string insert_random_sample_string;

		insert_random_sample_string += "INSERT INTO \"";
		insert_random_sample_string += random_sampling_columns.view_name;
		insert_random_sample_string += "\" (";

		bool first = true;
		std::for_each(random_sampling_columns.columns_in_view.cbegin(),
			random_sampling_columns.columns_in_view.cend(), [&](
			ColumnsInTempView::ColumnInTempView const & random_sampling_column)
		{

			if (!first)
			{
				insert_random_sample_string += ", ";
			}
			first = false;

			insert_random_sample_string += random_sampling_column.column_name_in_temporary_table;

		});

		insert_random_sample_string += ") VALUES (";

		first = true;
		for (size_t n = 0; n < random_sampling_columns.columns_in_view.size(); ++n)
		{

			if (!first)
			{
				insert_random_sample_string += ", ";
			}
			first = false;

			insert_random_sample_string += "?";

		}

		insert_random_sample_string += ")";

		sqlite3_prepare_v2(db, insert_random_sample_string.c_str(), static_cast<int>(insert_random_sample_string.size()) + 1, &insert_random_sample_stmt, NULL);

		if (insert_random_sample_stmt == NULL)
		{
			std::string sql_error = sqlite3_errmsg(db);
			boost::format msg("Unable to prepare SQL query to insert a random K-ad row: %1% (%2%)");
			msg % sql_error.c_str() % insert_random_sample_string.c_str();
			throw NewGeneException() << newgene_error_description(msg.str());
		}

	}

}

void OutputModel::OutputGenerator::BindTermToInsertStatement(sqlite3_stmt * insert_random_sample_stmt, InstanceData const & data, int bindIndex)
{
	bind_visitor visitor(insert_random_sample_stmt, bindIndex);
	boost::apply_visitor(visitor, data);
}

void OutputModel::OutputGenerator::RandomSamplerFillDataForChildGroups(AllWeightings & allWeightings)
{
	
	// **************************************************************************************** //
	// Top-level variable groups that are *not* primary are considered child data
	// **************************************************************************************** //

	int current_top_level_vg_index = 0;
	std::for_each(primary_variable_groups_column_info.cbegin(), primary_variable_groups_column_info.cend(), [&](ColumnsInTempView const & primary_variable_group_raw_data_columns)
	{

		if (failed || CheckCancelled()) return;

		if (current_top_level_vg_index == top_level_vg_index)
		{
			// Skip the primary top-level variable group;
			// we are only populating columns of secondary data
			// for NON-primary top-level variable groups,
			// which are for the purposes of this function
			// considered to be child variable groups
			return;
		}

		SqlAndColumnSet selected_raw_data_table_schema = CreateTableOfSelectedVariablesFromRawData(primary_variable_group_raw_data_columns, current_top_level_vg_index);
		if (failed || CheckCancelled()) return;
		selected_raw_data_table_schema.second.most_recent_sql_statement_executed__index = -1;
		ExecuteSQL(selected_raw_data_table_schema);
		merging_of_children_column_sets.push_back(selected_raw_data_table_schema);

		std::vector<std::string> errorMessages;
		RandomSampling_ReadData_AddToTimeSlices(selected_raw_data_table_schema.second, current_top_level_vg_index, allWeightings, VARIABLE_GROUP_MERGE_MODE__TOP_LEVEL, errorMessages);
		if (failed || CheckCancelled()) return;

		++current_top_level_vg_index;

	});


	// **************************************************************************************** //
	// Child variable groups - secondary columns
	// **************************************************************************************** //

	int current_child_vg_index = 0;
	std::for_each(secondary_variable_groups_column_info.cbegin(), secondary_variable_groups_column_info.cend(), [&](ColumnsInTempView const & child_variable_group_raw_data_columns)
	{

		if (failed || CheckCancelled()) return;

		SqlAndColumnSet selected_raw_data_table_schema = CreateTableOfSelectedVariablesFromRawData(child_variable_group_raw_data_columns, current_child_vg_index);
		if (failed || CheckCancelled()) return;
		selected_raw_data_table_schema.second.most_recent_sql_statement_executed__index = -1;
		ExecuteSQL(selected_raw_data_table_schema);
		merging_of_children_column_sets.push_back(selected_raw_data_table_schema);

		int const the_child_multiplicity = child_uoas__which_multiplicity_is_greater_than_1[*(child_variable_group_raw_data_columns.variable_groups[0].identifier_parent)].second;


		// **************************************************************************************** //
		// Populate the index mappings that map each child group's branch DMU data,
		// and leaf DMU data, to a corresponding specific index in the
		// top-level variable group's branch or a specific index in one of its specific leaves.
		// **************************************************************************************** //

		std::vector<ChildToPrimaryMapping> mappings_from_child_branch_to_primary;
		std::vector<ChildToPrimaryMapping> mappings_from_child_leaf_to_primary;

		int current_primary_branch_index = 0;
		int current_primary_internal_leaf_index = 0;
		int current_primary_leaf_number = 0;
		std::for_each(primary_variable_groups_column_info[0].columns_in_view.cbegin(), primary_variable_groups_column_info[0].columns_in_view.cend(), [&](decltype(primary_variable_groups_column_info[0].columns_in_view)::value_type const & primary_variable_group_set_member)
		{

			std::for_each(sequence.primary_key_sequence_info.cbegin(), sequence.primary_key_sequence_info.cend(), [&](PrimaryKeySequence::PrimaryKeySequenceEntry const & full_kad_key_info)
			{

				bool is_current_index_a_top_level_primary_group_branch = false;
				if (full_kad_key_info.total_outer_multiplicity__for_the_current_dmu_category__corresponding_to_the_uoa_corresponding_to_top_level_variable_group == 1)
				{
					is_current_index_a_top_level_primary_group_branch = true;
				}

				std::for_each(full_kad_key_info.variable_group_info_for_primary_keys.cbegin(),
					full_kad_key_info.variable_group_info_for_primary_keys.cend(), [&](PrimaryKeySequence::VariableGroup_PrimaryKey_Info const & full_kad_key_info_primary_not_child_variable_group)
				{

					// The primary key information we seek here is for the TOP LEVEL variable group
					if (full_kad_key_info_primary_not_child_variable_group.vg_identifier.IsEqual(WidgetInstanceIdentifier::EQUALITY_CHECK_TYPE__STRING_CODE, primary_variable_groups_column_info[0].variable_groups[0]))
					{

						if (!full_kad_key_info_primary_not_child_variable_group.column_name.empty())
						{

							if (boost::iequals(full_kad_key_info_primary_not_child_variable_group.table_column_name, primary_variable_group_set_member.column_name_in_original_data_table))
							{

								// Valid TOP-LEVEL primary key in sequence of TOP-LEVEL primary keys

								std::for_each(full_kad_key_info.variable_group_info_for_primary_keys.cbegin(),
									full_kad_key_info.variable_group_info_for_primary_keys.cend(), [&](PrimaryKeySequence::VariableGroup_PrimaryKey_Info const & full_kad_key_info_child_variable_group)
								{

									// The primary key information we seek is that for our own, CHILD, variable group
									if (full_kad_key_info_child_variable_group.vg_identifier.IsEqual(WidgetInstanceIdentifier::EQUALITY_CHECK_TYPE__STRING_CODE, child_variable_group_raw_data_columns.variable_groups[0]))
									{

										if (!full_kad_key_info_child_variable_group.column_name.empty())
										{

											if (full_kad_key_info_child_variable_group.total_outer_multiplicity__in_total_kad__for_current_dmu_category__for_current_variable_group == 0)
											{
												// There are no columns in this DMU for our current child variable group
												return;
											}

											// This primary key is guaranteed to be a primary key in our child variable group,
											// with columns present,
											// AND it appears in the proper order of the top-level primary keys

											bool is_child_group_branch = false;
											if (full_kad_key_info_child_variable_group.total_outer_multiplicity__in_total_kad__for_current_dmu_category__for_current_variable_group == 1)
											{
												is_child_group_branch = true;
											}

											if (is_current_index_a_top_level_primary_group_branch && is_child_group_branch)
											{
												mappings_from_child_branch_to_primary.push_back(ChildToPrimaryMapping(CHILD_TO_PRIMARY_MAPPING__MAPS_TO_BRANCH, current_primary_branch_index));
											}
											else
											if (is_current_index_a_top_level_primary_group_branch && !is_child_group_branch)
											{
												mappings_from_child_leaf_to_primary.push_back(ChildToPrimaryMapping(CHILD_TO_PRIMARY_MAPPING__MAPS_TO_BRANCH, current_primary_branch_index));
											}
											else
											if (!is_current_index_a_top_level_primary_group_branch && is_child_group_branch)
											{
												mappings_from_child_branch_to_primary.push_back(ChildToPrimaryMapping(CHILD_TO_PRIMARY_MAPPING__MAPS_TO_LEAF, current_primary_internal_leaf_index, current_primary_leaf_number));
											}
											else if (!is_current_index_a_top_level_primary_group_branch && !is_child_group_branch)
											{
												mappings_from_child_leaf_to_primary.push_back(ChildToPrimaryMapping(CHILD_TO_PRIMARY_MAPPING__MAPS_TO_LEAF, current_primary_internal_leaf_index, current_primary_leaf_number));
											}
											else
											{
												boost::format msg("No mapping possible from child to primary DMU!");
												throw NewGeneException() << newgene_error_description(msg.str());
											}

										}

									}

								});

								if (is_current_index_a_top_level_primary_group_branch)
								{
									++current_primary_branch_index;
								}
								else
								{
									++current_primary_internal_leaf_index;
									if (full_kad_key_info.sequence_number_within_dmu_category_primary_uoa + 1 == full_kad_key_info.total_k_count_within_high_level_variable_group_uoa_for_this_dmu_category)
									{
										++current_primary_leaf_number;
										current_primary_internal_leaf_index = 0;
									}
								}

							}

						}

					}

				});

			});


		});

		if (failed || CheckCancelled()) return;

		// **************************************************************************************** //
		// We here loop through child variable group raw data
		// (previously stored in a temporary table that includes only primary keys and selected variables)
		// and pass INDIVIDUAL child leaves to the time slice data handling functions
		// (we do not pass multiple leaves, even though multiple child leaves might appear
		//  in each individual row of output),
		// and internally this SINGLE child variable group leaf (with its branch)
		// will be used to locate ALL rows of actual output that contain that individual child leaf,
		// regardless of the child leaf number in that particular output row.
		// **************************************************************************************** //

		std::vector<std::string> errorMessages;
		RandomSampling_ReadData_AddToTimeSlices(selected_raw_data_table_schema.second, current_child_vg_index, allWeightings, VARIABLE_GROUP_MERGE_MODE__CHILD, errorMessages, mappings_from_child_branch_to_primary, mappings_from_child_leaf_to_primary);

		++current_child_vg_index;

	});

}

void OutputModel::OutputGenerator::RandomSamplingWriteResultsToFileOrScreen(AllWeightings & allWeightings)
{

	std::string setting_path_to_kad_output = CheckOutputFileExists();

	if (failed || CheckCancelled())
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

	// Write columns headers
	int column_index = 0;
	bool first = true;
	std::for_each(final_result.second.columns_in_view.begin(),
		final_result.second.columns_in_view.end(), [this, &output_file, &first, &column_index](ColumnsInTempView::ColumnInTempView & unformatted_column)
	{

		++column_index;

		if (column_index >= final_result.second.columns_in_view.size() - 1)
		{
			return; // for now, do not output datetime columns
		}

		if (!first)
		{
			output_file << ",";
		}
		first = false;

		output_file << unformatted_column.column_name_in_original_data_table;
		if (unformatted_column.total_outer_multiplicity__in_total_kad__for_current_dmu_category__for_current_variable_group > 1)
		{
			output_file << "_";
			output_file << boost::lexical_cast<std::string>(unformatted_column.current_multiplicity__of__current_inner_table__within__current_vg_inner_table_set);
		}

	});
	output_file << std::endl;


	std::int64_t rows_written = 0;

	std::for_each(allWeightings.timeSlices.begin(), allWeightings.timeSlices.end(), [&](std::pair<TimeSlice const, VariableGroupTimeSliceData> & timeSliceData)
	{

		if (failed || CheckCancelled())
		{
			return;
		}

		TimeSlice const & timeSlice = timeSliceData.first;
		VariableGroupTimeSliceData const & variableGroupTimeSliceData = timeSliceData.second;

		VariableGroupBranchesAndLeavesVector const & variableGroupBranchesAndLeavesVector = variableGroupTimeSliceData.branches_and_leaves;

		// For now, assume only one variable group
		if (variableGroupBranchesAndLeavesVector.size() > 1)
		{
			boost::format msg("Only one top-level variable group is currently supported for the random and full sampler in ConsolidateHits().");
			throw NewGeneException() << newgene_error_description(msg.str());
		}

		VariableGroupBranchesAndLeaves const & variableGroupBranchesAndLeaves = variableGroupBranchesAndLeavesVector[0];
		BranchesAndLeaves const & branchesAndLeaves = variableGroupBranchesAndLeaves.branches_and_leaves;

		std::for_each(branchesAndLeaves.cbegin(), branchesAndLeaves.cend(), [&](std::pair<Branch const, Leaves> const & branchAndLeaves)
		{

			if (failed || CheckCancelled())
			{
				return;
			}

			Branch const & branch = branchAndLeaves.first;
			Leaves const & leaves = branchAndLeaves.second;

			std::for_each(branch.hits.cbegin(), branch.hits.cend(), [&](std::pair<boost::multiprecision::cpp_int const, std::set<BranchOutputRow>> const & time_unit_and_rows)
			{

				if (failed || CheckCancelled())
				{
					return;
				}

				std::set<BranchOutputRow> const & outputRows = time_unit_and_rows.second;

				std::for_each(outputRows.cbegin(), outputRows.cend(), [&](BranchOutputRow const & outputRow)
				{

					// We have a row to output

					if (failed || CheckCancelled())
					{
						return;
					}

					if (false) // set to true when debugging
					{
						std::string thebranch;
						SpitBranch(thebranch, branch);

						std::string theleaves;
						theleaves += "<LEAVES>";
						std::for_each(outputRow.primary_leaves_cache.cbegin(), outputRow.primary_leaves_cache.cend(), [&](int const & leafIndex)
						{
							theleaves += "<LEAF_PLUS_INDEX>";
							theleaves += "<LEAF_INDEX>";
							theleaves += boost::lexical_cast<std::string>(leafIndex);
							theleaves += "</LEAF_INDEX>";
							Leaf & leaf = branch.leaves_cache[leafIndex];
							SpitLeaf(theleaves, leaf);
							theleaves += "</LEAF_PLUS_INDEX>";
						});
						theleaves += "</LEAVES>";

						std::string therow;
						SpitOutputRow(therow, outputRow);

						std::vector<std::string> sdataall;
						SpitAllWeightings(sdataall, allWeightings, true);
						int m = 0;
					}

					first = true;

					// First, the branch primary keys
					std::for_each(branch.primary_keys.cbegin(), branch.primary_keys.cend(), [&](DMUInstanceData const & data)
					{
						boost::apply_visitor(write_to_output_visitor(output_file, first), data);
					});

					// Then, the leaf primary keys - for multiple leaves
					// (this is the K-ad)
					int numberLeavesHandled = 0;
					std::for_each(outputRow.primary_leaves_cache.cbegin(), outputRow.primary_leaves_cache.cend(), [&](int const & leafIndex)
					{
						Leaf & leaf = branch.leaves_cache[leafIndex];

						// For the case K = 1, there ARE no primary keys for this leaf.
						// But this is still covered here, see comments starting next line.
						// In the K = 1 case, the branch has one and only one leaf object with no primary keys
						// (just a data lookup), and everything is guaranteed to be output from the *branch* in this case.
						// But the branch data has already been output, above, so we're set in the K=1 case.
						// (Note that every output row has one leaf index in the K=1 case, pointing to this branch's single leaf).
						//
						// For the K > N case (i.e., there are more leaves requested by the user
						// than there are leaves available for this branch),
						// see the following code block after this std::for_each() exits.
						std::for_each(leaf.primary_keys.cbegin(), leaf.primary_keys.cend(), [&](DMUInstanceData const & data)
						{
							boost::apply_visitor(write_to_output_visitor(output_file, first), data);
						});
						++numberLeavesHandled;
					});

					// Fill in remaining leaf slots with blanks for the case K > N.
					// Note that K >= 1, and N >= 1.
					// Note that the K = 1 case corresponds to no data in any leaves,
					// and all primary key data will be output from the *branch*.
					// which has already been done, above.
					// (The K = 1 case is indicated by "which_primary_index_has_multiplicity_greater_than_1 == -1".)
					if (which_primary_index_has_multiplicity_greater_than_1 != -1)
					{
						int numberColumnsInTheDMUWithMultiplicityGreaterThan1 = biggest_counts[0].second[which_primary_index_has_multiplicity_greater_than_1].second;
						for (int n = numberLeavesHandled; n < K; ++n)
						{
							for (int nk = 0; nk < numberColumnsInTheDMUWithMultiplicityGreaterThan1; ++n)
							{
								boost::apply_visitor(write_to_output_visitor(output_file, first), InstanceData(std::string()));
							}
						}
					}

					// Then, the primary top-level variable group secondary data.
					// This info is stored in each leaf.
					numberLeavesHandled = 0;
					std::for_each(outputRow.primary_leaves_cache.cbegin(), outputRow.primary_leaves_cache.cend(), [&](int const & leafIndex)
					{
						Leaf & leaf = branch.leaves_cache[leafIndex];

						// Even the K=1 case is handled in the "index_into_raw_data > 0" block,
						// because although the leaf has no primary keys,
						// **it does have an index to data** (See RandomSampling_ReadData_AddToTimeSlices(),
						//   where the leaf's data index is set regardless of whether there are DMU columns for the leaf.)
						// Further, this secondary data is guaranteed to be present,
						// because it came from the same row of input data that the branch+leaf (or branch, for K=1) did.
						if (leaf.index_into_raw_data > 0) // index_into_raw_data is 1-based because it corresponds to SQLite's automatically-generated "rowid" column.
						{
							SecondaryInstanceDataVector const & secondary_data_vector = allWeightings.dataCache[leaf.index_into_raw_data];
							std::for_each(secondary_data_vector.cbegin(), secondary_data_vector.cend(), [&](SecondaryInstanceData const & data)
							{
								boost::apply_visitor(write_to_output_visitor(output_file, first), data);
							});
						}
						else
						{
							// no data available.  Import should always place blanks, so this should never happen.
							boost::format msg("Logic error: Missing primary variable group data (there isn't even blank data).");
							throw NewGeneException() << newgene_error_description(msg.str());
						}
						++numberLeavesHandled;
					});

					// Fill in remaining leaf slots with blanks for the case K > N.
					// See comments for identical scenario above, where the leaf primary data is being output.
					if (which_primary_index_has_multiplicity_greater_than_1 != -1)
					{
						int numberSecondaryColumns = top_level_number_secondary_columns[top_level_vg_index];
						for (int n = numberLeavesHandled; n < K; ++n)
						{
							for (int nk = 0; nk < numberSecondaryColumns; ++n)
							{
								boost::apply_visitor(write_to_output_visitor(output_file, first), InstanceData(std::string()));
							}
						}
					}

					// Then, the non-primary top-level variable group secondary data.
					// This info is stored in the leaf also.
					//
					// Show secondary data grouped by variable group,
					// then by multiplicity.
					// There might be multiple fields for each variable group and within each multiplicity,
					// ... so the logic is a bit non-trivial to get the display order right.
					int numberTopLevelGroups = static_cast<int>(primary_variable_groups_vector.size());
					for (int vgNumber = 0; vgNumber < numberTopLevelGroups; ++vgNumber)
					{
						for (int multiplicity = 0; multiplicity < K; ++multiplicity)
						{

							bool matchedVariableGroup = false;
							bool matchedMultiplicity = false;

							// Find the variable group and leaf that is desired.
							// Some variable groups have no data, in which case they will not be in the list;
							// this case is covered below.
							int testMultiplicity = 0;

							// Again, the K=1 case is covered here.
							// In the K=1 case, there is one leaf per output row,
							// and this leaf has no primary keys,
							// but it has the same secondary key information as any other leaf -
							// for both the primary variable group, and all non-primary top-level variable groups.
							// I.e., the non-primary top-level variable groups populate their
							// "other_top_level_indices_into_raw_data" variable in the same way that the K>1 case does
							//  (see RandomSampling_ReadData_AddToTimeSlices(), the VARIABLE_GROUP_MERGE_MODE__TOP_LEVEL case,
							//   for where "other_top_level_indices_into_raw_data" is populated (regardless of the
							//   K=1 case where there are no primary keys)).
							std::for_each(outputRow.primary_leaves_cache.cbegin(), outputRow.primary_leaves_cache.cend(), [&](int const & leafIndex)
							{

								if (testMultiplicity != multiplicity)
								{
									++testMultiplicity;
									return;
								}

								matchedMultiplicity = true;

								Leaf & leaf = branch.leaves_cache[leafIndex];

								// Here is where, in addition to the K>1 case, the K=1 case is supported,
								// because "other_top_level_indices_into_raw_data" is populated
								// regardless of whether "primary_keys" is populated for the leaf.
								std::for_each(leaf.other_top_level_indices_into_raw_data.cbegin(), leaf.other_top_level_indices_into_raw_data.cend(), [&](std::pair<int const, std::int64_t> const & top_level_vg_and_data_index)
								{
									int const vg_number = top_level_vg_and_data_index.first;
									if (vg_number == vgNumber)
									{

										// We now have both the multiplicity and the variable group that is desired
										// for the correct sequence of output data
										matchedVariableGroup = true;

										// *********************************************************************** //
										// If we have ANY secondary data, which we do if we're here,
										// then we have ALL COLUMNS of secondary data,
										// because the schema controls this and the data is pulled from the DB,
										// which has empty columns rather than missing columns
										// *********************************************************************** //
										int const vg_number = top_level_vg_and_data_index.first;
										std::int64_t const & data_index = top_level_vg_and_data_index.second;
										DataCache & data_cache = allWeightings.otherTopLevelCache[vg_number];
										SecondaryInstanceDataVector const & secondary_data_vector = data_cache[data_index];
										std::for_each(secondary_data_vector.cbegin(), secondary_data_vector.cend(), [&](SecondaryInstanceData const & data)
										{
											boost::apply_visitor(write_to_output_visitor(output_file, first), data);
										});

									}
								});
								++testMultiplicity;
							
							});

							if (matchedMultiplicity)
							{
								// The case of no matching data for an existing leaf,
								// for non-primary top-level variable groups, is covered here.
								//
								// We have matched the multiplicity,
								// but there was no variable group data for this multiplicity.
								// We must fill in all secondary keys for this multiplicity with blanks
								if (!matchedVariableGroup)
								{
									if (vgNumber != top_level_vg_index)
									{
										// Missing a variable group.  Fill with blanks.
										int numberSecondaries = top_level_number_secondary_columns[vgNumber];
										for (int n = 0; n < numberSecondaries; ++n)
										{
											boost::apply_visitor(write_to_output_visitor(output_file, first), InstanceData(std::string()));
										}
									}
								}
							}
							else
							{
								// The K > N case is covered here (missing leaves).
								//
								// We did not match the multiplicity.
								// This means there are missing leaves.
								// For the current leaf and variable group,
								// we must now fill in the secondary keys.
								// This "else" block could be merged with
								// the "if" block, but for conceptual clarity it is separated.
								if (vgNumber != top_level_vg_index)
								{
									int numberSecondaries = top_level_number_secondary_columns[vgNumber];
									for (int n = 0; n < numberSecondaries; ++n)
									{
										boost::apply_visitor(write_to_output_visitor(output_file, first), InstanceData(std::string()));
									}
								}
							}
						
						}
					}

					// Then, the child variable group secondary data.
					// This info is stored in the output row itself.
					//
					// Show secondary data grouped by variable group,
					// then by multiplicity.
					// There might be multiple fields for each variable group and within each multiplicity,
					// ... so the logic is a bit non-trivial to get the display order right.
					int numberChildGroups = static_cast<int>(secondary_variable_groups_vector.size());
					for (int vgNumber = 0; vgNumber < numberChildGroups; ++vgNumber)
					{
						int const the_child_multiplicity = child_uoas__which_multiplicity_is_greater_than_1[*(secondary_variable_groups_vector[vgNumber].first.identifier_parent)].second;
						for (int multiplicity = 0; multiplicity < the_child_multiplicity; ++multiplicity)
						{
							bool matched = false;
							std::for_each(outputRow.child_indices_into_raw_data.cbegin(), outputRow.child_indices_into_raw_data.cend(), [&](std::pair<int const, std::map<int, std::int64_t>> const & leaf_index_mappings)
							{
								int const vg_number = leaf_index_mappings.first;
								if (vg_number != vgNumber)
								{
									return;
								}
								std::map<int, std::int64_t> const & leaf_number_to_data_index = leaf_index_mappings.second;
								std::for_each(leaf_number_to_data_index.cbegin(), leaf_number_to_data_index.cend(), [&](std::pair<int const, std::int64_t> const & leaf_index_mapping)
								{

									int const leaf_number = leaf_index_mapping.first;
									if (leaf_number != multiplicity)
									{
										return;
									}

									if (leaf_number < 0)
									{
										return;
									}

									// This is the desired variable group and multiplicity
									matched = true;

									std::int64_t const & data_index = leaf_index_mapping.second;
									DataCache & data_cache = allWeightings.childCache[vg_number];
									SecondaryInstanceDataVector & secondary_data_vector = data_cache[data_index];
									std::for_each(secondary_data_vector.cbegin(), secondary_data_vector.cend(), [&](SecondaryInstanceData const & data)
									{
										boost::apply_visitor(write_to_output_visitor(output_file, first), data);
									});

								});
							});

							// All cases of missing data are covered here.
							// There was no child data that matched the given child leaf slots,
							// which could occur in one of two ways:
							// Either:
							// (1) There IS data present in the corresponding primary keys
							//     for this particular child leaf, but there is no matching child data
							// (2) There is no data present in the corresponding primary keys
							//     because K > N for the top-level unit of analysis for this branch
							// In either case, this means there is no child data so we will not have
							// an entry in "child_indices_into_raw_data" for the given
							// variable group and leaf number.
							// Special case: If there is no data for *any* child leaf, 
							// there will be no entry in "child_indices_into_raw_data" for the child variable group.
							//
							// See "AllWeightings::MergeTimeSliceDataIntoMap()" for where "child_indices_into_raw_data" is populated:
							// The code looks into the "helper_lookup__from_child_key_set__to_matching_output_rows" cache
							// for the branch to lookup both the output rows, and corresponding child leaf
							// indices within those output rows, that match against any given incoming child leaf.
							// Therefore, "leaf_number_to_data_index" will exist for all (and only for) those
							// matching output row / leaf indexes that match.
							if (!matched)
							{
								int numberSecondaries = child_number_secondary_columns[vgNumber];
								for (int n = 0; n < numberSecondaries; ++n)
								{
									boost::apply_visitor(write_to_output_visitor(output_file, first), InstanceData(std::string()));
								}
							}
						}
					}

					output_file << std::endl;
					++rows_written;

				});

			});

		});

	});

	if (failed || CheckCancelled())
	{
		return;
	}

	if (output_file.good())
	{
		output_file.close();
	}

	boost::format msg("%1% rows written to output.");
	msg % rows_written;
	messager.AppendKadStatusText(msg.str(), this);

}
