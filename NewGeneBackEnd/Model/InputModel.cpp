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
