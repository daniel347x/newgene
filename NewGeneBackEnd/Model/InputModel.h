#ifndef INPUTMODEL_H
#define INPUTMODEL_H

#include "Model.h"
#include "..\Settings\InputModelSettings.h"
#include "..\Settings\Setting.h"
#include "Tables/TableManager.h"
#include <memory>

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

};

bool InputModelImportTableFn(Model_basemost * model_, Table_basemost * table_, Importer::DataBlock const & table_block, int const number_rows);

#endif
