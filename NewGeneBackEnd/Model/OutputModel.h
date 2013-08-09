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

};

bool OutputModelImportTableFn(Model_basemost * model_, ImportDefinition & import_definition, Table_basemost * table_, DataBlock const & table_block, int const number_rows);

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
				int sequence_number_within_dmu_category_variable_group_uoa;
				int current_multiplicity;
				int total_multiplicity;
				std::string view_table_name;
				std::string join_table_name;
				std::string join_table_name_withtime;
				bool is_primary_column_selected;
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

class ColumnsInViews
{

	public:

		class ColumnsInView
		{

			public:

				// Only used by sub-views
				std::vector<std::string> primary_key_of_multiplicity_one_in_sub_view_names;

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
