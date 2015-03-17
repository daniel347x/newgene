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

		t_uoa_setmemberlookup.Load(db, this);
		t_uoa_setmemberlookup.Sort();

		t_uoa_category.Load(db, this);
		t_uoa_category.Sort();

		t_vgp_identifiers.Load(db, this);
		t_vgp_identifiers.Sort(); // ORDER BY clause is also used to sort, so the ORDER BY sort is wiped out here, but the two sorts should yield the same result - someday, this could be tested to confirm

		t_vgp_setmembers.Load(db, this);
		t_vgp_setmembers.Sort(); // ORDER BY clause is also used to sort, so the ORDER BY sort is wiped out here, but the two sorts should yield the same result - someday, this could be tested to confirm

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

					// disable, now that real import is completed

					if (false)
					{

						ImportDefinition new_definition = ImportDefinitions::CreateImportDefinition(*variable_group_identifier.code);

						if (new_definition.IsEmpty())
						{
							// Todo: log warning
							return; // from lambda
						}

						std::string errorMsg;
						Importer table_importer(new_definition, this, vg_instance_data.get(), Importer::INSERT_IN_BULK, variable_group_identifier, InputModelImportTableFn,
												Importer::IMPORT_VG_INSTANCE_DATA, errorMsg);

						// disable, now that real import is completed

						//bool success = table_importer.DoImport(errorMsg, messager);
						//if (!success)
						//{
						//	boost::format msg("Unable to refresh the DMU list from the file: %1%");
						//	msg % errorMsg;
						//	throw NewGeneException() << newgene_error_description(msg.str());
						//}

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

bool InputModelImportTableFn(Importer * importer, Model_basemost * model_, ImportDefinition & import_definition, Table_basemost * table_, DataBlock const & table_block,
							 int const number_rows, long & linenum, long & badwritelines, long & goodwritelines, long & goodupdatelines, std::vector<std::string> & errors)
{

	int number_errors_at_start = errors.size();
	std::string errorMsg;

	try
	{
		if (table_->table_model_type == Table_basemost::TABLE_MODEL_TYPE__INPUT_MODEL)
		{

			InputModel * input_model = dynamic_cast<InputModel *>(model_);

			if (!input_model)
			{
				boost::format msg("Bad input model in InputModelImportTableFn");
				errorMsg = msg.str();
				errors.push_back(errorMsg);
				errorMsg.clear();
				return false;
			}

			if (input_model->getDb() == nullptr)
			{
				boost::format msg("Bad input model db in InputModelImportTableFn");
				errorMsg = msg.str();
				errors.push_back(errorMsg);
				errorMsg.clear();
				return false;
			}

			errorMsg.clear();

			switch (importer->mode)
			{

				case Importer::INSERT_IN_BULK:
					{
						long numlinesupdated = 0;
						table_->ImportBlockBulk(input_model->getDb(), import_definition, nullptr, input_model, table_block, number_rows, linenum, badwritelines, goodwritelines, goodupdatelines, errors);
						int number_errors_now = errors.size();

						if (number_errors_now > number_errors_at_start)
						{
							return false;
						}
					}
					break;

				case Importer::INSERT_OR_UPDATE:
					{
						long numlinesupdated = 0;
						table_->ImportBlockUpdate(input_model->getDb(), import_definition, nullptr, input_model, table_block, number_rows, linenum, badwritelines, goodwritelines, goodupdatelines,
												  numlinesupdated, errors);
						int number_errors_now = errors.size();

						if (number_errors_now > number_errors_at_start)
						{
							return false;
						}
					}
					break;

				default:
					{
						boost::format msg("Incorrect import mode attempting to call block update function.");
						errorMsg = msg.str();
						errors.push_back(errorMsg);
						errorMsg.clear();
						return false;
					}
					break;

			}

		}
		else
		{
			boost::format msg("Incorrect model type in block update function.");
			errorMsg = msg.str();
			errors.push_back(errorMsg);
			errorMsg.clear();
			return false;
		}
	}
	catch (std::bad_cast &)
	{
		boost::format msg("Unable to cast to input model in block update function.");
		errorMsg = msg.str();
		errors.push_back(errorMsg);
		errorMsg.clear();
		return false;
	}

	return true;

}
