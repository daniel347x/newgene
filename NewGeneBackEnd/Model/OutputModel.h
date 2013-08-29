#ifndef OUTPUTMODEL_H
#define OUTPUTMODEL_H

#include "Model.h"
#include "..\Settings\OutputModelSettings.h"
#include "..\Settings\InputModelSettings.h"
#include "InputModel.h"
#include "..\Settings\Setting.h"
#include "Tables/TableManager.h"
#include <memory>
#include <tuple>

class PrimaryKeySequence
{

public:

	class VariableGroup_PrimaryKey_Info
	{

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
			int current_multiplicity;
			int total_multiplicity;
			int total_multiplicity_of_dmu_category_in_primary_uoa_corresponding_to_top_level_variable_group;
			int total_number_columns_for_dmu_category__internal_to_uoa_corresponding_to_this_variable_group;
			int total_number_columns_for_dmu_category__internal_to_the_uoa_corresponding_to_primary_uoa_for_the_same_dmu_category;

			bool is_primary_column_selected;

	};

	class PrimaryKeySequenceEntry
	{

		public:

			WidgetInstanceIdentifier dmu_category;
			int sequence_number_within_dmu_category_spin_control;
			int sequence_number_within_dmu_category_primary_uoa;
			int sequence_number_in_all_primary_keys;
			int total_k_count_within_high_level_variable_group_uoa_for_this_dmu_category;
			int total_kad_spin_count_for_this_dmu_category;
			int total_multiplicity_of_uoa_corresponding_to_top_level_variable_group_for_the_current_dmu_category;
			std::vector<VariableGroup_PrimaryKey_Info> variable_group_info_for_primary_keys; // one per variable group 

	};

	std::vector<PrimaryKeySequenceEntry> primary_key_sequence_info; // one per primary key, in sequence

};

class ColumnsInTempView
{

	public:

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
					, COLUMN_TYPE__DATETIMESTART_MERGED
					, COLUMN_TYPE__DATETIMEEND_MERGED
					, COLUMN_TYPE__DATETIMESTART_MERGED_FINAL
					, COLUMN_TYPE__DATETIMEEND_MERGED_FINAL
					, COLUMN_TYPE__DATETIMESTART_MERGED_BETWEEN_FINALS
					, COLUMN_TYPE__DATETIMEEND_MERGED_BETWEEN_FINALS
					, COLUMN_TYPE__DATETIMESTART_MERGED_KAD_OUTPUT
					, COLUMN_TYPE__DATETIMEEND_MERGED_KAD_OUTPUT
				};

				ColumnInTempView()
					: column_type(COLUMN_TYPE__UNKNOWN)
					, primary_key_index_within_total_kad_for_dmu_category(-1)
					, primary_key_index__within_uoa_corresponding_to_variable_group_corresponding_to_current_inner_table__for_dmu_category(-1)
					, primary_key_index_within_primary_uoa_for_dmu_category(-1)
					, current_multiplicity__corresponding_to__current_inner_table(-1)
					, total_multiplicity__of_current_dmu_category__within_uoa_corresponding_to_the_current_inner_tables_variable_group(-1)
					, primary_key_should_be_treated_as_numeric(false)
					, total_k_count__within_uoa_corresponding_to_current_variable_group__for_current_dmu_category(-1)
					, total_k_count__within_uoa_corresponding_to_top_level_variable_group__for_current_dmu_category(-1)
					, total_k_spin_count_across_multiplicities_for_dmu_category(-1)
					, is_within_inner_table_corresponding_to_top_level_uoa(false)
					, inner_table_set_number(-1)
				{

				}

				bool is_within_inner_table_corresponding_to_top_level_uoa;
				int inner_table_set_number; // 0 for primary, 1 for first child variable group, etc.  Note that each set includes potentially multiple inner tables, depending on the multiplicity of the given primary or child variable group.
				std::string column_name_in_temporary_table; // The name of the column in the temporary view (includes UUID)
				std::string column_name_in_temporary_table_no_uuid; // The name of the column in the temporary view (without UUID)
				std::string column_name_in_original_data_table; // The name of the column in the original raw data table corresponding to this column (if any)
				WidgetInstanceIdentifier variable_group_associated_with_current_inner_table; // The variable group associated with this column
				WidgetInstanceIdentifier uoa_associated_with_variable_group_associated_with_current_inner_table; // The unit of analysis associated with the variable group associated with this column
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

				int primary_key_index__within_uoa_corresponding_to_variable_group_corresponding_to_current_inner_table__for_dmu_category;
				int primary_key_index_within_primary_uoa_for_dmu_category;
				int current_multiplicity__corresponding_to__current_inner_table;
				int total_multiplicity__of_current_dmu_category__within_uoa_corresponding_to_the_current_inner_tables_variable_group;
				int total_k_count__within_uoa_corresponding_to_current_variable_group__for_current_dmu_category;
				int total_k_count__within_uoa_corresponding_to_top_level_variable_group__for_current_dmu_category;
				int total_k_spin_count_across_multiplicities_for_dmu_category;
				bool primary_key_should_be_treated_as_numeric;

		};

		ColumnsInTempView()
			: view_number(-1)
			, most_recent_sql_statement_executed__index(-1)
			, has_no_datetime_columns(false)
			, has_no_datetime_columns_originally(false)
		{

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

		int most_recent_sql_statement_executed__index;

};

class OutputModel : public Model<OUTPUT_MODEL_SETTINGS_NAMESPACE::OUTPUT_MODEL_SETTINGS>
{

	public:

		typedef std::pair<std::vector<std::string>, std::vector<std::string>> PrimaryKey_SecondaryKey_Names;
		typedef std::vector<PrimaryKey_SecondaryKey_Names> VariableGroup__PrimaryKey_SecondaryKey_Names__Vector;

		OutputModel(Messager & messager, boost::filesystem::path const path_to_model_database, std::shared_ptr<InputModelSettings> const & input_model_settings_, std::shared_ptr<InputModel> const & input_model_)
			: Model(messager, path_to_model_database)
			, input_model_settings(input_model_settings_)
			, input_model(input_model_)
		{

		}

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

				OutputGenerator(OutputModel & model_);
				~OutputGenerator();

				void GenerateOutput(DataChangeMessage & change_response);
			
			private:

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
						};

						SQLExecutor(sqlite3 * db_);
						SQLExecutor(sqlite3 * db_, std::string const & sql_);
						SQLExecutor(sqlite3 * db_, std::string const & sql_, std::vector<std::string> const & bound_parameter_strings_, std::vector<std::int64_t> const & bound_parameter_ints_, std::vector<WHICH_BINDING> & bound_parameter_which_binding_to_use_, sqlite3_stmt * stmt_to_use = nullptr, bool const prepare_statement_if_null = false);
						SQLExecutor(SQLExecutor const & rhs);
						SQLExecutor(SQLExecutor && rhs);
						SQLExecutor & operator=(SQLExecutor const & rhs);
						SQLExecutor & operator=(SQLExecutor && rhs);
						void Copy(SQLExecutor const & rhs);
						void CopyOwned(SQLExecutor & rhs);
						~SQLExecutor();

						void Execute();
						bool Step();
						void Empty(bool const empty_sql = true); // closes prepared statement and optionally clears stored SQL
					
						std::string sql;
						std::string sql_error;

						sqlite3 * db;
						sqlite3_stmt * stmt;
						STATEMENT_TYPE statement_type;
						bool statement_is_owned;
						bool statement_is_prepared;
						bool failed;

						std::vector<std::string> bound_parameter_strings;
						std::vector<std::int64_t> bound_parameter_ints;
						std::vector<WHICH_BINDING> bound_parameter_which_binding_to_use;

				};

				class SavedRowData
				{

					public:
						
						SavedRowData()
							: datetime_start(0)
							, datetime_end(0)
							, failed(false)
						{

						}

						void Clear();
						void PopulateFromCurrentRowInDatabase(ColumnsInTempView & preliminary_sorted_top_level_variable_group_result_columns, sqlite3_stmt * stmt_result);

						std::int64_t datetime_start;
						std::int64_t datetime_end;
						std::vector<std::string> current_parameter_strings;
						std::vector<std::int64_t> current_parameter_ints;
						std::vector<SQLExecutor::WHICH_BINDING> current_parameter_which_binding_to_use;

						std::vector<std::pair<SQLExecutor::WHICH_BINDING, int>> indices_of_primary_key_columns;
						std::vector<bool> is_index_a_primary_key;

						bool failed;

				};

				enum XR_TABLE_CATEGORY
				{
					  PRIMARY_VARIABLE_GROUP
					, CHILD_VARIABLE_GROUP
					, FINAL_MERGE_OF_PRIMARY_VARIABLE_GROUP
				};

				typedef std::pair<std::vector<SQLExecutor>, ColumnsInTempView> SqlAndColumnSet;
				typedef std::vector<SqlAndColumnSet> SqlAndColumnSets;

				// Initialize generator
				void Prepare();
				void PopulateDMUCounts();
				void PopulateUOAs();
				void ValidateUOAs();
				void DetermineChildMultiplicitiesGreaterThanOne();
				void PopulateVariableGroups();
				void PopulatePrimaryKeySequenceInfo();

				// Functions involved in different phases of generation
				void ObtainColumnInfoForRawDataTables();
				void PopulateColumnsFromRawDataTable(std::pair<WidgetInstanceIdentifier, WidgetInstanceIdentifiers> const & the_primary_variable_group, int view_count, std::vector<ColumnsInTempView> & variable_groups_column_info, bool const & is_primary);
				void LoopThroughPrimaryVariableGroups();
				void MergeHighLevelGroupResults();
				void FormatResultsForOutput();
				void WriteResultsToFileOrScreen();
				SqlAndColumnSet ConstructFullOutputForSinglePrimaryGroup(ColumnsInTempView const & primary_variable_group_raw_data_columns, SqlAndColumnSets & sql_and_column_sets, int const primary_group_number);
				SqlAndColumnSet CreateInitialPrimaryXTable(ColumnsInTempView const & primary_variable_group_raw_data_columns, int const primary_group_number);
				SqlAndColumnSet CreateInitialPrimaryXRTable(ColumnsInTempView const & primary_variable_group_x1_columns, int const primary_group_number);
				SqlAndColumnSet CreateInitialPrimaryMergeXRTable(ColumnsInTempView const & primary_variable_group_x1_columns);
				SqlAndColumnSet CreatePrimaryXTable(ColumnsInTempView const & primary_variable_group_raw_data_columns, ColumnsInTempView const & previous_xr_columns, int const current_multiplicity, int const primary_group_number);
				SqlAndColumnSet CreateChildXTable(ColumnsInTempView const & child_variable_group_raw_data_columns, ColumnsInTempView const & previous_xr_columns, int const current_multiplicity, int const primary_group_number, int const child_set_number, int const current_child_view_name_index);
				SqlAndColumnSet CreateXRTable(ColumnsInTempView & previous_x_or_final_columns_being_cleaned_over_timerange, int const current_multiplicity, int const primary_group_number, XR_TABLE_CATEGORY const xr_table_category, int const current_set_number, int const current_view_name_index);
				SqlAndColumnSet CreateSortedTable(ColumnsInTempView const & final_xr_columns, int const primary_group_number);
				SqlAndColumnSet RemoveDuplicates(ColumnsInTempView & preliminary_sorted_top_level_variable_group_result_columns, int const primary_group_number);

				// Helper functions used by the functions above
				void BeginNewTransaction();
				void EndTransaction();
				void ExecuteSQL(SqlAndColumnSet & sql_and_column_set);
				void ObtainData(ColumnsInTempView & column_set);
				bool StepData();
				bool CreateNewXRRow(bool & first_row_added, std::string const & datetime_start_col_name, std::string const & datetime_end_col_name, std::string const & xr_view_name, std::string & sql_add_xr_row, std::vector<std::string> & bound_parameter_strings, std::vector<std::int64_t> & bound_parameter_ints, std::vector<SQLExecutor::WHICH_BINDING> & bound_parameter_which_binding_to_use, std::int64_t const datetime_start, std::int64_t const datetime_end, ColumnsInTempView & previous_x_columns, ColumnsInTempView & current_xr_columns, bool const include_previous_data, bool const include_current_data);
				bool TestIfCurrentRowMatchesPrimaryKeys(SavedRowData const & current_row_of_data, SavedRowData const & previous_row_of_data);
				bool ProcessCurrentDataRowOverlapWithFrontSavedRow(SavedRowData & first_incoming_row, SavedRowData & current_row_of_data, std::deque<SavedRowData> & intermediate_rows_of_data);
				SavedRowData MergeRows(SavedRowData const & current_row_of_data, SavedRowData const & first_incoming_row);
				void WriteRowsToFinalTable(std::deque<SavedRowData> & outgoing_rows_of_data, std::string const & datetime_start_col_name, std::string const & datetime_end_col_name, sqlite3_stmt *& the_prepared_stmt, std::vector<SQLExecutor> & sql_strings, sqlite3 * db, std::string & result_columns_view_name, ColumnsInTempView & preliminary_sorted_top_level_variable_group_result_columns, int & current_rows_added, int & current_rows_added_since_execution, std::string & sql_add_xr_row, bool & first_row_added, std::vector<std::string> & bound_parameter_strings, std::vector<std::int64_t> & bound_parameter_ints, std::vector<SQLExecutor::WHICH_BINDING> & bound_parameter_which_binding_to_use);
				SqlAndColumnSet MergeIndividualTopLevelGroupIntoPrevious(ColumnsInTempView const & primary_variable_group_final_result, OutputModel::OutputGenerator::SqlAndColumnSet & previous_merged_primary_variable_groups_table, int const count);

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
				SqlAndColumnSets primary_group_final_results;
				SqlAndColumnSets intermediate_merging_of_primary_groups_column_sets;
				SqlAndColumnSet all_merged_results_unformatted;
				SqlAndColumnSet final_result;

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
				// are selected by the user for any variable groups associated with this UOA).
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
				// (and must, if the primary keys are the same)
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
				std::vector<int> multiplicities_primary_uoa;
				int highest_multiplicity_primary_uoa;
				std::string highest_multiplicity_primary_uoa_dmu_string_code;
				bool any_primary_dmu_has_multiplicity_greater_than_1;
				int which_primary_index_has_multiplicity_greater_than_1;

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
				Table_VARIABLES_SELECTED::VariableGroup_To_VariableSelections_Vector primary_variable_groups_vector;
				Table_VARIABLES_SELECTED::VariableGroup_To_VariableSelections_Vector secondary_variable_groups_vector;

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
				sqlite3 * db;
				sqlite3_stmt * stmt_result; // An overall statement handle that is used only to iterate through the final result of various temporary tables
				std::string sql_error;
				Executor executor;

				// Helper variable saves the number of columns in the current primary variable group's inner table
				int inner_table_no_multiplicities_column_count;

				bool failed;

				bool debug_ordering;

				// If we ever switch to using the SQLite "temp" mechanism, utilize temp_dot
				std::string temp_dot;

		};

};

bool OutputModelImportTableFn(Model_basemost * model_, ImportDefinition & import_definition, Table_basemost * table_, DataBlock const & table_block, int const number_rows);

#endif
