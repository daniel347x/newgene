#include "InputModel.h"
#include "../sqlite/sqlite-amalgamation-3071700/sqlite3.h"
#include "Tables\Import\ImportDefinitions.h"

void InputModel::LoadTables()
{

	LoadDatabase();

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
				std::unique_ptr<Table_VariableGroupData> vg_instance_data(new Table_VariableGroupData(*variable_group_identifier.code));
#				if 0
					// Do not load!  Wait until output dataset is generated
					vg_instance_data->Load(db, this);
#				endif
				if (!tableManager().TableExists(db, vg_instance_data->table_name))
				{
					ImportDefinition new_definition = ImportDefinitions::CreateImportDefinition(*variable_group_identifier.code);
					if (new_definition.IsEmpty())
					{
						return; // from lambda
					}
					Importer table_importer(new_definition, this, vg_instance_data.get(), InputModelImportTableFn);
					bool success = table_importer.DoImport();
					if (!success)
					{
						return; // from lambda
					}
				}
				t_vgp_data_vector.push_back(vg_instance_data);
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
