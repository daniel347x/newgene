#include "OutputModel.h"

void OutputModel::LoadTables()
{

	LoadDatabase();

	if (db != nullptr)
	{
		t_variables_selected_identifiers.Load(db, this, input_model.get());
		t_kad_count.Load(db, this, input_model.get());
	}

}

bool OutputModelImportTableFn(Model_basemost * model_, ImportDefinition & import_definition, Table_basemost * table_, DataBlock const & table_block, int const number_rows)
{
	try
	{
		if (table_->table_model_type == Table_basemost::TABLE_MODEL_TYPE__OUTPUT_MODEL)
		{
			OutputModel * output_model = dynamic_cast<OutputModel*>(model_);
			if (!output_model)
			{
				// Todo: log warning
				return false;
			}
			if (output_model->getDb() == nullptr)
			{
				// Todo: log warning
				return false;
			}
			table_->ImportBlock(output_model->getDb(), import_definition, output_model, &output_model->getInputModel(), table_block, number_rows);
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

void OutputModel::GenerateOutput(DataChangeMessage & change_response)
{

	InputModel & input_model = getInputModel();

	Table_VARIABLES_SELECTED::UOA_To_Variables_Map the_map = t_variables_selected_identifiers.GetSelectedVariablesByUOA(getDb(), this, &input_model);

	// Check for overlap in UOA's

	// Place UOA's and their associated DMU categories into a vector for convenience
	WidgetInstanceIdentifiers UOAs;
	std::for_each(the_map.cbegin(), the_map.cend(), [this, &UOAs](std::pair<WidgetInstanceIdentifier, Table_VARIABLES_SELECTED::VariableGroup_To_VariableSelections_Map> const & uoa__to__variable_groups__pair)
	{
		WidgetInstanceIdentifier const & uoa = uoa__to__variable_groups__pair.first;
		UOAs.push_back(uoa);
	});



}
