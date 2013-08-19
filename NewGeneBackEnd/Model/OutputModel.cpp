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

OutputModel::OutputGenerator::OutputGenerator(OutputModel & model_)
{
	model = &model_;
}


void OutputModel::OutputGenerator::GenerateOutput(DataChangeMessage & change_response)
{

	InputModel & input_model = model->getInputModel();

	Prepare();

	if (failed)
	{
		// failed
		return;
	}

	ObtainColumnInfoForRawDataTables();

	if (failed)
	{
		// failed
		return;
	}

	LoopThroughPrimaryVariableGroups();

	if (failed)
	{
		// failed
		return;
	}

	MergeHighLevelGroupResults();

	if (failed)
	{
		// failed
		return;
	}

}

void OutputModel::OutputGenerator::MergeHighLevelGroupResults()
{

}

void OutputModel::OutputGenerator::LoopThroughPrimaryVariableGroups()
{

	std::for_each(primary_variable_groups_column_info.cbegin(), primary_variable_groups_column_info.cend(), [this](ColumnsInTempView const & primary_variable_group_raw_data_columns)
	{
		if (failed)
		{
			return;
		}
		primary_variable_group_column_sets.push_back(SqlAndColumnSets());
		SqlAndColumnSets & primary_group_column_sets = primary_variable_group_column_sets.back();
		ConstructFullOutputForSinglePrimaryGroup(primary_variable_group_raw_data_columns, primary_group_column_sets);
	});

}

void OutputModel::OutputGenerator::ConstructFullOutputForSinglePrimaryGroup(ColumnsInTempView const & primary_variable_group_raw_data_columns, SqlAndColumnSets & sql_and_column_sets)
{

	SqlAndColumnSet x_table_result = CreateInitialPrimaryXTable(primary_variable_group_raw_data_columns);
	sql_and_column_sets.push_back(x_table_result);

	if (failed)
	{
		return;
	}

	SqlAndColumnSet xr_table_result = CreateInitialPrimaryXRTable(primary_variable_group_raw_data_columns);
	sql_and_column_sets.push_back(xr_table_result);

	if (failed)
	{
		return;
	}

	for (int current_multiplicity = 2; current_multiplicity <= highest_multiplicity_primary_uoa; ++current_multiplicity)
	{

		SqlAndColumnSet x_table_result = CreatePrimaryXTable(primary_variable_group_raw_data_columns, current_multiplicity);
		sql_and_column_sets.push_back(x_table_result);

		if (failed)
		{
			return;
		}

		SqlAndColumnSet xr_table_result = CreatePrimaryXRTable(primary_variable_group_raw_data_columns, current_multiplicity);
		sql_and_column_sets.push_back(xr_table_result);

		if (failed)
		{
			return;
		}

	}

}

OutputModel::OutputGenerator::SqlAndColumnSet OutputModel::OutputGenerator::CreateInitialPrimaryXTable(ColumnsInTempView const & primary_variable_group_raw_data_columns)
{

}

OutputModel::OutputGenerator::SqlAndColumnSet OutputModel::OutputGenerator::CreateInitialPrimaryXRTable(ColumnsInTempView const & primary_variable_group_raw_data_columns)
{

}

OutputModel::OutputGenerator::SqlAndColumnSet OutputModel::OutputGenerator::CreatePrimaryXTable(ColumnsInTempView const & primary_variable_group_raw_data_columns, int const current_multiplicity)
{

}

OutputModel::OutputGenerator::SqlAndColumnSet OutputModel::OutputGenerator::CreatePrimaryXRTable(ColumnsInTempView const & primary_variable_group_raw_data_columns, int const current_multiplicity)
{

}

void OutputModel::OutputGenerator::Prepare()
{

	failed = false;

	//temp_dot = "temp.";
	temp_dot = "";

	input_model = &model->getInputModel();

	db = input_model->getDb();

	Table_VARIABLES_SELECTED::UOA_To_Variables_Map the_map_ = model->t_variables_selected_identifiers.GetSelectedVariablesByUOA(model->getDb(), model, input_model);
	the_map = &the_map_;

	PopulateUOAs();

	PopulateDMUCounts();

	ValidateUOAs();

	DetermineChildMultiplicitiesGreaterThanOne();

	PopulateVariableGroups();

	PopulatePrimaryKeySequenceInfo();

}

void OutputModel::OutputGenerator::ObtainColumnInfoForRawDataTables()
{

	int primary_view_count = 0;
	std::for_each(primary_variable_groups_vector.cbegin(), primary_variable_groups_vector.cend(), [this, &primary_view_count](std::pair<WidgetInstanceIdentifier, WidgetInstanceIdentifiers> const & the_primary_variable_group)
	{
		PopulateColumnsFromRawDataTable(the_primary_variable_group, primary_view_count, primary_variable_groups_column_info, true);
	});

	int secondary_view_count = 0;
	std::for_each(secondary_variable_groups_vector.cbegin(), secondary_variable_groups_vector.cend(), [this, &secondary_view_count](std::pair<WidgetInstanceIdentifier, WidgetInstanceIdentifiers> const & the_secondary_variable_group)
	{
		PopulateColumnsFromRawDataTable(the_secondary_variable_group, secondary_view_count, secondary_variable_groups_column_info, false);
	});

}

void OutputModel::OutputGenerator::PopulateColumnsFromRawDataTable(std::pair<WidgetInstanceIdentifier, WidgetInstanceIdentifiers> const & the_variable_group, int view_count, std::vector<ColumnsInTempView> & variable_groups_column_info, bool const & is_primary)
{

	// ************************************************************************************************ //
	// the_variable_group:
	// A pair: VG identifier -> Variables in this group selected by the user.
	// ************************************************************************************************ //

	// Convert data into a far more useful form for construction of K-adic output

	if (failed)
	{
		return;
	}

	std::string vg_code = *the_variable_group.first.code;
	std::string vg_data_table_name = Table_VariableGroupData::TableNameFromVGCode(vg_code);

	variable_groups_column_info.push_back(ColumnsInTempView());
	ColumnsInTempView & columns_in_variable_group_view = variable_groups_column_info.back();

	columns_in_variable_group_view.view_number = view_count;
	std::string view_name;
	view_name = *the_variable_group.first.code;
	columns_in_variable_group_view.view_name_no_uuid = view_name;
	view_name += "_";
	view_name += newUUID(true);
	columns_in_variable_group_view.view_name = view_name;

	columns_in_variable_group_view.original_table_name = vg_data_table_name;

	WidgetInstanceIdentifiers & variables_in_group = input_model->t_vgp_setmembers.getIdentifiers(*the_variable_group.first.uuid);

	std::set<WidgetInstanceIdentifier> variables_in_group_sorted;
	std::for_each(variables_in_group.cbegin(), variables_in_group.cend(), [&variables_in_group_sorted](WidgetInstanceIdentifier const & variable_group_set_member)
	{
		variables_in_group_sorted.insert(variable_group_set_member);
	});

	WidgetInstanceIdentifiers & datetime_columns = input_model->t_vgp_data_metadata__datetime_columns.getIdentifiers(vg_data_table_name);
	if (datetime_columns.size() > 0 && datetime_columns.size() != 2)
	{
		failed = true;
		return;
	}

	columns_in_variable_group_view.has_no_datetime_columns = false;
	if (datetime_columns.size() == 0)
	{
		columns_in_variable_group_view.has_no_datetime_columns = true;
	}

	std::for_each(variables_in_group_sorted.cbegin(), variables_in_group_sorted.cend(), [this, &is_primary, &columns_in_variable_group_view, &datetime_columns, &the_variable_group](WidgetInstanceIdentifier const & variable_group_set_member)
	{
		columns_in_variable_group_view.columns_in_view.push_back(ColumnsInTempView::ColumnInTempView());
		ColumnsInTempView::ColumnInTempView & column_in_variable_group_data_table = columns_in_variable_group_view.columns_in_view.back();

		std::string column_name_no_uuid = *variable_group_set_member.code;
		column_in_variable_group_data_table.column_name_no_uuid = column_name_no_uuid;

		std::string column_name = column_name_no_uuid;
		column_name += "_";
		column_name += newUUID(true);

		column_in_variable_group_data_table.column_name = column_name;

		column_in_variable_group_data_table.table_column_name = column_name_no_uuid;

		column_in_variable_group_data_table.variable_group_identifier = the_variable_group.first;

		if (!the_variable_group.first.identifier_parent)
		{
			failed = true;
			return;
		}

		column_in_variable_group_data_table.uoa_associated_with_variable_group_identifier = *the_variable_group.first.identifier_parent;

		if (is_primary)
		{
			column_in_variable_group_data_table.column_type = ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__PRIMARY;
		}
		else
		{
			column_in_variable_group_data_table.column_type = ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__SECONDARY;
		}

		if (datetime_columns.size())
		{
			if (datetime_columns[0].code && variable_group_set_member.code && *datetime_columns[0].code == *variable_group_set_member.code)
			{
				// The current column is the datetime_start column
				column_in_variable_group_data_table.column_type = ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__DATETIMESTART;
			}
			else if (datetime_columns[1].code && variable_group_set_member.code && *datetime_columns[1].code == *variable_group_set_member.code)
			{
				// The current column is the datetime_end column
				column_in_variable_group_data_table.column_type = ColumnsInTempView::ColumnInTempView::COLUMN_TYPE__DATETIMEEND;
			}
		}

		std::for_each(sequence.primary_key_sequence_info.cbegin(), sequence.primary_key_sequence_info.cend(), [&the_variable_group, &column_in_variable_group_data_table](PrimaryKeySequence::PrimaryKeySequenceEntry const & primary_key_entry)
		{
			std::for_each(primary_key_entry.variable_group_info_for_primary_keys.cbegin(), primary_key_entry.variable_group_info_for_primary_keys.cend(), [&the_variable_group, &column_in_variable_group_data_table, &primary_key_entry](PrimaryKeySequence::VariableGroup_PrimaryKey_Info const & current_variable_group_primary_key_entry)
			{
				if (current_variable_group_primary_key_entry.vg_identifier.IsEqual(WidgetInstanceIdentifier::EQUALITY_CHECK_TYPE__STRING_CODE, the_variable_group.first))
				{
					if (!current_variable_group_primary_key_entry.column_name.empty())
					{
						if (boost::iequals(current_variable_group_primary_key_entry.table_column_name, column_in_variable_group_data_table.table_column_name))
						{
							column_in_variable_group_data_table.primary_key_dmu_category_identifier = primary_key_entry.dmu_category;
							column_in_variable_group_data_table.primary_key_index_within_total_kad_for_dmu_category = primary_key_entry.sequence_number_within_dmu_category_spin_control;
							column_in_variable_group_data_table.primary_key_index_within_total_kad_for_all_dmu_categories = primary_key_entry.sequence_number_in_all_primary_keys;
							column_in_variable_group_data_table.primary_key_index_within_uoa_corresponding_to_variable_group_for_dmu_category = current_variable_group_primary_key_entry.sequence_number_within_dmu_category_variable_group_uoa;
							column_in_variable_group_data_table.primary_key_index_within_primary_uoa_for_dmu_category = primary_key_entry.sequence_number_within_dmu_category_primary_uoa;
						}
					}
				}
			});
		});

	});

}

void OutputModel::OutputGenerator::PopulateDMUCounts()
{

	bool first = true;
	std::for_each(UOAs.cbegin(), UOAs.cend(), [this, &first](std::pair<WidgetInstanceIdentifier, Table_UOA_Identifier::DMU_Counts> const & uoa__to__dmu_counts__pair)
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
		std::for_each(current_dmu_counts.cbegin(), current_dmu_counts.cend(), [this, &current_is_bigger, &current_is_smaller, &current_is_same, &current](Table_UOA_Identifier::DMU_Plus_Count const & current_dmu_plus_count)
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

}

void OutputModel::OutputGenerator::ValidateUOAs()
{
	// Check for overlap in UOA's

	// For primary UOA:
	// Make sure that at most 1 DMU has a multiplicity greater than 1,
	// and save the name/index of the DMU category with the highest multiplicity.

	multiplicities_primary_uoa.clear();
	highest_multiplicity_primary_uoa = 0;
	highest_multiplicity_primary_uoa_dmu_string_code.clear();

	// Looking at the first entry in biggest_counts is the same as looking at any other entry
	// in terms of the DMU counts
	std::for_each(biggest_counts[0].second.cbegin(), biggest_counts[0].second.cend(), [this](Table_UOA_Identifier::DMU_Plus_Count const & dmu_category)
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
		WidgetInstanceIdentifier_Int_Pair kad_count_pair = this->model->t_kad_count.getIdentifier(*the_dmu_category.uuid);
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
		multiplicities_primary_uoa.push_back(multiplicity);
		if (multiplicity > highest_multiplicity_primary_uoa)
		{
			highest_multiplicity_primary_uoa = multiplicity;
			highest_multiplicity_primary_uoa_dmu_string_code = *the_dmu_category.code;
		}
	});

	if (failed)
	{
		// Todo: Error message
		return;
	}

	// check that only 1 primary UOA's DMU category has multiplicity > 1 (for now)
	any_primary_dmu_has_multiplicity_greater_than_1 = false;
	which_primary_index_has_multiplicity_greater_than_1 = -1;

	int current_index = 0;
	std::for_each(multiplicities_primary_uoa.cbegin(), multiplicities_primary_uoa.cend(), [this, &current_index](int const & test_multiplicity)
	{
		if (failed)
		{
			return; // from lambda
		}
		if (test_multiplicity > 1)
		{
			if (any_primary_dmu_has_multiplicity_greater_than_1)
			{
				// ********************************************************************************************************************** //
				// A second DMU category's multiplicity is greater than 1 - for now, not allowed.  This can be implemented in the future.
				// ********************************************************************************************************************** //
				failed = true;
				return; // from lambda
			}
			any_primary_dmu_has_multiplicity_greater_than_1 = true;
			which_primary_index_has_multiplicity_greater_than_1 = current_index;
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
	// Make sure that, for each child UOA, the UOA k-value for all DMU categories
	// is either 0, 1, or the corresponding UOA k-value of the primary UOA
	// and that where not 0, it is less than the corresponding UOA k-value of the primary UOA
	// (i.e., has a value of 1)
	// for only 1 DMU category,
	// and this DMU category must match the DMU category with multiplicity greater than 1 (if any)
	// for the primary UOAs
	std::for_each(child_counts.cbegin(), child_counts.cend(), [this](std::pair<WidgetInstanceIdentifier, Table_UOA_Identifier::DMU_Counts> const & uoa__to__dmu_counts__pair)
	{
		if (failed)
		{
			return; // from lambda
		}
		int primary_dmu_categories_for_which_child_has_less = 0;
		Table_UOA_Identifier::DMU_Counts const & current_dmu_counts = uoa__to__dmu_counts__pair.second;
		std::for_each(current_dmu_counts.cbegin(), current_dmu_counts.cend(), [this, &primary_dmu_categories_for_which_child_has_less](Table_UOA_Identifier::DMU_Plus_Count const & current_dmu_plus_count)
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
			std::for_each(biggest_counts[0].second.cbegin(), biggest_counts[0].second.cend(), [this, &current_dmu_plus_count, &primary_dmu_categories_for_which_child_has_less](Table_UOA_Identifier::DMU_Plus_Count const & biggest_dmu_plus_count)
			{
				if (failed)
				{
					return; // from lambda
				}
				if (current_dmu_plus_count.first.IsEqual(WidgetInstanceIdentifier::EQUALITY_CHECK_TYPE__STRING_CODE, biggest_dmu_plus_count.first))
				{
					if (current_dmu_plus_count.second == biggest_dmu_plus_count.second) // biggest_dmu_plus_count.second is the K-value of the unit of analysis, not the K-value chosen by the user in the spin control
					{
						if (highest_multiplicity_primary_uoa > 1)
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
					else if (!boost::iequals(*current_dmu_plus_count.first.code, highest_multiplicity_primary_uoa_dmu_string_code))
					{
						if (highest_multiplicity_primary_uoa > 1)
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
}

void OutputModel::OutputGenerator::PopulateUOAs()
{
	std::for_each(the_map->cbegin(), the_map->cend(), [this](std::pair<WidgetInstanceIdentifier /* UOA identifier */,
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
		Table_UOA_Identifier::DMU_Counts dmu_counts = input_model->t_uoa_category.RetrieveDMUCounts(input_model->getDb(), input_model, *uoa.uuid);
		UOAs.push_back(std::make_pair(uoa, dmu_counts));
	});
}

void OutputModel::OutputGenerator::DetermineChildMultiplicitiesGreaterThanOne()
{
	// For child UOA's:
	// Save the name/index of the DMU category multiplicity greater than one.
	// Currently, this can be only one of the DMU categories.
	std::for_each(child_counts.cbegin(), child_counts.cend(), [this](std::pair<WidgetInstanceIdentifier, Table_UOA_Identifier::DMU_Counts> const & child_uoa__dmu_counts__pair)
	{

		if (failed)
		{
			return; // from lambda
		}

		WidgetInstanceIdentifier uoa_identifier = child_uoa__dmu_counts__pair.first;
		std::for_each(child_uoa__dmu_counts__pair.second.cbegin(), child_uoa__dmu_counts__pair.second.cend(), [this, &uoa_identifier](Table_UOA_Identifier::DMU_Plus_Count const & dmu_category)
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

			WidgetInstanceIdentifier_Int_Pair kad_count_pair = this->model->t_kad_count.getIdentifier(*the_dmu_category_identifier.uuid);
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
}

void OutputModel::OutputGenerator::PopulateVariableGroups()
{
	std::for_each(biggest_counts.cbegin(), biggest_counts.cend(), [this](std::pair<WidgetInstanceIdentifier, Table_UOA_Identifier::DMU_Counts> const & uoa__to__dmu_category_counts)
	{
		// Get all the variable groups corresponding to the primary UOA (identical except possibly for time granularity)
		Table_VARIABLES_SELECTED::VariableGroup_To_VariableSelections_Map const & variable_groups_map_current = (*the_map)[uoa__to__dmu_category_counts.first];
		primary_variable_groups_vector.insert(primary_variable_groups_vector.end(), variable_groups_map_current.cbegin(), variable_groups_map_current.cend());
	});
	std::for_each(child_counts.cbegin(), child_counts.cend(), [this](std::pair<WidgetInstanceIdentifier, Table_UOA_Identifier::DMU_Counts> const & uoa__to__dmu_category_counts)
	{
		Table_VARIABLES_SELECTED::VariableGroup_To_VariableSelections_Map const & variable_groups_map_current = (*the_map)[uoa__to__dmu_category_counts.first];
		secondary_variable_groups_vector.insert(secondary_variable_groups_vector.end(), variable_groups_map_current.cbegin(), variable_groups_map_current.cend());
	});
}

void OutputModel::OutputGenerator::PopulatePrimaryKeySequenceInfo()
{
	int overall_primary_key_sequence_number = 0;
	std::for_each(biggest_counts[0].second.cbegin(), biggest_counts[0].second.cend(), [this, &overall_primary_key_sequence_number](Table_UOA_Identifier::DMU_Plus_Count const & dmu_category)
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
		WidgetInstanceIdentifier_Int_Pair kad_count_pair = this->model->t_kad_count.getIdentifier(*the_dmu_category.uuid);
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
			std::for_each(primary_variable_groups_vector.cbegin(), primary_variable_groups_vector.cend(), [this, &the_dmu_category, &current_dmu_sequence_number, &uoa_count_current_dmu_category, &kad_count_current_dmu_category, &current_primary_key_sequence, &variable_group_info_for_primary_keys](std::pair<WidgetInstanceIdentifier, WidgetInstanceIdentifiers> const & the_variable_group)
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
				WidgetInstanceIdentifiers const & dmu_primary_key_codes_metadata = input_model->t_vgp_data_metadata__primary_keys.getIdentifiers(vg_data_table_name);

				// Todo: To implement global variables (i.e., variables with no primary key),
				// make the necessary changes and then remove the following requirement
				if (dmu_primary_key_codes_metadata.size() == 0)
				{
					// Todo: error message
					failed = true;
					return;
				}

				int current_variable_group_current_primary_key_dmu_category_total_number = 0;
				std::for_each(dmu_primary_key_codes_metadata.cbegin(), dmu_primary_key_codes_metadata.cend(), [this, the_dmu_category, &current_variable_group_current_primary_key_dmu_category_total_number, &current_dmu_sequence_number, &uoa_count_current_dmu_category, &kad_count_current_dmu_category, &current_primary_key_sequence, &variable_group_info_for_primary_keys](WidgetInstanceIdentifier const & current_variable_group_current_dmu_primary_key)
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
					std::for_each(dmu_primary_key_codes_metadata.cbegin(), dmu_primary_key_codes_metadata.cend(), [this, &m, &multiplicity, &current_variable_group_current_primary_key_info, &the_variable_group, &current_variable_group_current_primary_key_dmu_category_total_sequence_number, &the_dmu_category, &current_variable_group_current_primary_key_dmu_category_sequence_number, &current_dmu_sequence_number, &uoa_count_current_dmu_category, &kad_count_current_dmu_category, &current_primary_key_sequence, &variable_group_info_for_primary_keys](WidgetInstanceIdentifier const & current_variable_group_current_dmu_primary_key)
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
								bool found_variable_group_set_member_identifier = input_model->t_vgp_setmembers.getIdentifierFromStringCodeAndParentUUID(*current_variable_group_current_dmu_primary_key.longhand, *the_variable_group.first.uuid, vg_setmember_identifier);
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
		// Get all the variable groups corresponding to the primary UOA (identical except possibly for time granularity)
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
				WidgetInstanceIdentifiers const & dmu_primary_key_codes_metadata = input_model.t_vgp_data_metadata__primary_keys.getIdentifiers(vg_data_table_name);

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
		columnsInView.view_name = Table_VariableGroupData::ViewNameFromCount(view_count);

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
		WidgetInstanceIdentifiers const & dmu_primary_key_codes_metadata = input_model.t_vgp_data_metadata__primary_keys.getIdentifiers(vg_data_table_name);

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





		std::vector<ColumnsInViews::ColumnsInView> & columns_in_temp_views = columnsInView.columns_in_temp_views.columns_in_temp_views_vector;

		char ns_[1024];
		for (int m=1; m<=highest_multiplicity_to_use; ++m)
		{

			std::string sql_tmp_view;
			sql_tmp_view += "CREATE TABLE ";
			sql_tmp_view += temp_dot;
			sql_tmp_view += Table_VariableGroupData::ViewNameFromCountTemp(view_count, m);
			sql_tmp_view += " AS SELECT ";



			std::string ns = itoa(m, ns_, 10);

			columns_in_temp_views.push_back(ColumnsInViews::ColumnsInView());
			ColumnsInViews::ColumnsInView & columns_in_temp_view = columns_in_temp_views.back();
			columns_in_temp_view.view_number = m;
			columns_in_temp_view.view_name = Table_VariableGroupData::ViewNameFromCountTemp(view_count, m);
			std::vector<ColumnsInViews::ColumnsInView::ColumnInView> & columns_in_temp_view_vector = columns_in_temp_view.columns_in_view;



			// Date-time columns

			bool first_select = true;
			if (current_uoa_identifier.time_granularity != 0)
			{
				// The current variable group being joined into the full set of data
				// has a time range granularity associated with it

				// At least one of the primary keys has this variable group active, so use that to get the date-time column names
				// ... just break out of loop once we have it
				std::for_each(sequence.primary_key_sequence_info.cbegin(), sequence.primary_key_sequence_info.cend(), [&sql_tmp_view, &ns, &m, &columns_in_temp_view, &columns_in_temp_view_vector, &vg_data_table_name, &view_count, &first_select, &the_variable_group, &failed](PrimaryKeySequence::PrimaryKeySequenceEntry const & total_primary_key_sequence_entry)
				{
					if (first_select == false)
					{
						// We already have the information we need
						return; // from lambda
					}
					std::vector<PrimaryKeySequence::VariableGroup_PrimaryKey_Info> const & variable_group_info_for_primary_keys = total_primary_key_sequence_entry.variable_group_info_for_primary_keys;
					std::for_each(variable_group_info_for_primary_keys.cbegin(), variable_group_info_for_primary_keys.cend(), [&sql_tmp_view, &ns, &m, &columns_in_temp_view, &columns_in_temp_view_vector, &vg_data_table_name, &view_count, &first_select, &the_variable_group, &failed](PrimaryKeySequence::VariableGroup_PrimaryKey_Info const & variable_group_primary_key_info)
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
								std::string datetime_start_col_name;
								datetime_start_col_name += variable_group_primary_key_info.datetime_row_start_column_name;
								datetime_start_col_name += "__";
								datetime_start_col_name += ns;

								std::string datetime_end_col_name;
								datetime_end_col_name += variable_group_primary_key_info.datetime_row_end_column_name;
								datetime_end_col_name += "__";
								datetime_end_col_name += ns;

								first_select = false;
								sql_tmp_view += vg_data_table_name;
								sql_tmp_view += ".";
								sql_tmp_view += variable_group_primary_key_info.datetime_row_start_table_column_name;
								sql_tmp_view += " AS ";
								sql_tmp_view += datetime_start_col_name;
								sql_tmp_view += ", ";
								sql_tmp_view += vg_data_table_name;
								sql_tmp_view += ".";
								sql_tmp_view += variable_group_primary_key_info.datetime_row_end_table_column_name;
								sql_tmp_view += " AS ";
								sql_tmp_view += datetime_end_col_name;

								columns_in_temp_view_vector.push_back(ColumnsInViews::ColumnsInView::ColumnInView());
								ColumnsInViews::ColumnsInView::ColumnInView & column_in_view_datetime_start = columns_in_temp_view_vector.back();
								column_in_view_datetime_start.column_name = datetime_start_col_name;
								column_in_view_datetime_start.column_name_no_uuid = datetime_start_col_name;
								column_in_view_datetime_start.table_column_name = variable_group_primary_key_info.datetime_row_start_table_column_name;
								column_in_view_datetime_start.variable_group_identifier = variable_group_primary_key_info.vg_identifier;
								column_in_view_datetime_start.uoa_associated_with_variable_group_identifier = *variable_group_primary_key_info.vg_identifier.identifier_parent;
								column_in_view_datetime_start.column_type = ColumnsInViews::ColumnsInView::ColumnInView::COLUMN_TYPE__DATETIMESTART;

								columns_in_temp_view_vector.push_back(ColumnsInViews::ColumnsInView::ColumnInView());
								ColumnsInViews::ColumnsInView::ColumnInView & column_in_view_datetime_end = columns_in_temp_view_vector.back();
								column_in_view_datetime_end.column_name = datetime_end_col_name;
								column_in_view_datetime_end.column_name_no_uuid = datetime_end_col_name;
								column_in_view_datetime_end.table_column_name = variable_group_primary_key_info.datetime_row_end_table_column_name;
								column_in_view_datetime_end.variable_group_identifier = variable_group_primary_key_info.vg_identifier;
								column_in_view_datetime_end.uoa_associated_with_variable_group_identifier = *variable_group_primary_key_info.vg_identifier.identifier_parent;
								column_in_view_datetime_end.column_type = ColumnsInViews::ColumnsInView::ColumnInView::COLUMN_TYPE__DATETIMEEND;

								columns_in_temp_view.datetime_start_column_name = datetime_start_col_name;
								columns_in_temp_view.datetime_end_column_name = datetime_end_col_name;
							}
						}
					});
				});
			}

			std::for_each(sequence.primary_key_sequence_info.cbegin(), sequence.primary_key_sequence_info.cend(), [&sql_tmp_view, &ns, &m, &columns_in_temp_view_vector, &columns_in_temp_view, &vg_data_table_name, &view_count, &first_select, &the_variable_group, &failed](PrimaryKeySequence::PrimaryKeySequenceEntry const & total_primary_key_sequence_entry)
			{
				std::vector<PrimaryKeySequence::VariableGroup_PrimaryKey_Info> const & variable_group_info_for_primary_keys = total_primary_key_sequence_entry.variable_group_info_for_primary_keys;
				std::for_each(variable_group_info_for_primary_keys.cbegin(), variable_group_info_for_primary_keys.cend(), [&sql_tmp_view, &ns, &m, &columns_in_temp_view_vector, &columns_in_temp_view, &vg_data_table_name, &view_count, &first_select, &the_variable_group, &total_primary_key_sequence_entry, &failed](PrimaryKeySequence::VariableGroup_PrimaryKey_Info const & variable_group_primary_key_info)
				{
					if (variable_group_primary_key_info.vg_identifier.IsEqual(WidgetInstanceIdentifier::EQUALITY_CHECK_TYPE__STRING_CODE, the_variable_group.first))		
					{
						if (!variable_group_primary_key_info.column_name.empty())
						{

							if (variable_group_primary_key_info.total_multiplicity > 1)
							{
								if (variable_group_primary_key_info.current_multiplicity != m)
								{
									// Only one primary key field per DMU in the UOA here
									columns_in_temp_view.primary_key_names_in_sub_view.push_back("");
									columns_in_temp_view.primary_key_names_in_sub_view_no_uuid.push_back("");
									return; // from lambda
								}
							}

							std::string name_column_temp_view;
							name_column_temp_view += variable_group_primary_key_info.column_name;
							name_column_temp_view += "_";
							name_column_temp_view += ns;

							if (!first_select)
							{
								sql_tmp_view += ", ";
							}
							first_select = false;
							sql_tmp_view += vg_data_table_name;
							sql_tmp_view += ".";
							sql_tmp_view += variable_group_primary_key_info.table_column_name;
							sql_tmp_view += " AS ";
							sql_tmp_view += name_column_temp_view;

							columns_in_temp_view_vector.push_back(ColumnsInViews::ColumnsInView::ColumnInView());
							ColumnsInViews::ColumnsInView::ColumnInView & column_in_view = columns_in_temp_view_vector.back();
							column_in_view.column_name = name_column_temp_view;
							column_in_view.column_name_no_uuid = name_column_temp_view;
							column_in_view.table_column_name = variable_group_primary_key_info.table_column_name;
							column_in_view.variable_group_identifier = variable_group_primary_key_info.vg_identifier;
							column_in_view.uoa_associated_with_variable_group_identifier = *variable_group_primary_key_info.vg_identifier.identifier_parent;
							column_in_view.column_type = ColumnsInViews::ColumnsInView::ColumnInView::COLUMN_TYPE__PRIMARY;
							column_in_view.primary_key_dmu_category_identifier = total_primary_key_sequence_entry.dmu_category;
							column_in_view.primary_key_index_within_total_kad_for_dmu_category = total_primary_key_sequence_entry.sequence_number_within_dmu_category_spin_control;
							column_in_view.primary_key_index_within_total_kad_for_all_dmu_categories = total_primary_key_sequence_entry.sequence_number_in_all_primary_keys;
							column_in_view.primary_key_index_within_uoa_corresponding_to_variable_group_for_dmu_category = variable_group_primary_key_info.sequence_number_within_dmu_category_variable_group_uoa;
							column_in_view.primary_key_index_within_primary_uoa_for_dmu_category = total_primary_key_sequence_entry.sequence_number_within_dmu_category_primary_uoa;

							columns_in_temp_view.primary_key_names_in_sub_view.push_back(name_column_temp_view);
							columns_in_temp_view.primary_key_names_in_sub_view_no_uuid.push_back(name_column_temp_view);

						}
					}
				});
			});

			if (failed)
			{
				// Todo: Error message
				return;
			}

			// ************************************************* //
			// Display all variables selected by user
			// ************************************************* //
			int secondary_key_index = 0;
			std::for_each(variables_selected_in_this_group.cbegin(), variables_selected_in_this_group.cend(), [this, &columns_in_temp_view, &secondary_key_index, &columns_in_temp_view_vector, &vg_data_table_name, &ns, &m, &sql_tmp_view, &the_variable_group, &highest_multiplicity_to_use, &current_uoa_identifier, &first_select, &this_variable_group__primary_key_names__uuid_stripped, &this_variable_group__secondary_key_names, &child_uoas__which_multiplicity_is_greater_than_1, &failed](WidgetInstanceIdentifier const & variable_selected_in_this_group)
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

				this_variable_group_secondary_key_name += "_tmp";
				this_variable_group_secondary_key_name += ns;

				std::string saved_secondary_key_name_no_uuid = this_variable_group_secondary_key_name;

				this_variable_group_secondary_key_name += "_";
				this_variable_group_secondary_key_name += newUUID(true);

				if (!first_select)
				{
					sql_tmp_view += ", ";
				}
				first_select = false;

				sql_tmp_view += vg_data_table_name;
				sql_tmp_view += ".";
				sql_tmp_view += *variable_selected_in_this_group.code;
				sql_tmp_view += " AS ";
				sql_tmp_view += this_variable_group_secondary_key_name;

				columns_in_temp_view_vector.push_back(ColumnsInViews::ColumnsInView::ColumnInView());
				ColumnsInViews::ColumnsInView::ColumnInView & column_in_view = columns_in_temp_view_vector.back();
				column_in_view.column_name = this_variable_group_secondary_key_name;
				column_in_view.column_name_no_uuid = saved_secondary_key_name_no_uuid;
				column_in_view.table_column_name = *variable_selected_in_this_group.code;
				column_in_view.variable_group_identifier = the_variable_group.first;
				column_in_view.uoa_associated_with_variable_group_identifier = *the_variable_group.first.identifier_parent;
				column_in_view.column_type = ColumnsInViews::ColumnsInView::ColumnInView::COLUMN_TYPE__SECONDARY;

				columns_in_temp_view.secondary_key_names_in_sub_view.push_back(this_variable_group_secondary_key_name);
				columns_in_temp_view.secondary_key_names_in_sub_view_no_uuid.push_back(saved_secondary_key_name_no_uuid);
			
				++secondary_key_index;

			});

			if (failed)
			{
				// Todo: Error message
				return;
			}

			if (m > 1)
			{
				sql_tmp_view += ", ";
				sql_tmp_view += Table_VariableGroupData::ViewNameFromCountTempTimeRanged(view_count, m - 1);
				sql_tmp_view += ".*";
			}

			sql_tmp_view += " FROM ";
			sql_tmp_view += vg_data_table_name;
			sql_tmp_view += " ";
			sql_tmp_view += vg_data_table_name;
			if (m > 1)
			{
				sql_tmp_view += " INNER JOIN ";
				sql_tmp_view += temp_dot;
				sql_tmp_view += Table_VariableGroupData::ViewNameFromCountTempTimeRanged(view_count, m - 1);
				sql_tmp_view += " ";
				sql_tmp_view += Table_VariableGroupData::ViewNameFromCountTempTimeRanged(view_count, m - 1);

				bool on_has_been_written = false;

				ColumnsInViews::ColumnsInView & columns_in_temp_view_ = columnsInView.columns_in_temp_views.columns_in_temp_views_vector[m-2];
				
				bool and_required = false;
				int the_primary_key_sequence_number = 0;
				std::for_each(sequence.primary_key_sequence_info.cbegin(), sequence.primary_key_sequence_info.cend(), [&sql_tmp_view, &on_has_been_written, &m, &the_primary_key_sequence_number, &and_required, &columns_in_temp_view_, &vg_data_table_name, &view_count, &the_variable_group, &failed](PrimaryKeySequence::PrimaryKeySequenceEntry const & total_primary_key_sequence_entry)
				{
					std::vector<PrimaryKeySequence::VariableGroup_PrimaryKey_Info> const & variable_group_info_for_primary_keys = total_primary_key_sequence_entry.variable_group_info_for_primary_keys;
					std::for_each(variable_group_info_for_primary_keys.cbegin(), variable_group_info_for_primary_keys.cend(), [&sql_tmp_view, &on_has_been_written, &m, &the_primary_key_sequence_number, &and_required, &columns_in_temp_view_, &vg_data_table_name, &view_count, &the_variable_group, &failed](PrimaryKeySequence::VariableGroup_PrimaryKey_Info const & variable_group_primary_key_info)
					{
						if (variable_group_primary_key_info.vg_identifier.IsEqual(WidgetInstanceIdentifier::EQUALITY_CHECK_TYPE__STRING_CODE, the_variable_group.first))		
						{
							if (!variable_group_primary_key_info.column_name.empty())
							{
								if (variable_group_primary_key_info.total_multiplicity == 1)
								{
									std::string vg_data_column_name = variable_group_primary_key_info.table_column_name;

									if (!on_has_been_written)
									{
										sql_tmp_view += " ON ";
									}
									on_has_been_written = true;

									if (and_required)
									{
										sql_tmp_view += " AND ";
									}
									and_required = true;

									sql_tmp_view += vg_data_table_name;
									sql_tmp_view += ".";
									sql_tmp_view += vg_data_column_name;
									sql_tmp_view += " = ";
									sql_tmp_view += Table_VariableGroupData::ViewNameFromCountTempTimeRanged(view_count, m - 1);
									sql_tmp_view += ".";
									sql_tmp_view += columns_in_temp_view_.primary_key_names_in_sub_view[the_primary_key_sequence_number];
								}
							}
						}
					});
					++the_primary_key_sequence_number;
				});
			}

			sqlite3_stmt * stmt_tmp = NULL;
			sqlite3_prepare_v2(db, sql_tmp_view.c_str(), sql_tmp_view.size() + 1, &stmt_tmp, NULL);
			if (stmt_tmp == NULL)
			{
				// Todo: Error message
				std::string error_msg = sqlite3_errmsg(db);
				failed = true;
				return;
			}

			int step_result_tmp = 0;
			if ((step_result_tmp = sqlite3_step(stmt_tmp)) != SQLITE_DONE)
			{
				// Todo: Error message
				std::string error_msg = sqlite3_errmsg(db);
				failed = true;
				return; // from lambda
			}


			if (current_uoa_identifier.time_granularity == 0)
			{
				// Time granularity is 0.  Special case: set time range columns to 0, 0
				std::string datetime_start_col_name;
				datetime_start_col_name += "DATETIME_ROW_START";
				datetime_start_col_name += "__";
				datetime_start_col_name += ns;

				std::string datetime_end_col_name;
				datetime_end_col_name += "DATETIME_ROW_START";
				datetime_end_col_name += "__";
				datetime_end_col_name += ns;

				std::string sql_add_datetime_start;
				sql_add_datetime_start += "ALTER TABLE ";
				sql_add_datetime_start += Table_VariableGroupData::ViewNameFromCountTemp(view_count, m);
				sql_add_datetime_start += " ADD COLUMN ";
				sql_add_datetime_start += datetime_start_col_name;
				sql_add_datetime_start += " INTEGER DEFAULT 0";

				sqlite3_stmt * stmt_add_datetime_start = NULL;
				sqlite3_prepare_v2(db, sql_add_datetime_start.c_str(), sql_add_datetime_start.size() + 1, &stmt_add_datetime_start, NULL);
				if (stmt_add_datetime_start == NULL)
				{
					// Todo: Error message
					std::string error_msg = sqlite3_errmsg(db);
					failed = true;
					return;
				}

				int step_result_add_datetime_start = 0;
				if ((step_result_add_datetime_start = sqlite3_step(stmt_add_datetime_start)) != SQLITE_DONE)
				{
					// Todo: Error message
					std::string error_msg = sqlite3_errmsg(db);
					failed = true;
					return; // from lambda
				}


				std::string sql_add_datetime_end;
				sql_add_datetime_end += "ALTER TABLE ";
				sql_add_datetime_end += Table_VariableGroupData::ViewNameFromCountTemp(view_count, m);
				sql_add_datetime_end += " ADD COLUMN ";
				sql_add_datetime_end += datetime_end_col_name;
				sql_add_datetime_end += " INTEGER DEFAULT 0";

				sqlite3_stmt * stmt_add_datetime_end = NULL;
				sqlite3_prepare_v2(db, sql_add_datetime_end.c_str(), sql_add_datetime_end.size() + 1, &stmt_add_datetime_end, NULL);
				if (stmt_add_datetime_end == NULL)
				{
					// Todo: Error message
					std::string error_msg = sqlite3_errmsg(db);
					failed = true;
					return;
				}

				int step_result_add_datetime_end = 0;
				if ((step_result_add_datetime_end = sqlite3_step(stmt_add_datetime_end)) != SQLITE_DONE)
				{
					// Todo: Error message
					std::string error_msg = sqlite3_errmsg(db);
					failed = true;
					return; // from lambda
				}

				columns_in_temp_view_vector.push_back(ColumnsInViews::ColumnsInView::ColumnInView());
				ColumnsInViews::ColumnsInView::ColumnInView & column_in_view_datetime_start = columns_in_temp_view_vector.back();
				column_in_view_datetime_start.column_name = datetime_start_col_name;
				column_in_view_datetime_start.column_name_no_uuid = datetime_start_col_name;
				column_in_view_datetime_start.variable_group_identifier = the_variable_group.first;
				column_in_view_datetime_start.uoa_associated_with_variable_group_identifier = *the_variable_group.first.identifier_parent;
				column_in_view_datetime_start.column_type = ColumnsInViews::ColumnsInView::ColumnInView::COLUMN_TYPE__DATETIMESTART;

				columns_in_temp_view_vector.push_back(ColumnsInViews::ColumnsInView::ColumnInView());
				ColumnsInViews::ColumnsInView::ColumnInView & column_in_view_datetime_end = columns_in_temp_view_vector.back();
				column_in_view_datetime_end.column_name = datetime_end_col_name;
				column_in_view_datetime_end.column_name_no_uuid = datetime_end_col_name;
				column_in_view_datetime_end.variable_group_identifier = the_variable_group.first;
				column_in_view_datetime_end.uoa_associated_with_variable_group_identifier = *the_variable_group.first.identifier_parent;
				column_in_view_datetime_end.column_type = ColumnsInViews::ColumnsInView::ColumnInView::COLUMN_TYPE__DATETIMEEND;
			}


			std::string sql_create_real_view;
			sql_create_real_view += "CREATE TABLE ";
			sql_create_real_view += temp_dot;
			sql_create_real_view += Table_VariableGroupData::ViewNameFromCountTempTimeRanged(view_count, m);
			sql_create_real_view += " AS SELECT * FROM ";
			sql_create_real_view += Table_VariableGroupData::ViewNameFromCountTemp(view_count, m);
			if (m > 1)
			{
				// First table is just a copy of the source table, so do not perform the time range processing.
				// But all others must merge into the previous table, so just create them as empty tables,
				// and do proceed with the time range processing.
				sql_create_real_view += " WHERE 0";
			}

			sqlite3_stmt * stmt_real_view = NULL;
			sqlite3_prepare_v2(db, sql_create_real_view.c_str(), sql_create_real_view.size() + 1, &stmt_real_view, NULL);
			if (stmt_real_view == NULL)
			{
				// Todo: Error message
				std::string error_msg = sqlite3_errmsg(db);
				failed = true;
				return;
			}

			int step_result_real_view = 0;
			if ((step_result_real_view = sqlite3_step(stmt_real_view)) != SQLITE_DONE)
			{
				// Todo: Error message
				std::string error_msg = sqlite3_errmsg(db);
				failed = true;
				return; // from lambda
			}

			if (m > 1)
			{

				// Loop through vtmp_m table to create vtmp_TR_m table.
				// The vtmp_m table contains m sets of all variables
				// from the current variable group, from identical columns (all from the same data table),
				// except the column names are appended with a UUID and the value of 'm'.
				// Only the internal time columns for the highest two values of the 'm' index matter.
				// When decomposing rows to create new table, set only the current 'm''s time columns,
				// which will be picked up by later joins to this table being created.

				std::string sql_loop;
				sql_loop = "SELECT * FROM ";
				sql_loop += Table_VariableGroupData::ViewNameFromCountTemp(view_count, m);

				sqlite3_stmt * stmt_loop = NULL;
				sqlite3_prepare_v2(db, sql_loop.c_str(), sql_loop.size() + 1, &stmt_loop, NULL);
				if (stmt_loop == NULL)
				{
					// Todo: Error message
					std::string error_msg = sqlite3_errmsg(db);
					failed = true;
					return;
				}

				long rows_appended = 0;
				long rows_read = 0;
				int step_result_loop = 0;
				while ((step_result_loop = sqlite3_step(stmt_loop)) == SQLITE_ROW)
				{

					if (failed)
					{
						// attempt more rows; do not exit
					}

					++rows_read;

					std::string new_data_string;

					std::string sql_columns;
					std::string sql_values_previous;
					std::string sql_values_previous_null;
					std::string sql_values_before_datetime;
					std::string sql_values_after_datetime;
					std::string sql_values_before_datetime_null;
					std::string sql_values_after_datetime_null;
					bool datetime_active = false;
					bool after_datetime = false;

					int overall_column_number_input_previous = 0;
					int overall_column_number_input_before_datetime = 0;
					int overall_column_number_input_after_datetime = 0;
					int overall_column_number_input = 0;
					std::int64_t datetime_start_previous = 0;
					std::int64_t datetime_end_previous = 0;
					std::int64_t datetime_start_current = 0;
					std::int64_t datetime_end_current = 0;
					for (int j = m - 1; j >= 0; --j)
					{
						if (failed)
						{
							// move on to the next row
							break;
						}
						ColumnsInViews::ColumnsInView & columns_in_temp_view = columnsInView.columns_in_temp_views.columns_in_temp_views_vector[j];
						std::vector<ColumnsInViews::ColumnsInView::ColumnInView> & columns_in_temp_view_vector = columns_in_temp_view.columns_in_view;
						std::for_each(columns_in_temp_view_vector.cbegin(), columns_in_temp_view_vector.cend(), [&sql_columns, &rows_appended, &overall_column_number_input_previous, &sql_values_previous, &sql_values_previous_null, &sql_values_before_datetime_null, &sql_values_after_datetime_null, &new_data_string, &after_datetime, &datetime_active, &sql_values_before_datetime, &sql_values_after_datetime, &overall_column_number_input_before_datetime, &overall_column_number_input, &overall_column_number_input_after_datetime, &j, &view_count, &datetime_start_previous, &datetime_end_previous, &datetime_start_current, &datetime_end_current, &stmt_loop, &failed](ColumnsInViews::ColumnsInView::ColumnInView const & column_in_view)
						{
							if (failed)
							{
								return; // from lambda
							}

							bool current_view = false;

							if (j == view_count - 1)
							{
								// This is the new view being joined in to the previous ones
								current_view = true;
							}

							bool doing_datetime = false;
							if (current_view)
							{
								if (column_in_view.column_type == ColumnsInViews::ColumnsInView::ColumnInView::COLUMN_TYPE__DATETIMESTART)
								{
									// currently, start and end columns must be contiguous
									if (datetime_active || after_datetime)
									{
										failed = true;
										return; // from lambda
									}
									datetime_active = true;
									datetime_start_current = sqlite3_column_int64(stmt_loop, overall_column_number_input);
									doing_datetime = true;
								}
								else if (column_in_view.column_type == ColumnsInViews::ColumnsInView::ColumnInView::COLUMN_TYPE__DATETIMEEND)
								{
									// currently, start and end columns must be contiguous
									if (!datetime_active || after_datetime)
									{
										failed = true;
										return; // from lambda
									}
									datetime_active = false;
									after_datetime = true;
									datetime_end_current = sqlite3_column_int64(stmt_loop, overall_column_number_input);
									doing_datetime = true;
								}
							}
							else
							{
								if (column_in_view.column_type == ColumnsInViews::ColumnsInView::ColumnInView::COLUMN_TYPE__DATETIMESTART)
								{
									datetime_start_previous = sqlite3_column_int64(stmt_loop, overall_column_number_input);
								}
								else if (column_in_view.column_type == ColumnsInViews::ColumnsInView::ColumnInView::COLUMN_TYPE__DATETIMEEND)
								{
									datetime_end_previous = sqlite3_column_int64(stmt_loop, overall_column_number_input);
								}
							}

							if (overall_column_number_input > 0)
							{
								sql_columns += ", ";
							}

							if (!doing_datetime)
							{
								if (after_datetime)
								{
									if (overall_column_number_input_after_datetime > 0)
									{
										sql_values_after_datetime += ", ";
										sql_values_after_datetime_null += ", ";
									}
								}
								else
								{
									if (current_view)
									{
										if (overall_column_number_input_before_datetime > 0)
										{
											sql_values_before_datetime += ", ";
											sql_values_before_datetime_null += ", ";
										}
									}
									else
									{
										if (overall_column_number_input_previous > 0)
										{
											sql_values_previous += ", ";
											sql_values_previous_null += ", ";
										}
									}
								}
							}

							sql_columns += column_in_view.column_name;

							int column_data_type = sqlite3_column_type(stmt_loop, overall_column_number_input);
							bool data_is_null = false;
							switch (column_data_type)
							{
							case SQLITE_INTEGER:
								{
									std::int64_t data = sqlite3_column_int64(stmt_loop, overall_column_number_input);
									new_data_string = boost::lexical_cast<std::string>(data);
									if (!doing_datetime)
									{
										if (after_datetime)
										{
											sql_values_after_datetime += new_data_string;
											sql_values_after_datetime_null += "NULL";
										}
										else
										{
											if (current_view)
											{
												sql_values_before_datetime += new_data_string;
												sql_values_before_datetime_null += "NULL";
											}
											else
											{
												sql_values_previous += new_data_string;
												sql_values_previous_null += "NULL";
											}
										}
									}
								}
								break;
							case SQLITE_FLOAT:
								{
									long double data = sqlite3_column_double(stmt_loop, overall_column_number_input);
									new_data_string = boost::lexical_cast<std::string>(data);
									if (after_datetime)
									{
										sql_values_after_datetime += new_data_string;
										sql_values_after_datetime_null += "NULL";
									}
									else
									{
										if (current_view)
										{
											sql_values_before_datetime += new_data_string;
											sql_values_before_datetime_null += "NULL";
										}
										else
										{
											sql_values_previous += new_data_string;
											sql_values_previous_null += "NULL";
										}
									}
								}
								break;
							case SQLITE_TEXT:
								{
									char const * data = reinterpret_cast<char const *>(sqlite3_column_text(stmt_loop, overall_column_number_input));
									new_data_string = '\'';
									new_data_string += Table_VariableGroupData::EscapeTicks(boost::lexical_cast<std::string>(data));
									new_data_string += '\'';
									if (after_datetime)
									{
										sql_values_after_datetime += new_data_string;
										sql_values_after_datetime_null += "NULL";
									}
									else
									{
										if (current_view)
										{
											sql_values_before_datetime += new_data_string;
											sql_values_before_datetime_null += "NULL";
										}
										else
										{
											sql_values_previous += new_data_string;
											sql_values_previous_null += "NULL";
										}
									}
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
									new_data_string = "NULL";
									if (after_datetime)
									{
										sql_values_after_datetime += new_data_string;
										sql_values_after_datetime_null += "NULL";
									}
									else
									{
										if (current_view)
										{
											sql_values_before_datetime += new_data_string;
											sql_values_before_datetime_null += "NULL";
										}
										else
										{
											sql_values_previous += new_data_string;
											sql_values_previous_null += "NULL";
										}
									}
								}
								break;
							default:
								{
									// Todo: Error message
									failed = true;
									return; // from lambda
								}
							}
							++overall_column_number_input;
							if (!doing_datetime)
							{
								if (after_datetime)
								{
									++overall_column_number_input_after_datetime;
								}
								else
								{
									if (current_view)
									{
										++overall_column_number_input_before_datetime;
									}
									else
									{
										++overall_column_number_input_previous;
									}
								}
							}
						});
					}

					std::string temp_view_table_with_time_ranges_name = Table_VariableGroupData::ViewNameFromCountTempTimeRanged(view_count, m);
					
					bool continue_ = true;

					// handle 0, 0 special cases
					if (continue_)
					{
						if (datetime_start_current == 0 && datetime_end_current == 0)
						{
							if (datetime_start_previous == 0 && datetime_end_previous == 0)
							{
								// No time ranges yet.  Just add the current input row, as-is, with 0 and 0 as indicators of this
								failed = AddTimeRangeMergedRowTemp(false, false, db, 0, 0, overall_column_number_input_previous, overall_column_number_input_before_datetime, overall_column_number_input_after_datetime, sql_columns, sql_values_previous_null, sql_values_previous_null, sql_values_before_datetime, sql_values_before_datetime_null, sql_values_after_datetime, sql_values_after_datetime_null, temp_view_table_with_time_ranges_name);
								++rows_appended;
								continue_ = false;
							}
							else if (datetime_end_previous > datetime_start_previous)
							{
								failed = AddTimeRangeMergedRowTemp(false, false, db, datetime_start_previous, datetime_end_previous, overall_column_number_input_previous, overall_column_number_input_before_datetime, overall_column_number_input_after_datetime, sql_columns, sql_values_previous_null, sql_values_previous_null, sql_values_before_datetime, sql_values_before_datetime_null, sql_values_after_datetime, sql_values_after_datetime_null, temp_view_table_with_time_ranges_name);
								++rows_appended;
								continue_ = false;
							}
							continue_ = false;
						}
						else if (datetime_start_previous == 0 && datetime_end_previous == 0)
						{
							// The equality case is handled above
							if (datetime_end_current > datetime_start_current)
							{
								failed = AddTimeRangeMergedRowTemp(false, false, db, datetime_start_current, datetime_end_current, overall_column_number_input_previous, overall_column_number_input_before_datetime, overall_column_number_input_after_datetime, sql_columns, sql_values_previous_null, sql_values_previous_null, sql_values_before_datetime, sql_values_before_datetime_null, sql_values_after_datetime, sql_values_after_datetime_null, temp_view_table_with_time_ranges_name);
								++rows_appended;
								continue_ = false;
							}
							continue_ = false;
						}
					}

					if (continue_ && datetime_end_current <= datetime_start_current)
					{
						if (continue_ && datetime_end_previous <= datetime_start_previous)
						{
							continue_ = false;
						}
						else
						{
							// Add a row from the start of the previous, to the end of the previous
							failed = AddTimeRangeMergedRowTemp(false, true, db, datetime_start_previous, datetime_end_previous, overall_column_number_input_previous, overall_column_number_input_before_datetime, overall_column_number_input_after_datetime, sql_columns, sql_values_previous, sql_values_previous_null, sql_values_before_datetime, sql_values_before_datetime_null, sql_values_after_datetime, sql_values_after_datetime_null, temp_view_table_with_time_ranges_name);
							++rows_appended;
							//if (failed) continue;

							continue_ = false;
						}
					}

					if (continue_ && datetime_end_previous <= datetime_start_previous)
					{
						// Add a row from the start of the current, to the end of the current
						failed = AddTimeRangeMergedRowTemp(false, true, db, datetime_start_current, datetime_end_current, overall_column_number_input_previous, overall_column_number_input_before_datetime, overall_column_number_input_after_datetime, sql_columns, sql_values_previous, sql_values_previous_null, sql_values_before_datetime, sql_values_before_datetime_null, sql_values_after_datetime, sql_values_after_datetime_null, temp_view_table_with_time_ranges_name);
						++rows_appended;
						//if (failed) continue;

						continue_ = false;
					}

					if (continue_ && datetime_start_current == 0 && datetime_end_current == 0 && datetime_start_previous == 0 && datetime_end_previous == 0)
					{
						// No time ranges yet.  Just add the current input row, as-is, with 0 and 0 as indicators of this
						failed = AddTimeRangeMergedRowTemp(false, false, db, 0, 0, overall_column_number_input_previous, overall_column_number_input_before_datetime, overall_column_number_input_after_datetime, sql_columns, sql_values_previous_null, sql_values_previous_null, sql_values_before_datetime, sql_values_before_datetime_null, sql_values_after_datetime, sql_values_after_datetime_null, temp_view_table_with_time_ranges_name);
						++rows_appended;
						continue_ = false;
					}
					else
					{
						if (continue_&& datetime_start_current == 0 && datetime_end_current == 0)
						{
							// Previous time range exists, but new does not.
							// There can be only one new row per previous row.
							// Add current row as-is, bringing the previous time range over to the new.
							failed = AddTimeRangeMergedRowTemp(false, false, db, datetime_start_previous, datetime_end_previous, overall_column_number_input_previous, overall_column_number_input_before_datetime, overall_column_number_input_after_datetime, sql_columns, sql_values_previous, sql_values_previous_null, sql_values_before_datetime, sql_values_before_datetime_null, sql_values_after_datetime, sql_values_after_datetime_null, temp_view_table_with_time_ranges_name);
							++rows_appended;
							continue_ = false;
						}
						else if (continue_ && datetime_start_previous == 0 && datetime_end_previous == 0)
						{
							// Previous time range did not exist (or there is no previous table),
							// but new time range does exist.
							// There can be only one previous row per new row (if there are previous rows).
							// Add current row as-is, using the current time range.
							failed = AddTimeRangeMergedRowTemp(false, false, db, datetime_start_current, datetime_end_current, overall_column_number_input_previous, overall_column_number_input_before_datetime, overall_column_number_input_after_datetime, sql_columns, sql_values_previous, sql_values_previous_null, sql_values_before_datetime, sql_values_before_datetime_null, sql_values_after_datetime, sql_values_after_datetime_null, temp_view_table_with_time_ranges_name);
							++rows_appended;
							continue_ = false;
						}
						else
						{

							// normal merge - both previous and current time ranges exist
							
							if (datetime_start_current < datetime_start_previous)
							{
								
								if (datetime_end_current >= datetime_start_previous)
								{
									// Add a row from the start of the current, to the start of the previous
									failed = AddTimeRangeMergedRowTemp(true, false, db, datetime_start_current, datetime_start_previous, overall_column_number_input_previous, overall_column_number_input_before_datetime, overall_column_number_input_after_datetime, sql_columns, sql_values_previous, sql_values_previous_null, sql_values_before_datetime, sql_values_before_datetime_null, sql_values_after_datetime, sql_values_after_datetime_null, temp_view_table_with_time_ranges_name);
									++rows_appended;
									//if (failed) continue;
								}
								else
								{
									// Add a row from the start of the previous, to the end of the previous
									failed = AddTimeRangeMergedRowTemp(false, true, db, datetime_start_previous, datetime_end_previous, overall_column_number_input_previous, overall_column_number_input_before_datetime, overall_column_number_input_after_datetime, sql_columns, sql_values_previous, sql_values_previous_null, sql_values_before_datetime, sql_values_before_datetime_null, sql_values_after_datetime, sql_values_after_datetime_null, temp_view_table_with_time_ranges_name);
									++rows_appended;
									//if (failed) continue;

									// Add a row from the start of the current, to the end of the current
									failed = AddTimeRangeMergedRowTemp(true, false, db, datetime_start_current, datetime_end_current, overall_column_number_input_previous, overall_column_number_input_before_datetime, overall_column_number_input_after_datetime, sql_columns, sql_values_previous, sql_values_previous_null, sql_values_before_datetime, sql_values_before_datetime_null, sql_values_after_datetime, sql_values_after_datetime_null, temp_view_table_with_time_ranges_name);
									++rows_appended;
									//if (failed) continue;

									continue_ = false;
								}

								datetime_start_current = datetime_start_previous;
							
							}


							// datetime_start_current is guaranteed to be equal to datetime_start_previous,
							// and both previous and current are guaranteed to be valid, and of non-zero width

							if (continue_ && datetime_end_current >= datetime_end_previous)
							{
								// Add a row from the start of the current, to the end of the previous
								failed = AddTimeRangeMergedRowTemp(false, false, db, datetime_start_current, datetime_end_previous, overall_column_number_input_previous, overall_column_number_input_before_datetime, overall_column_number_input_after_datetime, sql_columns, sql_values_previous, sql_values_previous_null, sql_values_before_datetime, sql_values_before_datetime_null, sql_values_after_datetime, sql_values_after_datetime_null, temp_view_table_with_time_ranges_name);
								++rows_appended;
								//if (failed) continue;

								// Add a row from the end of the previous, to the end of the current
								failed = AddTimeRangeMergedRowTemp(false, false, db, datetime_end_previous, datetime_end_current, overall_column_number_input_previous, overall_column_number_input_before_datetime, overall_column_number_input_after_datetime, sql_columns, sql_values_previous, sql_values_previous_null, sql_values_before_datetime, sql_values_before_datetime_null, sql_values_after_datetime, sql_values_after_datetime_null, temp_view_table_with_time_ranges_name);
								++rows_appended;
								//if (failed) continue;
							}
							else if (continue_)
							{
								// Add a row from the start of the current, to the end of the current
								failed = AddTimeRangeMergedRowTemp(false, false, db, datetime_start_current, datetime_end_current, overall_column_number_input_previous, overall_column_number_input_before_datetime, overall_column_number_input_after_datetime, sql_columns, sql_values_previous, sql_values_previous_null, sql_values_before_datetime, sql_values_before_datetime_null, sql_values_after_datetime, sql_values_after_datetime_null, temp_view_table_with_time_ranges_name);
								++rows_appended;
								//if (failed) continue;

								// ... then from the end of the current, to the end of the previous, and we're done
								failed = AddTimeRangeMergedRowTemp(false, true, db, datetime_end_current, datetime_end_previous, overall_column_number_input_previous, overall_column_number_input_before_datetime, overall_column_number_input_after_datetime, sql_columns, sql_values_previous, sql_values_previous_null, sql_values_before_datetime, sql_values_before_datetime_null, sql_values_after_datetime, sql_values_after_datetime_null, temp_view_table_with_time_ranges_name);
								++rows_appended;
								//if (failed) continue;
							}
						
						}
					}

					if (failed)
					{
						// Do nothing.  Just continue with next row, if possible
					}

					//++number_input_rows;

					//if (number_input_rows >= 1000)
					//{
					//	// For development - limit to 1000 rows
					//	break;
					//}

				}
			}
		}




		// *************************************************************************************** //
		// Construct the SQL used to create a (temporary in release mode) View
		// that caches all primary keys + variable data for the current variable group
		// *************************************************************************************** //

		std::string sql_generate_output;

		sql_generate_output += "CREATE TABLE ";
		sql_generate_output += temp_dot;
		sql_generate_output += Table_VariableGroupData::ViewNameFromCount(view_count);
		sql_generate_output += " AS SELECT ";


		ColumnsInViews::ColumnsInView & the_most_recent_view = columns_in_temp_views[highest_multiplicity_to_use - 1];


		// The first two columns in the main View tables are the time range columns.
		// These correspond to the time range columns added by the LAST variable group that was added to a subview,
		// which is the LAST subview that was created (this subview also contains ALL columns
		// appended to all previous subviews).
		//
		// Note that the ColumnsInView objects only contain the columns APPENDED by the variable group
		// that triggered the creation of the subview, even though the subview contains all previously appended
		// columns as well.

		char nsv_[1024];
		std::string nsv = itoa(view_count, nsv_, 10);

		std::string datetime_start_col_name;
		datetime_start_col_name += "DATETIME_ROW_START";
		datetime_start_col_name += "_";
		datetime_start_col_name += nsv;

		std::string datetime_end_col_name;
		datetime_end_col_name += "DATETIME_ROW_END";
		datetime_end_col_name += "_";
		datetime_end_col_name += nsv;

		bool first_select = false;
		sql_generate_output += Table_VariableGroupData::ViewNameFromCountTempTimeRanged(view_count, highest_multiplicity_to_use);
		sql_generate_output += ".";
		sql_generate_output += the_most_recent_view.datetime_start_column_name;
		sql_generate_output += " AS ";
		sql_generate_output += datetime_start_col_name;
		sql_generate_output += ", ";
		sql_generate_output += Table_VariableGroupData::ViewNameFromCountTempTimeRanged(view_count, highest_multiplicity_to_use);
		sql_generate_output += ".";
		sql_generate_output += the_most_recent_view.datetime_end_column_name;
		sql_generate_output += " AS ";
		sql_generate_output += datetime_end_col_name;

		std::vector<ColumnsInViews::ColumnsInView::ColumnInView> & columns_in_view = columnsInView.columns_in_view;

		columns_in_view.push_back(ColumnsInViews::ColumnsInView::ColumnInView());
		ColumnsInViews::ColumnsInView::ColumnInView & column_in_view_datetime_start = columns_in_view.back();
		column_in_view_datetime_start.column_name = datetime_start_col_name;
		column_in_view_datetime_start.column_name_no_uuid = datetime_start_col_name;
		column_in_view_datetime_start.variable_group_identifier = the_variable_group.first;
		column_in_view_datetime_start.uoa_associated_with_variable_group_identifier = *the_variable_group.first.identifier_parent;
		column_in_view_datetime_start.column_type = ColumnsInViews::ColumnsInView::ColumnInView::COLUMN_TYPE__DATETIMESTART;

		columns_in_view.push_back(ColumnsInViews::ColumnsInView::ColumnInView());
		ColumnsInViews::ColumnsInView::ColumnInView & column_in_view_datetime_end = columns_in_view.back();
		column_in_view_datetime_end.column_name = datetime_end_col_name;
		column_in_view_datetime_end.column_name_no_uuid = datetime_end_col_name;
		column_in_view_datetime_end.variable_group_identifier = the_variable_group.first;
		column_in_view_datetime_end.uoa_associated_with_variable_group_identifier = *the_variable_group.first.identifier_parent;
		column_in_view_datetime_end.column_type = ColumnsInViews::ColumnsInView::ColumnInView::COLUMN_TYPE__DATETIMEEND;

		// Primary keys of multiplicity 1
		int the_primary_key_index = 0;
		std::for_each(sequence.primary_key_sequence_info.cbegin(), sequence.primary_key_sequence_info.cend(), [&the_primary_key_index, &view_count, &highest_multiplicity_to_use, &sql_generate_output, &the_most_recent_view, &columnsInView, &first_select, &the_variable_group, &failed](PrimaryKeySequence::PrimaryKeySequenceEntry const & total_primary_key_sequence_entry)
		{
			std::vector<PrimaryKeySequence::VariableGroup_PrimaryKey_Info> const & variable_group_info_for_primary_keys = total_primary_key_sequence_entry.variable_group_info_for_primary_keys;
			std::for_each(variable_group_info_for_primary_keys.cbegin(), variable_group_info_for_primary_keys.cend(), [&the_primary_key_index, &view_count, &highest_multiplicity_to_use, &sql_generate_output, &the_most_recent_view, &columnsInView, &first_select, &the_variable_group, &total_primary_key_sequence_entry, &failed](PrimaryKeySequence::VariableGroup_PrimaryKey_Info const & variable_group_primary_key_info)
			{
				if (variable_group_primary_key_info.vg_identifier.IsEqual(WidgetInstanceIdentifier::EQUALITY_CHECK_TYPE__STRING_CODE, the_variable_group.first))		
				{
					if (!variable_group_primary_key_info.column_name.empty())
					{
						if (variable_group_primary_key_info.total_multiplicity == 1)
						{
							if (!first_select)
							{
								sql_generate_output += ", ";
							}
							first_select = false;
							sql_generate_output += Table_VariableGroupData::ViewNameFromCountTempTimeRanged(view_count, highest_multiplicity_to_use);
							sql_generate_output += ".";
							sql_generate_output += the_most_recent_view.primary_key_names_in_sub_view[the_primary_key_index];
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

						++the_primary_key_index;
					}
				}
			});
		});

		if (failed)
		{
			// Todo: Error message
			return;
		}

		// Primary keys of multiplicity greater than 1
		for (int m=1; m<=highest_multiplicity_to_use; ++m)
		{

			ColumnsInViews::ColumnsInView & a_single_subview = columns_in_temp_views[m-1];

			int the_primary_key_index = 0;
			std::for_each(sequence.primary_key_sequence_info.cbegin(), sequence.primary_key_sequence_info.cend(), [&m, &a_single_subview, &the_primary_key_index, &view_count, &highest_multiplicity_to_use, &sql_generate_output, &the_most_recent_view, &columnsInView, &first_select, &the_variable_group, &failed](PrimaryKeySequence::PrimaryKeySequenceEntry const & total_primary_key_sequence_entry)
			{
				std::vector<PrimaryKeySequence::VariableGroup_PrimaryKey_Info> const & variable_group_info_for_primary_keys = total_primary_key_sequence_entry.variable_group_info_for_primary_keys;
				std::for_each(variable_group_info_for_primary_keys.cbegin(), variable_group_info_for_primary_keys.cend(), [&m, &a_single_subview, &the_primary_key_index, &view_count, &highest_multiplicity_to_use, &sql_generate_output, &the_most_recent_view, &columnsInView, &first_select, &the_variable_group, &total_primary_key_sequence_entry, &failed](PrimaryKeySequence::VariableGroup_PrimaryKey_Info const & variable_group_primary_key_info)
				{
					if (variable_group_primary_key_info.vg_identifier.IsEqual(WidgetInstanceIdentifier::EQUALITY_CHECK_TYPE__STRING_CODE, the_variable_group.first))		
					{
						if (!variable_group_primary_key_info.column_name.empty())
						{
							if (variable_group_primary_key_info.total_multiplicity > 1)
							{
								if (variable_group_primary_key_info.current_multiplicity == m)
								{
									if (!first_select)
									{
										sql_generate_output += ", ";
									}
									first_select = false;
									sql_generate_output += Table_VariableGroupData::ViewNameFromCountTempTimeRanged(view_count, highest_multiplicity_to_use);
									sql_generate_output += ".";
									sql_generate_output += a_single_subview.primary_key_names_in_sub_view[the_primary_key_index];
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

							++the_primary_key_index;
						}
					}
				});
			});

		}

		// ************************************************* //
		// Display all variables selected by user
		// ************************************************* //
		for (int m=1; m<=highest_multiplicity_to_use; ++m)
		{

			std::string current_table_token = Table_VariableGroupData::ViewNameFromCountTempTimeRanged(view_count, highest_multiplicity_to_use);
			char ns__[64];
			std::string ms__ = itoa(m, ns__, 10);

			ColumnsInViews::ColumnsInView & a_single_subview = columns_in_temp_views[m-1];

			int the_secondary_key_index = 0;
			std::for_each(variables_selected_in_this_group.cbegin(), variables_selected_in_this_group.cend(), [this, &the_secondary_key_index, &a_single_subview, &columnsInView, &the_variable_group, &highest_multiplicity_to_use, &current_uoa_identifier, &first_select, &sql_generate_output, &current_table_token, &ms__, &this_variable_group__primary_key_names__uuid_stripped, &this_variable_group__secondary_key_names, &child_uoas__which_multiplicity_is_greater_than_1, &failed](WidgetInstanceIdentifier const & variable_selected_in_this_group)
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

				if (!first_select)
				{
					sql_generate_output += ", ";
				}
				first_select = false;

				sql_generate_output += current_table_token;
				sql_generate_output += ".";
				sql_generate_output += a_single_subview.secondary_key_names_in_sub_view[the_secondary_key_index];
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

				++the_secondary_key_index;

			});

		}

		if (failed)
		{
			// Todo: Error message
			return;
		}

		sql_generate_output += " FROM ";
		sql_generate_output += Table_VariableGroupData::ViewNameFromCountTempTimeRanged(view_count, highest_multiplicity_to_use);
		sql_generate_output += " ";
		sql_generate_output += Table_VariableGroupData::ViewNameFromCountTempTimeRanged(view_count, highest_multiplicity_to_use);
		sqlite3_stmt * stmt = NULL;
		sqlite3_prepare_v2(db, sql_generate_output.c_str(), sql_generate_output.size() + 1, &stmt, NULL);
		if (stmt == NULL)
		{
			// Todo: Error message
			std::string error_msg = sqlite3_errmsg(db);
			failed = true;
			return;
		}

		int step_result = 0;
		if ((step_result = sqlite3_step(stmt)) != SQLITE_DONE)
		{
			// Todo: Error message
			std::string error_msg = sqlite3_errmsg(db);
			failed = true;
			return; // from lambda
		}

		char join_count_as_text_[1024];
		std::string join_count_as_text = itoa(view_count, join_count_as_text_, 10);

		std::string sql_update_add_datetime_start_internal_column;
		sql_update_add_datetime_start_internal_column += "ALTER TABLE ";
		sql_update_add_datetime_start_internal_column += temp_dot;
		sql_update_add_datetime_start_internal_column += Table_VariableGroupData::ViewNameFromCount(view_count);
		sql_update_add_datetime_start_internal_column += " ADD COLUMN ";
		sql_update_add_datetime_start_internal_column += "DATETIME_START_NEWGENE_INTERNAL_";
		sql_update_add_datetime_start_internal_column += join_count_as_text;
		sql_update_add_datetime_start_internal_column += " INTEGER DEFAULT 0";

		sqlite3_stmt * stmt_add_datetime_start_internal_column = NULL;
		sqlite3_prepare_v2(db, sql_update_add_datetime_start_internal_column.c_str(), sql_update_add_datetime_start_internal_column.size() + 1, &stmt_add_datetime_start_internal_column, NULL);
		if (stmt_add_datetime_start_internal_column == NULL)
		{
			// Todo: Error message
			std::string error_msg = sqlite3_errmsg(db);
			failed = true;
			return;
		}

		int step_result_add_datetime_start_internal_column = 0;
		if ((step_result_add_datetime_start_internal_column = sqlite3_step(stmt_add_datetime_start_internal_column)) != SQLITE_DONE)
		{
			// Todo: Error message
			std::string error_msg = sqlite3_errmsg(db);
			failed = true;
			return; // from lambda
		}

		columns_in_view.push_back(ColumnsInViews::ColumnsInView::ColumnInView());
		ColumnsInViews::ColumnsInView::ColumnInView & column_in_view_start = columns_in_view.back();
		column_in_view_start.column_name = "DATETIME_START_NEWGENE_INTERNAL_";
		column_in_view_start.column_name += join_count_as_text;
		column_in_view_start.column_name_no_uuid = column_in_view_start.column_name;
		column_in_view_start.variable_group_identifier = the_variable_group.first;
		column_in_view_start.uoa_associated_with_variable_group_identifier = *the_variable_group.first.identifier_parent;
		column_in_view_start.column_type = ColumnsInViews::ColumnsInView::ColumnInView::COLUMN_TYPE__DATETIMESTART_INTERNAL;

		std::string sql_update_add_datetime_end_internal_column;
		sql_update_add_datetime_end_internal_column += "ALTER TABLE ";
		sql_update_add_datetime_end_internal_column += temp_dot;
		sql_update_add_datetime_end_internal_column += Table_VariableGroupData::ViewNameFromCount(view_count);
		sql_update_add_datetime_end_internal_column += " ADD COLUMN ";
		sql_update_add_datetime_end_internal_column += "DATETIME_END_NEWGENE_INTERNAL_";
		sql_update_add_datetime_end_internal_column += join_count_as_text;
		sql_update_add_datetime_end_internal_column += " INTEGER DEFAULT 0";

		sqlite3_stmt * stmt_add_datetime_end_internal_column = NULL;
		sqlite3_prepare_v2(db, sql_update_add_datetime_end_internal_column.c_str(), sql_update_add_datetime_end_internal_column.size() + 1, &stmt_add_datetime_end_internal_column, NULL);
		if (stmt_add_datetime_end_internal_column == NULL)
		{
			// Todo: Error message
			std::string error_msg = sqlite3_errmsg(db);
			failed = true;
			return;
		}

		int step_result_add_datetime_end_internal_column = 0;
		if ((step_result_add_datetime_end_internal_column = sqlite3_step(stmt_add_datetime_end_internal_column)) != SQLITE_DONE)
		{
			// Todo: Error message
			std::string error_msg = sqlite3_errmsg(db);
			failed = true;
			return; // from lambda
		}

		columns_in_view.push_back(ColumnsInViews::ColumnsInView::ColumnInView());
		ColumnsInViews::ColumnsInView::ColumnInView & column_in_view_end = columns_in_view.back();
		column_in_view_end.column_name = "DATETIME_END_NEWGENE_INTERNAL_";
		column_in_view_end.column_name += join_count_as_text;
		column_in_view_end.column_name_no_uuid = column_in_view_end.column_name;
		column_in_view_end.variable_group_identifier = the_variable_group.first;
		column_in_view_end.uoa_associated_with_variable_group_identifier = *the_variable_group.first.identifier_parent;
		column_in_view_end.column_type = ColumnsInViews::ColumnsInView::ColumnInView::COLUMN_TYPE__DATETIMEEND_INTERNAL;
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

		if (join_count == 1)
		{
			sql_generate_output += temp_dot;
			sql_generate_output += Table_VariableGroupData::ViewNameFromCount(join_count);
			sql_generate_output += " ";
			sql_generate_output += Table_VariableGroupData::ViewNameFromCount(join_count);
		}
		else
		{
			sql_generate_output += temp_dot;
			sql_generate_output += Table_VariableGroupData::JoinViewNameWithTimeRangesFromCount(join_count - 1);
			sql_generate_output += " ";
			sql_generate_output += Table_VariableGroupData::JoinViewNameWithTimeRangesFromCount(join_count - 1);
		}

		if (join_count > 1)
		{
		
			// join with the previous join

			PrimaryKey_SecondaryKey_Names & this_variable_group__key_names__previous = variable_group__key_names__vectors[join_count - 2];
			std::vector<std::string> & this_variable_group__primary_key_names__previous = this_variable_group__key_names__previous.first;
			std::vector<std::string> & this_variable_group__secondary_key_names__previous = this_variable_group__key_names__previous.second;

			sql_generate_output += " LEFT OUTER JOIN ";
			sql_generate_output += temp_dot;
			sql_generate_output += Table_VariableGroupData::ViewNameFromCount(join_count);
			sql_generate_output += " ";
			sql_generate_output += Table_VariableGroupData::ViewNameFromCount(join_count);
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

			// Uncomment to full outer join child variable groups to parent variable groups
			// ... this will include child rows even where there is no parent data
			// (some modifications to the code may be necessary ... check through if this code is needed)
#			if 0
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
#			endif

		}
	
		sqlite3_stmt * stmt = NULL;
		sqlite3_prepare_v2(db, sql_generate_output.c_str(), sql_generate_output.size() + 1, &stmt, NULL);
		if (stmt == NULL)
		{
			// Todo: Error message
			std::string error_msg = sqlite3_errmsg(db);
			failed = true;
			return;
		}

		int step_result = 0;
		if ((step_result = sqlite3_step(stmt)) != SQLITE_DONE)
		{
			// Todo: Error message
			std::string error_msg = sqlite3_errmsg(db);
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
			std::string error_msg = sqlite3_errmsg(db);
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
		sql_create_timerange_table += ".*";
		sql_create_timerange_table += " ";
		sql_create_timerange_table += " FROM ";
		sql_create_timerange_table += temp_dot;
		sql_create_timerange_table += Table_VariableGroupData::JoinViewNameFromCount(join_count);
		sql_create_timerange_table += " ";
		sql_create_timerange_table += Table_VariableGroupData::JoinViewNameFromCount(join_count);
		sql_create_timerange_table += " WHERE 0"; // just create an empty table whose column types match, by default
		sqlite3_stmt * stmt_create_timerange_table = NULL;
		sqlite3_prepare_v2(db, sql_create_timerange_table.c_str(), sql_create_timerange_table.size() + 1, &stmt_create_timerange_table, NULL);
		if (stmt_create_timerange_table == NULL)
		{
			// Todo: Error message
			std::string error_msg = sqlite3_errmsg(db);
			failed = true;
			return; // from lambda
		}
		int step_result_create_table = 0;
		if ((step_result_create_table = sqlite3_step(stmt_create_timerange_table)) != SQLITE_DONE)
		{
			// Todo: Error message
			std::string error_msg = sqlite3_errmsg(db);
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
			std::string sql_values_previous_null;
			std::string sql_values_current_null;
			std::string sql_values_previous_filled;
			std::string sql_values_current_filled;
			//std::string sql_values_current_datetime_internal;
			std::string new_data_string;
			//std::string new_current_datetime_string;

			// The rows that are returned from this query
			// are the union of all rows from all views up to
			// and including the current
			// Work from the outside in, since that's how the tables were joined
			int overall_column_number_input = 0;
			int overall_column_number_previous = 0;
			int overall_column_number_current_regular = 0;
			//int overall_column_number_current_datetime_internal = 0;
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
				std::for_each(columns_in_view_vector.cbegin(), columns_in_view_vector.cend(), [&sql_columns, &new_data_string, &sql_values_previous_null, &sql_values_previous_filled, &sql_values_current_null, &sql_values_current_filled, &overall_column_number_previous, &overall_column_number_current_regular, &overall_column_number_input, &j, &join_count, &datetime_start_previous, &datetime_end_previous, &datetime_start_current, &datetime_end_current, &stmt_select_output, &failed](ColumnsInViews::ColumnsInView::ColumnInView const & column_in_view)
				{
					if (failed)
					{
						return; // from lambda
					}

					bool current_view = false;

					if (j == join_count - 1)
					{
						// This is the new view being joined in to the previous ones
						current_view = true;
					}

					bool is_current_datetime_internal_column_being_read = false;

					if (current_view)
					{
						if (column_in_view.column_type == ColumnsInViews::ColumnsInView::ColumnInView::COLUMN_TYPE__DATETIMESTART)
						{
							datetime_start_current = sqlite3_column_int64(stmt_select_output, overall_column_number_input);
							//new_current_datetime_string = boost::lexical_cast<std::string>(datetime_start_current);
						}
						else if (column_in_view.column_type == ColumnsInViews::ColumnsInView::ColumnInView::COLUMN_TYPE__DATETIMEEND)
						{
							datetime_end_current = sqlite3_column_int64(stmt_select_output, overall_column_number_input);
							//new_current_datetime_string = boost::lexical_cast<std::string>(datetime_end_current);
						}
						else if (column_in_view.column_type == ColumnsInViews::ColumnsInView::ColumnInView::COLUMN_TYPE__DATETIMESTART_INTERNAL)
						{
							is_current_datetime_internal_column_being_read = true;
						}
						else if (column_in_view.column_type == ColumnsInViews::ColumnsInView::ColumnInView::COLUMN_TYPE__DATETIMEEND_INTERNAL)
						{
							is_current_datetime_internal_column_being_read = true;
						}
					}
					else
					{
						if (column_in_view.column_type == ColumnsInViews::ColumnsInView::ColumnInView::COLUMN_TYPE__DATETIMESTART_INTERNAL)
						{
							datetime_start_previous = sqlite3_column_int64(stmt_select_output, overall_column_number_input);
						}
						else if (column_in_view.column_type == ColumnsInViews::ColumnsInView::ColumnInView::COLUMN_TYPE__DATETIMEEND_INTERNAL)
						{
							datetime_end_previous = sqlite3_column_int64(stmt_select_output, overall_column_number_input);
						}
					}

					if (overall_column_number_input > 0)
					{
						sql_columns += ", ";
					}

					if (current_view)
					{
						if (is_current_datetime_internal_column_being_read)
						{
							//if (overall_column_number_current_datetime_internal > 0)
							//{
								//sql_values_current_datetime_internal += ", ";
							//}
						}
						else
						{
							if (overall_column_number_current_regular > 0)
							{
								sql_values_current_null += ", ";
								sql_values_current_filled += ", ";
							}
						}
					}
					else
					{
						if (overall_column_number_previous > 0)
						{
							sql_values_previous_null += ", ";
							sql_values_previous_filled += ", ";
						}
					}

					sql_columns += column_in_view.column_name;

					int column_data_type = sqlite3_column_type(stmt_select_output, overall_column_number_input);
					bool data_is_null = false;
					switch (column_data_type)
					{
					case SQLITE_INTEGER:
						{
							std::int64_t data = sqlite3_column_int64(stmt_select_output, overall_column_number_input);
							new_data_string = boost::lexical_cast<std::string>(data);
							if (current_view)
							{
								if (column_in_view.column_type == ColumnsInViews::ColumnsInView::ColumnInView::COLUMN_TYPE__DATETIMESTART_INTERNAL)
								{
									//sql_values_current_datetime_internal += new_current_datetime_string;
								}
								else if (column_in_view.column_type == ColumnsInViews::ColumnsInView::ColumnInView::COLUMN_TYPE__DATETIMEEND_INTERNAL)
								{
									//sql_values_current_datetime_internal += new_current_datetime_string;
								}
								else
								{
									// Only if not internal date time column do we fill it here
									sql_values_current_filled += new_data_string;
									sql_values_current_null += "NULL";
								}
							}
							else
							{
								sql_values_previous_filled += new_data_string;
								sql_values_previous_null += "NULL";
							}
						}
						break;
					case SQLITE_FLOAT:
						{
							long double data = sqlite3_column_double(stmt_select_output, overall_column_number_input);
							new_data_string = boost::lexical_cast<std::string>(data);
							if (current_view)
							{
								sql_values_current_filled += new_data_string;
								sql_values_current_null += "NULL";
							}
							else
							{
								sql_values_previous_filled += new_data_string;
								sql_values_previous_null += "NULL";
							}
						}
						break;
					case SQLITE_TEXT:
						{
							char const * data = reinterpret_cast<char const *>(sqlite3_column_text(stmt_select_output, overall_column_number_input));
							new_data_string = '\'';
							new_data_string += Table_VariableGroupData::EscapeTicks(boost::lexical_cast<std::string>(data));
							new_data_string += '\'';
							if (current_view)
							{
								sql_values_current_filled += new_data_string;
								sql_values_current_null += "NULL";
							}
							else
							{
								sql_values_previous_filled += new_data_string;
								sql_values_previous_null += "NULL";
							}
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
							new_data_string = "NULL";
							if (current_view)
							{
								sql_values_current_filled += new_data_string;
								sql_values_current_null += "NULL";
							}
							else
							{
								sql_values_previous_filled += new_data_string;
								sql_values_previous_null += "NULL";
							}
						}
						break;
					default:
						{
							// Todo: Error message
							failed = true;
							return; // from lambda
						}
					}
					++overall_column_number_input;
					if (current_view)
					{
						if (is_current_datetime_internal_column_being_read)
						{
							//++overall_column_number_current_datetime_internal;
						}
						else
						{
							++overall_column_number_current_regular;
						}
					}
					else
					{
						++overall_column_number_previous;
					}
				});
			}

			if (datetime_start_current == 0 && datetime_end_current == 0 && datetime_start_previous == 0 && datetime_end_previous == 0)
			{
				// No time ranges yet.  Just add the current input row, as-is, with 0 and 0 as indicators of this
				failed = AddTimeRangeMergedRow(false, false, db, 0, 0, overall_column_number_previous, overall_column_number_current_regular, sql_columns, sql_values_previous_null, sql_values_previous_filled, sql_values_current_null, sql_values_current_filled, join_count_as_text, join_table_with_time_ranges_name);
			}
			else
			{
				if (datetime_start_current == 0 && datetime_end_current == 0)
				{
					// Previous time range exists, but new does not.
					// There can be only one new row per previous row.
					// Add current row as-is, bringing the previous time range over to the new.
					failed = AddTimeRangeMergedRow(false, false, db, datetime_start_previous, datetime_end_previous, overall_column_number_previous, overall_column_number_current_regular, sql_columns, sql_values_previous_null, sql_values_previous_filled, sql_values_current_null, sql_values_current_filled, join_count_as_text, join_table_with_time_ranges_name);
				}
				else if (datetime_start_previous == 0 && datetime_end_previous == 0)
				{
					// Previous time range did not exist (or there is no previous table),
					// but new time range does exist.
					// There can be only one previous row per new row (if there are previous rows).
					// Add current row as-is, using the current time range.
					failed = AddTimeRangeMergedRow(false, false, db, datetime_start_current, datetime_end_current, overall_column_number_previous, overall_column_number_current_regular, sql_columns, sql_values_previous_null, sql_values_previous_filled, sql_values_current_null, sql_values_current_filled, join_count_as_text, join_table_with_time_ranges_name);
				}
				else
				{
					// normal merge - both previous and current time ranges exist

					// If we're here, a child is being merged in.

					// Method two: Assume that only 1 primary variable group is being included,
					// and any following variable groups are children.
					// The children will not provide independent row, but only rows
					// that overlap the time range of the primary.
					bool method_two = true;

					if (method_two)
					{
						if (datetime_start_current < datetime_start_previous)
						{
							// Skip the part of the child row that precedes the start of the time range of the previous row being merged into
							datetime_start_current = datetime_start_previous;
						}
						if (datetime_end_current > datetime_start_current)
						{
							if (datetime_start_current < datetime_end_previous)
							{
								// We have something to do
								if (datetime_start_current >= datetime_start_previous)
								{

									if (datetime_start_current > datetime_start_previous)
									{
										// Add a row from the start of the previous, to the start of the current
										failed = AddTimeRangeMergedRow(false, true, db, datetime_start_previous, datetime_start_current, overall_column_number_previous, overall_column_number_current_regular, sql_columns, sql_values_previous_null, sql_values_previous_filled, sql_values_current_null, sql_values_current_filled, join_count_as_text, join_table_with_time_ranges_name);
										//if (failed) continue;
									}

									if (datetime_end_current >= datetime_end_previous)
									{
										// Add a row from the start of the current, to the end of the previous, and we're done
										failed = AddTimeRangeMergedRow(false, false, db, datetime_start_current, datetime_end_previous, overall_column_number_previous, overall_column_number_current_regular, sql_columns, sql_values_previous_null, sql_values_previous_filled, sql_values_current_null, sql_values_current_filled, join_count_as_text, join_table_with_time_ranges_name);
										//if (failed) continue;

										// Do not add the part of the child row that is past the end of the time range of the previous row being merged into
									}
									else
									{
										// Add a row from the start of the current, to the end of the current
										failed = AddTimeRangeMergedRow(false, false, db, datetime_start_current, datetime_end_current, overall_column_number_previous, overall_column_number_current_regular, sql_columns, sql_values_previous_null, sql_values_previous_filled, sql_values_current_null, sql_values_current_filled, join_count_as_text, join_table_with_time_ranges_name);
										//if (failed) continue;

										// ... then from the end of the current, to the end of the previous, and we're done
										failed = AddTimeRangeMergedRow(false, true, db, datetime_end_current, datetime_end_previous, overall_column_number_previous, overall_column_number_current_regular, sql_columns, sql_values_previous_null, sql_values_previous_filled, sql_values_current_null, sql_values_current_filled, join_count_as_text, join_table_with_time_ranges_name);
										//if (failed) continue;
									}
								}
							}
						}
					}
				}
			}

			if (failed)
			{
				// Do nothing.  Just continue with next row, if possible
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
		// Do nothing: Failures at this point mean that individual rows failed;
		// but others may have succeeded
		//return;
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
		std::string error_msg = sqlite3_errmsg(db);
		failed = true;
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
			std::string error_msg = sqlite3_errmsg(db);
			failed = true;
			return;
		}

		int step_result = 0;
		if ((step_result = sqlite3_step(stmt)) != SQLITE_DONE)
		{
			// Todo: Error message
			std::string error_msg = sqlite3_errmsg(db);
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
			std::string error_msg = sqlite3_errmsg(db);
			failed = true;
			return;
		}

		int step_result = 0;
		if ((step_result = sqlite3_step(stmt)) != SQLITE_DONE)
		{
			// Todo: Error message
			std::string error_msg = sqlite3_errmsg(db);
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

bool OutputModel::AddTimeRangeMergedRow(bool previous_is_null, bool current_is_null, sqlite3 * db, std::int64_t const datetime_start_new, std::int64_t const datetime_end_new, int & overall_column_number_previous, int & overall_column_number_current_regular, std::string & sql_columns, std::string & sql_values_previous_null, std::string & sql_values_previous_filled, std::string & sql_values_current_null, std::string & sql_values_current_filled, std::string & join_count_as_text, std::string const & join_table_with_time_ranges_name)
{
	std::string sql_insert_time_range_row;
	sql_insert_time_range_row += "INSERT INTO ";
	sql_insert_time_range_row += join_table_with_time_ranges_name;
	sql_insert_time_range_row += " (";
	sql_insert_time_range_row += sql_columns;
	sql_insert_time_range_row += ") VALUES (";

	if (previous_is_null)
	{
		sql_insert_time_range_row += sql_values_previous_null;
	}
	else
	{
		sql_insert_time_range_row += sql_values_previous_filled;
	}

	if (overall_column_number_previous > 0)
	{
		sql_insert_time_range_row += ", ";
	}

	if (current_is_null)
	{
		sql_insert_time_range_row += sql_values_current_null;
	}
	else
	{
		sql_insert_time_range_row += sql_values_current_filled;
	}

	std::string sql_values_current_datetime_internal;
	sql_values_current_datetime_internal += boost::lexical_cast<std::string>(datetime_start_new);
	sql_values_current_datetime_internal += ", ";
	sql_values_current_datetime_internal += boost::lexical_cast<std::string>(datetime_end_new);

	if (overall_column_number_current_regular > 0 || overall_column_number_previous > 0)
	{
		sql_insert_time_range_row += ", ";
	}

	sql_insert_time_range_row += sql_values_current_datetime_internal;

	sql_insert_time_range_row += ")";
	sqlite3_stmt * stmt_insert_new_row = NULL;
	sqlite3_prepare_v2(db, sql_insert_time_range_row.c_str(), sql_insert_time_range_row.size() + 1, &stmt_insert_new_row, NULL);
	if (stmt_insert_new_row == NULL)
	{
		std::string error_msg = sqlite3_errmsg(db);
		return true;
	}
	int step_result_insert_new_row = 0;
	if ((step_result_insert_new_row = sqlite3_step(stmt_insert_new_row)) != SQLITE_DONE)
	{
		// Move on to the next row
		return true;
	}
	return false;
}

bool OutputModel::AddTimeRangeMergedRowTemp(bool previous_is_null, bool current_is_null, sqlite3 * db, std::int64_t const datetime_start_new, std::int64_t const datetime_end_new, int const overall_column_number_input_previous, int const overall_column_number_input_before_datetime, int const overall_column_number_input_after_datetime, std::string const & sql_columns, std::string const & sql_values_previous, std::string const & sql_values_previous_null, std::string const & sql_values_before_datetime, std::string const & sql_values_before_datetime_null, std::string const & sql_values_after_datetime, std::string const & sql_values_after_datetime_null, std::string const & join_table_with_time_ranges_name)
{
	std::string sql_insert_time_range_row;
	sql_insert_time_range_row += "INSERT INTO ";
	sql_insert_time_range_row += join_table_with_time_ranges_name;
	sql_insert_time_range_row += " (";
	sql_insert_time_range_row += sql_columns;
	sql_insert_time_range_row += ") VALUES (";

	if (previous_is_null)
	{
		sql_insert_time_range_row += sql_values_previous_null;
	}
	else
	{
		sql_insert_time_range_row += sql_values_previous;
	}

	if (overall_column_number_input_previous > 0)
	{
		sql_insert_time_range_row += ", ";
	}

	if (current_is_null)
	{
		sql_insert_time_range_row += sql_values_before_datetime_null;
	}
	else
	{
		sql_insert_time_range_row += sql_values_before_datetime;
	}

	if (overall_column_number_input_before_datetime > 0)
	{
		sql_insert_time_range_row += ", ";
	}

	std::string sql_values_current_datetime;
	sql_values_current_datetime += boost::lexical_cast<std::string>(datetime_start_new);
	sql_values_current_datetime += ", ";
	sql_values_current_datetime += boost::lexical_cast<std::string>(datetime_end_new);

	sql_insert_time_range_row += sql_values_current_datetime;

	if (overall_column_number_input_after_datetime > 0)
	{
		sql_insert_time_range_row += ", ";
	}

	if (current_is_null)
	{
		sql_insert_time_range_row += sql_values_after_datetime_null;
	}
	else
	{
		sql_insert_time_range_row += sql_values_after_datetime;
	}

	sql_insert_time_range_row += ")";
	sqlite3_stmt * stmt_insert_new_row = NULL;
	sqlite3_prepare_v2(db, sql_insert_time_range_row.c_str(), sql_insert_time_range_row.size() + 1, &stmt_insert_new_row, NULL);
	if (stmt_insert_new_row == NULL)
	{
		std::string error_msg = sqlite3_errmsg(db);
		return true;
	}
	int step_result_insert_new_row = 0;
	if ((step_result_insert_new_row = sqlite3_step(stmt_insert_new_row)) != SQLITE_DONE)
	{
		// Move on to the next row
		return true;
	}
	return false;
}
