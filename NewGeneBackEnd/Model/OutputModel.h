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
#include "./RandomSampling/RandomSampling.h" 

class PrimaryKeySequence
{

	public:

		class VariableGroup_PrimaryKey_Info
		{

			public:

				VariableGroup_PrimaryKey_Info()
					: sequence_number_within_dmu_category_for_this_variable_groups_uoa(-1) // will only be populated later if exists
					, current_outer_multiplicity_of_this_primary_key__in_relation_to__the_uoa_corresponding_to_the_current_variable_group___same_as___current_inner_table_number_within_the_inner_table_set_corresponding_to_the_current_variable_group(-1) // will be populated later
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
					: sequence_number_within_dmu_category_spin_control{ -1 }
					, sequence_number_within_dmu_category_primary_uoa{ -1 }
					, sequence_number_in_all_primary_keys__of__global_primary_key_sequence_metadata__NOT__of_order_columns_appear_in_top_level_vg{ -1 }
					, sequence_number_in_all_primary_keys__of__order_columns_appear_in_top_level_vg{ -1 }
					, total_k_count_within_high_level_variable_group_uoa_for_this_dmu_category{ -1 }
					, total_kad_spin_count_for_this_dmu_category{ -1 }
					, total_outer_multiplicity__for_the_current_dmu_category__corresponding_to_the_uoa_corresponding_to_top_level_variable_group{ -1 }
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
					, COLUMN_TYPE__DATETIMESTART__PRIMARY_VG_INNER_TABLE_MERGE__BEFORE_DUPLICATES_REMOVED
					, COLUMN_TYPE__DATETIMEEND__PRIMARY_VG_INNER_TABLE_MERGE__BEFORE_DUPLICATES_REMOVED
					, COLUMN_TYPE__DATETIMESTART__PRIMARY_VG_INNER_TABLE_MERGE__AFTER_DUPLICATES_REMOVED
					, COLUMN_TYPE__DATETIMEEND__PRIMARY_VG_INNER_TABLE_MERGE__AFTER_DUPLICATES_REMOVED
					, COLUMN_TYPE__DATETIMESTART__TIMERANGE_MERGED_BETWEEN_TOP_LEVEL_PRIMARY_VARIABLE_GROUPS
					, COLUMN_TYPE__DATETIMEEND__TIMERANGE_MERGED_BETWEEN_TOP_LEVEL_PRIMARY_VARIABLE_GROUPS
					, COLUMN_TYPE__DATETIMESTART__POST_TIMERANGE_MERGED_BETWEEN_TOP_LEVEL_PRIMARY_VARIABLE_GROUPS
					, COLUMN_TYPE__DATETIMEEND__POST_TIMERANGE_MERGED_BETWEEN_TOP_LEVEL_PRIMARY_VARIABLE_GROUPS
					, COLUMN_TYPE__DATETIMESTART_CHILD_MERGE
					, COLUMN_TYPE__DATETIMEEND_CHILD_MERGE
					, COLUMN_TYPE__DATETIMESTART_PRE_MERGED_KAD_OUTPUT
					, COLUMN_TYPE__DATETIMEEND_PRE_MERGED_KAD_OUTPUT
					, COLUMN_TYPE__DATETIMESTART_MERGED_KAD_OUTPUT
					, COLUMN_TYPE__DATETIMEEND_MERGED_KAD_OUTPUT
					, COLUMN_TYPE__DATETIMESTART_TEXT
					, COLUMN_TYPE__DATETIMEEND_TEXT
					, COLUMN_TYPE__DATETIMESTART__TIME_SLICE
					, COLUMN_TYPE__DATETIMEEND__TIME_SLICE
				};

				ColumnInTempView()
					: is_within_inner_table_corresponding_to_top_level_uoa(false)
					, column_type(COLUMN_TYPE__UNKNOWN)
					, primary_key_index_within_total_kad_for_dmu_category(-1)
					, primary_key_index__within_uoa_corresponding_to_variable_group_corresponding_to_current_inner_table__for_dmu_category(-1)
					, primary_key_index_within_primary_uoa_for_dmu_category(-1)
					, current_multiplicity__corresponding_to__current_inner_table___is_1_in_all_inner_tables_when_multiplicity_is_1_for_that_dmu_category_for_that_vg(-1)
					, current_multiplicity__of__current_inner_table__within__current_vg_inner_table_set(-1)
					, number_inner_tables_in_set(-1)
					, total_outer_multiplicity__in_total_kad__for_current_dmu_category__for_current_variable_group(-1)
					, total_k_count__within_uoa_corresponding_to_current_variable_group__for_current_dmu_category(-1)
					, total_k_count__within_uoa_corresponding_to_top_level_variable_group__for_current_dmu_category(-1)
					, total_k_spin_count_across_multiplicities_for_dmu_category(-1)
					, primary_key_should_be_treated_as_integer_____float_not_allowed_as_primary_key(false)
					, originally_datetime(false)
				{

				}

				bool is_within_inner_table_corresponding_to_top_level_uoa;
				std::string column_name_in_temporary_table; // The name of the column in the temporary view (includes UUID)
				std::string column_name_in_temporary_table_no_uuid; // The name of the column in the temporary view (without UUID)
				std::string column_name_in_original_data_table; // The name of the column in the original raw data table corresponding to this column (if any)
				WidgetInstanceIdentifier variable_group_associated_with_current_inner_table; // The variable group associated with this column
				WidgetInstanceIdentifier
				uoa_associated_with_variable_group_associated_with_current_inner_table; // The unit of analysis associated with the variable group associated with this column
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
				int current_multiplicity__corresponding_to__current_inner_table___is_1_in_all_inner_tables_when_multiplicity_is_1_for_that_dmu_category_for_that_vg;
				int current_multiplicity__of__current_inner_table__within__current_vg_inner_table_set;
				int number_inner_tables_in_set;
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
							, number_of_columns__in_a_single_inner_table__for_the_columns_only_having_the_dmu_category_with_multiplicity_greater_than_one__but_this_info_is_present_for_all_primary_key_columns(
								1)
							, failed(false)
						{

						}

						void Clear();
						void PopulateFromCurrentRowInDatabase(ColumnsInTempView const & column_schema, sqlite3_stmt * stmt_result,
															  XR_TABLE_CATEGORY const xr_table_category, bool const obtain_rowid = false);

						SavedRowData & GetSavedRowData() { return *this; }

						std::int64_t rowid; // for use with SQLite's "rowid" - currently only used for random sampling algorithm

						std::int64_t datetime_start;
						std::int64_t datetime_end;
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

						std::vector<std::pair<SQLExecutor::WHICH_BINDING, std::pair<int, int>>> indices_of_all_columns_in_final_inner_table;
						std::vector<std::pair<SQLExecutor::WHICH_BINDING, std::pair<int, int>>> indices_of_all_columns_in_all_but_final_inner_table;
						std::vector<std::pair<SQLExecutor::WHICH_BINDING, std::pair<int, int>>> indices_of_primary_key_columns;
						std::vector<std::pair<SQLExecutor::WHICH_BINDING, std::pair<int, int>>> indices_of_primary_key_columns_with_multiplicity_greater_than_1;
						std::vector<std::pair<SQLExecutor::WHICH_BINDING, std::pair<int, int>>> indices_of_primary_key_columns_with_multiplicity_equal_to_1;
						std::vector<std::pair<SQLExecutor::WHICH_BINDING, std::pair<int, int>>> indices_of_all_primary_key_columns_in_final_inner_table;
						std::vector<std::pair<SQLExecutor::WHICH_BINDING, std::pair<int, int>>> indices_of_all_primary_key_columns_in_all_but_final_inner_table;
						std::vector<std::pair<SQLExecutor::WHICH_BINDING, std::pair<int, int>>> indices_of_secondary_key_columns;

						std::vector<bool> is_index_in_final_inner_table;
						std::vector<bool> is_index_in_all_but_final_inner_table;
						std::vector<bool> is_index_a_primary_key;
						std::vector<bool> is_index_a_primary_key_with_outer_multiplicity_greater_than_1;
						std::vector<bool> is_index_a_primary_key_with_outer_multiplicity_equal_to_1;
						std::vector<bool> is_index_a_primary_key_in_the_final_inner_table;
						std::vector<bool> is_index_a_primary_key_in_not_the_final_inner_table;
						std::vector<bool> is_index_a_secondary_key;

						int number_of_columns__in_a_single_inner_table__for_the_columns_only_having_the_dmu_category_with_multiplicity_greater_than_one__but_this_info_is_present_for_all_primary_key_columns;
						int number_of_columns_in_inner_table;

						std::vector<int> inner_table_number;
						int number_of_multiplicities;

						bool operator<(SavedRowData const & rhs) const;
						bool TestLessEqual(SavedRowData const & rhs, OutputModel::OutputGenerator & generator) const;

						std::string error_message;

						bool failed;

						void SwapBindings(std::vector<std::string> const & new_strings,
										  std::vector<std::int64_t> const & new_ints,
										  std::vector<long double> const & new_floats,
										  std::vector<SQLExecutor::WHICH_BINDING> const & new_bindings,
										  bool enforce_all_datetimes = false,
										  std::int64_t const startdate_current = 0,
										  std::int64_t const enddate_current = 0,
										  std::int64_t const startdate_previous = 0,
										  std::int64_t const enddate_previous = 0,
										  int const current_datetime_start_column_index = 0,
										  int const current_datetime_end_column_index = 0,
										  int const previous_datetime_start_column_index = 0,
										  int const previous_datetime_end_column_index = 0);
						void SwapBindings(std::vector<std::string> const & new_strings,
										  std::vector<std::int64_t> const & new_ints,
										  std::vector<long double> const & new_floats,
										  std::vector < std::pair < SQLExecutor::WHICH_BINDING,
										  std::pair<int, int >>> & new_indices,
										  bool enforce_all_datetimes = false,
										  std::int64_t const startdate_current = 0,
										  std::int64_t const enddate_current = 0,
										  std::int64_t const startdate_previous = 0,
										  std::int64_t const enddate_previous = 0,
										  int const current_datetime_start_column_index = 0,
										  int const current_datetime_end_column_index = 0,
										  int const previous_datetime_start_column_index = 0,
										  int const previous_datetime_end_column_index = 0);
						void AddBinding(std::vector<bool> const & binding_test, std::vector<std::pair<SQLExecutor::WHICH_BINDING, std::pair<int, int>>> & bindings, SQLExecutor::WHICH_BINDING binding_type,
										int const binding_index, std::pair<SQLExecutor::WHICH_BINDING, std::pair<int, int>> const & potential_current_int_binding_to_add,
										std::pair<SQLExecutor::WHICH_BINDING, std::pair<int, int>> const & potential_current_float_binding_to_add,
										std::pair<SQLExecutor::WHICH_BINDING, std::pair<int, int>> const & potential_current_string_binding_to_add);
						void SetFinalInnerTableToNull(bool const set_datetime_to_previous_block);
						void SetLast2DateTimeColumns(std::int64_t const start_datetime_to_set, std::int64_t const end_datetime_to_set);

						void ReturnAllNonNullPrimaryKeyGroups(std::set<std::vector<std::int64_t>> & inner_table_primary_key_groups) const;
						void ReturnAllNonNullPrimaryKeyGroups(std::set<std::vector<long double>> & inner_table_primary_key_groups) const;
						void ReturnAllNonNullPrimaryKeyGroups(std::set<std::vector<std::string>> & inner_table_primary_key_groups) const;

				};

				class ColumnSorter
				{

					public:

						std::vector<std::string> strings;
						std::vector<std::int64_t> ints;
						std::vector<long double> floats;
						std::vector<std::pair<SQLExecutor::WHICH_BINDING, int>> bindings;
						std::vector<std::pair<SQLExecutor::WHICH_BINDING, int>> bindings__primary_keys_with_multiplicity_greater_than_1;
						std::vector<std::pair<SQLExecutor::WHICH_BINDING, int>> bindings__all_primary_keys;

						ColumnSorter()
						{

						}

						ColumnSorter(
							std::vector<std::string> const & strings_
							, std::vector<std::int64_t> const & ints_
							, std::vector<long double> const & floats_
							, std::vector<std::pair<SQLExecutor::WHICH_BINDING, int>> bindings_
							, std::vector<std::pair<SQLExecutor::WHICH_BINDING, int>> const & bindings__primary_keys_with_multiplicity_greater_than_1_
							, std::vector<std::pair<SQLExecutor::WHICH_BINDING, int>> const & bindings__all_primary_keys_
						)
							:
							strings(strings_)
							, ints(ints_)
							, floats(floats_)
							, bindings(bindings_)
							, bindings__primary_keys_with_multiplicity_greater_than_1(bindings__primary_keys_with_multiplicity_greater_than_1_)
							, bindings__all_primary_keys(bindings__all_primary_keys_)
						{

						}

						ColumnSorter(ColumnSorter const & rhs)
							:
							strings(rhs.strings)
							, ints(rhs.ints)
							, floats(rhs.floats)
							, bindings(rhs.bindings)
							, bindings__primary_keys_with_multiplicity_greater_than_1(rhs.bindings__primary_keys_with_multiplicity_greater_than_1)
							, bindings__all_primary_keys(rhs.bindings__all_primary_keys)
						{

						}

						bool operator<(ColumnSorter const & rhs) const;

				};

				class TimeRangeSorter
				{

					public:

						TimeRangeSorter() : ShouldReturnEqual_EvenIf_TimeRangesAreDifferent(false), DoCompareFinalInnerTable(false), bad_row(false) {}
						TimeRangeSorter(TimeRangeSorter const & rhs) { the_data_row_to_be_sorted__with_guaranteed_primary_key_match_on_all_but_last_inner_table = rhs.the_data_row_to_be_sorted__with_guaranteed_primary_key_match_on_all_but_last_inner_table; ShouldReturnEqual_EvenIf_TimeRangesAreDifferent = rhs.ShouldReturnEqual_EvenIf_TimeRangesAreDifferent; DoCompareFinalInnerTable = rhs.DoCompareFinalInnerTable; bad_row = rhs.bad_row; }
						TimeRangeSorter(SavedRowData const & rhs) : ShouldReturnEqual_EvenIf_TimeRangesAreDifferent(false), DoCompareFinalInnerTable(false), bad_row(false) { the_data_row_to_be_sorted__with_guaranteed_primary_key_match_on_all_but_last_inner_table = rhs; }
						SavedRowData the_data_row_to_be_sorted__with_guaranteed_primary_key_match_on_all_but_last_inner_table;
						bool operator<(TimeRangeSorter const & rhs) const;
						SavedRowData const & GetSavedRowData() const { return the_data_row_to_be_sorted__with_guaranteed_primary_key_match_on_all_but_last_inner_table; }
						SavedRowData & GetSavedRowData() { return the_data_row_to_be_sorted__with_guaranteed_primary_key_match_on_all_but_last_inner_table; }

						bool ShouldReturnEqual_EvenIf_TimeRangesAreDifferent;
						bool DoCompareFinalInnerTable;

						bool bad_row;

				};

				class TimeRanges
				{

					public:

						std::list<std::pair<std::int64_t, std::int64_t>> ranges;

						void append(std::int64_t const datetime_start, std::int64_t const datetime_end);
						void subtract(TimeRanges const & rhs);
						bool empty();

				};

				class TimeRangeMapper_Ints
				{

					public:

						TimeRangeMapper_Ints(TimeRangeMapper_Ints const & rhs);
						TimeRangeMapper_Ints(std::set<std::vector<std::int64_t>> const & rhs);

						bool operator<(TimeRangeMapper_Ints const & rhs) const;
						bool operator==(TimeRangeMapper_Ints const & rhs) const;
						bool operator==(std::set<std::vector<std::int64_t>> const & rhs) const;

						std::set<std::vector<std::int64_t>> sets;
						static std::set<std::vector<std::int64_t>> test_set;

				};

				class TimeRangeMapper_Floats
				{

					public:

						TimeRangeMapper_Floats(TimeRangeMapper_Floats const & rhs);
						TimeRangeMapper_Floats(std::set<std::vector<long double>> const & rhs);

						bool operator<(TimeRangeMapper_Floats const & rhs) const;
						bool operator==(TimeRangeMapper_Floats const & rhs) const;
						bool operator==(std::set<std::vector<long double>> const & rhs) const;

						std::set<std::vector<long double>> sets;
						static std::set<std::vector<long double>> test_set;

				};

				class TimeRangeMapper_Strings
				{

					public:

						TimeRangeMapper_Strings(TimeRangeMapper_Strings const & rhs);
						TimeRangeMapper_Strings(std::set<std::vector<std::string>> const & rhs);

						bool operator<(TimeRangeMapper_Strings const & rhs) const;
						bool operator==(TimeRangeMapper_Strings const & rhs) const;
						bool operator==(std::set<std::vector<std::string>> const & rhs) const;

						std::set<std::vector<std::string>> sets;
						static std::set<std::vector<std::string>> test_set;

				};

				typedef std::map<TimeRangeMapper_Ints, TimeRanges> TimeRangesForIndividualGroup_IntKeys;
				typedef std::map<TimeRangeMapper_Floats, TimeRanges> TimeRangesForIndividualGroup_FloatKeys;
				typedef std::map<TimeRangeMapper_Strings, TimeRanges> TimeRangesForIndividualGroup_StringKeys;

				typedef std::pair<std::vector<SQLExecutor>, ColumnsInTempView> SqlAndColumnSet;
				typedef std::vector<SqlAndColumnSet> SqlAndColumnSets;

				void SetFailureMessage(std::string const & failure_message_);

				// Initialize generator
				void Prepare(AllWeightings & allWeightings);
				void PopulateDMUCounts();
				void PopulateUOAs();
				void ValidateUOAs();
				void DetermineChildMultiplicitiesGreaterThanOne();
				void PopulateVariableGroups();  
				void PopulatePrimaryKeySequenceInfo();

				size_t top_level_vg_index; // in case there are multiple top-level variable groups, which one to use as primary (the others will be treated as children)
				int K; // the multiplicity

				// Random sampling
				bool consolidate_rows;
				bool random_sampling;
				std::int64_t random_sampling_number_rows;
				// For each child variable group, a vector of mapping from the child key columns to the top-level key columns
				std::map<int, std::vector<ChildToPrimaryMapping>> mappings_from_child_branch_to_primary;
				std::map<int, std::vector<ChildToPrimaryMapping>> mappings_from_child_leaf_to_primary;
				std::map<int, int> childInternalToOneLeafColumnCountForDMUWithMultiplicityGreaterThan1;
				int overall_total_number_of_primary_key_columns_including_all_branch_columns_and_all_leaves_and_all_columns_internal_to_each_leaf;
				OutputModel::OutputGenerator::SqlAndColumnSet CreateTableOfSelectedVariablesFromRawData(ColumnsInTempView const & variable_group_raw_data_columns, int const group_number);
				void RandomSamplerFillDataForChildGroups(AllWeightings & allWeightings);
				void RandomSampling_ReadData_AddToTimeSlices(ColumnsInTempView const & primary_variable_group_x1_columns, int const primary_group_number, AllWeightings & allWeightings, VARIABLE_GROUP_MERGE_MODE const merge_mode, std::vector<std::string> & errorMessages);
				SqlAndColumnSet RandomSamplingBuildOutputSchema(std::vector<ColumnsInTempView> const & primary_variable_groups_raw_data_columns, std::vector<ColumnsInTempView> const & secondary_variable_groups_column_info);
				void RandomSamplingCreateOutputTable();
				void RandomSamplingWriteToOutputTable(AllWeightings & allWeightings, std::vector<std::string> & errorMessages);
				void PrepareInsertStatement(sqlite3_stmt *& insert_random_sample_stmt, ColumnsInTempView const & random_sampling_columns);
				void BindTermToInsertStatement(sqlite3_stmt * insert_random_sample_stmt, InstanceData const & data, int bindIndex);
				void CreateOutputRow(Branch const &branch, BranchOutputRow const &outputRow, AllWeightings &allWeightings);
				void ConsolidateData(bool const random_sampling, AllWeightings &allWeightings);
				void EmplaceIncomingRowFromTimeSliceBranchDuringConsolidation(AllWeightings &allWeightings, Branch const & branch, BranchOutputRow const & incoming_row, std::set<MergedTimeSliceRow> &incoming, TimeSlice const & the_slice, int & orig_row_count);

				void RandomSamplingWriteResultsToFileOrScreen(AllWeightings & allWeightings);
				void OutputGranulatedRow(TimeSlice const & current_time_slice, fast_branch_output_row_set &output_rows_for_this_full_time_slice, std::fstream & output_file, Branch const & branch, AllWeightings & allWeightings, std::int64_t &rows_written);
				void DetermineInternalChildLeafCountMultiplicityGreaterThanOne(AllWeightings & allWeightings, ColumnsInTempView const & column_schema, int const child_variable_group_index);

				std::map<int, int> top_level_number_secondary_columns; 
				std::map<int, int> child_number_secondary_columns; 
				// Functions involved in different phases of generation
				void PopulateSchemaForRawDataTables(AllWeightings & allWeightings);
				void PopulateSchemaForRawDataTable(std::pair<WidgetInstanceIdentifier, WidgetInstanceIdentifiers> const & the_primary_variable_group, int view_count,
					std::vector<ColumnsInTempView> & variable_groups_column_info, bool const & is_primary, int const primary_or_secondary_view_index);
				void LoopThroughPrimaryVariableGroups();
				void MergeHighLevelGroupResults();
				void MergeChildGroups();
				void FormatResultsForOutput();
				void WriteResultsToFileOrScreen();
				SqlAndColumnSet ConstructFullOutputForSinglePrimaryGroup(ColumnsInTempView const & primary_variable_group_raw_data_columns, SqlAndColumnSets & sql_and_column_sets,
						int const primary_group_number = 1);
				SqlAndColumnSet CreateInitialPrimaryXTable_OrCount(ColumnsInTempView const & primary_variable_group_raw_data_columns, int const primary_group_number, bool const count_only);
				SqlAndColumnSet CreateInitialPrimaryXRTable(ColumnsInTempView const & primary_variable_group_x1_columns, int const primary_group_number);
				SqlAndColumnSet CreateInitialPrimaryMergeXRTable(ColumnsInTempView const & primary_variable_group_x1_columns);
				SqlAndColumnSet CreatePrimaryXTable(ColumnsInTempView const & primary_variable_group_raw_data_columns, ColumnsInTempView const & previous_xr_columns,
													int const current_multiplicity, int const primary_group_number);
				SqlAndColumnSet CreateChildXTable(ColumnsInTempView const & child_variable_group_raw_data_columns, ColumnsInTempView const & previous_xr_columns, int const current_multiplicity,
												  int const primary_group_number, int const child_set_number, int const current_child_view_name_index);
				SqlAndColumnSet CreateXRTable(ColumnsInTempView const & previous_x_or_final_columns_being_cleaned_over_timerange, int const current_multiplicity, int const primary_group_number,
											  XR_TABLE_CATEGORY const xr_table_category, int const current_set_number, int const current_view_name_index);
				SqlAndColumnSet CreateSortedTable(ColumnsInTempView const & final_xr_columns, int const primary_group_number, int const current_multiplicity,
												  XR_TABLE_CATEGORY const xr_table_category);
				SqlAndColumnSet RemoveDuplicates_Or_OrderWithinRows(ColumnsInTempView const & previous_result_columns, int const primary_group_number, std::int64_t & current_rows_added,
						int const current_multiplicity, XR_TABLE_CATEGORY const xr_table_category, bool const consider_merging_timerange_adjacent_identical_rows = false);
				void RemoveDuplicatesFromPrimaryKeyMatches(std::int64_t const & current_rows_stepped, SqlAndColumnSet & result, std::deque<SavedRowData> & rows_to_sort,
						std::string datetime_start_col_name, std::string datetime_end_col_name, std::shared_ptr<bool> & statement_is_prepared, sqlite3_stmt *& the_prepared_stmt,
						std::vector<SQLExecutor> & sql_strings, ColumnsInTempView & result_columns, ColumnsInTempView const & sorted_result_columns, std::int64_t & current_rows_added,
						std::int64_t & current_rows_added_since_execution, std::string & sql_add_xr_row, bool & first_row_added, std::vector<std::string> & bound_parameter_strings,
						std::vector<std::int64_t> & bound_parameter_ints, std::vector<long double> & bound_parameter_floats, std::vector<SQLExecutor::WHICH_BINDING> & bound_parameter_which_binding_to_use,
						std::int64_t const minimum_desired_rows_per_transaction, XR_TABLE_CATEGORY const xr_table_category, bool const consider_merging_timerange_adjacent_identical_rows = false,
						std::string const datetimestart_colname_text = std::string(), std::string const datetimeend_colname_text = std::string());

				enum PRIMARY_KEY_MATCH_CONDITION
				{
					MATCH_ON_ALL_KEYS
					, MATCH_ON_ALL_BUT_FINAL_INNER_TABLE
					, MATCH_ON_ALL_BUT_FINAL_TWO_INNER_TABLES
					, MATCH_ON_ALL_MULTIPLICITY_1_KEYS
				};

				// Helper functions used by the functions above
				void BeginNewTransaction();
				void EndTransaction();
				void ExecuteSQL(SqlAndColumnSet & sql_and_column_set);
				void ObtainData(ColumnsInTempView const & column_set, bool const obtain_rowid = false);
				void CloseObtainData();
				std::int64_t ObtainCount(ColumnsInTempView const & column_set);
				bool StepData();
				bool CreateNewXRRow(SavedRowData const & current_row_of_data, bool & first_row_added, std::string const & datetime_start_col_name, std::string const & datetime_end_col_name,
									std::string const & xr_view_name, std::string & sql_add_xr_row, std::vector<std::string> & bound_parameter_strings, std::vector<std::int64_t> & bound_parameter_ints,
									std::vector<long double> & bound_parameter_floats, std::vector<SQLExecutor::WHICH_BINDING> & bound_parameter_which_binding_to_use, std::int64_t const datetime_start,
									std::int64_t const datetime_end, ColumnsInTempView const & previous_x_columns, ColumnsInTempView & current_xr_columns, bool const include_previous_data,
									bool const include_current_data, XR_TABLE_CATEGORY const xr_table_category, bool const sort_only, bool const no_new_column_names);
				bool TestPrimaryKeyMatch(SavedRowData const & current_row_of_data, SavedRowData const & previous_row_of_data, bool & use_newest_row_index,
										 PRIMARY_KEY_MATCH_CONDITION const match_condition);
				bool ProcessCurrentDataRowOverlapWithPreviousSavedRow(SavedRowData & first_incoming_row, SavedRowData & current_row_of_data, std::deque<SavedRowData> & intermediate_rows_of_data,
						XR_TABLE_CATEGORY const xr_table_category);
				void MergeRows(SavedRowData & merged_data_row, SavedRowData const & current_row_of_data, SavedRowData const & first_incoming_row, XR_TABLE_CATEGORY const xr_table_category);
				void WriteRowsToFinalTable(std::deque<SavedRowData> & outgoing_rows_of_data, std::string const & datetime_start_col_name, std::string const & datetime_end_col_name,
										   std::shared_ptr<bool> & statement_is_prepared, sqlite3_stmt *& the_prepared_stmt, std::vector<SQLExecutor> & sql_strings, sqlite3 * db, std::string & result_columns_view_name,
										   ColumnsInTempView const & preliminary_sorted_top_level_variable_group_result_columns, std::int64_t & current_rows_added, std::int64_t & current_rows_added_since_execution,
										   std::string & sql_add_xr_row, bool & first_row_added, std::vector<std::string> & bound_parameter_strings, std::vector<std::int64_t> & bound_parameter_ints,
										   std::vector<long double> & bound_parameter_floats, std::vector<SQLExecutor::WHICH_BINDING> & bound_parameter_which_binding_to_use, XR_TABLE_CATEGORY const xr_table_category,
										   bool const no_new_column_names, std::string const datetimestart_text_colname = std::string(), std::string const datetimeend_text_colname = std::string());
				SqlAndColumnSet MergeIndividualTopLevelGroupIntoPrevious(ColumnsInTempView const & primary_variable_group_final_result,
						OutputModel::OutputGenerator::SqlAndColumnSet & previous_merged_primary_variable_groups_table, int const count);
				void ClearTables(SqlAndColumnSets const & tables_to_clear);
				void ClearTable(SqlAndColumnSet const & table_to_clear);
				std::string CheckOutputFileExists();
				SqlAndColumnSet SortAndOrRemoveDuplicates(ColumnsInTempView const & column_set, WidgetInstanceIdentifier const & variable_group, std::string & msg_sort_preface,
						std::string & msg_remove_duplicates_preface, int const current_multiplicity, int const primary_group_number, SqlAndColumnSets & sql_and_column_sets, bool const do_clear_table,
						XR_TABLE_CATEGORY const xr_table_category, bool const consider_merging_timerange_adjacent_identical_rows = false);
				SqlAndColumnSet CreateKadResultSet(ColumnsInTempView const & column_set);
				void SortOrderByMultiplicityOnes(ColumnsInTempView const & result_columns, XR_TABLE_CATEGORY const xr_table_category, WidgetInstanceIdentifier const & first_variable_group,
												 std::string & sql_create_final_primary_group_table, bool & first);
				void SortOrderByMultiplicityGreaterThanOnes(ColumnsInTempView const & result_columns, XR_TABLE_CATEGORY const xr_table_category,
						WidgetInstanceIdentifier const & first_variable_group, std::string & sql_create_final_primary_group_table, bool & first);
				void PopulateSplitRowInfo_FromCurrentMergingColumns(std::vector<std::tuple<bool, bool, std::pair<std::int64_t, std::int64_t>>> & rows_to_insert_info,
						int const previous_datetime_start_column_index, int const current_datetime_start_column_index, int const previous_datetime_end_column_index,
						int const current_datetime_end_column_index, SavedRowData const & current_row_of_data, XR_TABLE_CATEGORY const xr_table_category);
				void Process_RowsToCheckForDuplicates_ThatMatchOnAllButFinalInnerTable_ExceptForNullCount_InXRalgorithm(std::vector<SavedRowData> & saved_rows_with_null_in_final_inner_table,
						TimeRangesForIndividualGroup_IntKeys & group_time_ranges__intkeys, TimeRangesForIndividualGroup_FloatKeys & group_time_ranges__floatkeys,
						TimeRangesForIndividualGroup_StringKeys & group_time_ranges__stringkeys,
						ColumnsInTempView const & previous_full_table__each_row_containing_two_sets_of_data_being_cleaned_against_one_another, std::vector<SavedRowData> & outgoing_rows_of_data,
						std::vector<TimeRangeSorter> const & rows_to_check_for_duplicates_in_newly_joined_primary_key_columns, int const previous_datetime_start_column_index,
						int const current_datetime_start_column_index, int const previous_datetime_end_column_index, int const current_datetime_end_column_index,
						XR_TABLE_CATEGORY const xr_table_category);
				void HandleCompletionOfProcessingOfNormalizedGroupOfMatchingRowsInXRalgorithm(std::vector<SavedRowData> & saved_rows_with_null_in_final_inner_table,
						TimeRangesForIndividualGroup_IntKeys & group_time_ranges__intkeys, TimeRangesForIndividualGroup_FloatKeys & group_time_ranges__floatkeys,
						TimeRangesForIndividualGroup_StringKeys & group_time_ranges__stringkeys, std::vector<TimeRangeSorter> const & rows_to_check_for_duplicates_in_newly_joined_primary_key_columns,
						int const previous_datetime_start_column_index, int const current_datetime_start_column_index, int const previous_datetime_end_column_index,
						int const current_datetime_end_column_index, XR_TABLE_CATEGORY const xr_table_category, std::vector<SQLExecutor> & sql_strings, sqlite3_stmt *& the_prepared_stmt,
						std::shared_ptr<bool> & statement_is_prepared, std::int64_t & current_rows_added, std::int64_t & current_rows_added_since_execution, bool & first_row_added,
						std::string const & datetime_start_col_name, std::string const & datetime_end_col_name, ColumnsInTempView & result_columns, std::string & sql_add_xr_row,
						std::vector<std::string> & bound_parameter_strings, std::vector<std::int64_t> & bound_parameter_ints, std::vector<long double> & bound_parameter_floats,
						std::vector<SQLExecutor::WHICH_BINDING> & bound_parameter_which_binding_to_use, std::int64_t & datetime_range_start, std::int64_t & datetime_range_end,
						ColumnsInTempView const & previous_full_table__each_row_containing_two_sets_of_data_being_cleaned_against_one_another);
				void AddRowsToXRTable(std::vector<SavedRowData> & outgoing_rows_of_data, std::vector<SQLExecutor> & sql_strings, sqlite3_stmt *& the_prepared_stmt,
									  std::shared_ptr<bool> & statement_is_prepared, std::int64_t & current_rows_added, std::int64_t & current_rows_added_since_execution, bool & first_row_added,
									  std::string const & datetime_start_col_name, std::string const & datetime_end_col_name, ColumnsInTempView & result_columns, std::string & sql_add_xr_row,
									  std::vector<std::string> & bound_parameter_strings, std::vector<std::int64_t> & bound_parameter_ints, std::vector<long double> & bound_parameter_floats,
									  std::vector<SQLExecutor::WHICH_BINDING> & bound_parameter_which_binding_to_use, std::int64_t & datetime_range_start, std::int64_t & datetime_range_end,
									  ColumnsInTempView const & previous_full_table__each_row_containing_two_sets_of_data_being_cleaned_against_one_another);
				void EliminateRedundantNullsInFinalInnerTable(std::vector<SavedRowData> & saved_rows_with_null_in_final_inner_table,
						TimeRangesForIndividualGroup_IntKeys const & group_time_ranges__intkeys, TimeRangesForIndividualGroup_FloatKeys const & group_time_ranges__floatkeys,
						TimeRangesForIndividualGroup_StringKeys const & group_time_ranges__stringkeys);
				void FindDatetimeIndices(ColumnsInTempView const & columns, int & previous_datetime_start_column_index, int & previous_datetime_end_column_index,
										 int & current_datetime_start_column_index, int & current_datetime_end_column_index, XR_TABLE_CATEGORY const xr_table_category);
				bool CheckForIdenticalData(ColumnsInTempView const & columns, SavedRowData const & previous_row, SavedRowData const & current_row);
				SqlAndColumnSet Randomize(ColumnsInTempView const & columns, WidgetInstanceIdentifier const & variable_group, int const current_multiplicity, int const primary_group_number,
										  SqlAndColumnSets & sql_and_column_sets);

				SavedRowData saved_temp_merged_row;
				std::deque<std::vector<std::string>> saved_strings_deque;
				std::deque<std::vector<std::int64_t>> saved_ints_deque;
				std::deque<std::vector<long double>> saved_floats_deque;

				template <typename ROW_DEQUE>
				void HandleSetOfRowsThatMatchOnPrimaryKeys(ColumnsInTempView const & columns, ROW_DEQUE & rows_to_sort, std::deque<SavedRowData> & outgoing_rows_of_data,
						XR_TABLE_CATEGORY const xr_table_category, bool const consider_merging_timerange_adjacent_identical_rows = false)
				{

					SavedRowData current_row_of_data;
					std::deque<SavedRowData> incoming_rows_of_data;
					std::deque<SavedRowData> intermediate_rows_of_data;

					while (!rows_to_sort.empty())
					{

						current_row_of_data = std::move(rows_to_sort.front().GetSavedRowData());
						rows_to_sort.pop_front();

						// If we're starting fresh, just add the current row of input to incoming_rows_of_data
						// and proceed to the next row of input.
						if (incoming_rows_of_data.empty())
						{
							incoming_rows_of_data.push_back(std::move(current_row_of_data));
							continue;
						}

						// Quick check here before moving on to main part of loop:
						// If the current row of input starts past
						// the end of any of the saved rows, then
						// there can be no overlap with these rows, and they are done.
						// Move them to outgoing_rows_of_data.
						while (!incoming_rows_of_data.empty())
						{
							SavedRowData & first_incoming_row = incoming_rows_of_data.front();

							if (first_incoming_row.datetime_end <= current_row_of_data.datetime_start)
							{
								outgoing_rows_of_data.push_back(std::move(first_incoming_row));
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
							current_row_complete = ProcessCurrentDataRowOverlapWithPreviousSavedRow(first_incoming_row, current_row_of_data, intermediate_rows_of_data, xr_table_category);

							if (failed)
							{
								return;
							}

							incoming_rows_of_data.pop_front();
						}

						if (!current_row_complete)
						{
							intermediate_rows_of_data.push_back(std::move(current_row_of_data));
						}

						incoming_rows_of_data.insert(incoming_rows_of_data.cbegin(), std::make_move_iterator(intermediate_rows_of_data.begin()), std::make_move_iterator(intermediate_rows_of_data.end()));
						intermediate_rows_of_data.clear();

					}

					outgoing_rows_of_data.insert(outgoing_rows_of_data.cend(), std::make_move_iterator(incoming_rows_of_data.begin()), std::make_move_iterator(incoming_rows_of_data.end()));
					incoming_rows_of_data.clear();

					if (consider_merging_timerange_adjacent_identical_rows && consolidate_rows)
					{
						// Do a pass to merge adjacent rows timerange-wise that have identical secondary key data
						while (outgoing_rows_of_data.size() > 1)
						{
							SavedRowData const previous_row = std::move(outgoing_rows_of_data.front());
							SavedRowData current_row = std::move(*(++outgoing_rows_of_data.cbegin()));
							outgoing_rows_of_data.pop_front();
							outgoing_rows_of_data.pop_front();

							if (previous_row.datetime_end == current_row.datetime_start)
							{
								bool is_data_identical = CheckForIdenticalData(columns, previous_row, current_row);

								if (is_data_identical)
								{
									// Merge the rows into one
									current_row.datetime_start = previous_row.datetime_start;
									outgoing_rows_of_data.push_front(std::move(current_row));
								}
								else
								{
									// Leave the rows as-is
									intermediate_rows_of_data.push_back(std::move(previous_row));
									outgoing_rows_of_data.push_front(std::move(current_row));
								}
							}
							else
							{
								// Leave the rows as-is
								intermediate_rows_of_data.push_back(std::move(previous_row));
								outgoing_rows_of_data.push_front(std::move(current_row));
							}
						}

						if (outgoing_rows_of_data.size() == 1)
						{
							intermediate_rows_of_data.push_back(std::move(outgoing_rows_of_data.front()));
							outgoing_rows_of_data.pop_front();
						}

						outgoing_rows_of_data.swap(intermediate_rows_of_data);
					}

				}

				inline static bool CheckCancelled()
				{
					// No lock - not necessary for a boolean checked multiple times by back end and that will not cause an error if it is messed up in extraordinarily rare circumstances
					return cancelled; // opportunity for further checking here
				}

				// Progress bar variables and functions
				void DetermineNumberStages();
				void UpdateProgressBarToNextStage(std::string const helper_text_first_choice, std::string helper_text_second_choice);
				void UpdateProgressBarValue(Messager & messager, std::int64_t const current_rows_stepped);
				std::map<WidgetInstanceIdentifier, std::int64_t> total_number_incoming_rows;
				std::map<WidgetInstanceIdentifier, int> multiplicities;
				std::map<WidgetInstanceIdentifier, std::int64_t> total_number_primary_merged_rows;
				std::int64_t rough_progress_range;
				std::int64_t rough_progress_increment_one_percent;
				std::int64_t total_number_primary_rows;
				int current_progress_stage;
				int total_progress_stages;
				int progress_increment_per_stage;
				std::string setting_path_to_kad_output;
				int current_progress_value;
				std::int64_t ms_elapsed;
				std::int64_t current_number_rows_to_sort;

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
				SqlAndColumnSets merging_of_children_column_sets;
				SqlAndColumnSet random_sampling_schema;
				SqlAndColumnSet primary_group_merged_results;
				SqlAndColumnSet all_merged_results_unformatted;
				SqlAndColumnSet child_merge_final_result;
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
				std::vector<int> outer_multiplicities_primary_uoa___ie___if_there_are_3_cols_for_a_single_dmu_in_the_primary_uoa__and_K_is_12__then__this_value_is_4_for_that_DMU____note_this_is_greater_than_1_for_only_1_DMU_in_the_primary_UOA;
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


				// To be options / settings integrated later
				bool remove_self_kads;
				bool random_sampling_old;

				// optimization
				std::vector<std::string> test_strings;
				std::vector<std::int64_t> test_ints;
				std::vector<long double> test_floats;
				std::vector<std::string> rhs_test_strings;
				std::vector<std::int64_t> rhs_test_ints;
				std::vector<long double> rhs_test_floats;

		};

};

template<>
class ModelFactory<OutputModel>
{

	public:

		OutputModel * operator()(Messager & messager, boost::filesystem::path const path_to_model_database, std::shared_ptr<InputModelSettings> const input_model_settings_,
								 std::shared_ptr<InputModel> const input_model_)
		{
			OutputModel * new_model = new OutputModel(messager, path_to_model_database, input_model_settings_, input_model_);
			return new_model;
		}

};

bool OutputModelImportTableFn(Importer * importer, Model_basemost * model_, ImportDefinition & import_definition, Table_basemost * table_, DataBlock const & table_block,
							  int const number_rows, long & linenum, long & badwritelines, std::vector<std::string> & errors);

#endif
