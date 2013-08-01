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

	// Map: UOA identifier => [ map: VG identifier => list of variables ]
	Table_VARIABLES_SELECTED::UOA_To_Variables_Map the_map = t_variables_selected_identifiers.GetSelectedVariablesByUOA(getDb(), this, &input_model);

	// Check for overlap in UOA's

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
			biggest_counts = uoa__to__dmu_counts__pair;
			return; // from lambda
		}
		first = false;
		
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
	std::for_each(biggest_counts.second.cbegin(), biggest_counts.second.cend(), [this, &multiplicities, &highest_multiplicity, &failed](Table_UOA_Identifier::DMU_Plus_Count const & dmu_category)
	{
		if (failed)
		{
			return; // from lamda
		}
		WidgetInstanceIdentifier const & the_dmu_category = dmu_category.first;
		if (!the_dmu_category.uuid)
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


	// The vector of variable groups corresponding to this UOA (independent of time granularity; i.e., all time granularities appear here)
	Table_VARIABLES_SELECTED::VariableGroup_To_VariableSelections_Map & variable_groups_map = the_map[*biggest_counts.first.uuid];

	// TODO: If more than one variable group corresponds to this UOA,
	// test for time range of each, and deal with that.
	// If the same time range, perform multiple SELECT's, one per VG,
	// and take the union.

	// For now, just use the first VG.
	bool first_vg = true;
	std::pair<WidgetInstanceIdentifier, WidgetInstanceIdentifiers> the_variable_group;
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

	sql_generate_output += "SELECT * FROM ";
	sql_generate_output += vg_data_table_name;
	sql_generate_output += " t1 ";
	char nBuffer[1024];
	for (int m=1; m<highest_multiplicity; ++m)
	{
		memset(nBuffer, '\0', 1024);
		sql_generate_output += "JOIN ";
		sql_generate_output += vg_data_table_name;
		sql_generate_output += " t";
		sql_generate_output += itoa(m+1, nBuffer, 10);
		sql_generate_output += " ON ";

	}

}
