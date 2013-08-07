#include "OutputModel.h"
#include "..\Utilities\UUID.h"

#include <cstdint>

#ifndef Q_MOC_RUN
#	include <boost/lexical_cast.hpp>
#endif

void OutputModel::LoadTables()
{

	LoadDatabase();

	if (db != nullptr)
	{
		t_variables_selected_identifiers.Load(db, this, input_model.get());
		t_variables_selected_identifiers.Sort();

		t_kad_count.Load(db, this, input_model.get());
		t_kad_count.Sort();
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

std::string OutputModel::StripUUIDFromVariableName(std::string const & variable_name)
{
	std::string test_uuid = newUUID();
	size_t length_uuid = test_uuid.size();
	length_uuid += 1; // preceding underscore
	size_t length_variable_name = variable_name.size();
	int left_chars = (int)length_variable_name - (int)length_uuid;
	if (left_chars < 0)
	{
		return variable_name;
	}
	std::string stripped_variable_name = variable_name.substr(0, left_chars);
	return stripped_variable_name;
}

void OutputModel::GenerateOutput(DataChangeMessage & change_response)
{


	InputModel & input_model = getInputModel();
	sqlite3 * db = input_model.getDb();

	//std::string temp_dot("temp.");
	std::string temp_dot("");

	PrimaryKeySequence sequence;


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
	// biggest_counts and child_counts are:
	// Vector of: (one for each identical UOA, so that multiple, identical 
	// UOA's may appear as multiple vector elements,
	// except possibly for time granularity)
	// Pair consisting of: UOA identifier and its associated list of [DMU Category / Count]
	// ... for the UOA that has been determined to be the primary UOA.
	// Ditto child_counts, but for child UOA's, with the limitation that all DMU categories
	// but one must have 0 or the max number of DMU elements in the category.  For the one,
	// it can have one but no other term.
	// ************************************************************************************ //
	std::vector<std::pair<WidgetInstanceIdentifier, Table_UOA_Identifier::DMU_Counts>> biggest_counts;
	std::vector<std::pair<WidgetInstanceIdentifier, Table_UOA_Identifier::DMU_Counts>> child_counts;

	bool first = true;
	std::for_each(UOAs.cbegin(), UOAs.cend(), [&first, &biggest_counts, &child_counts, &failed](std::pair<WidgetInstanceIdentifier, Table_UOA_Identifier::DMU_Counts> const & uoa__to__dmu_counts__pair)
	{

		if (failed)
		{
			return; // from lambda
		}

		if (first)
		{
			first = false;
			biggest_counts.push_back(uoa__to__dmu_counts__pair);
			return; // from lambda
		}
		
		// Check if the current DMU_Counts overlaps (which is currently an error)
		// or is bigger
		Table_UOA_Identifier::DMU_Counts current_dmu_counts = uoa__to__dmu_counts__pair.second;
		bool current_is_bigger = false;
		bool current_is_smaller = false;
		bool current_is_same = true;

		int current = 0;
		std::for_each(current_dmu_counts.cbegin(), current_dmu_counts.cend(), [&biggest_counts, &current_is_bigger, &current_is_smaller, &current_is_same, &current](Table_UOA_Identifier::DMU_Plus_Count const & current_dmu_plus_count)
		{
			bool matched_current_dmu = false;
			// Looking at the first entry in biggest_counts is the same as looking at any other entry
			// in terms of the DMU counts
			std::for_each(biggest_counts[0].second.cbegin(), biggest_counts[0].second.cend(), [&matched_current_dmu, &current_dmu_plus_count, &current_is_bigger, &current_is_smaller, &current_is_same, &current](Table_UOA_Identifier::DMU_Plus_Count const & biggest_dmu_plus_count)
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
			++current;
		});

		// Looking at the first entry in biggest_counts is the same as looking at any other entry
		// in terms of the DMU counts
		std::for_each(biggest_counts[0].second.cbegin(), biggest_counts[0].second.cend(), [&current_dmu_counts, &current_is_bigger, &current_is_smaller, &current_is_same](Table_UOA_Identifier::DMU_Plus_Count const & biggest_dmu_plus_count)
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
				// This is just fine
				biggest_counts.push_back(uoa__to__dmu_counts__pair);
			}
		}
		else if (current_is_bigger && current_is_smaller)
		{
			// overlapping UOA's: not yet implemented
			// Todo: Error message
			failed = true;
			return; // from labmda
		}
		else if (current_is_bigger)
		{
			// Wipe the current UOA's and start fresh with this new, bigger one
			std::vector<std::pair<WidgetInstanceIdentifier, Table_UOA_Identifier::DMU_Counts>> new_child_uoas;
			biggest_counts.swap(new_child_uoas);
			child_counts.insert(child_counts.end(), new_child_uoas.cbegin(), new_child_uoas.cend());
			biggest_counts.push_back(uoa__to__dmu_counts__pair);
		}
		else if (current_is_smaller)
		{
			// Add to child UOA's here
			child_counts.push_back(uoa__to__dmu_counts__pair);
		}
		else
		{
			// error in algorithm logic
			failed = true;
			return; // from lambda
		}

	});

	if (failed)
	{
		// Todo: Error
		return;
	}


	// ******************************************************************************************* //
	// Variable groups for the primary UOA's, and for the child UOA's, have now been populated.
	// ******************************************************************************************* //


	// For primary UOA:
	// Make sure that at most 1 DMU has a multiplicity greater than 1,
	// and save the name/index of the DMU category with the highest multiplicity.
	std::vector<int> multiplicities;
	int highest_multiplicity = 0;
	std::string highest_multiplicity_dmu_string_code;

	// Looking at the first entry in biggest_counts is the same as looking at any other entry
	// in terms of the DMU counts
	std::for_each(biggest_counts[0].second.cbegin(), biggest_counts[0].second.cend(), [this, &multiplicities, &highest_multiplicity, &highest_multiplicity_dmu_string_code, &failed](Table_UOA_Identifier::DMU_Plus_Count const & dmu_category)
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
			// User's K-ad selection is too small in this DMU category to support the variables they have selected
			// Todo: Error message
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

	// check that only 1 primary UOA's DMU category has multiplicity > 1 (for now)
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
				// ********************************************************************************************************************** //
				// A second DMU category's multiplicity is greater than 1 - for now, not allowed.  This can be implemented in the future.
				// ********************************************************************************************************************** //
				failed = true;
				return; // from lambda
			}
			anything_has_multiplicity_greater_than_1 = true;
			which_index_has_multiplicity_greater_than_1 = current_index;
		}
		++current_index;
	});

	if (failed)
	{
		// Todo: Error message
		return;
	}

	// Validate child UOA's
	// For child UOA's:
	// Make sure that, for each child UOA, the actual k-value for all DMU categories
	// is either 0, 1, or the corresponding k-value of the primary UOA
	// and that where not 0, it is less than the corresponding k-value of the primary UOA
	// (i.e., has a value of 1)
	// for only 1 DMU category,
	// and this DMU category must match the DMU category with multiplicity greater than 1 (if any)
	// for the primary UOAs
	std::for_each(child_counts.cbegin(), child_counts.cend(), [&biggest_counts, &highest_multiplicity, &highest_multiplicity_dmu_string_code, &failed](std::pair<WidgetInstanceIdentifier, Table_UOA_Identifier::DMU_Counts> const & uoa__to__dmu_counts__pair)
	{
		if (failed)
		{
			return; // from lambda
		}
		int primary_dmu_categories_for_which_child_has_less = 0;
		Table_UOA_Identifier::DMU_Counts const & current_dmu_counts = uoa__to__dmu_counts__pair.second;
		std::for_each(current_dmu_counts.cbegin(), current_dmu_counts.cend(), [&biggest_counts, &primary_dmu_categories_for_which_child_has_less, &highest_multiplicity, &highest_multiplicity_dmu_string_code, &failed](Table_UOA_Identifier::DMU_Plus_Count const & current_dmu_plus_count)
		{
			if (failed)
			{
				return; // from lambda
			}
			if (current_dmu_plus_count.second == 0)
			{
				// just fine
				return; // from lambda
			}
			std::for_each(biggest_counts[0].second.cbegin(), biggest_counts[0].second.cend(), [&current_dmu_plus_count, &primary_dmu_categories_for_which_child_has_less, &highest_multiplicity, &highest_multiplicity_dmu_string_code, &failed](Table_UOA_Identifier::DMU_Plus_Count const & biggest_dmu_plus_count)
			{
				if (failed)
				{
					return; // from lambda
				}
				if (current_dmu_plus_count.first.IsEqual(WidgetInstanceIdentifier::EQUALITY_CHECK_TYPE__STRING_CODE, biggest_dmu_plus_count.first))
				{
					if (current_dmu_plus_count.second == biggest_dmu_plus_count.second) // biggest_dmu_plus_count.second is the K-value of the unit of analysis, not the K-value chosen by the user in the spin control
					{
						if (highest_multiplicity > 1)
						{
							// Special case:
							// The child UOA has the same K-value in this DMU category
							// as the K-value of the primary UOA,
							// but the K-value chosen by the user in the spin control for this DMU category
							// has incremented the multiplicity of the primary UOA for this DMU category greater than 1.
							++primary_dmu_categories_for_which_child_has_less;
						}
						return; // from lambda
					}
					if (current_dmu_plus_count.second > 1)
					{
						// Todo: error message
						// Invalid child UOA for this output
						failed = true;
						return; // from lambda
					}
					// Redundant 0-check for future code foolproofing
					else if (current_dmu_plus_count.second == 0)
					{
						// just fine
						return; // from lambda
					}
					// Current UOA's current DMU category's K-value is 1
					else if (!boost::iequals(*current_dmu_plus_count.first.code, highest_multiplicity_dmu_string_code))
					{
						if (highest_multiplicity > 1)
						{
							// Todo: error message

							// *************************************************************************************** //
							// Subtle edge case failure:
							//
							// Is this edge case common enough that it should be implemented for version 1 of NewGene?
							//
							// The current child DMU category has K-value less than the K-value
							// of the primary UOA's corresponding DMU category,
							// AND this is not the DMU category for which the primary UOA has
							// multiplicity greater than 1
							// *************************************************************************************** //

							// Invalid child UOA for this output
							failed = true;
							return; // from lambda
						}
					}
					++primary_dmu_categories_for_which_child_has_less;
				}
			});
		});

		if (primary_dmu_categories_for_which_child_has_less > 1)
		{
			// Todo: error message
			// Invalid child UOA for this output
			failed = true;
			return; // from lambda
		}

	});

	if (failed)
	{
		// todo: error message
		return;
	}

	// For child UOA's:
	// Save the name/index of the DMU category multiplicity greater than one.
	// Currently, this can be only one of the DMU categories.

	// child_uoas__which_multiplicity_is_greater_than_1:
	// Map of:
	// UOA identifier => pair<DMU category identifier of the effective DMU category that, in relation to the child, has multiplicity greater than 1, ... and the given multiplicity>
	std::map<WidgetInstanceIdentifier, std::pair<WidgetInstanceIdentifier, int>> child_uoas__which_multiplicity_is_greater_than_1;

	std::for_each(child_counts.cbegin(), child_counts.cend(), [this, &child_uoas__which_multiplicity_is_greater_than_1, &failed](std::pair<WidgetInstanceIdentifier, Table_UOA_Identifier::DMU_Counts> const & child_uoa__dmu_counts__pair)
	{

		if (failed)
		{
			return; // from lambda
		}

		WidgetInstanceIdentifier uoa_identifier = child_uoa__dmu_counts__pair.first;
		std::for_each(child_uoa__dmu_counts__pair.second.cbegin(), child_uoa__dmu_counts__pair.second.cend(), [this, &uoa_identifier, &child_uoas__which_multiplicity_is_greater_than_1, &failed](Table_UOA_Identifier::DMU_Plus_Count const & dmu_category)
		{

			if (failed)
			{
				return; // from lamda
			}

			WidgetInstanceIdentifier const & the_dmu_category_identifier = dmu_category.first;
			if (!the_dmu_category_identifier.uuid || !the_dmu_category_identifier.code)
			{
				failed = true;
				return; // from lambda
			}

			WidgetInstanceIdentifier_Int_Pair kad_count_pair = this->t_kad_count.getIdentifier(*the_dmu_category_identifier.uuid);
			int uoa_k_count__current_dmu_category = dmu_category.second;
			int k_spin_control_count__current_dmu_category = kad_count_pair.second;
			int multiplicity = 0;

			if (k_spin_control_count__current_dmu_category >= uoa_k_count__current_dmu_category)
			{
				multiplicity = 1;
			}
			if (multiplicity == 0)
			{
				// User's K-ad selection is too small in this DMU category to support the variables they have selected
				// Todo: Error message
				failed = true;
				return; // from lambda
			}

			int test_kad_count = uoa_k_count__current_dmu_category;
			while (test_kad_count <= k_spin_control_count__current_dmu_category)
			{
				test_kad_count += uoa_k_count__current_dmu_category;
				if (test_kad_count <= k_spin_control_count__current_dmu_category)
				{
					++multiplicity;
				}
			}

			// Note: Code above has already validated that there is only 1 DMU category for which the multiplicity is greater than 1.
			child_uoas__which_multiplicity_is_greater_than_1[uoa_identifier] = std::make_pair(the_dmu_category_identifier, multiplicity);

		});

	});

	if (failed)
	{
		// Todo: Error message
		return;
	}

	// variable_group__key_names__vectors:
	// A vector of: pair<vector<string>, vector<string>> - each such pair represents
	// one variable group (with at least one variable selected by the user for output)
	// ... and lists the internal *column names* used by this SQL query,
	// one vector for the primary keys, and one for the non-primary (i.e., secondary) keys.
	// Note that a UUID is appended to every variable name to avoid duplicates among variable groups.
	// Also note that the order is preserved (stored) here, in case the user names
	// columns in a different order with the same name in different variable groups.
	VariableGroup__PrimaryKey_SecondaryKey_Names__Vector variable_group__key_names__vectors;

	// variable_groups_vector:
	// Vector:
	// Variable group identifier,
	// The selected variables in that group.
	// (independent of time granularity; different time granularities can appear here)
	// (Also, multiple, identical UOA's are acceptable, possibly differing in time granularity)
	// I.e., possibly multiple PRIMARY variable groups, all corresponding to the same primary UOA (regardless of time granularity and/or UOA multiplicity)
	Table_VARIABLES_SELECTED::VariableGroup_To_VariableSelections_Vector variable_groups_vector;
	size_t number_primary_variable_groups = 0;
	std::for_each(biggest_counts.cbegin(), biggest_counts.cend(), [&variable_groups_vector, &number_primary_variable_groups, &the_map](std::pair<WidgetInstanceIdentifier, Table_UOA_Identifier::DMU_Counts> const & uoa__to__dmu_category_counts)
	{
		// Get all the variable groups corresponding to the current UOA in this iteration of the loop...
		// .. in this iteration of the loop,
		// these are all primary UOAs (i.e., identical except for time granularity)
		Table_VARIABLES_SELECTED::VariableGroup_To_VariableSelections_Map const & variable_groups_map_current = the_map[uoa__to__dmu_category_counts.first];
		number_primary_variable_groups += variable_groups_map_current.size();
		variable_groups_vector.insert(variable_groups_vector.end(), variable_groups_map_current.cbegin(), variable_groups_map_current.cend());
	});
	std::for_each(child_counts.cbegin(), child_counts.cend(), [&variable_groups_vector, &the_map](std::pair<WidgetInstanceIdentifier, Table_UOA_Identifier::DMU_Counts> const & uoa__to__dmu_category_counts)
	{
		Table_VARIABLES_SELECTED::VariableGroup_To_VariableSelections_Map const & variable_groups_map_current = the_map[uoa__to__dmu_category_counts.first];
		variable_groups_vector.insert(variable_groups_vector.end(), variable_groups_map_current.cbegin(), variable_groups_map_current.cend());
	});

	int overall_primary_key_sequence_number = 0;
	std::for_each(biggest_counts[0].second.cbegin(), biggest_counts[0].second.cend(), [this, &input_model, &sequence, &overall_primary_key_sequence_number, &variable_groups_vector, &failed](Table_UOA_Identifier::DMU_Plus_Count const & dmu_category)
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

		int kad_count_current_dmu_category_sequence = 0;
		for (int current_dmu_sequence_number = 0; current_dmu_sequence_number < kad_count_current_dmu_category; ++current_dmu_sequence_number)
		{
			if (kad_count_current_dmu_category_sequence == uoa_count_current_dmu_category)
			{
				kad_count_current_dmu_category_sequence = 0;
			}

			sequence.primary_key_sequence_info.push_back(PrimaryKeySequence::PrimaryKeySequenceEntry());
			PrimaryKeySequence::PrimaryKeySequenceEntry & current_primary_key_sequence = sequence.primary_key_sequence_info.back();
			std::vector<PrimaryKeySequence::VariableGroup_PrimaryKey_Info> & variable_group_info_for_primary_keys = current_primary_key_sequence.variable_group_info_for_primary_keys;

			current_primary_key_sequence.dmu_category = the_dmu_category;
			current_primary_key_sequence.sequence_number_within_dmu_category_spin_control = current_dmu_sequence_number;
			current_primary_key_sequence.sequence_number_within_dmu_category_primary_uoa = kad_count_current_dmu_category_sequence;
			current_primary_key_sequence.sequence_number_in_all_primary_keys = overall_primary_key_sequence_number;

			int view_count = 0;
			std::for_each(variable_groups_vector.cbegin(), variable_groups_vector.cend(), [this, &input_model, &the_dmu_category, &sequence, &current_dmu_sequence_number, &uoa_count_current_dmu_category, &kad_count_current_dmu_category, &current_primary_key_sequence, &variable_group_info_for_primary_keys, &failed](std::pair<WidgetInstanceIdentifier, WidgetInstanceIdentifiers> const & the_variable_group)
			{
				if (failed)
				{
					return; // from lambda
				}

				if (!the_variable_group.first.code)
				{
					// Todo: error message
					failed = true;
					return;
				}

				if (!the_variable_group.first.identifier_parent)
				{
					// Todo: error message
					failed = true;
					return;
				}

				WidgetInstanceIdentifier current_uoa_identifier = *the_variable_group.first.identifier_parent;

				std::string vg_code = *the_variable_group.first.code;
				std::string vg_data_table_name = Table_VariableGroupData::TableNameFromVGCode(vg_code);

				// dmu_primary_key_codes_metadata:
				// The primary key metadata for the_variable_group - a possible SUBSET of all primary keys from the primary UOA.
				// This include *all* DMU categories that form the primary keys for this variable group,
				// each of which might appear multiple times as a separate entry here.
				// This metadata just states which DMU category the key refers to,
				// what the total (overall) sequence number is of the primary key in the variable group table,
				// and the column name in the variable group table for this column.
				// In the WidgetInstanceIdentifier, the CODE is set to the DMU category code,
				// the LONGHAND is set to the column name corresponding to this DMU in the variable group data table,
				// and the SEQUENCE NUMBER is set to the sequence number of the primary key in this variable group.
				WidgetInstanceIdentifiers const & dmu_primary_key_codes_metadata = input_model.t_vgp_data_metadata.getIdentifiers(vg_data_table_name);

				// Todo: To implement global variables (i.e., variables with no primary key),
				// make the necessary changes and then remove the following requirement
				if (dmu_primary_key_codes_metadata.size() == 0)
				{
					// Todo: error message
					failed = true;
					return;
				}

				int current_variable_group_current_primary_key_dmu_category_total_number = 0;
				std::for_each(dmu_primary_key_codes_metadata.cbegin(), dmu_primary_key_codes_metadata.cend(), [this, &input_model, &the_dmu_category, &current_variable_group_current_primary_key_dmu_category_total_number, &sequence, &current_dmu_sequence_number, &uoa_count_current_dmu_category, &kad_count_current_dmu_category, &current_primary_key_sequence, &variable_group_info_for_primary_keys, &failed](WidgetInstanceIdentifier const & current_variable_group_current_dmu_primary_key)
				{
					if (current_variable_group_current_dmu_primary_key.IsEqual(WidgetInstanceIdentifier::EQUALITY_CHECK_TYPE__STRING_CODE, the_dmu_category))
					{
						++current_variable_group_current_primary_key_dmu_category_total_number;
					}
				});

				int multiplicity = 0;
				if (current_variable_group_current_primary_key_dmu_category_total_number > 0)
				{
					multiplicity = 1;
					int test_kad_count = current_variable_group_current_primary_key_dmu_category_total_number;
					while (test_kad_count <= kad_count_current_dmu_category)
					{
						test_kad_count += current_variable_group_current_primary_key_dmu_category_total_number;
						if (test_kad_count <= kad_count_current_dmu_category)
						{
							++multiplicity;
						}
					}
				}

				variable_group_info_for_primary_keys.push_back(PrimaryKeySequence::VariableGroup_PrimaryKey_Info());
				PrimaryKeySequence::VariableGroup_PrimaryKey_Info & current_variable_group_current_primary_key_info = variable_group_info_for_primary_keys.back();
				current_variable_group_current_primary_key_info.vg_identifier = the_variable_group.first;
				current_variable_group_current_primary_key_info.is_primary_column_selected = false;
				current_variable_group_current_primary_key_info.associated_uoa_identifier = current_uoa_identifier;

				int current_variable_group_current_primary_key_dmu_category_total_sequence_number = 0;
				for (int m=0; m<multiplicity; ++m)
				{
					int current_variable_group_current_primary_key_dmu_category_sequence_number = 0;
					std::for_each(dmu_primary_key_codes_metadata.cbegin(), dmu_primary_key_codes_metadata.cend(), [this, &m, &multiplicity, &current_variable_group_current_primary_key_info, &the_variable_group, &current_variable_group_current_primary_key_dmu_category_total_sequence_number, &input_model, &the_dmu_category, &current_variable_group_current_primary_key_dmu_category_sequence_number, &sequence, &current_dmu_sequence_number, &uoa_count_current_dmu_category, &kad_count_current_dmu_category, &current_primary_key_sequence, &variable_group_info_for_primary_keys, &failed](WidgetInstanceIdentifier const & current_variable_group_current_dmu_primary_key)
					{
						if (failed)
						{
							return; // from lambda
						}
						if (current_variable_group_current_dmu_primary_key.IsEqual(WidgetInstanceIdentifier::EQUALITY_CHECK_TYPE__STRING_CODE, the_dmu_category))
						{
							if (current_dmu_sequence_number == current_variable_group_current_primary_key_dmu_category_total_sequence_number)
							{
								// In the WidgetInstanceIdentifier, the CODE is set to the DMU category code,
								// the LONGHAND is set to the column name corresponding to this DMU in the variable group data table,
								// and the SEQUENCE NUMBER is set to the sequence number of the primary key in this variable group.
								current_variable_group_current_primary_key_info.table_column_name = *current_variable_group_current_dmu_primary_key.longhand;
								current_variable_group_current_primary_key_info.sequence_number_within_dmu_category_variable_group_uoa = current_variable_group_current_primary_key_dmu_category_sequence_number;
								current_variable_group_current_primary_key_info.current_multiplicity = m+1;
								current_variable_group_current_primary_key_info.total_multiplicity = multiplicity;
								char ns__[64];
								current_variable_group_current_primary_key_info.view_table_name = "t";
								current_variable_group_current_primary_key_info.view_table_name += itoa(m+1, ns__, 10);
								current_variable_group_current_primary_key_info.join_table_name = "j";
								current_variable_group_current_primary_key_info.join_table_name += itoa(m+1, ns__, 10);
								current_variable_group_current_primary_key_info.join_table_name_withtime = "jt";
								current_variable_group_current_primary_key_info.join_table_name_withtime += itoa(m+1, ns__, 10);

								if (current_variable_group_current_primary_key_info.associated_uoa_identifier.time_granularity != 0)
								{
									current_variable_group_current_primary_key_info.datetime_row_start_table_column_name = "DATETIME_ROW_START";
									current_variable_group_current_primary_key_info.datetime_row_end_table_column_name = "DATETIME_ROW_END";

									std::string datetime_start_row_name = "DATETIME_ROW_START_";
									datetime_start_row_name += *current_variable_group_current_primary_key_info.vg_identifier.code;
									current_variable_group_current_primary_key_info.datetime_row_start_column_name = datetime_start_row_name;
									current_variable_group_current_primary_key_info.datetime_row_start_column_name_no_uuid = datetime_start_row_name;

									std::string datetime_end_row_name = "DATETIME_ROW_END_";
									datetime_end_row_name += *current_variable_group_current_primary_key_info.vg_identifier.code;
									current_variable_group_current_primary_key_info.datetime_row_end_column_name = datetime_end_row_name;
									current_variable_group_current_primary_key_info.datetime_row_end_column_name_no_uuid = datetime_end_row_name;
								}

								std::string this_variable_group__this_primary_key__unique_name;
								this_variable_group__this_primary_key__unique_name += *current_variable_group_current_dmu_primary_key.longhand;
								current_variable_group_current_primary_key_info.column_name_no_uuid = this_variable_group__this_primary_key__unique_name;
								if (multiplicity > 1)
								{
									this_variable_group__this_primary_key__unique_name += "_";
									this_variable_group__this_primary_key__unique_name += itoa(m+1, ns__, 10);
								}
								this_variable_group__this_primary_key__unique_name += "_";
								this_variable_group__this_primary_key__unique_name += newUUID(true);
								current_variable_group_current_primary_key_info.column_name = this_variable_group__this_primary_key__unique_name;
								WidgetInstanceIdentifier vg_setmember_identifier;
								bool found_variable_group_set_member_identifier = input_model.t_vgp_setmembers.getIdentifierFromStringCodeAndParentUUID(*current_variable_group_current_dmu_primary_key.longhand, *the_variable_group.first.uuid, vg_setmember_identifier);
								if (!found_variable_group_set_member_identifier)
								{
									failed = true;
									return; // from lambda
								}

								// Is this primary key selected by the user for output?
								bool found = false;
								std::for_each(the_variable_group.second.cbegin(), the_variable_group.second.cend(), [&found, &vg_setmember_identifier](WidgetInstanceIdentifier const & selected_variable_identifier)
								{
									if (found)
									{
										return; // from lambda
									}
									if (selected_variable_identifier.IsEqual(WidgetInstanceIdentifier::EQUALITY_CHECK_TYPE__UUID_PLUS_STRING_CODE, vg_setmember_identifier))
									{
										found = true;
									}
								});
								if (found)
								{
									current_variable_group_current_primary_key_info.is_primary_column_selected = true;
								}
								else
								{
									current_variable_group_current_primary_key_info.is_primary_column_selected = false;
								}
							}
							++current_variable_group_current_primary_key_dmu_category_sequence_number;
							++current_variable_group_current_primary_key_dmu_category_total_sequence_number;
						}
					});
				}

				if (failed)
				{
					return; // from lambda
				}
			});

			if (failed)
			{
				return; // from lambda
			}

			++overall_primary_key_sequence_number;
			++kad_count_current_dmu_category_sequence;
		}

	});

	if (failed)
	{
		// Todo: Error message
		return;
	}

	ColumnsInViews columnsInViews;

	int view_count = 0;
	std::for_each(variable_groups_vector.cbegin(), variable_groups_vector.cend(), [this, &db, &sequence, &columnsInViews, &temp_dot, &view_count, &input_model, &number_primary_variable_groups, &highest_multiplicity, &highest_multiplicity_dmu_string_code, &variable_group__key_names__vectors, &child_uoas__which_multiplicity_is_greater_than_1, &failed](std::pair<WidgetInstanceIdentifier, WidgetInstanceIdentifiers> const & the_variable_group)
	{
		// the_variable_group:
		// A pair: VG identifier -> Variables in this group selected by the user.
		// This VG is one of those that correspond to the current UOA.

		if (failed)
		{
			return; // from lambda
		}

		if (!the_variable_group.first.code)
		{
			// Todo: error message
			failed = true;
			return;
		}

		if (!the_variable_group.first.identifier_parent)
		{
			// Todo: error message
			failed = true;
			return;
		}

		++view_count;

		columnsInViews.columns_in_views.push_back(ColumnsInViews::ColumnsInView());
		ColumnsInViews::ColumnsInView & columnsInView = columnsInViews.columns_in_views.back();
		columnsInView.view_number = view_count;

		WidgetInstanceIdentifier current_uoa_identifier = *the_variable_group.first.identifier_parent;

		if ((size_t)view_count > number_primary_variable_groups)
		{

			if (child_uoas__which_multiplicity_is_greater_than_1.find(current_uoa_identifier) == child_uoas__which_multiplicity_is_greater_than_1.end())
			{
				// No UOA associated with this variable group!
				// Todo: error message
				failed = true;
				return;
			}

			if (!child_uoas__which_multiplicity_is_greater_than_1[current_uoa_identifier].first.code)
			{
				// No UOA code associated with this variable group!
				// Todo: error message
				failed = true;
				return;
			}

		}

		int highest_multiplicity_to_use = highest_multiplicity;
		std::string highest_multiplicity_dmu_string_code_to_use = highest_multiplicity_dmu_string_code;
		if ((size_t)view_count > number_primary_variable_groups)
		{
			highest_multiplicity_to_use = child_uoas__which_multiplicity_is_greater_than_1[current_uoa_identifier].second;
			highest_multiplicity_dmu_string_code_to_use = *child_uoas__which_multiplicity_is_greater_than_1[current_uoa_identifier].first.code;
		}

		variable_group__key_names__vectors.push_back(std::make_pair(std::vector<std::string>(), std::vector<std::string>()));
		PrimaryKey_SecondaryKey_Names & this_variable_group__key_names = variable_group__key_names__vectors.back();
		std::vector<std::string> & this_variable_group__primary_key_names = this_variable_group__key_names.first;
		std::vector<std::string> & this_variable_group__secondary_key_names = this_variable_group__key_names.second;
		std::vector<std::string> this_variable_group__primary_key_names__uuid_stripped;
		std::vector<std::string> this_variable_group__secondary_key_names__uuid_stripped;

		std::string vg_code = *the_variable_group.first.code;
		std::string vg_data_table_name = Table_VariableGroupData::TableNameFromVGCode(vg_code);

		// dmu_primary_key_codes:
		// The primary key metadata for the_variable_group.
		// This include *all* DMU categories that form the primary key,
		// each of which might appear multiple times as a separate entry here.
		// This metadata just states which DMU category the key refers to,
		// what the total (overall) sequence number is of the primary key in the variable group table,
		// and the column name in the variable group table for this column.
		WidgetInstanceIdentifiers const & dmu_primary_key_codes_metadata = input_model.t_vgp_data_metadata.getIdentifiers(vg_data_table_name);

		// Todo: To implement global variables (i.e., variables with no primary key),
		// make the necessary changes and then remove the following requirement
		if (dmu_primary_key_codes_metadata.size() == 0)
		{
			// Todo: error message
			failed = true;
			return;
		}

		// variables_selected_in_this_group:
		// A vector of variables selected by the user in the current variable group
		WidgetInstanceIdentifiers const & variables_selected_in_this_group = the_variable_group.second;


		// *************************************************************************************** //
		// Construct the SQL used to create a (temporary in release mode) View
		// that caches all primary keys + variable data for the current variable group
		// *************************************************************************************** //

		std::string sql_generate_output;

		sql_generate_output += "CREATE VIEW ";
		sql_generate_output += temp_dot;
		sql_generate_output += Table_VariableGroupData::ViewNameFromCount(view_count);
		sql_generate_output += " AS SELECT ";

		bool first_select = true;
		if (current_uoa_identifier.time_granularity != 0)
		{
			// The current variable group being joined into the full set of data
			// has a time range granularity associated with it

			// At least one of the primary keys has this variable group active, so use that to get the date-time column names
			// ... just break out of loop once we have it
			std::for_each(sequence.primary_key_sequence_info.cbegin(), sequence.primary_key_sequence_info.cend(), [&sql_generate_output, &columnsInView, &first_select, &the_variable_group, &failed](PrimaryKeySequence::PrimaryKeySequenceEntry const & total_primary_key_sequence_entry)
			{
				if (first_select == false)
				{
					// We already have the information we need
					return; // from lambda
				}
				std::vector<PrimaryKeySequence::VariableGroup_PrimaryKey_Info> const & variable_group_info_for_primary_keys = total_primary_key_sequence_entry.variable_group_info_for_primary_keys;
				std::for_each(variable_group_info_for_primary_keys.cbegin(), variable_group_info_for_primary_keys.cend(), [&sql_generate_output, &columnsInView, &first_select, &the_variable_group, &failed](PrimaryKeySequence::VariableGroup_PrimaryKey_Info const & variable_group_primary_key_info)
				{
					if (first_select == false)
					{
						// We already have the information we need
						return; // from lambda
					}
					if (variable_group_primary_key_info.vg_identifier.IsEqual(WidgetInstanceIdentifier::EQUALITY_CHECK_TYPE__STRING_CODE, the_variable_group.first))		
					{
						if (!variable_group_primary_key_info.column_name.empty())
						{
							first_select = false;
							sql_generate_output += variable_group_primary_key_info.view_table_name;
							sql_generate_output += ".";
							sql_generate_output += variable_group_primary_key_info.datetime_row_start_table_column_name;
							sql_generate_output += " AS ";
							sql_generate_output += variable_group_primary_key_info.datetime_row_start_column_name;
							sql_generate_output += ", ";
							sql_generate_output += variable_group_primary_key_info.view_table_name;
							sql_generate_output += ".";
							sql_generate_output += variable_group_primary_key_info.datetime_row_end_table_column_name;
							sql_generate_output += " AS ";
							sql_generate_output += variable_group_primary_key_info.datetime_row_end_column_name;

							std::vector<ColumnsInViews::ColumnsInView::ColumnInView> & columns_in_view = columnsInView.columns_in_view;

							columns_in_view.push_back(ColumnsInViews::ColumnsInView::ColumnInView());
							ColumnsInViews::ColumnsInView::ColumnInView & column_in_view_datetime_start = columns_in_view.back();
							column_in_view_datetime_start.column_name = variable_group_primary_key_info.datetime_row_start_column_name;
							column_in_view_datetime_start.column_name_no_uuid = variable_group_primary_key_info.datetime_row_start_column_name_no_uuid;
							column_in_view_datetime_start.table_column_name = variable_group_primary_key_info.datetime_row_start_table_column_name;
							column_in_view_datetime_start.variable_group_identifier = variable_group_primary_key_info.vg_identifier;
							column_in_view_datetime_start.uoa_associated_with_variable_group_identifier = *variable_group_primary_key_info.vg_identifier.identifier_parent;
							column_in_view_datetime_start.column_type = ColumnsInViews::ColumnsInView::ColumnInView::COLUMN_TYPE__DATETIMESTART;

							columns_in_view.push_back(ColumnsInViews::ColumnsInView::ColumnInView());
							ColumnsInViews::ColumnsInView::ColumnInView & column_in_view_datetime_end = columns_in_view.back();
							column_in_view_datetime_end.column_name = variable_group_primary_key_info.datetime_row_end_column_name;
							column_in_view_datetime_end.column_name_no_uuid = variable_group_primary_key_info.datetime_row_end_column_name_no_uuid;
							column_in_view_datetime_end.table_column_name = variable_group_primary_key_info.datetime_row_end_table_column_name;
							column_in_view_datetime_end.variable_group_identifier = variable_group_primary_key_info.vg_identifier;
							column_in_view_datetime_end.uoa_associated_with_variable_group_identifier = *variable_group_primary_key_info.vg_identifier.identifier_parent;
							column_in_view_datetime_end.column_type = ColumnsInViews::ColumnsInView::ColumnInView::COLUMN_TYPE__DATETIMEEND;
						}
					}
				});
			});
		}

		std::for_each(sequence.primary_key_sequence_info.cbegin(), sequence.primary_key_sequence_info.cend(), [&sql_generate_output, &columnsInView, &first_select, &the_variable_group, &failed](PrimaryKeySequence::PrimaryKeySequenceEntry const & total_primary_key_sequence_entry)
		{
			std::vector<PrimaryKeySequence::VariableGroup_PrimaryKey_Info> const & variable_group_info_for_primary_keys = total_primary_key_sequence_entry.variable_group_info_for_primary_keys;
			std::for_each(variable_group_info_for_primary_keys.cbegin(), variable_group_info_for_primary_keys.cend(), [&sql_generate_output, &columnsInView, &first_select, &the_variable_group, &total_primary_key_sequence_entry, &failed](PrimaryKeySequence::VariableGroup_PrimaryKey_Info const & variable_group_primary_key_info)
			{
				if (variable_group_primary_key_info.vg_identifier.IsEqual(WidgetInstanceIdentifier::EQUALITY_CHECK_TYPE__STRING_CODE, the_variable_group.first))		
				{
					if (!variable_group_primary_key_info.column_name.empty())
					{
						if (!first_select)
						{
							sql_generate_output += ", ";
						}
						first_select = false;
						sql_generate_output += variable_group_primary_key_info.view_table_name;
						sql_generate_output += ".";
						sql_generate_output += variable_group_primary_key_info.table_column_name;
						sql_generate_output += " AS ";
						sql_generate_output += variable_group_primary_key_info.column_name;

						std::vector<ColumnsInViews::ColumnsInView::ColumnInView> & columns_in_view = columnsInView.columns_in_view;

						columns_in_view.push_back(ColumnsInViews::ColumnsInView::ColumnInView());
						ColumnsInViews::ColumnsInView::ColumnInView & column_in_view = columns_in_view.back();
						column_in_view.column_name = variable_group_primary_key_info.column_name;
						column_in_view.column_name_no_uuid = variable_group_primary_key_info.column_name_no_uuid;
						column_in_view.table_column_name = variable_group_primary_key_info.table_column_name;
						column_in_view.variable_group_identifier = variable_group_primary_key_info.vg_identifier;
						column_in_view.uoa_associated_with_variable_group_identifier = *variable_group_primary_key_info.vg_identifier.identifier_parent;
						column_in_view.column_type = ColumnsInViews::ColumnsInView::ColumnInView::COLUMN_TYPE__PRIMARY;
						column_in_view.primary_key_dmu_category_identifier = total_primary_key_sequence_entry.dmu_category;
						column_in_view.primary_key_index_within_total_kad_for_dmu_category = total_primary_key_sequence_entry.sequence_number_within_dmu_category_spin_control;
						column_in_view.primary_key_index_within_total_kad_for_all_dmu_categories = total_primary_key_sequence_entry.sequence_number_in_all_primary_keys;
						column_in_view.primary_key_index_within_uoa_corresponding_to_variable_group_for_dmu_category = variable_group_primary_key_info.sequence_number_within_dmu_category_variable_group_uoa;
						column_in_view.primary_key_index_within_primary_uoa_for_dmu_category = total_primary_key_sequence_entry.sequence_number_within_dmu_category_primary_uoa;
					}
				}
			});
		});

		if (failed)
		{
			// Todo: Error message
			return;
		}


		//std::for_each(this_variable_group__primary_key_names.cbegin(), this_variable_group__primary_key_names.cend(), [this, &this_variable_group__primary_key_names__uuid_stripped](std::string const & this_variable_group__primary_key_name)
		//{
		//	this_variable_group__primary_key_names__uuid_stripped.push_back(this->StripUUIDFromVariableName(this_variable_group__primary_key_name));
		//});


		// ************************************************* //
		// Display all variables selected by user
		// ************************************************* //
		for (int m=1; m<=highest_multiplicity_to_use; ++m)
		{

			std::string current_table_token = CurrentTableTokenName(m);
			char ns__[64];
			std::string ms__ = itoa(m, ns__, 10);

			std::for_each(variables_selected_in_this_group.cbegin(), variables_selected_in_this_group.cend(), [this, &columnsInView, &the_variable_group, &highest_multiplicity_to_use, &current_uoa_identifier, &first_select, &sql_generate_output, &current_table_token, &ms__, &this_variable_group__primary_key_names__uuid_stripped, &this_variable_group__secondary_key_names, &child_uoas__which_multiplicity_is_greater_than_1, &failed](WidgetInstanceIdentifier const & variable_selected_in_this_group)
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

				std::string this_variable_group_secondary_key_name;
				this_variable_group_secondary_key_name += *variable_selected_in_this_group.code;

				if (highest_multiplicity_to_use > 1)
				{
					this_variable_group_secondary_key_name += "_";
					this_variable_group_secondary_key_name += ms__;
				}

				std::string saved_secondary_key_name_no_uuid = this_variable_group_secondary_key_name;

				this_variable_group_secondary_key_name += "_";
				this_variable_group_secondary_key_name += newUUID(true);

				this_variable_group__secondary_key_names.push_back(this_variable_group_secondary_key_name);

				if (!first_select)
				{
					sql_generate_output += ", ";
				}
				first_select = false;

				sql_generate_output += current_table_token;
				sql_generate_output += ".";
				sql_generate_output += *variable_selected_in_this_group.code;
				sql_generate_output += " AS ";
				sql_generate_output += this_variable_group_secondary_key_name;

				std::vector<ColumnsInViews::ColumnsInView::ColumnInView> & columns_in_view = columnsInView.columns_in_view;

				columns_in_view.push_back(ColumnsInViews::ColumnsInView::ColumnInView());
				ColumnsInViews::ColumnsInView::ColumnInView & column_in_view = columns_in_view.back();
				column_in_view.column_name = this_variable_group_secondary_key_name;
				column_in_view.column_name_no_uuid = saved_secondary_key_name_no_uuid;
				column_in_view.table_column_name = *variable_selected_in_this_group.code;
				column_in_view.variable_group_identifier = the_variable_group.first;
				column_in_view.uoa_associated_with_variable_group_identifier = *the_variable_group.first.identifier_parent;
				column_in_view.column_type = ColumnsInViews::ColumnsInView::ColumnInView::COLUMN_TYPE__SECONDARY;
			});

		}

		if (failed)
		{
			// Todo: Error message
			return;
		}

		std::for_each(this_variable_group__secondary_key_names.cbegin(), this_variable_group__secondary_key_names.cend(), [this, &this_variable_group__secondary_key_names__uuid_stripped](std::string const & this_variable_group__secondary_key_name)
		{
			this_variable_group__secondary_key_names__uuid_stripped.push_back(this->StripUUIDFromVariableName(this_variable_group__secondary_key_name));
		});

		sql_generate_output += " FROM ";
		sql_generate_output += vg_data_table_name;
		sql_generate_output += " t1";
		for (int m=1; m<highest_multiplicity_to_use; ++m)
		{

			std::string current_table_token = CurrentTableTokenName(m+1);

			sql_generate_output += " JOIN ";
			sql_generate_output += vg_data_table_name;
			sql_generate_output += " ";
			sql_generate_output += current_table_token;
			sql_generate_output += " ON ";

			bool and_required = false;
			if (current_uoa_identifier.time_granularity != TIME_GRANULARITY__NONE)
			{
				// Always match on time range fields
				sql_generate_output += current_table_token;
				sql_generate_output += ".";
				sql_generate_output += "DATETIME_ROW_START";
				sql_generate_output += " = ";
				sql_generate_output += "t1.";
				sql_generate_output += "DATETIME_ROW_START";
				sql_generate_output += " AND ";
				sql_generate_output += current_table_token;
				sql_generate_output += ".";
				sql_generate_output += "DATETIME_ROW_END";
				sql_generate_output += " = ";
				sql_generate_output += "t1.";
				sql_generate_output += "DATETIME_ROW_END";
				and_required = true;
			}

			std::for_each(sequence.primary_key_sequence_info.cbegin(), sequence.primary_key_sequence_info.cend(), [&sql_generate_output, &and_required, &current_table_token, &the_variable_group, &failed](PrimaryKeySequence::PrimaryKeySequenceEntry const & total_primary_key_sequence_entry)
			{
				std::vector<PrimaryKeySequence::VariableGroup_PrimaryKey_Info> const & variable_group_info_for_primary_keys = total_primary_key_sequence_entry.variable_group_info_for_primary_keys;
				std::for_each(variable_group_info_for_primary_keys.cbegin(), variable_group_info_for_primary_keys.cend(), [&sql_generate_output, &and_required, &current_table_token, &the_variable_group, &failed](PrimaryKeySequence::VariableGroup_PrimaryKey_Info const & variable_group_primary_key_info)
				{
					if (variable_group_primary_key_info.vg_identifier.IsEqual(WidgetInstanceIdentifier::EQUALITY_CHECK_TYPE__STRING_CODE, the_variable_group.first))		
					{
						if (!variable_group_primary_key_info.column_name.empty())
						{
							if (variable_group_primary_key_info.total_multiplicity == 1)
							{
								std::string vg_data_column_name = variable_group_primary_key_info.table_column_name;

								if (and_required)
								{
									sql_generate_output += " AND ";
								}
								and_required = true;

								sql_generate_output += current_table_token;
								sql_generate_output += ".";
								sql_generate_output += vg_data_column_name;
								sql_generate_output += " = ";
								sql_generate_output += "t1.";
								sql_generate_output += vg_data_column_name; // Creating VIEW views, not JOIN views, so it's a self-join; column names are the same
							}
						}
					}
				});
			});

		}

		sqlite3_stmt * stmt = NULL;
		sqlite3_prepare_v2(db, sql_generate_output.c_str(), sql_generate_output.size() + 1, &stmt, NULL);
		if (stmt == NULL)
		{
			// Todo: Error message
			failed = true;
			return;
		}

		int step_result = 0;
		if ((step_result = sqlite3_step(stmt)) != SQLITE_DONE)
		{
			// Todo: Error message
			failed = true;
			return; // from lambda
		}

	});

	int join_count = 0;
	std::for_each(variable_groups_vector.cbegin(), variable_groups_vector.cend(), [this, &sequence, &columnsInViews, &temp_dot, &db, &join_count, &input_model, &variable_group__key_names__vectors, &failed](std::pair<WidgetInstanceIdentifier, WidgetInstanceIdentifiers> const & the_variable_group)
	{
		if (failed)
		{
			return; // from lambda
		}
		++join_count;

		char join_count_as_text_[1024];
		std::string join_count_as_text = itoa(join_count, join_count_as_text_, 10);

		PrimaryKey_SecondaryKey_Names & this_variable_group__key_names = variable_group__key_names__vectors[join_count - 1];
		std::vector<std::string> & this_variable_group__primary_key_names = this_variable_group__key_names.first;
		std::vector<std::string> & this_variable_group__secondary_key_names = this_variable_group__key_names.second;

		std::string vg_code = *the_variable_group.first.code;
		std::string vg_data_table_name = Table_VariableGroupData::TableNameFromVGCode(vg_code);

		std::string sql_generate_output;
		sql_generate_output += "CREATE VIEW ";
		sql_generate_output += temp_dot;
		sql_generate_output += Table_VariableGroupData::JoinViewNameFromCount(join_count);
		sql_generate_output += " AS SELECT ";
		sql_generate_output += Table_VariableGroupData::ViewNameFromCount(join_count);
		sql_generate_output += ".*";
		if (join_count > 1)
		{
			// join with the previous join
			sql_generate_output += ", ";
			sql_generate_output += Table_VariableGroupData::JoinViewNameWithTimeRangesFromCount(join_count - 1);
			sql_generate_output += ".*";

		}
		sql_generate_output += " FROM ";
		sql_generate_output += temp_dot;
		sql_generate_output += Table_VariableGroupData::ViewNameFromCount(join_count);
		sql_generate_output += " ";
		sql_generate_output += Table_VariableGroupData::ViewNameFromCount(join_count);
		if (join_count > 1)
		{
		
			// join with the previous join

			PrimaryKey_SecondaryKey_Names & this_variable_group__key_names__previous = variable_group__key_names__vectors[join_count - 2];
			std::vector<std::string> & this_variable_group__primary_key_names__previous = this_variable_group__key_names__previous.first;
			std::vector<std::string> & this_variable_group__secondary_key_names__previous = this_variable_group__key_names__previous.second;

			sql_generate_output += " LEFT OUTER JOIN ";
			sql_generate_output += temp_dot;
			sql_generate_output += Table_VariableGroupData::JoinViewNameWithTimeRangesFromCount(join_count - 1);
			sql_generate_output += " ";
			sql_generate_output += Table_VariableGroupData::JoinViewNameWithTimeRangesFromCount(join_count - 1);
			sql_generate_output += " ON ";

			bool first_select = true;
			std::for_each(sequence.primary_key_sequence_info.cbegin(), sequence.primary_key_sequence_info.cend(), [&sql_generate_output, &first_select, &join_count, &the_variable_group, &failed](PrimaryKeySequence::PrimaryKeySequenceEntry const & total_primary_key_sequence_entry)
			{
				std::vector<PrimaryKeySequence::VariableGroup_PrimaryKey_Info> const & variable_group_info_for_primary_keys = total_primary_key_sequence_entry.variable_group_info_for_primary_keys;
				std::for_each(variable_group_info_for_primary_keys.cbegin(), variable_group_info_for_primary_keys.cend(), [&sql_generate_output, &variable_group_info_for_primary_keys, &first_select, &join_count, &the_variable_group, &failed](PrimaryKeySequence::VariableGroup_PrimaryKey_Info const & variable_group_primary_key_info)
				{
					if (variable_group_primary_key_info.vg_identifier.IsEqual(WidgetInstanceIdentifier::EQUALITY_CHECK_TYPE__STRING_CODE, the_variable_group.first))		
					{
						if (!variable_group_primary_key_info.column_name.empty())
						{
							if (!first_select)
							{
								sql_generate_output += " AND ";
							}
							first_select = false;
							sql_generate_output += Table_VariableGroupData::ViewNameFromCount(join_count);
							sql_generate_output += ".";
							sql_generate_output += variable_group_primary_key_info.column_name;
							sql_generate_output += " = ";
							sql_generate_output += Table_VariableGroupData::JoinViewNameWithTimeRangesFromCount(join_count - 1);
							sql_generate_output += ".";
							sql_generate_output += variable_group_info_for_primary_keys[0].column_name;
						}
					}
				});
			});

			sql_generate_output += " UNION ALL SELECT ";
			sql_generate_output += Table_VariableGroupData::ViewNameFromCount(join_count);
			sql_generate_output += ".*";
			sql_generate_output += ", ";
			sql_generate_output += Table_VariableGroupData::JoinViewNameWithTimeRangesFromCount(join_count - 1);
			sql_generate_output += ".*";
			sql_generate_output += " FROM ";
			sql_generate_output += temp_dot;
			sql_generate_output += Table_VariableGroupData::JoinViewNameWithTimeRangesFromCount(join_count - 1);
			sql_generate_output += " ";
			sql_generate_output += Table_VariableGroupData::JoinViewNameWithTimeRangesFromCount(join_count - 1);
			sql_generate_output += " LEFT OUTER JOIN ";
			sql_generate_output += temp_dot;
			sql_generate_output += Table_VariableGroupData::ViewNameFromCount(join_count);
			sql_generate_output += " ";
			sql_generate_output += Table_VariableGroupData::ViewNameFromCount(join_count);

			sql_generate_output += " ON ";

			first_select = true;
			std::for_each(sequence.primary_key_sequence_info.cbegin(), sequence.primary_key_sequence_info.cend(), [&sql_generate_output, &first_select, &join_count, &the_variable_group, &failed](PrimaryKeySequence::PrimaryKeySequenceEntry const & total_primary_key_sequence_entry)
			{
				std::vector<PrimaryKeySequence::VariableGroup_PrimaryKey_Info> const & variable_group_info_for_primary_keys = total_primary_key_sequence_entry.variable_group_info_for_primary_keys;
				std::for_each(variable_group_info_for_primary_keys.cbegin(), variable_group_info_for_primary_keys.cend(), [&sql_generate_output, &variable_group_info_for_primary_keys, &first_select, &join_count, &the_variable_group, &failed](PrimaryKeySequence::VariableGroup_PrimaryKey_Info const & variable_group_primary_key_info)
				{
					if (variable_group_primary_key_info.vg_identifier.IsEqual(WidgetInstanceIdentifier::EQUALITY_CHECK_TYPE__STRING_CODE, the_variable_group.first))		
					{
						if (!variable_group_primary_key_info.column_name.empty())
						{
							if (!first_select)
							{
								sql_generate_output += " AND ";
							}
							first_select = false;
							sql_generate_output += Table_VariableGroupData::ViewNameFromCount(join_count);
							sql_generate_output += ".";
							sql_generate_output += variable_group_primary_key_info.column_name;
							sql_generate_output += " = ";
							sql_generate_output += Table_VariableGroupData::JoinViewNameWithTimeRangesFromCount(join_count - 1);
							sql_generate_output += ".";
							sql_generate_output += variable_group_info_for_primary_keys[0].column_name;
						}
					}
				});
			});

			sql_generate_output += " WHERE ";

			first_select = true;
			std::for_each(sequence.primary_key_sequence_info.cbegin(), sequence.primary_key_sequence_info.cend(), [&sql_generate_output, &first_select, &join_count, &the_variable_group, &failed](PrimaryKeySequence::PrimaryKeySequenceEntry const & total_primary_key_sequence_entry)
			{
				std::vector<PrimaryKeySequence::VariableGroup_PrimaryKey_Info> const & variable_group_info_for_primary_keys = total_primary_key_sequence_entry.variable_group_info_for_primary_keys;
				std::for_each(variable_group_info_for_primary_keys.cbegin(), variable_group_info_for_primary_keys.cend(), [&sql_generate_output, &variable_group_info_for_primary_keys, &first_select, &join_count, &the_variable_group, &failed](PrimaryKeySequence::VariableGroup_PrimaryKey_Info const & variable_group_primary_key_info)
				{
					if (variable_group_primary_key_info.vg_identifier.IsEqual(WidgetInstanceIdentifier::EQUALITY_CHECK_TYPE__STRING_CODE, the_variable_group.first))		
					{
						if (!variable_group_primary_key_info.column_name.empty())
						{
							if (!first_select)
							{
								sql_generate_output += " OR ";
							}
							else
							{
								//sql_generate_output += "(";
							}
							first_select = false;
							sql_generate_output += Table_VariableGroupData::ViewNameFromCount(join_count);
							sql_generate_output += ".";
							sql_generate_output += variable_group_primary_key_info.column_name;
							sql_generate_output += " IS NULL";
						}
					}
				});
			});
			if (!first_select)
			{
				//sql_generate_output += ")";
			}

		}
	
		sqlite3_stmt * stmt = NULL;
		sqlite3_prepare_v2(db, sql_generate_output.c_str(), sql_generate_output.size() + 1, &stmt, NULL);
		if (stmt == NULL)
		{
			// Todo: Error message
			failed = true;
			return;
		}

		int step_result = 0;
		if ((step_result = sqlite3_step(stmt)) != SQLITE_DONE)
		{
			// Todo: Error message
			failed = true;
			return; // from lambda
		}

		// Now, merge time ranges and place into a new view
		std::string sql_select_output;
		sql_select_output += "SELECT ";
		sql_select_output += Table_VariableGroupData::JoinViewNameFromCount(join_count);
		sql_select_output += ".* FROM ";
		sql_select_output += temp_dot;
		sql_select_output += Table_VariableGroupData::JoinViewNameFromCount(join_count);
		sql_select_output += " ";
		sql_select_output += Table_VariableGroupData::JoinViewNameFromCount(join_count);

		sqlite3_stmt * stmt_select_output = NULL;
		sqlite3_prepare_v2(db, sql_select_output.c_str(), sql_select_output.size() + 1, &stmt_select_output, NULL);
		if (stmt_select_output == NULL)
		{
			// Todo: Error message
			failed = true;
			return; // from lambda
		}

		std::string join_table_with_time_ranges_name = Table_VariableGroupData::JoinViewNameWithTimeRangesFromCount(join_count);

		// Create the table to store the same data, but with merged time ranges
		std::string sql_create_timerange_table;
		sql_create_timerange_table += "CREATE TABLE ";
		sql_create_timerange_table += join_table_with_time_ranges_name;
		sql_create_timerange_table += " AS SELECT ";
		sql_create_timerange_table += Table_VariableGroupData::JoinViewNameFromCount(join_count);
		sql_create_timerange_table += ".*, ";
		sql_create_timerange_table += "DATETIME_START_NEWGENE_INTERNAL_";
		sql_create_timerange_table += join_count_as_text;
		sql_create_timerange_table += ", ";
		sql_create_timerange_table += "DATETIME_END_NEWGENE_INTERNAL_";
		sql_create_timerange_table += join_count_as_text;
		sql_create_timerange_table += " ";
		sql_create_timerange_table += " FROM ";
		sql_create_timerange_table += temp_dot;
		sql_create_timerange_table += Table_VariableGroupData::JoinViewNameFromCount(join_count);
		sql_create_timerange_table += " ";
		sql_create_timerange_table += Table_VariableGroupData::JoinViewNameFromCount(join_count);
		sql_create_timerange_table += ", 1 AS ";
		sql_create_timerange_table += "DATETIME_START_NEWGENE_INTERNAL_";
		sql_create_timerange_table += join_count_as_text;
		sql_create_timerange_table += ", 1 AS ";
		sql_create_timerange_table += "DATETIME_END_NEWGENE_INTERNAL_";
		sql_create_timerange_table += join_count_as_text;
		sql_create_timerange_table += " WHERE 0"; // just create an empty table whose column types match, by default
		sqlite3_stmt * stmt_create_timerange_table = NULL;
		sqlite3_prepare_v2(db, sql_create_timerange_table.c_str(), sql_create_timerange_table.size() + 1, &stmt_create_timerange_table, NULL);
		if (stmt_create_timerange_table == NULL)
		{
			// Todo: Error message
			failed = true;
			return; // from lambda
		}
		int step_result_create_table = 0;
		if ((step_result_create_table = sqlite3_step(stmt_create_timerange_table)) != SQLITE_DONE)
		{
			// Todo: Error message
			failed = true;
			return; // from lambda
		}

		long number_input_rows = 0;
		int step_result_select_output = 0;
		while ((step_result_select_output = sqlite3_step(stmt_select_output)) == SQLITE_ROW)
		{

			if (failed)
			{
				// attempt more rows; do not exit
			}

			std::string sql_columns;
			std::string sql_values;

			// The rows that are returned from this query
			// are the union of all rows from all views up to
			// and including the current
			// Work from the outside in, since that's how the tables were joined
			int overall_column_number = 0;
			std::int64_t datetime_start_previous = 0;
			std::int64_t datetime_end_previous = 0;
			std::int64_t datetime_start_current = 0;
			std::int64_t datetime_end_current = 0;
			for (int j=join_count - 1; j>=0; --j)
			{
				if (failed)
				{
					// move on to the next row
					break;
				}
				ColumnsInViews::ColumnsInView & columns_in_view = columnsInViews.columns_in_views[j];
				std::vector<ColumnsInViews::ColumnsInView::ColumnInView> & columns_in_view_vector = columns_in_view.columns_in_view;
				std::for_each(columns_in_view_vector.cbegin(), columns_in_view_vector.cend(), [&sql_columns, &sql_values, &overall_column_number, &j, &join_count, &datetime_start_previous, &datetime_end_previous, &datetime_start_current, &datetime_end_current, &stmt_select_output, &failed](ColumnsInViews::ColumnsInView::ColumnInView const & column_in_view)
				{
					if (failed)
					{
						return; // from lambda
					}

					if (j == join_count - 1)
					{
						// This is the new view being joined in to the previous ones
						if (column_in_view.column_type == ColumnsInViews::ColumnsInView::ColumnInView::COLUMN_TYPE__DATETIMESTART)
						{
							datetime_start_current = sqlite3_column_int64(stmt_select_output, overall_column_number);
						}
						if (column_in_view.column_type == ColumnsInViews::ColumnsInView::ColumnInView::COLUMN_TYPE__DATETIMEEND)
						{
							datetime_end_current = sqlite3_column_int64(stmt_select_output, overall_column_number);
						}
					}

					if (overall_column_number > 0)
					{
						sql_columns += ", ";
						sql_values += ", ";
					}

					sql_columns += column_in_view.column_name;

					int column_data_type = sqlite3_column_type(stmt_select_output, overall_column_number);
					bool data_is_null = false;
					switch (column_data_type)
					{
					case SQLITE_INTEGER:
						{
							std::int64_t data = sqlite3_column_int64(stmt_select_output, overall_column_number);
							sql_values += boost::lexical_cast<std::string>(data);
						}
						break;
					case SQLITE_FLOAT:
						{
							long double data = sqlite3_column_double(stmt_select_output, overall_column_number);
							sql_values += boost::lexical_cast<std::string>(data);
						}
						break;
					case SQLITE_TEXT:
						{
							char const * data = reinterpret_cast<char const *>(sqlite3_column_text(stmt_select_output, overall_column_number));
							sql_values += '\'';
							sql_values += Table_VariableGroupData::EscapeTicks(boost::lexical_cast<std::string>(data));
							sql_values += '\'';
						}
						break;
					case SQLITE_BLOB:
						{
							// Todo: Error message
							failed = true;
							return; // from lambda
						}
						break;
					case SQLITE_NULL:
						{
							data_is_null = true;
							sql_values += "NULL";
						}
						break;
					default:
						{
							// Todo: Error message
							failed = true;
							return; // from lambda
						}
					}
					++overall_column_number;
				});

				if (j != join_count - 1)
				{
					// This is a previous join table.  Read in the two "internal time range" columns.
					std::int64_t datetime_start_internal = sqlite3_column_int64(stmt_select_output, overall_column_number);
					++overall_column_number;
					std::int64_t datetime_end_internal = sqlite3_column_int64(stmt_select_output, overall_column_number);
					++overall_column_number;
					if (j == join_count - 2)
					{
						// This is the immediately preceeding join table.  Save internal datetime values.
						datetime_start_previous = datetime_start_internal;
						datetime_end_previous = datetime_end_internal;
					}
				}
			}

			// Now we need to decompose the current input row
			// into possibly multiple output rows in order to
			// have discrete time ranges

			while (datetime_start_current < datetime_end_current)
			{
				if (failed)
				{
					break;
				}
				std::int64_t datetime_start_new = datetime_start_current;
				std::int64_t datetime_end_new = datetime_end_current;
				if (datetime_start_current < datetime_start_previous)
				{
					datetime_end_new = datetime_start_previous;
					if (datetime_end_new > datetime_end_current)
					{
						datetime_end_new = datetime_end_current;

						// add row here
						failed = AddTimeRangeMergedRow(datetime_start_new, datetime_end_new, overall_column_number, sql_columns, sql_values, join_count_as_text, join_table_with_time_ranges_name);
						if (failed) continue;

						datetime_start_new = datetime_end_new;
						datetime_end_new = datetime_start_previous;

						// add row here
						failed = AddTimeRangeMergedRow(datetime_start_new, datetime_end_new, overall_column_number, sql_columns, sql_values, join_count_as_text, join_table_with_time_ranges_name);
						if (failed) continue;

						datetime_start_new = datetime_start_previous;
						datetime_end_new = datetime_end_previous;

						// add row here
						failed = AddTimeRangeMergedRow(datetime_start_new, datetime_end_new, overall_column_number, sql_columns, sql_values, join_count_as_text, join_table_with_time_ranges_name);
						if (failed) continue;

						datetime_start_current = datetime_end_new;
					}
					else if (datetime_end_current == datetime_start_previous)
					{
						// add row here as-is
						failed = AddTimeRangeMergedRow(datetime_start_new, datetime_end_new, overall_column_number, sql_columns, sql_values, join_count_as_text, join_table_with_time_ranges_name);
						if (failed) continue;

						datetime_start_new = datetime_start_previous;
						datetime_end_new = datetime_end_previous;

						// add row here
						failed = AddTimeRangeMergedRow(datetime_start_new, datetime_end_new, overall_column_number, sql_columns, sql_values, join_count_as_text, join_table_with_time_ranges_name);
						if (failed) continue;

						datetime_start_current = datetime_end_new;
					}
					else if (datetime_end_current < datetime_end_previous)
					{
						datetime_start_new = datetime_start_current;
						datetime_end_new = datetime_start_previous;

						// add row here
						failed = AddTimeRangeMergedRow(datetime_start_new, datetime_end_new, overall_column_number, sql_columns, sql_values, join_count_as_text, join_table_with_time_ranges_name);
						if (failed) continue;

						datetime_start_new = datetime_start_previous;
						datetime_end_new = datetime_end_current;

						// add row here
						failed = AddTimeRangeMergedRow(datetime_start_new, datetime_end_new, overall_column_number, sql_columns, sql_values, join_count_as_text, join_table_with_time_ranges_name);
						if (failed) continue;

						datetime_start_new = datetime_end_current;
						datetime_end_new = datetime_end_previous;

						// add row here
						failed = AddTimeRangeMergedRow(datetime_start_new, datetime_end_new, overall_column_number, sql_columns, sql_values, join_count_as_text, join_table_with_time_ranges_name);
						if (failed) continue;

						datetime_start_current = datetime_end_new;
					}
					else if (datetime_end_current == datetime_end_previous)
					{
						datetime_start_new = datetime_start_current;
						datetime_end_new = datetime_start_previous;

						// add row here
						failed = AddTimeRangeMergedRow(datetime_start_new, datetime_end_new, overall_column_number, sql_columns, sql_values, join_count_as_text, join_table_with_time_ranges_name);
						if (failed) continue;

						datetime_start_new = datetime_start_previous;
						datetime_end_new = datetime_end_current;

						// add row here
						failed = AddTimeRangeMergedRow(datetime_start_new, datetime_end_new, overall_column_number, sql_columns, sql_values, join_count_as_text, join_table_with_time_ranges_name);
						if (failed) continue;

						datetime_start_current = datetime_end_new;
					}
					else if (datetime_end_current > datetime_end_previous)
					{
						datetime_start_new = datetime_start_current;
						datetime_end_new = datetime_start_previous;

						// add row here
						failed = AddTimeRangeMergedRow(datetime_start_new, datetime_end_new, overall_column_number, sql_columns, sql_values, join_count_as_text, join_table_with_time_ranges_name);
						if (failed) continue;

						datetime_start_new = datetime_start_previous;
						datetime_end_new = datetime_end_previous;

						// add row here
						failed = AddTimeRangeMergedRow(datetime_start_new, datetime_end_new, overall_column_number, sql_columns, sql_values, join_count_as_text, join_table_with_time_ranges_name);
						if (failed) continue;

						datetime_start_new = datetime_end_previous;
						datetime_end_new = datetime_end_current;

						// add row here
						failed = AddTimeRangeMergedRow(datetime_start_new, datetime_end_new, overall_column_number, sql_columns, sql_values, join_count_as_text, join_table_with_time_ranges_name);
						if (failed) continue;

						datetime_start_current = datetime_end_new;
					}
				}
				else if (datetime_start_current == datetime_start_previous)
				{
					if (datetime_end_current < datetime_end_previous)
					{
						datetime_end_new = datetime_end_current;

						// add row here
						failed = AddTimeRangeMergedRow(datetime_start_new, datetime_end_new, overall_column_number, sql_columns, sql_values, join_count_as_text, join_table_with_time_ranges_name);
						if (failed) continue;

						datetime_start_current = datetime_end_new;
					}
					else if (datetime_end_current >= datetime_end_previous)
					{
						datetime_end_new = datetime_end_previous;

						// add row here
						failed = AddTimeRangeMergedRow(datetime_start_new, datetime_end_new, overall_column_number, sql_columns, sql_values, join_count_as_text, join_table_with_time_ranges_name);
						if (failed) continue;

						datetime_start_current = datetime_end_new;
					}
				}
				else if (datetime_start_current > datetime_start_previous)
				{
					if (datetime_start_current < datetime_end_previous)
					{
						datetime_end_new = datetime_end_previous;

						// add row here
						failed = AddTimeRangeMergedRow(datetime_start_new, datetime_end_new, overall_column_number, sql_columns, sql_values, join_count_as_text, join_table_with_time_ranges_name);
						if (failed) continue;

						datetime_start_current = datetime_end_new;
					}
					else if (datetime_start_current == datetime_end_previous)
					{
						datetime_end_new = datetime_end_current;

						// add row here
						failed = AddTimeRangeMergedRow(datetime_start_new, datetime_end_new, overall_column_number, sql_columns, sql_values, join_count_as_text, join_table_with_time_ranges_name);
						if (failed) continue;

						datetime_start_current = datetime_end_new;
					}
					else if (datetime_start_current > datetime_end_previous)
					{
						// add two rows

						datetime_start_new = datetime_end_previous;
						datetime_end_new = datetime_start_current;

						// add row here
						failed = AddTimeRangeMergedRow(datetime_start_new, datetime_end_new, overall_column_number, sql_columns, sql_values, join_count_as_text, join_table_with_time_ranges_name);
						if (failed) continue;

						datetime_start_new = datetime_start_current;
						datetime_end_new = datetime_end_current;

						// add row here
						failed = AddTimeRangeMergedRow(datetime_start_new, datetime_end_new, overall_column_number, sql_columns, sql_values, join_count_as_text, join_table_with_time_ranges_name);
						if (failed) continue;

						datetime_start_current = datetime_end_new;
					}
				}
			}

			if (datetime_end_current < datetime_end_previous)
			{
				// add row
				std::int64_t datetime_start_new = datetime_end_current;
				std::int64_t datetime_end_new = datetime_end_previous;
				failed = AddTimeRangeMergedRow(datetime_start_new, datetime_end_new, overall_column_number, sql_columns, sql_values, join_count_as_text, join_table_with_time_ranges_name);
				if (failed) continue;
			}

			++number_input_rows;

			if (number_input_rows >= 1000)
			{
				// For development - limit to 1000 rows
				break;
			}
		}

	});

	if (failed)
	{
		// Todo: Error message
		return;
	}

	std::string sql_generate_output;
	sql_generate_output += "SELECT ";
	sql_generate_output += Table_VariableGroupData::JoinViewNameWithTimeRangesFromCount(variable_groups_vector.size());
	sql_generate_output += ".* FROM ";
	sql_generate_output += temp_dot;
	sql_generate_output += Table_VariableGroupData::JoinViewNameWithTimeRangesFromCount(variable_groups_vector.size());
	sql_generate_output += " ";
	sql_generate_output += Table_VariableGroupData::JoinViewNameWithTimeRangesFromCount(variable_groups_vector.size());

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
		break;
	}

	join_count = variable_groups_vector.size();
	std::for_each(variable_groups_vector.crbegin(), variable_groups_vector.crend(), [this, &db, &temp_dot, &join_count, &input_model, &failed](std::pair<WidgetInstanceIdentifier, WidgetInstanceIdentifiers> const & the_variable_group)
	{
		if (failed)
		{
			return; // from lambda
		}

		std::string sql_generate_output;
		sql_generate_output += "DROP VIEW ";
		sql_generate_output += temp_dot;
		sql_generate_output += Table_VariableGroupData::JoinViewNameFromCount(join_count);

		sqlite3_stmt * stmt = NULL;
		sqlite3_prepare_v2(db, sql_generate_output.c_str(), sql_generate_output.size() + 1, &stmt, NULL);
		if (stmt == NULL)
		{
			// Todo: Error message
			failed = true;
			return;
		}

		int step_result = 0;
		if ((step_result = sqlite3_step(stmt)) != SQLITE_DONE)
		{
			// Todo: Error message
			failed = true;
			return; // from lambda
		}

		--join_count;
	});

	if (failed)
	{
		// Todo: Error message
		return;
	}

	view_count = variable_groups_vector.size();
	std::for_each(variable_groups_vector.crbegin(), variable_groups_vector.crend(), [this, &db, &temp_dot, &view_count, &input_model, &failed](std::pair<WidgetInstanceIdentifier, WidgetInstanceIdentifiers> const & the_variable_group)
	{
		if (failed)
		{
			return; // from lambda
		}

		std::string sql_generate_output;
		sql_generate_output += "DROP VIEW ";
		sql_generate_output += temp_dot;
		sql_generate_output += Table_VariableGroupData::ViewNameFromCount(view_count);

		sqlite3_stmt * stmt = NULL;
		sqlite3_prepare_v2(db, sql_generate_output.c_str(), sql_generate_output.size() + 1, &stmt, NULL);
		if (stmt == NULL)
		{
			// Todo: Error message
			failed = true;
			return;
		}

		int step_result = 0;
		if ((step_result = sqlite3_step(stmt)) != SQLITE_DONE)
		{
			// Todo: Error message
			failed = true;
			return; // from lambda
		}

		--view_count;
	});

	if (failed)
	{
		// Todo: Error message
		return;
	}

}

bool OutputModel::AddTimeRangeMergedRow(std::int64_t const datetime_start_new, std::int64_t const datetime_end_new, int & overall_column_number, std::string & sql_columns, std::string & sql_values, std::string & join_count_as_text, std::string const & join_table_with_time_ranges_name)
{
	// Add two more "internal datetime" columns here,
	// that contain the actual time range for this row
	// (a merge of all views so far).
	if (overall_column_number > 0)
	{
		sql_columns += ", ";
		sql_values += ", ";
	}
	sql_columns += "DATETIME_START_NEWGENE_INTERNAL_";
	sql_columns += join_count_as_text;
	sql_values += boost::lexical_cast<std::string>(datetime_start_new);
	++overall_column_number;
	sql_columns += ", ";
	sql_values += ", ";
	sql_columns += "DATETIME_END_NEWGENE_INTERNAL_";
	sql_columns += join_count_as_text;
	sql_values += boost::lexical_cast<std::string>(datetime_end_new);
	++overall_column_number;
	std::string sql_insert_time_range_row;
	sql_insert_time_range_row += "INSERT INTO ";
	sql_insert_time_range_row += join_table_with_time_ranges_name;
	sql_insert_time_range_row += " (";
	sql_insert_time_range_row += sql_columns;
	sql_insert_time_range_row += ") VALUES (";
	sql_insert_time_range_row += sql_values;
	sql_insert_time_range_row += ")";
	sqlite3_stmt * stmt_insert_new_row = NULL;
	sqlite3_prepare_v2(db, sql_insert_time_range_row.c_str(), sql_insert_time_range_row.size() + 1, &stmt_insert_new_row, NULL);
	if (stmt_insert_new_row == NULL)
	{
		return false;
	}
	int step_result_insert_new_row = 0;
	if ((step_result_insert_new_row = sqlite3_step(stmt_insert_new_row)) != SQLITE_DONE)
	{
		// Move on to the next row
		return false;
	}
	return true;
}
