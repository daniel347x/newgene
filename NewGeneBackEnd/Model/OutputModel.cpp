#include "OutputModel.h"

void OutputModel::LoadTables()
{

	Model::LoadTables();

	if (db != nullptr)
	{
		t_variables_selected_identifiers.Load(db, input_model.get());
	}

}
