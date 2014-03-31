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

AllWeightings::AllWeightings()
: insert_random_sample_stmt(nullptr)
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

bool AllWeightings::HandleBranchAndLeaf(Branch const & branch, TimeSliceLeaf & newTimeSliceLeaf, int const & variable_group_number, VARIABLE_GROUP_MERGE_MODE const merge_mode, std::vector<ChildToPrimaryMapping> mappings_from_child_branch_to_primary, std::vector<ChildToPrimaryMapping> mappings_from_child_leaf_to_primary)
{

	TimeSlice const & newTimeSlice = newTimeSliceLeaf.first;

	if (!newTimeSlice.Validate())
	{
		return false;
	}

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

			if (newTimeSlice.time_start < startMapSlice.time_start)
			{

				// The new time slice starts to the left of the map element returned by upper_bound.

				if (newTimeSlice.time_end <= startMapSlice.time_start)
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

					// The new time slice starts to the left of the map element returned by upper_bound,
					// and ends inside or at the right edge of the first time slice in the map element returned by upper_bound.

					// Slice off the left-most piece of the new time slice
					// (and add it to the map if appropriate)
					TimeSliceLeaf new_left_slice;
					SliceOffLeft(newTimeSliceLeaf, startMapSlicePtr->first.time_start, new_left_slice);

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
				// The right edge of the new time slice could end anywhere (past the left edge of the new time slice).

				// This is the normal case.
				normalCase = true;
				mapIterator = startMapSlicePtr;

			}

		}

	}

	if (normalCase)
	{
		bool no_more_time_slice = HandleTimeSliceNormalCase(added, branch, newTimeSliceLeaf, mapIterator, variable_group_number, merge_mode, mappings_from_child_branch_to_primary, mappings_from_child_leaf_to_primary);
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
			bool added_recurse = HandleBranchAndLeaf(branch, newTimeSliceLeaf, variable_group_number, merge_mode, mappings_from_child_branch_to_primary, mappings_from_child_leaf_to_primary);
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
bool AllWeightings::HandleTimeSliceNormalCase(bool & added, Branch const & branch, TimeSliceLeaf & newTimeSliceLeaf, TimeSlices::iterator & mapElementPtr, int const & variable_group_number, VARIABLE_GROUP_MERGE_MODE const merge_mode, std::vector<ChildToPrimaryMapping> mappings_from_child_branch_to_primary, std::vector<ChildToPrimaryMapping> mappings_from_child_leaf_to_primary)
{

	bool added_new = false;

	TimeSlice const & newTimeSlice = newTimeSliceLeaf.first;
	TimeSlice const & mapElement   = mapElementPtr->first;

	TimeSlices::iterator newMapElementLeftPtr;
	TimeSlices::iterator newMapElementMiddlePtr;
	TimeSlices::iterator newMapElementRightPtr;

	TimeSliceLeaf new_left_slice;

	bool newTimeSliceEatenCompletelyUp = false;

	if (newTimeSlice.time_start == mapElement.time_start)
	{

		// The new time slice starts at the left edge of the map element.

		if (newTimeSlice.time_end < mapElement.time_end)
		{

			// The new time slice starts at the left edge of the map element,
			// and fits inside the map element,
			// with space left over in the map element.

			// Slice the map element into two pieces.
			// Merge the first piece with the new time slice.
			// Leave the second piece unchanged.

			SliceMapEntry(mapElementPtr, newTimeSlice.time_end, newMapElementLeftPtr, newMapElementRightPtr);
			added_new = MergeTimeSliceDataIntoMap(branch, newTimeSliceLeaf, newMapElementLeftPtr, variable_group_number, merge_mode, mappings_from_child_branch_to_primary, mappings_from_child_leaf_to_primary);

			mapElementPtr = newMapElementLeftPtr;

			newTimeSliceEatenCompletelyUp = true;

		}
		else if (newTimeSlice.time_end == mapElement.time_end)
		{

			// The new time slice exactly matches the first map element.

			// Merge the new time slice with the map element.

			added_new = MergeTimeSliceDataIntoMap(branch, newTimeSliceLeaf, mapElementPtr, variable_group_number, merge_mode, mappings_from_child_branch_to_primary, mappings_from_child_leaf_to_primary);

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

			SliceOffLeft(newTimeSliceLeaf, mapElement.time_end, new_left_slice);
			added_new = MergeTimeSliceDataIntoMap(branch, new_left_slice, mapElementPtr, variable_group_number, merge_mode, mappings_from_child_branch_to_primary, mappings_from_child_leaf_to_primary);

			mapElementPtr = ++mapElementPtr;

		}

	}
	else
	{

		// The new time slice is inside the map element,
		// but starts past its left edge.

		if (newTimeSlice.time_end < mapElement.time_end)
		{

			// The new time slice starts in the map element,
			// but past its left edge.
			// The new time slice ends before the right edge of the map element.

			// Slice the map element into three pieces.
			// The first is unchanged.
			// The second merges with the new time slice.
			// The third is unchanged.

			SliceMapEntry(mapElementPtr, newTimeSlice.time_start, newTimeSlice.time_end, newMapElementMiddlePtr);
			added_new = MergeTimeSliceDataIntoMap(branch, newTimeSliceLeaf, newMapElementMiddlePtr, variable_group_number, merge_mode, mappings_from_child_branch_to_primary, mappings_from_child_leaf_to_primary);

			mapElementPtr = newMapElementMiddlePtr;

			newTimeSliceEatenCompletelyUp = true;

		}
		else if (newTimeSlice.time_end == mapElement.time_end)
		{

			// The new time slice starts in the map element,
			// but past its left edge.
			// The new time slice ends at exactly the right edge of the map element.

			// Slice the map element into two pieces.
			// The first is unchanged.
			// The second merges with the new time slice
			// (with the right edge equal to the right edge of the map element).

			SliceMapEntry(mapElementPtr, newTimeSlice.time_start, newMapElementLeftPtr, newMapElementRightPtr);
			added_new = MergeTimeSliceDataIntoMap(branch, newTimeSliceLeaf, newMapElementRightPtr, variable_group_number, merge_mode, mappings_from_child_branch_to_primary, mappings_from_child_leaf_to_primary);

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

			SliceOffLeft(newTimeSliceLeaf, mapElement.time_end, new_left_slice);
			SliceMapEntry(mapElementPtr, newTimeSlice.time_start, newMapElementLeftPtr, newMapElementRightPtr);
			added_new = MergeTimeSliceDataIntoMap(branch, new_left_slice, newMapElementRightPtr, variable_group_number, merge_mode, mappings_from_child_branch_to_primary, mappings_from_child_leaf_to_primary);

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
void AllWeightings::SliceMapEntry(TimeSlices::iterator const & existingMapElementPtr, std::int64_t const middle, TimeSlices::iterator & newMapElementLeftPtr, TimeSlices::iterator & newMapElementRightPtr)
{

	TimeSlice timeSlice = existingMapElementPtr->first;
	VariableGroupTimeSliceData const timeSliceData = existingMapElementPtr->second;

	std::int64_t left = timeSlice.time_start;
	std::int64_t right = timeSlice.time_end;

	timeSlices.erase(existingMapElementPtr);

	timeSlice.Reshape(left, middle);
	timeSlices[timeSlice] = timeSliceData;

	newMapElementLeftPtr = timeSlices.find(timeSlice);

	timeSlice.Reshape(middle, right);
	timeSlices[timeSlice] = timeSliceData;

	newMapElementRightPtr = timeSlices.find(timeSlice);

}

// breaks an existing map entry into three pieces and returns an iterator to the middle piece
void AllWeightings::SliceMapEntry(TimeSlices::iterator const & existingMapElementPtr, std::int64_t const left, std::int64_t const right, TimeSlices::iterator & newMapElementMiddlePtr)
{

	TimeSlice timeSlice = existingMapElementPtr->first;
	VariableGroupTimeSliceData const timeSliceData = existingMapElementPtr->second;

	std::int64_t leftedge = timeSlice.time_start;
	std::int64_t rightedge = timeSlice.time_end;

	timeSlices.erase(existingMapElementPtr);

	timeSlice.Reshape(leftedge, left);
	timeSlices[timeSlice] = timeSliceData;

	timeSlice.Reshape(left, right);
	timeSlices[timeSlice] = timeSliceData;

	newMapElementMiddlePtr = timeSlices.find(timeSlice);

	timeSlice.Reshape(right, rightedge);
	timeSlices[timeSlice] = timeSliceData;

}

// Slices off the left part of the "incoming_slice" TimeSliceLeaf and returns it in the "new_left_slice" TimeSliceLeaf.
// The "incoming_slice" TimeSliceLeaf is adjusted to become equal to the remaining part on the right.
void AllWeightings::SliceOffLeft(TimeSliceLeaf & incoming_slice, std::int64_t const slicePoint, TimeSliceLeaf & new_left_slice)
{
	new_left_slice = incoming_slice;
	new_left_slice.first.time_end = slicePoint;

	incoming_slice.first.time_start = slicePoint;
}

// Merge time slice data into a map element
bool AllWeightings::MergeTimeSliceDataIntoMap(Branch const & branch, TimeSliceLeaf const & timeSliceLeaf, TimeSlices::iterator & mapElementPtr, int const & variable_group_number, VARIABLE_GROUP_MERGE_MODE const merge_mode, std::vector<ChildToPrimaryMapping> mappings_from_child_branch_to_primary, std::vector<ChildToPrimaryMapping> mappings_from_child_leaf_to_primary)
{

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

		VariableGroupBranchesAndLeaves newVariableGroupBranch(variable_group_number);
		BranchesAndLeaves & newBranchesAndLeaves = newVariableGroupBranch.branches_and_leaves;
		newBranchesAndLeaves[branch].emplace(timeSliceLeaf.second); // add Leaf to the set of Leaves attached to the new Branch
		variableGroupBranchesAndLeavesVector.push_back(newVariableGroupBranch);

		added = true;

	}
	else
	{

		// Branches already exists for this variable group.
		// The incoming branch might match one of these, or it might not.
		// In any case, retrieve the existing set of branches for this variable group.

		VariableGroupBranchesAndLeaves & variableGroupBranch = *VariableGroupBranchesAndLeavesPtr;
		BranchesAndLeaves & branchesAndLeaves = variableGroupBranch.branches_and_leaves;

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
				branchesAndLeaves[branch].emplace(timeSliceLeaf.second); // add Leaf to the set of Leaves attached to the new Branch, if one doesn't already exist there

				added = true;

			}
			break;

			case VARIABLE_GROUP_MERGE_MODE__TOP_LEVEL:
			{
				
				// Let's take a peek and see if our branch is already present

				BranchesAndLeaves::iterator branchAndLeavesPtr = branchesAndLeaves.find(branch);
				if (branchAndLeavesPtr != branchesAndLeaves.end())
				{
					
					// *********************************************************************************** //
					// The incoming branch *does* already exist!
					// We want to see if this branch contains the incoming leaf, or not.
					// *********************************************************************************** //

					Leaves & leaves = branchAndLeavesPtr->second;
					auto leafPtr = leaves.find(timeSliceLeaf.second);
					if (leafPtr != leaves.end())
					{

						// This branch *does* contain the incoming leaf!
						// Set the data in the leaf for this non-primary top-level variable group.

						// Note that many different OUTPUT ROWS might reference this leaf;
						// perhaps even multiple times within a single output row.  Fine!

						Leaf const & leaf = *leafPtr;

						// pass the index over from the incoming leaf (which contains only the index for the current top-level variable group being merged in)
						// into the active leaf saved in the AllWeightings instance, and used to construct the output rows.
						// (This active leaf may also have been called previously to set other top-level variable group rows.)
						leaf.other_top_level_indices_into_raw_data[variable_group_number] = timeSliceLeaf.second.other_top_level_indices_into_raw_data[variable_group_number];

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
				// For each, we must therefore build its cache that maps the incoming 
				// *********************************************************************************** //

				std::for_each(branchesAndLeaves.begin(), branchesAndLeaves.end(), [&](std::pair<Branch const, Leaves> & branchAndLeaves)
				{

					// *********************************************************************************** //
					// "leaves_cache" is a vector cache containing the same leaves in the same order
					// as the official "leaves" set containing the leaves for the current branch.
					//
					// Note that a call to "ResetBranchCaches()" previous to the high-level call to "HandleBranchAndLeaf()",
					// in which we are nested, has already set the "leaves_cache" cache,
					// and this cache is copied whenever any map entry changes.
					// *********************************************************************************** //
					std::vector<Leaf> & leaves_cache = branch.leaves_cache;

					// The following cache will only be filled on the first pass
					branch.ConstructChildCombinationCache(*this, leaves_cache, variable_group_number, mappings_from_child_branch_to_primary, mappings_from_child_leaf_to_primary, static_cast<int>(timeSliceLeaf.second.primary_keys.size()));

					// *********************************************************************************** //
					// We have an incoming child variable group branch and leaf.
					// Find all matching output rows that contain the same DMU data on the matching columns.
					// *********************************************************************************** //
					std::map<BranchOutputRow const *, std::vector<int>> const & matchingOutputRows = branch.helper_lookup__from_child_key_set__to_matching_output_rows[dmu_keys];

					// Loop through all matching output rows
					std::for_each(matchingOutputRows.cbegin(), matchingOutputRows.cend(), [&](std::pair<BranchOutputRow const *, std::vector<int>> const & matchingOutputRow)
					{

						BranchOutputRow const * outputRowPtr = matchingOutputRow.first;
						BranchOutputRow const & outputRow = *outputRowPtr;
						std::vector<int> const & matchingOutputChildLeaves = matchingOutputRow.second;

						// Loop through all matching output row child leaves
						std::for_each(matchingOutputChildLeaves.cbegin(), matchingOutputChildLeaves.cend(), [&](int const matching_child_leaf_index)
						{

							std::map<int, std::int64_t> & outputRowLeafIndexToSecondaryDataCacheIndex = outputRow.child_indices_into_raw_data[variable_group_number];
							outputRowLeafIndexToSecondaryDataCacheIndex[matching_child_leaf_index] = timeSliceLeaf.second.index_into_raw_data;

							added = true;

						});

					});

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

	std::for_each(timeSlices.begin(), timeSlices.end(), [&](std::pair<TimeSlice const, VariableGroupTimeSliceData> & timeSliceEntry)
	{

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
			
			BranchesAndLeaves & branchesAndLeaves = variableGroupBranchesAndLeaves.branches_and_leaves;
			Weighting & variableGroupBranchesAndLeavesWeighting = variableGroupBranchesAndLeaves.weighting;
			variableGroupBranchesAndLeavesWeighting.setWeightingRangeStart(currentWeighting);

			std::for_each(branchesAndLeaves.begin(), branchesAndLeaves.end(), [&](std::pair<Branch const, Leaves> & branchAndLeaves)
			{

				Branch const & branch = branchAndLeaves.first;
				Leaves & leaves = branchAndLeaves.second;

				Weighting & branchWeighting = branch.weighting;

				// Count the leaves
				int numberLeaves = static_cast<int>(leaves.size());

				// The number of K-ad combinations for this branch is easily calculated.
				// It is just the binomial coefficient (assuming K <= N)

				branch.number_branch_combinations = 1; // covers K > numberLeaves condition, and numberLeaves == 0 condition
#				ifdef _DEBUG
				branch.number_branch_combinations_string = branch.number_branch_combinations.str();
#				endif
				if (K <= numberLeaves)
				{
					//branch.number_branch_combinations = boost::math::binomial_coefficient<boost::multiprecision::cpp_int>(numberLeaves, K);
					branch.number_branch_combinations = BinomialCoefficient(numberLeaves, K);
#				ifdef _DEBUG
					branch.number_branch_combinations_string = branch.number_branch_combinations.str();
#				endif
				}

				// clear the hits cache
				branch.hits.clear();

				// Holes between time slices are handled here -
				// There is no gap in the sequence of discretized weight values in branches.
				branchWeighting.setWeighting(timeSlice.Width(ms_per_unit_time) * branch.number_branch_combinations);
				branchWeighting.setWeightingRangeStart(currentWeighting);
				currentWeighting += branchWeighting.getWeighting();

				variableGroupBranchesAndLeavesWeighting.addWeighting(branchWeighting.getWeighting());

			});

			variableGroupTimeSliceDataWeighting.addWeighting(variableGroupBranchesAndLeavesWeighting.getWeighting());

		});

		weighting.addWeighting(variableGroupTimeSliceDataWeighting.getWeighting());

	});

}

void AllWeightings::AddNewTimeSlice(int const & variable_group_number, Branch const & branch, TimeSliceLeaf const & newTimeSliceLeaf)
{
	VariableGroupTimeSliceData variableGroupTimeSliceData;
	VariableGroupBranchesAndLeavesVector & variableGroupBranchesAndLeavesVector = variableGroupTimeSliceData.branches_and_leaves;
	VariableGroupBranchesAndLeaves newVariableGroupBranch(variable_group_number);
	BranchesAndLeaves & newBranchesAndLeaves = newVariableGroupBranch.branches_and_leaves;
	newBranchesAndLeaves[branch].emplace(newTimeSliceLeaf.second); // add Leaf to the set of Leaves attached to the new Branch
	variableGroupBranchesAndLeavesVector.push_back(newVariableGroupBranch);
	timeSlices[newTimeSliceLeaf.first] = variableGroupTimeSliceData;
}

void AllWeightings::PrepareRandomNumbers(int how_many)
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
				boost::format msg("Too many output rows have been requested for the given data set.");
				throw NewGeneException() << newgene_error_description(msg.str());
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

	TimeSlices::const_iterator timeSlicePtr = std::lower_bound(timeSlices.cbegin(), timeSlices.cend(), random_number, [&](std::pair<TimeSlice, VariableGroupTimeSliceData> const & timeSliceData, boost::multiprecision::cpp_int const & test_random_number)
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

	BranchesAndLeaves const & branchesAndLeaves = variableGroupBranchesAndLeaves.branches_and_leaves;
		
	// Pick a branch randomly (with weight!)
	BranchesAndLeaves::const_iterator branchesAndLeavesPtr = std::lower_bound(branchesAndLeaves.cbegin(), branchesAndLeaves.cend(), random_number, [&](std::pair<Branch, Leaves> const & testBranchAndLeaves, boost::multiprecision::cpp_int const & test_random_number)
	{
		Branch const & testBranch = testBranchAndLeaves.first;
		if (testBranch.weighting.getWeightingRangeEnd() < test_random_number)
		{
			return true;
		}
		return false;
	});

	const Branch & new_branch = branchesAndLeavesPtr->first;

	Leaves const & tmp_leaves = branchesAndLeavesPtr->second;

	// random_number is now an actual *index* to which combination of leaves in this VariableGroupTimeSliceData
	GetLeafCombination(random_number, K, new_branch, tmp_leaves);

	++random_number_iterator;

	return true;

}

void AllWeightings::GetLeafCombination(boost::multiprecision::cpp_int random_number, int const K, Branch const & branch, Leaves const & leaves)
{

	random_number -= branch.weighting.getWeightingRangeStart();

	boost::multiprecision::cpp_int which_time_unit = random_number / branch.number_branch_combinations;

	static int saved_range = -1;
	static std::mt19937 engine(static_cast<std::int32_t>(std::time(0)));

	BranchOutputRow test_leaf_combination;

	bool skip = false;
	if (static_cast<size_t>(K) >= leaves.size())
	{
		skip = true;
		for (int n = 0; n < leaves.size(); ++n)
		{
			test_leaf_combination.Insert(n);
		}
		test_leaf_combination.SaveCache();
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
					PopulateAllLeafCombinations(which_time_unit, K, branch, leaves);
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
				for (size_t n = 0; n < leaves.size(); ++n)
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

				test_leaf_combination.SaveCache();

			}

		}

	}

	// It might easily be a duplicate - random sampling will produce multiple hits on the same row
	// because some rows can have a heavier weight than other rows;
	// this is handled by storing a map of every *time unut* (corresponding to the primary variable group)
	// and all leaf combinations that have been hit for that time unit.
	branch.hits[which_time_unit].insert(test_leaf_combination);

}

void AllWeightings::PopulateAllLeafCombinations(boost::multiprecision::cpp_int const & which_time_unit, int const K, Branch const & branch, Leaves const & leaves)
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
		bool succeeded = IncrementPosition(K, position, leaves);

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
	new_remaining.SaveCache();

	if (branch.hits[which_time_unit].count(new_remaining) == 0)
	{
		branch.remaining[which_time_unit].push_back(new_remaining);
	}

}

bool AllWeightings::IncrementPosition(int const K, std::vector<int> & position, Leaves const & leaves)
{

	int sub_k_being_managed = K;
	
	int new_leaf = IncrementPositionManageSubK(K, sub_k_being_managed, position, leaves);

	if (new_leaf == -1)
	{
		// No more positions!
		return false;
	}

	return true;

}

int AllWeightings::IncrementPositionManageSubK(int const K, int const subK_, std::vector<int> & position, Leaves const & leaves)
{

	int number_leaves = static_cast<int>(leaves.size());

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
		int previous_k_new_leaf = IncrementPositionManageSubK(K, subK, position, leaves);
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

void AllWeightings::ResetBranchCaches(bool const empty_all)
{

	std::for_each(timeSlices.begin(), timeSlices.end(), [&](std::pair<TimeSlice const, VariableGroupTimeSliceData> & timeSliceData)
	{

		TimeSlice const & timeSlice = timeSliceData.first;
		VariableGroupTimeSliceData & variableGroupTimeSliceData = timeSliceData.second;

		VariableGroupBranchesAndLeavesVector & variableGroupBranchesAndLeavesVector = variableGroupTimeSliceData.branches_and_leaves;

		// For now, assume only one variable group
		if (variableGroupBranchesAndLeavesVector.size() > 1)
		{
			boost::format msg("Only one top-level variable group is currently supported for the random and full sampler in ConsolidateHits().");
			throw NewGeneException() << newgene_error_description(msg.str());
		}

		VariableGroupBranchesAndLeaves & variableGroupBranchesAndLeaves = variableGroupBranchesAndLeavesVector[0];
		BranchesAndLeaves & branchesAndLeaves = variableGroupBranchesAndLeaves.branches_and_leaves;

		std::for_each(branchesAndLeaves.begin(), branchesAndLeaves.end(), [&](std::pair<Branch const, Leaves> & branchAndLeaves)
		{

			Branch const & branch = branchAndLeaves.first;
			Leaves const & leaves = branchAndLeaves.second;
			branch.helper_lookup__from_child_key_set__to_matching_output_rows.clear();

			if (empty_all)
			{
				branch.leaves_cache.clear();
			}
			else
			{
				branch.CreateLeafCache(leaves);
			}

		});

	});

}

void PrimaryKeysGroupingMultiplicityOne::ConstructChildCombinationCache(AllWeightings & allWeightings, std::vector<Leaf> & leaves_cache, int const variable_group_number, std::vector<ChildToPrimaryMapping> mappings_from_child_branch_to_primary, std::vector<ChildToPrimaryMapping> mappings_from_child_leaf_to_primary, int const number_columns_in_one_child_leaf, bool const force) const
{

	if (force || helper_lookup__from_child_key_set__to_matching_output_rows.empty())
	{

		// The cache has yet to be filled, or we are specifically being requested to refresh it

		ChildDMUInstanceDataVector child_hit_vector_branch_components;
		ChildDMUInstanceDataVector child_hit_vector;
		std::for_each(hits.cbegin(), hits.cend(), [&](std::pair<boost::multiprecision::cpp_int, std::set<BranchOutputRow>> const & time_unit_output_rows)
		{

			for (std::set<BranchOutputRow>::const_iterator outputRowPtr = time_unit_output_rows.second.cbegin(); outputRowPtr != time_unit_output_rows.second.cend(); ++outputRowPtr)
			{

				BranchOutputRow const & outputRow = *outputRowPtr;

				// We have a new hit we're dealing with
				child_hit_vector_branch_components.clear();

				// First in the "child DMU" vector are the child's BRANCH DMU values
				std::for_each(mappings_from_child_branch_to_primary.cbegin(), mappings_from_child_branch_to_primary.cend(), [&](ChildToPrimaryMapping const & childToPrimaryMapping)
				{

					// We have the next DMU data in the sequence of DMU's for the child branch/leaf (we're still working on the branch)

					switch (childToPrimaryMapping.mapping)
					{

						case CHILD_TO_PRIMARY_MAPPING__MAPS_TO_BRANCH:
						{

							// The next DMU in the child branch's DMU sequence maps to a branch in the top-level DMU sequence
							child_hit_vector_branch_components.push_back(DMUInstanceData(primary_keys[childToPrimaryMapping.index]));

						}
						break;

						case CHILD_TO_PRIMARY_MAPPING__MAPS_TO_LEAF:
						{

							// leaf_number tells us which leaf
							// index tells us which index in that leaf

							// The next DMU in the child branch's DMU sequence maps to a leaf in the top-level DMU sequence
							child_hit_vector_branch_components.push_back(DMUInstanceData(leaves_cache[outputRow.primary_leaves_cache[childToPrimaryMapping.leaf_number]].primary_keys[childToPrimaryMapping.index]));

						}
						break;

					default:
						{}
						break;

					}

				});

				// Next in the "child DMU" vector are the child's LEAF DMU values
				int child_leaf_index_crossing_multiple_child_leaves = 0;
				int child_leaf_index_within_a_single_child_leaf = 0;
				int current_child_leaf_number = 0;
				child_hit_vector.clear();
				child_hit_vector.insert(child_hit_vector.begin(), child_hit_vector_branch_components.begin(), child_hit_vector_branch_components.end());
				std::for_each(mappings_from_child_leaf_to_primary.cbegin(), mappings_from_child_leaf_to_primary.cend(), [&](ChildToPrimaryMapping const & childToPrimaryMapping)
				{

					// We have the next DMU data in the sequence of DMU's for the child branch/leaf (we're still working on the branch)

					switch (childToPrimaryMapping.mapping)
					{

						case CHILD_TO_PRIMARY_MAPPING__MAPS_TO_BRANCH:
						{

							// The next DMU in the child branch's DMU sequence maps to a branch in the top-level DMU sequence
							child_hit_vector.push_back(DMUInstanceData(primary_keys[childToPrimaryMapping.index]));

						}
						break;

					case CHILD_TO_PRIMARY_MAPPING__MAPS_TO_LEAF:
						{

							// leaf_number tells us which leaf
							// index tells us which index in that leaf

							// The next DMU in the child branch's DMU sequence maps to a leaf in the top-level DMU sequence
							child_hit_vector.push_back(DMUInstanceData(leaves_cache[outputRow.primary_leaves_cache[childToPrimaryMapping.leaf_number]].primary_keys[childToPrimaryMapping.index]));

						}
						break;

					default:
						{}
						break;

					}

					++child_leaf_index_crossing_multiple_child_leaves;
					++child_leaf_index_within_a_single_child_leaf;
					if (child_leaf_index_within_a_single_child_leaf == number_columns_in_one_child_leaf)
					{
						helper_lookup__from_child_key_set__to_matching_output_rows[child_hit_vector][&outputRow].push_back(current_child_leaf_number);

						++current_child_leaf_number;
						child_leaf_index_within_a_single_child_leaf = 0;
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

	Branch branch;
	Leaves leaves;
	TimeSlice time_slice;
	BranchOutputRow outputRow;
	while (RetrieveNextBranchAndLeaves(K))
	{
	}

}
