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

};

bool OutputModelImportTableFn(Model_basemost * model_, ImportDefinition & import_definition, Table_basemost * table_, DataBlock const & table_block, int const number_rows);

class PrimaryKeySequence
{

	public:
	
		class VariableGroup_PrimaryKey_Info
		{
			public:
				WidgetInstanceIdentifier vg_identifier;
				std::string column_name;
				std::string column_name_no_uuid;
				std::string table_column_name;
				int sequence_number_within_dmu_category_variable_group_uoa;
				int current_multiplicity;
				int total_multiplicity;
				std::string view_table_name;
				std::string join_table_name;
				bool is_primary_column_selected;
		};

		class PrimaryKeySequenceEntry
		{
			public:
				WidgetInstanceIdentifier dmu_category;
				int sequence_number_within_dmu_category_spin_control;
				int sequence_number_within_dmu_category_primary_uoa;
				std::vector<VariableGroup_PrimaryKey_Info> variable_group_info_for_primary_keys; // one per variable group 
		};

		//std::map<WidgetInstanceIdentifier, std::tuple<int, int, int>> dmu_count_map; // DMU category => <Total K-spin-count for this DMU, Starting index of this DMU, K-count of this DMU for the primary UOA>
		//std::map<WidgetInstanceIdentifier, std::vector<int>> dmu_variablegroup_count_map; // DMU category => vector of [K-count of this DMU for the given variable group]

		std::vector<PrimaryKeySequenceEntry> primary_key_sequence_info; // one per primary key, in sequence

};

#endif
