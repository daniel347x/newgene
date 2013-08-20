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
		int sequence_number_within_dmu_category_variable_group_uoa;
		int current_multiplicity;
		int total_multiplicity;


		std::string view_table_name;
		std::string join_table_name;
		std::string join_table_name_withtime;
		bool is_primary_column_selected;

		// Unused in new algorithm
		std::string datetime_row_start_column_name;
		std::string datetime_row_end_column_name;
		std::string datetime_row_start_column_name_no_uuid;
		std::string datetime_row_end_column_name_no_uuid;
		std::string datetime_row_start_table_column_name;
		std::string datetime_row_end_table_column_name;
	};

	class PrimaryKeySequenceEntry
	{
	public:
		WidgetInstanceIdentifier dmu_category;
		int sequence_number_within_dmu_category_spin_control;
		int sequence_number_within_dmu_category_primary_uoa;
		int sequence_number_in_all_primary_keys;
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
		};

		ColumnInTempView()
			: column_type(COLUMN_TYPE__UNKNOWN)
			, primary_key_index_within_total_kad_for_dmu_category(-1)
			, primary_key_index_within_total_kad_for_all_dmu_categories(-1)
			, primary_key_index_within_uoa_corresponding_to_variable_group_for_dmu_category(-1)
			, primary_key_index_within_primary_uoa_for_dmu_category(-1)
		{

		}

		std::string column_name; // The name of the column in the temporary view (includes UUID)
		std::string column_name_no_uuid; // The name of the column in the temporary view (without UUID)
		std::string table_column_name; // The name of the column in the original raw data table corresponding to this column (if any)
		WidgetInstanceIdentifier variable_group_identifier; // The variable group associated with this column
		WidgetInstanceIdentifier uoa_associated_with_variable_group_identifier; // The unit of analysis associated with the variable group associated with this column
		COLUMN_TYPE column_type; // Primary key?  Secondary column (i.e., data, not a primary key)?  One of the two special DateTime columns?
		WidgetInstanceIdentifier primary_key_dmu_category_identifier;
		int primary_key_index_within_total_kad_for_dmu_category;
		int primary_key_index_within_total_kad_for_all_dmu_categories;
		int primary_key_index_within_uoa_corresponding_to_variable_group_for_dmu_category;
		int primary_key_index_within_primary_uoa_for_dmu_category;

	};

	ColumnsInTempView()
		: view_number(-1)
		, most_recent_sql_statement_executed__index(-1)
	{

	}

	std::vector<ColumnInTempView> columns_in_view;
	int view_number;
	bool has_no_datetime_columns;
	bool has_no_datetime_columns_originally;
	std::vector<std::string> original_table_names;
	std::vector<std::string> variable_group_codes;
	std::vector<std::string> variable_group_longhand_names;
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
		void GenerateOutput(DataChangeMessage & change_response);
		void GenerateOutput2(DataChangeMessage & change_response);

		Table_VARIABLES_SELECTED t_variables_selected_identifiers;
		Table_KAD_COUNT t_kad_count;

	protected:

		std::shared_ptr<InputModelSettings> input_model_settings;
		std::shared_ptr<InputModel> input_model;

	private:

		// helpers used in GenerateOutput()
		std::string CurrentTableTokenName(int const multiplicity);
		std::string StripUUIDFromVariableName(std::string const & variable_name);
		bool AddTimeRangeMergedRow(bool previous_is_null, bool current_is_null, sqlite3 * db, std::int64_t const datetime_start_new, std::int64_t const datetime_end_new, int & overall_column_number_previous, int & overall_column_number_current_regular, std::string & sql_columns, std::string & sql_values_previous_null, std::string & sql_values_previous_filled, std::string & sql_values_current_null, std::string & sql_values_current_filled, std::string & join_count_as_text, std::string const & join_table_with_time_ranges_name);
		bool AddTimeRangeMergedRowTemp(bool previous_is_null, bool current_is_null, sqlite3 * db, std::int64_t const datetime_start_new, std::int64_t const datetime_end_new, int const overall_column_number_input_previous, int const overall_column_number_input_before_datetime, int const overall_column_number_input_after_datetime, std::string const & sql_columns, std::string const & sql_values_previous, std::string const & sql_values_previous_null, std::string const & sql_values_before_datetime, std::string const & sql_values_before_datetime_null, std::string const & sql_values_after_datetime, std::string const & sql_values_after_datetime_null, std::string const & join_table_with_time_ranges_name);

	public:
	
		class OutputGenerator
		{

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

						SQLExecutor(sqlite3 * db_)
							: executed(false)
							, statement_type(DOES_NOT_RETURN_ROWS)
							, db(db_)
							, stmt(nullptr)
							, failed(false)
						{

						}

						SQLExecutor(sqlite3 * db_, std::string const & sql_)
							: sql(sql_)
							, executed(false)
							, statement_type(DOES_NOT_RETURN_ROWS)
							, db(db_)
							, stmt(nullptr)
							, failed(false)
						{

						}

						void Execute();
						bool Step();
						void Empty(bool const empty_sql); // closes prepared statement and optionally clears stored SQL
					
						std::string sql;
						std::string sql_error;

						sqlite3 * db;
						sqlite3_stmt * stmt;
						STATEMENT_TYPE statement_type;
						bool executed;
						bool failed;

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
				void ConstructFullOutputForSinglePrimaryGroup(ColumnsInTempView const & primary_variable_group_raw_data_columns, SqlAndColumnSets & sql_and_column_sets, int const primary_group_number);
				SqlAndColumnSet CreateInitialPrimaryXTable(ColumnsInTempView const & primary_variable_group_raw_data_columns, int const primary_group_number);
				SqlAndColumnSet CreateInitialPrimaryXRTable(ColumnsInTempView const & primary_variable_group_x1_columns, int const primary_group_number);
				SqlAndColumnSet CreatePrimaryXTable(ColumnsInTempView const & primary_variable_group_raw_data_columns, ColumnsInTempView const & previous_xr_columns, int const current_multiplicity, int const primary_group_number);
				SqlAndColumnSet CreatePrimaryXRTable(ColumnsInTempView & previous_x_columns, int const current_multiplicity, int const primary_group_number);

				void BeginNewTransaction();
				void EndTransaction();
				void ExecuteSQL(SqlAndColumnSet & sql_and_column_set);
				void ObtainData(ColumnsInTempView & column_set);
				bool StepData();

				// Save the SQL and column sets corresponding to each primary variable group in a global data structure
				std::vector<SqlAndColumnSets> primary_variable_group_column_sets;

				// ***************************************************************** //
				// the_map is:
				// Map: UOA identifier => [ map: VG identifier => list of variables ]
				// ***************************************************************** //
				Table_VARIABLES_SELECTED::UOA_To_Variables_Map * the_map;

				// ************************************************************************** //
				// UOAs is:
				// Vector of: UOA identifier and its associated list of [DMU Category / Count]
				// ************************************************************************** //
				// Place UOA's and their associated DMU categories into a vector for convenience
				std::vector<std::pair<WidgetInstanceIdentifier, Table_UOA_Identifier::DMU_Counts>> UOAs;
				
				// ****************************************************************************************** //
				// biggest_counts and child_counts are:
				// Vector of: (one for each (possibly identical) UOA, so that multiple, identical 
				// UOA's may appear as multiple vector elements,
				// except possibly for time granularity)
				// Pair consisting of: UOA identifier and its associated list of [DMU Category / Count]
				// ... for the UOA that has been determined to be the primary UOA.
				// Ditto child_counts, but for child UOA's:
				// The UOA k-value for all DMU categories
				// is either 0, 1, or the corresponding UOA k-value of the primary UOA
				// and that where not 0, it is less than the corresponding UOA k-value of the primary UOA
				// (i.e., has a value of 1)
				// for only 1 DMU category,
				// and this DMU category must match the DMU category with multiplicity greater than 1 (if any)
				// for the primary UOAs
				// ****************************************************************************************** //
				std::vector<std::pair<WidgetInstanceIdentifier, Table_UOA_Identifier::DMU_Counts>> biggest_counts;
				std::vector<std::pair<WidgetInstanceIdentifier, Table_UOA_Identifier::DMU_Counts>> child_counts;

				std::vector<int> multiplicities_primary_uoa;
				int highest_multiplicity_primary_uoa;
				std::string highest_multiplicity_primary_uoa_dmu_string_code;
				bool any_primary_dmu_has_multiplicity_greater_than_1;
				int which_primary_index_has_multiplicity_greater_than_1;

				// child_uoas__which_multiplicity_is_greater_than_1:
				// Map of:
				// UOA identifier => pair<DMU category identifier of the effective DMU category that, in relation to the child,
				// has multiplicity greater than 1, ... and the given multiplicity, with respect to the total spin control count>
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

				// Information about *all* primary keys, in the sequence they appear in the output
				// ... including "duplicate" primary keys resulting from multiplicity greater than 1
				PrimaryKeySequence sequence;

				std::vector<ColumnsInTempView> primary_variable_groups_column_info;
				std::vector<ColumnsInTempView> secondary_variable_groups_column_info;

				OutputModel * model;
				InputModel * input_model;
				sqlite3 * db;
				sqlite3_stmt * stmt_result;
				std::string sql_error;
				Executor executor;

				bool failed;

				std::string temp_dot;

		};

};

bool OutputModelImportTableFn(Model_basemost * model_, ImportDefinition & import_definition, Table_basemost * table_, DataBlock const & table_block, int const number_rows);

class ColumnsInViews
{

	public:

		class ColumnsInView
		{

			public:

				// Only used by sub-views
				std::vector<std::string> primary_key_names_in_sub_view;
				std::vector<std::string> secondary_key_names_in_sub_view;
				std::vector<std::string> primary_key_names_in_sub_view_no_uuid;
				std::vector<std::string> secondary_key_names_in_sub_view_no_uuid;
				std::string datetime_start_column_name;
				std::string datetime_end_column_name;

				class ColumnInView
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
						};

						ColumnInView()
							: column_type(COLUMN_TYPE__UNKNOWN)
							, primary_key_index_within_total_kad_for_dmu_category(-1)
							, primary_key_index_within_total_kad_for_all_dmu_categories(-1)
							, primary_key_index_within_uoa_corresponding_to_variable_group_for_dmu_category(-1)
							, primary_key_index_within_primary_uoa_for_dmu_category(-1)
						{

						}

						std::string column_name;
						std::string column_name_no_uuid;
						std::string table_column_name;
						WidgetInstanceIdentifier variable_group_identifier;
						WidgetInstanceIdentifier uoa_associated_with_variable_group_identifier;
						COLUMN_TYPE column_type;
						WidgetInstanceIdentifier primary_key_dmu_category_identifier;
						int primary_key_index_within_total_kad_for_dmu_category;
						int primary_key_index_within_total_kad_for_all_dmu_categories;
						int primary_key_index_within_uoa_corresponding_to_variable_group_for_dmu_category;
						int primary_key_index_within_primary_uoa_for_dmu_category;

				};

				ColumnsInView()
					: view_number(-1)
				{

				}

				std::vector<ColumnInView> columns_in_view;
				int view_number;
				std::string view_name;

				class ColumnsInTempViews
				{

					public:

						std::vector<ColumnsInView> columns_in_temp_views_vector;

				};

				ColumnsInTempViews columns_in_temp_views;

		};

		std::vector<ColumnsInView> columns_in_views;

};

#endif
