#include "InputModel.h"
#include "../sqlite/sqlite-amalgamation-3071700/sqlite3.h"

void InputModel::LoadTables()
{

	if (db != nullptr)
	{
		t_dmu_category.Load(db, this);
		t_dmu_setmembers.Load(db, this);
		t_uoa_category.Load(db, this);
		t_uoa_setmemberlookup.Load(db, this);
		t_vgp_identifiers.Load(db, this);
		t_vgp_setmembers.Load(db, this);

		WidgetInstanceIdentifiers variable_group_identifiers = t_vgp_identifiers.getIdentifiers();
		std::for_each(variable_group_identifiers.cbegin(), variable_group_identifiers.cend(), [this](WidgetInstanceIdentifier const & variable_group_identifier)
		{
			if (variable_group_identifier.code)
			{
				std::string variable_group_name = *variable_group_identifier.code;
				std::string variable_group_data_table_name("VG_INSTANCE_DATA_");
				variable_group_data_table_name += variable_group_name;
				std::unique_ptr<Table_VariableGroupData> vg_instance_data(new Table_VariableGroupData(variable_group_name, variable_group_data_table_name));
			}
		});
	}

}

bool InputModelImportTableFn(Model_basemost * model_, Table_basemost * table_, Importer::DataBlock const & table_block, int const number_rows)
{
	try
	{
		if (table_->table_model_type == Table_basemost::TABLE_MODEL_TYPE__INPUT_MODEL)
		{
			InputModel * input_model = dynamic_cast<InputModel*>(model_);
			if (!input_model)
			{
				// Todo: log warning
				return false;
			}
			if (input_model->getDb() == nullptr)
			{
				// Todo: log warning
				return false;
			}
			table_->Import(input_model->getDb(), nullptr, input_model);
		}
		else
		{
			// Todo: log warning
			return false;
		}
	}
	catch (std::bad_cast &)
	{
		// Todo: log warning
		return false;
	}
	return true;
}
