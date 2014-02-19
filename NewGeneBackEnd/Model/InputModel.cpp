#include "InputModel.h"
#include "../sqlite/sqlite-amalgamation-3071700/sqlite3.h"
#include "Tables/Import/ImportDefinitions.h"

#include "InputModelDdlSql.h"

std::string InputModel::GetCreationSQL()
{
	return InputModelDDLSQL();
}

void InputModel::LoadTables()
{

	LoadDatabase();

	if (db != nullptr)
	{

		t_dmu_category.Load(db, this);
		t_dmu_category.Sort();

		t_dmu_setmembers.Load(db, this);
		t_dmu_setmembers.Sort();

		t_uoa_category.Load(db, this);
		t_uoa_category.Sort();

		t_uoa_setmemberlookup.Load(db, this);
		t_uoa_setmemberlookup.Sort();

		t_vgp_identifiers.Load(db, this);
		t_vgp_identifiers.Sort();

		t_vgp_setmembers.Load(db, this);
		t_vgp_setmembers.Sort();

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
					if (false)
					{
						ImportDefinition new_definition = ImportDefinitions::CreateImportDefinition(*variable_group_identifier.code);
						if (new_definition.IsEmpty())
						{
							// Todo: log warning
							return; // from lambda
						}
						Importer table_importer(new_definition, this, vg_instance_data.get(), InputModelImportTableFn);
						bool success = vg_instance_data->ImportStart(db, *variable_group_identifier.code, new_definition, nullptr, this);
						if (!success)
						{
							// Todo: log warning
							return; // from lambda
						}
						success = table_importer.DoImport();
						if (!success)
						{
							// Todo: log warning
							return; // from lambda
						}
						success = vg_instance_data->ImportEnd(db, new_definition, nullptr, this);
						if (!success)
						{
							// Todo: log warning
							return; // from lambda
						}
					}
				}
				else
				{
					t_vgp_data_vector.push_back(std::move(vg_instance_data));
				}
			}
		});

		t_vgp_data_metadata__primary_keys.Load(db, this);
		t_vgp_data_metadata__primary_keys.Sort();

		t_vgp_data_metadata__datetime_columns.Load(db, this);
		// Do not sort!!!
	}

}

bool InputModelImportTableFn(Model_basemost * model_, ImportDefinition & import_definition, Table_basemost * table_, DataBlock const & table_block, int const number_rows)
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
			table_->ImportBlock(input_model->getDb(), import_definition, nullptr, input_model, table_block, number_rows);
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
