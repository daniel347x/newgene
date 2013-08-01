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

std::string OutputModel::CurrentTableTokenName(int const multiplicity)
{
	char nBuffer[1024];
	memset(nBuffer, '\0', 1024);
	std::string current_table_token = "t";
	current_table_token += itoa(multiplicity, nBuffer, 10);
	return current_table_token;
}

void OutputModel::GenerateOutput(DataChangeMessage & change_response)
{


	InputModel & input_model = getInputModel();


	// ***************************************************************** //
	// the_map is:
	// Map: UOA identifier => [ map: VG identifier => list of variables ]
	// ***************************************************************** //
	Table_VARIABLES_SELECTED::UOA_To_Variables_Map the_map = t_variables_selected_identifiers.GetSelectedVariablesByUOA(getDb(), this, &input_model);

	// Check for overlap in UOA's

	// ************************************************************************** //
	// UOAs is:
	// Vector of: UOA identifier and its associated list of [DMU Category / Count]
	// ************************************************************************** //
	// Place UOA's and their associated DMU categories into a vector for convenience
	std::vector<std::pair<WidgetInstanceIdentifier, Table_UOA_Identifier::DMU_Counts>> UOAs;
	bool failed = false;
	std::for_each(the_map.cbegin(), the_map.cend(), [this, &input_model, &UOAs, &failed](std::pair<WidgetInstanceIdentifier /* UOA identifier */,
																								   Table_VARIABLES_SELECTED::VariableGroup_To_VariableSelections_Map> /* map: VG identifier => List of variables */
																																									  const & uoa__to__variable_groups__pair)
	{
		if (failed)
		{
			return; // from lambda
		}
		WidgetInstanceIdentifier const & uoa = uoa__to__variable_groups__pair.first;
		if (!uoa.uuid)
		{
			failed = true;
			return; // from lambda
		}
		Table_UOA_Identifier::DMU_Counts dmu_counts = input_model.t_uoa_category.RetrieveDMUCounts(getDb(), &input_model, *uoa.uuid);
		UOAs.push_back(std::make_pair(uoa, dmu_counts));
	});

	if (failed)
	{
		// Todo: Error message
		return;
	}

	// ************************************************************************************ //
	// biggest_counts is:
	// Pair consisting of: UOA identifier and its associated list of [DMU Category / Count]
	// ... for the UOA that has been determined to be the primary UOA
	// ************************************************************************************ //
	std::pair<WidgetInstanceIdentifier, Table_UOA_Identifier::DMU_Counts> biggest_counts;
	bool first = true;
	std::for_each(UOAs.cbegin(), UOAs.cend(), [&first, &biggest_counts, &failed](std::pair<WidgetInstanceIdentifier, Table_UOA_Identifier::DMU_Counts> const & uoa__to__dmu_counts__pair)
	{

		if (failed)
		{
			return; // from lambda
		}

		if (first)
		{
			first = false;
			biggest_counts = uoa__to__dmu_counts__pair;
			return; // from lambda
		}
		
		// Check if the current DMU_Counts overlaps (which is currently an error)
		// or is bigger
		Table_UOA_Identifier::DMU_Counts current_dmu_counts = uoa__to__dmu_counts__pair.second;
		bool current_is_bigger = false;
		bool current_is_smaller = false;
		bool current_is_same = true;

		std::for_each(current_dmu_counts.cbegin(), current_dmu_counts.cend(), [&biggest_counts, &current_is_bigger, &current_is_smaller, &current_is_same](Table_UOA_Identifier::DMU_Plus_Count const & current_dmu_plus_count)
		{
			bool matched_current_dmu = false;
			std::for_each(biggest_counts.second.cbegin(), biggest_counts.second.cend(), [&matched_current_dmu, &current_dmu_plus_count, &current_is_bigger, &current_is_smaller, &current_is_same](Table_UOA_Identifier::DMU_Plus_Count const & biggest_dmu_plus_count)
			{
				if (current_dmu_plus_count.first.IsEqual(WidgetInstanceIdentifier::EQUALITY_CHECK_TYPE__STRING_CODE, biggest_dmu_plus_count.first))
				{
					matched_current_dmu = true;
					if (current_dmu_plus_count.second > biggest_dmu_plus_count.second)
					{
						current_is_same = false;
						current_is_bigger = true;
					}
					else if (current_dmu_plus_count.second < biggest_dmu_plus_count.second)
					{
						current_is_same = false;
						current_is_smaller = true;
					}
				}
			});
			if (!matched_current_dmu)
			{
				// The DMU in the current UOA being tested does not exist in the "biggest" UOA previous to this
				current_is_same = false;
				current_is_bigger = true;
			}
		});

		std::for_each(biggest_counts.second.cbegin(), biggest_counts.second.cend(), [&current_dmu_counts, &current_is_bigger, &current_is_smaller, &current_is_same](Table_UOA_Identifier::DMU_Plus_Count const & biggest_dmu_plus_count)
		{
			bool matched_biggest_dmu = false;
			std::for_each(current_dmu_counts.cbegin(), current_dmu_counts.cend(), [&matched_biggest_dmu, &biggest_dmu_plus_count, &current_is_bigger, &current_is_smaller, &current_is_same](Table_UOA_Identifier::DMU_Plus_Count const & current_dmu_plus_count)
			{
				if (current_dmu_plus_count.first.IsEqual(WidgetInstanceIdentifier::EQUALITY_CHECK_TYPE__STRING_CODE, biggest_dmu_plus_count.first))
				{
					matched_biggest_dmu = true;
				}
			});
			if (!matched_biggest_dmu)
			{
				// The DMU in the "biggest" UOA does not exist in the current UOA being tested
				current_is_same = false;
				current_is_smaller = true;
			}
		});

		if (current_is_same)
		{
			if (current_is_smaller || current_is_bigger)
			{
				// error in algorithm logic
				failed = true;
				return; // from lambda
			}
			else
			{
				// ambiguity: two UOA's are the same
				// Todo: Implement this
				failed = true;
				return; // from lambda
			}
		}

		if (current_is_bigger)
		{
			biggest_counts = uoa__to__dmu_counts__pair;
		}

	});

	// Proceed to generate output based on "biggest_counts" UOA

	// We have the primary UOA
	// Now get the K-ad counts chosen by the user,
	// and compare the K-ad count for each DMU category
	// against the UOA's k-count for the same DMU category.
	// For each matching DMU category, determine the largest
	// whole multiple of the UOA's k-count that is equal to or
	// smaller than the user's k-selection, and store this
	// largest whole multiple in a vector.

	// Each multiple greater than 1 corresponds to running
	// a generation cycle, in which all other DMU columns
	// are fixed at a SINGLE choice of their possible column variants,
	// and the current DMU columns are a simple JOIN matching on the
	// other columns, one join per increment of the multiple
	// above 1.

	// The final result is the union of all such generation cycles.
	
	// To start off, only 1 generation cycle will be allowed
	// (i.e. only one multiple may be greater than 0).

	// For now: Make sure that at most 1 DMU has a multiplicity greater than 1
	if (!biggest_counts.first.uuid)
	{
		// Todo: Error message
		return;
	}
	failed = false;
	std::vector<int> multiplicities;
	int highest_multiplicity = 0;
	std::string highest_multiplicity_dmu_string_code;
	std::for_each(biggest_counts.second.cbegin(), biggest_counts.second.cend(), [this, &multiplicities, &highest_multiplicity, &highest_multiplicity_dmu_string_code, &failed](Table_UOA_Identifier::DMU_Plus_Count const & dmu_category)
	{
		if (failed)
		{
			return; // from lamda
		}
		WidgetInstanceIdentifier const & the_dmu_category = dmu_category.first;
		if (!the_dmu_category.uuid || !the_dmu_category.code)
		{
			failed = true;
			return; // from lambda
		}
		WidgetInstanceIdentifier_Int_Pair kad_count_pair = this->t_kad_count.getIdentifier(*the_dmu_category.uuid);
		int uoa_count_current_dmu_category = dmu_category.second;
		int kad_count_current_dmu_category = kad_count_pair.second;
		int multiplicity = 0;
		if (kad_count_current_dmu_category >= uoa_count_current_dmu_category)
		{
			multiplicity = 1;
		}
		if (multiplicity == 0)
		{
			failed = true;
			return; // from lambda
		}
		int test_kad_count = uoa_count_current_dmu_category;
		while (test_kad_count <= kad_count_current_dmu_category)
		{
			test_kad_count += uoa_count_current_dmu_category;
			if (test_kad_count <= kad_count_current_dmu_category)
			{
				++multiplicity;
			}
		}
		multiplicities.push_back(multiplicity);
		if (multiplicity > highest_multiplicity)
		{
			highest_multiplicity = multiplicity;
			highest_multiplicity_dmu_string_code = *the_dmu_category.code;
		}
	});

	if (failed)
	{
		// Todo: Error message
		return;
	}

	// check that only 1 has multiplicity > 1 (for now)
	bool anything_has_multiplicity_greater_than_1 = false;
	int which_index_has_multiplicity_greater_than_1 = -1;
	int current_index = 0;
	std::for_each(multiplicities.cbegin(), multiplicities.cend(), [&anything_has_multiplicity_greater_than_1, &which_index_has_multiplicity_greater_than_1, &current_index, &failed](int const & test_multiplicity)
	{
		if (failed)
		{
			return; // from lambda
		}
		if (test_multiplicity > 1)
		{
			if (anything_has_multiplicity_greater_than_1)
			{
				// **************************************************************************************************** //
				// A second multiplicity is greater than 1 - for now, not allowed.  This can be implemented in the future.
				// **************************************************************************************************** //
				failed = true;
				return; // from lambda
			}
			anything_has_multiplicity_greater_than_1 = true;
			which_index_has_multiplicity_greater_than_1 = current_index;
		}
		++current_index;
	});

	// Construct initial SQL
	std::string sql_generate_output;


	// variable_groups_map:
	// The vector of variable groups corresponding to this UOA (independent of time granularity; i.e., all time granularities appear here)
	// ... and the selected variables in that group
	Table_VARIABLES_SELECTED::VariableGroup_To_VariableSelections_Map & variable_groups_map = the_map[biggest_counts.first];

	// TODO: If more than one variable group corresponds to this UOA,
	// test for time range of each, and deal with that.
	// If the same time range, perform multiple SELECT's, one per VG,
	// and take the union.

	// For now, just use the first VG.
	// the_variable_group:
	// A pair: VG identifier -> Variables in this group selected by the user.
	// This VG is the first of those that correspond to the primary UOA.
	std::pair<WidgetInstanceIdentifier, WidgetInstanceIdentifiers> the_variable_group;
	bool first_vg = true;
	std::for_each(variable_groups_map.cbegin(), variable_groups_map.cend(), [&first_vg, &the_variable_group](std::pair<WidgetInstanceIdentifier, WidgetInstanceIdentifiers> const & one_variable_group)
	{
		if (!first_vg)
		{
			return; // from lambda - for now
		}
		first_vg = false;
		the_variable_group = one_variable_group;
	});

	if (!the_variable_group.first.code)
	{
		// Todo: error message
		return;
	}

	std::string vg_code = *the_variable_group.first.code;
	std::string vg_data_table_name = Table_VariableGroupData::TableNameFromVGCode(vg_code);

	WidgetInstanceIdentifiers const & dmu_primary_key_codes = input_model.t_vgp_data_metadata.getIdentifiers(vg_data_table_name);

	if (dmu_primary_key_codes.size() == 0)
	{
		// Todo: error message
		return;
	}

	// variables_selected_in_this_group:
	// A vector of variables selected by the user in the first variable group
	// corresponding to the primary UOA.
	WidgetInstanceIdentifiers & variables_selected_in_this_group = the_variable_group.second;

	// Todo: Enhance this SELECT by looping through all variable groups that correspond to the primary UOA
	// and doing the same SELECT from each of them into a temporary table, then performing a UNION.
	sql_generate_output += "SELECT ";
	bool first_select = true;
	for (int m=1; m<=highest_multiplicity; ++m)
	{
		std::string current_table_token = CurrentTableTokenName(m);
		char ns__[64];
		std::string ms__ = itoa(m, ns__, 10);
		std::for_each(variables_selected_in_this_group.cbegin(), variables_selected_in_this_group.cend(), [this, &first_select, &sql_generate_output, &current_table_token,&ms__, &failed](WidgetInstanceIdentifier const & variable_selected_in_this_group)
		{
			if (failed)
			{
				// Todo: Error message
				return; // from lambda
			}
			if (!variable_selected_in_this_group.code)
			{
				failed = true;
				return; // from lambda
			}
			if (!first_select)
			{
				sql_generate_output += ", ";
			}
			first_select = false;
			sql_generate_output += current_table_token;
			sql_generate_output += ".";
			sql_generate_output += *variable_selected_in_this_group.code;
			sql_generate_output += " AS ";
			sql_generate_output += *variable_selected_in_this_group.code;
			sql_generate_output += "_";
			sql_generate_output += ms__;
		});
	}
	sql_generate_output += " FROM ";
	sql_generate_output += vg_data_table_name;
	sql_generate_output += " t1";
	for (int m=1; m<highest_multiplicity; ++m)
	{

		std::string current_table_token = CurrentTableTokenName(m+1);

		std::for_each(dmu_primary_key_codes.cbegin(), dmu_primary_key_codes.cend(), [&sql_generate_output, &highest_multiplicity_dmu_string_code, &vg_data_table_name, &current_table_token, &failed](WidgetInstanceIdentifier const & dmu_identifier)
		{
			if (failed)
			{
				return; // from lambda
			}
			if (!dmu_identifier.code || !dmu_identifier.longhand) // The column name has been packed into "longhand" field
			{
				failed = true;
				return; // from lambda
			}

			if (*dmu_identifier.code == highest_multiplicity_dmu_string_code)
			{
				// This is one of the DMU's in the set of (identical code) DMU's with multiplicity greater than 1; ignore
				return; // from lambda
			}
			
			// good to go

			std::string vg_data_column_name = *dmu_identifier.longhand;

			sql_generate_output += " JOIN ";
			sql_generate_output += vg_data_table_name;
			sql_generate_output += " ";
			sql_generate_output += current_table_token;
			sql_generate_output += " ON ";

			sql_generate_output += current_table_token;
			sql_generate_output += ".";
			sql_generate_output += vg_data_column_name;
			sql_generate_output += " = ";
			sql_generate_output += "t1.";
			sql_generate_output += vg_data_column_name;
		});

	}

	if (failed)
	{
		// Todo: Error message
		return;
	}

	sqlite3_stmt * stmt = NULL;
	sqlite3_prepare_v2(db, sql_generate_output.c_str(), sql_generate_output.size() + 1, &stmt, NULL);
	if (stmt == NULL)
	{
		// Todo: Error message
		return;
	}

	int step_result = 0;
	while ((step_result = sqlite3_step(stmt)) == SQLITE_ROW)
	{
		//sqlite3
	}

}
