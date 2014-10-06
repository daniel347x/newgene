#ifndef OUTPUTMODEL_H
#define OUTPUTMODEL_H

#include "Model.h"
#include "../Settings/OutputModelSettings.h"
#include "../Settings/InputModelSettings.h"
#include "InputModel.h"
#include "../Settings/Setting.h"
#include "Tables/TableManager.h"
#include <memory>
#include <tuple>
#include <cstdint>
#include <atomic>
#include <mutex>
#include <fstream>
#include <set>
#include <string>
#include <list>
#include "./KadSampler/KadSampler.h"

class PrimaryKeySequence
{

	public:

		class VariableGroup_PrimaryKey_Info
		{

			public:

				VariableGroup_PrimaryKey_Info()
					: sequence_number_within_dmu_category_for_this_variable_groups_uoa(-1) // will only be populated later if exists
					, current_outer_multiplicity_of_this_primary_key__in_relation_to__the_uoa_corresponding_to_the_current_variable_group___same_as___current_inner_table_number_within_the_inner_table_set_corresponding_to_the_current_variable_group(
						-1) // will be populated later
					, total_outer_multiplicity__in_total_kad__for_current_dmu_category__for_current_variable_group(0) // will only be populated later if exists
					, total_outer_multiplicity__for_the_current_dmu_category__corresponding_to_the_uoa_corresponding_to_top_level_variable_group(-1) // will be populated later
					, total_number_columns_for_dmu_category__internal_to_uoa_corresponding_to_this_variable_group(0) // will only be populated later if exists
					, total_number_columns_for_dmu_category__internal_to_the_uoa_corresponding_to_primary_uoa_for_the_same_dmu_category(-1) // will be populated later
					, is_primary_column_selected(false)
				{}

			public:

				WidgetInstanceIdentifier vg_identifier;
				WidgetInstanceIdentifier associated_uoa_identifier;
				std::string column_name;
				std::string column_name_no_uuid;
				std::string table_column_name;

				// Regarding the three integer variables below,
				// their purpose is best explained by providing an example.
				//
				// Primary UOA (i.e., the "largest" unit of analysis corresponding to the variable groups with variables selected by the user):
				// A B B B B C C
				//
				// Selected K-ad (i.e., the spin control values chosen by the user):
				// A B B B B B B B B C C
				//
				// Variable group's UOA (were the particular choice given in this example allowed, as it could be in the future) (i.e., a given variable group, which might not be the "largest"):
				// A B B
				//
				// ... then, the primary key sequence will include eight entries for "B"
				// And for the given variable group, we have,
				// for the following three integers for each of the eight "B"'s:
				// B    B    B    B    B    B    B    B
				// ^114 ^214 ^124 ^224 ^134 ^234 ^144 ^244
				//
				int sequence_number_within_dmu_category_for_this_variable_groups_uoa;
				int current_outer_multiplicity_of_this_primary_key__in_relation_to__the_uoa_corresponding_to_the_current_variable_group___same_as___current_inner_table_number_within_the_inner_table_set_corresponding_to_the_current_variable_group;
				int total_outer_multiplicity__in_total_kad__for_current_dmu_category__for_current_variable_group;
				int total_outer_multiplicity__for_the_current_dmu_category__corresponding_to_the_uoa_corresponding_to_top_level_variable_group;
				int total_number_columns_for_dmu_category__internal_to_uoa_corresponding_to_this_variable_group;
				int total_number_columns_for_dmu_category__internal_to_the_uoa_corresponding_to_primary_uoa_for_the_same_dmu_category;

				bool is_primary_column_selected;

		};

		class PrimaryKeySequenceEntry
		{

			public:

				PrimaryKeySequenceEntry()
					: sequence_number_within_dmu_category_spin_control { -1 }
				, sequence_number_within_dmu_category_primary_uoa { -1 }
				, sequence_number_in_all_primary_keys__of__global_primary_key_sequence_metadata__NOT__of_order_columns_appear_in_top_level_vg { -1 }
				, sequence_number_in_all_primary_keys__of__order_columns_appear_in_top_level_vg { -1 }
				, total_k_count_within_high_level_variable_group_uoa_for_this_dmu_category { -1 }
				, total_kad_spin_count_for_this_dmu_category { -1 }
				, total_outer_multiplicity__for_the_current_dmu_category__corresponding_to_the_uoa_corresponding_to_top_level_variable_group { -1 }
				{}

				WidgetInstanceIdentifier dmu_category;
				int sequence_number_within_dmu_category_spin_control;
				int sequence_number_within_dmu_category_primary_uoa;
				int sequence_number_in_all_primary_keys__of__global_primary_key_sequence_metadata__NOT__of_order_columns_appear_in_top_level_vg;
				int sequence_number_in_all_primary_keys__of__order_columns_appear_in_top_level_vg;
				int total_k_count_within_high_level_variable_group_uoa_for_this_dmu_category;
				int total_kad_spin_count_for_this_dmu_category;
				int total_outer_multiplicity__for_the_current_dmu_category__corresponding_to_the_uoa_corresponding_to_top_level_variable_group;
				std::vector<VariableGroup_PrimaryKey_Info> variable_group_info_for_primary_keys__top_level_and_child; // one per variable group.  Includes child variable groups.

		};

		std::vector<PrimaryKeySequenceEntry> primary_key_sequence_info; // one per primary key, in sequence

};

class ColumnsInTempView
{

	public:

		enum SCHEMA_TYPE
		{
			SCHEMA_TYPE__UNKNOWN
			, SCHEMA_TYPE__RAW__SELECTED_VARIABLES_PRIMARY
			, SCHEMA_TYPE__RAW__SELECTED_VARIABLES_TOP_LEVEL_NOT_PRIMARY
			, SCHEMA_TYPE__RAW__SELECTED_VARIABLES_CHILD
			, SCHEMA_TYPE__DEDUCED
		};

		class ColumnInTempView
		{

			public:

				enum COLUMN_TYPE
				{
					COLUMN_TYPE__UNKNOWN = 0
					, COLUMN_TYPE__PRIMARY
					, COLUMN_TYPE__SECONDARY
					, COLUMN_TYPE__DATETIMESTART
					, COLUMN_TYPE__DATETIMEEND
					, COLUMN_TYPE__DATETIMESTART_INTERNAL
					, COLUMN_TYPE__DATETIMEEND_INTERNAL
					, COLUMN_TYPE__DATETIMESTART__TIME_SLICE
					, COLUMN_TYPE__DATETIMEEND__TIME_SLICE
				};

				ColumnInTempView()
					: column_type(COLUMN_TYPE__UNKNOWN)
					, primary_key_index_within_total_kad_for_dmu_category(-1)
					, primary_key_index__within_uoa_corresponding_to_current_variable_group(-1)
					, primary_key_index_within_primary_uoa_for_dmu_category(-1)
					, current_multiplicity__of__this_column__in__output__same_as__current_multiplicity__of___this_column__in_its_own_variable_group(-1)
					, total_outer_multiplicity__in_total_kad__for_current_dmu_category__for_current_variable_group(-1)
					, total_k_count__within_uoa_corresponding_to_current_variable_group__for_current_dmu_category(-1)
					, total_k_count__within_uoa_corresponding_to_top_level_variable_group__for_current_dmu_category(-1)
					, total_k_spin_count_across_multiplicities_for_dmu_category(-1)
					, primary_key_should_be_treated_as_integer_____float_not_allowed_as_primary_key(false)
					, originally_datetime(false)
				{

				}

				std::string column_name_in_temporary_table; // The name of the column in the temporary view (currently does NOT always include UUID, but it can)
				std::string column_name_in_temporary_table_no_uuid; // The name of the column in the temporary view (without UUID)
				std::string column_name_in_original_data_table; // The name of the column in the original raw data table corresponding to this column (if any)
				WidgetInstanceIdentifier current_variable_group; // The variable group associated with this column
				WidgetInstanceIdentifier
				uoa_associated_with_current_variable_group; // The unit of analysis associated with the variable group associated with this column
				COLUMN_TYPE column_type; // Primary key?  Secondary column (i.e., data, not a primary key)?  One of the two special DateTime columns?
				WidgetInstanceIdentifier primary_key_dmu_category_identifier; // Empty if this column is not a primary key column

				// For both primary and child inner tables,
				// spans multiple inner tables; i.e., the first inner table starts at 0,
				// the next inner table starts higher than 0, etc.,
				// but only for those dmu categories with multiplicity greater than 1
				// (note that the SAME dmu category always has multiplicity greater than 1
				// for both child and primary variable groups,
				// except that if all DMU categories have multiplicity of 1 for primary VG's,
				// the child can have multiplicity greater than 1 for a single such VG).
				// Other dmu categories (those with multiplicity of 1) reset to 0 for each inner table.
				// Resets to 0 for each group of inner tables corresponding to a new child variable group.
				// Note that child variable groups may increment differently across inner tables
				// than primary variable groups.
				int primary_key_index_within_total_kad_for_dmu_category;

				int primary_key_index__within_uoa_corresponding_to_current_variable_group;
				int primary_key_index_within_primary_uoa_for_dmu_category;
				int current_multiplicity__of__this_column__in__output__same_as__current_multiplicity__of___this_column__in_its_own_variable_group;
				int total_outer_multiplicity__in_total_kad__for_current_dmu_category__for_current_variable_group;
				int total_k_count__within_uoa_corresponding_to_current_variable_group__for_current_dmu_category;
				int total_k_count__within_uoa_corresponding_to_top_level_variable_group__for_current_dmu_category;
				int total_k_spin_count_across_multiplicities_for_dmu_category;
				bool primary_key_should_be_treated_as_integer_____float_not_allowed_as_primary_key;
				bool originally_datetime;

		};

		ColumnsInTempView()
			: view_number(-1)
			, has_no_datetime_columns(false)
			, has_no_datetime_columns_originally(false)
			, most_recent_sql_statement_executed__index(-1)
			, make_table_permanent(false)
			, not_first_variable_group_column_index(-1)
			, schema_type(SCHEMA_TYPE__UNKNOWN)
		{

		}

		ColumnsInTempView(ColumnsInTempView const & rhs)
			: columns_in_view(rhs.columns_in_view)
			, view_number(rhs.view_number)
			, has_no_datetime_columns(rhs.has_no_datetime_columns)
			, has_no_datetime_columns_originally(rhs.has_no_datetime_columns_originally)
			, original_table_names(rhs.original_table_names)
			, variable_group_codes(rhs.variable_group_codes)
			, variable_group_longhand_names(rhs.variable_group_longhand_names)
			, variable_groups(rhs.variable_groups)
			, view_name(rhs.view_name)
			, view_name_no_uuid(rhs.view_name_no_uuid)
			, previous_block_datetime_column_types(rhs.previous_block_datetime_column_types)
			, current_block_datetime_column_types(rhs.current_block_datetime_column_types)
			, most_recent_sql_statement_executed__index(rhs.most_recent_sql_statement_executed__index)
			, make_table_permanent(rhs.make_table_permanent)
			, schema_type(rhs.schema_type)
		{
			// For optimization variables
			// Force recalculation of this
			not_first_variable_group_column_index = -1;
		}

		std::vector<ColumnInTempView> columns_in_view;
		int view_number;
		bool has_no_datetime_columns;
		bool has_no_datetime_columns_originally;
		std::vector<std::string> original_table_names;
		std::vector<std::string> variable_group_codes;
		std::vector<std::string> variable_group_longhand_names;
		std::vector<WidgetInstanceIdentifier> variable_groups;
		std::string view_name;
		std::string view_name_no_uuid;

		std::pair<ColumnInTempView::COLUMN_TYPE, ColumnInTempView::COLUMN_TYPE> previous_block_datetime_column_types;
		std::pair<ColumnInTempView::COLUMN_TYPE, ColumnInTempView::COLUMN_TYPE> current_block_datetime_column_types;

		int most_recent_sql_statement_executed__index;

		bool make_table_permanent;

		SCHEMA_TYPE schema_type;

		// optimizations
		mutable int not_first_variable_group_column_index;

};

class OutputModel : public Model<OUTPUT_MODEL_SETTINGS_NAMESPACE::OUTPUT_MODEL_SETTINGS>
{

	public:

		typedef std::pair<std::vector<std::string>, std::vector<std::string>> PrimaryKey_SecondaryKey_Names;
		typedef std::vector<PrimaryKey_SecondaryKey_Names> VariableGroup__PrimaryKey_SecondaryKey_Names__Vector;

		OutputModel(Messager & messager, boost::filesystem::path const path_to_model_database, std::shared_ptr<InputModelSettings> const & input_model_settings_,
					std::shared_ptr<InputModel> const & input_model_)
			: Model(messager, path_to_model_database)
			, input_model_settings(input_model_settings_)
			, input_model(input_model_)
		{

		}

		std::string GetCreationSQL();

		InputModelSettings & getInputModelSettings()
		{
			return *input_model_settings;
		}

		InputModel & getInputModel()
		{
			return *input_model;
		}

		void LoadTables();

		Table_VARIABLES_SELECTED t_variables_selected_identifiers;
		Table_KAD_COUNT t_kad_count;
		Table_TIME_RANGE t_time_range;
		Table_GENERAL_OPTIONS t_general_options;
		Table__Limit_DMUS__Categories t_limit_dmus_categories;
		Table__Limit_DMUs__Elements t_limit_dmus_set_members;

	protected:

		std::shared_ptr<InputModelSettings> input_model_settings;
		std::shared_ptr<InputModel> input_model;

	private:

		// helpers used by the output generator
		std::string StripUUIDFromVariableName(std::string const & variable_name);

	public:

		class OutputGenerator
		{

				// **************************************************************************************************************************************** //
				// The algorithm, in a nutshell
				//
				// For each high-level variable group (variable group corresponding to the "biggest" unit of analysis):
				//
				// (1) Create an initial so-called "X" table (labeled X1)
				// that is an exact copy of the raw data table for this variable group,
				// but that includes (in addition to all primary key columns)
				// only the user's selected variables, and that adjusts the position
				// of the time range columns (if any) to appear at the right.
				// If there are no time range columns, they are added (at the right)
				// and populated with 0.
				// Also, this table only includes those rows which overlap the time range
				// of interest selected by the user.
				// In debug mode, the results are ordered by the primary key columns (with multiplicity greater than 1).
				//
				// (2) Create an initial so-called "XR" table (labeled XR1)
				// that is an exact copy of the X1 table, but has two columns
				// appended to the right, called "merge" columns, which are
				// time range columns representing the time range of the given
				// row, after merging with previous tables.  Because this is
				// the original table, these columns are just copies of the
				// time range columns from the X1 table.
				//
				// (3) For each multiplicity greater than 1, create another "X" table
				// (labeled by the multiplicity count) which is a join of the following two tables:
				// (a) The previous "XR" table (i.e., it includes the most recent "merge" columns)
				// (b) An exact copy of columns of the raw data table, created from the raw data
				// table *exactly* as the X1 table was (i.e., adjusting/adding the time range
				// columns at the right).
				// This "X" table will therefore have the "merge" columns from the previous multiplicity
				// available, and it will have the "raw data" time range columns from the current multiplicity available.
				// The rows are sorted so that, for each row, all NULL columns in primary keys appear at the right,
				// and all non-NULL primary keys are sorted from left to right across multiplicities.
				// This sorting is necessary to avoid duplicates.
				// In debug mode, the results are ordered by the primary key columns (with multiplicity greater than 1).
				// (The final XR table will be ordered even in release mode, as this is part of a later step
				// that walks through the final result removing duplicates).
				//
				// (4) Proceed to create an "XR" table corresponding to the "X" table just created
				// (do this before moving on to the next X table in the sequence).
				// This table simply adds the "merge" columns to the "X" table.
				// However, it does so in a special way: It performs an algorithm to make
				// sure that the current data being merged into the previous data has
				// any time range overlaps handled properly, creating multiple rows if necessary,
				// which includes creating NULL values for some data on some rows to make
				// sure that all data that is displayed on a row is valid for the sub-time-range
				// generated for that row.
				//
				// (5) Move on to child variable groups.
				// **************************************************************************************************************************************** //

			public:

				OutputGenerator(Messager & messager_, OutputModel & model_, OutputProject & project_);
				~OutputGenerator();

				void GenerateOutput(DataChangeMessage & change_response);

			public:

				class SQLExecutor
				{

					public:

						enum STATEMENT_TYPE
						{
							UNKNOWN
							, RETURNS_ROWS
							, DOES_NOT_RETURN_ROWS
						};

						enum WHICH_BINDING
						{
							UNKNOWN_BINDING
							, NULL_BINDING
							, STRING
							, INT64
							, FLOAT
						};

						SQLExecutor(OutputModel::OutputGenerator * generator_, sqlite3 * db_);
						SQLExecutor(OutputModel::OutputGenerator * generator_, sqlite3 * db_, std::string const & sql_);

						static void Execute(bool statement_is_owned, OutputModel::OutputGenerator::SQLExecutor::STATEMENT_TYPE statement_type_, OutputModel::OutputGenerator * generator_, sqlite3 * db_,
											std::string const & sql_, std::vector<std::string> const & bound_parameter_strings_, std::vector<std::int64_t> const & bound_parameter_ints_,
											std::vector<long double> const & bound_parameter_floats_, std::vector<WHICH_BINDING> & bound_parameter_which_binding_to_use_, std::shared_ptr<bool> & stmt_is_prepared,
											sqlite3_stmt *& stmt_to_use, bool const prepare_statement_if_null = false);
						//SQLExecutor(OutputModel::OutputGenerator * generator_, sqlite3 * db_, std::string const & sql_, std::vector<std::string> const & bound_parameter_strings_, std::vector<std::int64_t> const & bound_parameter_ints_, std::vector<long double> const & bound_parameter_floats_, std::vector<WHICH_BINDING> & bound_parameter_which_binding_to_use_, std::shared_ptr<bool> & stmt_is_prepared, sqlite3_stmt * stmt_to_use = nullptr, bool const prepare_statement_if_null = false);

						SQLExecutor(SQLExecutor const & rhs);
						SQLExecutor(SQLExecutor && rhs);
						SQLExecutor & operator=(SQLExecutor const & rhs);
						SQLExecutor & operator=(SQLExecutor && rhs);
						void Copy(SQLExecutor const & rhs);
						void CopyOwned(SQLExecutor & rhs);
						~SQLExecutor();

						//static void Execute(sqlite3 * db, bool unused_disambiguator, OutputModel::OutputGenerator * generator, sqlite3_stmt * stmt, SQLExecutor::STATEMENT_TYPE statement_type, std::vector<std::string> const & bound_parameter_strings, std::vector<std::int64_t> const & bound_parameter_ints, std::vector<long double> const & bound_parameter_floats, std::vector<WHICH_BINDING> & bound_parameter_which_binding_to_use, bool statement_is_owned, std::shared_ptr<bool> & statement_is_prepared);
						static void DoExecute(sqlite3 * db, OutputModel::OutputGenerator * generator, sqlite3_stmt *& stmt, SQLExecutor::STATEMENT_TYPE statement_type,
											  std::vector<std::string> const & bound_parameter_strings_, std::vector<std::int64_t> const & bound_parameter_ints_, std::vector<long double> const & bound_parameter_floats_,
											  std::vector<WHICH_BINDING> & bound_parameter_which_binding_to_use_, bool statement_is_owned, std::shared_ptr<bool> & stmt_is_prepared, bool statement_is_shared);
						void Execute();
						bool Step();
						void Empty(bool const empty_sql = true); // closes prepared statement and optionally clears stored SQL

						std::string sql;
						std::string sql_error;

						sqlite3 * db;
						static sqlite3_stmt * stmt_insert;
						sqlite3_stmt * stmt;
						STATEMENT_TYPE statement_type;
						bool statement_is_owned;
						std::shared_ptr<bool> statement_is_prepared;
						bool failed;

						std::vector<std::string> bound_parameter_strings;
						std::vector<std::int64_t> bound_parameter_ints;
						std::vector<long double> bound_parameter_floats;
						std::vector<WHICH_BINDING> bound_parameter_which_binding_to_use;

						bool statement_is_shared;

						std::string error_message;

						OutputModel::OutputGenerator * generator;

					public:

						static int number_statement_prepares;
						static int number_statement_finalizes;

				};

			private:

				enum XR_TABLE_CATEGORY
				{
					PRIMARY_VARIABLE_GROUP
					, FINAL_MERGE_OF_PRIMARY_VARIABLE_GROUP
					, CHILD_VARIABLE_GROUP
					, KAD_RESULTS
					, RANDOMIZE // special case - not the same format of "XR" table
				};

				class SavedRowData
				{

					public:

						SavedRowData()
							: datetime_start(0)
							, datetime_end(0)
							, failed(false)
							, branch_has_excluded_dmu(false)
							, leaf_has_excluded_dmu(false)
						{

						}

						void Clear();
						void PopulateFromCurrentRowInDatabase(ColumnsInTempView const & column_schema, sqlite3_stmt * stmt_result, OutputModel & model);

						SavedRowData & GetSavedRowData() { return *this; }

						std::int64_t rowid; // for use with SQLite's "rowid" - currently only used for random sampling algorithm
						bool branch_has_excluded_dmu;
						bool leaf_has_excluded_dmu;

						std::int64_t datetime_start;
						std::int64_t datetime_end;

						// ************************************************************************************** //
						// TODO: switch to use Boost Variant (as is now used everywhere else)
						// ************************************************************************************** //
						std::vector<fast_string> current_parameter_strings;
						std::vector<std::int64_t> current_parameter_ints;
						std::vector<long double> current_parameter_floats;
						std::vector<SQLExecutor::WHICH_BINDING> current_parameter_which_binding_to_use;

						// ************************************************************************************************* //
						// Use these vectors to obtain the data you want.
						// First, get the binding and index into the proper bound vector, above.
						// Then, retrieve the data from the bound vector, above.
						// ************************************************************************************************* //

						// std::pair<SQLExecutor::WHICH_BINDING, std::pair<int, int>> contains the following data:
						// WHICH_BINDING, the index in the corresponding bound vector (above), and the column number

						std::vector<std::pair<SQLExecutor::WHICH_BINDING, std::pair<int, int>>> indices_of_all_columns;

						std::vector<std::pair<SQLExecutor::WHICH_BINDING, std::pair<int, int>>> indices_of_primary_key_columns;
						std::vector<std::pair<SQLExecutor::WHICH_BINDING, std::pair<int, int>>> indices_of_primary_key_columns_with_outer_multiplicity_greater_than_1; // There can be only 1 leaf loaded, even if this leaf corresponds to multiplicity > 1
						std::vector<std::pair<SQLExecutor::WHICH_BINDING, std::pair<int, int>>> indices_of_primary_key_columns_with_outer_multiplicity_equal_to_1;
						std::vector<std::pair<SQLExecutor::WHICH_BINDING, std::pair<int, int>>> indices_of_secondary_key_columns;

						std::vector<bool> is_index_a_primary_key;
						std::vector<bool> is_index_a_primary_key_with_outer_multiplicity_greater_than_1;
						std::vector<bool> is_index_a_primary_key_with_outer_multiplicity_equal_to_1;
						std::vector<bool> is_index_a_secondary_key;

						std::string error_message;

						bool failed;

				};

				typedef std::pair<std::vector<SQLExecutor>, ColumnsInTempView> SqlAndColumnSet;
				typedef std::vector<SqlAndColumnSet> SqlAndColumnSets;

				void SetFailureErrorMessage(std::string const & failure_message_);

				// Initialize generator
				void Prepare(KadSampler & allWeightings);
				void PopulateDMUCounts();
				void PopulateUOAs();
				void ValidateUOAs();
				void DetermineChildMultiplicitiesGreaterThanOne();
				void PopulateVariableGroups();
				void PopulatePrimaryKeySequenceInfo();

				size_t primary_vg_index__in__top_level_vg_vector; // in case there are multiple top-level variable groups, which one to use as primary (the others will be treated as children)
				int K; // the multiplicity

				// Random sampling
				WidgetInstanceIdentifier top_level_vg;
				bool consolidate_rows;
				bool display_absolute_time_columns;
				bool general_info;
				bool random_sampling;
				std::int64_t random_sampling_number_rows;
				bool has_non_primary_top_level_groups;
				bool has_child_groups;
				// For each child variable group, a vector of mapping from the child key columns to the top-level key columns
				std::map<int, int> childInternalToOneLeafColumnCountForDMUWithMultiplicityGreaterThan1;
				int overall_total_number_of_primary_key_columns_including_all_branch_columns_and_all_leaves_and_all_columns_internal_to_each_leaf;
				OutputModel::OutputGenerator::SqlAndColumnSet CreateTableOfSelectedVariablesFromRawData(ColumnsInTempView const & variable_group_raw_data_columns, int const group_number);
				void KadSamplerFillDataForChildGroups(KadSampler & allWeightings);
				void KadSampler_ReadData_AddToTimeSlices(ColumnsInTempView const & primary_variable_group_x1_columns, int const primary_group_number, KadSampler & allWeightings,
						VARIABLE_GROUP_MERGE_MODE const merge_mode, std::vector<std::string> & errorMessages);
				SqlAndColumnSet KadSamplerBuildOutputSchema();
				void KadSamplerCreateOutputTable();
				void PrepareInsertStatement(sqlite3_stmt *& insert_random_sample_stmt, ColumnsInTempView const & random_sampling_columns);
				void BindTermToInsertStatement(sqlite3_stmt * insert_random_sample_stmt, InstanceData const & data, int bindIndex);

				template <typename MEMORY_TAG>
				bool CreateOutputRow(Branch const & branch, BranchOutputRow<MEMORY_TAG> const & outputRow, KadSampler & allWeightings); // returns whether no columns of data were added

				void ConsolidateData(bool const random_sampling, KadSampler & allWeightings);
				void ConsolidateRowsWithinSingleTimeSlicesAcrossTimeUnits(KadSampler & allWeightings);

				template <typename MEMORY_TAG_OUTPUT_ROW, typename MEMORY_TAG_SET_OF_ROWS>
				void EmplaceIncomingRowFromTimeSliceBranchDuringConsolidation(KadSampler & allWeightings, Branch const & branch,
					BranchOutputRow<MEMORY_TAG_OUTPUT_ROW> const & incoming_row, FastSetMemoryTag<MergedTimeSliceRow<MEMORY_TAG_SET_OF_ROWS>, MEMORY_TAG_SET_OF_ROWS> & merging, TimeSlice const & the_slice, std::int64_t & orig_row_count);

				void KadSamplerWriteResultsToFileOrScreen(KadSampler & allWeightings);

				template <typename MEMORY_TAG>
				void OutputGranulatedRow(TimeSlice const & current_time_slice, fast_branch_output_row_set<MEMORY_TAG> const & output_rows_for_this_full_time_slice, std::fstream & output_file,
										 Branch const & branch,
										 KadSampler & allWeightings, std::int64_t & rows_written);

				void DetermineInternalChildLeafCountMultiplicityGreaterThanOne(KadSampler & allWeightings, ColumnsInTempView const & column_schema, int const child_variable_group_index);

				std::map<int, int> top_level_number_secondary_columns;
				std::map<int, int> child_number_secondary_columns;

				// Functions involved in different phases of generation
				void PopulateSchemaForRawDataTables(KadSampler & allWeightings);
				void PopulateSchemaForRawDataTable(std::pair<WidgetInstanceIdentifier, WidgetInstanceIdentifiers> const & the_primary_variable_group, int view_count,
												   std::vector<ColumnsInTempView> & variable_groups_column_info, bool const & is_primary, int const primary_or_secondary_view_index);

				// Helper functions used by the functions above
				void BeginNewTransaction();
				void EndTransaction();
				void ExecuteSQL(SqlAndColumnSet & sql_and_column_set);
				void ObtainData(ColumnsInTempView const & column_set, bool const obtain_rowid = false);
				void CloseObtainData();
				std::int64_t ObtainCount(ColumnsInTempView const & column_set);
				bool StepData();
				void ClearTables(SqlAndColumnSets const & tables_to_clear);
				void ClearTable(SqlAndColumnSet const & table_to_clear);
				std::string CheckOutputFileExists();

				inline static bool CheckCancelled()
				{
					// No lock - not necessary for a boolean checked multiple times by back end and that will not cause an error if it is messed up in extraordinarily rare circumstances
					return cancelled; // opportunity for further checking here
				}

				// Save the SQL and column sets corresponding to each primary and child variable group in global data structures
				//
				// "primary_variable_group_column_sets":
				// Includes all intermediate tables for each of the primary variable groups
				// (i.e., a single SqlAndColumnSets contains all of the intermediate tables,
				// each with a single SqlAndColumnSet, for a given primary variable group).
				//
				// "primary_group_final_results", on the other hand, contains
				// only 1 table per primary variable group: The final table
				// (each one corresponding to a single SqlAndColumnSet).
				//
				// "intermediate_merging_of_primary_groups_column_sets":
				// Each merging of a top-level primary variable group
				// into the previously-merged ones requires a new temporary table.
				// These are stored in this variable (one per SqlAndColumnSet).
				//
				// "all_merged_results_unformatted" contains a single table:
				// the final results - including a merge across all top-level (primary) variable groups -
				// but unformatted (i.e., with extra columns that have internally-generated column names).
				//
				// "final_result":
				// This is the final result of the K-ad generator, in a form presentable as-is
				// to the end user.
				std::vector<SqlAndColumnSets> primary_variable_group_column_sets;
				SqlAndColumnSets merging_of_children_column_sets;
				SqlAndColumnSet random_sampling_schema;
				SqlAndColumnSet final_result;

				std::string setting_path_to_kad_output;

				// ********************************************************************* //
				// the_map is:
				// Map: UOA identifier => [ map: VG identifier => list of variables ].
				// Each UOA appears at most only once (some appear none, if no variables
				// are selected for any variable group associated with the UOA).
				//
				// But, these UOA's are those defined / imported by the USER,
				// so if the USER chooses to have two UOA's that are identical,
				// they will appear separately.
				// In particular, UOA's with different TIME GRANULARITY *must* be
				// different, even with exactly the same primary keys, so it isn't
				// uncommon to see this (the time granularity is not reflected in
				// the primary keys).
				// ********************************************************************* //
				Table_VARIABLES_SELECTED::UOA_To_Variables_Map * the_map;

				// *************************************************************************** //
				// UOAs is:
				// Vector of: UOA identifier and its associated list of [DMU Category / Count].
				// Note that each UOA appears exactly once (regardless of whether variables
				// are selected by the user for multiple variable groups associated with this UOA).
				// A given UOA will NOT appear unless at least one variable is selected
				// in at least one variable group corresponding to this UOA.
				//
				// But, these UOA's are those defined / imported by the USER,
				// so if the USER chooses to have two UOA's that are identical,
				// they will appear separately.
				// In particular, UOA's with different TIME GRANULARITY *must* be
				// different, even with exactly the same primary keys, so it isn't
				// uncommon to see this (the time granularity is not reflected in
				// the primary keys).
				// **************************************************************************** //
				// Place UOA's and their associated DMU categories into a vector for convenience
				std::vector<std::pair<WidgetInstanceIdentifier, Table_UOA_Identifier::DMU_Counts>> UOAs;

				// ******************************************************************************************* //
				// biggest_counts and child_counts are each a Vector of:
				// Pair consisting of: UOA identifier and its associated list of [DMU Category / Count].
				//
				// For biggest_counts, each UOA is one that has has been determined to
				// correspond to the "primary UOA", which is defined only in terms of the primary
				// keys (not the time granularity, and not reflecting the possibility that the user
				// can define multiple, identical UOA's in terms of primary keys
				// and even time granularity).
				// Therefore, there could be multiple UOA's in biggest_counts,
				// each different but with exactly the same primary keys.
				// Also, specifically reiterating the time granularity issue, multiple UOA's can
				// be defined that have different TIME GRANULARITY, even if the primary keys
				// are exactly the same.
				// ******************************************************************************************* //
				std::vector<std::pair<WidgetInstanceIdentifier, Table_UOA_Identifier::DMU_Counts>> biggest_counts;

				// ******************************************************************************************* //
				// biggest_counts and child_counts are each a Vector of:
				// Pair consisting of: UOA identifier and its associated list of [DMU Category / Count].
				//
				// For child_counts, each UOA is enforced to be a smaller subset of the primary keys
				// in the primary UOA's that appear in biggest_counts.
				// Further, the UOA k-value for all DMU categories
				// is either 0, 1, or the corresponding UOA k-value of the primary UOA's,
				// and that where not 0, it is less than the corresponding UOA k-value of the primary UOA's
				// (i.e., has a value of 1) for only 1 DMU category,
				// and this DMU category must match the DMU category with multiplicity greater than 1 (if any)
				// for the primary UOA's.
				// As is the case with biggest_counts,
				// each UOA is different, but might have the same primary keys.
				// ******************************************************************************************* //
				std::vector<std::pair<WidgetInstanceIdentifier, Table_UOA_Identifier::DMU_Counts>> child_counts;

				// Keep track of multiplicities involved
				// The following variable has a hideously long name
				// because it is used rarely, but when it is used it is critical
				// to understand exactly what it is.
				// The order of elements in this vector exactly corresponds
				// to the order of elements in the vector corresponding to
				// each UOA in 'biggest_counts', above.
				std::vector<int>
				outer_multiplicities_primary_uoa___ie___if_there_are_3_cols_for_a_single_dmu_in_the_primary_uoa__and_K_is_12__then__this_value_is_4_for_that_DMU____note_this_is_greater_than_1_for_only_1_DMU_in_the_primary_UOA;
				int highest_multiplicity_primary_uoa;
				std::string highest_multiplicity_primary_uoa_dmu_string_code;
				bool any_primary_dmu_has_multiplicity_greater_than_1;
				int which_primary_index_has_multiplicity_greater_than_1; // corresponds to the single DMU in the primary UOA whose outer multiplicity is greater than 1; if none have outer multiplicity greater than 1, this value will be set to -1.  Corresponds to the above "outer_multiplicities_primary_uoa___ie___if_there_are_3_cols_for_a_single_dmu_in_the_primary_uoa__and_K_is_12__then__this_value_is_4_for_that_DMU____note_this_is_greater_than_1_for_only_1_DMU_in_the_primary_UOA" variable.

				// child_uoas__which_multiplicity_is_greater_than_1:
				// Map of:
				// UOA identifier => pair<DMU category identifier of the effective DMU category that, in relation to the child,
				// has multiplicity greater than 1, ... and the given multiplicity, with respect to the total spin control count>.
				// There could be multiple UOA's with identical primary keys, but they will correspond to different UOA's
				// defined / imported by the user.
				std::map<WidgetInstanceIdentifier, std::pair<WidgetInstanceIdentifier, int>> child_uoas__which_multiplicity_is_greater_than_1;

				// variable_group__key_names__vectors:
				// A vector of: pair<vector<string>, vector<string>> - each such pair represents
				// one variable group (with at least one variable selected by the user for output)
				// ... and lists the internal *column names* used by this SQL query,
				// one vector for the primary keys, and one for the non-primary (i.e., secondary) keys.
				// Note that a UUID is appended to every variable name to avoid duplicates among variable groups.
				// Also note that the order is preserved (stored) here, in case the user names
				// columns in a different order with the same name in different variable groups.
				VariableGroup__PrimaryKey_SecondaryKey_Names__Vector variable_group__key_names__vectors;

				// variable_groups_vector:
				// Vector:
				// Variable group identifier,
				// The selected variables in that group.
				// (independent of time granularity; different time granularities can appear here)
				// (Also, multiple, identical UOA's are acceptable, possibly differing in time granularity)
				// I.e., possibly multiple PRIMARY variable groups, all corresponding to the same primary UOA (regardless of time granularity and/or UOA multiplicity)
				Table_VARIABLES_SELECTED::VariableGroup_To_VariableSelections_Vector top_level_variable_groups_vector; // both primary and non-primary top level variable groups
				Table_VARIABLES_SELECTED::VariableGroup_To_VariableSelections_Vector child_variable_groups_vector; // all child variable groups

				// Information about *all* primary key columns, in the sequence they appear in the output
				// ... including "duplicate" primary keys resulting from multiplicity greater than 1.
				// In other words, a class (wrapping a vector) that tracks all primary keys that
				// will appear in the K-adic *output*, in the default order it will appear
				// (not counting any column order modifications set by the user).
				PrimaryKeySequence sequence;

				// Column metadata for the raw *data* tables (not Generator-created temporary tables)
				// corresponding to every variable group for which there is at least 1 variable
				// selected for output by the user.
				// This breaks down into top-level variable groups ("primary"), which
				// correspond to the "biggest" unit of analysis,
				// and "child" or "secondary" variable groups, which simply add additional
				// columns of output variables, but do not add to the multiplicity of the
				// DMU categories, which are obtained from the primary variable groups.
				std::vector<ColumnsInTempView> primary_variable_groups_column_info;
				std::vector<ColumnsInTempView> secondary_variable_groups_column_info;

				// Basic variables used throughout different functions of this Generator
				OutputModel * model;
				InputModel * input_model;
				OutputProject & project;
				Messager & messager;
				sqlite3 * db;
				sqlite3_stmt * stmt_result; // An overall statement handle that is used only to iterate through the final result of various temporary tables
				std::string sql_error;
				Executor executor;

				// Helper variable saves the number of columns in the current primary variable group's inner table
				int inner_table_no_multiplicities__with_all_datetime_columns_included__column_count;

				std::int64_t timerange_start;
				std::int64_t timerange_end;
				bool at_least_one_variable_group_has_timerange;

				bool failed;

				bool debug_ordering;
				bool delete_tables;

				bool initialized;
				bool overwrite_if_output_file_already_exists;

			public:

				static std::recursive_mutex is_generating_output_mutex;
				static std::atomic<bool> is_generating_output;
				static bool cancelled;
				bool done;

			private:

				// If we ever switch to using the SQLite "temp" mechanism, utilize temp_dot
				std::string temp_dot;

				std::string failure_message;

			public:

				boost::filesystem::path debug_sql_path;
				std::fstream debug_sql_file;
				std::set<std::string> tables_deleted;

				static int number_transaction_begins;
				static int number_transaction_ends;

		};

};

template<>
class ModelFactory<OutputModel>
{

	public:

		OutputModel * operator()(Messager & messager, boost::filesystem::path const path_to_model_database, std::shared_ptr<InputModelSettings> const input_model_settings_,
								 std::shared_ptr<InputModel> const input_model_)
		{
			// Will be wrapped up in a unique_ptr by caller... this is a factory function
			OutputModel * new_model = new OutputModel(messager, path_to_model_database, input_model_settings_, input_model_);
			return new_model;
		}

};

bool OutputModelImportTableFn(Importer * importer, Model_basemost * model_, ImportDefinition & import_definition, Table_basemost * table_, DataBlock const & table_block,
							  int const number_rows, long & linenum, long & badwritelines, std::vector<std::string> & errors);

template <typename MEMORY_TAG>
bool OutputModel::OutputGenerator::CreateOutputRow(Branch const & branch, BranchOutputRow<MEMORY_TAG> const & outputRow, KadSampler & allWeightings)
{
	bool first = true;

	// First, the branch primary keys
	std::for_each(branch.primary_keys.cbegin(), branch.primary_keys.cend(), [&](DMUInstanceData const & data)
	{
		boost::apply_visitor(create_output_row_visitor(first), data);
	});

	// Then, the leaf primary keys - for multiple leaves
	// (this is the K-ad)
	int numberLeavesHandled = 0;
	std::for_each(outputRow.primary_leaves_cache.cbegin(), outputRow.primary_leaves_cache.cend(), [&](int const & leafIndex)
	{
		Leaf const & leaf = branch.getLeafAtIndex(leafIndex);

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
			boost::apply_visitor(create_output_row_visitor(first), data);
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
			for (int nk = 0; nk < numberColumnsInTheDMUWithMultiplicityGreaterThan1; ++nk)
			{
				boost::apply_visitor(create_output_row_visitor(first), InstanceData(fast_string()));
			}
		}
	}

	// Then, the primary top-level variable group secondary data.
	// This info is stored in each leaf.
	numberLeavesHandled = 0;
	std::for_each(outputRow.primary_leaves_cache.cbegin(), outputRow.primary_leaves_cache.cend(), [&](int const & leafIndex)
	{
		Leaf const & leaf = branch.getLeafAtIndex(leafIndex);

		// Even the K=1 case is handled in the "index_into_raw_data > 0" block,
		// because although the leaf has no primary keys,
		// **it does have an index to data** (See KadSampler_ReadData_AddToTimeSlices(),
		//   where the leaf's data index is set regardless of whether there are DMU columns for the leaf.)
		// Further, this secondary data is guaranteed to be present,
		// because it came from the same row of input data that the branch+leaf (or branch, for K=1) did.
		if (leaf.index_into_raw_data > 0) // index_into_raw_data is 1-based because it corresponds to SQLite's automatically-generated "rowid" column.
		{
			SecondaryInstanceDataVector<hits_tag> const & secondary_data_vector = allWeightings.dataCache[leaf.index_into_raw_data];
			std::for_each(secondary_data_vector.cbegin(), secondary_data_vector.cend(), [&](SecondaryInstanceData const & data)
			{
				boost::apply_visitor(create_output_row_visitor(first), data);
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
		int numberSecondaryColumns = top_level_number_secondary_columns[primary_vg_index__in__top_level_vg_vector];

		for (int n = numberLeavesHandled; n < K; ++n)
		{
			for (int nk = 0; nk < numberSecondaryColumns; ++nk)
			{
				boost::apply_visitor(create_output_row_visitor(first), InstanceData(fast_string()));
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
	int numberTopLevelGroups = static_cast<int>(top_level_variable_groups_vector.size());

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
			//  (see KadSampler_ReadData_AddToTimeSlices(), the VARIABLE_GROUP_MERGE_MODE__TOP_LEVEL case,
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

				Leaf const & leaf = branch.getLeafAtIndex(leafIndex);

				// Here is where, in addition to the K>1 case, the K=1 case is supported,
				// because "other_top_level_indices_into_raw_data" is populated
				// regardless of whether "primary_keys" is populated for the leaf.
				std::for_each(leaf.other_top_level_indices_into_raw_data.cbegin(),
					leaf.other_top_level_indices_into_raw_data.cend(), [&](fast_short_to_int_map<MEMORY_TAG>::value_type const & top_level_vg_and_data_index)
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
						std::int32_t const & data_index = top_level_vg_and_data_index.second;
						DataCache<hits_tag> & data_cache = allWeightings.otherTopLevelCache[vg_number];
						SecondaryInstanceDataVector<hits_tag> const & secondary_data_vector = data_cache[data_index];
						std::for_each(secondary_data_vector.cbegin(), secondary_data_vector.cend(), [&](SecondaryInstanceData const & data)
						{
							boost::apply_visitor(create_output_row_visitor(first), data);
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
					if (vgNumber != primary_vg_index__in__top_level_vg_vector)
					{
						// Missing a variable group.  Fill with blanks.
						int numberSecondaries = top_level_number_secondary_columns[vgNumber];

						for (int n = 0; n < numberSecondaries; ++n)
						{
							boost::apply_visitor(create_output_row_visitor(first), InstanceData(fast_string()));
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
				if (vgNumber != primary_vg_index__in__top_level_vg_vector)
				{
					int numberSecondaries = top_level_number_secondary_columns[vgNumber];

					for (int n = 0; n < numberSecondaries; ++n)
					{
						boost::apply_visitor(create_output_row_visitor(first), InstanceData(fast_string()));
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
	int numberChildGroups = static_cast<int>(child_variable_groups_vector.size());

	for (int vgNumber = 0; vgNumber < numberChildGroups; ++vgNumber)
	{
		int const the_child_multiplicity = child_uoas__which_multiplicity_is_greater_than_1[*(child_variable_groups_vector[vgNumber].first.identifier_parent)].second;

		for (int multiplicity = 0; multiplicity < the_child_multiplicity; ++multiplicity)
		{
			bool matched = false;
			std::for_each(outputRow.child_indices_into_raw_data.cbegin(),
				outputRow.child_indices_into_raw_data.cend(), [&](fast__short__to__fast_short_to_int_map__loaded<MEMORY_TAG>::value_type const & leaf_index_mappings)
			{
				int const vg_number = leaf_index_mappings.first;

				if (vg_number != vgNumber)
				{
					return;
				}

				auto const & leaf_number_to_data_index = leaf_index_mappings.second;
				std::for_each(leaf_number_to_data_index.cbegin(), leaf_number_to_data_index.cend(), [&](fast_short_to_int_map<MEMORY_TAG>::value_type const & leaf_index_mapping)
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

					std::int32_t const & data_index = leaf_index_mapping.second;
					DataCache<hits_tag> & data_cache = allWeightings.childCache[vg_number];
					SecondaryInstanceDataVector<hits_tag> & secondary_data_vector = data_cache[data_index];

					if (!secondary_data_vector.empty())
					{
						matched = true;
					}

					std::for_each(secondary_data_vector.cbegin(), secondary_data_vector.cend(), [&](SecondaryInstanceData const & data)
					{
						boost::apply_visitor(create_output_row_visitor(first), data);
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
					boost::apply_visitor(create_output_row_visitor(first), InstanceData(fast_string()));
				}
			}
		}
	}

	return first;
}

template <typename MEMORY_TAG_OUTPUT_ROW, typename MEMORY_TAG_SET_OF_ROWS>
void OutputModel::OutputGenerator::EmplaceIncomingRowFromTimeSliceBranchDuringConsolidation(KadSampler & allWeightings, Branch const & branch,
	BranchOutputRow<MEMORY_TAG_OUTPUT_ROW> const & incoming_row, FastSetMemoryTag<MergedTimeSliceRow<MEMORY_TAG_SET_OF_ROWS>, MEMORY_TAG_SET_OF_ROWS> & merging, TimeSlice const & the_slice, std::int64_t & orig_row_count)
{
	create_output_row_visitor::mode = create_output_row_visitor::CREATE_ROW_MODE__INSTANCE_DATA_VECTOR;
	allWeightings.create_output_row_visitor_global_data_cache.clear();
	CreateOutputRow(branch, incoming_row, allWeightings);
	create_output_row_visitor::output_file = nullptr;
	create_output_row_visitor::mode = create_output_row_visitor::CREATE_ROW_MODE__NONE;

	merging.emplace(the_slice, allWeightings.create_output_row_visitor_global_data_cache);

	allWeightings.create_output_row_visitor_global_data_cache.clear();

	++orig_row_count;
}

template <typename MEMORY_TAG>
void OutputModel::OutputGenerator::OutputGranulatedRow(TimeSlice const & current_time_slice, fast_branch_output_row_set<MEMORY_TAG> const & output_rows_for_this_full_time_slice,
	std::fstream & output_file, Branch const & branch, KadSampler & allWeightings, std::int64_t & rows_written)
{

	// current_time_slice to be used when the time-slice-start and time-slice-end rows are output

	std::for_each(output_rows_for_this_full_time_slice.cbegin(), output_rows_for_this_full_time_slice.cend(), [&](BranchOutputRow<MEMORY_TAG> const & outputRow)
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
			output_file << current_time_slice.toStringStart().c_str();
			output_file << ",";
			output_file << current_time_slice.toStringEnd().c_str();
		}

		output_file << std::endl;
		++rows_written;

	});

}

#endif
