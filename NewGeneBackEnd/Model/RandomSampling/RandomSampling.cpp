#include "RandomSampling.h"

#ifndef Q_MOC_RUN
#	include <boost/scope_exit.hpp>
#	include <boost/math/special_functions/binomial.hpp>
#	include <boost/multiprecision/random.hpp>
#	include <boost/multiprecision/cpp_dec_float.hpp>
#	include <boost/assert.hpp>
#endif
#include <random>
#include <functional>
#include <list>
#include <algorithm>
#include "../../Utilities/TimeRangeHelper.h"

std::fstream * create_output_row_visitor::output_file = nullptr;
int create_output_row_visitor::mode = static_cast<int>(create_output_row_visitor::CREATE_ROW_MODE__NONE);
std::vector<InstanceData> create_output_row_visitor::data;
int * create_output_row_visitor::bind_index = nullptr;
sqlite3_stmt * create_output_row_visitor::insert_stmt = nullptr;
bool MergedTimeSliceRow::RHS_wins = false;
std::string create_output_row_visitor::row_in_process;

AllWeightings::AllWeightings(Messager & messager_)
: insert_random_sample_stmt(nullptr)
, numberChildVariableGroups(0)
, time_granularity(TIME_GRANULARITY__NONE)
, random_rows_added(0)
, messager(messager_)
{

}

AllWeightings::~AllWeightings()
{
	if (insert_random_sample_stmt)
	{
		sqlite3_finalize(insert_random_sample_stmt);
		insert_random_sample_stmt = nullptr;
	}
}

bool AllWeightings::HandleIncomingNewBranchAndLeaf(Branch const & branch, TimeSliceLeaf & newTimeSliceLeaf, int const & variable_group_number, VARIABLE_GROUP_MERGE_MODE const merge_mode, std::int64_t const AvgMsperUnit, bool const consolidate_rows, bool const random_sampling)
{

	TimeSlice const & newTimeSlice = newTimeSliceLeaf.first;

	newTimeSlice.Validate();

	bool added = false; // true if there is a match

	if (merge_mode == VARIABLE_GROUP_MERGE_MODE__PRIMARY)
	{
		// Always add data for primary variable group
		added = true;
	}

	// determine which case we are in terms of the relationship of the incoming new 'timeSliceLeaf' 
	// so that we know whether/how to break up either the new 'rhs' time slice
	// or the potentially overlapping existing time slices in the 'timeSlices' map

	// Save beginning, one past end time slices in the existing map for reference
	TimeSlices::iterator existing_start_slice          = timeSlices.begin();
	TimeSlices::iterator existing_one_past_end_slice   = timeSlices.end();

	TimeSlices::iterator mapIterator;
	bool normalCase = false;

	if (existing_start_slice == existing_one_past_end_slice)
	{
		if (merge_mode == VARIABLE_GROUP_MERGE_MODE__PRIMARY)
		{
			// No entries in the 'timeSlices' map yet
			// Add the entire new time slice as the first entry in the map
			AddNewTimeSlice(variable_group_number, branch, newTimeSliceLeaf);
		}
	}
	else
	{

		// ********************************************************************************************* //
		// ********************************************************************************************* //
		//
		// Another slick use of upper_bound!
		// Map elements are never compared against each other.
		// They are only compared against the new time slice.
		// If the new time slice is EQUAL to a given map entry, for example,
		// the map entry will be considered GREATER than the new time slice using this comparison.
		//
		// ********************************************************************************************* //
		// ********************************************************************************************* //

		TimeSlices::iterator startMapSlicePtr = std::upper_bound(timeSlices.begin(), timeSlices.end(), newTimeSliceLeaf, &AllWeightings::is_map_entry_end_time_greater_than_new_time_slice_start_time);
		bool start_of_new_slice_is_past_end_of_map = false;
		if (startMapSlicePtr == existing_one_past_end_slice)
		{
			start_of_new_slice_is_past_end_of_map = true;
		}

		if (start_of_new_slice_is_past_end_of_map)
		{
			if (merge_mode == VARIABLE_GROUP_MERGE_MODE__PRIMARY)
			{
				// The new slice is entirely past the end of the map.
				// Add new map entry consisting solely of the new slice.
				AddNewTimeSlice(variable_group_number, branch, newTimeSliceLeaf);
			}
		}
		else
		{

			// The start of the new slice is to the left of the end of the map

			TimeSlice const & startMapSlice = startMapSlicePtr->first;

			// The new time slice MIGHT start to the left of the map element returned by upper_bound.
			// Or it might not - it might start IN or at the left edge of this element.
			// In any case, upper_bound ensures that it starts PAST any previous map elements.

			if (newTimeSlice.IsStartLessThanRHSStart(startMapSlice))
			{

				// The new time slice starts to the left of the map element returned by upper_bound.

				if (newTimeSlice.IsEndLessThanOrEqualToRHSStart(startMapSlice))
				{
					if (merge_mode == VARIABLE_GROUP_MERGE_MODE__PRIMARY)
					{
						// The entire new time slice is less than the map element returned by upper_bound.
						// (But past previous map elements, if any.)
						// Add the entire new time slice as a unit to the map.
						AddNewTimeSlice(variable_group_number, branch, newTimeSliceLeaf);
					}
				}
				else
				{

					// The new time slice starts to the left of the map element returned by upper_bound.
					// The right edge of the new time slice could end anywhere past the left edge
					// of the map element, including past the right edge of the map element.

					// Slice off the left-most piece of the new time slice
					// (and add it to the map if appropriate)
					TimeSliceLeaf new_left_slice;
					SliceOffLeft(newTimeSliceLeaf, startMapSlicePtr->first.getStart(), new_left_slice);

					if (merge_mode == VARIABLE_GROUP_MERGE_MODE__PRIMARY)
					{
						AddNewTimeSlice(variable_group_number, branch, new_left_slice);
					}

					// For the remainder of the slice, proceed with normal case
					normalCase = true;
					mapIterator = startMapSlicePtr;

				}

			}
			else
			{

				// The new time slice starts at the left edge, or to the right of the left edge,
				// of the map element returned by upper_bound.
				// The right edge of the new time slice could end anywhere (of course, it must be past the left edge of the new time slice).

				// This is the normal case.
				normalCase = true;
				mapIterator = startMapSlicePtr;

			}

		}

	}

	if (normalCase)
	{
		// This is the normal case.  The incoming new slice is guaranteed to have
		// its left edge either equal to the left edge of the map element,
		// or to the right of the left edge of the map element but less than the right edge of the map element.
		// The right edge of the incoming new slice could lie anywhere greater than the left edge -
		// it could lie either inside the map element, at the right edge of the map element,
		// or past the right edge of the map element.
		bool no_more_time_slice = HandleTimeSliceNormalCase(added, branch, newTimeSliceLeaf, mapIterator, variable_group_number, merge_mode, AvgMsperUnit, consolidate_rows, random_sampling);
		if (no_more_time_slice)
		{
			// no-op
		}
		else if (mapIterator == timeSlices.end())
		{
			// We have a chunk left but it's past the end of the map.
			if (merge_mode == VARIABLE_GROUP_MERGE_MODE__PRIMARY)
			{
				// Add it solo to the end of the map.
				AddNewTimeSlice(variable_group_number, branch, newTimeSliceLeaf);
			}
		}
		else
		{
			bool added_recurse = HandleIncomingNewBranchAndLeaf(branch, newTimeSliceLeaf, variable_group_number, merge_mode, AvgMsperUnit, consolidate_rows, random_sampling);
			if (added_recurse)
			{
				added = true; // It could have been true previously, so never set to false
			}
		}
	}

	return added;

}

// This function that takes the following arguments:
// - A time slice
// - An entry in the map
// The time slice's left edge is guaranteed to be inside or at the left edge of
// (not at the right edge of) an existing (non-"end()") entry of the map -
// it is guaranteed not to be to the left of the left edge of the map entry.
// This function does the following:
// Processes the given entry in the map in relation to the time slice.
//     This involves either merging the entire slice into some or all
//     of the map entry (if the entire slice fits inside the map entry),
//     or slicing off the left part of the slice and merging it with
//     either the entire map entry (if it is a perfect overlap)
//     or slicing the map entry and merging the left part of the slice
//     with the right part of the map entry (if the left of the map entry
//     extends to the left of the left edge of the slice).
// The function returns the following information:
// - Was the entire slice merged (true), or is there a part of the slice
//   that extends beyond the right edge of the map entry (false)?
// The function returns the following data through incoming parameters (by reference):
// If the return value is true:
// - undefined value of time slice
// - (unused currently) iterator to the map entry whose left edge corresponds to the left edge
//   of the original time slice (but this is unused by the algorithm)
// If the return value is false:
// - The portion of the incoming slice that extends past the right edge
//   of the map entry
// - An iterator to the next map entry.
bool AllWeightings::HandleTimeSliceNormalCase(bool & added, Branch const & branch, TimeSliceLeaf & newTimeSliceLeaf, TimeSlices::iterator & mapElementPtr, int const & variable_group_number, VARIABLE_GROUP_MERGE_MODE const merge_mode, std::int64_t const AvgMsperUnit, bool const consolidate_rows, bool const random_sampling)
{

	bool added_new = false;

	TimeSlice const & newTimeSlice = newTimeSliceLeaf.first;
	TimeSlice const & mapElement   = mapElementPtr->first;

	TimeSlices::iterator newMapElementLeftPtr;
	TimeSlices::iterator newMapElementMiddlePtr;
	TimeSlices::iterator newMapElementRightPtr;

	TimeSliceLeaf new_left_slice;

	bool newTimeSliceEatenCompletelyUp = false;

	if (newTimeSlice.IsStartEqualToRHSStart(mapElement))
	{

		// The new time slice starts at the left edge of the map element.

		if (newTimeSlice.IsEndLessThanRHSEnd(mapElement))
		{

			// The new time slice starts at the left edge of the map element,
			// and fits inside the map element,
			// with space left over in the map element.

			// Slice the map element into two pieces.
			// Merge the first piece with the new time slice.
			// Leave the second piece unchanged.

			SliceMapEntry(mapElementPtr, newTimeSlice.getEnd(), newMapElementLeftPtr, newMapElementRightPtr, AvgMsperUnit, consolidate_rows, random_sampling);
			added_new = MergeTimeSliceDataIntoMap(branch, newTimeSliceLeaf, newMapElementLeftPtr, variable_group_number, merge_mode);

			mapElementPtr = newMapElementLeftPtr;

			newTimeSliceEatenCompletelyUp = true;

		}
		else if (newTimeSlice.IsEndEqualToRHSEnd(mapElement))
		{

			// The new time slice exactly matches the first map element.

			// Merge the new time slice with the map element.

			added_new = MergeTimeSliceDataIntoMap(branch, newTimeSliceLeaf, mapElementPtr, variable_group_number, merge_mode);

			newTimeSliceEatenCompletelyUp = true;

		}
		else
		{

			// The new time slice starts at the left edge of the map element,
			// and extends past the right edge.

			// Slice the first part of the new time slice off so that it
			// perfectly overlaps the map element.
			// Merge it with the map element.

			// Then set the iterator to point to the next map element, ready for the next iteration.
			// The remainder of the new time slice (at the right) is now in the variable "newTimeSliceLeaf" and ready for the next iteration.

			SliceOffLeft(newTimeSliceLeaf, mapElement.getEnd(), new_left_slice);
			added_new = MergeTimeSliceDataIntoMap(branch, new_left_slice, mapElementPtr, variable_group_number, merge_mode);

			mapElementPtr = ++mapElementPtr;

		}

	}
	else
	{

		// The new time slice is inside the map element,
		// but starts past its left edge.

		if (newTimeSlice.IsEndLessThanRHSEnd(mapElement))
		{

			// The new time slice starts in the map element,
			// but past its left edge.
			// The new time slice ends before the right edge of the map element.

			// Slice the map element into three pieces.
			// The first is unchanged.
			// The second merges with the new time slice.
			// The third is unchanged.

			SliceMapEntry(mapElementPtr, newTimeSlice.getStart(), newTimeSlice.getEnd(), newMapElementMiddlePtr, AvgMsperUnit, consolidate_rows, random_sampling);
			added_new = MergeTimeSliceDataIntoMap(branch, newTimeSliceLeaf, newMapElementMiddlePtr, variable_group_number, merge_mode);

			mapElementPtr = newMapElementMiddlePtr;

			newTimeSliceEatenCompletelyUp = true;

		}
		else if (newTimeSlice.IsEndEqualToRHSEnd(mapElement))
		{

			// The new time slice starts in the map element,
			// but past its left edge.
			// The new time slice ends at exactly the right edge of the map element.

			// Slice the map element into two pieces.
			// The first is unchanged.
			// The second merges with the new time slice
			// (with the right edge equal to the right edge of the map element).

			SliceMapEntry(mapElementPtr, newTimeSlice.getStart(), newMapElementLeftPtr, newMapElementRightPtr, AvgMsperUnit, consolidate_rows, random_sampling);
			added_new = MergeTimeSliceDataIntoMap(branch, newTimeSliceLeaf, newMapElementRightPtr, variable_group_number, merge_mode);

			mapElementPtr = newMapElementRightPtr;

			newTimeSliceEatenCompletelyUp = true;

		}
		else
		{

			// The new time slice starts in the map element,
			// but past its left edge.
			// The new time slice ends past the right edge of the map element.

			// Slice the right part of the map element
			// and the left part of the time slice
			// so that they are equal, and merge.

			// Then set the iterator to point to the next map element, ready for the next iteration.
			// The remainder of the new time slice (at the right) is now in the variable "newTimeSliceLeaf" and ready for the next iteration.

			SliceOffLeft(newTimeSliceLeaf, mapElement.getEnd(), new_left_slice);
			SliceMapEntry(mapElementPtr, new_left_slice.first.getStart(), newMapElementLeftPtr, newMapElementRightPtr, AvgMsperUnit, consolidate_rows, random_sampling);
			added_new = MergeTimeSliceDataIntoMap(branch, new_left_slice, newMapElementRightPtr, variable_group_number, merge_mode);

			mapElementPtr = ++newMapElementRightPtr;

		}

	}

	if (added_new)
	{
		added = true; // It could have been added previously, so never set to false
	}

	return newTimeSliceEatenCompletelyUp;

}

// breaks an existing map entry into two pieces and returns an iterator to both
void AllWeightings::SliceMapEntry(TimeSlices::iterator const & existingMapElementPtr, std::int64_t const middle, TimeSlices::iterator & newMapElementLeftPtr, TimeSlices::iterator & newMapElementRightPtr, std::int64_t const AvgMsperUnit, bool const consolidate_rows, bool const random_sampling)
{

	TimeSlice timeSlice = existingMapElementPtr->first;
	TimeSlice originalMapTimeSlice = timeSlice;
	VariableGroupTimeSliceData const timeSliceData = existingMapElementPtr->second;

	timeSlices.erase(existingMapElementPtr);

	timeSlice = originalMapTimeSlice;
	timeSlice.setEnd(middle);
	timeSlices[timeSlice] = timeSliceData;
	timeSlices[timeSlice].PruneTimeUnits(*this, originalMapTimeSlice, timeSlice, AvgMsperUnit, consolidate_rows, random_sampling);

	newMapElementLeftPtr = timeSlices.find(timeSlice);

	timeSlice = originalMapTimeSlice;
	timeSlice.setStart(middle);
	timeSlices[timeSlice] = timeSliceData;
	timeSlices[timeSlice].PruneTimeUnits(*this, originalMapTimeSlice, timeSlice, AvgMsperUnit, consolidate_rows, random_sampling);

	newMapElementRightPtr = timeSlices.find(timeSlice);

}

// breaks an existing map entry into three pieces and returns an iterator to the middle piece
void AllWeightings::SliceMapEntry(TimeSlices::iterator const & existingMapElementPtr, std::int64_t const left, std::int64_t const right, TimeSlices::iterator & newMapElementMiddlePtr, std::int64_t const AvgMsperUnit, bool const consolidate_rows, bool const random_sampling)
{

	TimeSlice timeSlice = existingMapElementPtr->first;
	TimeSlice originalMapTimeSlice = timeSlice;
	VariableGroupTimeSliceData const timeSliceData = existingMapElementPtr->second;

	timeSlices.erase(existingMapElementPtr);

	timeSlice = originalMapTimeSlice;
	timeSlice.setEnd(left);
	timeSlices[timeSlice] = timeSliceData;
	timeSlices[timeSlice].PruneTimeUnits(*this, originalMapTimeSlice, timeSlice, AvgMsperUnit, consolidate_rows, random_sampling);

	timeSlice = originalMapTimeSlice;
	timeSlice.Reshape(left, right);
	timeSlices[timeSlice] = timeSliceData;
	timeSlices[timeSlice].PruneTimeUnits(*this, originalMapTimeSlice, timeSlice, AvgMsperUnit, consolidate_rows, random_sampling);

	newMapElementMiddlePtr = timeSlices.find(timeSlice);

	timeSlice = originalMapTimeSlice;
	timeSlice.setStart(right);
	timeSlices[timeSlice] = timeSliceData;
	timeSlices[timeSlice].PruneTimeUnits(*this, originalMapTimeSlice, timeSlice, AvgMsperUnit, consolidate_rows, random_sampling);

}

// Slices off the left part of the "incoming_slice" TimeSliceLeaf and returns it in the "new_left_slice" TimeSliceLeaf.
// The "incoming_slice" TimeSliceLeaf is adjusted to become equal to the remaining part on the right.
void AllWeightings::SliceOffLeft(TimeSliceLeaf & incoming_slice, std::int64_t const slicePoint, TimeSliceLeaf & new_left_slice)
{
	new_left_slice = incoming_slice;
	new_left_slice.first.setEnd(slicePoint);
	incoming_slice.first.setStart(slicePoint);
}

// Merge time slice data into a map element
bool AllWeightings::MergeTimeSliceDataIntoMap(Branch const & branch, TimeSliceLeaf const & timeSliceLeaf, TimeSlices::iterator & mapElementPtr, int const & variable_group_number, VARIABLE_GROUP_MERGE_MODE const merge_mode)
{

	// ******************************************************************************************************************************** //
	// ******************************************************************************************************************************** //
	//
	// If we reach this function, the time slice of the leaf is guaranteed to match the time slice of the map element
	// it is being merge into.
	// We do not check this here.
	//
	// ******************************************************************************************************************************** //
	// ******************************************************************************************************************************** //

	bool added = false;

	VariableGroupTimeSliceData & variableGroupTimeSliceData = mapElementPtr->second;
	VariableGroupBranchesAndLeavesVector & variableGroupBranchesAndLeavesVector = variableGroupTimeSliceData.branches_and_leaves;

	// Note: Currently, only one primary top-level variable group is supported.
	// It will be the "begin()" element, if it exists.
	VariableGroupBranchesAndLeavesVector::iterator VariableGroupBranchesAndLeavesPtr = variableGroupBranchesAndLeavesVector.begin();

	if (VariableGroupBranchesAndLeavesPtr == variableGroupBranchesAndLeavesVector.end())
	{

		// add new variable group entry,
		// and then add new branch corresponding to this variable group

		// This case will only be hit for the primary variable group!

		if (merge_mode == VARIABLE_GROUP_MERGE_MODE__PRIMARY)
		{
			VariableGroupBranchesAndLeaves newVariableGroupBranch(variable_group_number);
			Branches & newBranchesAndLeaves = newVariableGroupBranch.branches;
			branch.InsertLeaf(timeSliceLeaf.second); // add Leaf to the set of Leaves attached to the new Branch
			newBranchesAndLeaves.insert(branch);
			variableGroupBranchesAndLeavesVector.push_back(newVariableGroupBranch);

			added = true;
		}

		else
		{
			boost::format msg("Logic error: Attempting to add a new branch when merging a non-primary variable group!");
			throw NewGeneException() << newgene_error_description(msg.str());
		}

	}
	else
	{

		// Branches already exists for this variable group.
		// The incoming branch might match one of these, or it might not.
		// In any case, retrieve the existing set of branches for this variable group.

		VariableGroupBranchesAndLeaves & variableGroupBranch = *VariableGroupBranchesAndLeavesPtr;
		Branches & branches = variableGroupBranch.branches;

		switch (merge_mode)
		{

			case VARIABLE_GROUP_MERGE_MODE__PRIMARY:
			{

				// *********************************************************************************** //
				// This is where multiple rows with duplicated primary keys 
				// and overlapping time range will be wiped out.
				// ... Only one row with given primary keys and time range is allowed
				// ... (the first one - emplace does nothing if the entry already exists).
				// ... (Note that the leaf points to a specific row of secondary data.)
				// *********************************************************************************** //
				auto branchPtr = branches.find(branch);
				if (branchPtr == branches.cend())
				{
					auto inserted = branches.insert(branch);
					branchPtr = inserted.first;
				}
				branchPtr->InsertLeaf(timeSliceLeaf.second); // add Leaf to the set of Leaves attached to the new Branch, if one doesn't already exist there

				added = true;

			}
			break;

			case VARIABLE_GROUP_MERGE_MODE__TOP_LEVEL:
			{
				
				// Let's take a peek and see if our branch is already present

				Branches::iterator branchPtr = branches.find(branch);
				if (branchPtr != branches.end())
				{
					
					// *********************************************************************************** //
					// The incoming branch *does* already exist!
					// We want to see if this branch contains the incoming leaf, or not.
					// *********************************************************************************** //

					Branch const & current_branch = *branchPtr;

					if (current_branch.doesLeafExist(timeSliceLeaf.second))
					{

						// This branch *does* contain the incoming leaf!
						// Set the data in the leaf for this non-primary top-level variable group.

						// Note that many different OUTPUT ROWS might reference this leaf;
						// perhaps even multiple times within a single output row.  Fine!

						// pass the index over from the incoming leaf (which contains only the index for the current top-level variable group being merged in)
						// into the active leaf saved in the AllWeightings instance, and used to construct the output rows.
						// (This active leaf may also have been called previously to set other top-level variable group rows.)
						current_branch.setTopGroupIndexIntoRawData(timeSliceLeaf.second, variable_group_number, timeSliceLeaf.second.other_top_level_indices_into_raw_data[variable_group_number]);

						added = true;

					}

				}

			}
			break;

			case VARIABLE_GROUP_MERGE_MODE__CHILD:
			{

				// Construct the child's DMU keys, including leaf
				ChildDMUInstanceDataVector dmu_keys;
				dmu_keys.insert(dmu_keys.end(), branch.primary_keys.begin(), branch.primary_keys.end());
				dmu_keys.insert(dmu_keys.end(), timeSliceLeaf.second.primary_keys.begin(), timeSliceLeaf.second.primary_keys.end());

				// *********************************************************************************** //
				// Loop through all BRANCHES for this variable group in this time slice.
				// For each, we have a set of output rows.
				// *********************************************************************************** //

				std::for_each(branches.begin(), branches.end(), [&](Branch const & the_current_map_branch)
				{

					// *********************************************************************************** //
					// "leaves_cache" is a vector cache containing the same leaves in the same order
					// as the official "leaves" set containing the leaves for the current branch.
					//
					// Note that a call to "ResetBranchCaches()" previous to the high-level call to "HandleBranchAndLeaf()",
					// in which we are nested, has already set the "leaves_cache" cache,
					// and this cache is copied whenever any map entry changes.
					// *********************************************************************************** //

					// The following cache will only be filled on the first pass
					the_current_map_branch.ConstructChildCombinationCache(*this, variable_group_number, false);

					// *********************************************************************************** //
					// We have an incoming child variable group branch and leaf.
					// Find all matching output rows that contain the same DMU data on the matching columns.
					// *********************************************************************************** //
					auto const & matchingOutputRows = the_current_map_branch.helper_lookup__from_child_key_set__to_matching_output_rows.find(dmu_keys);

					bool no_matches_for_this_child = false;
					if (matchingOutputRows == the_current_map_branch.helper_lookup__from_child_key_set__to_matching_output_rows.cend())
					{
						no_matches_for_this_child = true;
					}

					// Loop through all matching output rows
					if (!no_matches_for_this_child)
					{
						for (auto matchingOutputRowPtr = matchingOutputRows->second.cbegin(); matchingOutputRowPtr != matchingOutputRows->second.cend(); ++matchingOutputRowPtr)
						{

							BranchOutputRow const & outputRow = *matchingOutputRowPtr->first;
							std::vector<int> const & matchingOutputChildLeaves = matchingOutputRowPtr->second;

							// Loop through all matching output row child leaves
							std::for_each(matchingOutputChildLeaves.cbegin(), matchingOutputChildLeaves.cend(), [&](int const matching_child_leaf_index)
							{

								auto const found = outputRow.child_indices_into_raw_data.find(variable_group_number);
								if (found == outputRow.child_indices_into_raw_data.cend())
								{
									outputRow.child_indices_into_raw_data[variable_group_number] = std::map<int, std::int64_t>();
								}
								auto & outputRowLeafIndexToSecondaryDataCacheIndex = outputRow.child_indices_into_raw_data[variable_group_number];
								outputRowLeafIndexToSecondaryDataCacheIndex[matching_child_leaf_index] = timeSliceLeaf.second.index_into_raw_data;

								added = true;

							});

						}
					}

				});

			}
			break;

			default:
			{

			}
			break;

		}

	}

	return added;

}

void AllWeightings::CalculateWeightings(int const K, std::int64_t const ms_per_unit_time)
{

	boost::multiprecision::cpp_int currentWeighting = 0;

	std::int64_t branch_count = 0;
	std::int64_t time_slice_count = 0;

	std::for_each(timeSlices.begin(), timeSlices.end(), [&](std::pair<TimeSlice const, VariableGroupTimeSliceData> & timeSliceEntry)
	{

		++time_slice_count;

		TimeSlice const & timeSlice = timeSliceEntry.first;
		VariableGroupTimeSliceData & variableGroupTimeSliceData = timeSliceEntry.second;
		VariableGroupBranchesAndLeavesVector & variableGroupBranchesAndLeavesVector = variableGroupTimeSliceData.branches_and_leaves;
		Weighting & variableGroupTimeSliceDataWeighting = variableGroupTimeSliceData.weighting;

		if (variableGroupBranchesAndLeavesVector.size() > 1)
		{
			boost::format msg("Only one top-level variable group is currently supported for the random / full sampler.");
			throw NewGeneException() << newgene_error_description(msg.str());
		}

		variableGroupTimeSliceDataWeighting.setWeightingRangeStart(currentWeighting);

		// We know there's only one variable group currently supported, but include the loop as a reminder that
		// we may support multiple variable groups in the random sampler in the future.
		std::for_each(variableGroupBranchesAndLeavesVector.begin(), variableGroupBranchesAndLeavesVector.end(), [&](VariableGroupBranchesAndLeaves & variableGroupBranchesAndLeaves)
		{
			
			Branches & branchesAndLeaves = variableGroupBranchesAndLeaves.branches;
			Weighting & variableGroupBranchesAndLeavesWeighting = variableGroupBranchesAndLeaves.weighting;
			variableGroupBranchesAndLeavesWeighting.setWeightingRangeStart(currentWeighting);

			std::for_each(branchesAndLeaves.begin(), branchesAndLeaves.end(), [&](Branch const & branch)
			{

				++branch_count;

				//Leaves & leaves = branchAndLeaves.second;

				Weighting & branchWeighting = branch.weighting;

				// Count the leaves
				int numberLeaves = static_cast<int>(branch.numberLeaves());

				// The number of K-ad combinations for this branch is easily calculated.
				// It is just the binomial coefficient (assuming K <= N)

				branch.number_branch_combinations = 1; // covers K > numberLeaves condition, and numberLeaves == 0 condition
				if (K <= numberLeaves)
				{
					//branch.number_branch_combinations = boost::math::binomial_coefficient<boost::multiprecision::cpp_int>(numberLeaves, K);
					branch.number_branch_combinations = BinomialCoefficient(numberLeaves, K);
				}

				// clear the hits cache
				branch.hits.clear();
				
				// Holes between time slices are handled here, as well as the standard case of no holes between time slices -
				// There is no gap in the sequence of discretized weight values in branches.
				branchWeighting.setWeighting(timeSlice.WidthForWeighting(ms_per_unit_time) * branch.number_branch_combinations);
				branchWeighting.setWeightingRangeStart(currentWeighting);
				currentWeighting += branchWeighting.getWeighting();

				//boost::format msg("New weighting being added for branch: %1%");
				//msg % boost::lexical_cast<std::string>(weighting.getWeighting());
				//messager.AppendKadStatusText(msg.str(), this);

				variableGroupBranchesAndLeavesWeighting.addWeighting(branchWeighting.getWeighting());

			});

			variableGroupTimeSliceDataWeighting.addWeighting(variableGroupBranchesAndLeavesWeighting.getWeighting());

		});

		weighting.addWeighting(variableGroupTimeSliceDataWeighting.getWeighting());

	});

	boost::format msg("%1% time slices and %2% branches");
	msg % time_slice_count % branch_count;
	messager.AppendKadStatusText(msg.str(), this);

}

void AllWeightings::AddNewTimeSlice(int const & variable_group_number, Branch const & branch, TimeSliceLeaf const & newTimeSliceLeaf)
{
	VariableGroupTimeSliceData variableGroupTimeSliceData;
	VariableGroupBranchesAndLeavesVector & variableGroupBranchesAndLeavesVector = variableGroupTimeSliceData.branches_and_leaves;
	VariableGroupBranchesAndLeaves newVariableGroupBranch(variable_group_number);
	Branches & newBranchesAndLeaves = newVariableGroupBranch.branches;
	newBranchesAndLeaves.insert(branch);
	Branch const & current_branch = *(newBranchesAndLeaves.find(branch));
	current_branch.InsertLeaf(newTimeSliceLeaf.second); // add Leaf to the set of Leaves attached to the new Branch
	variableGroupBranchesAndLeavesVector.push_back(newVariableGroupBranch);
	timeSlices[newTimeSliceLeaf.first] = variableGroupTimeSliceData;
}

void AllWeightings::PrepareRandomNumbers(std::int64_t how_many)
{

	if (weighting.getWeighting() < 1)
	{
		boost::format msg("There is no data from which the sampler can obtain rows.");
		throw NewGeneException() << newgene_error_description(msg.str());
	}

	if (how_many < 1)
	{
		boost::format msg("The number of desired rows is zero.  There is nothing for the sampler to do, and no output will be generated.");
		throw NewGeneException() << newgene_error_description(msg.str());
	}

	random_numbers.clear();
	boost::random::mt19937 engine(static_cast<std::int32_t>(std::time(0)));
	boost::random::uniform_int_distribution<boost::multiprecision::cpp_int> distribution(weighting.getWeightingRangeStart(), weighting.getWeightingRangeEnd());

	// Check out this clean, simple approach:
	// The available row samples are discretely numbered.
	// So duplicates are rejected here, guaranteeing unbiased
	// choice of branches (each branch corresponding to a
	// window of discrete values), including no-replacement.
	bool reverse_mode = false;
	std::vector<boost::multiprecision::cpp_int> remaining;
	while (random_numbers.size() < static_cast<size_t>(how_many))
	{

		// Check if we've consumed over 50% of the random numbers available
		if (reverse_mode)
		{

			if (remaining.empty())
			{
				// TODO: warning
				//boost::format msg("Too many output rows have been requested for the given data set.");
				//throw NewGeneException() << newgene_error_description(msg.str());
				break;
			}

			std::uniform_int_distribution<size_t> remaining_distribution(0, remaining.size() - 1);
			size_t which_remaining_random_number = remaining_distribution(engine);

			random_numbers.insert(remaining[which_remaining_random_number]);

			auto remainingPtr = remaining.begin() + which_remaining_random_number;
			remaining.erase(remainingPtr);

		}
		else
		if (boost::multiprecision::cpp_int(random_numbers.size()) > weighting.getWeighting() / 2)
		{
			// Over 50% of the available numbers are already consumed.
			// It must be a "somewhat" small number of available numbers.

			// Begin pulling numbers randomly from the remaining set available.
			for (boost::multiprecision::cpp_int n = 0; n < weighting.getWeighting(); ++n)
			{
				if (random_numbers.count(n) == 0)
				{
					remaining.push_back(n);
				}
			}

			// Prepare reverse mode, but do not populate a new random number
			reverse_mode = true;
		}
		else
		{
			random_numbers.insert(distribution(engine));
		}

	}

	random_number_iterator = random_numbers.cbegin();

}

bool AllWeightings::RetrieveNextBranchAndLeaves(int const K)
{
	
	if (random_number_iterator == random_numbers.cend())
	{
		return false;
	}

	boost::multiprecision::cpp_int const & random_number = *random_number_iterator;

#	ifdef _DEBUG
	std::string val1 = random_number.str();
#	endif

	BOOST_ASSERT_MSG(random_number >= 0 && random_number < weighting.getWeighting() && weighting.getWeightingRangeStart() == 0 && weighting.getWeightingRangeEnd() == weighting.getWeighting() - 1, "Invalid weights in RetrieveNextBranchAndLeaves().");

	TimeSlices::const_iterator timeSlicePtr = std::lower_bound(timeSlices.cbegin(), timeSlices.cend(), random_number, [&](std::pair<TimeSlice const, VariableGroupTimeSliceData> const & timeSliceData, boost::multiprecision::cpp_int const & test_random_number)
	{
		VariableGroupTimeSliceData const & testVariableGroupTimeSliceData = timeSliceData.second;
		if (testVariableGroupTimeSliceData.weighting.getWeightingRangeEnd() < test_random_number)
		{
			return true;
		}
		return false;
	});

	TimeSlice const & timeSlice = timeSlicePtr->first;
	VariableGroupTimeSliceData const & variableGroupTimeSliceData = timeSlicePtr->second;
	VariableGroupBranchesAndLeavesVector const & variableGroupBranchesAndLeavesVector = variableGroupTimeSliceData.branches_and_leaves;

	// For now, assume only one variable group
	if (variableGroupBranchesAndLeavesVector.size() > 1)
	{
		boost::format msg("Only one top-level variable group is currently supported for the random and full sampler.");
		throw NewGeneException() << newgene_error_description(msg.str());
	}

	VariableGroupBranchesAndLeaves const & variableGroupBranchesAndLeaves = variableGroupBranchesAndLeavesVector[0];

	Branches const & branches = variableGroupBranchesAndLeaves.branches;
		
	// Pick a branch randomly (with weight!)
	Branches::const_iterator branchesPtr = std::lower_bound(branches.cbegin(), branches.cend(), random_number, [&](Branch const & testBranch, boost::multiprecision::cpp_int const & test_random_number)
	{
		if (testBranch.weighting.getWeightingRangeEnd() < test_random_number)
		{
			return true;
		}
		return false;
	});

	const Branch & new_branch = *branchesPtr;

	// random_number is now an actual *index* to which combination of leaves in this VariableGroupTimeSliceData
	GenerateOutputRow(random_number, K, new_branch);

	++random_number_iterator;

	return true;

}

void AllWeightings::GenerateAllOutputRows(int const K, Branch const & branch)
{

	boost::multiprecision::cpp_int which_time_unit = -1;  // -1 means "full sampling for branch" - no need to break down into time units (which have identical full sets)

	branch.hits[which_time_unit].clear();
	branch.remaining[which_time_unit].clear();

	static int saved_range = -1;

	BranchOutputRow single_leaf_combination;

	bool skip = false;
	if (K >= branch.numberLeaves())
	{
		skip = true;
		for (int n = 0; n < static_cast<int>(branch.numberLeaves()); ++n)
		{
			single_leaf_combination.Insert(n);
		}
		branch.remaining[which_time_unit].push_back(single_leaf_combination);
	}

	if (K <= 0)
	{
		return;
	}

	if (!skip)
	{
		PopulateAllLeafCombinations(which_time_unit, K, branch);
	}

	branch.hits[which_time_unit].insert(branch.remaining[which_time_unit].begin(), branch.remaining[which_time_unit].end());
	branch.remaining[which_time_unit].clear();

}

void AllWeightings::GenerateOutputRow(boost::multiprecision::cpp_int random_number, int const K, Branch const & branch)
{

	random_number -= branch.weighting.getWeightingRangeStart();

	boost::multiprecision::cpp_int which_time_unit = -1;
	if (time_granularity != TIME_GRANULARITY__NONE)
	{
		which_time_unit = random_number / branch.number_branch_combinations;
	}
	else
	{
		BOOST_ASSERT_MSG(random_number < branch.number_branch_combinations, "Generated random number is greater than the number of branch combinations when the time granularity is none!");
	}

	static int saved_range = -1;
	static std::mt19937 engine(static_cast<std::int32_t>(std::time(0)));

	BranchOutputRow test_leaf_combination;

	bool skip = false;
	if (K >= branch.numberLeaves())
	{
		skip = true;
		for (int n = 0; n < branch.numberLeaves(); ++n)
		{
			test_leaf_combination.Insert(n);
		}
	}

	if (K <= 0)
	{
		return;
	}

	if (!skip)
	{

		BOOST_ASSERT_MSG(boost::multiprecision::cpp_int(branch.hits[which_time_unit].size()) < branch.number_branch_combinations, "The number of hits is as large as the number of combinations for a branch.  Invalid!");

		// skip any leaf combinations returned from previous random numbers - count will be 1 if previously hit for this time unit
		// THIS is where random selection WITH REMOVAL is implemented (along with the fact that the random numbers generated are also with removal)
		while (test_leaf_combination.Empty() || branch.hits[which_time_unit].count(test_leaf_combination))
		{

			test_leaf_combination.Clear();

			if (boost::multiprecision::cpp_int(branch.hits[which_time_unit].size()) > branch.number_branch_combinations / 2)
			{

				// There are so many requests that it is more efficient to populate a list with all the remaining possibilities,
				// and then pick randomly from that

				// A previous call may have populated "remaining"

				if (branch.remaining[which_time_unit].size() == 0)
				{
					PopulateAllLeafCombinations(which_time_unit, K, branch);
				}

				std::uniform_int_distribution<size_t> distribution(0, branch.remaining[which_time_unit].size() - 1);
				size_t which_remaining_leaf_combination = distribution(engine);

				test_leaf_combination = branch.remaining[which_time_unit][which_remaining_leaf_combination];
				auto remainingPtr = branch.remaining[which_time_unit].begin() + which_remaining_leaf_combination;

				branch.remaining[which_time_unit].erase(remainingPtr);

			}
			else
			{

				std::vector<int> remaining_leaves;
				for (int n = 0; n < branch.numberLeaves(); ++n)
				{
					remaining_leaves.push_back(n);
				}

				// Pull random leaves, one at a time, to create the random row
				while (test_leaf_combination.Size() < static_cast<size_t>(K))
				{

					// ************************************************************************ //
					// TODO: 
					// This could be optimized in case K is high
					// and the number of leaves is just a little larger than K
					// ************************************************************************ //

					std::uniform_int_distribution<size_t> distribution(0, remaining_leaves.size() - 1);
					size_t index_of_index = distribution(engine);
					int index_of_leaf = remaining_leaves[index_of_index];
					auto remainingPtr = remaining_leaves.begin() + index_of_index;
					remaining_leaves.erase(remainingPtr);
					test_leaf_combination.Insert(index_of_leaf);
				}

			}

		}

	}

#	ifdef _DEBUG
	std::for_each(test_leaf_combination.primary_leaves.cbegin(), test_leaf_combination.primary_leaves.cend(), [&](int const & index)
	{
		if (index >= branch.numberLeaves())
		{
			boost::format msg("Attempting to generate an output row whose leaf indexes point outside the range of available leaves for the given branch!");
			throw NewGeneException() << newgene_error_description(msg.str());
		}
	});
#	endif

	// It might easily be a duplicate - random sampling will produce multiple hits on the same row
	// because some rows can have a heavier weight than other rows;
	// this is handled by storing a map of every *time unut* (corresponding to the primary variable group)
	// and all leaf combinations that have been hit for that time unit.
	++random_rows_added;
	branch.hits[which_time_unit].insert(test_leaf_combination);

}

void AllWeightings::PopulateAllLeafCombinations(boost::multiprecision::cpp_int const & which_time_unit, int const K, Branch const & branch)
{

	boost::multiprecision::cpp_int total_added = 0;

	branch.remaining.clear();
	std::vector<int> position;
	for (int n = 0; n < K; ++n)
	{
		position.push_back(n);
	}

	while (total_added < branch.number_branch_combinations)
	{

		AddPositionToRemaining(which_time_unit, position, branch);
		bool succeeded = IncrementPosition(K, position, branch);

		BOOST_ASSERT_MSG(succeeded || (total_added + 1) == branch.number_branch_combinations, "Invalid logic in position incrementer in sampler!");

		++total_added;

	}	

}

void AllWeightings::AddPositionToRemaining(boost::multiprecision::cpp_int const & which_time_unit, std::vector<int> const & position, Branch const & branch)
{

	BranchOutputRow new_remaining;
	std::for_each(position.cbegin(), position.cend(), [&](int const position_index)
	{
		new_remaining.Insert(position_index);
	});

	if (branch.hits[which_time_unit].count(new_remaining) == 0)
	{
		branch.remaining[which_time_unit].push_back(new_remaining);
	}

}

bool AllWeightings::IncrementPosition(int const K, std::vector<int> & position, Branch const & branch)
{

	int sub_k_being_managed = K;
	
	int new_leaf = IncrementPositionManageSubK(K, sub_k_being_managed, position, branch);

	if (new_leaf == -1)
	{
		// No more positions!
		return false;
	}

	return true;

}

int AllWeightings::IncrementPositionManageSubK(int const K, int const subK_, std::vector<int> & position, Branch const & branch)
{

	int number_leaves = static_cast<int>(branch.numberLeaves());

	int max_leaf_for_subk = number_leaves - 1 - (K - subK_); // this is the highest possible value of the leaf index that the given subK can be pointing to.
	// i.e., if there are 10 leaves and K=3,
	// 'positions' will have a size of three, with the 'lowest' position = {0,1,2}
	// and the 'highest' position = {7,8,9}.
	// If subK = K (=3), this corresponds to the final slot which has 9 as its highest allowed value (it can point at the last leaf).
	// But if subK = K-1, the highest leaf it can point to is 8.
	// Etc.

	int const subK = subK_ - 1; // make it 0-based

	int & index_being_managed_value = position[subK];
	if (index_being_managed_value == max_leaf_for_subk)
	{
		if (subK == 0)
		{
			// All done!
			return -1;
		}
		int previous_k_new_leaf = IncrementPositionManageSubK(K, subK, position, branch);
		if (previous_k_new_leaf == -1)
		{
			// All done, as reported by nested call!
			return -1;
		}
		index_being_managed_value = previous_k_new_leaf; // we will point to the leaf one higher than the leaf to which the previous index points
	}

	++index_being_managed_value;

	return index_being_managed_value;

}

boost::multiprecision::cpp_int AllWeightings::BinomialCoefficient(int const N, int const K)
{

	if (N == 0)
	{
		return 0;
	}

	if (K >= N)
	{
		return 1;
	}

	// Goddamn boost::multiprecision under no circumstances will allow an integer calculation of the binomial coefficient

	boost::multiprecision::cpp_int numerator{ 1 };
	boost::multiprecision::cpp_int denominator{ 1 };

	for (int n = N; n > N - K; --n)
	{
		numerator *= boost::multiprecision::cpp_int(n);
	}

	for (int n = 1; n <= K; ++n)
	{
		denominator *= boost::multiprecision::cpp_int(n);
	}

	return numerator / denominator;

}

void AllWeightings::ResetBranchCaches()
{

	std::for_each(timeSlices.begin(), timeSlices.end(), [&](std::pair<TimeSlice const, VariableGroupTimeSliceData> & timeSliceData)
	{

		timeSliceData.second.ResetBranchCachesSingleTimeSlice(*this);

	});

}

void PrimaryKeysGroupingMultiplicityOne::ConstructChildCombinationCache(AllWeightings & allWeightings, int const variable_group_number, bool const force) const
{

	// ************************************************************************************************************************************************** //
	// This function builds a cache:
	// A single map for this entire branch that 
	// maps a CHILD primary key set (i.e., a single row of child data,
	// including the child's branch and the 0 or 1 child leaf)
	// to a particular output row in a particular time unit in this branch,
	// for rapid lookup later.  The map also includes the *child* leaf number in the *output*
	// for the given row.
	// (I.e., the *input* is just a single child row of data with at most only one child leaf,
	//  but this single row of monadic child input data maps to a particular k-value
	//  in the corresponding output row, because child data can appear multiple times
	//  for a single output row.)
	// ************************************************************************************************************************************************** //

	if (force || helper_lookup__from_child_key_set__to_matching_output_rows.empty())
	{

		// The cache has yet to be filled, or we are specifically being requested to refresh it

		helper_lookup__from_child_key_set__to_matching_output_rows.clear();

		ChildDMUInstanceDataVector child_hit_vector_branch_components;
		ChildDMUInstanceDataVector child_hit_vector;
		std::for_each(hits.cbegin(), hits.cend(), [&](decltype(hits)::value_type const & time_unit_output_rows)
		{

			// ***************************************************************************************** //
			// We have one time unit entry within this time slice.
			// We proceed to build the cache
			// ***************************************************************************************** //

			for (auto outputRowPtr = time_unit_output_rows.second.cbegin(); outputRowPtr != time_unit_output_rows.second.cend(); ++outputRowPtr)
			{

				// ******************************************************************************************************** //
				// We have a new hit we're dealing with
				// ******************************************************************************************************** //

				BranchOutputRow const & outputRow = *outputRowPtr;

				child_hit_vector_branch_components.clear();

				// Some child leaves map, in part, to branch leaves of the top-level UOA.
				// If there is no leaf data for these leaves (which can happen in the K>N case),
				// there cannot possibly be a match for any of the child leaves for this output row.
				// Set the following flag to capture this case.
				bool branch_component_bad = false;

				// First in the "child DMU" vector are the child's BRANCH DMU values
				std::for_each(allWeightings.mappings_from_child_branch_to_primary[variable_group_number].cbegin(), allWeightings.mappings_from_child_branch_to_primary[variable_group_number].cend(), [&](ChildToPrimaryMapping const & childToPrimaryMapping)
				{

					// We have the next DMU data in the sequence of DMU's for the child branch/leaf (we're working on the branch)

					switch (childToPrimaryMapping.mapping)
					{

						case CHILD_TO_PRIMARY_MAPPING__MAPS_TO_BRANCH:
						{

							// The next DMU in the child branch's DMU sequence maps to a branch in the top-level DMU sequence
							child_hit_vector_branch_components.push_back(DMUInstanceData(primary_keys[childToPrimaryMapping.index_of_column_within_top_level_branch_or_single_leaf]));

						}
						break;

						case CHILD_TO_PRIMARY_MAPPING__MAPS_TO_LEAF:
						{

							// leaf_number tells us which leaf
							// index tells us which index in that leaf

							// The next DMU in the child branch's DMU sequence maps to a leaf in the top-level DMU sequence

							if (childToPrimaryMapping.leaf_number_in_top_level_group__only_applicable_when_child_key_column_points_to_top_level_column_that_is_in_top_level_leaf >= static_cast<int>(outputRow.primary_leaves_cache.size()))
							{
								branch_component_bad = true;
								break;
							}

							if (leaves_cache[outputRow.primary_leaves_cache[childToPrimaryMapping.leaf_number_in_top_level_group__only_applicable_when_child_key_column_points_to_top_level_column_that_is_in_top_level_leaf]].primary_keys.size() == 0)
							{
								// This is the K=1 case - the matching leaf of the *top-level* UOA
								// has no primary keys.  This is a logic error, as we should never match
								// a "leaf" in the top-level UOA in this case.
								//
								// To confirm this is a legitimate logic error, see "OutputModel::OutputGenerator::RandomSamplerFillDataForChildGroups()",
								// in particular the following lines:
								// --> // if (full_kad_key_info.total_outer_multiplicity__for_the_current_dmu_category__corresponding_to_the_uoa_corresponding_to_top_level_variable_group == 1)
								// --> // {
								// --> //     is_current_index_a_top_level_primary_group_branch = true;
								// --> // }

								boost::format msg("Logic error: attempting to match child branch data to a leaf in the top-level unit of analysis when K=1.  There can be no leaves when K=1.");
								throw NewGeneException() << newgene_error_description(msg.str());
							}

							child_hit_vector_branch_components.push_back(DMUInstanceData(leaves_cache[outputRow.primary_leaves_cache[childToPrimaryMapping.leaf_number_in_top_level_group__only_applicable_when_child_key_column_points_to_top_level_column_that_is_in_top_level_leaf]].primary_keys[childToPrimaryMapping.index_of_column_within_top_level_branch_or_single_leaf]));

						}
						break;

						default:
						{}
						break;

					}

				});

				if (branch_component_bad)
				{
					// No luck for this output row for any child leaf.
					// Try the next row.
					continue;
				}

				// Next in the "child DMU" vector are the child's LEAF DMU values
				int child_leaf_index_crossing_multiple_child_leaves = 0;
				int child_leaf_index_within_a_single_child_leaf = 0;
				int current_child_leaf_number = 0;
				bool missing_top_level_leaf = false;
				child_hit_vector.clear();
				child_hit_vector.insert(child_hit_vector.begin(), child_hit_vector_branch_components.begin(), child_hit_vector_branch_components.end());
				std::for_each(allWeightings.mappings_from_child_leaf_to_primary[variable_group_number].cbegin(), allWeightings.mappings_from_child_leaf_to_primary[variable_group_number].cend(), [&](ChildToPrimaryMapping const & childToPrimaryMapping)
				{

					// We have the next DMU data in the sequence of DMU's for the child branch/leaf (we're working on the leaf)

					switch (childToPrimaryMapping.mapping)
					{

						case CHILD_TO_PRIMARY_MAPPING__MAPS_TO_BRANCH:
						{

							// The next DMU in the child leaf's DMU sequence maps to a branch in the top-level DMU sequence
							child_hit_vector.push_back(DMUInstanceData(primary_keys[childToPrimaryMapping.index_of_column_within_top_level_branch_or_single_leaf]));

						}
						break;

						case CHILD_TO_PRIMARY_MAPPING__MAPS_TO_LEAF:
						{

							// leaf_number tells us which leaf
							// index tells us which index in that leaf

							// The next DMU in the child leaf's DMU sequence maps to a leaf in the top-level DMU sequence

							if (childToPrimaryMapping.leaf_number_in_top_level_group__only_applicable_when_child_key_column_points_to_top_level_column_that_is_in_top_level_leaf >= static_cast<int>(outputRow.primary_leaves_cache.size()))
							{
								// The current child leaf maps to a top-level leaf that has no data.
								// We therefore cannot match.
								missing_top_level_leaf = true;
								break;
							}

							if (outputRow.primary_leaves_cache[childToPrimaryMapping.leaf_number_in_top_level_group__only_applicable_when_child_key_column_points_to_top_level_column_that_is_in_top_level_leaf] >= static_cast<int>(leaves_cache.size()))
							{
								boost::format msg("Logic error: Output rows saved with the branch point to leaf indexes that do not exist in the branch!");
								throw NewGeneException() << newgene_error_description(msg.str());
							}

							if (leaves_cache[outputRow.primary_leaves_cache[childToPrimaryMapping.leaf_number_in_top_level_group__only_applicable_when_child_key_column_points_to_top_level_column_that_is_in_top_level_leaf]].primary_keys.size() == 0)
							{
								// This is the K=1 case - the matching leaf of the *top-level* UOA
								// has no primary keys.  This is a logic error, as we should never match
								// a "leaf" in the top-level UOA in this case.
								//
								// To confirm this is a legitimate logic error, see "OutputModel::OutputGenerator::RandomSamplerFillDataForChildGroups()",
								// in particular the following lines:
								// --> // if (full_kad_key_info.total_outer_multiplicity__for_the_current_dmu_category__corresponding_to_the_uoa_corresponding_to_top_level_variable_group == 1)
								// --> // {
								// --> //     is_current_index_a_top_level_primary_group_branch = true;
								// --> // }

								boost::format msg("Logic error: attempting to match child leaf data to a leaf in the top-level unit of analysis when K=1");
								throw NewGeneException() << newgene_error_description(msg.str());
							}

							child_hit_vector.push_back(DMUInstanceData(leaves_cache[outputRow.primary_leaves_cache[childToPrimaryMapping.leaf_number_in_top_level_group__only_applicable_when_child_key_column_points_to_top_level_column_that_is_in_top_level_leaf]].primary_keys[childToPrimaryMapping.index_of_column_within_top_level_branch_or_single_leaf]));

						}
						break;

						default:
						{}
						break;

					}

					++child_leaf_index_crossing_multiple_child_leaves;
					++child_leaf_index_within_a_single_child_leaf;

					if (child_leaf_index_within_a_single_child_leaf == allWeightings.childInternalToOneLeafColumnCountForDMUWithMultiplicityGreaterThan1[variable_group_number])
					{
						if (!missing_top_level_leaf)
						{
							helper_lookup__from_child_key_set__to_matching_output_rows[child_hit_vector][&outputRow].push_back(current_child_leaf_number);
						}
						++current_child_leaf_number;
						child_leaf_index_within_a_single_child_leaf = 0;
						missing_top_level_leaf = false;
						child_hit_vector.clear();
						child_hit_vector.insert(child_hit_vector.begin(), child_hit_vector_branch_components.begin(), child_hit_vector_branch_components.end());
					}

				});

			}

		});

	}

}

void AllWeightings::PrepareRandomSamples(int const K)
{

	while (RetrieveNextBranchAndLeaves(K))
	{
		// It's all happening in the condition
	}

}

void AllWeightings::PrepareFullSamples(int const K)
{

	std::for_each(timeSlices.cbegin(), timeSlices.cend(), [&](decltype(timeSlices)::value_type const & timeSlice)
	{

		TimeSlice const & the_slice = timeSlice.first;
		VariableGroupTimeSliceData const & variableGroupTimeSliceData = timeSlice.second;
		VariableGroupBranchesAndLeavesVector const & variableGroupBranchesAndLeaves = variableGroupTimeSliceData.branches_and_leaves;

		std::for_each(variableGroupBranchesAndLeaves.cbegin(), variableGroupBranchesAndLeaves.cend(), [&](VariableGroupBranchesAndLeaves const & variableGroupBranchesAndLeaves)
		{
			std::for_each(variableGroupBranchesAndLeaves.branches.cbegin(), variableGroupBranchesAndLeaves.branches.cend(), [&](Branch const & branch)
			{
				GenerateAllOutputRows(K, branch);
			});

		});

	});

}

void AllWeightings::ConsolidateRowsWithinBranch(Branch const & branch, int & orig_random_number_rows)
{

	if (time_granularity == TIME_GRANULARITY__NONE)
	{
		// The data is already stored in a single time unit within this branch at index -1
		return;
	}

	branch.hits[-1].clear();

	std::for_each(branch.hits.begin(), branch.hits.end(), [&](decltype(branch.hits)::value_type & hit)
	{
		if (hit.first != -1)
		{
			orig_random_number_rows += hit.second.size();
			branch.hits[-1].insert(hit.second.cbegin(), hit.second.cend());
			hit.second.clear();
		}
	});

	branch.hits.erase(++branch.hits.begin(), branch.hits.end());

	for (int c = 0; c < numberChildVariableGroups; ++c)
	{
		branch.ConstructChildCombinationCache(*this, c, true);
	}

}

void VariableGroupTimeSliceData::ResetBranchCachesSingleTimeSlice(AllWeightings & allWeightings)
{

	VariableGroupBranchesAndLeavesVector & variableGroupBranchesAndLeavesVector = branches_and_leaves;

	// For now, assume only one variable group
	if (variableGroupBranchesAndLeavesVector.size() > 1)
	{
		boost::format msg("Only one top-level variable group is currently supported for the random and full sampler in ConsolidateHits().");
		throw NewGeneException() << newgene_error_description(msg.str());
	}

	VariableGroupBranchesAndLeaves & variableGroupBranchesAndLeaves = variableGroupBranchesAndLeavesVector[0];
	Branches & branchesAndLeaves = variableGroupBranchesAndLeaves.branches;

	std::for_each(branchesAndLeaves.begin(), branchesAndLeaves.end(), [&](Branch const & branch)
	{

		branch.helper_lookup__from_child_key_set__to_matching_output_rows.clear();
		branch.ResetLeafCache();
		for (int c = 0; c < allWeightings.numberChildVariableGroups; ++c)
		{
			branch.ConstructChildCombinationCache(allWeightings, c, true);
		}

	});
}

BranchOutputRow::BranchOutputRow()
{
}

BranchOutputRow::BranchOutputRow(BranchOutputRow const & rhs)
: primary_leaves(rhs.primary_leaves)
, child_indices_into_raw_data(rhs.child_indices_into_raw_data)
{
	SaveCache();
}

BranchOutputRow::BranchOutputRow(BranchOutputRow && rhs)
: primary_leaves(std::move(rhs.primary_leaves))
, child_indices_into_raw_data(rhs.child_indices_into_raw_data)
{
	SaveCache();
}

BranchOutputRow & BranchOutputRow::operator=(BranchOutputRow const & rhs)
{
	if (this == &rhs)
	{
		return *this;
	}
	primary_leaves = rhs.primary_leaves;
	child_indices_into_raw_data = rhs.child_indices_into_raw_data;
	SaveCache();
	return *this;
}

BranchOutputRow & BranchOutputRow::operator = (BranchOutputRow && rhs)
{
	if (this == &rhs)
	{
		return *this;
	}
	primary_leaves = std::move(rhs.primary_leaves);
	child_indices_into_raw_data = std::move(rhs.child_indices_into_raw_data);
	SaveCache();
	return *this;
}

BranchOutputRow::~BranchOutputRow()
{
}

//#ifdef _DEBUG
void SpitTimeSlice(std::string & sdata, TimeSlice const & time_slice)
{

	bool startinf = false;
	bool endinf = false;

	sdata += "<TIMESTAMP_START>";
	if (!time_slice.hasTimeGranularity())
	{
		if (time_slice.startsAtNegativeInfinity())
		{
			sdata += "NEG_INF";
			startinf = true;
		}
	}
	if (!startinf)
	{
		sdata += boost::lexical_cast<std::string>(time_slice.getStart());
	}
	sdata += "</TIMESTAMP_START>";

	sdata += "<TIMESTAMP_END>";
	if (!time_slice.hasTimeGranularity())
	{
		if (time_slice.endsAtPlusInfinity())
		{
			sdata += "INF";
			endinf = true;
		}
	}
	if (!endinf)
	{
		sdata += boost::lexical_cast<std::string>(time_slice.getEnd());
	}
	sdata += "</TIMESTAMP_END>";

	sdata += "<DATETIME_START>";
	if (!startinf)
	{
		sdata += TimeRange::convertTimestampToString(time_slice.getStart());
	}
	sdata += "</DATETIME_START>";

	sdata += "<DATETIME_END>";
	if (!endinf)
	{
		sdata += TimeRange::convertTimestampToString(time_slice.getEnd());
	}
	sdata += "</DATETIME_END>";

}

void SpitKeys(std::string & sdata, std::vector<DMUInstanceData> const & dmu_keys)
{
	int index = 0;
	sdata += "<DATA_VALUES>";
	std::for_each(dmu_keys.cbegin(), dmu_keys.cend(), [&](DMUInstanceData const & data)
	{
		sdata += "<DATA_VALUE>";
		sdata += "<DATA_VALUE_INDEX>";
		sdata += boost::lexical_cast<std::string>(index);
		sdata += "</DATA_VALUE_INDEX>";
		sdata += "<DATA>";
		sdata += boost::lexical_cast<std::string>(data);
		sdata += "</DATA>";
		++index;
		sdata += "</DATA_VALUE>";
	});
	sdata += "</DATA_VALUES>";
}

void SpitDataCache(std::string & sdata, DataCache const & dataCache)
{
	sdata += "<DATA_CACHE>";
	std::for_each(dataCache.cbegin(), dataCache.cend(), [&](std::pair<std::int64_t const, SecondaryInstanceDataVector> const & dataEntry)
	{
		sdata += "<DATA_CACHE_ELEMENT>";
		sdata += "<INDEX_WITHIN_DATA_CACHE>";
		sdata += boost::lexical_cast<std::string>(dataEntry.first);
		sdata += "</INDEX_WITHIN_DATA_CACHE>";
		sdata += "<DATA_VALUES_WITHIN_DATA_CACHE>";
		SpitKeys(sdata, dataEntry.second);
		sdata += "</DATA_VALUES_WITHIN_DATA_CACHE>";
		sdata += "</DATA_CACHE_ELEMENT>";
	});
	sdata += "</DATA_CACHE>";
}

void SpitDataCaches(std::string & sdata, std::map<int, DataCache> const & dataCaches)
{
	sdata += "<DATA_CACHES>";
	std::for_each(dataCaches.cbegin(), dataCaches.cend(), [&](std::pair<int const, DataCache> const & dataEntry)
	{
		sdata += "<DATA_CACHE_NUMBER>";
		sdata += boost::lexical_cast<std::string>(dataEntry.first);
		sdata += "</DATA_CACHE_NUMBER>";
		SpitDataCache(sdata, dataEntry.second);
	});
	sdata += "</DATA_CACHES>";
}

void SpitHits(std::string & sdata, std::map<boost::multiprecision::cpp_int, std::set<BranchOutputRow>> const & hits)
{
	sdata += "<HITS>";
	std::for_each(hits.cbegin(), hits.cend(), [&](std::pair<boost::multiprecision::cpp_int const, std::set<BranchOutputRow>> const & hitsEntry)
	{
		sdata += "<TIME_UNIT>";
		sdata += "<TIME_UNIT_INDEX>";
		sdata += boost::lexical_cast<std::string>(hitsEntry.first);
		sdata += "</TIME_UNIT_INDEX>";
		sdata += "<OUTPUT_ROWS>";
		SpitSetOfOutputRows(sdata, hitsEntry.second);
		sdata += "</OUTPUT_ROWS>";
		sdata += "</TIME_UNIT>";
	});
	sdata += "</HITS>";
}

void SpitSetOfOutputRows(std::string & sdata, std::set<BranchOutputRow> const & setOfRows)
{
	sdata += "<SET_OF_ROWS>";
	int index = 0;
	std::for_each(setOfRows.cbegin(), setOfRows.cend(), [&](BranchOutputRow const & row)
	{
		sdata += "<SINGLE_ROW>";
		sdata += "<ROW_NUMBER>";
		sdata += boost::lexical_cast<std::string>(index);
		sdata += "</ROW_NUMBER>";
		SpitOutputRow(sdata, row);
		sdata += "</SINGLE_ROW>";
		++index;
	});
	sdata += "</SET_OF_ROWS>";
}

void SpitOutputRow(std::string & sdata, BranchOutputRow const & row)
{
	sdata += "<ROW>";

	sdata += "<INDICES_FOR_THIS_SINGLE_ROW_POINTING_INTO_LEAF_SET_FOR_THIS_BRANCH>";
	std::for_each(row.primary_leaves.cbegin(), row.primary_leaves.cend(), [&](int const & leafindex)
	{
		sdata += "<INDEX_POINTING_INTO_LEAF_SET>";
		sdata += boost::lexical_cast<std::string>(leafindex);
		sdata += "</INDEX_POINTING_INTO_LEAF_SET>";
	});
	sdata += "</INDICES_FOR_THIS_SINGLE_ROW_POINTING_INTO_LEAF_SET_FOR_THIS_BRANCH>";

	sdata += "<CHILD_SECONDARY_DATA_CORRESPONDING_TO_THIS_OUTPUT_ROW>";
	std::for_each(row.child_indices_into_raw_data.cbegin(), row.child_indices_into_raw_data.cend(), [&](std::pair<int const, std::map<int, std::int64_t>> const & childindices)
	{
		sdata += "<SPECIFIC_VARIABLE_GROUP_CHILD_SECONDARY_DATA>";
		sdata += "<VARIABLE_GROUP_NUMBER>";
		sdata += boost::lexical_cast<std::string>(childindices.first);
		sdata += "</VARIABLE_GROUP_NUMBER>";
		std::for_each(childindices.second.cbegin(), childindices.second.cend(), [&](std::pair<int const, std::int64_t> const & childleaves)
		{
			sdata += "<SINGLE_LEAF_OF_CHILD_DATA_FOR_THIS_OUTPUT_ROW>";
			sdata += "<LEAF_NUMBER>";
			sdata += boost::lexical_cast<std::string>(childleaves.first);
			sdata += "</LEAF_NUMBER>";
			sdata += "<INDEX_OF_LEAF_IN_CHILD_DATA_CACHE>";
			sdata += boost::lexical_cast<std::string>(childleaves.second);
			sdata += "</INDEX_OF_LEAF_IN_CHILD_DATA_CACHE>";
			sdata += "</SINGLE_LEAF_OF_CHILD_DATA_FOR_THIS_OUTPUT_ROW>";
		});
		sdata += "</SPECIFIC_VARIABLE_GROUP_CHILD_SECONDARY_DATA>";
	});
	sdata += "</CHILD_SECONDARY_DATA_CORRESPONDING_TO_THIS_OUTPUT_ROW>";

	sdata += "</ROW>";
}

void SpitChildLookup(std::string & sdata, std::map<ChildDMUInstanceDataVector, std::map<BranchOutputRow const *, std::vector<int>>> const & helperLookup)
{
	sdata += "<LIST_OF_CHILD_KEYLISTS_THAT_MATCH_SOMETHING_IN_THIS_BRANCH>";
	std::for_each(helperLookup.cbegin(), helperLookup.cend(), [&](std::pair<ChildDMUInstanceDataVector const, std::map<BranchOutputRow const *, std::vector<int>>> const & helper)
	{
		sdata += "<CHILD_KEYLIST_THAT_MATCHES_SOMETHING_IN_THIS_BRANCH>";

		sdata += "<CHILD_DMU_KEYS>";
		SpitKeys(sdata, helper.first);
		sdata += "</CHILD_DMU_KEYS>";

		sdata += "<ROWS_CONTAINING_DATA_THAT_INCLUDES_THE_GIVEN_CHILD_KEYS>";
		std::for_each(helper.second.cbegin(), helper.second.cend(), [&](std::pair<BranchOutputRow const *, std::vector<int>> const & helperrow)
		{
			sdata += "<ROW_CONTAINING_DATA_THAT_INCLUDES_THE_GIVEN_CHILD_KEYS>";

			sdata += "<SINGLE_ROW_CONTAINING_DATA_THAT_INCLUDES_THE_GIVEN_CHILD_KEYS>";
			SpitOutputRow(sdata, *helperrow.first);
			sdata += "</SINGLE_ROW_CONTAINING_DATA_THAT_INCLUDES_THE_GIVEN_CHILD_KEYS>";

			sdata += "<WHICH_CHILD_LEAF_NUMBERS_IN_THE_FULL_ROW_MATCH_THIS_CHILD_LEAF_KEY_DATA>";
			std::for_each(helperrow.second.cbegin(), helperrow.second.cend(), [&](int const & leafindex)
			{
				sdata += "<CHILD_LEAF_NUMBER_THAT_MATCHES_THE_GIVEN_CHILD_LEAF_KEY_DATA>";
				sdata += boost::lexical_cast<std::string>(leafindex);
				sdata += "</CHILD_LEAF_NUMBER_THAT_MATCHES_THE_GIVEN_CHILD_LEAF_KEY_DATA>";
			});
			sdata += "</WHICH_CHILD_LEAF_NUMBERS_IN_THE_FULL_ROW_MATCH_THIS_CHILD_LEAF_KEY_DATA>";

			sdata += "</ROW_CONTAINING_DATA_THAT_INCLUDES_THE_GIVEN_CHILD_KEYS>";
		});
		sdata += "</ROWS_CONTAINING_DATA_THAT_INCLUDES_THE_GIVEN_CHILD_KEYS>";

		sdata += "</CHILD_KEYLIST_THAT_MATCHES_SOMETHING_IN_THIS_BRANCH>";
	});
	sdata += "</LIST_OF_CHILD_KEYLISTS_THAT_MATCH_SOMETHING_IN_THIS_BRANCH>";
}

void SpitLeaf(std::string & sdata, Leaf const & leaf)
{
	sdata += "<LEAF>";
	sdata += "<INDEX_POINTING_TO_PRIMARY_VG_RAW_SECONDARY_DATA_FOR_THIS_LEAF>";
	sdata += boost::lexical_cast<std::string>(leaf.index_into_raw_data);
	sdata += "</INDEX_POINTING_TO_PRIMARY_VG_RAW_SECONDARY_DATA_FOR_THIS_LEAF>";
	sdata += "<LEAF_DMU_DATALIST>";
	SpitKeys(sdata, leaf.primary_keys);
	sdata += "</LEAF_DMU_DATALIST>";
	sdata += "<OTHER_NON_PRIMARY_TOP_LEVEL_INDICES__ONE_PER_LEAF__POINTING_INTO_DATA_CACHE>";
	std::for_each(leaf.other_top_level_indices_into_raw_data.cbegin(), leaf.other_top_level_indices_into_raw_data.cend(), [&](std::pair<int const, std::int64_t> const & leafindicesintorawdata)
	{
		sdata += "<VARIABLE_GROUP>";
		sdata += "<VARIABLE_GROUP_NUMBER>";
		sdata += boost::lexical_cast<std::string>(leafindicesintorawdata.first);
		sdata += "</VARIABLE_GROUP_NUMBER>";
		sdata += "<INDEX_POINTING_TO_SINGLE_LEAF_RAW_DATA>";
		sdata += boost::lexical_cast<std::string>(leafindicesintorawdata.second);
		sdata += "</INDEX_POINTING_TO_SINGLE_LEAF_RAW_DATA>";
		sdata += "</VARIABLE_GROUP>";
	});
	sdata += "</OTHER_NON_PRIMARY_TOP_LEVEL_INDICES__ONE_PER_LEAF__POINTING_INTO_DATA_CACHE>";
	sdata += "</LEAF>";
}

void SpitBranch(std::string & sdata, Branch const & branch)
{
	sdata += "<BRANCH>";

	sdata += "<PRIMARY_KEYS>";
	SpitKeys(sdata, branch.primary_keys);
	sdata += "</PRIMARY_KEYS>";

	SpitHits(sdata, branch.hits);

	sdata += "<CHILD_KEY_LOOKUP_TO_QUICKLY_DETERMINE_IF_ANY_PARTICULAR_CHILD_KEYSET_EXISTS_FOR_ANY_OUTPUT_ROW_FOR_THIS_BRANCH>";
	SpitChildLookup(sdata, branch.helper_lookup__from_child_key_set__to_matching_output_rows);
	sdata += "</CHILD_KEY_LOOKUP_TO_QUICKLY_DETERMINE_IF_ANY_PARTICULAR_CHILD_KEYSET_EXISTS_FOR_ANY_OUTPUT_ROW_FOR_THIS_BRANCH>";

	sdata += "<ALL_THE_LEAVES_FOR_THIS_BRANCH>";
	branch.SpitLeaves(sdata);
	sdata += "</ALL_THE_LEAVES_FOR_THIS_BRANCH>";

	sdata += "<NUMBER_OF_LEAF_COMBINATIONS>";
	sdata += branch.number_branch_combinations.str();
	sdata += "</NUMBER_OF_LEAF_COMBINATIONS>";

	sdata += "<WEIGHTING>";
	SpitWeighting(sdata, branch.weighting);
	sdata += "</WEIGHTING>";

	sdata += "</BRANCH>";
}

void SpitWeighting(std::string & sdata, Weighting const & weighting)
{
	sdata += "<WEIGHTING_START>";
	sdata += weighting.getWeightingRangeStart().str();
	sdata += "</WEIGHTING_START>";

	sdata += "<WEIGHTING_END>";
	sdata += weighting.getWeightingRangeEnd().str();
	sdata += "</WEIGHTING_END>";

	sdata += "<WEIGHTING_VALUE>";
	sdata += weighting.getWeighting().str();
	sdata += "</WEIGHTING_VALUE>";
}

void SpitChildToPrimaryKeyColumnMapping(std::string & sdata, ChildToPrimaryMapping const & childToPrimaryMapping)
{
	sdata += "<MAPPING_TYPE>";
	sdata += ChildToPrimaryMapping::MappingToText(childToPrimaryMapping.mapping);
	sdata += "</MAPPING_TYPE>";

	sdata += "<index_of_column_within_top_level_branch_or_single_leaf>";
	sdata += boost::lexical_cast<std::string>(childToPrimaryMapping.index_of_column_within_top_level_branch_or_single_leaf);
	sdata += "</index_of_column_within_top_level_branch_or_single_leaf>";

	sdata += "<leaf_number_in_top_level_group__only_applicable_when_child_key_column_points_to_top_level_column_that_is_in_top_level_leaf>";
	sdata += boost::lexical_cast<std::string>(childToPrimaryMapping.leaf_number_in_top_level_group__only_applicable_when_child_key_column_points_to_top_level_column_that_is_in_top_level_leaf);
	sdata += "</leaf_number_in_top_level_group__only_applicable_when_child_key_column_points_to_top_level_column_that_is_in_top_level_leaf>";

}

void SpitAllWeightings(std::vector<std::string> & sdata_, AllWeightings const & allWeightings, bool const to_file)
{

	std::fstream file_;
	if (to_file)
	{
		file_.open("all_weightings.xml", std::ofstream::out | std::ios::trunc);
		if (!file_.is_open())
		{
			std::string theerr = strerror(errno);
			int m = 0;
		}
	}

	sdata_.push_back(std::string());

	std::string * sdata = &sdata_.back();

	*sdata += "<ALL_WEIGHTINGS>";

	*sdata += "<TIME_GRANULARITY>";
	*sdata += GetTimeGranularityText(allWeightings.time_granularity);
	*sdata += "</TIME_GRANULARITY>";

	*sdata += "<NUMBER_CHILD_VARIABLE_GROUPS>";
	*sdata += boost::lexical_cast<std::string>(allWeightings.numberChildVariableGroups);
	*sdata += "</NUMBER_CHILD_VARIABLE_GROUPS>";

	*sdata += "<childInternalToOneLeafColumnCountForDMUWithMultiplicityGreaterThan1>";
	std::for_each(allWeightings.childInternalToOneLeafColumnCountForDMUWithMultiplicityGreaterThan1.cbegin(), allWeightings.childInternalToOneLeafColumnCountForDMUWithMultiplicityGreaterThan1.cend(), [&](decltype(allWeightings.childInternalToOneLeafColumnCountForDMUWithMultiplicityGreaterThan1)::value_type const & oneChildGroup)
	{
		*sdata += "<child_group>";

		*sdata += "<child_variable_group_index>";
		*sdata += boost::lexical_cast<std::string>(oneChildGroup.first);
		*sdata += "</child_variable_group_index>";
		*sdata += "<column_count_for_child_dmu_with_child_multiplicity_greater_than_1>";
		*sdata += boost::lexical_cast<std::string>(oneChildGroup.second);
		*sdata += "</column_count_for_child_dmu_with_child_multiplicity_greater_than_1>";

		*sdata += "</child_group>";
	});
	*sdata += "</childInternalToOneLeafColumnCountForDMUWithMultiplicityGreaterThan1>";

	*sdata += "<CHILD_COLUMN_TO_TOP_LEVEL_COLUMN_KEY_MAPPINGs>";
	for (int c = 0; c < allWeightings.numberChildVariableGroups; ++c)
	{
		*sdata += "<CHILD_GROUP>";

		*sdata += "<CHILD_GROUP_INDEX>";
		*sdata += boost::lexical_cast<std::string>(c);
		*sdata += "</CHILD_GROUP_INDEX>";

		*sdata += "<CHILD_GROUP_COLUMN_MAPPINGS>";
		*sdata += "<BRANCH_MAPPINGS>";
		std::for_each(allWeightings.mappings_from_child_branch_to_primary.cbegin(), allWeightings.mappings_from_child_branch_to_primary.cend(), [&](decltype(allWeightings.mappings_from_child_branch_to_primary)::value_type const & oneChildGroupBranchMappings)
		{
			if (oneChildGroupBranchMappings.first == c)
			{
				std::for_each(oneChildGroupBranchMappings.second.cbegin(), oneChildGroupBranchMappings.second.cend(), [&](ChildToPrimaryMapping const & childToPrimaryMapping)
				{
					*sdata += "<BRANCH_MAPPING>";
					SpitChildToPrimaryKeyColumnMapping(*sdata, childToPrimaryMapping);
					*sdata += "</BRANCH_MAPPING>";
				});
			}
		});
		*sdata += "</BRANCH_MAPPINGS>";
		*sdata += "<LEAF_MAPPINGS>";
		std::for_each(allWeightings.mappings_from_child_leaf_to_primary.cbegin(), allWeightings.mappings_from_child_leaf_to_primary.cend(), [&](decltype(allWeightings.mappings_from_child_leaf_to_primary)::value_type const & oneChildGroupLeafMappings)
		{
			if (oneChildGroupLeafMappings.first == c)
			{
				std::for_each(oneChildGroupLeafMappings.second.cbegin(), oneChildGroupLeafMappings.second.cend(), [&](ChildToPrimaryMapping const & childToPrimaryMapping)
				{
					*sdata += "<LEAF_MAPPING>";
					SpitChildToPrimaryKeyColumnMapping(*sdata, childToPrimaryMapping);
					*sdata += "</LEAF_MAPPING>";
				});
			}
		});
		*sdata += "</LEAF_MAPPINGS>";
		*sdata += "</CHILD_GROUP_COLUMN_MAPPINGS>";

		*sdata += "</CHILD_GROUP>";
	}
	*sdata += "</CHILD_COLUMN_TO_TOP_LEVEL_COLUMN_KEY_MAPPINGs>";

	*sdata += "<DATA_CACHE_PRIMARY>";
	SpitDataCache(*sdata, allWeightings.dataCache);
	*sdata += "</DATA_CACHE_PRIMARY>";

	*sdata += "<DATA_CACHES_TOP_LEVEL_NON_PRIMARY>";
	SpitDataCaches(*sdata, allWeightings.otherTopLevelCache);
	*sdata += "</DATA_CACHES_TOP_LEVEL_NON_PRIMARY>";

	*sdata += "<DATA_CACHES_CHILDREN>";
	SpitDataCaches(*sdata, allWeightings.childCache);
	*sdata += "</DATA_CACHES_CHILDREN>";

	*sdata += "<TIME_SLICES>";
	std::for_each(allWeightings.timeSlices.cbegin(), allWeightings.timeSlices.cend(), [&](decltype(allWeightings.timeSlices)::value_type const & timeSlice)
	{

		if (sdata->size() > 500000)
		{
			sdata_.push_back(std::string());
			sdata = &sdata_.back();
		}

		*sdata += "<TIME_SLICE>";

		TimeSlice const & the_slice = timeSlice.first;
		VariableGroupTimeSliceData const & variableGroupTimeSliceData = timeSlice.second;
		VariableGroupBranchesAndLeavesVector const & variableGroupBranchesAndLeaves = variableGroupTimeSliceData.branches_and_leaves;

		*sdata += "<TIME>";
		SpitTimeSlice(*sdata, the_slice);
		*sdata += "</TIME>";

		*sdata += "<WEIGHTING>";
		SpitWeighting(*sdata, variableGroupTimeSliceData.weighting);
		*sdata += "</WEIGHTING>";

		*sdata += "<VARIABLE_GROUPS_BRANCHES_AND_LEAVES>";
		std::for_each(variableGroupBranchesAndLeaves.cbegin(), variableGroupBranchesAndLeaves.cend(), [&](VariableGroupBranchesAndLeaves const & variableGroupBranchesAndLeaves)
		{
			*sdata += "<VARIABLE_GROUP_BRANCHES_AND_LEAVES>";

			*sdata += "<VARIABLE_GROUP_NUMBER>";
			*sdata += boost::lexical_cast<std::string>(variableGroupBranchesAndLeaves.variable_group_number);
			*sdata += "</VARIABLE_GROUP_NUMBER>";

			*sdata += "<WEIGHTING>";
			SpitWeighting(*sdata, variableGroupBranchesAndLeaves.weighting);
			*sdata += "</WEIGHTING>";

			*sdata += "<BRANCHES_AND_LEAVES>";
			std::for_each(variableGroupBranchesAndLeaves.branches.cbegin(), variableGroupBranchesAndLeaves.branches.cend(), [&](Branch const & branch)
			{
				*sdata += "<BRANCH_AND_LEAVES>";

				SpitBranch(*sdata, branch);

				branch.SpitLeaves(*sdata);

				*sdata += "</BRANCH_AND_LEAVES>";
			});
			*sdata += "</BRANCHES_AND_LEAVES>";

			*sdata += "</VARIABLE_GROUP_BRANCHES_AND_LEAVES>";
		});
		*sdata += "</VARIABLE_GROUPS_BRANCHES_AND_LEAVES>";

		*sdata += "</TIME_SLICE>";

	});
	*sdata += "</TIME_SLICES>";

	*sdata += "<WEIGHTING>";
	SpitWeighting(*sdata, allWeightings.weighting);
	*sdata += "</WEIGHTING>";

	*sdata += "</ALL_WEIGHTINGS>";

	if (to_file)
	{
		std::for_each(sdata_.cbegin(), sdata_.cend(), [&](std::string const & sdata__)
		{
			file_ << sdata__;
		});
		file_.close();
	}

}

void SpitLeaves(std::string & sdata, Branch const & branch)
{
	sdata += "<LEAVES>";
	branch.ResetLeafCache();
	branch.SpitLeaves(sdata);
	sdata += "</LEAVES>";
}

//#endif

void VariableGroupTimeSliceData::PruneTimeUnits(AllWeightings & allWeightings, TimeSlice const & originalTimeSlice, TimeSlice const & currentTimeSlice, std::int64_t const AvgMsperUnit, bool const consolidate_rows, bool const random_sampling)
{

	// ********************************************************************************************** //
	// This function is called when time slices (and corresponding output row data)
	// are SPLIT.
	// Note that the opposite case - when time slices are MERGED - 
	// it is the context of CONSOLIDATED output,
	// a scenario where precisely the "hits" data is wiped out and consolidated
	// so that this function would be irrelevent (and is not called).
	// So again, in the context of this function, we are only slicing time slices, never merging.
	// Furthermore, the calling code guarantees that time slices are only shrunk from both sides
	// (or left the same), never expanded.
	// So it is guaranteed that "originalTimeSlice" fully covers "currentTimeSlice".
	//
	// This function takes the current time slice (this instance)
	// and recalculates its "hits" data, throwing away "hits" time units that
	// are outside the range indicated by "current time slice" in relation to "original time slice".
	// Note that each "hit" element contains possibly MANY output rows, all corresponding
	// to a fixed (sub-)time-width that is exactly equal to the time width corresponding
	// to a single unit of time at the time range granularity corresponding to the primary variable group.
	// I.e., for "day", the time unit is 1 day.
	// Further note that each TIME SLICE can cover (in this example) many days -
	// this is because rows of raw input data can also cover multiple days.
	//
	// Finally, note that if a time slice within the above "day" scenario (and this equally
	// applies to any time range granularity corresponding to the primary variable group)
	// is pruned into a width smaller than
	// a day (which can happen during child variable group merges), the number of "time units"
	// in the resulting "hits" variable could round to 0.  However, the algorithm guarantees
	// that in this scenario, no matter how small the time slice is being pruned to, there
	// will be at least 1 entry (set of rows) in "hits".  This will correspond to having
	// the data output file contain *multiple* sets of rows of data that lie in the same
	// "day" time slice - but with sub-day granularity.  This is desired so that all K-ads
	// are successfully output.
	//
	// Finally, note that this function is always exited if full sampling is being performed
	// (not random), EVEN THOUGH the full sampler must distribute rows over the time units
	// discussed above within individual time slices that span multiple time units.
	// However, the full sampler, in "non-consolidate" mode, uses a different approach
	// to do this that does not involve the "hits" data member.
	// Namely, the full sampler simply looks at how many time units there actually are
	// corresponding to the given time slice, and just outputs the same data that many times,
	// properly changing the time slice start & end datetimes as appropriate, of course.
	// Only the random sampler, which randomly chooses rows from among different time units
	// within each time slice (that is the entire purpose of the "hits" data structure and its data!)
	// must process the "hits" when time slices are pruned.
	//
	// Note that we call it "pruning" time slices, but really the calling function is "splitting"
	// existing time slices into multiple pieces, each a sub-piece of the original (never expanding)
	// and each with an exact copy of the original piece's "hits" data, and then the calling function
	// calls "prune" on each of the relevant sub-pieces (those that are not being thrown away).
	//
	// Again, the *merging* of time slices only occurs when "consolidate_rows" is true,
	// so there's no need for this function to handle the case of EXPANDING (rather than pruning)
	// the current time slice.
	// ********************************************************************************************** //

	if (!random_sampling)
	{
		// ************************************************************* //
		// For full sampling, the equivalent to this "prune" algorithm -
		// which handles WEIGHTS across non-uniform slices -
		// is handled in the OUTPUT TO DISK routine,
		// where an algorithm loops through all time slices
		// and outputs them X times, once per time unit or fraction
		// of a time unit.
		// We do the equivalent thing in this function by
		// making sure to include all rows even for fractions
		// of a time unit, but to divide rows among full-sized
		// time units that are being separated;
		// OUR output routine will also output X times, once
		// per time unit or fraction of a time unit,
		// where the time units are carried by the bins
		// managed in this function, rather than by the time window
		// of the full slice which gets cut into time unit pieces
		// (or fraction thereof), as is the case for the full sampling algorithm.
		// ************************************************************* //

		// If rows are being merged (when possible) in the output,
		// this function does not apply.
		// Only if rows are being output in identical time units
		// (or sub-time-units, if such a row has been split into
		// sub-time-units by child data merges) does this routine apply.
		// Also note that this routine processes the "hits" data cache
		// containing all output rows for the given branch, distributed
		// over even "time unit" chunks corresponding to the time granularity
		// corresponding to the primary variable group, and the "hits" data
		// is only so distributed for random sampling (which could still
		// call this function, but only if rows are not being merged,
		// because even with random sampling, after the sampling is complete,
		// the user still has the option to display the results with
		// rows consolidated, in which case this function, again, won't be called).
		return;
	}

	if (!originalTimeSlice.hasTimeGranularity() || !currentTimeSlice.hasTimeGranularity())
	{
		boost::format msg("Attempting to prune time units with no time granularity!");
		throw NewGeneException() << newgene_error_description(msg.str());
	}

	std::int64_t originalWidth = originalTimeSlice.WidthForWeighting(AvgMsperUnit);

	if (originalWidth < 0)
	{
		boost::format msg("Logic error when pruning time slice: original width is empty for time slice");
		throw NewGeneException() << newgene_error_description(msg.str());
	}

	std::int64_t leftWidth = 0;
	std::int64_t middleWidth = 0;
	std::int64_t rightWidth = 0;

	if (originalWidth == 0)
	{
		// Do not throw.
		// Child groups can enter and break a large primary slice into a miniscule sliver.
		// When this happens, that sliver will have 0 weight since weight is an integer (and rounded).
		// Because it is a child, we want the row to be output, because its time range
		// will indicate that it is a sliver.
		//
		// The way to handle this is the following.
		//
		// Any time slices LARGER than one time unit that get spliced should properly
		// be pruned relative to the amount they've been sliced.
		// That way, in the output, the total weight (as represented by the number of rows,
		// which is proportional to the number of time units on average)
		// will remain correct (unbiased sampling) because the output rows
		// will all have this "unit" width, so the weights will add up properly.
		//
		// However, any SINGLE-UNIT slices that get cut SMALLER than a single time unit -
		// or, any slices that get a piece shaved off that is smaller than a single time unit -
		// should still appear in the output.  In this case, their time range will indicate to 
		// the end user their proper weight.  However, internally, they will hold a weight of 1 time unit
		// and be treated that way.
		// 
		// In the 0 case we can bypass the calculations below that determine how many "time units" (hits)
		// to divvy between the spliced pieces.  We just add them all to the single target piece.
		//
		// In the calculation below, all widths will nicely turn to 0,
		// but the proper target piece will still be properly selected.
		// The code below handles the 0 case just fine, stepping it up to 1 for the target piece.
		//
		// Look at the comments in the "random sampling, non-consolidated" case
		// in the function that writes the final data to the output file
		// to see how the time ranges are used in relation to their presence in the "hits" time indexes.
		//
		// In short, we *leave the data available* no matter how small the time slice is,
		// and let the output routine ensure that the time widths properly weight the slices.
		//
		// [.............][.............][.............][.............][.............][.............][.............][.............]
		//             |____A____|       |______________B_____________]
		//
		// In this example, each [.............] corresponds to a time unit with some output rows - on average, the dots each represent an output row.
		// B is a time slice that evenly covers two time units.
		// A covers just a sliver of one time unit, but A, since it is being merged in, will actually be sliced and its left sliver
		// will contain ALL of the output rows from the left time unit (and ditto for the right piece of A);
		// You can see that the algorithm that outputs the data will simply output all rows in the sliver,
		// but the sliver will have a very narrow range, so the weighting still works out.  (The algorithm
		// will also output the big left chunk of the first time unit with all the same rows and the bigger time range.)
		//
		// For this algorithm to work, it is critical that the PRIMARY variable group always arrive in multiples
		// of the time unit, and evenly aligned.  The data import and time granularity of the unit of analysis assure this.
		//
		// It's all about counting

		//boost::format msg("Logic error when pruning time slice: original width is empty for time slice");
		//throw NewGeneException() << newgene_error_description(msg.str());

		originalWidth = 1;
	}

	bool useLeft = false;
	bool useMiddle = false;
	bool useRight = false;

	// Time slices get split apart, but never merged together, in the context of this function.


	// ************************************************************************************************* //
	// This is the critical block that deals properly with unbiased random sampling
	// to handle time slices that are not aligned on unit time boundaries
	// (with the unit defined by the unit of analysis)
	// ************************************************************************************************* //
	if (originalTimeSlice.getStart() < currentTimeSlice.getStart())
	{
		if (originalTimeSlice.getEnd() <= currentTimeSlice.getStart())
		{
			// no overlap
			boost::format msg("Attempting to prune time units that do not overlap!");
			throw NewGeneException() << newgene_error_description(msg.str());
		}
		else if (originalTimeSlice.getEnd() < currentTimeSlice.getEnd())
		{
			// left of original is by itself, right of original overlaps left of current, right of current is by itself
			// The current slice must be embedded within the original slice for a legitimate prune!
			boost::format msg("Attempting to prune time units when current exceeds bounds of previous");
			throw NewGeneException() << newgene_error_description(msg.str());
		}
		else if (originalTimeSlice.getEnd() == currentTimeSlice.getEnd())
		{

			// left of original is by itself, right of original overlaps entire current with nothing left over
			leftWidth = currentTimeSlice.getStart() - originalTimeSlice.getStart();
			rightWidth = currentTimeSlice.getEnd() - currentTimeSlice.getStart();

			useRight = true;

		}
		else
		{

			// original end is past current end
			// left of original is by itself, current is completely overlapped by original, right of original is by itself
			leftWidth = currentTimeSlice.getStart() - originalTimeSlice.getStart();
			middleWidth = currentTimeSlice.getEnd() - currentTimeSlice.getStart();
			rightWidth = originalTimeSlice.getEnd() - currentTimeSlice.getEnd();

			useMiddle = true;

		}
	}
	else if (originalTimeSlice.getStart() == currentTimeSlice.getStart())
	{
		if (originalTimeSlice.getEnd() < currentTimeSlice.getEnd())
		{
			// left of original matches left of current, entire original overlaps the left part of current, and the right of current is by itself
			// The current slice must be embedded within the original slice for a legitimate prune!
			boost::format msg("Attempting to prune time units when current exceeds bounds of previous");
			throw NewGeneException() << newgene_error_description(msg.str());
		}
		else if (originalTimeSlice.getEnd() == currentTimeSlice.getEnd())
		{

			// both time slices are identical
			// Treat this as the middle piece
			middleWidth = originalTimeSlice.getEnd() - originalTimeSlice.getStart();
			useMiddle = true;

		}
		else
		{

			// original end is past current end
			// Left edges are the same, entire current overlaps original, and right side of original is by itself on the right
			leftWidth = currentTimeSlice.getEnd() - currentTimeSlice.getStart();
			rightWidth = originalTimeSlice.getEnd() - currentTimeSlice.getEnd();

			useLeft = true;

		}
	}
	else if (originalTimeSlice.getStart() < currentTimeSlice.getEnd())
	{
		if (originalTimeSlice.getEnd() < currentTimeSlice.getEnd())
		{
			// left of current is by itself, entire original is overlapped by current, right of current is by itself
			// The current slice must be embedded within the original slice for a legitimate prune!
			boost::format msg("Attempting to prune time units when current exceeds bounds of previous");
			throw NewGeneException() << newgene_error_description(msg.str());
		}
		else if (originalTimeSlice.getEnd() == currentTimeSlice.getEnd())
		{
			// left of current is by itself, entire original overlaps entire right of current exactly with nothing left over
			// The current slice must be embedded within the original slice for a legitimate prune!
			boost::format msg("Attempting to prune time units when current exceeds bounds of previous");
			throw NewGeneException() << newgene_error_description(msg.str());
		}
		else
		{
			// original end is past current end
			// left of current is by itself, right of current overlaps the left of original, and the right of original is by itself on the right
			// The current slice must be embedded within the original slice for a legitimate prune!
			boost::format msg("Attempting to prune time units when current exceeds bounds of previous");
			throw NewGeneException() << newgene_error_description(msg.str());
		}
	}
	else
	{
		// no overlap
		boost::format msg("Attempting to prune time units that do not overlap!");
		throw NewGeneException() << newgene_error_description(msg.str());
	}

	long double leftUnits = 0.0;
	long double middleUnits = 0.0;
	long double rightUnits = 0.0;

	std::int64_t leftRounded = 0;
	std::int64_t middleRounded = 0;
	std::int64_t rightRounded = 0;

	leftUnits = TimeSlice::WidthForWeighting(leftWidth, AvgMsperUnit);
	middleUnits = TimeSlice::WidthForWeighting(middleWidth, AvgMsperUnit);
	rightUnits = TimeSlice::WidthForWeighting(rightWidth, AvgMsperUnit);

	leftRounded = static_cast<std::int64_t>(leftUnits + 0.5);
	middleRounded = static_cast<std::int64_t>(middleUnits + 0.5);
	rightRounded = static_cast<std::int64_t>(rightUnits + 0.5);

	// Handle rounding issues to ensure that all "hits" entries are included somewhere
	if (useLeft || useRight)
	{

		if (leftRounded + rightRounded != originalWidth)
		{
			if (leftRounded + rightRounded - originalWidth != 1 && leftRounded + rightRounded - originalWidth != -1)
			{
				boost::format msg("Logic error when pruning time slice: time unit counts do not match");
				throw NewGeneException() << newgene_error_description(msg.str());
			}
			if (leftRounded + rightRounded - originalWidth == 1)
			{
				if (leftRounded > 1)
				{
					--leftRounded;
				}
				else
				{
					--rightRounded;
				}
			}
			else if (leftRounded + rightRounded - originalWidth == -1)
			{
				if (leftRounded == 0)
				{
					++leftRounded;
				}
				else
				{
					++rightRounded;
				}
			}
		}

		if (leftRounded < 0 || rightRounded < 0 || leftRounded > originalWidth || rightRounded > originalWidth)
		{
			boost::format msg("Logic error when pruning time slice: time unit counts do not match after adjustment");
			throw NewGeneException() << newgene_error_description(msg.str());
		}

	}
	else if (useMiddle)
	{
		if (leftRounded + middleRounded + rightRounded != originalWidth)
		{
			if (leftRounded + rightRounded + middleRounded - originalWidth != 1 && leftRounded + rightRounded + middleRounded - originalWidth != -1)
			{
				boost::format msg("Logic error when pruning time slice: time unit counts do not match");
				throw NewGeneException() << newgene_error_description(msg.str());
			}
			if (leftRounded + rightRounded + middleRounded - originalWidth == 1)
			{
				if (leftRounded > 1)
				{
					--leftRounded;
				}
				else if (middleRounded > 1)
				{
					--middleRounded;
				}
				else
				{
					--rightRounded;
				}
			}
			else if (leftRounded + rightRounded + middleRounded - originalWidth == -1)
			{
				if (leftRounded == 0)
				{
					++leftRounded;
				}
				else if (middleRounded == 0)
				{
					++middleRounded;
				}
				else
				{
					++rightRounded;
				}
			}
		}
	}

	if (leftRounded < 0 || rightRounded < 0 || middleRounded < 0 || leftRounded > originalWidth || rightRounded > originalWidth || middleRounded > originalWidth || (leftRounded + middleRounded + rightRounded != originalWidth) )
	{
		boost::format msg("Logic error when pruning time slice: time unit counts do not match after adjustment");
		throw NewGeneException() << newgene_error_description(msg.str());
	}

	// As noted above in the comments regarding 0 width,
	// we display at least one row with this data, not 0,
	// because slicing pieces *smaller* than unit time
	// is handled by the time range in the output,
	// and only happens during non-primary merges so 
	// therefore the weighting isn't messed up.
	//
	// To reiterate: As long as 
	// SUM[(output rows)*(time width)]
	// remains the same, the algorithm remains unbiased.
	// We achieve this here in two ways, depending on the circumstance.
	//
	// (1) Time slices larger than a unit get cut down to a smaller number of units.
	//     In this case, by dividing the output rows into the different resulting
	//     slices (recall that the data is the same, on average),
	//     such that the total number of rows remains the same,
	//     since the time slice width total also remains the same
	//     (each new time slice is a multiple of a number of time units in this extreme),
	//     the above formula holds.  (Note that when the data is output,
	//     rows are only listed as having UNIT width, even if they are inside
	//     a time slice with larger than unit width, which allows the above
	//     formula to hold.)
	//
	// (2) Time slices appear that are less than a time unit in width.
	//     In this scenario, instead of dividing the time slices between the target pruned slices,
	//     we instead keep all slices in all pieces.  In this way, the total width across
	//     the slices multiplied by the slices remains the same, so the above
	//     formula holds.  (Note here that each output row will have its time be LESS
	//     than a unit, analagous to the above case where each time slice is output
	//     as just a single time unit, even if in a time slice that is larger than a single time unit,
	//     so for an analagous reason the formula holds.)
	bool left_was_zero = false;
	bool middle_was_zero = false;
	bool right_was_zero = false;
	if (leftRounded == 0)
	{
		left_was_zero = true;
		++leftRounded;
	}
	if (rightRounded == 0)
	{
		right_was_zero = true;
		++rightRounded;
	}
	if (middleRounded == 0)
	{
		middle_was_zero = true;
		++middleRounded;
	}

	std::map<boost::multiprecision::cpp_int, std::set<BranchOutputRow>> new_hits;
	VariableGroupBranchesAndLeavesVector const & variableGroupBranchesAndLeavesVector = branches_and_leaves;
	std::for_each(variableGroupBranchesAndLeavesVector.cbegin(), variableGroupBranchesAndLeavesVector.cend(), [&](VariableGroupBranchesAndLeaves const & variableGroupBranchesAndLeaves)
	{

		std::for_each(variableGroupBranchesAndLeaves.branches.cbegin(), variableGroupBranchesAndLeaves.branches.cend(), [&](Branch const & current_branch)
		{

			// Single branch

			new_hits.clear();

			auto & hits = current_branch.hits;

			std::for_each(hits.cbegin(), hits.cend(), [&](std::pair<boost::multiprecision::cpp_int const, std::set<BranchOutputRow>> const & hit)
			{

				// Single time unit in branch, with its own set of rows

				boost::multiprecision::cpp_int hit_time_index = hit.first;
				boost::multiprecision::cpp_int hit_time_index_one_based = hit_time_index + 1;
				bool matches_left = false;
				bool matches_right = false;
				bool matches_middle = false;
				
				// Determine if matches left
				if (hit_time_index_one_based <= leftRounded)
				{
					// matches left
					matches_left = true;
				}

				// Determine if matches right
				if (hit_time_index_one_based > originalWidth - rightRounded)
				{
					matches_right = true;
				}
				 
				// Determine if matches middle
				if (left_was_zero)
				{
					if (hit_time_index_one_based <= middleRounded)
					{
						matches_middle = true;
					}
				}
				else if (right_was_zero)
				{
					if (hit_time_index_one_based > originalWidth - middleRounded)
					{
						matches_middle = true;
					}
				}
				else if (hit_time_index_one_based > leftRounded && hit_time_index_one_based <= (originalWidth - rightRounded))
				{
					matches_middle = true;
				}

				if (useRight)
				{
					if (matches_right)
					{
						new_hits[hit_time_index - (originalWidth - rightRounded)] = hit.second;
					}
				}
				else
				if (useLeft)
				{
					if (matches_left)
					{
						new_hits[hit_time_index] = hit.second;
					}
				}
				else
				if (useMiddle)
				{
					if (matches_middle)
					{
						if (left_was_zero)
						{
							new_hits[hit_time_index] = hit.second;
						}
						else if (right_was_zero)
						{
							new_hits[hit_time_index - (originalWidth - middleRounded)] = hit.second;
						}
						else
						{
							new_hits[hit_time_index - leftRounded] = hit.second;
						}
					}
				}

			});

			if (new_hits.empty())
			{
				int m = 0;
			}

			hits.clear();
			std::for_each(new_hits.cbegin(), new_hits.cend(), [&](std::pair<boost::multiprecision::cpp_int const, std::set<BranchOutputRow>> const & new_hit)
			{
				hits[new_hit.first] = new_hit.second;
			});

			current_branch.ValidateOutputRowLeafIndexes();

		});

	});
	
	ResetBranchCachesSingleTimeSlice(allWeightings);

}

void BindTermToInsertStatement(sqlite3_stmt * insert_random_sample_stmt, InstanceData const & data, int bindIndex)
{
	bind_visitor visitor(insert_random_sample_stmt, bindIndex);
	boost::apply_visitor(visitor, data);
}
