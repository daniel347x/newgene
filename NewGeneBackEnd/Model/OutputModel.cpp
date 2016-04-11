#ifndef Q_MOC_RUN
	#include <boost/lexical_cast.hpp>
	#include <deque>
	#include <boost/filesystem.hpp>
	#include <boost/format.hpp>
	#include <boost/scope_exit.hpp>
	#include <boost/date_time/local_time/local_time.hpp>
#endif
#include "OutputModel.h"
#include "../Utilities/NewGeneUUID.h"
#include "../Settings/OutputProjectSettings_list.h"
#include <cstdint>
#include <fstream>
#include <algorithm>

#include "OutputModelDdlSql.h"

std::recursive_mutex OutputModel::OutputGenerator::is_generating_output_mutex;
std::atomic<bool> OutputModel::OutputGenerator::is_generating_output(false);
bool OutputModel::OutputGenerator::cancelled = false;

int OutputModel::OutputGenerator::SQLExecutor::number_statement_prepares = 0;
int OutputModel::OutputGenerator::SQLExecutor::number_statement_finalizes = 0;
int OutputModel::OutputGenerator::number_transaction_begins = 0;
int OutputModel::OutputGenerator::number_transaction_ends = 0;

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

		t_limit_dmus_categories.Load(db, this, input_model.get());
		t_limit_dmus_set_members.Load(db, this, input_model.get());
	}

}

bool OutputModelImportTableFn(Importer * importer, Model_basemost * model_, ImportDefinition & import_definition, Table_basemost * table_, DataBlock const & table_block,
							  int const number_rows, long & linenum, long & badwritelines, long & goodwritelines, long & goodupdatelines, std::vector<std::string> & errors)
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
						table_->ImportBlockBulk(output_model->getDb(), import_definition, output_model, &output_model->getInputModel(), table_block, number_rows, linenum, badwritelines, goodwritelines,
												goodupdatelines, errors);
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
						table_->ImportBlockUpdate(output_model->getDb(), import_definition, output_model, &output_model->getInputModel(), table_block, number_rows, linenum, badwritelines, goodwritelines,
												  goodupdatelines, numlinesupdated,
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
	, delete_tables(true)
	, debug_ordering(false)
	, consolidate_rows(true)
	, display_absolute_time_columns(false)
	, random_sampling_number_rows(1)
	, random_sampling(false)
	, primary_vg_index__in__top_level_vg_vector(0)
	, K(0)
	, N_grand_total(0)
	, overall_total_number_of_primary_key_columns_including_all_branch_columns_and_all_leaves_and_all_columns_internal_to_each_leaf(0)
	, has_non_primary_top_level_groups { 0 }
	, has_child_groups{ 0 }
{}

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

	std::for_each(merging_of_children_column_sets.begin(), merging_of_children_column_sets.end(), [this](SqlAndColumnSet & sql_and_column_set)
	{

		std::for_each(sql_and_column_set.first.begin(), sql_and_column_set.first.end(), [this](SQLExecutor & sql_executor)
		{

			sql_executor.Empty();

		});

	});

	std::for_each(random_sampling_schema.first.begin(), random_sampling_schema.first.end(), [this](SQLExecutor & sql_executor)
	{

		sql_executor.Empty();

	});

	std::for_each(final_result.first.begin(), final_result.first.end(), [this](SQLExecutor & sql_executor)
	{

		sql_executor.Empty();

	});

}

KadSampler * currentKadSampler = nullptr;

void OutOfMemoryHandler()
{
	if (currentKadSampler)
	{
		currentKadSampler->Clear();
		currentKadSampler = nullptr;
	}

	throw std::bad_alloc();
}

void OutputModel::OutputGenerator::GenerateOutput(DataChangeMessage & change_response)
{

	cancelled = false;
	done = false;

	{
		std::lock_guard<std::recursive_mutex> guard(is_generating_output_mutex);

		if (is_generating_output)
		{
			messager.ShowMessageBox("Another k-ad output generation operation is in progress.  Please wait for that operation to complete first.");
			return;
		}

		is_generating_output = true;
	}

	messager.SetRunStatus(RUN_STATUS__RUNNING);
	BOOST_SCOPE_EXIT(&is_generating_output, &is_generating_output_mutex, &messager)
	{
		is_generating_output = false;
		messager.SetPerformanceLabel("");
		messager.SetRunStatus(RUN_STATUS__NOT_RUNNING);
	} BOOST_SCOPE_EXIT_END

	messager.AppendKadStatusText("", nullptr); // This will clear the pane
	boost::posix_time::ptime now = boost::posix_time::second_clock::local_time();
	std::string time_start_formatted = boost::posix_time::to_simple_string(now);
	boost::format msg_start("NewGene k-ad generation");
	messager.AppendKadStatusText(msg_start.str(), nullptr);
	boost::format msg_starttime("Starting run at %1%");
	msg_starttime % time_start_formatted;
	messager.AppendKadStatusText(msg_starttime.str(), nullptr);

	input_model = &model->getInputModel();

	bool delete_tables_ = delete_tables;
	BOOST_SCOPE_EXIT(&input_model, &delete_tables_)
	{

		// This is also done explicitLy at the end,
		// but it's better to include in both places,
		// the first (at end) so that user can benefit from status text,
		// and the second (here) in case of exit due to failure
		if (delete_tables_)
		{
			input_model->ClearRemnantTemporaryTables();
		}

		input_model->VacuumDatabase();

	} BOOST_SCOPE_EXIT_END

	Table_VARIABLES_SELECTED::UOA_To_Variables_Map the_map_ = model->t_variables_selected_identifiers.GetSelectedVariablesByUOA(model->getDb(), model, input_model);
	the_map = &the_map_;

	if (the_map->size() == 0)
	{
		SetFailureErrorMessage(boost::format("No variables are selected for output.").str().c_str());
		failed = true;
		return;
	}

	bool found = false;
	WidgetInstanceIdentifier_Int64_Pair timerange_start_identifier;
	found = model->t_time_range.getIdentifierFromStringCodeAndFlags("0", "s", timerange_start_identifier);

	if (!found)
	{
		SetFailureErrorMessage(boost::format("Cannot determine time range from database.").str().c_str());
		failed = true;
		return;
	}

	timerange_start = timerange_start_identifier.second;
	found = false;

	WidgetInstanceIdentifier_Int64_Pair timerange_end_identifier;
	found = model->t_time_range.getIdentifierFromStringCodeAndFlags("0", "e", timerange_end_identifier);

	if (!found)
	{
		SetFailureErrorMessage(boost::format("Cannot determine time range end value from database.").str().c_str());
		failed = true;
		return;
	}

	timerange_end = timerange_end_identifier.second;

	// The time range controls are only accurate to 1 second
	if (timerange_end <= (timerange_start + 999))
	{
		SetFailureErrorMessage(boost::format("The ending value of the time range must be greater than the starting value.").str().c_str());
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
		SetFailureErrorMessage(boost::format("No output file has been specified.").str().c_str());
		failed = true;
		return;
	}

	debug_sql_path = setting_path_to_kad_output;
	debug_sql_path.replace_extension(".debugsql.txt");

	messager.AppendKadStatusText("Validating database...", nullptr);
	input_model->ClearRemnantTemporaryTables();

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
		boost::format
		msg("Unable to open logfile.  This usually means that you are attempting to save data directly in the 'Program Files (x86)\\NewGene\\' directory.  Please save to a different location.");
		throw NewGeneException() << newgene_error_description(msg.str());
	}

	messager.UpdateStatusBarText((boost::format("Generating output to file %1%") % boost::filesystem::path(setting_path_to_kad_output).filename()).str().c_str(), this);


	// ******************************************************************************************* //
	// ******************************************************************************************* //
	// ******************************************************************************************* //
	//
	// Do NOT use managed memory, shared_ptr, or unique_ptr for the following object!
	// NEVER delete this object!
	// Instead, let the pointer fall off the stack,
	// and we manage ALL of the heap memory contained in the object ourselves
	// via the Boost Memory pool.
	//
	// This way, all of the objects and sub-objects created by the AllWeightings instance
	// will be deleted at the end of the k-ad run in one fell swoop
	// by a call to Boost Pool's "purge_memory()", without calling (or needing to call)
	// any destructors.
	//
	// The reason is that the data structures are so intricately nested,
	// and the heap so fragmented, that it literally requires 20 full minutes on one of the world's most
	// powerful CPU's to clean up the memory and exit the k-ad routine after NewGene completes writing
	// the output to file (during which time NewGene can't be used) for a reasonably complex run.
	//
	// We don't need to call any of those destructors, because they have no desired side effects,
	// so why not just purge a memory pool and be done in one second?
	// This is the point of a custom allocator, and we use the Boost Pool's custom memory allocator
	// to manage all of the memory ourselves in this way.
	//
	// Therefore, we never delete the following object.
	//
	// Note that a few bytes of data from the "POD-style" AllWeightings instance data members *do* leak
	// (not any nested data, however) - but this is trivial.  NewGene could run thousands of k-ad
	// routines of *any* complexity without being shut down and the leak would barely even be a tick on the scale.
	//
	// ******************************************************************************************* //
	// ******************************************************************************************* //
	// ******************************************************************************************* //

	// Memory management!
	// The following class is defined to use a Boost memory pool.
	// This single data structure is responsible for holding the "leaf-most" nodes
	// during the k-ad generation process, and there can easily be millions, scattered through the heap.
	// The memory pool requires that these all lie in a single block, at odds with the scattered nature of creation of these nodes.
	// We therefore CREATE an instance of this class once, before creating the KadSampler instance,
	// at a time when memory is available, and then request it to store something,
	// which triggers initial instantiation of a very large global pool for all instances of this class
	// that will be shared by the KadSampler.
	//fast_short_to_int_map dummy_memory_instantiator;
	//dummy_memory_instantiator[0] = 0; // Will trigger allocation of a huge block of global storage for use by all other instances of this class.

	KadSampler * allWeightings_ = new KadSampler(messager, cancelled); // SEE NOTE!  Do not delete this object!
	KadSampler & allWeightings = *allWeightings_;
	std::set_new_handler(OutOfMemoryHandler);
	BOOST_SCOPE_EXIT(&allWeightings, &allWeightings_, &model, &input_model, &currentKadSampler)
	{
		currentKadSampler = nullptr;
		std::set_new_handler(nullptr);

		if (model)
		{
			if (model->getDb())
			{
				sqlite3_db_release_memory(model->getDb());
			}

			sqlite3_db_release_memory(input_model->getDb());
		}

		allWeightings.Clear(); // This is the routine that purges all of the memory from the pool.

		// Do not delete!  That would trigger a cascade of deletes on all owned containers,
		// hanging NewGene for minutes for a ~1,000,000 row run.
		// Instead, allow the Boost memory pool to dump the memory en-bulk in the "Clear()" routine, above
		//delete allWeightings_;
	} BOOST_SCOPE_EXIT_END

	// ********************************************************************************************************************************************************* //
	// The main body of the k-ad generation routine follows
	// ********************************************************************************************************************************************************* //
	try
	{

		// ********************************************************************************************************************************************************* //
		// Initialize all metadata about the selected variables and variable groups,
		// multiplicities, units of analysis, primary keys, output column names, etc.
		// ********************************************************************************************************************************************************* //
		messager.AppendKadStatusText((boost::format("Populating k-ad metadata...")).str().c_str(), this);
		create_output_row_visitor::data = &allWeightings.create_output_row_visitor_global_data_cache;
		Prepare(allWeightings);

		if (failed || CheckCancelled()) { return; }

		if (allWeightings.time_granularity != TIME_GRANULARITY__NONE)
		{
			// Now that the time granularity has been set (in "Prepare()"),
			// make sure the time range begins and ends at the closest absolute granularity
			std::int64_t timerange_start_test_down = TimeRange::determineAligningTimestamp(timerange_start, allWeightings.time_granularity, TimeRange::ALIGN_MODE_DOWN);
			std::int64_t timerange_start_test_up = TimeRange::determineAligningTimestamp(timerange_start, allWeightings.time_granularity, TimeRange::ALIGN_MODE_UP);
			std::int64_t timerange_end_test_down = TimeRange::determineAligningTimestamp(timerange_end, allWeightings.time_granularity, TimeRange::ALIGN_MODE_DOWN);
			std::int64_t timerange_end_test_up = TimeRange::determineAligningTimestamp(timerange_end, allWeightings.time_granularity, TimeRange::ALIGN_MODE_UP);

			if (timerange_start != timerange_start_test_down)
			{
				// round
				if (timerange_start - timerange_start_test_down > timerange_start_test_up - timerange_start)
				{
					timerange_start = timerange_start_test_up;
				}
				else
				{
					timerange_start = timerange_start_test_down;
				}
			}

			if (timerange_end != timerange_end_test_up)
			{
				// round
				if (timerange_end - timerange_end_test_down >= timerange_end_test_up - timerange_end)
				{
					timerange_end = timerange_end_test_up;
				}
				else
				{
					timerange_end = timerange_end_test_down;
				}
			}
		}

		// ********************************************************************************************************************************************************* //
		// Build a schema object, one per variable group with selected data,
		// intended for internal use by the k-ad algorithm (i.e., converted from the raw format used in the database
		// into a handy set of schema instances)
		// ********************************************************************************************************************************************************* //
		// Also, determine, and set, the value of K for this run
		// (note: there is only a single DMU for which K can be greater than 1, currently)
		// ********************************************************************************************************************************************************* //
		PopulateSchemaForRawDataTables_And_SetK(allWeightings);

		if (failed || CheckCancelled()) { return; }

		primary_variable_group_column_sets.push_back(SqlAndColumnSets());
		SqlAndColumnSets & primary_group_column_sets = primary_variable_group_column_sets.back();

		// ********************************************************************************************************************************************************* //
		// From the schema for the selected columns for the primary variable group,
		// create a temporary table and store just the selected columns over just the selected time range.
		// This function both creates the table, and loads it with raw data,
		// but just for the desired columns (and time range), from the permanent raw data tables.
		// ********************************************************************************************************************************************************* //
		SqlAndColumnSet selected_raw_data_table_schema = CreateTableOfSelectedVariablesFromRawData(top_level_variable_groups_schema[primary_vg_index__in__top_level_vg_vector],
				primary_vg_index__in__top_level_vg_vector);

		if (failed || CheckCancelled()) { return; }

		selected_raw_data_table_schema.second.most_recent_sql_statement_executed__index = -1;
		ExecuteSQL(selected_raw_data_table_schema);

		if (failed || CheckCancelled()) { return; }

		primary_group_column_sets.push_back(selected_raw_data_table_schema);

		N_grand_total = ObtainCount(selected_raw_data_table_schema.second);

		if (K > N_grand_total)
		{
			SetFailureErrorMessage((
									   boost::format("The chosen value of K (%1%) exceeds the total number of rows of raw data (%2%) for the primary variable group \"%3%\" over the desired time range.  Please decrease the value of K.")
									   % boost::lexical_cast<std::string>(K) % boost::lexical_cast<std::string>(N_grand_total) % selected_raw_data_table_schema.second.variable_group_longhand_names[0]).str().c_str());
			failed = true;
			return;
		}

		// ********************************************************************************************************************************************************* //
		// Create TIME SLICES.
		// This is the first stage of k-ad generation.
		// It loads the raw data (as stored in the temporary table, above)
		// for the PRIMARY variable group
		// into the k-ad sampler, broken into time slices, to be made ready for following processing by the sampler.
		//
		// Later steps will load the other top-level, and the child, variable groups.
		// ********************************************************************************************************************************************************* //
		messager.AppendKadStatusText((boost::format("*****************************************************")).str().c_str(), this);
		messager.AppendKadStatusText((boost::format("*****************************************************")).str().c_str(), this);
		messager.AppendKadStatusText((boost::format("Load raw data for primary variable group \"%1%\" over the selected time range...") % Table_VG_CATEGORY::GetVgDisplayTextShort(
										  top_level_variable_groups_vector[primary_vg_index__in__top_level_vg_vector].first)).str().c_str(), this);
		std::vector<std::string> errorMessages;
		KadSampler_ReadData_AddToTimeSlices(selected_raw_data_table_schema.second, primary_vg_index__in__top_level_vg_vector, allWeightings, VARIABLE_GROUP_MERGE_MODE__PRIMARY,
											errorMessages);

		if (failed || CheckCancelled())
		{
			std::string errorsOut;
			bool first = true;

			for (auto errorMsg : errorMessages)
			{
				if (!first)
				{
					errorsOut += "\n";
				}

				first = false;
				errorsOut += errorMsg;
			}

			SetFailureErrorMessage(errorsOut);
			return;
		}

		// ********************************************************************************************************************************************************* //
		// For each branch, copy the leaf cache for that branch for fast lookup.
		//
		// Note that the leaves for each branch are stored in both a std::set and a std::vector.
		// They are populated into a std::set<> so that they are automatically ordered and so that inserts go quickly.
		// But now, we want fast access without ever adding or removing leaves,
		// rather than the ability to add or remove new leaves.
		// So do a one-time pass to copy the leaves from the set into the vector
		// for rapid lookup access WHEN CONSTRUCTING OUTPUT ROWS.
		//
		// The code that constructs the output rows looks in each "BranchOutputRow" instance within (a sub-time unit within) the branch
		// for an INDEX into a leaf, so that it can look into that leaf and retrieve
		// the actual DMU data for the leaf, AND indexes into the non-primary top-level data cache
		// for that data.
		//
		// Do not build the child DMU key lookup cache here.
		// Do that prior to loading each child variable group.
		// ********************************************************************************************************************************************************* //
		messager.AppendKadStatusText((boost::format("Build cache of available data for variable group \"%1%\"...") % Table_VG_CATEGORY::GetVgDisplayTextShort(
										  top_level_variable_groups_vector[primary_vg_index__in__top_level_vg_vector].first)).str().c_str(), this);
		allWeightings.ResetBranchCaches(-1, false);

		if (failed || CheckCancelled()) { return; }

		// ********************************************************************************************************************************************************* //
		// Calculate the number of possible k-ad combinations for each branch, given the leaves available.
		// ********************************************************************************************************************************************************* //
		messager.AppendKadStatusText((boost::format("Calculate the total number of k-adic combinations for the given DMU selection/s...")).str().c_str(), this);
		allWeightings.CalculateWeightings(K);

		if (failed || CheckCancelled()) { return; }

		// ********************************************************************************************************************************************************* //
		// The user will be interested to know how many k-ad combinations there are for their selection, so display that number now
		// ********************************************************************************************************************************************************* //
		messager.AppendKadStatusText((boost::format("Total number of granulated k-adic combinations available for this run: %1%") %
									  allWeightings.weighting.getWeightingString().c_str()).str().c_str(), this);
		messager.AppendKadStatusText((
										 boost::format("Guaranteed upper limit for total number of *consolidated* k-adic combinations: %1% (a quick estimate, but guaranteed to be an upper limit)") %
										 allWeightings.weighting_consolidated.getWeightingString().c_str()).str().c_str(), this);

		if (random_sampling)
		{

			// ********************************************************************************************************************************************************* //
			// Give text feedback that the sampler is entering random sampling mode
			// ********************************************************************************************************************************************************* //
			messager.AppendKadStatusText((boost::format("*****************************************************")).str().c_str(), this);
			messager.AppendKadStatusText((boost::format("Entering random sampling mode")).str().c_str(), this);
			messager.AppendKadStatusText((boost::format("Number of random samples selected: %1% ") % boost::lexical_cast<std::string>
										  (random_sampling_number_rows).c_str()).str().c_str(), this);

			// ********************************************************************************************************************************************************* //
			// If the user selects more random rows than there are k-ad combinations, then set the number of rows to be equal to the number of k-ad combinations.
			// Note: We remain nonetheless in random sampling mode, mostly for testing purposes.  One of the best ways to confirm that the random sampling and full
			// sampling modes are both working is to see that they give identical results when the number of random samples is set to be equal to the number of full samples.
			// Also, it is possible that the end user wishes to test the scaling properties of the code for their given data when randomly sampled, in which case
			// they will want to be able to remain in random sampling mode even at the 100% sampling level
			// ********************************************************************************************************************************************************* //
			std::int64_t samples = random_sampling_number_rows;

			if (newgene_cpp_int(random_sampling_number_rows) > allWeightings.weighting.getWeighting())
			{
				messager.AppendKadStatusText((boost::format("Decreasing the number of random samples to match the number of total k-adic combinations.")).str().c_str(), this);
				samples = allWeightings.weighting.getWeighting().convert_to<int>();
			}

			// ********************************************************************************************************************************************************* //
			// Generate and store the desired number of random numbers - one random number per output row is stored here
			// ********************************************************************************************************************************************************* //
			messager.AppendKadStatusText((boost::format("Generating %1% random numbers between 1 and %2%...") % boost::lexical_cast<std::string>
										  (samples).c_str() % allWeightings.weighting.getWeightingString().c_str()).str(), this);
			allWeightings.PrepareRandomNumbers(samples);

			if (failed || CheckCancelled()) { return; }

			// ********************************************************************************************************************************************************* //
			// Select the random rows now and stash in memory
			// ********************************************************************************************************************************************************* //
			messager.AppendKadStatusText((boost::format("Selecting %1% random k-adic combinations from %2% available granulated combinations...") % boost::lexical_cast<std::string>
										  (samples).c_str() % allWeightings.weighting.getWeightingString().c_str()).str().c_str(), this);
			allWeightings.PrepareRandomSamples(K);

			if (failed || CheckCancelled()) { return; }

			// *** OLD COMMENT LEFT IN PLACE TO ADD CLARITY TO CURRENT CODE, SEE BELOW FOR CURRENT STATUS ***
			// Do not clear random numbers!  Deallocation requires ~5-10 seconds for 1-2M on a solid CPU, but takes ~10-20MB memory.
			// For now - prioritize speed over memory.  Leave the memory in place and allow the Boost memory pool to clear at the end.
			// Note that when Boost Pool is enhanced to support class-level pools, THEN we'll be able to get the best of both worlds
			// and just clear the pool right here (in an instant).
			//allWeightings.ClearRandomNumbers(); // This function will not only "clear()" the vector, but "swap()" it with an empty vector to actually force deallocation - a major C++ gotcha!
			//messager.SetPerformanceLabel((boost::format("Clearing cache of random numbers...")).str().c_str());
			// *** DONE OLD COMMENT ***
			//
			// Update: NewGene has been enhanced to use a Boost pool for the random numbers, so we can now purge the pool in an instant.
			RandomVectorPool::purge_memory();
			RandomSetPool::purge_memory();
			purge_pool<newgene_cpp_int_random_tag, sizeof(boost::multiprecision::limb_type)>();
			purge_pool<newgene_cpp_int_random_tag, sizeof(newgene_cpp_int)>();
			allWeightings.ClearRemaining();
			messager.SetPerformanceLabel("");

			if (failed || CheckCancelled()) { return; }

		}
		else
		{

			// ********************************************************************************************************************************************************* //
			// Give text feedback that the sampler is entering full sampling mode
			// ********************************************************************************************************************************************************* //
			messager.AppendKadStatusText((boost::format("*****************************************************")).str().c_str(), this);

			if (consolidate_rows)
			{
				messager.AppendKadStatusText((boost::format("Entering full sampling mode, consolidating contiguous identical data.")).str().c_str(), this);
				messager.AppendKadStatusText((boost::format("Generating all k-adic combinations (no more than %1% consolidated combinations, and probably less)...") %
											  allWeightings.weighting_consolidated.getWeightingString().c_str()).str(), this);
			}
			else
			{
				messager.AppendKadStatusText((boost::format("Entering full sampling mode, with time granulation.  All %1% k-adic combinations will be generated.") %
											  allWeightings.weighting.getWeightingString().c_str()).str().c_str(), this);
				messager.AppendKadStatusText((boost::format("Generating all %1% k-adic combinations from the raw data ...") % allWeightings.weighting.getWeightingString().c_str()).str(), this);
			}

			// ********************************************************************************************************************************************************* //
			// Generate all k-ad combinations now
			// ********************************************************************************************************************************************************* //
			allWeightings.PrepareFullSamples(K);

			if (failed || CheckCancelled()) { return; }

		}

		// "remaining_tag" memory pool is used both by random sampler, and by full sampler
		allWeightings.PurgeTags<remaining_tag>();

		if (failed || CheckCancelled()) { return; }

		if (false)
		{

			// ********************************************************************************************************************************************************* //
			// This is only necessary for debugging
			// or further sorting/ordering/processing
			//
			// We aren't doing any post-processing of the data that requires it be stored in a table, rather than in memory,
			// but leave this code here as a starting point for when larger data sets are supported that require
			// disk-based management during k-ad generation
			// ********************************************************************************************************************************************************* //
			KadSamplerCreateOutputTable();

			if (failed || CheckCancelled()) { return; }

		}

		// ********************************************************************************************************************************************************* //
		// Create the schema for the output
		// ********************************************************************************************************************************************************* //
		random_sampling_schema = KadSamplerBuildOutputSchema();

		if (failed || CheckCancelled()) { return; }

		final_result = random_sampling_schema;

		if (has_non_primary_top_level_groups || has_child_groups)
		{

			// ********************************************************************************************************************************************************* //
			// Populate (merge) *BOTH* child variable groups *AND* non-primary top-level variable groups
			// ********************************************************************************************************************************************************* //
			messager.AppendKadStatusText((boost::format("*****************************************************")).str().c_str(), this);
			messager.AppendKadStatusText((
											 boost::format("Note: Merging of additional variable groups could increase the number of output rows beyond the numbers indicated above.")).str().c_str(), this);
			KadSamplerFillDataForNonPrimaryGroups(allWeightings);

			if (failed || CheckCancelled()) { return; }

			// ********************************************************************************************************************************************************* //
			// For each branch, copy the leaf cache for that branch for fast lookup.
			//
			// Note that the leaves for each branch are stored in both a std::set and a std::vector.
			// They are populated into a std::set<> so that they are automatically ordered and so that inserts go quickly.
			// But now, we want fast access without ever adding or removing leaves,
			// rather than the ability to add or remove new leaves.
			// So do a one-time pass to copy the leaves from the set into the vector
			// for rapid lookup access WHEN CONSTRUCTING OUTPUT ROWS.
			//
			// The code that constructs the output rows looks in each "BranchOutputRow" instance within (a time unit within) the branch
			// for an INDEX into a leaf, so that it can look into that leaf and retrieve
			// the actual DMU data for the leaf, AND indexes into the non-primary top-level data cache
			// for that data.
			//
			// This time, do not build the child DMU key lookup map, because the children
			// are already merged in, so we don't need to look up in the map to see what output rows incoming child rows map to.
			//
			// No need to rebuild any of the leaf caches if there are no non-primary variable groups;
			// hence this belongs in this block.
			// ********************************************************************************************************************************************************* //
			messager.AppendKadStatusText((boost::format("*****************************************************")).str().c_str(), this);
			messager.AppendKadStatusText((boost::format("Prepare cache for creation of output rows...")).str().c_str(), this);
			allWeightings.ClearBranchCaches();
			allWeightings.ResetBranchCaches(-1, false); // just update the primary "leaves" vector - not the child leaf lookup cache

			if (failed || CheckCancelled()) { return; }

		}

		if (consolidate_rows)
		{

			// ********************************************************************************************************************************************************* //
			// Eliminates the division of sets of output rows within
			// each branch among internal (to the branch)
			// "time unit entries" (each such entry containing
			// data for only a single time unit corresponding to the
			// primary variable group - noting that it is possible
			// for a single branch to contain *multiple* such time units;
			// i.e., the primary variable group has "month" time granularity,
			// but some branches (corresponding to rows of raw input data)
			// span multiple months.
			// Note that the merging of child groups can split
			// branches corresponding to only a SINGLE time unit
			// into branches corresponding to SUB-time units;
			// such as a "second" granularity child group
			// intersecting a month.
			// (Even time slices with multiple months in this example
			// can be split such that one or two resulting time slices
			// has such a sub-month width.)
			// This latter case is handled properly by the "PruneTimeUnits()"
			// function, guaranteeing that even in such sub-time-unit
			// branches, the rows appear once.
			// See detailed comments in the "PruneTimeUnits()" function.
			//
			// The output is stored in "consolidated_rows" of the AllWeightings object.
			// ********************************************************************************************************************************************************* //
			messager.AppendKadStatusText((boost::format("Consolidating adjacent rows...")).str(), this);
			ConsolidateData(random_sampling, allWeightings);

			if (failed || CheckCancelled()) { return; }

		}


		// ********************************************************************************************************************************************************* //
		// ********************************************************************************************************************************************************* //
		//
		// For future developers:
		// Here is an excellent way to debug the data structures used by this software.
		// Generate a full XML output of *all* data loaded into the KadSampler here.
		// The XML tags are almost disgracefully wordy so the file can take up a huge amount of space,
		// but the lengthy tag names make it clear what the purpose of the data is.
		//
		// Note that the full file is written to an array of strings first (in memory), so could crash the program
		// when trying to allocate memory for the strings if there's hundreds of MB's worth of XML
		//
		// std::vector<std::string> sdata_;
		// SpitAllWeightings(allWeightings, "filename"); // ".xml" automatically appended to the filename
		//
		// ********************************************************************************************************************************************************* //
		// ********************************************************************************************************************************************************* //


		// ********************************************************************************************************************************************************* //
		// Write the output to disk
		// ********************************************************************************************************************************************************* //
		messager.AppendKadStatusText("Writing results to disk...", this);
		messager.SetPerformanceLabel("Writing results to disk...");
		KadSamplerWriteResultsToFileOrScreen(allWeightings);
		messager.AppendKadStatusText((boost::format("Wrote %1% rows to output file \"%2%\"") % allWeightings.rowsWritten % setting_path_to_kad_output.c_str()).str(), this);

		if (failed || CheckCancelled()) { return; }

		// ********************************************************************************************************************************************************* //
		// Vacuum database, because for large runs, SQLite often increases the database file size from,
		// for example, 20 MB to nearly a GB.
		// The database will be left at this size on the end-user's machine if we do not vacuum it here.
		// ********************************************************************************************************************************************************* //
		// DISABLED for now
		//messager.AppendKadStatusText("Vacuuming and defragmenting database...", this);
		//messager.SetPerformanceLabel("Vacuuming and defragmenting database...");

		if (delete_tables)
		{
			// ********************************************************************************************************************************************************* //
			// Delete the tables we used to store the selected columns of raw data over the selected time range
			// ********************************************************************************************************************************************************* //
			input_model->ClearRemnantTemporaryTables();

			if (failed || CheckCancelled()) { return; }
		}

		input_model->VacuumDatabase();
		messager.SetPerformanceLabel("");
		messager.UpdateProgressBarValue(1000);
		messager.UpdateStatusBarText((boost::format("Output successfully generated (%1%)") % boost::filesystem::path(setting_path_to_kad_output).filename().string().c_str()).str().c_str(),
									 this);
		messager.AppendKadStatusText("Done.", this);

	}
	catch (boost::exception & e)
	{
		if (std::string const * error_desc = boost::get_error_info<newgene_error_description>(e))
		{
			boost::format msg("Error: %1%");
			msg % (*error_desc).c_str();
			SetFailureErrorMessage(msg.str());
			failed = true;
		}
		else
		{
			std::string the_error = boost::diagnostic_information(e);
			boost::format msg("Error: %1%");
			msg % the_error.c_str();
			SetFailureErrorMessage(msg.str());
			failed = true;
		}
	}
	catch (std::bad_alloc &)
	{
		SetFailureErrorMessage("Exception thrown: OUT OF MEMORY!  Exiting run.  Please re-run with a smaller and/or less complex output dataset.");
		failed = true;
	}
	catch (std::exception & e)
	{
		boost::format msg("Exception thrown: %1%");
		msg % e.what();
		SetFailureErrorMessage(msg.str());
		failed = true;
	}

	if (failed || CheckCancelled()) { return; }

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
	indices_of_primary_key_columns_with_outer_multiplicity_greater_than_1.clear();
	is_index_a_primary_key_with_outer_multiplicity_greater_than_1.clear();
	indices_of_primary_key_columns_with_outer_multiplicity_equal_to_1.clear();
	is_index_a_primary_key_with_outer_multiplicity_equal_to_1.clear();
	is_index_a_secondary_key.clear();
	indices_of_secondary_key_columns.clear();
	indices_of_all_columns.clear();
	rowid = 0;
	branch_has_excluded_dmu = false;
	leaf_has_excluded_dmu = false;
}

void OutputModel::OutputGenerator::DetermineInternalChildLeafCountMultiplicityGreaterThanOne(KadSampler & allWeightings, NewGeneSchema const & column_schema,
		int const child_variable_group_index)
{

	int internal_leaf_dmu_count = 0;
	std::for_each(column_schema.column_definitions.cbegin(),
				  column_schema.column_definitions.cend(), [&](NewGeneSchema::NewGeneColumnDefinition const & one_column)
	{

		if (one_column.column_type == NewGeneSchema::NewGeneColumnDefinition::COLUMN_TYPE__PRIMARY)
		{
			if (one_column.total_outer_multiplicity__in_total_kad__for_current_dmu_category__for_current_variable_group > 1)
			{
				++internal_leaf_dmu_count;
			}
		}

	});

	childInternalToOneLeafColumnCountForDMUWithMultiplicityGreaterThan1[child_variable_group_index] = internal_leaf_dmu_count;
	allWeightings.childInternalToOneLeafColumnCountForDMUWithMultiplicityGreaterThan1[child_variable_group_index] = internal_leaf_dmu_count;

}

void OutputModel::OutputGenerator::SavedRowData::PopulateFromCurrentRowInDatabase(NewGeneSchema const & column_schema, sqlite3_stmt * stmt_result, OutputModel & model)
{

	Clear();

	int datetime_start_column_index_of_possible_duplicate = (int)column_schema.column_definitions.size() - 2;
	int datetime_end_column_index_of_possible_duplicate = (int)column_schema.column_definitions.size() - 1;
	std::int64_t datetime_start_of_possible_duplicate = sqlite3_column_int64(stmt_result, datetime_start_column_index_of_possible_duplicate);
	std::int64_t datetime_end_of_possible_duplicate = sqlite3_column_int64(stmt_result, datetime_end_column_index_of_possible_duplicate);
	datetime_start = datetime_start_of_possible_duplicate;
	datetime_end = datetime_end_of_possible_duplicate;

	int column_data_type = 0;
	std::int64_t data_int64 = 0;
	long double data_float = 0.0;
	fast_string data_string;
	long double data_long = 0.0;

	int current_column = 0;

	std::for_each(column_schema.column_definitions.cbegin(),
				  column_schema.column_definitions.cend(), [&](NewGeneSchema::NewGeneColumnDefinition const & one_column)
	{

		if (failed || OutputModel::OutputGenerator::CheckCancelled())
		{
			return;
		}

		bool add_as_primary_key_column = false;
		bool add_as_primary_key_with_multiplicity_greater_than_1 = false;
		bool add_as_primary_key_with_multiplicity_equal_to_1 = false;
		bool add_as_secondary_key_column = false;

		if (one_column.column_type == NewGeneSchema::NewGeneColumnDefinition::COLUMN_TYPE__PRIMARY)
		{
			add_as_primary_key_column = true;

			if (one_column.total_outer_multiplicity__in_total_kad__for_current_dmu_category__for_current_variable_group > 1)
			{
				add_as_primary_key_with_multiplicity_greater_than_1 = true;
			}
			else
			{
				add_as_primary_key_with_multiplicity_equal_to_1 = true;
			}
		}

		if (one_column.column_type == NewGeneSchema::NewGeneColumnDefinition::COLUMN_TYPE__SECONDARY)
		{
			add_as_secondary_key_column = true;
		}

		if (add_as_primary_key_column)
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

		if (add_as_primary_key_with_multiplicity_greater_than_1)
		{
			is_index_a_primary_key_with_outer_multiplicity_greater_than_1.push_back(true);
		}
		else
		{
			is_index_a_primary_key_with_outer_multiplicity_greater_than_1.push_back(false);
		}

		if (add_as_primary_key_with_multiplicity_equal_to_1)
		{
			is_index_a_primary_key_with_outer_multiplicity_equal_to_1.push_back(true);
		}
		else
		{
			is_index_a_primary_key_with_outer_multiplicity_equal_to_1.push_back(false);
		}

		bool add_as_secondary_datetime_column = one_column.originally_datetime;

		bool is_excluded_dmu_category = false;
		bool is_excluded_dmu_set_member = false; // this gets set, if necessary, below

		if (add_as_primary_key_column)
		{
			// Test if this DMU member is being excluded by the user in the "Limit DMU's" tab
			is_excluded_dmu_category = model.t_limit_dmus_categories.ExistsInCache(model.getDb(), model, model.getInputModel(), *one_column.primary_key_dmu_category_identifier.code);
		}

		column_data_type = sqlite3_column_type(stmt_result, current_column);

		switch (column_data_type)
		{

			case SQLITE_INTEGER:
				{

					data_int64 = sqlite3_column_int64(stmt_result, current_column);

					if (is_excluded_dmu_category)
					{
						is_excluded_dmu_set_member = ! model.t_limit_dmus_set_members.ExistsInCache(model.getDb(), model, model.getInputModel(), one_column.primary_key_dmu_category_identifier,
													 data_int64);
					}

					std::pair<SQLExecutor::WHICH_BINDING, std::pair<int, int>> binding;

					current_parameter_ints.push_back(data_int64);
					current_parameter_which_binding_to_use.push_back(SQLExecutor::INT64);
					binding = std::make_pair(SQLExecutor::INT64, std::make_pair((int)current_parameter_ints.size() - 1, current_column));

					indices_of_all_columns.push_back(binding);

					if (is_index_a_primary_key[current_column])
					{
						indices_of_primary_key_columns.push_back(binding);
					}

					if (is_index_a_primary_key_with_outer_multiplicity_greater_than_1[current_column])
					{
						indices_of_primary_key_columns_with_outer_multiplicity_greater_than_1.push_back(binding);

						if (is_excluded_dmu_set_member)
						{
							leaf_has_excluded_dmu = true;
						}
					}

					if (is_index_a_primary_key_with_outer_multiplicity_equal_to_1[current_column])
					{
						indices_of_primary_key_columns_with_outer_multiplicity_equal_to_1.push_back(binding);

						if (is_excluded_dmu_set_member)
						{
							branch_has_excluded_dmu = true;
						}
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

					if (is_index_a_primary_key[current_column])
					{
						indices_of_primary_key_columns.push_back(std::make_pair(SQLExecutor::FLOAT, std::make_pair((int)current_parameter_floats.size() - 1, current_column)));
					}

					if (is_index_a_primary_key_with_outer_multiplicity_greater_than_1[current_column])
					{
						indices_of_primary_key_columns_with_outer_multiplicity_greater_than_1.push_back(std::make_pair(OutputModel::OutputGenerator::SQLExecutor::FLOAT,
								std::make_pair((int)current_parameter_floats.size() - 1, current_column)));

						if (is_excluded_dmu_set_member)
						{
							leaf_has_excluded_dmu = true;
						}
					}

					if (is_index_a_primary_key_with_outer_multiplicity_equal_to_1[current_column])
					{
						indices_of_primary_key_columns_with_outer_multiplicity_equal_to_1.push_back(std::make_pair(OutputModel::OutputGenerator::SQLExecutor::FLOAT,
								std::make_pair((int)current_parameter_floats.size() - 1, current_column)));

						if (is_excluded_dmu_set_member)
						{
							branch_has_excluded_dmu = true;
						}
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

					if (is_excluded_dmu_category)
					{
						is_excluded_dmu_set_member = ! model.t_limit_dmus_set_members.ExistsInCache(model.getDb(), model, model.getInputModel(), one_column.primary_key_dmu_category_identifier,
													 data_string.c_str());
					}

					current_parameter_strings.push_back(data_string);
					current_parameter_which_binding_to_use.push_back(SQLExecutor::STRING);

					indices_of_all_columns.push_back(std::make_pair(SQLExecutor::STRING, std::make_pair((int)current_parameter_strings.size() - 1, current_column)));

					if (is_index_a_primary_key[current_column])
					{
						indices_of_primary_key_columns.push_back(std::make_pair(SQLExecutor::STRING, std::make_pair((int)current_parameter_strings.size() - 1, current_column)));
					}

					if (is_index_a_primary_key_with_outer_multiplicity_greater_than_1[current_column])
					{
						indices_of_primary_key_columns_with_outer_multiplicity_greater_than_1.push_back(std::make_pair(OutputModel::OutputGenerator::SQLExecutor::STRING,
								std::make_pair((int)current_parameter_strings.size() - 1, current_column)));

						if (is_excluded_dmu_set_member)
						{
							leaf_has_excluded_dmu = true;
						}
					}

					if (is_index_a_primary_key_with_outer_multiplicity_equal_to_1[current_column])
					{
						indices_of_primary_key_columns_with_outer_multiplicity_equal_to_1.push_back(std::make_pair(OutputModel::OutputGenerator::SQLExecutor::STRING,
								std::make_pair((int)current_parameter_strings.size() - 1, current_column)));

						if (is_excluded_dmu_set_member)
						{
							branch_has_excluded_dmu = true;
						}
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

					if (is_index_a_primary_key[current_column])
					{
						indices_of_primary_key_columns.push_back(std::make_pair(SQLExecutor::NULL_BINDING, std::make_pair(0, current_column)));
					}

					if (is_index_a_primary_key_with_outer_multiplicity_greater_than_1[current_column])
					{
						indices_of_primary_key_columns_with_outer_multiplicity_greater_than_1.push_back(std::make_pair(OutputModel::OutputGenerator::SQLExecutor::NULL_BINDING, std::make_pair(0,
								current_column)));
					}

					if (is_index_a_primary_key_with_outer_multiplicity_equal_to_1[current_column])
					{
						indices_of_primary_key_columns_with_outer_multiplicity_equal_to_1.push_back(std::make_pair(OutputModel::OutputGenerator::SQLExecutor::NULL_BINDING, std::make_pair(0,
								current_column)));
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

	data_int64 = sqlite3_column_int64(stmt_result, current_column);
	rowid = data_int64;

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
		SetFailureErrorMessage(msg.str());
		failed = true;
		return false;
	}

	return true;

}

void OutputModel::OutputGenerator::ObtainData(NewGeneSchema const & column_set, bool const obtain_rowid)
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
		SetFailureErrorMessage(msg.str());
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

std::int64_t OutputModel::OutputGenerator::ObtainCount(NewGeneSchema const & column_set)
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
		SetFailureErrorMessage(msg.str());
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
		SetFailureErrorMessage(msg.str());
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
			SetFailureErrorMessage(sql_executor.error_message);
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
	this->sql_error = rhs.sql_error;
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
	this->sql_error = rhs.sql_error;
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
					  bound_parameter_which_binding_to_use.cend(), [&stmt, &bound_parameter_strings, &bound_parameter_ints, &bound_parameter_floats, &bound_parameter_which_binding_to_use,
							  &current_string_index, &current_int64_index, &current_float_index, &current_index](
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

OutputModel::OutputGenerator::SqlAndColumnSet OutputModel::OutputGenerator::CreateTableOfSelectedVariablesFromRawData(NewGeneSchema const & variable_group_raw_data_columns,
		int const group_number)
{

	SqlAndColumnSet result = std::make_pair(std::vector<SQLExecutor>(), NewGeneSchema());
	std::vector<SQLExecutor> & sql_strings = result.first;
	NewGeneSchema & result_columns = result.second;

	// Note: The "columns_in_view" of result_columns will be CLEARED
	// and REFILLED below!
	result_columns = variable_group_raw_data_columns;

	result_columns.view_number =
		1; // which set of secondary keys is this table - from 1 to K where K is the multiplicity.  This is the first, so set to 1.  (Regardless of which top-level primary variable group this is (currently multiple top-level primary variable groups is not supported).)
	result_columns.has_no_datetime_columns =
		false; // Only the actual permanent data table in the database can have this be set to true.  Here, we are doing a SELECT of the permanent data for the first time into a temporary table.
	std::string view_name;

	switch (variable_group_raw_data_columns.schema_type)
	{
		case NewGeneSchema::SCHEMA_TYPE__RAW__SELECTED_VARIABLES_PRIMARY:
			view_name = "NGTEMP__RAW__SELECTED_VARIABLES_PRIMARY";
			break;

		case NewGeneSchema::SCHEMA_TYPE__RAW__SELECTED_VARIABLES_TOP_LEVEL_NOT_PRIMARY:
			view_name = "NGTEMP__RAW__SELECTED_VARIABLES_TOP_LEVEL_NOT_PRIMARY";
			break;

		case NewGeneSchema::SCHEMA_TYPE__RAW__SELECTED_VARIABLES_CHILD:
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

	result_columns.column_definitions.clear();

	// Create the schema columns from the raw data table into the temporary table.
	// Start with the primary key columns.
	std::for_each(variable_group_raw_data_columns.column_definitions.cbegin(),
				  variable_group_raw_data_columns.column_definitions.cend(), [&result_columns, &variables_selected](NewGeneSchema::NewGeneColumnDefinition const & column_in_view)
	{
		if (column_in_view.column_type == NewGeneSchema::NewGeneColumnDefinition::COLUMN_TYPE__DATETIMESTART
			|| column_in_view.column_type == NewGeneSchema::NewGeneColumnDefinition::COLUMN_TYPE__DATETIMEEND)
		{
			return; // Enforce that datetime columns appear last.
		}

		bool match = true;

		if (column_in_view.column_type == NewGeneSchema::NewGeneColumnDefinition::COLUMN_TYPE__SECONDARY)
		{
			return; // Enforce that primary key columns appear first.
		}

		if (match)
		{
			result_columns.column_definitions.push_back(column_in_view);
		}
	});

	switch (variable_group_raw_data_columns.schema_type)
	{
		case NewGeneSchema::SCHEMA_TYPE__RAW__SELECTED_VARIABLES_PRIMARY:
		case NewGeneSchema::SCHEMA_TYPE__RAW__SELECTED_VARIABLES_TOP_LEVEL_NOT_PRIMARY:
			top_level_number_secondary_columns[group_number] = 0;
			break;

		case NewGeneSchema::SCHEMA_TYPE__RAW__SELECTED_VARIABLES_CHILD:
			child_number_secondary_columns[group_number] = 0;
			break;
	}

	// Proceed to the secondary key columns.
	std::for_each(variable_group_raw_data_columns.column_definitions.cbegin(),
				  variable_group_raw_data_columns.column_definitions.cend(), [&](NewGeneSchema::NewGeneColumnDefinition const & column_in_view)
	{
		bool make_secondary_datetime_column = false;

		if (column_in_view.column_type == NewGeneSchema::NewGeneColumnDefinition::COLUMN_TYPE__DATETIMESTART
			|| column_in_view.column_type == NewGeneSchema::NewGeneColumnDefinition::COLUMN_TYPE__DATETIMEEND)
		{
			// No!  If the user selects these columns, they should appear as regular secondary key columns.  Change the column type in this case to "secondary".
			//return; // Enforce that datetime columns appear last.
			make_secondary_datetime_column = true;
		}

		if (column_in_view.column_type != NewGeneSchema::NewGeneColumnDefinition::COLUMN_TYPE__SECONDARY)
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
				case NewGeneSchema::SCHEMA_TYPE__RAW__SELECTED_VARIABLES_PRIMARY:
				case NewGeneSchema::SCHEMA_TYPE__RAW__SELECTED_VARIABLES_TOP_LEVEL_NOT_PRIMARY:
					++top_level_number_secondary_columns[group_number];
					break;

				case NewGeneSchema::SCHEMA_TYPE__RAW__SELECTED_VARIABLES_CHILD:
					++child_number_secondary_columns[group_number];
					break;
			}


			result_columns.column_definitions.push_back(column_in_view);

			if (make_secondary_datetime_column)
			{
				result_columns.column_definitions.back().column_type = NewGeneSchema::NewGeneColumnDefinition::COLUMN_TYPE__SECONDARY;
				result_columns.column_definitions.back().originally_datetime = true;
			}
		}
	});

	// Proceed, finally, to the datetime columns, if they exist.  (If they don't, they will be added via ALTER TABLE to the temporary table under construction.)
	std::for_each(variable_group_raw_data_columns.column_definitions.cbegin(),
				  variable_group_raw_data_columns.column_definitions.cend(), [&result_columns](NewGeneSchema::NewGeneColumnDefinition const & column_in_view)
	{
		// Now do the datetime_start column
		if (column_in_view.column_type == NewGeneSchema::NewGeneColumnDefinition::COLUMN_TYPE__DATETIMESTART)
		{
			result_columns.column_definitions.push_back(column_in_view);
		}
	});
	std::for_each(variable_group_raw_data_columns.column_definitions.cbegin(),
				  variable_group_raw_data_columns.column_definitions.cend(), [&result_columns](NewGeneSchema::NewGeneColumnDefinition const & column_in_view)
	{
		// Now do the datetime_end column
		if (column_in_view.column_type == NewGeneSchema::NewGeneColumnDefinition::COLUMN_TYPE__DATETIMEEND)
		{
			result_columns.column_definitions.push_back(column_in_view);
		}
	});

	WidgetInstanceIdentifier variable_group_saved;
	WidgetInstanceIdentifier uoa_saved;

	// Make column names for this temporary table unique (not the same as the column names from the previous table that is being copied).
	// Also, set the primary UOA flag.
	bool first = true;
	std::for_each(result_columns.column_definitions.begin(), result_columns.column_definitions.end(), [&first, &variable_group_saved, &uoa_saved](
					  NewGeneSchema::NewGeneColumnDefinition & new_column)
	{
		new_column.column_name_in_temporary_table = new_column.column_name_in_temporary_table_no_uuid;
		new_column.column_name_in_temporary_table += "_";
		new_column.column_name_in_temporary_table += newUUID(true);

		if (first)
		{
			first = false;
			variable_group_saved = new_column.current_variable_group;
			uoa_saved = new_column.uoa_associated_with_current_variable_group;
		}
	});

	sql_strings.push_back(SQLExecutor(this, db));
	std::string & sql_string = sql_strings.back().sql;

	sql_string = "CREATE TABLE \"";
	sql_string += result_columns.view_name;
	sql_string += "\" AS ";
	sql_string += "SELECT ";

	first = true;
	std::for_each(result_columns.column_definitions.begin(), result_columns.column_definitions.end(), [&sql_string, &first](NewGeneSchema::NewGeneColumnDefinition & new_column)
	{
		if (!first)
		{
			sql_string += ", ";
		}

		first = false;

		if (new_column.column_type == NewGeneSchema::NewGeneColumnDefinition::COLUMN_TYPE__PRIMARY
			&& new_column.primary_key_should_be_treated_as_integer_____float_not_allowed_as_primary_key)
		{
			sql_string += "CAST (";
		}

		sql_string += "`";
		sql_string += new_column.column_name_in_temporary_table_no_uuid; // This is the original column name
		sql_string += "`";

		if (new_column.column_type == NewGeneSchema::NewGeneColumnDefinition::COLUMN_TYPE__PRIMARY
			&& new_column.primary_key_should_be_treated_as_integer_____float_not_allowed_as_primary_key)
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
	// in both time range fields)
	if (!variable_group_raw_data_columns.has_no_datetime_columns_originally)
	{
		sql_string += " WHERE ( CASE WHEN `";
		sql_string += result_columns.column_definitions[result_columns.column_definitions.size() - 2].column_name_in_temporary_table;
		sql_string += "` = 0 AND `";
		sql_string += result_columns.column_definitions[result_columns.column_definitions.size() - 1].column_name_in_temporary_table;
		sql_string += "` = 0 ";
		sql_string += " THEN 1 ";
		sql_string += " WHEN `";
		sql_string += result_columns.column_definitions[result_columns.column_definitions.size() - 2].column_name_in_temporary_table;
		sql_string += "` < ";
		sql_string += boost::lexical_cast<std::string>(timerange_end);
		sql_string += " THEN `";
		sql_string += result_columns.column_definitions[result_columns.column_definitions.size() - 1].column_name_in_temporary_table;
		sql_string += "` > ";
		sql_string += boost::lexical_cast<std::string>(timerange_start);
		sql_string += " WHEN `";
		sql_string += result_columns.column_definitions[result_columns.column_definitions.size() - 1].column_name_in_temporary_table;
		sql_string += "` > ";
		sql_string += boost::lexical_cast<std::string>(timerange_start);
		sql_string += " THEN `";
		sql_string += result_columns.column_definitions[result_columns.column_definitions.size() - 2].column_name_in_temporary_table;
		sql_string += "` < ";
		sql_string += boost::lexical_cast<std::string>(timerange_end);
		sql_string += " ELSE 0";
		sql_string += " END )";
	}

	// SQL to add the datetime columns, if they are not present in the raw data table (filled with 0)
	if (variable_group_raw_data_columns.has_no_datetime_columns_originally)
	{

		result_columns.current_block_datetime_column_types = std::make_pair(NewGeneSchema::NewGeneColumnDefinition::COLUMN_TYPE__DATETIMESTART_INTERNAL,
				NewGeneSchema::NewGeneColumnDefinition::COLUMN_TYPE__DATETIMEEND_INTERNAL);

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
			SetFailureErrorMessage(sql_error);
			return result;
		}

		if (CheckCancelled())
		{
			return result;
		}

		result_columns.column_definitions.push_back(NewGeneSchema::NewGeneColumnDefinition());
		NewGeneSchema::NewGeneColumnDefinition & datetime_start_column = result_columns.column_definitions.back();
		datetime_start_column.column_name_in_temporary_table = datetime_start_col_name;
		datetime_start_column.column_name_in_temporary_table_no_uuid = datetime_start_col_name_no_uuid;
		datetime_start_column.column_type = NewGeneSchema::NewGeneColumnDefinition::COLUMN_TYPE__DATETIMESTART_INTERNAL;
		datetime_start_column.current_variable_group = variable_group_saved;
		datetime_start_column.uoa_associated_with_current_variable_group = uoa_saved;
		datetime_start_column.column_name_in_original_data_table = "";
		datetime_start_column.current_multiplicity__of__this_column__in__output__same_as__current_multiplicity__of___this_column__in_its_own_variable_group = 1;

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
			SetFailureErrorMessage(sql_error);
			return result;
		}

		if (CheckCancelled())
		{
			return result;
		}

		result_columns.column_definitions.push_back(NewGeneSchema::NewGeneColumnDefinition());
		NewGeneSchema::NewGeneColumnDefinition & datetime_end_column = result_columns.column_definitions.back();
		datetime_end_column.column_name_in_temporary_table = datetime_end_col_name;
		datetime_end_column.column_name_in_temporary_table_no_uuid = datetime_end_col_name_no_uuid;
		datetime_end_column.column_type = NewGeneSchema::NewGeneColumnDefinition::COLUMN_TYPE__DATETIMEEND_INTERNAL;
		datetime_end_column.current_variable_group = variable_group_saved;
		datetime_end_column.uoa_associated_with_current_variable_group = uoa_saved;
		datetime_end_column.column_name_in_original_data_table = "";
		datetime_end_column.current_multiplicity__of__this_column__in__output__same_as__current_multiplicity__of___this_column__in_its_own_variable_group = 1;

	}
	else
	{
		result_columns.current_block_datetime_column_types = std::make_pair(NewGeneSchema::NewGeneColumnDefinition::COLUMN_TYPE__DATETIMESTART,
				NewGeneSchema::NewGeneColumnDefinition::COLUMN_TYPE__DATETIMEEND);
		at_least_one_variable_group_has_timerange = true;
	}

	result_columns.previous_block_datetime_column_types = result_columns.current_block_datetime_column_types;

	return result;

}

void OutputModel::OutputGenerator::Prepare(KadSampler & allWeightings)
{

	// If we ever switch to using the SQLite "temp" mechanism, utilize temp_dot
	//temp_dot = "temp.";
	temp_dot = "";

	db = input_model->getDb();

	executor.db = db;

	tables_deleted.clear();

	initialized = true;

	std::tuple<bool, std::int64_t, bool, bool> general_info = model->t_general_options.getKadSamplerInfo(model->getDb());
	random_sampling = std::get<0>(general_info);
	random_sampling_number_rows = std::get<1>(general_info);
	consolidate_rows = std::get<2>(general_info);
	display_absolute_time_columns = std::get<3>(general_info);

	if (random_sampling && (random_sampling_number_rows <= 0))
	{
		boost::format msg("You have selected random sampling, but the number of desired rows to generate is 0.");
		SetFailureErrorMessage(msg.str());
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

	if (failed || CheckCancelled())
	{
		return;
	}

	primary_vg_index__in__top_level_vg_vector = 0;

	if (top_level_variable_groups_vector.size() > 1)
	{
		std::vector<WidgetInstanceIdentifier> variableGroupOptions;
		std::for_each(top_level_variable_groups_vector.cbegin(),
					  top_level_variable_groups_vector.cend(), [&](std::pair<WidgetInstanceIdentifier, WidgetInstanceIdentifiers> const & vg_to_selected)
		{
			variableGroupOptions.push_back(vg_to_selected.first);
		});
		boost::format msgTitle("Select top-level variable group");
		boost::format
		msgQuestion("There are multiple variable groups with the same set of unit of analysis fields that might be used as the \"primary\" variable group for the run.\nPlease select one to use as the primary variable group for this run:");

		// 0-based
		primary_vg_index__in__top_level_vg_vector = static_cast<size_t>(messager.ShowOptionMessageBox(msgTitle.str(), msgQuestion.str(), variableGroupOptions));

		if (primary_vg_index__in__top_level_vg_vector == -1)
		{
			// user cancelled
			failed = true;
			return;
		}
	}

	top_level_vg = top_level_variable_groups_vector[primary_vg_index__in__top_level_vg_vector].first;

	if (failed || CheckCancelled())
	{
		return;
	}

	PopulatePrimaryKeySequenceInfo();

	if (failed || CheckCancelled())
	{
		return;
	}

	allWeightings.time_granularity = top_level_variable_groups_vector[primary_vg_index__in__top_level_vg_vector].first.time_granularity;

	// ************************************************************ //
	// IMPORTANT:
	// Disallow "granulated output" mode for time granularity = none
	// ************************************************************ //
	if (!consolidate_rows && allWeightings.time_granularity == TIME_GRANULARITY__NONE)
	{
		boost::format
		msg("\"Granulated output\" mode (i.e., consolidate output rows is UNCHECKED) is not possible for the primary variable group you have selected because the primary variable group has no time granularity associated with it.  NewGene would not know how to separate the output rows into time slices.");
		SetFailureErrorMessage(msg.str());
		failed = true;
	}

}

void OutputModel::OutputGenerator::PopulateSchemaForRawDataTables_And_SetK(KadSampler & allWeightings)
{

	// ***************************************************************************************************************** //
	// Build schemas for top-level variable groups (this includes the primary variable group)
	// ***************************************************************************************************************** //
	int primary_view_count = 0;
	int primary_or_secondary_view_index = 0;
	std::for_each(top_level_variable_groups_vector.cbegin(),
				  top_level_variable_groups_vector.cend(), [&](std::pair<WidgetInstanceIdentifier, WidgetInstanceIdentifiers> const & the_primary_variable_group)
	{
		PopulateSchemaForRawDataTable(the_primary_variable_group, primary_view_count, top_level_variable_groups_schema, true, primary_or_secondary_view_index);

		// Store the number of branch & leaf columns for the primary variable group
		if (primary_or_secondary_view_index == primary_vg_index__in__top_level_vg_vector)
		{
			allWeightings.number_branch_columns = 0;
			allWeightings.number_primary_variable_group_single_leaf_columns = 0;
			NewGeneSchema & columns_in_variable_group_view = top_level_variable_groups_schema.back();
			columns_in_variable_group_view.is_primary_vg = true;

			for (auto const & column : columns_in_variable_group_view.column_definitions)
			{
				if (column.column_type == NewGeneSchema::NewGeneColumnDefinition::COLUMN_TYPE__PRIMARY)
				{
					if (column.total_outer_multiplicity__in_total_kad__for_current_dmu_category__for_current_variable_group == 1)
					{
						++allWeightings.number_branch_columns;
					}
					else
					{
						++allWeightings.number_primary_variable_group_single_leaf_columns;
					}
				}
			}
		}

		++primary_or_secondary_view_index;
	});

	// ***************************************************************************************************************** //
	// Build schemas for child variable groups
	// ***************************************************************************************************************** //
	int secondary_view_count = 0;
	primary_or_secondary_view_index = 0;
	allWeightings.numberChildVariableGroups = static_cast<int>(child_variable_groups_vector.size());
	std::for_each(child_variable_groups_vector.cbegin(),
				  child_variable_groups_vector.cend(), [&](std::pair<WidgetInstanceIdentifier, WidgetInstanceIdentifiers> const & the_secondary_variable_group)
	{
		PopulateSchemaForRawDataTable(the_secondary_variable_group, secondary_view_count, secondary_variable_groups_schema, false, primary_or_secondary_view_index);
		NewGeneSchema const & childSchema = secondary_variable_groups_schema.back();
		DetermineInternalChildLeafCountMultiplicityGreaterThanOne(allWeightings, childSchema, primary_or_secondary_view_index);
		++primary_or_secondary_view_index;
	});

	K = 0;
	int highest_multiplicity = 1;
	NewGeneSchema const & primary_variable_group_raw_data_columns = top_level_variable_groups_schema[primary_vg_index__in__top_level_vg_vector];
	std::for_each(primary_variable_group_raw_data_columns.column_definitions.cbegin(),
				  primary_variable_group_raw_data_columns.column_definitions.cend(), [&](NewGeneSchema::NewGeneColumnDefinition const & raw_data_table_column)
	{

		if (raw_data_table_column.total_outer_multiplicity__in_total_kad__for_current_dmu_category__for_current_variable_group > highest_multiplicity)
		{
			highest_multiplicity = raw_data_table_column.total_outer_multiplicity__in_total_kad__for_current_dmu_category__for_current_variable_group;
		}

	});

	K = highest_multiplicity;
	has_non_primary_top_level_groups = (top_level_variable_groups_schema.size() - 1) > 0;
	has_child_groups = secondary_variable_groups_schema.size() > 0;

}

void OutputModel::OutputGenerator::PopulateSchemaForRawDataTable(std::pair<WidgetInstanceIdentifier, WidgetInstanceIdentifiers> const & the_variable_group, int view_count,
		std::vector<NewGeneSchema> & variable_groups_column_info, bool const & is_primary, int const primary_or_secondary_view_index)
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

	// Convert column metadata into a far more useful form for construction of k-adic output

	if (failed || CheckCancelled())
	{
		return;
	}

	std::string vg_code = *the_variable_group.first.code;
	std::string vg_data_table_name = Table_VariableGroupData::TableNameFromVGCode(vg_code);

	variable_groups_column_info.push_back(NewGeneSchema());
	NewGeneSchema & columns_in_variable_group_view = variable_groups_column_info.back();

	if (is_primary)
	{
		if (primary_or_secondary_view_index == primary_vg_index__in__top_level_vg_vector)
		{
			columns_in_variable_group_view.schema_type = NewGeneSchema::SCHEMA_TYPE__RAW__SELECTED_VARIABLES_PRIMARY;
		}
		else
		{
			columns_in_variable_group_view.schema_type = NewGeneSchema::SCHEMA_TYPE__RAW__SELECTED_VARIABLES_TOP_LEVEL_NOT_PRIMARY;
		}
	}
	else
	{
		columns_in_variable_group_view.schema_type = NewGeneSchema::SCHEMA_TYPE__RAW__SELECTED_VARIABLES_CHILD;
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

	// Check if this is the *primary* top-level variable group.
	// If so, now that we are loading it, this is the first time
	// we know about or care about the sequence of primary key fields
	// in the output.
	// It is time to set this information in the global "sequence" metadata object.
	if (primary_or_secondary_view_index == primary_vg_index__in__top_level_vg_vector)
	{

		int primary_vg_output_column_sequence = 0;

		// First, the keys of multiplicity 1
		// ... which always appear first in the output
		for (auto const & variable_group_set_member : variables_in_group)
		{

			int count_of_this_raw_variable_column_in_final_output = 0;

			for (auto & primary_key_entry__test_sequence : sequence.primary_key_sequence_info)
			{
				if (*variable_group_set_member.code ==
					primary_key_entry__test_sequence.variable_group_info_for_primary_keys__top_level_and_child[primary_vg_index__in__top_level_vg_vector].column_name_no_uuid)
				{
					++count_of_this_raw_variable_column_in_final_output;
				}
			}

			if (count_of_this_raw_variable_column_in_final_output == 1)
			{

				int main_sequence_test = 0;

				for (auto & primary_key_entry__output__including_multiplicities : sequence.primary_key_sequence_info)
				{
					if (*variable_group_set_member.code ==
						primary_key_entry__output__including_multiplicities.variable_group_info_for_primary_keys__top_level_and_child[primary_vg_index__in__top_level_vg_vector].column_name_no_uuid)
					{
						primary_key_entry__output__including_multiplicities.sequence_number_in_all_primary_keys__of__order_columns_appear_in_top_level_vg = primary_vg_output_column_sequence;
						++primary_vg_output_column_sequence;
					}
				}

			}

		}

		// Next, the keys of multiplicity > 1
		// ... which always appear later in the output
		int multiplicity_count_test = 0;

		for (auto const & variable_group_set_member : variables_in_group)
		{

			int count_of_this_raw_variable_column_in_final_output = 0;

			for (auto & primary_key_entry__test_sequence : sequence.primary_key_sequence_info)
			{
				if (*variable_group_set_member.code ==
					primary_key_entry__test_sequence.variable_group_info_for_primary_keys__top_level_and_child[primary_vg_index__in__top_level_vg_vector].column_name_no_uuid)
				{
					++count_of_this_raw_variable_column_in_final_output;
				}
			}

			if (count_of_this_raw_variable_column_in_final_output > 1)
			{
				if (multiplicity_count_test > 0)
				{
					if (multiplicity_count_test != count_of_this_raw_variable_column_in_final_output)
					{
						boost::format msg("Logic error: Every DMU column in the DMU with multiplicity greater than 1 must have the same multiplicity.");
						throw NewGeneException() << newgene_error_description(msg.str());
					}
				}

				multiplicity_count_test = count_of_this_raw_variable_column_in_final_output;
			}

		}

		for (int current_multiplicity = 0; current_multiplicity < multiplicity_count_test; ++current_multiplicity)
		{

			for (auto const & variable_group_set_member : variables_in_group)
			{

				int count_of_this_raw_variable_column_in_final_output = 0;

				for (auto & primary_key_entry__test_sequence : sequence.primary_key_sequence_info)
				{
					if (*variable_group_set_member.code ==
						primary_key_entry__test_sequence.variable_group_info_for_primary_keys__top_level_and_child[primary_vg_index__in__top_level_vg_vector].column_name_no_uuid)
					{
						++count_of_this_raw_variable_column_in_final_output;
					}
				}

				bool has_multiplicity_greater_than_1 = count_of_this_raw_variable_column_in_final_output > 1;

				if (has_multiplicity_greater_than_1)
				{
					int test_multiplicity = 0;

					for (auto & primary_key_entry__output__including_multiplicities : sequence.primary_key_sequence_info)
					{
						if (*variable_group_set_member.code ==
							primary_key_entry__output__including_multiplicities.variable_group_info_for_primary_keys__top_level_and_child[primary_vg_index__in__top_level_vg_vector].column_name_no_uuid)
						{
							if (test_multiplicity == current_multiplicity)
							{
								primary_key_entry__output__including_multiplicities.sequence_number_in_all_primary_keys__of__order_columns_appear_in_top_level_vg = primary_vg_output_column_sequence;
								++primary_vg_output_column_sequence;
							}

							++test_multiplicity;
						}
					}
				}

			}

		}

	}

	WidgetInstanceIdentifiers datetime_columns = input_model->t_vgp_data_metadata__datetime_columns.getIdentifiers(vg_data_table_name);

	if (datetime_columns.size() > 0 && datetime_columns.size() != 2)
	{
		boost::format msg("The number of datetime columns in the raw data tables in the database is not either 0 or 2 (table %1%)");
		msg % vg_data_table_name;
		SetFailureErrorMessage(msg.str());
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
		columns_in_variable_group_view.current_block_datetime_column_types = std::make_pair(NewGeneSchema::NewGeneColumnDefinition::COLUMN_TYPE__DATETIMESTART,
				NewGeneSchema::NewGeneColumnDefinition::COLUMN_TYPE__DATETIMEEND);
		columns_in_variable_group_view.previous_block_datetime_column_types = columns_in_variable_group_view.current_block_datetime_column_types;
	}



	std::for_each(variables_in_group_sorted.cbegin(),
				  variables_in_group_sorted.cend(), [this, &vg_data_table_name, &dmu_counts_corresponding_to_top_level_uoa,
						  &dmu_counts_corresponding_to_uoa_for_current_primary_or_child_variable_group, &is_primary, &columns_in_variable_group_view, &datetime_columns, &the_variable_group,
						  &variables_in_group_primary_keys_metadata](
					  WidgetInstanceIdentifier const & variable_group_set_member)
	{
		columns_in_variable_group_view.column_definitions.push_back(NewGeneSchema::NewGeneColumnDefinition());
		NewGeneSchema::NewGeneColumnDefinition & column_in_variable_group_data_table = columns_in_variable_group_view.column_definitions.back();

		std::string column_name_no_uuid = *variable_group_set_member.code;
		column_in_variable_group_data_table.column_name_in_temporary_table_no_uuid = column_name_no_uuid;

		std::string column_name = column_name_no_uuid;

		column_in_variable_group_data_table.column_name_in_temporary_table = column_name;

		column_in_variable_group_data_table.column_name_in_original_data_table = column_name_no_uuid;

		column_in_variable_group_data_table.current_variable_group = the_variable_group.first;

		column_in_variable_group_data_table.current_multiplicity__of__this_column__in__output__same_as__current_multiplicity__of___this_column__in_its_own_variable_group = 1;

		if (!the_variable_group.first.identifier_parent)
		{
			boost::format msg("There is no unit of analysis that can be retrieved for table %1% while attempting to retrieve raw data from this table.");
			msg % vg_data_table_name;
			SetFailureErrorMessage(msg.str());
			failed = true;
			return;
		}

		column_in_variable_group_data_table.uoa_associated_with_current_variable_group = *the_variable_group.first.identifier_parent;

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
			column_in_variable_group_data_table.column_type = NewGeneSchema::NewGeneColumnDefinition::COLUMN_TYPE__PRIMARY;
		}
		else
		{
			column_in_variable_group_data_table.column_type = NewGeneSchema::NewGeneColumnDefinition::COLUMN_TYPE__SECONDARY;
		}

		if (datetime_columns.size())
		{
			if (datetime_columns[0].code && variable_group_set_member.code && *datetime_columns[0].code == *variable_group_set_member.code)
			{
				// The current column is the datetime_start column
				column_in_variable_group_data_table.column_type = NewGeneSchema::NewGeneColumnDefinition::COLUMN_TYPE__DATETIMESTART;
			}
			else if (datetime_columns[1].code && variable_group_set_member.code && *datetime_columns[1].code == *variable_group_set_member.code)
			{
				// The current column is the datetime_end column
				column_in_variable_group_data_table.column_type = NewGeneSchema::NewGeneColumnDefinition::COLUMN_TYPE__DATETIMEEND;
			}
		}


		// Populate primary key column data, for those columns that are primary keys.
		// The logic is a bit tricky, but works... See comments above, and below, for context.

		std::for_each(sequence.primary_key_sequence_info.cbegin(),
					  sequence.primary_key_sequence_info.cend(), [this, &dmu_counts_corresponding_to_top_level_uoa,
							  &dmu_counts_corresponding_to_uoa_for_current_primary_or_child_variable_group, &the_variable_group, &column_in_variable_group_data_table, &variables_in_group_primary_keys_metadata](
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
			std::for_each(primary_key_entry__output__including_multiplicities.variable_group_info_for_primary_keys__top_level_and_child.cbegin(),
						  primary_key_entry__output__including_multiplicities.variable_group_info_for_primary_keys__top_level_and_child.cend(), [this,
								  &k_count__corresponding_to_top_level_uoa__and_current_dmu_category, &dmu_counts_corresponding_to_top_level_uoa,
								  &dmu_counts_corresponding_to_uoa_for_current_primary_or_child_variable_group, &the_variable_group, &column_in_variable_group_data_table,
								  &primary_key_entry__output__including_multiplicities, &variables_in_group_primary_keys_metadata](
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

								column_in_variable_group_data_table.primary_key_index__within_uoa_corresponding_to_current_variable_group =
									current_variable_group_primary_key_entry.sequence_number_within_dmu_category_for_this_variable_groups_uoa;

								column_in_variable_group_data_table.primary_key_index_within_primary_uoa_for_dmu_category =
									primary_key_entry__output__including_multiplicities.sequence_number_within_dmu_category_primary_uoa;

								column_in_variable_group_data_table.total_outer_multiplicity__in_total_kad__for_current_dmu_category__for_current_variable_group =
									current_variable_group_primary_key_entry.total_outer_multiplicity__in_total_kad__for_current_dmu_category__for_current_variable_group;

								column_in_variable_group_data_table.total_k_count__within_uoa_corresponding_to_top_level_variable_group__for_current_dmu_category =
									k_count__corresponding_to_top_level_uoa__and_current_dmu_category;

								std::for_each(dmu_counts_corresponding_to_uoa_for_current_primary_or_child_variable_group.cbegin(),
											  dmu_counts_corresponding_to_uoa_for_current_primary_or_child_variable_group.cend(), [this, &column_in_variable_group_data_table,
													  &primary_key_entry__output__including_multiplicities](
												  Table_UOA_Identifier::DMU_Plus_Count const & dmu_count)
								{

									if (dmu_count.first.IsEqual(WidgetInstanceIdentifier::EQUALITY_CHECK_TYPE__STRING_CODE, primary_key_entry__output__including_multiplicities.dmu_category))
									{

										column_in_variable_group_data_table.total_k_count__within_uoa_corresponding_to_current_variable_group__for_current_dmu_category = dmu_count.second;

										WidgetInstanceIdentifier_Int_Pair Kad_Data = model->t_kad_count.getIdentifier(*primary_key_entry__output__including_multiplicities.dmu_category.uuid);
										column_in_variable_group_data_table.total_k_spin_count_across_multiplicities_for_dmu_category = Kad_Data.second;

									}

								});

								// Now determine if this primary key field should be treated as numeric for sorting and ordering
								std::for_each(variables_in_group_primary_keys_metadata.cbegin(),
											  variables_in_group_primary_keys_metadata.cend(), [&column_in_variable_group_data_table, &primary_key_entry__output__including_multiplicities,
													  &current_variable_group_primary_key_entry](
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

	});

}

void OutputModel::OutputGenerator::PopulateDMUCounts()
{

	// Make sure the UOA's with the highest number of DMU's appear first
	// ... so that the first UOA evaluated in the following function is a legitimate top-level UOA

	// VS bug:
	// Must wrap lambda in a std::function.
	// See http://connect.microsoft.com/VisualStudio/feedback/details/727957/vc11-beta-compiler-fails-to-compile-lambda-key-comparer-for-maps-and-sets
	auto dmu_count_comparator =
		std::function<bool(std::pair<WidgetInstanceIdentifier, Table_UOA_Identifier::DMU_Counts> const &, std::pair<WidgetInstanceIdentifier, Table_UOA_Identifier::DMU_Counts> const &)>([](
					std::pair<WidgetInstanceIdentifier, Table_UOA_Identifier::DMU_Counts> const & lhs, std::pair<WidgetInstanceIdentifier, Table_UOA_Identifier::DMU_Counts> const & rhs) -> bool
	{
		// Customized comparison that simply counts the number of DMU's
		if (lhs.second.size() < rhs.second.size())
		{
			return true;
		}
		else if (lhs.second.size() > rhs.second.size())
		{
			return false;
		}
		// Same size.  So, to make sure all elements are considered distinct, do a text comparison of the UOA name.
		// The order does not matter.
		return *lhs.first.uuid < *rhs.first.uuid;

	});

	std::set<std::pair<WidgetInstanceIdentifier, Table_UOA_Identifier::DMU_Counts>, decltype(dmu_count_comparator)> UOASet(dmu_count_comparator);

	UOASet.insert(UOAs.cbegin(), UOAs.cend());

	bool first = true;
	std::for_each(UOASet.crbegin(), UOASet.crend(), [this, &first](std::pair<WidgetInstanceIdentifier, Table_UOA_Identifier::DMU_Counts> const & uoa__to__dmu_counts__pair)
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

		std::vector<WidgetInstanceIdentifier> erroneousDmus; // for error reporting when UOA's overlap

		int current = 0;
		std::for_each(current_dmu_counts.cbegin(), current_dmu_counts.cend(), [this, &current_is_bigger, &current_is_smaller, &current_is_same, &current, &erroneousDmus](
						  Table_UOA_Identifier::DMU_Plus_Count const & current_dmu_plus_count)
		{
			bool matched_current_dmu = false;
			// Looking at the first entry in biggest_counts is the same as looking at any other entry
			// in terms of the DMU counts
			std::for_each(biggest_counts[0].second.cbegin(),
						  biggest_counts[0].second.cend(), [&matched_current_dmu, &current_dmu_plus_count, &current_is_bigger, &current_is_smaller, &current_is_same, &current, &erroneousDmus](
							  Table_UOA_Identifier::DMU_Plus_Count const & k_count_for_primary_uoa_for_given_dmu_category__info)
			{
				if (current_dmu_plus_count.first.IsEqual(WidgetInstanceIdentifier::EQUALITY_CHECK_TYPE__STRING_CODE, k_count_for_primary_uoa_for_given_dmu_category__info.first))
				{
					matched_current_dmu = true;

					if (current_dmu_plus_count.second > k_count_for_primary_uoa_for_given_dmu_category__info.second)
					{
						erroneousDmus.push_back(current_dmu_plus_count.first);
						current_is_same = false;
						current_is_bigger = true;
					}
					else if (current_dmu_plus_count.second < k_count_for_primary_uoa_for_given_dmu_category__info.second)
					{
						erroneousDmus.push_back(k_count_for_primary_uoa_for_given_dmu_category__info.first);
						current_is_same = false;
						current_is_smaller = true;
					}
				}
			});

			if (!matched_current_dmu)
			{
				// The DMU in the current UOA being tested does not exist in the "biggest" UOA previous to this
				erroneousDmus.push_back(current_dmu_plus_count.first);
				current_is_same = false;
				current_is_bigger = true;
			}

			++current;
		});

		// Looking at the first entry in biggest_counts is the same as looking at any other entry
		// in terms of the DMU counts
		std::for_each(biggest_counts[0].second.cbegin(), biggest_counts[0].second.cend(), [&current_dmu_counts, &current_is_bigger, &current_is_smaller, &current_is_same, &erroneousDmus](
						  Table_UOA_Identifier::DMU_Plus_Count const & k_count_for_primary_uoa_for_given_dmu_category__info)
		{
			bool matched_biggest_dmu = false;
			std::for_each(current_dmu_counts.cbegin(),
						  current_dmu_counts.cend(), [&matched_biggest_dmu, &k_count_for_primary_uoa_for_given_dmu_category__info, &current_is_bigger, &current_is_smaller, &current_is_same, &erroneousDmus](
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
				erroneousDmus.push_back(k_count_for_primary_uoa_for_given_dmu_category__info.first);
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
				SetFailureErrorMessage(msg.str());
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
			std::string erroneousDmuList {"("};
			int countErroneous = erroneousDmus.size();
			int currentErroneous = 0;
			std::for_each(erroneousDmus.cbegin(), erroneousDmus.cend(), [&](WidgetInstanceIdentifier const & erroneousDmu)
			{
				++currentErroneous;

				if (currentErroneous > 1 && currentErroneous < countErroneous)
				{
					erroneousDmuList += ", ";
				}
				else if (currentErroneous == countErroneous && countErroneous > 1)
				{
					erroneousDmuList += " and ";
				}

				erroneousDmuList += *erroneousDmu.code;
			});
			erroneousDmuList += ")";
			boost::format
			msg("There are unrelated DMUs in the units of analysis for the variable groups you've selected %1%!  There must be a full relationship between the variable groups you've selected.  This means that at least one variable group must contain the full set of all DMUs, taken as a whole, for all variable groups you've selected.");
			msg % erroneousDmuList;
			SetFailureErrorMessage(msg.str());
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
			SetFailureErrorMessage(msg.str());
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
			boost::format msg("DMU code (or NewGeneUUID) is unknown while validating units of analysis.");
			SetFailureErrorMessage(msg.str());
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
			// User's k-ad selection is too small in this DMU category to support the variables they have selected
			if (the_dmu_category.longhand)
			{
				if (biggest_counts[0].first.longhand)
				{
					boost::format
					msg("The choice of K in the spin control for DMU %1% (%2%) (%3%) is too small to support the unit/s of analysis for the variables selected, with required minimum K-value %4% for unit of analysis \"%5%\".");
					msg % *the_dmu_category.code % *the_dmu_category.longhand % kad_count_current_dmu_category % uoa_count_current_dmu_category % *biggest_counts[0].first.longhand;
					SetFailureErrorMessage(msg.str());
				}
				else
				{
					boost::format
					msg("The choice of K in the spin control for DMU %1% (%2%) (%3%) is too small to support the unit/s of analysis for the variables selected, with required minimum K-value %4% for unit of analysis %5%.");
					msg % *the_dmu_category.code % *the_dmu_category.longhand % kad_count_current_dmu_category % uoa_count_current_dmu_category % *biggest_counts[0].first.code;
					SetFailureErrorMessage(msg.str());
				}
			}
			else
			{
				if (biggest_counts[0].first.longhand)
				{
					boost::format
					msg("The choice of K in the spin control for DMU %1% (%2%) is too small to support the unit/s of analysis for the variables selected, with required minimum K-value %3% for unit of analysis \"%4%\".");
					msg % *the_dmu_category.code % kad_count_current_dmu_category % uoa_count_current_dmu_category % *biggest_counts[0].first.longhand;
					SetFailureErrorMessage(msg.str());
				}
				else
				{
					boost::format
					msg("The choice of K in the spin control for DMU %1% (%2%) is too small to support the unit/s of analysis for the variables selected, with required minimum K-value %3% for unit of analysis %4%.");
					msg % *the_dmu_category.code % kad_count_current_dmu_category % uoa_count_current_dmu_category % *biggest_counts[0].first.code;
					SetFailureErrorMessage(msg.str());
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
					SetFailureErrorMessage(msg.str());
				}
				else
				{
					boost::format msg("The choice of K in the spin control for DMU %1% (%2%) (%3%) is not an even multiple of the minimum K-value for the unit of analysis %4% (%5%).");
					msg % *the_dmu_category.code % *the_dmu_category.longhand % kad_count_current_dmu_category % *biggest_counts[0].first.code % uoa_count_current_dmu_category;
					SetFailureErrorMessage(msg.str());
				}
			}
			else
			{
				if (biggest_counts[0].first.longhand)
				{
					boost::format msg("The choice of K in the spin control for DMU %1% (%2%) is not an even multiple of the minimum K-value for the unit of analysis %3% (%4%).");
					msg % *the_dmu_category.code % kad_count_current_dmu_category % *biggest_counts[0].first.longhand % uoa_count_current_dmu_category;
					SetFailureErrorMessage(msg.str());
				}
				else
				{
					boost::format msg("The choice of K in the spin control for DMU %1% (%2%) is not an even multiple of the minimum K-value for the unit of analysis %3% (%4%).");
					msg % *the_dmu_category.code % kad_count_current_dmu_category % *biggest_counts[0].first.code % uoa_count_current_dmu_category;
					SetFailureErrorMessage(msg.str());
				}
			}

			failed = true;
			return;
		}

		outer_multiplicities_primary_uoa___ie___if_there_are_3_cols_for_a_single_dmu_in_the_primary_uoa__and_K_is_12__then__this_value_is_4_for_that_DMU____note_this_is_greater_than_1_for_only_1_DMU_in_the_primary_UOA.push_back(
			multiplicity);

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
	std::for_each(
		outer_multiplicities_primary_uoa___ie___if_there_are_3_cols_for_a_single_dmu_in_the_primary_uoa__and_K_is_12__then__this_value_is_4_for_that_DMU____note_this_is_greater_than_1_for_only_1_DMU_in_the_primary_UOA.cbegin(),
		outer_multiplicities_primary_uoa___ie___if_there_are_3_cols_for_a_single_dmu_in_the_primary_uoa__and_K_is_12__then__this_value_is_4_for_that_DMU____note_this_is_greater_than_1_for_only_1_DMU_in_the_primary_UOA.cend(), [this,
				&current_index](
			int const & test_multiplicity)
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
				std::string theMsg("Only a single DMU category may have its K value in the spin control set to larger than that DMU category's minimum (e.g. ");
				Table_UOA_Identifier::DMU_Counts const & counts = biggest_counts[0].second;

				bool first = true;

				for (auto const & count : counts)
				{
					if (!first)
					{
						theMsg += "; ";
					}

					WidgetInstanceIdentifier const & id = count.first;
					int const thisCount = count.second;
					std::string idText = Table_DMU_Identifier::GetDmuCategoryDisplayText(id);
					theMsg += idText;
					theMsg += " has a minimum K of ";
					theMsg += std::to_string(thisCount);
					first = false;
				}

				theMsg += ").";

				boost::format msg(theMsg);
				SetFailureErrorMessage(msg.str());
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
	// is either 0, 1, or the corresponding UOA k-value of the primary UOA;
	//
	// and that it is 1 only if it is also 1 in the primary group for that DMU
	// or if the primary group has outer multiplicity > 1 in that DMU
	//
	// A way to visualize this restriction is the following:
	//
	// PRIMARY:
	// Assume 3 DMU columns of DMU category A in the UOA,
	// 4 DMU columns of DMU category B in the UOA,
	// and 3 DMU columns of DMU category C in the UOA.
	// Also assume that the outer multiplicity of DMU category C is equal to 4
	// (i.e., the spin control is set to 12 for DMU category C).
	// (Note that only 1 DMU category can have outer multiplicity greater than 1
	// for the primary group, as well as for child groups.)
	// A A A   B B B B   C C C   C C C   C C C   C C C
	//
	// The child group MUST have either 0 or an identical number of DMU columns for A, and B.
	// But for C, it can have 0, 1, or 3.
	// Child:
	// A A A   B B B B
	// A A A   B B B B   C
	// A A A   B B B B   C C C
	// ... are all acceptable.
	//
	// Also fine: The primary has multiplicity 1
	// Primary:
	// A A A   B B B B   C C C
	//
	// ... and the child has K=1 for any of these
	// Child:
	// A       B B B B   C C C
	// A A A   B         C C C
	// A A A   B B B B   C
	std::for_each(child_counts.cbegin(), child_counts.cend(), [this](std::pair<WidgetInstanceIdentifier, Table_UOA_Identifier::DMU_Counts> const & uoa__to__dmu_counts__pair)
	{
		if (failed || CheckCancelled())
		{
			return; // from lambda
		}

		int primary_dmu_categories_for_which_child_has_less = 0;
		Table_UOA_Identifier::DMU_Counts const & current_child_dmu_counts = uoa__to__dmu_counts__pair.second;
		std::for_each(current_child_dmu_counts.cbegin(), current_child_dmu_counts.cend(), [this, &primary_dmu_categories_for_which_child_has_less](
						  Table_UOA_Identifier::DMU_Plus_Count const & current_child_dmu_plus_count)
		{
			if (failed || CheckCancelled())
			{
				return; // from lambda
			}

			if (current_child_dmu_plus_count.second == 0)
			{
				// just fine
				return; // from lambda
			}

			std::for_each(biggest_counts[0].second.cbegin(), biggest_counts[0].second.cend(), [this, &current_child_dmu_plus_count, &primary_dmu_categories_for_which_child_has_less](
							  Table_UOA_Identifier::DMU_Plus_Count const & k_count_for_primary_uoa_for_given_dmu_category__info)
			{
				if (failed || CheckCancelled())
				{
					return; // from lambda
				}

				if (current_child_dmu_plus_count.first.IsEqual(WidgetInstanceIdentifier::EQUALITY_CHECK_TYPE__STRING_CODE, k_count_for_primary_uoa_for_given_dmu_category__info.first))
				{

					if (current_child_dmu_plus_count.second == k_count_for_primary_uoa_for_given_dmu_category__info.second)
					{

						// The number of DMU columns for this DMU category is the same for both child and primary group

						if (boost::iequals(*current_child_dmu_plus_count.first.code, highest_multiplicity_primary_uoa_dmu_string_code))
						{
							if (highest_multiplicity_primary_uoa > 1)
							{
								// ... And this DMU category has multiplicity selected by the user in the K-spin-control
								// of greater than 1
								// (i.e., the K-value in the spin control for this DMU category
								// is higher than the number of DMU columns in this DMU category
								// (for both child and primary))

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

					if (current_child_dmu_plus_count.second > 1)
					{

						// The number of child DMU columns for this DMU category is greater than 1,
						// but not equal to the number of DMU columns in the primary for this DMU category

						// Invalid child UOA for this output
						if (current_child_dmu_plus_count.first.longhand)
						{
							boost::format
							msg("For child variable groups, the K-value within the unit of analysis for DMU category %1% (%2%) may only be greater than 1 if it matches the K-value within the UOA of the top-level variable group/s.");
							msg % *current_child_dmu_plus_count.first.code % *current_child_dmu_plus_count.first.longhand;
							SetFailureErrorMessage(msg.str());
						}
						else
						{
							boost::format
							msg("For child variable groups, the K-value within the unit of analysis for DMU category %1% may only be greater than 1 if it matches the K-value within the UOA of the top-level variable group/s.");
							msg % *current_child_dmu_plus_count.first.code;
							SetFailureErrorMessage(msg.str());
						}

						failed = true;
						return; // from lambda

					}

					// Redundant 0-check for future code foolproofing
					else if (current_child_dmu_plus_count.second == 0)
					{
						// just fine
						return; // from lambda
					}

					// Current UOA's current DMU category has 1 column and the same DMU category in the primary group has more than one column
					// (due to the first "if" block above and the fact that the child cannot have more columns than the primary),
					// so if this is NOT the DMU category with multiplicity > 1 (implying that there *IS* a DMU category
					// with multiplicity greater than 1), then we exclude it.
					//
					// If the following block is not reached, it's because the primary UOA has a multiplicity of 1 (i.e., it's all branch);
					// and therefore this is an acceptable condition - the child has 1 column in this DMU, and the primary has more than 1
					// (but a primary multiplicity of 1),
					// so drop through.
					else if (! boost::iequals(*current_child_dmu_plus_count.first.code, highest_multiplicity_primary_uoa_dmu_string_code))
					{

						// ... But the current DMU category is not the one with multiplicity greater than 1

						// The following check is redundant, because the variable holding the string code for DMU with multiplicity > 1 (above)
						// will be an empty string if there isn't a DMU with multiplicity > 1
						if (highest_multiplicity_primary_uoa > 1)
						{

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
							if (current_child_dmu_plus_count.first.longhand)
							{
								boost::format
								msg("For child variable groups, the K-value within the unit of analysis for DMU category %1% (%2%) cannot currently be 1 if the K-value within the UOA of the top-level variable group/s is greater than 1 when that DMU category does not have multiplicity greater than 1 for the top-level variable group.");
								msg % *current_child_dmu_plus_count.first.code % *current_child_dmu_plus_count.first.longhand;
								SetFailureErrorMessage(msg.str());
							}
							else
							{
								boost::format
								msg("For child variable groups, the K-value within the unit of analysis for DMU category %1% cannot currently be 1 if the K-value within the UOA of the top-level variable group/s is greater than 1 when that DMU category does not have multiplicity greater than 1 for the top-level variable group.");
								msg % *current_child_dmu_plus_count.first.code;
								SetFailureErrorMessage(msg.str());
							}

							failed = true;
							return; // from lambda

						}

					}

					// The number of DMU columns for the child in this DMU category
					// is not the same as the number of DMU columns for the primary,
					// because of the first "if" block, above.
					//
					// And, the number of DMU columns for the child
					// is not greater than 1, and it's not 0,
					// due to the next two "if" blocks.
					//
					// So the number of DMU columns for the child must be 1.
					// Given that the number of DMU columns for the child
					// is not equal to the number of DMU columns for the primary
					// (see first paragraph in this comment),
					// the number in the primary is either 0, or greater than 1.
					// Due to the logic in PopulateDMUCounts(),
					// this child group would not have been defined as a child group
					// if it had a larger number of DMU columns than the primary
					// for any of its DMU categories.
					// Therefore it must have less DMU columns than the primary
					// for this DMU category (given that it's not equal).
					//
					// Given the previous sentence, we can safely increment the following variable.
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
			SetFailureErrorMessage(msg.str());
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
			boost::format msg("Unit of analysis %1% does not have a NewGeneUUID.  (Error while populating metadata for units of analysis.)");
			msg % *uoa.code;
			SetFailureErrorMessage(msg.str());
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
				SetFailureErrorMessage(msg.str());
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
				// User's k-ad selection is too small in this DMU category to support the variables they have selected
				// Todo: Error message
				if (the_dmu_category_identifier.longhand)
				{
					if (uoa_identifier.longhand)
					{
						boost::format
						msg("The choice of K in the spin control for DMU %1% (%2%) (%3%) is too small to support the unit/s of analysis for the variables selected, with required minimum K-value %4% for unit of analysis \"%5%\".");
						msg % *the_dmu_category_identifier.code % *the_dmu_category_identifier.longhand % k_spin_control_count__current_dmu_category % uoa_k_count__current_dmu_category %
						*uoa_identifier.longhand;
						SetFailureErrorMessage(msg.str());
					}
					else
					{
						boost::format
						msg("The choice of K in the spin control for DMU %1% (%2%) (%3%) is too small to support the unit/s of analysis for the variables selected, with required minimum K-value %4% for unit of analysis %5%.");
						msg % *the_dmu_category_identifier.code % *the_dmu_category_identifier.longhand % k_spin_control_count__current_dmu_category % uoa_k_count__current_dmu_category %
						*uoa_identifier.code;
						SetFailureErrorMessage(msg.str());
					}
				}
				else
				{
					if (uoa_identifier.longhand)
					{
						boost::format
						msg("The choice of K in the spin control for DMU %1% (%2%) is too small to support the unit/s of analysis for the variables selected, with required minimum K-value %3% for unit of analysis \"%4%\".");
						msg % *the_dmu_category_identifier.code % k_spin_control_count__current_dmu_category % uoa_k_count__current_dmu_category % *uoa_identifier.longhand;
						SetFailureErrorMessage(msg.str());
					}
					else
					{
						boost::format
						msg("The choice of K in the spin control for DMU %1% (%2%) is too small to support the unit/s of analysis for the variables selected, with required minimum K-value %3% for unit of analysis %4%.");
						msg % *the_dmu_category_identifier.code % k_spin_control_count__current_dmu_category % uoa_k_count__current_dmu_category % *uoa_identifier.code;
						SetFailureErrorMessage(msg.str());
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
		top_level_variable_groups_vector.insert(top_level_variable_groups_vector.end(), variable_groups_map_current.cbegin(), variable_groups_map_current.cend());
	});
	std::for_each(child_counts.cbegin(), child_counts.cend(), [this](std::pair<WidgetInstanceIdentifier, Table_UOA_Identifier::DMU_Counts> const & uoa__to__dmu_category_counts)
	{
		Table_VARIABLES_SELECTED::VariableGroup_To_VariableSelections_Map const & variable_groups_map_current = (*the_map)[uoa__to__dmu_category_counts.first];
		child_variable_groups_vector.insert(child_variable_groups_vector.end(), variable_groups_map_current.cbegin(), variable_groups_map_current.cend());
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
			SetFailureErrorMessage(msg.str());
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
			std::vector<PrimaryKeySequence::VariableGroup_PrimaryKey_Info> & variable_group_info_for_primary_keys =
				current_primary_key_sequence.variable_group_info_for_primary_keys__top_level_and_child;

			current_primary_key_sequence.dmu_category = the_dmu_category;
			current_primary_key_sequence.sequence_number_within_dmu_category_spin_control = k_sequence_number_count_for_given_dmu_category_out_of_total_spin_count_for_that_dmu_category;
			current_primary_key_sequence.sequence_number_within_dmu_category_primary_uoa = k_sequence_number_count_for_given_dmu_category_out_of_k_count_for_primary_uoa_for_that_dmu_category;
			current_primary_key_sequence.sequence_number_in_all_primary_keys__of__global_primary_key_sequence_metadata__NOT__of_order_columns_appear_in_top_level_vg =
				overall_primary_key_sequence_number;
			current_primary_key_sequence.sequence_number_in_all_primary_keys__of__order_columns_appear_in_top_level_vg =
				-1; // We do not know this yet.  We will fill this in when we populate the schema for the primary top-level variable group.
			current_primary_key_sequence.total_k_count_within_high_level_variable_group_uoa_for_this_dmu_category = k_count_for_primary_uoa_for_given_dmu_category;
			current_primary_key_sequence.total_kad_spin_count_for_this_dmu_category = total_spin_control_k_count_for_given_dmu_category;

			std::map<WidgetInstanceIdentifier, int> map__dmu_category__total_outer_multiplicity_of_dmu_category_in_primary_uoa_corresponding_to_top_level_variable_group;

			int view_count = 0;
			std::for_each(top_level_variable_groups_vector.cbegin(),
						  top_level_variable_groups_vector.cend(), [this, &the_dmu_category, &k_sequence_number_count_for_given_dmu_category_out_of_total_spin_count_for_that_dmu_category,
								  &k_count_for_primary_uoa_for_given_dmu_category, &total_spin_control_k_count_for_given_dmu_category, &current_primary_key_sequence, &variable_group_info_for_primary_keys,
								  &map__dmu_category__total_outer_multiplicity_of_dmu_category_in_primary_uoa_corresponding_to_top_level_variable_group](
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
					SetFailureErrorMessage(msg.str());
					failed = true;
					return;
				}

				if (!the_variable_group.first.identifier_parent)
				{
					// Todo: error message
					boost::format msg("Unknown unit of analysis identifier while populating primary key sequence metadata.");
					SetFailureErrorMessage(msg.str());
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
					SetFailureErrorMessage(msg.str());
					failed = true;
					return;
				}

				int total_inner_number_columns_in_given_dmu_category_for_uoa_corresponding_to_current_variable_group = 0;
				std::for_each(dmu_category_metadata__for_current_primary_or_child_uoa.cbegin(),
							  dmu_category_metadata__for_current_primary_or_child_uoa.cend(), [this, the_dmu_category,
									  &total_inner_number_columns_in_given_dmu_category_for_uoa_corresponding_to_current_variable_group,
									  &k_sequence_number_count_for_given_dmu_category_out_of_total_spin_count_for_that_dmu_category, &k_count_for_primary_uoa_for_given_dmu_category,
									  &total_spin_control_k_count_for_given_dmu_category, &current_primary_key_sequence, &variable_group_info_for_primary_keys](
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
								  dmu_category_metadata__for_current_primary_or_child_uoa.cend(), [this, &m,
										  &outer_multiplicity__within_current_dmu_category__compared_to_total_spin_count__for_that_dmu_category__for_the_current_primary_or_child_uoa,
										  &current_variable_group_current_primary_key_info, &the_variable_group, &outer_sequence_number__current_variable_group__current_primary_key_dmu_category, &the_dmu_category,
										  &inner_sequence_number__current_variable_group__current_primary_key_dmu_category, &k_sequence_number_count_for_given_dmu_category_out_of_total_spin_count_for_that_dmu_category,
										  &k_count_for_primary_uoa_for_given_dmu_category, &total_spin_control_k_count_for_given_dmu_category, &current_primary_key_sequence, &variable_group_info_for_primary_keys](
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
									SetFailureErrorMessage(msg.str());
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
			std::for_each(child_variable_groups_vector.cbegin(),
						  child_variable_groups_vector.cend(), [this, &the_dmu_category, &k_sequence_number_count_for_given_dmu_category_out_of_total_spin_count_for_that_dmu_category,
								  &k_count_for_primary_uoa_for_given_dmu_category, &total_spin_control_k_count_for_given_dmu_category, &current_primary_key_sequence, &variable_group_info_for_primary_keys,
								  &map__dmu_category__total_outer_multiplicity_of_dmu_category_in_primary_uoa_corresponding_to_top_level_variable_group](
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
					SetFailureErrorMessage(msg.str());
					failed = true;
					return;
				}

				if (!the_variable_group.first.identifier_parent)
				{
					// Todo: error message
					boost::format msg("Unknown unit of analysis identifier while populating primary key sequence metadata.");
					SetFailureErrorMessage(msg.str());
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
					SetFailureErrorMessage(msg.str());
					failed = true;
					return;
				}

				int total_number_columns_in_given_dmu_category_for_uoa_corresponding_to_current_variable_group = 0;
				std::for_each(dmu_category_metadata__for_current_primary_or_child_uoa.cbegin(),
							  dmu_category_metadata__for_current_primary_or_child_uoa.cend(), [this, the_dmu_category,
									  &total_number_columns_in_given_dmu_category_for_uoa_corresponding_to_current_variable_group,
									  &k_sequence_number_count_for_given_dmu_category_out_of_total_spin_count_for_that_dmu_category, &k_count_for_primary_uoa_for_given_dmu_category,
									  &total_spin_control_k_count_for_given_dmu_category, &current_primary_key_sequence, &variable_group_info_for_primary_keys](
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
				// out of the full set of primary keys, including the full k-ad spin counts
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
								  dmu_category_metadata__for_current_primary_or_child_uoa.cend(), [this, &m,
										  &outer_multiplicity__within_current_dmu_category__compared_to_total_spin_count__for_that_dmu_category__for_the_current_primary_or_child_uoa,
										  &current_variable_group_current_primary_key_info, &the_variable_group, &outer_sequence_number__current_variable_group__current_primary_key_dmu_category, &the_dmu_category,
										  &inner_sequence_number__current_variable_group__current_primary_key_dmu_category, &k_sequence_number_count_for_given_dmu_category_out_of_total_spin_count_for_that_dmu_category,
										  &k_count_for_primary_uoa_for_given_dmu_category, &total_spin_control_k_count_for_given_dmu_category, &current_primary_key_sequence, &variable_group_info_for_primary_keys,
										  &total_number_columns_in_given_dmu_category_for_uoa_corresponding_to_current_variable_group](
									  WidgetInstanceIdentifier const & current_variable_group_current_dmu_primary_key)
					{

						if (failed || CheckCancelled())
						{
							return; // from lambda
						}

						if (current_variable_group_current_dmu_primary_key.IsEqual(WidgetInstanceIdentifier::EQUALITY_CHECK_TYPE__STRING_CODE, the_dmu_category))
						{

							// *************************************************************************************************************** //
							// Now see if the TOTAL sequence number of this primary key within the TOTAL k-ad spin control count
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
									SetFailureErrorMessage(msg.str());
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

	overall_total_number_of_primary_key_columns_including_all_branch_columns_and_all_leaves_and_all_columns_internal_to_each_leaf = overall_primary_key_sequence_number;

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

	if (setting_path_to_kad_output == nullptr)
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

void OutputModel::OutputGenerator::SetFailureErrorMessage(std::string const & failure_message_)
{
	failure_message = failure_message_;
	std::string report_failure_message = "Failed: ";
	report_failure_message += failure_message;
	messager.AppendKadStatusText(report_failure_message, this);
	messager.UpdateStatusBarText(report_failure_message, nullptr);
}

void OutputModel::OutputGenerator::KadSampler_ReadData_AddToTimeSlices(NewGeneSchema const & variable_group_selected_columns_schema, int const variable_group_number,
		KadSampler & allWeightings, VARIABLE_GROUP_MERGE_MODE const merge_mode, std::vector<std::string> & errorMessages)
{

	std::int64_t current_rows_stepped = 0;
	sqlite3_stmt *& the_prepared_stmt = SQLExecutor::stmt_insert;
	std::shared_ptr<bool> statement_is_prepared(std::make_shared<bool>(false));
	SQLExecutor::stmt_insert = nullptr;
	sqlite3_stmt *& the_stmt__ = SQLExecutor::stmt_insert;
	boost::format msg_for_progress_bar("Processed %1% of %2% incoming rows");
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

	BOOST_SCOPE_EXIT(this_)
	{
		this_->CloseObtainData();
	} BOOST_SCOPE_EXIT_END

	std::int64_t rowsToRead = ObtainCount(variable_group_selected_columns_schema);

	ObtainData(variable_group_selected_columns_schema, true);

	if (failed || CheckCancelled())
	{
		return;
	}

	SavedRowData sorting_row_of_data;

	ProgressBarMeter meter(messager, std::string("%1% / %2% rows of raw data loaded"), rowsToRead);

	while (StepData())
	{

		if (failed || CheckCancelled())
		{
			break;
		}

		// All data populated into the row corresponds to
		// the VARIABLE GROUP CURRENTLY BEING LOADED
		// (specifically, the "primary key columns" refer to those of the
		// variable group being loaded, NOT the primary variable group).
		//
		// The branch vs. leaf columns are flagged as such,
		// and these refer to the VARIABLE GROUP BEING LOADED,
		// not the primary variable group (unless that's the one being loaded).
		//
		// Note that only 1 leaf is available here, even when
		// (for THIS variable group) the multiplicity is greater than 1
		// (which it has to be in order for the columns to be identified as leaf columns).
		// We have not yet mapped these leaf columns to specific outer multiplicities yet -
		// that will occur only when actual output rows are generated.
		// For now, the leaf data will simply be loaded into a (class-)global map.
		sorting_row_of_data.PopulateFromCurrentRowInDatabase(variable_group_selected_columns_schema, stmt_result, *model);

		failed = sorting_row_of_data.failed;

		if (failed)
		{
			SetFailureErrorMessage(sorting_row_of_data.error_message);
			return;
		}

		if (failed || CheckCancelled()) { break; }

		if (variable_group_selected_columns_schema.has_no_datetime_columns_originally)
		{
			sorting_row_of_data.datetime_start = timerange_start;
			sorting_row_of_data.datetime_end = timerange_end;
		}
		else
		{
			// Data always starts and ends on the proper granularity
			// for any given variable group.
			//
			// Also, timerange_start and timerange_end
			// both currently lie on a valid absolute point for ALL time granularities
			// (currently)
			//
			// If you consider both of the above, you'll see that the following logic works
			if (sorting_row_of_data.datetime_start < timerange_start)
			{
				sorting_row_of_data.datetime_start = timerange_start;
			}

			if (sorting_row_of_data.datetime_end > timerange_end)
			{
				sorting_row_of_data.datetime_end = timerange_end;
			}
		}

		if (sorting_row_of_data.datetime_start >= sorting_row_of_data.datetime_end)
		{
			continue;
		}

		// Construct branch and leaf

		// Construct Leaf
		DMUInstanceDataVector<hits_tag> dmus_leaf__of_uoa_being_merged;

		bool bad = false;
		std::for_each(sorting_row_of_data.indices_of_primary_key_columns_with_outer_multiplicity_greater_than_1.cbegin(),
					  sorting_row_of_data.indices_of_primary_key_columns_with_outer_multiplicity_greater_than_1.cend(), [&](std::pair<SQLExecutor::WHICH_BINDING, std::pair<int, int>> const &
							  binding_info)
		{

			if (bad)
			{
				return;
			}

			if (failed || CheckCancelled()) { return; }

			SQLExecutor::WHICH_BINDING binding = binding_info.first;
			std::pair<int, int> const & indices = binding_info.second;
			int const index_in_bound_vector = indices.first;
			int const column_number = indices.second;

			switch (binding)
			{

				case SQLExecutor::INT64:
					{
						dmus_leaf__of_uoa_being_merged.push_back(static_cast<std::int32_t>(sorting_row_of_data.current_parameter_ints[index_in_bound_vector]));
					}
					break;

				case SQLExecutor::FLOAT:
					{
						dmus_leaf__of_uoa_being_merged.push_back(static_cast<double>(sorting_row_of_data.current_parameter_floats[index_in_bound_vector]));
					}
					break;

				case SQLExecutor::STRING:
					{
						dmus_leaf__of_uoa_being_merged.push_back(sorting_row_of_data.current_parameter_strings[index_in_bound_vector]);
					}
					break;

				default:
					{
						errorMessages.push_back((boost::format("Invalid row binding for row %1%") % boost::lexical_cast<std::string>(current_rows_stepped).c_str()).str());
						bad = true;
						return;
					}
					break;

			}

		});

		// Construct Branch
		DMUInstanceDataVector<hits_tag> dmus_branch__of_uoa_being_merged;

		std::for_each(sorting_row_of_data.indices_of_primary_key_columns_with_outer_multiplicity_equal_to_1.cbegin(),
					  sorting_row_of_data.indices_of_primary_key_columns_with_outer_multiplicity_equal_to_1.cend(), [&](std::pair<SQLExecutor::WHICH_BINDING, std::pair<int, int>> const & binding_info)
		{

			if (bad)
			{
				return;
			}

			if (failed || CheckCancelled()) { return; }

			SQLExecutor::WHICH_BINDING binding = binding_info.first;
			std::pair<int, int> const & indices = binding_info.second;
			int const index_in_bound_vector = indices.first;
			int const column_number = indices.second;

			switch (binding)
			{

				case SQLExecutor::INT64:
					{
						dmus_branch__of_uoa_being_merged.push_back(static_cast<std::int32_t>(sorting_row_of_data.current_parameter_ints[index_in_bound_vector]));
					}
					break;

				case SQLExecutor::FLOAT:
					{
						dmus_branch__of_uoa_being_merged.push_back(static_cast<double>(sorting_row_of_data.current_parameter_floats[index_in_bound_vector]));
					}
					break;

				case SQLExecutor::STRING:
					{
						dmus_branch__of_uoa_being_merged.push_back(sorting_row_of_data.current_parameter_strings[index_in_bound_vector]);
					}
					break;

				default:
					{
						errorMessages.push_back((boost::format("Invalid row binding for row %1%") % boost::lexical_cast<std::string>(current_rows_stepped).c_str()).str());
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
		DMUInstanceDataVector<hits_tag> secondary_data__for_the_current_vg;

		std::for_each(sorting_row_of_data.indices_of_secondary_key_columns.cbegin(),
					  sorting_row_of_data.indices_of_secondary_key_columns.cend(), [&](std::pair<SQLExecutor::WHICH_BINDING, std::pair<int, int>> const & binding_info)
		{

			if (bad)
			{
				return;
			}

			if (failed || CheckCancelled()) { return; }

			SQLExecutor::WHICH_BINDING binding = binding_info.first;
			std::pair<int, int> const & indices = binding_info.second;
			int const index_in_bound_vector = indices.first;
			int const column_number = indices.second;

			switch (binding)
			{

				case SQLExecutor::INT64:
					{
						secondary_data__for_the_current_vg.push_back(static_cast<std::int32_t>(sorting_row_of_data.current_parameter_ints[index_in_bound_vector]));
					}
					break;

				case SQLExecutor::FLOAT:
					{
						secondary_data__for_the_current_vg.push_back(static_cast<double>(sorting_row_of_data.current_parameter_floats[index_in_bound_vector]));
					}
					break;

				case SQLExecutor::STRING:
					{
						secondary_data__for_the_current_vg.push_back(sorting_row_of_data.current_parameter_strings[index_in_bound_vector]);
					}
					break;

				default:
					{
						errorMessages.push_back((boost::format("Invalid row binding for row %1%") % boost::lexical_cast<std::string>(current_rows_stepped).c_str()).str());
						bad = true;
						return;
					}
					break;

			}

		});

		// Currently, due to UOA validation, all CHILD (as well as primary and top-level)
		// BRANCH columns map to BRANCH columns in the primary UOA.
		//
		// Therefore, if the sorting_row_of_data has a bad branch
		// (due to Limit DMU functionality)
		// and it's a CHILD variable group being loaded,
		// then the corresponding bad branch column/s in the PRIMARY variable group that MIGHT have been loaded would NOT have been,
		// so skip out now.
		//
		// Child LEAVES, on the other hand, can currently map to
		// EITHER primary branches OR primary leaves.
		// IF a child LEAF is bad (due to Limit DMU functionality),
		// however, it has merge_mode = VARIABLE_GROUP_MERGE_MODE__CHILD
		// and will be instantly rejected in that case, below.
		if (!bad && !sorting_row_of_data.branch_has_excluded_dmu)
		{

			TIME_GRANULARITY time_granularity = top_level_variable_groups_vector[primary_vg_index__in__top_level_vg_vector].first.time_granularity;

			switch (merge_mode)
			{

				case VARIABLE_GROUP_MERGE_MODE__PRIMARY:
					{

						// *************************************************************************************************** //
						// All cases of a BAD LEAF (due to Limit DMU functionality)
						// are properly handled within "HandleIncomingNewBranchAndLeaf()"
						// (and the functions that it calls).
						//
						// Specifically, a bad leaf is handled in two places.
						//
						// (1) "AddNewTimeSlice()" will call "InsertLeaf()" and reject the leaf
						//     (but flag that a bad leaf was attempted to be added).
						//
						// (2) "MergeNewDataIntoTimeSlice()" will also, eventually, call "InsertLeaf()"
						//     and reject the leaf (but also flag that a bad leaf was attempted to be added).
						// *************************************************************************************************** //

						Leaf leaf(dmus_leaf__of_uoa_being_merged, static_cast<std::int32_t>(sorting_row_of_data.rowid), sorting_row_of_data.leaf_has_excluded_dmu);
						Branch branch(dmus_branch__of_uoa_being_merged);

						bool call_again = false;
						bool added = false;
						bool first = true;
						TimeSlices<hits_tag>::iterator mapIterator;
						auto incomingTimeSliceLeaf = std::make_pair(TimeSlice(sorting_row_of_data.datetime_start, sorting_row_of_data.datetime_end), leaf);

						while (first || call_again)
						{
							if (failed || CheckCancelled()) { break; }

							first = false;
							std::tuple<bool, bool, TimeSlices<hits_tag>::iterator> ret = allWeightings.HandleIncomingNewBranchAndLeaf(branch, incomingTimeSliceLeaf, variable_group_number, merge_mode,
									consolidate_rows, random_sampling, mapIterator, call_again);

							if (failed || CheckCancelled()) { break; }

							bool added_recurse = std::get<0>(ret);

							if (added_recurse)
							{
								added = true;
							}

							call_again = std::get<1>(ret);
							mapIterator = std::get<2>(ret);
						}

						if (failed || CheckCancelled()) { break; }

						if (added)
						{
							// Add the secondary data for this primary variable group to the cache
							allWeightings.dataCache[static_cast<std::int32_t>(sorting_row_of_data.rowid)] = secondary_data__for_the_current_vg;
						}

					}
					break;

				case VARIABLE_GROUP_MERGE_MODE__TOP_LEVEL:
					{

						// If ANY columns are bad for non-primary data loading,
						// (due to Limit DMU functionality)
						// then corresponding columns would never have been loaded (or rows generated from leaves)
						// when the primary group was previously loaded,
						// so skip out now.
						if (sorting_row_of_data.leaf_has_excluded_dmu)
						{
							break;
						}

						Leaf leaf(dmus_leaf__of_uoa_being_merged);
						Branch branch(dmus_branch__of_uoa_being_merged);

						// Set the secondary data index into the above cache for this non-primary top-level variable group
						// so that it can be set in the corresponding leaf already present for the branch
						leaf.other_top_level_indices_into_raw_data[static_cast<std::int16_t>(variable_group_number)] = static_cast<std::int32_t>(sorting_row_of_data.rowid);

						bool call_again = false;
						bool added = false;
						bool first = true;
						TimeSlices<hits_tag>::iterator mapIterator;
						auto incomingTimeSliceLeaf = std::make_pair(TimeSlice(sorting_row_of_data.datetime_start, sorting_row_of_data.datetime_end), leaf);

						while (first || call_again)
						{
							if (failed || CheckCancelled()) { break; }

							first = false;
							std::tuple<bool, bool, TimeSlices<hits_tag>::iterator> ret = allWeightings.HandleIncomingNewBranchAndLeaf(branch, incomingTimeSliceLeaf, variable_group_number, merge_mode,
									consolidate_rows, random_sampling, mapIterator, call_again);

							if (failed || CheckCancelled()) { break; }

							bool added_recurse = std::get<0>(ret);

							if (added_recurse)
							{
								added = true;
							}

							call_again = std::get<1>(ret);
							mapIterator = std::get<2>(ret);
						}

						if (failed || CheckCancelled()) { break; }

						if (added)
						{
							// Add the secondary data for this non-primary top-level variable group to the cache
							allWeightings.otherTopLevelCache[static_cast<std::int16_t>(variable_group_number)][static_cast<std::int32_t>(sorting_row_of_data.rowid)] = secondary_data__for_the_current_vg;
						}

					}
					break;

				case VARIABLE_GROUP_MERGE_MODE__CHILD:
					{

						// If ANY columns are bad for non-primary data loading,
						// (due to Limit DMU functionality)
						// then corresponding columns would never have been loaded (or rows generated from leaves)
						// when the primary group was previously loaded,
						// so skip out now.
						if (sorting_row_of_data.leaf_has_excluded_dmu)
						{
							break;
						}

						// pack the child data index into the main leaf for use in the function called below - because this leaf is TEMPORARY
						// (this data will be unpacked from the temporary leaf and put into the proper place in the function called below)
						Leaf leaf(dmus_leaf__of_uoa_being_merged, static_cast<std::int32_t>(sorting_row_of_data.rowid));
						Branch branch(dmus_branch__of_uoa_being_merged);

						// ************************************************************************************************** //
						// Important!
						// This is a *CHILD* merge,
						// so all LEAVES in the primary variable group have already been added to all branches.
						// No new leaves will be added to any branches here.
						// Therefore, even though 'HandleBranchAndLeaf()' is called,
						// which might slice the time slices, each such slice will not add any new primary leaves
						// and the previous set of cached leaves will be persisted in the time slice copies.
						// ************************************************************************************************** //
						bool call_again = false;
						bool added = false;
						bool first = true;
						TimeSlices<hits_tag>::iterator mapIterator;
						auto incomingTimeSliceLeaf = std::make_pair(TimeSlice(sorting_row_of_data.datetime_start, sorting_row_of_data.datetime_end), leaf);

						while (first || call_again)
						{
							if (failed || CheckCancelled()) { break; }

							first = false;
							std::tuple<bool, bool, TimeSlices<hits_tag>::iterator> ret = allWeightings.HandleIncomingNewBranchAndLeaf(branch, incomingTimeSliceLeaf, variable_group_number, merge_mode,
									consolidate_rows, random_sampling, mapIterator, call_again);

							if (failed || CheckCancelled()) { break; }

							bool added_recurse = std::get<0>(ret);

							if (added_recurse)
							{
								added = true;
							}

							call_again = std::get<1>(ret);
							mapIterator = std::get<2>(ret);
						}

						if (failed || CheckCancelled()) { break; }

						if (added)
						{
							// Add the secondary data for this child variable group to the cache
							allWeightings.childCache[static_cast<std::int16_t>(variable_group_number)][static_cast<std::int32_t>(sorting_row_of_data.rowid)] = secondary_data__for_the_current_vg;
						}

					}
					break;

				default:
					{
						boost::format msg("Invalid merge mode while reading raw data.");
						throw NewGeneException() << newgene_error_description(msg.str());
					}
					break;

			}

		}

		++current_rows_stepped;
		meter.UpdateProgressBarValue(current_rows_stepped);

		if (bad)
		{
			continue;
		}

	}

}

OutputModel::OutputGenerator::SqlAndColumnSet OutputModel::OutputGenerator::KadSamplerBuildOutputSchema()
{

	// **************************************************************************************** //
	// Initialize schema
	// **************************************************************************************** //

	SqlAndColumnSet result = std::make_pair(std::vector<SQLExecutor>(), NewGeneSchema());
	std::vector<SQLExecutor> & sql_strings = result.first;
	NewGeneSchema & result_columns = result.second;

	result_columns.most_recent_sql_statement_executed__index = -1;

	std::string view_name = "NGTEMP_RANDOM_SAMPLING";
	result_columns.view_name_no_uuid = view_name;

	view_name += "_";
	view_name += newUUID(true);

	result_columns.view_name = view_name;
	result_columns.view_number = 1;
	result_columns.has_no_datetime_columns = false;


	NewGeneSchema const & primary_variable_group_raw_data_columns = top_level_variable_groups_schema[primary_vg_index__in__top_level_vg_vector];

	// **************************************************************************************** //
	// Start with the primary key columns of multiplicity 1.
	// **************************************************************************************** //

	std::for_each(primary_variable_group_raw_data_columns.column_definitions.cbegin(),
				  primary_variable_group_raw_data_columns.column_definitions.cend(), [&](
					  NewGeneSchema::NewGeneColumnDefinition const & raw_data_table_column)
	{

		if (raw_data_table_column.column_type == NewGeneSchema::NewGeneColumnDefinition::COLUMN_TYPE__DATETIMESTART
			|| raw_data_table_column.column_type == NewGeneSchema::NewGeneColumnDefinition::COLUMN_TYPE__DATETIMESTART_INTERNAL
			|| raw_data_table_column.column_type == NewGeneSchema::NewGeneColumnDefinition::COLUMN_TYPE__DATETIMEEND
			|| raw_data_table_column.column_type == NewGeneSchema::NewGeneColumnDefinition::COLUMN_TYPE__DATETIMEEND_INTERNAL)
		{
			return; // Add these columns last, if selected by user
		}

		if (raw_data_table_column.column_type == NewGeneSchema::NewGeneColumnDefinition::COLUMN_TYPE__SECONDARY)
		{
			return; // Enforce that primary key columns appear first.
		}

		if (raw_data_table_column.column_type == NewGeneSchema::NewGeneColumnDefinition::COLUMN_TYPE__PRIMARY)
		{

			if (raw_data_table_column.total_outer_multiplicity__in_total_kad__for_current_dmu_category__for_current_variable_group > 1)
			{
				return; // Add the multiplicity = 1 primary keys first
			}

			result_columns.column_definitions.push_back(raw_data_table_column);
			NewGeneSchema::NewGeneColumnDefinition & new_column = result_columns.column_definitions.back();
			new_column.column_name_in_temporary_table = new_column.column_name_in_temporary_table_no_uuid;
			new_column.column_name_in_temporary_table += "_";
			new_column.column_name_in_temporary_table += newUUID(true);
			new_column.current_multiplicity__of__this_column__in__output__same_as__current_multiplicity__of___this_column__in_its_own_variable_group = 1;

		}

	});

	// **************************************************************************************** //
	// Proceed with the primary key columns of multiplicity greater than 1.
	// **************************************************************************************** //

	for (int current_multiplicity = 1; current_multiplicity <= K; ++current_multiplicity)
	{

		std::for_each(primary_variable_group_raw_data_columns.column_definitions.cbegin(),
					  primary_variable_group_raw_data_columns.column_definitions.cend(), [&](
						  NewGeneSchema::NewGeneColumnDefinition const & raw_data_table_column)
		{
			if (raw_data_table_column.column_type == NewGeneSchema::NewGeneColumnDefinition::COLUMN_TYPE__DATETIMESTART
				|| raw_data_table_column.column_type == NewGeneSchema::NewGeneColumnDefinition::COLUMN_TYPE__DATETIMESTART_INTERNAL
				|| raw_data_table_column.column_type == NewGeneSchema::NewGeneColumnDefinition::COLUMN_TYPE__DATETIMEEND
				|| raw_data_table_column.column_type == NewGeneSchema::NewGeneColumnDefinition::COLUMN_TYPE__DATETIMEEND_INTERNAL)
			{
				return; // Add these columns last, if selected by user
			}

			if (raw_data_table_column.column_type == NewGeneSchema::NewGeneColumnDefinition::COLUMN_TYPE__SECONDARY)
			{
				return; // Enforce that primary key columns appear first.
			}

			if (raw_data_table_column.column_type == NewGeneSchema::NewGeneColumnDefinition::COLUMN_TYPE__PRIMARY)
			{

				if (raw_data_table_column.total_outer_multiplicity__in_total_kad__for_current_dmu_category__for_current_variable_group == 1)
				{
					return; // Add the multiplicity = 1 primary keys first
				}

				result_columns.column_definitions.push_back(raw_data_table_column);
				NewGeneSchema::NewGeneColumnDefinition & new_column = result_columns.column_definitions.back();
				new_column.column_name_in_temporary_table = new_column.column_name_in_temporary_table_no_uuid;
				new_column.column_name_in_temporary_table += "_";
				new_column.column_name_in_temporary_table += newUUID(true);
				new_column.current_multiplicity__of__this_column__in__output__same_as__current_multiplicity__of___this_column__in_its_own_variable_group = current_multiplicity;
				new_column.primary_key_index_within_total_kad_for_dmu_category =
					new_column.primary_key_index__within_uoa_corresponding_to_current_variable_group
					+ (current_multiplicity - 1) * new_column.total_k_count__within_uoa_corresponding_to_current_variable_group__for_current_dmu_category;

			}

		});

	}




	// **************************************************************************************** //
	// For the primary top-level variable group, construct secondary data columns
	// **************************************************************************************** //

	{

		NewGeneSchema const & primary_variable_group_raw_data_columns = top_level_variable_groups_schema[primary_vg_index__in__top_level_vg_vector];

		WidgetInstanceIdentifiers const & variables_selected =
			(*the_map)[*primary_variable_group_raw_data_columns.variable_groups[0].identifier_parent][primary_variable_group_raw_data_columns.variable_groups[0]];

		// Proceed to the secondary key columns.
		for (int current_multiplicity = 1; current_multiplicity <= K; ++current_multiplicity)
		{

			std::for_each(primary_variable_group_raw_data_columns.column_definitions.cbegin(),
						  primary_variable_group_raw_data_columns.column_definitions.cend(), [&](NewGeneSchema::NewGeneColumnDefinition const & raw_data_table_column)
			{

				bool make_secondary_datetime_column = false;

				if (raw_data_table_column.column_type == NewGeneSchema::NewGeneColumnDefinition::COLUMN_TYPE__DATETIMESTART
					|| raw_data_table_column.column_type == NewGeneSchema::NewGeneColumnDefinition::COLUMN_TYPE__DATETIMEEND)
				{
					// Do not return!  If the user selects these columns, they should appear as regular secondary key columns.
					make_secondary_datetime_column = true;
				}

				if (!make_secondary_datetime_column && raw_data_table_column.column_type != NewGeneSchema::NewGeneColumnDefinition::COLUMN_TYPE__SECONDARY)
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
					result_columns.column_definitions.push_back(raw_data_table_column);
					NewGeneSchema::NewGeneColumnDefinition & new_column = result_columns.column_definitions.back();
					new_column.column_name_in_temporary_table = new_column.column_name_in_temporary_table_no_uuid;
					new_column.column_name_in_temporary_table += "_";
					new_column.column_name_in_temporary_table += newUUID(true);
					new_column.current_multiplicity__of__this_column__in__output__same_as__current_multiplicity__of___this_column__in_its_own_variable_group = current_multiplicity;
					new_column.total_outer_multiplicity__in_total_kad__for_current_dmu_category__for_current_variable_group = K;

					if (make_secondary_datetime_column)
					{
						new_column.column_type = NewGeneSchema::NewGeneColumnDefinition::COLUMN_TYPE__SECONDARY;
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
	std::for_each(top_level_variable_groups_schema.cbegin(),
				  top_level_variable_groups_schema.cend(), [&](NewGeneSchema const & primary_variable_group_raw_data_columns)
	{

		if (primary_group_number == primary_vg_index__in__top_level_vg_vector)
		{
			++primary_group_number;
			return; // already handled - the primary top-level variable group's secondary data always goes first
		}

		WidgetInstanceIdentifiers const & variables_selected =
			(*the_map)[*primary_variable_group_raw_data_columns.variable_groups[0].identifier_parent][primary_variable_group_raw_data_columns.variable_groups[0]];

		// Proceed to the secondary key columns.
		for (int current_multiplicity = 1; current_multiplicity <= K; ++current_multiplicity)
		{

			std::for_each(primary_variable_group_raw_data_columns.column_definitions.cbegin(),
						  primary_variable_group_raw_data_columns.column_definitions.cend(), [&](NewGeneSchema::NewGeneColumnDefinition const & raw_data_table_column)
			{

				bool make_secondary_datetime_column = false;

				if (raw_data_table_column.column_type == NewGeneSchema::NewGeneColumnDefinition::COLUMN_TYPE__DATETIMESTART
					|| raw_data_table_column.column_type == NewGeneSchema::NewGeneColumnDefinition::COLUMN_TYPE__DATETIMEEND)
				{
					// Do not return!  If the user selects these columns, they should appear as regular secondary key columns.
					make_secondary_datetime_column = true;
				}

				if (!make_secondary_datetime_column && raw_data_table_column.column_type != NewGeneSchema::NewGeneColumnDefinition::COLUMN_TYPE__SECONDARY)
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
					result_columns.column_definitions.push_back(raw_data_table_column);
					NewGeneSchema::NewGeneColumnDefinition & new_column = result_columns.column_definitions.back();
					new_column.column_name_in_temporary_table = new_column.column_name_in_temporary_table_no_uuid;
					new_column.column_name_in_temporary_table += "_";
					new_column.column_name_in_temporary_table += newUUID(true);
					new_column.current_multiplicity__of__this_column__in__output__same_as__current_multiplicity__of___this_column__in_its_own_variable_group = current_multiplicity;
					new_column.total_outer_multiplicity__in_total_kad__for_current_dmu_category__for_current_variable_group = K;

					if (make_secondary_datetime_column)
					{
						new_column.column_type = NewGeneSchema::NewGeneColumnDefinition::COLUMN_TYPE__SECONDARY;
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

	std::for_each(secondary_variable_groups_schema.cbegin(),
				  secondary_variable_groups_schema.cend(), [&](NewGeneSchema const & child_variable_group_raw_data_columns)
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

			std::for_each(child_variable_group_raw_data_columns.column_definitions.cbegin(),
						  child_variable_group_raw_data_columns.column_definitions.cend(), [&](NewGeneSchema::NewGeneColumnDefinition const & new_column_secondary)
			{
				bool make_secondary_datetime_column = false;

				if (new_column_secondary.column_type == NewGeneSchema::NewGeneColumnDefinition::COLUMN_TYPE__DATETIMESTART
					|| new_column_secondary.column_type == NewGeneSchema::NewGeneColumnDefinition::COLUMN_TYPE__DATETIMEEND)
				{
					// Do not return!  If the user selects these columns, they should appear as regular secondary key columns.
					make_secondary_datetime_column = true;
				}

				if (!make_secondary_datetime_column && new_column_secondary.column_type != NewGeneSchema::NewGeneColumnDefinition::COLUMN_TYPE__SECONDARY)
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
					result_columns.column_definitions.push_back(new_column_secondary);
					NewGeneSchema::NewGeneColumnDefinition & new_column = result_columns.column_definitions.back();
					new_column.column_name_in_temporary_table = new_column.column_name_in_temporary_table_no_uuid;
					new_column.column_name_in_temporary_table += "_";
					new_column.column_name_in_temporary_table += newUUID(true);
					new_column.current_multiplicity__of__this_column__in__output__same_as__current_multiplicity__of___this_column__in_its_own_variable_group = current_multiplicity;
					new_column.total_outer_multiplicity__in_total_kad__for_current_dmu_category__for_current_variable_group = the_child_multiplicity;

					if (make_secondary_datetime_column)
					{
						new_column.column_type = NewGeneSchema::NewGeneColumnDefinition::COLUMN_TYPE__SECONDARY;
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

	result_columns.column_definitions.push_back(NewGeneSchema::NewGeneColumnDefinition());
	NewGeneSchema::NewGeneColumnDefinition & datetime_start_column = result_columns.column_definitions.back();
	datetime_start_column.column_name_in_temporary_table = datetime_start_col_name;
	datetime_start_column.column_name_in_temporary_table_no_uuid = datetime_start_col_name_no_uuid;
	datetime_start_column.current_multiplicity__of__this_column__in__output__same_as__current_multiplicity__of___this_column__in_its_own_variable_group = K;
	datetime_start_column.column_type = NewGeneSchema::NewGeneColumnDefinition::COLUMN_TYPE__DATETIMESTART__TIME_SLICE;
	datetime_start_column.current_variable_group = WidgetInstanceIdentifier();
	datetime_start_column.uoa_associated_with_current_variable_group = WidgetInstanceIdentifier();
	datetime_start_column.column_name_in_original_data_table = "";

	std::string datetime_end_col_name_no_uuid = "DATETIMEEND__TIME_SLICE";
	datetime_end_col_name = datetime_end_col_name_no_uuid;
	datetime_end_col_name += "_";
	datetime_end_col_name += newUUID(true);

	result_columns.column_definitions.push_back(NewGeneSchema::NewGeneColumnDefinition());
	NewGeneSchema::NewGeneColumnDefinition & datetime_end_column = result_columns.column_definitions.back();
	datetime_end_column.column_name_in_temporary_table = datetime_end_col_name;
	datetime_end_column.column_name_in_temporary_table_no_uuid = datetime_end_col_name_no_uuid;
	datetime_end_column.current_multiplicity__of__this_column__in__output__same_as__current_multiplicity__of___this_column__in_its_own_variable_group = K;
	datetime_end_column.column_type = NewGeneSchema::NewGeneColumnDefinition::COLUMN_TYPE__DATETIMESTART__TIME_SLICE;
	datetime_end_column.current_variable_group = WidgetInstanceIdentifier();
	datetime_end_column.uoa_associated_with_current_variable_group = WidgetInstanceIdentifier();
	datetime_end_column.column_name_in_original_data_table = "";

	result_columns.current_block_datetime_column_types = std::make_pair(
				NewGeneSchema::NewGeneColumnDefinition::COLUMN_TYPE__DATETIMESTART__TIME_SLICE,
				NewGeneSchema::NewGeneColumnDefinition::COLUMN_TYPE__DATETIMESTART__TIME_SLICE);
	result_columns.previous_block_datetime_column_types = result_columns.current_block_datetime_column_types;

	ExecuteSQL(result); // Executes all SQL queries up to the current one

	if (failed || CheckCancelled())
	{
		return result;
	}

	return result;

}

void OutputModel::OutputGenerator::KadSamplerCreateOutputTable()
{

	std::vector<SQLExecutor> & sql_strings = random_sampling_schema.first;
	NewGeneSchema & random_sampling_columns = random_sampling_schema.second;
	random_sampling_columns.most_recent_sql_statement_executed__index = -1;

	std::string sql_create_empty_table;
	sql_create_empty_table += "CREATE TABLE \"";
	sql_create_empty_table += random_sampling_columns.view_name;
	sql_create_empty_table += "\"";
	sql_create_empty_table += " (";

	bool first = true;
	std::for_each(random_sampling_columns.column_definitions.cbegin(),
				  random_sampling_columns.column_definitions.cend(), [&](
					  NewGeneSchema::NewGeneColumnDefinition const & random_sampling_column)
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
		SetFailureErrorMessage(sql_error);
		return;
	}

	if (CheckCancelled())
	{
		return;
	}

}

void OutputModel::OutputGenerator::PrepareInsertStatement(sqlite3_stmt *& insert_random_sample_stmt, NewGeneSchema const & random_sampling_columns)
{

	if (insert_random_sample_stmt == nullptr)
	{
		std::string insert_random_sample_string;

		insert_random_sample_string += "INSERT INTO \"";
		insert_random_sample_string += random_sampling_columns.view_name;
		insert_random_sample_string += "\" (";

		bool first = true;
		std::for_each(random_sampling_columns.column_definitions.cbegin(),
					  random_sampling_columns.column_definitions.cend(), [&](
						  NewGeneSchema::NewGeneColumnDefinition const & random_sampling_column)
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

		for (size_t n = 0; n < random_sampling_columns.column_definitions.size(); ++n)
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
			boost::format msg("Unable to prepare SQL query to insert a random k-ad row: %1% (%2%)");
			msg % sql_error.c_str() % insert_random_sample_string.c_str();
			throw NewGeneException() << newgene_error_description(msg.str());
		}

	}

}

void OutputModel::OutputGenerator::KadSamplerFillDataForNonPrimaryGroups(KadSampler & allWeightings)
{

	// **************************************************************************************** //
	// Top-level variable groups that are *not* the single primary variable group, are handled here
	// **************************************************************************************** //

	int current_top_level_vg_index = 0;
	std::for_each(top_level_variable_groups_schema.cbegin(), top_level_variable_groups_schema.cend(), [&](NewGeneSchema const & primary_variable_group_raw_data_columns)
	{

		if (failed || CheckCancelled()) { return; }

		if (current_top_level_vg_index == primary_vg_index__in__top_level_vg_vector)
		{
			// Skip the primary top-level variable group;
			// we are only populating columns of secondary data
			// for NON-primary top-level variable groups,
			// which are for the purposes of this function
			// considered to be child variable groups
			++current_top_level_vg_index;
			return;
		}

		// ********************************************************************************************************************************************************* //
		// From the schema for the selected columns for the non-primary top-level variable group,
		// create a temporary table to store just the selected columns over just the selected time range.
		// This function both creates the table, and loads it with just the desired columns (and time range)
		// from the permanent raw data tables.
		// ********************************************************************************************************************************************************* //
		SqlAndColumnSet selected_raw_data_table_schema = CreateTableOfSelectedVariablesFromRawData(primary_variable_group_raw_data_columns, current_top_level_vg_index);

		if (failed || CheckCancelled()) { return; }

		selected_raw_data_table_schema.second.most_recent_sql_statement_executed__index = -1;
		ExecuteSQL(selected_raw_data_table_schema);
		merging_of_children_column_sets.push_back(selected_raw_data_table_schema);

		// ********************************************************************************************************************************************************* //
		// Merge into the existing TIME SLICES.
		// From the schema for the selected columns for the non-primary top-level variable group,
		// load the selected columns of raw data into the temporary table created with the same schema.
		// ********************************************************************************************************************************************************* //
		messager.AppendKadStatusText((boost::format("*****************************************************")).str().c_str(), this);
		messager.AppendKadStatusText((boost::format("Merge variable group \"%1%\"...") % Table_VG_CATEGORY::GetVgDisplayTextShort(
										  top_level_variable_groups_vector[current_top_level_vg_index].first)).str().c_str(), this);
		std::vector<std::string> errorMessages;
		KadSampler_ReadData_AddToTimeSlices(selected_raw_data_table_schema.second, current_top_level_vg_index, allWeightings, VARIABLE_GROUP_MERGE_MODE__TOP_LEVEL, errorMessages);

		if (failed || CheckCancelled() || errorMessages.size() > 0)
		{
			std::string errorsOut;
			bool first = true;

			for (auto errorMsg : errorMessages)
			{
				if (!first)
				{
					errorsOut += "\n";
				}

				first = false;
				errorsOut += errorMsg;
			}

			SetFailureErrorMessage(errorsOut);
			return;
		}

		++current_top_level_vg_index;

	});

	// **************************************************************************************** //
	// Child variable groups are handled here
	// **************************************************************************************** //

	int current_child_vg_index = 0;
	std::for_each(secondary_variable_groups_schema.cbegin(), secondary_variable_groups_schema.cend(), [&](NewGeneSchema const & child_variable_group_raw_data_columns)
	{

		if (failed || CheckCancelled()) { return; }

		// ********************************************************************************************************************************************************* //
		// From the schema for the selected columns for the child variable group,
		// create a temporary table to store just the selected columns over just the selected time range.
		// ********************************************************************************************************************************************************* //
		SqlAndColumnSet selected_raw_data_table_schema = CreateTableOfSelectedVariablesFromRawData(child_variable_group_raw_data_columns, current_child_vg_index);

		if (failed || CheckCancelled()) { return; }

		selected_raw_data_table_schema.second.most_recent_sql_statement_executed__index = -1;
		ExecuteSQL(selected_raw_data_table_schema);
		merging_of_children_column_sets.push_back(selected_raw_data_table_schema);

		int const the_child_multiplicity = child_uoas__which_multiplicity_is_greater_than_1[*(child_variable_group_raw_data_columns.variable_groups[0].identifier_parent)].second;

		// **************************************************************************************** //
		// Populate the index mappings that map each child group's branch DMU data,
		// and leaf DMU data, to a corresponding specific index in the
		// top-level variable group's branch or a specific index in one of its specific leaves.
		// **************************************************************************************** //

		int current_primary_branch_index = 0;
		int current_primary_internal_leaf_index = 0;
		int current_primary_leaf_number = 0;

		int overall_top_level_primary_key_sequence_number_including_branch_and_all_leaves_and_all_columns_within_each_leaf = 0;

		for (; overall_top_level_primary_key_sequence_number_including_branch_and_all_leaves_and_all_columns_within_each_leaf <
			 overall_total_number_of_primary_key_columns_including_all_branch_columns_and_all_leaves_and_all_columns_internal_to_each_leaf;
			 ++overall_top_level_primary_key_sequence_number_including_branch_and_all_leaves_and_all_columns_within_each_leaf)
		{

			if (failed || CheckCancelled()) { break; }

			// **************************************************************************************** //
			// The following column order bookkeeping is a pain in the &$^,
			// but it has to be done, and this is a fine place to do it.
			//
			// Enforce that the sequence number appears in order
			// **************************************************************************************** //
			std::for_each(sequence.primary_key_sequence_info.cbegin(), sequence.primary_key_sequence_info.cend(), [&](PrimaryKeySequence::PrimaryKeySequenceEntry const & full_kad_key_info)
			{

				if (failed || CheckCancelled()) { return; }

				if (overall_top_level_primary_key_sequence_number_including_branch_and_all_leaves_and_all_columns_within_each_leaf !=
					full_kad_key_info.sequence_number_in_all_primary_keys__of__order_columns_appear_in_top_level_vg)
				{
					return;
				}

				bool is_current_index_a_top_level_primary_group_branch = false;

				if (full_kad_key_info.total_outer_multiplicity__for_the_current_dmu_category__corresponding_to_the_uoa_corresponding_to_top_level_variable_group == 1)
				{
					is_current_index_a_top_level_primary_group_branch = true;
				}

				// Grab the primary key information for the TOP LEVEL variable group
				PrimaryKeySequence::VariableGroup_PrimaryKey_Info const & full_kad_key_info_primary_not_child_variable_group =
					full_kad_key_info.variable_group_info_for_primary_keys__top_level_and_child[primary_vg_index__in__top_level_vg_vector];

				// Now grab the primary key information for the current CHILD variable group
				PrimaryKeySequence::VariableGroup_PrimaryKey_Info const & full_kad_key_info_child_variable_group =
					full_kad_key_info.variable_group_info_for_primary_keys__top_level_and_child[top_level_variable_groups_schema.size() + current_child_vg_index];

				if (full_kad_key_info_child_variable_group.total_outer_multiplicity__in_total_kad__for_current_dmu_category__for_current_variable_group == 0)
				{
					// There are no columns in this DMU for our current child variable group
					if (is_current_index_a_top_level_primary_group_branch)
					{
						++current_primary_branch_index;
					}

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
					allWeightings.mappings_from_child_branch_to_primary[current_child_vg_index].push_back(ChildToPrimaryMapping(CHILD_TO_PRIMARY_MAPPING__MAPS_TO_BRANCH,
							current_primary_branch_index));
				}
				else if (is_current_index_a_top_level_primary_group_branch && !is_child_group_branch)
				{
					allWeightings.mappings_from_child_leaf_to_primary[current_child_vg_index].push_back(ChildToPrimaryMapping(CHILD_TO_PRIMARY_MAPPING__MAPS_TO_BRANCH, current_primary_branch_index));
				}
				else if (!is_current_index_a_top_level_primary_group_branch && is_child_group_branch)
				{
					allWeightings.mappings_from_child_branch_to_primary[current_child_vg_index].push_back(ChildToPrimaryMapping(CHILD_TO_PRIMARY_MAPPING__MAPS_TO_LEAF,
							current_primary_internal_leaf_index, current_primary_leaf_number));
				}
				else if (!is_current_index_a_top_level_primary_group_branch && !is_child_group_branch)
				{
					allWeightings.mappings_from_child_leaf_to_primary[current_child_vg_index].push_back(ChildToPrimaryMapping(CHILD_TO_PRIMARY_MAPPING__MAPS_TO_LEAF,
							current_primary_internal_leaf_index, current_primary_leaf_number));
				}
				else
				{
					boost::format msg("No mapping possible from child to primary DMU!");
					throw NewGeneException() << newgene_error_description(msg.str());
				}

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

			});

		}

		if (failed || CheckCancelled()) { return; }


		// **************************************************************************************** //
		// Clear the child DMU -> primary DMU lookup cache,
		// which is shared between child variable groups to save memory,
		// and which must be purged and re-populated prior to merging in the next
		// child variable group.
		// **************************************************************************************** //
		messager.AppendKadStatusText((boost::format("*****************************************************")).str().c_str(), this);
		messager.AppendKadStatusText((boost::format("Build cache to prepare for merge of child variable group \"%1%\"...") % Table_VG_CATEGORY::GetVgDisplayTextShort(
										  child_variable_groups_vector[current_child_vg_index].first)).str().c_str(), this);
		allWeightings.ResetBranchCaches(current_child_vg_index, true);

		if (failed || CheckCancelled()) { return; }

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

		// ********************************************************************************************************************************************************* //
		// Merge into the existing TIME SLICES.
		// From the schema for the selected columns for the child variable group,
		// load the selected columns of raw data into the temporary table created with the same schema.
		// ********************************************************************************************************************************************************* //
		messager.AppendKadStatusText((boost::format("Merge child variable group \"%1%\"...") % Table_VG_CATEGORY::GetVgDisplayTextShort(
										  child_variable_groups_vector[current_child_vg_index].first)).str().c_str(), this);
		std::vector<std::string> errorMessages;
		allWeightings.current_child_variable_group_being_merged = current_child_vg_index;
		KadSampler_ReadData_AddToTimeSlices(selected_raw_data_table_schema.second, current_child_vg_index, allWeightings, VARIABLE_GROUP_MERGE_MODE__CHILD, errorMessages);
		allWeightings.current_child_variable_group_being_merged = -1;

		if (failed || CheckCancelled() || errorMessages.size() > 0)
		{
			std::string errorsOut;
			bool first = true;

			for (auto errorMsg : errorMessages)
			{
				if (!first)
				{
					errorsOut += "\n";
				}

				first = false;
				errorsOut += errorMsg;
			}

			SetFailureErrorMessage(errorsOut);
			return;
		}

		++current_child_vg_index;

	});

}

void OutputModel::OutputGenerator::ConsolidateData(bool const random_sampling, KadSampler & allWeightings)
{

	if (random_sampling)
	{
		// This MUST come after the child groups have been merged!
		// For unbiased sampling, the child groups may only cross a subset of time units within a branch
		ConsolidateRowsWithinSingleTimeSlicesAcrossTimeUnits(allWeightings);

		if (failed || CheckCancelled()) { return; }
	}

	// Create pointer to prevent automatic deletion.
	// We are here using a Boost Pool to delete the memory in saved_historic_rows,
	// because standard deletion through the pool takes forever.
	//FastSetMemoryTag<MergedTimeSliceRow<saved_historic_rows_tag>, saved_historic_rows_tag> * saved_historic_rows_ = InstantiateUsingTopLevelObjectsPool<tag__saved_historic_rows<saved_historic_rows_tag>>();
	//FastSetMemoryTag<MergedTimeSliceRow<saved_historic_rows_tag>, saved_historic_rows_tag> & saved_historic_rows = *saved_historic_rows_;

	auto & saved_historic_rows = allWeightings.consolidated_rows;

	FastSetMemoryTag<MergedTimeSliceRow<ongoing_merged_rows_tag>, ongoing_merged_rows_tag> * ongoing_merged_rows_ =
		InstantiateUsingTopLevelObjectsPool<tag__ongoing_merged_rows<ongoing_merged_rows_tag>>();
	FastSetMemoryTag<MergedTimeSliceRow<ongoing_merged_rows_tag>, ongoing_merged_rows_tag> & ongoing_merged_rows = *ongoing_merged_rows_;

	// ***************************************************************************************************** //
	// First, calculate the number of rows that must be processed, for use by the progress bar
	// ***************************************************************************************************** //
	// Avoid variable name collisions by placing in block
	std::int64_t total_row_count = 0;
	{
		TimeSlice previousTimeSlice;
		std::for_each(allWeightings.timeSlices.cbegin(), allWeightings.timeSlices.cend(), [&](decltype(allWeightings.timeSlices)::value_type const & timeSlice)
		{
			if (failed || CheckCancelled()) { return; }

			TimeSlice const & the_slice = timeSlice.first;
			VariableGroupTimeSliceData const & variableGroupTimeSliceData = timeSlice.second;
			VariableGroupBranchesAndLeavesVector<hits_tag> const & variableGroupBranchesAndLeavesVector = *variableGroupTimeSliceData.branches_and_leaves;

			// See equivalent loop, below, for some comments
			if (previousTimeSlice.DoesOverlap(the_slice))
			{
				std::for_each(variableGroupBranchesAndLeavesVector.cbegin(), variableGroupBranchesAndLeavesVector.cend(), [&](VariableGroupBranchesAndLeaves const & variableGroupBranchesAndLeaves)
				{
					if (failed || CheckCancelled()) { return; }

					std::for_each(variableGroupBranchesAndLeaves.branches.cbegin(), variableGroupBranchesAndLeaves.branches.cend(), [&](Branch const & branch)
					{
						if (failed || CheckCancelled()) { return; }

						if (random_sampling)
						{
							if (allWeightings.time_granularity == TIME_GRANULARITY__NONE)
							{
								// Special case for random sampling: all data is in hits[-1], rather than hits_consolidated
								auto const & incoming_rows = branch.hits[-1];
								total_row_count += static_cast<std::int64_t>(incoming_rows.size());
							}
							else
							{
								auto & incoming_rows = branch.hits_consolidated;
								total_row_count += static_cast<std::int64_t>(incoming_rows.size());
							}
						}
						else
						{
							auto const & incoming_rows = branch.hits[-1];
							total_row_count += static_cast<std::int64_t>(incoming_rows.size());
						}
					});
				});
			}
			else
			{
				std::for_each(variableGroupBranchesAndLeavesVector.cbegin(), variableGroupBranchesAndLeavesVector.cend(), [&](VariableGroupBranchesAndLeaves const & variableGroupBranchesAndLeaves)
				{
					if (failed || CheckCancelled()) { return; }

					std::for_each(variableGroupBranchesAndLeaves.branches.cbegin(), variableGroupBranchesAndLeaves.branches.cend(), [&](Branch const & branch)
					{
						if (failed || CheckCancelled()) { return; }

						if (random_sampling)
						{
							if (allWeightings.time_granularity == TIME_GRANULARITY__NONE)
							{
								// Special case for random sampling: all data is in hits[-1], rather than hits_consolidated
								auto const & incoming_rows = branch.hits[-1];
								total_row_count += static_cast<std::int64_t>(incoming_rows.size());
							}
							else
							{
								auto const & incoming_rows = branch.hits_consolidated;
								total_row_count += static_cast<std::int64_t>(incoming_rows.size());
							}
						}
						else
						{
							auto const & incoming_rows = branch.hits[-1];
							total_row_count += static_cast<std::int64_t>(incoming_rows.size());
						}
					});
				});
			}

			previousTimeSlice = the_slice;
		});
	}


	// ***************************************************************************************************** //
	// Now, actually perform the consolidation of the rows
	// ***************************************************************************************************** //
	// Avoid variable name collisions by placing in block
	ProgressBarMeter meter(messager, std::string("%1% / %2% rows consolidated"), total_row_count);
	{

		TimeSlice previousTimeSlice;
		std::int64_t orig_row_count = 0;
		std::for_each(allWeightings.timeSlices.cbegin(), allWeightings.timeSlices.cend(), [&](decltype(allWeightings.timeSlices)::value_type const & timeSlice)
		{

			if (failed || CheckCancelled()) { return; }

			TimeSlice const & the_slice = timeSlice.first;
			VariableGroupTimeSliceData const & variableGroupTimeSliceData = timeSlice.second;
			VariableGroupBranchesAndLeavesVector<hits_tag> const & variableGroupBranchesAndLeavesVector = *variableGroupTimeSliceData.branches_and_leaves;

			if (previousTimeSlice.DoesOverlap(the_slice))
			{

				// The current time slice is adjacent to the previous.  Merge identical rows

				// We have a single time slice here

				// // std::set<MergedTimeSliceRow<saved_historic_rows_tag>> incoming;
				// Create pointer to prevent automatic deletion.
				// We are here using a Boost Pool to delete the memory in saved_historic_rows,
				// because standard deletion through the pool takes forever.
				FastSetMemoryTag<MergedTimeSliceRow<ongoing_consolidation_tag>, ongoing_consolidation_tag> * incoming_ =
					InstantiateUsingTopLevelObjectsPool<tag__ongoing_consolidation<ongoing_consolidation_tag>>();
				FastSetMemoryTag<MergedTimeSliceRow<ongoing_consolidation_tag>, ongoing_consolidation_tag> & incoming = *incoming_;

				std::for_each(variableGroupBranchesAndLeavesVector.cbegin(), variableGroupBranchesAndLeavesVector.cend(), [&](VariableGroupBranchesAndLeaves const & variableGroupBranchesAndLeaves)
				{

					if (failed || CheckCancelled()) { return; }

					// One primary variable group for the current time slice (for now, this is all that is supported).
					// Iterate through all of its branches (fixed values of raw data for the primary keys that are not part of the k-ad; i.e., have multiplicity 1)
					std::for_each(variableGroupBranchesAndLeaves.branches.cbegin(), variableGroupBranchesAndLeaves.branches.cend(), [&](Branch const & branch)
					{

						if (failed || CheckCancelled()) { return; }

						// Here is a single branch and its leaves, corresponding to the primary variable group and the current time slice.

						// ***************************************************************************************************** //
						// All data within a single branch is guaranteed to have been consolidated into index -1.
						// This was either done in the block above (for random sampling),
						// or it was done automatically by full sampling (in the case of full sampling,
						// the data within each branch is always placed into index -1
						// and "granulated" only upon output).
						// Special case exception: Random sampling with the primary variable group having no time granularity.
						// In this case, the random sampling algorithm will place all results into index -1,
						// so the above block that consolidates to index -1 is not necessary.
						// ***************************************************************************************************** //

						MergedTimeSliceRow_RHS_wins = true; // optimizer might call copy constructor, which calls operator=(), during "emplace"

						if (random_sampling)
						{

							if (allWeightings.time_granularity == TIME_GRANULARITY__NONE)
							{

								// Special case for random sampling: all data is in hits[-1], rather than hits_consolidated
								auto const & incoming_rows = branch.hits[-1];
								std::for_each(incoming_rows.cbegin(), incoming_rows.cend(), [&](BranchOutputRow<hits_tag> const & incoming_row)
								{
									if (failed || CheckCancelled()) { return; }

									EmplaceIncomingRowFromTimeSliceBranchDuringConsolidation<hits_consolidated_tag>(allWeightings, branch, incoming_row, incoming, the_slice, orig_row_count);
									meter.UpdateProgressBarValue(orig_row_count);
								});

							}
							else
							{

								// In this case, for optimization, the rows for each branch are stored in branch.hits_consolidated
								auto & incoming_rows = branch.hits_consolidated;
								std::for_each(incoming_rows.cbegin(), incoming_rows.cend(), [&](BranchOutputRow<hits_tag> const & incoming_row)
								{
									if (failed || CheckCancelled()) { return; }

									EmplaceIncomingRowFromTimeSliceBranchDuringConsolidation<hits_consolidated_tag>(allWeightings, branch, incoming_row, incoming, the_slice, orig_row_count);
									meter.UpdateProgressBarValue(orig_row_count);
								});

							}

						}
						else
						{

							// In this case, full sampling, the rows were already added to branch.hits[-1] in consolidated fashion within each branch
							auto const & incoming_rows = branch.hits[-1];
							std::for_each(incoming_rows.cbegin(), incoming_rows.cend(), [&](BranchOutputRow<hits_tag> const & incoming_row)
							{
								if (failed || CheckCancelled()) { return; }

								EmplaceIncomingRowFromTimeSliceBranchDuringConsolidation<hits_consolidated_tag>(allWeightings, branch, incoming_row, incoming, the_slice, orig_row_count);
								meter.UpdateProgressBarValue(orig_row_count);
							});

						}

					});

				});

				if (failed || CheckCancelled()) { return; }

				MergedTimeSliceRow_RHS_wins = false;

				// Ditto memory pool comments to "incoming", above
				FastVectorMemoryTag<MergedTimeSliceRow<ongoing_consolidation_tag>, ongoing_consolidation_tag> * intersection_ =
					InstantiateUsingTopLevelObjectsPool<tag__ongoing_consolidation_vector<ongoing_consolidation_tag>>();
				FastVectorMemoryTag<MergedTimeSliceRow<ongoing_consolidation_tag>, ongoing_consolidation_tag> * only_previous_ =
					InstantiateUsingTopLevelObjectsPool<tag__ongoing_consolidation_vector<ongoing_consolidation_tag>>();
				FastVectorMemoryTag<MergedTimeSliceRow<ongoing_consolidation_tag>, ongoing_consolidation_tag> * only_new_ =
					InstantiateUsingTopLevelObjectsPool<tag__ongoing_consolidation_vector<ongoing_consolidation_tag>>();

				FastVectorMemoryTag<MergedTimeSliceRow<ongoing_consolidation_tag>, ongoing_consolidation_tag> & intersection = *intersection_;
				FastVectorMemoryTag<MergedTimeSliceRow<ongoing_consolidation_tag>, ongoing_consolidation_tag> & only_previous = *only_previous_;
				FastVectorMemoryTag<MergedTimeSliceRow<ongoing_consolidation_tag>, ongoing_consolidation_tag> & only_new = *only_new_;

				intersection.resize(std::max(ongoing_merged_rows.size(), incoming.size()));
				only_previous.resize(ongoing_merged_rows.size());
				only_new.resize(incoming.size());

				// ************************************************************************************************************************************************************** //
				// Find out which rows match just in terms of DATA (not time) between the incoming, and the previous, sets of rows
				// ************************************************************************************************************************************************************** //
				{

					// Place into a block so that the iterators go off the stack
					// before the memory pool is purged at the end of the enclosing block.
					// Otherwise, the iterators are corrupt when they go out of scope.

					auto finalpos = std::set_intersection(ongoing_merged_rows.cbegin(), ongoing_merged_rows.cend(), incoming.cbegin(), incoming.cend(), intersection.begin());
					auto finalpos_previous = std::set_difference(ongoing_merged_rows.cbegin(), ongoing_merged_rows.cend(), incoming.cbegin(), incoming.cend(), only_previous.begin());
					auto finalpos_incoming = std::set_difference(incoming.cbegin(), incoming.cend(), ongoing_merged_rows.cbegin(), ongoing_merged_rows.cend(), only_new.begin());

					intersection.resize(finalpos - intersection.begin());
					only_previous.resize(finalpos_previous - only_previous.begin());
					only_new.resize(finalpos_incoming - only_new.begin());

				}

				// ************************************************************************************************************************************************************** //
				// Any rows that match in terms of DATA are also known to be adjacent in TIME,
				// and these can now be placed into the ongoing set.
				// Those previous rows that did not match are stashed away.
				// ************************************************************************************************************************************************************** //

				// We need to extend the time slice of the "intersection" rows,
				// which are guaranteed to have been copied by "set_intersection" from the "ongoing_merged_rows" container,
				// and will therefore (using the custom assignment operator which always takes the element that is not empty -
				// and the result vector contained default-constructed elements which were empty)
				// contain the time slices from the "ongoing_merged_rows" container,
				// guaranteeing that these time slices, which may start at different places for different rows
				// although they all *end* at the start of the new time slice, are saved.
				std::for_each(intersection.begin(), intersection.end(), [&](MergedTimeSliceRow<ongoing_consolidation_tag> & merged_row)
				{
					if (failed || CheckCancelled()) { return; }

					if (the_slice.endsAtPlusInfinity())
					{
						if (merged_row.time_slice.startsAtNegativeInfinity())
						{
							merged_row.time_slice.Reshape(0, 0);
						}
						else
						{
							std::int64_t saved_start_time = merged_row.time_slice.getStart();
							merged_row.time_slice.Reshape(0, 0);
							merged_row.time_slice.setStart(saved_start_time);
						}
					}
					else
					{
						merged_row.time_slice.setEnd(the_slice.getEnd());
					}
				});

				if (failed || CheckCancelled()) { return; }

				// Intersection now contains all previous rows that matched with incoming rows,
				// and they have been properly extended.

				ongoing_merged_rows.clear();


				MergedTimeSliceRow_RHS_wins = true; // optimizer might call operator=() during "insert"

				// Set ongoing to "intersection"
				ongoing_merged_rows.insert(intersection.cbegin(), intersection.cend());

				// Now, add any new rows that were not present previously...
				// these are new rows from this time slice that did not intersect the previous rows
				// in terms of data columns.
				ongoing_merged_rows.insert(only_new.cbegin(), only_new.cend());

				// The rows that existed previously, but do not match, now must be set aside and saved.
				saved_historic_rows.insert(only_previous.cbegin(), only_previous.cend());

				MergedTimeSliceRow_RHS_wins = false;

				allWeightings.PurgeTags<ongoing_consolidation_tag>();
				allWeightings.ClearTopLevelTag<tag__ongoing_consolidation>();
				allWeightings.ClearTopLevelTag<tag__ongoing_consolidation_vector>();

			}

			else
			{

				// There is a gap in the time slices.
				// Start fresh

				saved_historic_rows.insert(ongoing_merged_rows.cbegin(), ongoing_merged_rows.cend());

				ongoing_merged_rows.clear();

				std::for_each(variableGroupBranchesAndLeavesVector.cbegin(), variableGroupBranchesAndLeavesVector.cend(), [&](VariableGroupBranchesAndLeaves const & variableGroupBranchesAndLeaves)
				{

					if (failed || CheckCancelled()) { return; }

					std::for_each(variableGroupBranchesAndLeaves.branches.cbegin(), variableGroupBranchesAndLeaves.branches.cend(), [&](Branch const & branch)
					{

						if (failed || CheckCancelled()) { return; }

						MergedTimeSliceRow_RHS_wins = true; // optimizer might call operator=() during "insert"

						if (random_sampling)
						{

							if (allWeightings.time_granularity == TIME_GRANULARITY__NONE)
							{

								// Special case for random sampling: all data is in hits[-1], rather than hits_consolidated
								auto const & incoming_rows = branch.hits[-1];
								MergedTimeSliceRow_RHS_wins = true; // optimizer might call operator=() during "insert"
								std::for_each(incoming_rows.cbegin(), incoming_rows.cend(), [&](BranchOutputRow<hits_tag> const & incoming_row)
								{
									if (failed || CheckCancelled()) { return; }

									EmplaceIncomingRowFromTimeSliceBranchDuringConsolidation(allWeightings, branch, incoming_row, ongoing_merged_rows, the_slice, orig_row_count);
									meter.UpdateProgressBarValue(orig_row_count);
								});

							}
							else
							{

								// In this case, for optimization, the rows for each branch are stored in branch.hits_consolidated
								auto const & incoming_rows = branch.hits_consolidated;
								std::for_each(incoming_rows.cbegin(), incoming_rows.cend(), [&](BranchOutputRow<hits_consolidated_tag> const & incoming_row)
								{
									if (failed || CheckCancelled()) { return; }

									EmplaceIncomingRowFromTimeSliceBranchDuringConsolidation(allWeightings, branch, incoming_row, ongoing_merged_rows, the_slice, orig_row_count);
									meter.UpdateProgressBarValue(orig_row_count);
								});
								MergedTimeSliceRow_RHS_wins = false;

							}

						}
						else
						{

							// In this case, full sampling, the rows were already added to branch.hits[-1] in consolidated fashion within each branch
							auto const & incoming_rows = branch.hits[-1];
							MergedTimeSliceRow_RHS_wins = true; // optimizer might call operator=() during "insert"
							std::for_each(incoming_rows.cbegin(), incoming_rows.cend(), [&](BranchOutputRow<hits_tag> const & incoming_row)
							{
								if (failed || CheckCancelled()) { return; }

								EmplaceIncomingRowFromTimeSliceBranchDuringConsolidation(allWeightings, branch, incoming_row, ongoing_merged_rows, the_slice, orig_row_count);
								meter.UpdateProgressBarValue(orig_row_count);
							});

						}

						MergedTimeSliceRow_RHS_wins = false;

					});

				});

			}

			previousTimeSlice = the_slice;

		});

	}

	// Empty out the current ongoing rows; we've hit the end
	MergedTimeSliceRow_RHS_wins = true; // optimizer might call operator=() during "insert"
	saved_historic_rows.insert(ongoing_merged_rows.cbegin(), ongoing_merged_rows.cend());
	MergedTimeSliceRow_RHS_wins = false;

	// Do not clear!! Let the Boost Pool management handle this, just below
	//ongoing_merged_rows.clear();
	allWeightings.PurgeTags<ongoing_merged_rows_tag>();
	allWeightings.ClearTopLevelTag<tag__ongoing_merged_rows>();

	// Currently, saved_historic_rows is just a reference to allWeightings.consolidated_rows...
	// so do not enter the following block.
	// But if we return saved_historic_rows to being a separate, local data structure, then re-enable the following block.
	if (false)
	{
		allWeightings.consolidated_rows.clear(); // This should be empty anyways

		// Order the rows how we want them - first by time, then by keys

		MergedTimeSliceRow_RHS_wins = true; // operator=() can be called from ctor during "insert"
		allWeightings.consolidated_rows.insert(saved_historic_rows.cbegin(), saved_historic_rows.cend());
		MergedTimeSliceRow_RHS_wins = false;
	}

	allWeightings.PurgeTags<saved_historic_rows_tag>();
	allWeightings.ClearTopLevelTag<tag__saved_historic_rows>();

}

void OutputModel::OutputGenerator::KadSamplerWriteResultsToFileOrScreen(KadSampler & allWeightings)
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
		SetFailureErrorMessage(msg.str());
		failed = true;
		return;
	}

	BOOST_SCOPE_EXIT(&output_file)
	{
		output_file.flush();
		output_file.close();
	} BOOST_SCOPE_EXIT_END


	// Write columns headers
	int column_index = 0;
	bool first = true;
	std::for_each(final_result.second.column_definitions.begin(),
				  final_result.second.column_definitions.end(), [this, &output_file, &first, &column_index](NewGeneSchema::NewGeneColumnDefinition & unformatted_column)
	{

		if (failed || CheckCancelled())
		{
			return;
		}

		++column_index;

		if (column_index >= static_cast<int>(final_result.second.column_definitions.size()) - 1)
		{
			if (!display_absolute_time_columns)
			{
				return;
			}
		}

		if (!first)
		{
			output_file << ",";
		}

		first = false;

		if (column_index == static_cast<int>(final_result.second.column_definitions.size()) - 1)
		{
			// starting date / time of time slice
			output_file << "row_starts_at";
		}
		else if (column_index == static_cast<int>(final_result.second.column_definitions.size()))
		{
			// ending date / time of time slice
			output_file << "row_ends_at";
		}
		else
		{
			output_file << unformatted_column.column_name_in_original_data_table;

			if (unformatted_column.total_outer_multiplicity__in_total_kad__for_current_dmu_category__for_current_variable_group > 1)
			{
				output_file << "_";
				output_file << boost::lexical_cast<std::string>
							(unformatted_column.current_multiplicity__of__this_column__in__output__same_as__current_multiplicity__of___this_column__in_its_own_variable_group);
			}
		}

	});
	output_file << std::endl;


	std::int64_t rows_written = 0;

	if (consolidate_rows)
	{

		// In this case, "ConsolidateRows()" has already been called,
		// and has already populate "consolidated_rows".
		// "consolidated_rows" contains an EXACT representation of the output (except for explicit timerange columns),
		// column-for-column in the correct order, with blanks populated properly,
		// including all primary, top-level, and child secondary data
		// (as well as all primary key data for both branch and leaves),
		// and including all leaves.

		ProgressBarMeter meter(messager, std::string("%1% / %2% rows written to output file"), allWeightings.consolidated_rows.size());
		std::for_each(allWeightings.consolidated_rows.cbegin(), allWeightings.consolidated_rows.cend(), [&](decltype(allWeightings.consolidated_rows)::value_type const & output_row)
		{

			if (failed || CheckCancelled())
			{
				return;
			}

			bool first = true;
			std::for_each(output_row.output_row.cbegin(), output_row.output_row.cend(), [&](InstanceData const & data)
			{

				if (failed || CheckCancelled())
				{
					return;
				}

				if (!first)
				{
					output_file << ",";
				}

				first = false;

				static std::string converted_value;
				converted_value = boost::lexical_cast<std::string>(data);

				if (converted_value.find(",") != std::string::npos)
				{
					// Comma in data: must surround entire field with double quotes
					// Note: NewGene currently does not support double quote inside field
					converted_value = ("\"" + converted_value + "\"");
				}

				output_file << data;

			});

			if (failed || CheckCancelled())
			{
				return;
			}

			if (display_absolute_time_columns)
			{
				if (!first)
				{
					output_file << ",";
				}

				first = false;
				output_file << output_row.time_slice.toStringStart().c_str();
				output_file << ",";
				output_file << output_row.time_slice.toStringEnd().c_str();
			}

			output_file << std::endl;

			++rows_written;
			meter.UpdateProgressBarValue(rows_written);

		});

	}
	else
	{

		// We must display the results with one row per time unit corresponding to
		// the time granularity of the primary variable group
		// (or sub-time-unit, in case child data has split rows into pieces).
		// The proper splitting of rows has already occurred.

		TIME_GRANULARITY time_granularity = top_level_variable_groups_vector[primary_vg_index__in__top_level_vg_vector].first.time_granularity;

		std::int64_t total_number_output_rows = 0;

		// Do a first pass to calculate the number of output rows that will be generated
		// ... this is for the progress bar
		std::for_each(allWeightings.timeSlices.begin(), allWeightings.timeSlices.end(), [&](TimeSlices<hits_tag>::value_type const & timeSliceData)
		{

			if (failed || CheckCancelled())
			{
				return;
			}

			TimeSlice const & timeSlice = timeSliceData.first;
			VariableGroupTimeSliceData const & variableGroupTimeSliceData = timeSliceData.second;
			VariableGroupBranchesAndLeavesVector<hits_tag> const & variableGroupBranchesAndLeavesVector = *variableGroupTimeSliceData.branches_and_leaves;

			if (variableGroupBranchesAndLeavesVector.size() > 1)
			{
				boost::format msg("Only one top-level variable group is currently supported for the random and full sampler in ConsolidateHits().");
				throw NewGeneException() << newgene_error_description(msg.str());
			}

			VariableGroupBranchesAndLeaves const & variableGroupBranchesAndLeaves = variableGroupBranchesAndLeavesVector[0];
			Branches<hits_tag> const & branchesAndLeaves = variableGroupBranchesAndLeaves.branches;
			std::for_each(branchesAndLeaves.cbegin(), branchesAndLeaves.cend(), [&](Branch const & branch)
			{

				if (failed || CheckCancelled())
				{
					return;
				}

				if (!timeSlice.hasTimeGranularity())
				{
					// If the time slice has no time range granularity, just spit out the rows once
					for (auto const & time_unit_hit : branch.hits)
					{
						if (failed || CheckCancelled()) { break; }

						// *********************************************************************************** //
						// There should be only one hit unit at index -1
						// ... so this loop should only be entered once
						// *********************************************************************************** //
						auto const & output_rows_for_this_full_time_slice = time_unit_hit.second;
						total_number_output_rows += static_cast<std::int64_t>(output_rows_for_this_full_time_slice.size());
					}
				}
				else
				{
					if (random_sampling)
					{
						// granulated output, random sampling
						for (auto const & time_unit_hit : branch.hits)
						{
							if (failed || CheckCancelled()) { break; }

							auto const & output_rows_for_this_time_unit = time_unit_hit.second;
							total_number_output_rows += static_cast<std::int64_t>(output_rows_for_this_time_unit.size());
						}
					}
					else
					{

						auto const & output_rows_for_this_full_time_slice = branch.hits[-1];
						size_t number_rows_this_time_slice = output_rows_for_this_full_time_slice.size();

						// granulated output, full sampling
						timeSlice.loop_through_time_units(time_granularity, boost::function<void(std::int64_t const, std::int64_t const)>([&](std::int64_t const time_to_use_for_start,
														  std::int64_t const time_to_use_for_end)
						{
							if (failed || CheckCancelled()) { return; }

							total_number_output_rows += static_cast<std::int64_t>(number_rows_this_time_slice);
						}));

					}
				}

			});

		});

		int which_time_slice = 0;

		ProgressBarMeter meter(messager, std::string("%1% / %2% rows written to output file"), total_number_output_rows);
		std::for_each(allWeightings.timeSlices.begin(), allWeightings.timeSlices.end(), [&](TimeSlices<hits_tag>::value_type const & timeSliceData)
		{

			++which_time_slice;

			if (failed || CheckCancelled())
			{
				return;
			}

			TimeSlice const & timeSlice = timeSliceData.first;
			VariableGroupTimeSliceData const & variableGroupTimeSliceData = timeSliceData.second;

			VariableGroupBranchesAndLeavesVector<hits_tag> const & variableGroupBranchesAndLeavesVector = *variableGroupTimeSliceData.branches_and_leaves;

			// For now, assume only one variable group
			if (variableGroupBranchesAndLeavesVector.size() > 1)
			{
				boost::format msg("Only one top-level variable group is currently supported for the random and full sampler in ConsolidateHits().");
				throw NewGeneException() << newgene_error_description(msg.str());
			}

			VariableGroupBranchesAndLeaves const & variableGroupBranchesAndLeaves = variableGroupBranchesAndLeavesVector[0];
			Branches<hits_tag> const & branchesAndLeaves = variableGroupBranchesAndLeaves.branches;

			std::for_each(branchesAndLeaves.cbegin(), branchesAndLeaves.cend(), [&](Branch const & branch)
			{

				if (failed || CheckCancelled())
				{
					return;
				}

				// ***************************************************************************************** //
				// The output should *not* be consolidated -
				// i.e., the output should be displayed with one row per
				// unit time granularity (i.e., one row per day if the time granularity
				// of the primary variable group is "day"),
				// or with one row per *sub*-time-unit if child variable groups
				// have split time units apart in that way.
				//
				// In the case of full sampling, the "hits" data structure described in the previous block
				// is not populated in multiple elements.
				//
				// Instead, all data for the branch in the "full sampling" case is stowed away
				// in a single "hits" element (with special-case index -1).
				// Our job here is to display possibly *multiple* rows, each identical,
				// for every output row in this single "hits" element with index -1.
				//
				// Each such duplicated output row is identical in every column except for the
				// time-slice-start and time-slice-end columns, which properly walk through the full
				// range of the time slice for the branch, but in steps of the time unit
				// corresponding to the time granularity of the primary variable group.
				// If the time slice for this branch is a *sub*-time-unit, just display one row,
				// and the proper sub-time-unit time width for the time-slice-start and time-slice-end
				// columns will be set to those of the branch and automatically correctly reflect
				// the sub-time-unit represented by this branch.
				// ***************************************************************************************** //


				if (!timeSlice.hasTimeGranularity())
				{

					// If the time slice has no time range granularity, just spit out the rows once

					// *********************************************************************************** //
					// There should be only one hit unit at index -1
					// *********************************************************************************** //

					//auto const & time_unit_hit = branch.hits[-1];
					auto const & output_rows_for_this_full_time_slice = branch.hits[-1];
					std::for_each(output_rows_for_this_full_time_slice.cbegin(), output_rows_for_this_full_time_slice.cend(), [&](BranchOutputRow<hits_tag> const & outputRow)
					{

						// We have a row to output

						if (failed || CheckCancelled())
						{
							return;
						}

						create_output_row_visitor::mode = create_output_row_visitor::CREATE_ROW_MODE__OUTPUT_FILE;
						create_output_row_visitor::output_file = &output_file;
						bool first = CreateOutputRow(branch, outputRow, allWeightings);
						create_output_row_visitor::output_file = nullptr;
						create_output_row_visitor::mode = create_output_row_visitor::CREATE_ROW_MODE__NONE;

						if (display_absolute_time_columns)
						{
							if (!first)
							{
								output_file << ",";
							}

							first = false;
							output_file << timeSlice.toStringStart().c_str();
							output_file << ",";
							output_file << timeSlice.toStringEnd().c_str();
						}

						output_file << std::endl;

						++rows_written;
						meter.UpdateProgressBarValue(rows_written);

					});

				}
				else
				{

					if (random_sampling)
					{

						// random sampling; not consolidated

						// *********************************************************************************** //
						// There should be multiple "hits";
						// i.e., sub-time-units, each spanning
						// an exact amount of time corresponding to the time granularity
						// of the primary variable group's unit of analysis,
						// and each starting and ending exactly on an absolute timestamp
						// that is an integer multiple of such time widths relative to the
						// Unix timestamp 0-point (Jan 1, 1970 less than a moment past
						// the tick of midnight leading into that day).
						// This includes all necessary adjustments for leap years and leap seconds -
						// the calculation internal to "loop_through_time_units" ensures that.
						//
						// Each such "hit" contains 0, 1, or more output rows that have
						// been randomly sampled from the total set of available rows
						// for this particular branch.
						//
						// All available rows are the same for each sub-time-unit,
						// so if this were sampling 100% of random rows,
						// then the set of output rows for each sub-time-unit in this branch
						// would be identical.
						//
						// But for the general case of random sampling,
						// only a random subset (properly randomly distributed)
						// from each sub-time-unit will be selected.
						//
						// This approach guarantees that the random rows generated in the output
						// are weighted according to their time window.
						// *********************************************************************************** //

						auto & hits = branch.hits;
						std::int64_t hit_number = 0;
						timeSlice.loop_through_time_units(time_granularity, boost::function<void(std::int64_t const, std::int64_t const)>([&](std::int64_t const time_to_use_for_start,
														  std::int64_t const time_to_use_for_end)
						{
							if (failed || CheckCancelled()) { return; }

							//auto const & time_unit_hit = hits[hit_number];
							auto const & output_rows_for_this_time_unit = hits[hit_number];
							TimeSlice current_slice(time_to_use_for_start, time_to_use_for_end);
							OutputGranulatedRow(current_slice, output_rows_for_this_time_unit, output_file, branch, allWeightings, rows_written);
							meter.UpdateProgressBarValue(rows_written);
							++hit_number;
						}));

					}
					else
					{

						// full sampling; not consolidated

						// *********************************************************************************** //
						// There should be only one hit unit at index -1,
						// representing the full set of available output rows
						// for the given branch.
						// *********************************************************************************** //

						auto const & output_rows_for_this_full_time_slice = branch.hits[-1];
						timeSlice.loop_through_time_units(time_granularity, boost::function<void(std::int64_t const, std::int64_t const)>([&](std::int64_t const time_to_use_for_start,
														  std::int64_t const time_to_use_for_end)
						{
							if (failed || CheckCancelled()) { return; }

							TimeSlice current_slice(time_to_use_for_start, time_to_use_for_end);
							OutputGranulatedRow(current_slice, output_rows_for_this_full_time_slice, output_file, branch, allWeightings, rows_written);
							meter.UpdateProgressBarValue(rows_written);
						}));

					}
				}

			});

		});

	}

	allWeightings.rowsWritten = rows_written;

}

void OutputModel::OutputGenerator::ConsolidateRowsWithinSingleTimeSlicesAcrossTimeUnits(KadSampler & allWeightings)
{

	// ************************************************************************************************************* //
	// This is a necessary preparatory phase, internal to each branch within individual time slices,
	// before identical rows can be merged across adjacent time slices.
	//
	// In the random sampling case, the output rows that are stored inside each branch
	// are further separated (binned) into fixed-width time units.
	// Identical output rows can appear in different bins (but only once within each bin).
	// Rows are identical if their leaves are identical (this guarantees that all secondary
	// data will be identical), regardless of which bin within the given branch they lie in.
	//
	// In "Consolidate" mode, we consolidate the output into as few rows as possible.
	// This means that if adjacent rows (time-wise) contain identical data except for the time window,
	// we will merge these into a single row with the combined time window.
	//
	// Because rows in different bins with identical leaves are identical within a branch,
	// here we simply take advantage of std::set's uniqueness guarantee by looping through
	// all bins (time units) and inserting all output rows into a single set within the branch.
	//
	// The "time unit" index given to this set has the special value of -1.
	// Because inserts of duplicate data into the set are rejected, this effectively merges
	// all output rows across time units (bins) within each branch into a single set of rows
	// for each branch.
	//
	// Note that even though each row corresponds only to a single time unit within a time slice,
	// not to the entire time range of the time slice, that when we output the results
	// in *consolidated* mode, we expect to see output rows whose time range matches
	// that of the actual raw data.  Therefore,
	// each row of output always corresponds to the *full* time range of the time slice,
	// not to the time range of the individual bin (time unit).  These bins were only used
	// to ensure that the random sampling is unbiased, not to indicate that the row
	// only corresponds to the time range of the individual bin.
	//
	// Therefore, in consolidated mode,
	// we simply wipe out the time ranges associated with the bins, merge
	// all rows (regardless of the bin of origin within the time slice) into a single
	// bin with index -1, and the output routine will pro
	//
	// Once this is complete, we are ready to move on to the main consolidation phase
	// ... namely, merging identical rows *across* time slices.
	// ************************************************************************************************************* //

	// First, calculate the number of rows that will be consolidated,
	// for use by the progress bar
	std::int64_t total_rows = 0;
	std::for_each(allWeightings.timeSlices.cbegin(), allWeightings.timeSlices.cend(), [&](decltype(allWeightings.timeSlices)::value_type const & timeSlice)
	{

		TimeSlice const & the_slice = timeSlice.first;
		VariableGroupTimeSliceData const & variableGroupTimeSliceData = timeSlice.second;
		VariableGroupBranchesAndLeavesVector<hits_tag> const & variableGroupBranchesAndLeaves = *variableGroupTimeSliceData.branches_and_leaves;

		std::for_each(variableGroupBranchesAndLeaves.cbegin(), variableGroupBranchesAndLeaves.cend(), [&](VariableGroupBranchesAndLeaves const & variableGroupBranchesAndLeaves)
		{
			std::for_each(variableGroupBranchesAndLeaves.branches.cbegin(), variableGroupBranchesAndLeaves.branches.cend(), [&](Branch const & the_branch)
			{
				std::for_each(the_branch.hits.begin(), the_branch.hits.end(), [&](decltype(the_branch.hits)::value_type & hit)
				{
					if (hit.first != -1)
					{
						total_rows += hit.second.size();
					}
				});
			});

		});

	});

	// ************************************************************************************************************* //
	// Now actually perform the consolidation across time units within individual time slices
	// ************************************************************************************************************* //
	ProgressBarMeter meter(messager, std::string("Preparing internal time slice rows for consolidation: %1% / %2%"), total_rows);
	std::int64_t current_rows = 0;
	std::for_each(allWeightings.timeSlices.cbegin(), allWeightings.timeSlices.cend(), [&](decltype(allWeightings.timeSlices)::value_type const & timeSlice)
	{

		if (failed || CheckCancelled())
		{
			return;
		}

		TimeSlice const & the_slice = timeSlice.first;
		VariableGroupTimeSliceData const & variableGroupTimeSliceData = timeSlice.second;
		VariableGroupBranchesAndLeavesVector<hits_tag> const & variableGroupBranchesAndLeaves = *variableGroupTimeSliceData.branches_and_leaves;

		std::for_each(variableGroupBranchesAndLeaves.cbegin(), variableGroupBranchesAndLeaves.cend(), [&](VariableGroupBranchesAndLeaves const & variableGroupBranchesAndLeaves)
		{

			if (failed || CheckCancelled())
			{
				return;
			}

			std::for_each(variableGroupBranchesAndLeaves.branches.cbegin(), variableGroupBranchesAndLeaves.branches.cend(), [&](Branch const & the_branch)
			{
				if (failed || CheckCancelled())
				{
					return;
				}

				allWeightings.ConsolidateRowsWithinBranch(the_branch, current_rows, meter);
			});

		});

	});

}
