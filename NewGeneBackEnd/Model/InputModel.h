#ifndef INPUTMODEL_H
#define INPUTMODEL_H

#include "Model.h"
#include "../Settings/InputModelSettings.h"
#include "../Settings/Setting.h"
#include "Tables/TableManager.h"
#include <memory>
#include "InputModel.DDL.SQL.h"

typedef std::vector<std::unique_ptr<Table_VariableGroupData>> VariableGroup_DataTables;

class InputModel : public Model<INPUT_MODEL_SETTINGS_NAMESPACE::INPUT_MODEL_SETTINGS>
{

	public:

		InputModel(Messager & messager, boost::filesystem::path const path_to_model_database)
			: Model(messager, path_to_model_database)
		{

		}

		void LoadTables();

		Table_DMU_Identifier t_dmu_category;
		Table_DMU_Instance t_dmu_setmembers;
		Table_UOA_Identifier t_uoa_category;
		Table_UOA_Member t_uoa_setmemberlookup;
		Table_VG_CATEGORY t_vgp_identifiers;
		Table_VG_SET_MEMBER t_vgp_setmembers;

		VariableGroup_DataTables t_vgp_data_vector;
		Table_VariableGroupMetadata_PrimaryKeys t_vgp_data_metadata__primary_keys;
		Table_VariableGroupMetadata_DateTimeColumns t_vgp_data_metadata__datetime_columns;

};

template<>
class ModelFactory<InputModel>
{

public:

	InputModel * operator()(Messager & messager, boost::filesystem::path const path_to_model_database)
	{
		bool exists = true;
		if (!boost::filesystem::exists(path_to_model_database))
		{
			exists = false;
		}

		InputModel * new_model = new InputModel(messager, path_to_model_database);
		if (!exists)
		{
			// Create the new database
			std::string err;
			bool success = new_model->RunSQL(InputModelDDLSQL(), err);
			if (!success)
			{
				boost::format msg("Error creating new input project database: %1%");
				msg % err.c_str();
				throw NewGeneException() << newgene_error_description(msg.str());
			}
		}

		return new_model;
	}

};

bool InputModelImportTableFn(Model_basemost * model_, ImportDefinition & import_definition, Table_basemost * table_, DataBlock const & table_block, int const number_rows);

#endif
