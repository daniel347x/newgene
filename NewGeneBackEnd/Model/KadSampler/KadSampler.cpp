#include "KadSampler.h"

#ifndef Q_MOC_RUN
	#include <boost/scope_exit.hpp>
	#include <boost/math/special_functions/binomial.hpp>
	#include <boost/multiprecision/random.hpp>
	#include <boost/multiprecision/cpp_dec_float.hpp>
	#include <boost/assert.hpp>
#endif
#include <random>
#include <functional>
#include <list>
#include <algorithm>
#include "../../Utilities/TimeRangeHelper.h"
#include <fstream>

bool * __cancelled = nullptr;

std::fstream * create_output_row_visitor::output_file = nullptr;
int create_output_row_visitor::mode = static_cast<int>(create_output_row_visitor::CREATE_ROW_MODE__NONE);
InstanceDataVector<hits_tag> * create_output_row_visitor::data = nullptr;
int * create_output_row_visitor::bind_index = nullptr;
sqlite3_stmt * create_output_row_visitor::insert_stmt = nullptr;
bool MergedTimeSliceRow_RHS_wins = false;
std::string create_output_row_visitor::row_in_process;
fast_string create_output_row_visitor::converted_value;

int Weighting::how_many_weightings = 0;

bool VariableGroupTimeSliceData::when_destructing_do_not_delete = false;

bool CheckCancelled()
{
	return *__cancelled;
}

KadSampler::KadSampler(Messager & messager_, bool & cancelled_)
	: insert_random_sample_stmt{ nullptr }
	, numberChildVariableGroups{ 0 }
	, time_granularity{ TIME_GRANULARITY__NONE }
	, random_rows_added{ 0 }
	, messager{ messager_ }
	, current_child_variable_group_being_merged{ -1 }
	, number_branch_columns{ 0 }
	, number_primary_variable_group_single_leaf_columns{ 0 }
	, debuggingflag{ false }
	, rowsWritten{ 0 }
	, cancelled{ cancelled_ }
{
	__cancelled = &cancelled;
}

KadSampler::~KadSampler()
{
	// Never called: This is how we utilize the Boost memory pool
}

std::tuple<bool, bool, TimeSlices<hits_tag>::iterator> KadSampler::HandleIncomingNewBranchAndLeaf(Branch const & branch, TimeSliceLeaf & newTimeSliceLeaf,
		int const & variable_group_number, VARIABLE_GROUP_MERGE_MODE const merge_mode, bool const consolidate_rows, bool const random_sampling,
		TimeSlices<hits_tag>::iterator mapIterator_, bool const useIterator)
{

	TimeSlice const & newTimeSlice = newTimeSliceLeaf.first;

	newTimeSlice.Validate();

	// "added" has one purpose:
	// It determines whether to add the SECONDARY data
	// into the (class-)global cache (keyed off rowid,
	// which is always available, even for branch-only primary variable group data)
	// (whether child, top-level, or primary).
	//
	// The actual leaf DMU values are irrelevant.
	// The secondary values are all that matter, and they
	// are added to the cache based on the rowid.
	bool added = false; // true if there is a match

	if (merge_mode == VARIABLE_GROUP_MERGE_MODE__PRIMARY)
	{
		// Always add data for primary variable group
		//added = true; // Not with Limit DMU functionality!!!
	}

	// determine which case we are in terms of the relationship of the incoming new 'timeSliceLeaf'
	// so that we know whether/how to break up either the new 'rhs' time slice
	// or the potentially overlapping existing time slices in the 'TimeSlices<hits_tag>' map

	// Save beginning, one past end time slices in the existing map for reference
	TimeSlices<hits_tag>::iterator existing_start_slice = timeSlices.begin();
	TimeSlices<hits_tag>::iterator existing_one_past_end_slice = timeSlices.end();
	TimeSlices<hits_tag>::iterator mapIterator;

	if (useIterator)
	{
		mapIterator = mapIterator_;
	}

	bool normalCase = false;

	if (existing_start_slice == existing_one_past_end_slice)
	{
		if (merge_mode == VARIABLE_GROUP_MERGE_MODE__PRIMARY)
		{
			// No entries in the 'TimeSlices<hits_tag>' map yet
			// Add the entire new time slice as the first entry in the map
			bool did_add = AddNewTimeSlice(variable_group_number, branch, newTimeSliceLeaf);

			if (did_add)
			{
				added = true;
			}
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

		if (!useIterator)
		{
			mapIterator = std::upper_bound(timeSlices.begin(), timeSlices.end(), newTimeSliceLeaf, &KadSampler::is_map_entry_end_time_greater_than_new_time_slice_start_time);
		}

		TimeSlices<hits_tag>::iterator startMapSlicePtr = mapIterator;
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
				bool did_add = AddNewTimeSlice(variable_group_number, branch, newTimeSliceLeaf);

				if (did_add)
				{
					added = true;
				}
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
						bool did_add = AddNewTimeSlice(variable_group_number, branch, newTimeSliceLeaf);

						if (did_add)
						{
							added = true;
						}
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
						bool did_add = AddNewTimeSlice(variable_group_number, branch, new_left_slice);

						if (did_add)
						{
							added = true;
						}
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

	bool call_again = false;

	if (normalCase)
	{
		// This is the normal case.  The incoming new slice is guaranteed to have
		// its left edge either equal to the left edge of the map element,
		// or to the right of the left edge of the map element but less than the right edge of the map element.
		// The right edge of the incoming new slice could lie anywhere greater than the left edge -
		// it could lie either inside the map element, at the right edge of the map element,
		// or past the right edge of the map element.
		bool no_more_time_slice = HandleTimeSliceNormalCase(added, branch, newTimeSliceLeaf, mapIterator, variable_group_number, merge_mode, consolidate_rows,
								  random_sampling);

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
				bool did_add = AddNewTimeSlice(variable_group_number, branch, newTimeSliceLeaf);

				if (did_add)
				{
					added = true;
				}
			}
		}
		else
		{
			// We have a chunk left and there is at least one map element to the right of this
			// remaining chunk's starting point.
			// That one map element could start anywhere to the right, though...
			// there could be a gap.
			call_again = true;
		}
	}

	return std::make_tuple(added, call_again, mapIterator);

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
bool KadSampler::HandleTimeSliceNormalCase(bool & added, Branch const & branch, TimeSliceLeaf & newTimeSliceLeaf, TimeSlices<hits_tag>::iterator & mapElementPtr,
		int const & variable_group_number, VARIABLE_GROUP_MERGE_MODE const merge_mode, bool const consolidate_rows, bool const random_sampling)
{

	bool added_new = false;

	TimeSlice const & newTimeSlice = newTimeSliceLeaf.first;
	TimeSlice const & mapElement = mapElementPtr->first;

	TimeSlices<hits_tag>::iterator newMapElementLeftPtr;
	TimeSlices<hits_tag>::iterator newMapElementMiddlePtr;
	TimeSlices<hits_tag>::iterator newMapElementRightPtr;

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

			SliceMapEntry(mapElementPtr, newTimeSlice.getEnd(), newMapElementLeftPtr, newMapElementRightPtr, consolidate_rows, random_sampling);
			added_new = MergeNewDataIntoTimeSlice(branch, newTimeSliceLeaf, newMapElementLeftPtr, variable_group_number, merge_mode);

			mapElementPtr = newMapElementLeftPtr;

			newTimeSliceEatenCompletelyUp = true;

		}
		else if (newTimeSlice.IsEndEqualToRHSEnd(mapElement))
		{

			// The new time slice exactly matches the first map element.

			// Merge the new time slice with the map element.

			added_new = MergeNewDataIntoTimeSlice(branch, newTimeSliceLeaf, mapElementPtr, variable_group_number, merge_mode);

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

			added_new = MergeNewDataIntoTimeSlice(branch, new_left_slice, mapElementPtr, variable_group_number, merge_mode);

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

			SliceMapEntry(mapElementPtr, newTimeSlice.getStart(), newTimeSlice.getEnd(), newMapElementMiddlePtr, consolidate_rows, random_sampling);
			added_new = MergeNewDataIntoTimeSlice(branch, newTimeSliceLeaf, newMapElementMiddlePtr, variable_group_number, merge_mode);

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

			SliceMapEntry(mapElementPtr, newTimeSlice.getStart(), newMapElementLeftPtr, newMapElementRightPtr, consolidate_rows, random_sampling);
			added_new = MergeNewDataIntoTimeSlice(branch, newTimeSliceLeaf, newMapElementRightPtr, variable_group_number, merge_mode);

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
			SliceMapEntry(mapElementPtr, new_left_slice.first.getStart(), newMapElementLeftPtr, newMapElementRightPtr, consolidate_rows, random_sampling);
			added_new = MergeNewDataIntoTimeSlice(branch, new_left_slice, newMapElementRightPtr, variable_group_number, merge_mode);

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
void KadSampler::SliceMapEntry(TimeSlices<hits_tag>::iterator const & existingMapElementPtr, std::int64_t const middle, TimeSlices<hits_tag>::iterator & newMapElementLeftPtr,
							   TimeSlices<hits_tag>::iterator & newMapElementRightPtr, bool const consolidate_rows, bool const random_sampling)
{

	TimeSlice timeSlice = existingMapElementPtr->first;
	TimeSlice originalMapTimeSlice = timeSlice;
	VariableGroupTimeSliceData const timeSliceData(std::move(existingMapElementPtr->second));

	timeSlices.erase(existingMapElementPtr);

	timeSlice = originalMapTimeSlice;
	timeSlice.setEnd(middle);
	timeSlices[timeSlice] = timeSliceData;
	timeSlices[timeSlice].PruneTimeUnits(*this, originalMapTimeSlice, timeSlice, consolidate_rows, random_sampling);

	newMapElementLeftPtr = timeSlices.find(timeSlice);

	timeSlice = originalMapTimeSlice;
	timeSlice.setStart(middle);
	timeSlices[timeSlice] = timeSliceData;
	timeSlices[timeSlice].PruneTimeUnits(*this, originalMapTimeSlice, timeSlice, consolidate_rows, random_sampling);

	newMapElementRightPtr = timeSlices.find(timeSlice);

}

// breaks an existing map entry into three pieces and returns an iterator to the middle piece
void KadSampler::SliceMapEntry(TimeSlices<hits_tag>::iterator const & existingMapElementPtr, std::int64_t const left, std::int64_t const right,
							   TimeSlices<hits_tag>::iterator & newMapElementMiddlePtr, bool const consolidate_rows, bool const random_sampling)
{

	TimeSlice timeSlice = existingMapElementPtr->first;
	TimeSlice originalMapTimeSlice = timeSlice;
	VariableGroupTimeSliceData const timeSliceData(std::move(existingMapElementPtr->second));

	timeSlices.erase(existingMapElementPtr);

	timeSlice = originalMapTimeSlice;
	timeSlice.setEnd(left);
	timeSlices[timeSlice] = timeSliceData;
	timeSlices[timeSlice].PruneTimeUnits(*this, originalMapTimeSlice, timeSlice, consolidate_rows, random_sampling);

	timeSlice = originalMapTimeSlice;
	timeSlice.Reshape(left, right);
	timeSlices[timeSlice] = timeSliceData;
	timeSlices[timeSlice].PruneTimeUnits(*this, originalMapTimeSlice, timeSlice, consolidate_rows, random_sampling);

	newMapElementMiddlePtr = timeSlices.find(timeSlice);

	timeSlice = originalMapTimeSlice;
	timeSlice.setStart(right);
	timeSlices[timeSlice] = timeSliceData;
	timeSlices[timeSlice].PruneTimeUnits(*this, originalMapTimeSlice, timeSlice, consolidate_rows, random_sampling);

}

// Slices off the left part of the "incoming_slice" TimeSliceLeaf and returns it in the "new_left_slice" TimeSliceLeaf.
// The "incoming_slice" TimeSliceLeaf is adjusted to become equal to the remaining part on the right.
void KadSampler::SliceOffLeft(TimeSliceLeaf & incoming_slice, std::int64_t const slicePoint, TimeSliceLeaf & new_left_slice)
{
	new_left_slice = incoming_slice;
	new_left_slice.first.setEnd(slicePoint);
	incoming_slice.first.setStart(slicePoint);
}

// Merge time slice data into a map element
bool KadSampler::MergeNewDataIntoTimeSlice(Branch const & incoming_variable_group_branch, TimeSliceLeaf const & incoming_variable_group_time_slice_leaf,
		TimeSlices<hits_tag>::iterator & mapElementPtr, int const & variable_group_number,
		VARIABLE_GROUP_MERGE_MODE const merge_mode)
{

	// ******************************************************************************************************************************** //
	// ******************************************************************************************************************************** //
	//
	// If we reach this function, the time slice of the leaf is guaranteed to match the time slice of the map element
	// it is being merged into.
	// We do not check this here.
	//
	// ******************************************************************************************************************************** //
	// ******************************************************************************************************************************** //

	bool added = false;

	VariableGroupTimeSliceData & variableGroupTimeSliceData = mapElementPtr->second;
	VariableGroupBranchesAndLeavesVector<hits_tag> & variableGroupBranchesAndLeavesVector = *variableGroupTimeSliceData.branches_and_leaves;

	// Note: Currently, only one primary top-level variable group is supported.
	// It will be the "begin()" element, if it exists.
	VariableGroupBranchesAndLeavesVector<hits_tag>::iterator PrimaryVariableGroupBranchesAndLeavesPtr = variableGroupBranchesAndLeavesVector.begin();

	if (PrimaryVariableGroupBranchesAndLeavesPtr == variableGroupBranchesAndLeavesVector.end())
	{

		// add new variable group entry,
		// and then add new branch corresponding to this variable group

		// This case will only be hit for the primary variable group!

		if (merge_mode == VARIABLE_GROUP_MERGE_MODE__PRIMARY)
		{
			VariableGroupBranchesAndLeaves newPrimaryVariableGroupBranch(variable_group_number);
			Branches<hits_tag> & newBranchesAndLeaves = newPrimaryVariableGroupBranch.branches;

			// ****************************************************************************************************************** //
			// The leaf already contains lookup data into the PRIMARY variable group's cache
			// ****************************************************************************************************************** //
			bool did_add = incoming_variable_group_branch.InsertLeaf(incoming_variable_group_time_slice_leaf.second); // add Leaf to the set of Leaves attached to the new Branch

			// ****************************************************************************************************************** //
			// This needs to always be added - even if it does not exist yet AND the leaf is invalid due to "Limit DMU" options -
			// so that if a VALID leaf is ever added in the future, it knows that an INVALID leaf was attempted
			// ****************************************************************************************************************** //
			newBranchesAndLeaves.insert(incoming_variable_group_branch);
			variableGroupBranchesAndLeavesVector.push_back(newPrimaryVariableGroupBranch);

			if (did_add)
			{
				added = true;
			}
		}

		else
		{
			boost::format msg("Logic error: Attempting to add a new branch when merging a non-primary variable group!");
			throw NewGeneException() << newgene_error_description(msg.str());
		}

	}
	else
	{

		// Branches already exists.
		// The incoming branch might match one of these, or it might not.
		// In any case, retrieve the existing set of branches.

		VariableGroupBranchesAndLeaves & primaryVariableGroupBranchesAndLeaves = *PrimaryVariableGroupBranchesAndLeavesPtr;
		Branches<hits_tag> & primaryVariableGroupBranches = primaryVariableGroupBranchesAndLeaves.branches;

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
					auto primaryVariableGroupBranchPtr = primaryVariableGroupBranches.find(incoming_variable_group_branch);

					if (primaryVariableGroupBranchPtr == primaryVariableGroupBranches.cend())
					{
						auto inserted = primaryVariableGroupBranches.insert(incoming_variable_group_branch);
						primaryVariableGroupBranchPtr = inserted.first;
					}

					// ****************************************************************************************************************** //
					// The leaf already contains lookup data into the PRIMARY variable group's cache
					// ****************************************************************************************************************** //
					bool did_add = primaryVariableGroupBranchPtr->InsertLeaf(
									   incoming_variable_group_time_slice_leaf.second); // add Leaf to the set of Leaves attached to the new Branch, if one doesn't already exist there

					if (did_add)
					{
						added = true;
					}

				}
				break;

			case VARIABLE_GROUP_MERGE_MODE__TOP_LEVEL:
				{

					// Let's take a peek and see if our branch is already present

					Branches<hits_tag>::iterator primaryVariableGroupBranchPtr = primaryVariableGroupBranches.find(incoming_variable_group_branch);

					if (primaryVariableGroupBranchPtr != primaryVariableGroupBranches.end())
					{

						// *********************************************************************************** //
						// The incoming branch *does* already exist!
						// We want to see if this branch contains the incoming leaf, or not.
						// *********************************************************************************** //

						Branch const & current_primary_variable_group_branch = *primaryVariableGroupBranchPtr;

						// If the leaf does not exist, don't add it ... we're merging a non-primary group,
						// which should never add new leaves.
						// Instead, the only purpose of the merge is to set data for any *existing* leaves.
						//
						// Note regarding the Limit DMU functionality:
						// If the branch itself has a restricted DMU member,
						// this function will never be called.
						// If there are no leaf slots in the given UOA
						// but the branch is completely valid
						// (i.e., has no restricted DMU's), then we will be called
						// in the usual way with an empty leaf (a leaf with no DMU's)
						// which will MATCH in the operator<() function used in the doesLeafExist() implementation,
						// and which will then pass its index to use for secondary data
						// as normal inside the following block.
						// If the branch is valid and there ARE leaf slots (in which case
						// there must be at least two leaf slots), then
						// "doesLeafExist()" will return false because the PRIMARY
						// variable group, which preceded this, will not have
						// added the leaf.
						if (current_primary_variable_group_branch.doesLeafExist(incoming_variable_group_time_slice_leaf.second))
						{

							// This branch *does* contain the incoming leaf!

							// For any given leaf, including the "empty" leaf with no DMU columns
							// (corresponding to the top-level UOA being all branch and no leaves with outer K=1)
							// this function should only be called once - assuming the raw data
							// is valid (i.e., assuming that the raw data contains only one row
							// for the given time slice with a unique set of branch + leaf DMU values).
							// If the raw data - which is out of our control and is imported by the user -
							// contains multiple rows for the same keys in the same time slice,
							// then the last row loaded wins.

							// Set the data in the leaf for this non-primary top-level variable group.

							// Note that many different OUTPUT ROWS might reference this leaf;
							// perhaps even multiple times within a single output row.  Fine!

							// pass the index into the raw data for the current variable group being merged
							// from the incoming leaf (which contains only the index for the
							// current top-level variable group being merged in)
							// into the active leaf saved in the AllWeightings instance, and used to construct the output rows.
							// (This active leaf may also have been called previously to set other top-level variable group rows.)
							//
							// Note that in the case of NO LEAF, there is still a single "leaf" (with no DMU associated with it)
							// that nonetheless contains the data index lookup into the raw data for this branch
							// (the branch, in this case, having only one possible data lookup).
							current_primary_variable_group_branch.setTopGroupIndexIntoRawData(incoming_variable_group_time_slice_leaf.second, variable_group_number,
									incoming_variable_group_time_slice_leaf.second.other_top_level_indices_into_raw_data[variable_group_number]);

							added = true;

						}

					}

				}
				break;

			case VARIABLE_GROUP_MERGE_MODE__CHILD:
				{

					// *********************************************************************************** //
					// *********************************************************************************** //
					//
					// Output rows have ALREADY BEEN GENERATED,
					// and all of these output rows contain VALID (not limited) DMU KEYS
					// (in all primary branch and leaf slots that are represented in leaves_cache for the branch)
					//
					// ... Also the helper_lookup__from_child_key_set__to_matching_output_rows cache
					// has already been populated and again contains only VALID (not limited) DMU values.
					//
					// So the LIMIT DMU functionality is fully covered here.  See further notes below.
					//
					// *********************************************************************************** //
					// *********************************************************************************** //

					// Create a vector containing all of the child's DMU keys,
					// both child branch and child leaf (there being only one
					// child leaf in the incoming data, because it's straight from the raw data table,
					// even if there are multiple child leaf slots in the output data for this child's UOA

					ChildDMUInstanceDataVector<hits_tag> all_dmu_keys_child;

					all_dmu_keys_child.insert(all_dmu_keys_child.end(), incoming_variable_group_branch.primary_keys.begin(), incoming_variable_group_branch.primary_keys.end());
					all_dmu_keys_child.insert(all_dmu_keys_child.end(), incoming_variable_group_time_slice_leaf.second.primary_keys.begin(),
											  incoming_variable_group_time_slice_leaf.second.primary_keys.end());

					// *********************************************************************************** //
					// Loop through all BRANCHES for the PRIMARY variable group in this time slice
					// (not the branches in the child variable group).
					// For each, we have a set of output rows.
					// *********************************************************************************** //

					for (auto primaryVariableGroupBranchesPtr = primaryVariableGroupBranches.begin(); primaryVariableGroupBranchesPtr != primaryVariableGroupBranches.end();
						 ++primaryVariableGroupBranchesPtr)
					{

						if (CheckCancelled()) { break; }

						Branch const & the_current_primary_variable_group_branch = *primaryVariableGroupBranchesPtr;

						// *********************************************************************************** //
						// "leaves_cache" is a vector cache containing the same leaves in the same order
						// as the official "leaves" set containing the leaves for the current branch.
						//
						// Note that a call to "ResetBranchCaches()" previous to the high-level call to "HandleBranchAndLeaf()",
						// in which we are nested, has already set the "leaves_cache" cache,
						// and this cache is copied whenever any map entry changes.
						// *********************************************************************************** //

						// The following cache will only be filled on the first pass
						//the_current_primary_variable_group_branch.ConstructChildCombinationCache(*this, variable_group_number, false);

						// *********************************************************************************** //
						// We have an incoming child variable group branch and leaf.
						// Find all matching output rows that contain the same DMU data on the matching columns.
						// *********************************************************************************** //
						// ... never use "consolidating rows" version here, because consolidation always happens after all rows are merged
						//
						// ****************************************************************************************************************** //
						// "helper_lookup__from_child_key_set__to_matching_output_rows" CAN NEVER HAVE MAP KEYS
						// THAT ARE BLANKS -
						// every child [branch + leaf] MUST map to real values
						// ****************************************************************************************************************** //
						if (the_current_primary_variable_group_branch.helper_lookup__from_child_key_set__to_matching_output_rows == nullptr)
						{
							;							boost::format msg("Null child DMU key lookup cache.");
							throw NewGeneException() << newgene_error_description(msg.str());
						}

						// Different memory pools may be used, templatized on the memory pool,
						// so a simple "find" will not compile.
						//auto const & matchingOutputRows = the_current_primary_variable_group_branch.helper_lookup__from_child_key_set__to_matching_output_rows->find(dmu_keys);

						// ****************************************************************************************************************** //
						//
						// See "ConstructChildCombinationCache()"
						// ... for the filling of the "helper_lookup__from_child_key_set__to_matching_output_rows" map.
						//
						// This map allows us to pull up all of the output rows for which the current child data being merged matches
						// somewhere in that output row
						// (a SINGLE child branch + leaf match, even if there are MULTIPLE child leaf slots in the output row).
						// Once the output rows are pulled up (in a single data structure),
						// in turn they are looped through, and within each of THOSE is a vector that lists
						// the CHILD LEAF index/es within that given output row that matches the incoming child data.
						//
						// ****************************************************************************************************************** //

						// Also, std::binary_search only returns true or false, not an iterator to the match, if any,
						// so this must be followed up by std::lower_bound if there is a match, which does.

						// ****************************************************************************************************************** //
						// "helper_lookup__from_child_key_set__to_matching_output_rows" CAN NEVER HAVE MAP KEYS
						// THAT ARE BLANKS -
						// every child [branch + leaf] MUST map to real values
						// ****************************************************************************************************************** //
						bool found = std::binary_search(the_current_primary_variable_group_branch.helper_lookup__from_child_key_set__to_matching_output_rows->cbegin(),
														the_current_primary_variable_group_branch.helper_lookup__from_child_key_set__to_matching_output_rows->cend(), all_dmu_keys_child);

						if (found)
						{

							// ****************************************************************************************************************** //
							// "helper_lookup__from_child_key_set__to_matching_output_rows" CAN NEVER HAVE MAP KEYS
							// THAT ARE BLANKS -
							// every child [branch + leaf] MUST map to real values
							// ****************************************************************************************************************** //
							auto const & matchingOutputRows = std::lower_bound(the_current_primary_variable_group_branch.helper_lookup__from_child_key_set__to_matching_output_rows->cbegin(),
															  the_current_primary_variable_group_branch.helper_lookup__from_child_key_set__to_matching_output_rows->cend(), all_dmu_keys_child);

							// Loop through all matching output rows
							for (auto matchingOutputRowPtr = matchingOutputRows->second.cbegin(); matchingOutputRowPtr != matchingOutputRows->second.cend(); ++matchingOutputRowPtr)
							{

								if (CheckCancelled()) { break; }

								BranchOutputRow<hits_tag> const & outputRow = *matchingOutputRowPtr->first;
								auto const & matchingOutputChildLeaves = matchingOutputRowPtr->second;

								// matchingOutputChildLeaves is a vector

								// Loop through all matching output row CHILD leaves
								// (This is *NOT* a loop through PRIMARY variable group leaves)
								for (auto matchingOutputChildLeavesPtr = matchingOutputChildLeaves.cbegin(); matchingOutputChildLeavesPtr != matchingOutputChildLeaves.cend(); ++matchingOutputChildLeavesPtr)
								{

									if (CheckCancelled()) { break; }

									std::int16_t const & matching_child_leaf_index = *matchingOutputChildLeavesPtr;

									auto const found = outputRow.child_indices_into_raw_data.find(variable_group_number);

									if (found == outputRow.child_indices_into_raw_data.cend())
									{
										outputRow.child_indices_into_raw_data[variable_group_number] = fast_short_to_int_map__loaded<hits_tag>();
									}

									// ****************************************************************************************************************** //
									//
									// In the following code, we simply add the index into the global cache of child secondary data
									// to the current output row's map (using as key the child leaf index).
									//
									// Later, when we output the row, we simply look at the output row's map,
									// and for each key-value pair we know both the index of the child leaf to populate in the output,
									// and the place to look for the data that goes there.
									//
									// ****************************************************************************************************************** //

									auto & outputRowLeafIndexToSecondaryDataCacheIndex = outputRow.child_indices_into_raw_data[variable_group_number];
									outputRowLeafIndexToSecondaryDataCacheIndex[matching_child_leaf_index] = incoming_variable_group_time_slice_leaf.second.index_into_raw_data;

									added = true;

								}

								if (CheckCancelled()) { break; }

							}

							if (CheckCancelled()) { break; }

						}

					}

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

void KadSampler::CalculateWeightings(int const K)
{

	// first, calculate the number of branches that need to have weightings calculated
	std::int64_t total_branches = 0;
	std::for_each(timeSlices.begin(), timeSlices.end(), [&](TimeSlices<hits_tag>::value_type & timeSliceEntry)
	{
		if (CheckCancelled()) { return; }

		VariableGroupTimeSliceData & variableGroupTimeSliceData = timeSliceEntry.second;
		VariableGroupBranchesAndLeavesVector<hits_tag> & variableGroupBranchesAndLeavesVector = *variableGroupTimeSliceData.branches_and_leaves;
		Weighting & variableGroupTimeSliceDataWeighting = variableGroupTimeSliceData.weighting;
		std::for_each(variableGroupBranchesAndLeavesVector.begin(), variableGroupBranchesAndLeavesVector.end(), [&](VariableGroupBranchesAndLeaves & variableGroupBranchesAndLeaves)
		{
			Branches<hits_tag> & branchesAndLeaves = variableGroupBranchesAndLeaves.branches;
			total_branches += static_cast<std::int64_t>(branchesAndLeaves.size());
		});
	});

	if (CheckCancelled()) { return; }

	// Now calculate the weightings
	weighting.setWeighting(0);
	weighting.setWeightingRangeStart(0);

	// Also perform a special calculation -
	// the total number of CONSOLIDATED rows.
	// For this calculation, we must stash away into a temporary set (managed by the Boost memory pool because the standard allocator slows NewGene way down when exiting the function JUST calling the destructors of the elements of the set)
	// all of the branch + leaves combinations so that they only appear ONCE, and then calculate the total number of combinations.
	// This is because currently NewGene stores all data broken down by TIME SLICES, then internal to each time slice
	// are the set of BRANCHES for that time slice, and then internal to each branch are the set of TIME UNITS for that time slice
	// (each single time unit corresponds to the time granularity of the unit of analysis of the primary variable group;
	//  i.e., day for COW, year for Maoz, etc.)
	// ... with possibly a fraction of a time unit at either end of the time slice AFTER other variable groups have been merged in.
	// Therefore, multiple contiguous TIME SLICES could have the same branch + leaf combinations,
	// and these will be MERGED during consolidation.
	// So, the current approach is simply to store all branch + leaves combinations in a set, which guarantees uniqueness,
	// and then iterate through the set to calculate the total number of consolidated K-ad combinations.
	// The issue of whether time slices are adjacent or not is also handled
	// (if not, rows should not be consolidated).
	//
	// In terms of memory allocation, because we are just storing a single entry in the set for ALL leaves for a given branch,
	// i.e., for each MID we store only the total leaves for a given branch (not broken down into K-ad combinations),
	// we expect this to be light on memory in almost all use cases.
	//
	// Also use this custom sort to ensure holes and subsets are handled properly, as noted previously.
	//
	// Place the following vector into the 'hits_tag' memory pool, NOT the 'calculate_consolidated_total_number_rows_tag' memory pool.
	// That way, when this block exits, even though the 'calculate_consolidated_total_number_rows_tag' memory pool has been purged,
	// the following vector will still be valid.
	// (It will be purged when the 'hits_tag' memory pool is purged, which only happens once - at the end of the entire K-ad run.)
	auto calculateTotalConsolidatedRowsSortFunction =
		boost::function<bool(InstanceDataVector<calculate_consolidated_total_number_rows_tag> const & lhs, InstanceDataVector<calculate_consolidated_total_number_rows_tag> const & rhs)>([&](
					InstanceDataVector<calculate_consolidated_total_number_rows_tag> const & lhs, InstanceDataVector<calculate_consolidated_total_number_rows_tag> const & rhs) -> bool
	{

		if (CheckCancelled()) { return false; }

		if (lhs.size() < 5 || rhs.size() < 5)
		{
			boost::format
			msg("Logic error: InstanceDataVector used to count the total number of consolidated rows has less than the necessary number of columns (time columns and at least one primary key column) in sort function.");
			throw NewGeneException() << newgene_error_description(msg.str());
		}

		// From http://stackoverflow.com/a/2810302/368896
		// ... Must pack two 32-bit ints into a 64-bit
		std::uint32_t lhsstart1 = boost::lexical_cast<std::uint32_t>(lhs[0]);
		std::uint32_t lhsstart2 = boost::lexical_cast<std::uint32_t>(lhs[1]);
		std::uint32_t lhsend1 = boost::lexical_cast<std::uint32_t>(lhs[2]);
		std::uint32_t lhsend2 = boost::lexical_cast<std::uint32_t>(lhs[3]);
		std::uint32_t rhsstart1 = boost::lexical_cast<std::uint32_t>(rhs[0]);
		std::uint32_t rhsstart2 = boost::lexical_cast<std::uint32_t>(rhs[1]);
		std::uint32_t rhsend1 = boost::lexical_cast<std::uint32_t>(rhs[2]);
		std::uint32_t rhsend2 = boost::lexical_cast<std::uint32_t>(rhs[3]);
		std::int64_t lhs_time_start = static_cast<std::int64_t>((static_cast<std::uint64_t>(lhsstart1)) << 32 | static_cast<std::uint64_t>(lhsstart2));
		std::int64_t lhs_time_end = static_cast<std::int64_t>((static_cast<std::uint64_t>(lhsend1)) << 32 | static_cast<std::uint64_t>(lhsend2));
		std::int64_t rhs_time_start = static_cast<std::int64_t>((static_cast<std::uint64_t>(rhsstart1)) << 32 | static_cast<std::uint64_t>(rhsstart2));
		std::int64_t rhs_time_end = static_cast<std::int64_t>((static_cast<std::uint64_t>(rhsend1)) << 32 | static_cast<std::uint64_t>(rhsend2));

		// check if time slices overlap
		TimeSlice lhs_time_slice;
		TimeSlice rhs_time_slice;

		if (lhs_time_start == -1 || lhs_time_end == -1)
		{
			lhs_time_slice.Reshape(0, 0); // sets both ends to infinite

			if (lhs_time_start != -1)
			{
				lhs_time_slice.setStart(lhs_time_start);
			}

			if (lhs_time_end != -1)
			{
				lhs_time_slice.setEnd(lhs_time_end);
			}
		}
		else
		{
			lhs_time_slice.Reshape(lhs_time_start, lhs_time_end);
		}

		if (rhs_time_start == -1 || rhs_time_end == -1)
		{
			rhs_time_slice.Reshape(0, 0); // sets both ends to infinite

			if (rhs_time_start != -1)
			{
				rhs_time_slice.setStart(rhs_time_start);
			}

			if (rhs_time_end != -1)
			{
				rhs_time_slice.setEnd(rhs_time_end);
			}
		}
		else
		{
			rhs_time_slice.Reshape(rhs_time_start, rhs_time_end);
		}

		bool lhs_overlaps_rhs = lhs_time_slice.DoesOverlap(rhs_time_slice);

		if (!lhs_overlaps_rhs)
		{
			// The rows do not overlap (or match on edge) in time.
			// Return whether lhs is less than rhs as a time slice.
			return lhs_time_slice < rhs_time_slice;
		}

		// The time slices overlap.
		// For this scenario, do a "simple" test - just see if the entire vector matches.
		// If not, consider them not to match, even though
		// in reality, some K-ad combinations here WILL be contiguous with previous time slices.
		// This will result in overcounting, which we indicate to the user in the status text.
		// The overcounting is necessary: The only way to count accurately, so far as I can see,
		// is to perform the entire consolidation algorithm (which is both time-consuming and heavily memory-demanding).
		// But, the current "overcounting" count will still, in most cases, result is a HUGELY smaller number
		// than the "total K-adic combinations" count,
		// and so will give a 'reasonable' upper limit count in many cases for the end user.

		if (lhs.size() < rhs.size())
		{
			// If the size is smaller, consider lhs to be "less than"
			return true;
		}
		else if (rhs.size() < lhs.size())
		{
			// rhs is considered "less than" lhs
			return false;
		}

		// ... sizes are the same.  Perform a lexographic comparison, skipping the two time fields.

		for (size_t n = 2; n < lhs.size(); ++n)
		{
			if (lhs[n] < rhs[n])
			{
				return true;
			}
			else if (rhs[n] < lhs[n])
			{
				return false;
			}
		}

		// if we made it this far - lhs and rhs are the same.
		return false;

	});
	weighting_consolidated.setWeighting(0);
	weighting_consolidated.setWeightingRangeStart(0);
	FastSetMemoryTag<InstanceDataVector<calculate_consolidated_total_number_rows_tag>, calculate_consolidated_total_number_rows_tag, decltype(calculateTotalConsolidatedRowsSortFunction)>
	* branches_and_leaves_set_ = InstantiateUsingTopLevelObjectsPool<tag__calculate_consolidated_total_number_rows<calculate_consolidated_total_number_rows_tag>>
								 (calculateTotalConsolidatedRowsSortFunction);
	FastSetMemoryTag<InstanceDataVector<calculate_consolidated_total_number_rows_tag>, calculate_consolidated_total_number_rows_tag, decltype(calculateTotalConsolidatedRowsSortFunction)>
	& branches_and_leaves_set = *branches_and_leaves_set_;
	InstanceDataVector<calculate_consolidated_total_number_rows_tag> * temp_branches_and_leaves_ =
		InstantiateUsingTopLevelObjectsPool<tag__calculate_consolidated_total_number_rows__instance_vector<calculate_consolidated_total_number_rows_tag>>();
	InstanceDataVector<calculate_consolidated_total_number_rows_tag> & temp_branches_and_leaves = *temp_branches_and_leaves_;

	newgene_cpp_int currentWeighting = 0;
	std::int64_t branch_count = 0;
	std::int64_t time_slice_count = 0;
	ProgressBarMeter meter(messager, std::string("Weighting for %1% / %2% branches calculated"), total_branches);
	std::for_each(timeSlices.begin(), timeSlices.end(), [&](TimeSlices<hits_tag>::value_type & timeSliceEntry)
	{

		if (CheckCancelled()) { return; }

		++time_slice_count;

		TimeSlice const & timeSlice = timeSliceEntry.first;
		VariableGroupTimeSliceData & variableGroupTimeSliceData = timeSliceEntry.second;
		VariableGroupBranchesAndLeavesVector<hits_tag> & variableGroupBranchesAndLeavesVector = *variableGroupTimeSliceData.branches_and_leaves;
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

			if (CheckCancelled()) { return; }

			Branches<hits_tag> & branchesAndLeaves = variableGroupBranchesAndLeaves.branches;
			Weighting & variableGroupBranchesAndLeavesWeighting = variableGroupBranchesAndLeaves.weighting;
			variableGroupBranchesAndLeavesWeighting.setWeightingRangeStart(currentWeighting);

			std::for_each(branchesAndLeaves.begin(), branchesAndLeaves.end(), [&](Branch const & branch)
			{

				if (CheckCancelled()) { return; }

				++branch_count;

				// Add row (if not present) for counting total UN-consolidated rows
				auto const & leaves = branch.leaves;
				temp_branches_and_leaves.clear();

				std::uint64_t packed = 0;

				if (timeSlice.startsAtNegativeInfinity())
				{

					// Bit of a hack: -1 ms will never appear
					// in NewGene's supported time granularity,
					// which is at its most granular 1 second (i.e., 1000 ms)
					// ... so -1 represents "negative infinity"
					packed = static_cast<std::uint64_t>(-1);

				}
				else
				{

					packed = static_cast<std::uint64_t>(timeSlice.getStart());

				}

				// Need to unpack the 64-bit timestamp into two 32-bit integers
				// that will fit into an InstanceDataVector field
				// from http://stackoverflow.com/a/2810302/368896
				std::uint32_t x, y;
				x = (std::uint32_t)((packed & 0xFFFFFFFF00000000) >> 32);
				y = (std::uint32_t)(packed & 0xFFFFFFFF);
				temp_branches_and_leaves.emplace_back(static_cast<std::int32_t>(x));
				temp_branches_and_leaves.emplace_back(static_cast<std::int32_t>(y));

				packed = 0;

				if (timeSlice.endsAtPlusInfinity())
				{

					// Bit of a hack: -1 ms will never appear
					// in NewGene's supported time granularity,
					// which is at its most granular 1 second (i.e., 1000 ms)
					// ... so -1 represents "positive infinity"
					packed = static_cast<std::uint64_t>(-1);

				}
				else
				{

					packed = static_cast<std::uint64_t>(timeSlice.getEnd());

				}

				// Need to unpack the 64-bit timestamp into two 32-bit integers
				// that will fit into an InstanceDataVector field
				// from http://stackoverflow.com/a/2810302/368896
				x = (std::uint32_t)((packed & 0xFFFFFFFF00000000) >> 32);
				y = (std::uint32_t)(packed & 0xFFFFFFFF);
				temp_branches_and_leaves.emplace_back(static_cast<std::int32_t>(x));
				temp_branches_and_leaves.emplace_back(static_cast<std::int32_t>(y));

				temp_branches_and_leaves.insert(temp_branches_and_leaves.cend(), branch.primary_keys.cbegin(), branch.primary_keys.cend());

				for (auto const & leaf : leaves)
				{
					if (CheckCancelled()) { break; }

					temp_branches_and_leaves.insert(temp_branches_and_leaves.cend(), leaf.primary_keys.cbegin(), leaf.primary_keys.cend());
				}

				if (CheckCancelled()) { return; }

				branches_and_leaves_set.insert(temp_branches_and_leaves);

				Weighting & branchWeighting = branch.weighting;

				// Count the leaves
				int numberLeaves = static_cast<int>(branch.numberLeaves());

				// The number of K-ad combinations for this branch is easily calculated.
				// It is just the binomial coefficient (assuming K <= N)

				branch.number_branch_combinations = 1; // covers K > numberLeaves condition, and numberLeaves == 0 condition

				// Handle "Limit DMU's" scenario
				if (K > 1 && numberLeaves < K && branch.has_excluded_leaves)
				{
					// There would always be an excluded DMU member in every row
					branch.number_branch_combinations = 0;
				}

				if (K <= numberLeaves && numberLeaves > 0)
				{
					branch.number_branch_combinations = BinomialCoefficient(numberLeaves, K);
				}

				// clear the hits cache
				branch.hits.clear();



				// Holes between time slices are handled here, as well as the standard case of no holes between time slices -
				// There is no gap in the sequence of discretized weight values in branches.
				branchWeighting.setWeighting(timeSlice.WidthForWeighting(time_granularity) * branch.number_branch_combinations);
				branchWeighting.setWeightingRangeStart(currentWeighting);
				currentWeighting += branchWeighting.getWeighting();

				variableGroupBranchesAndLeavesWeighting.addWeighting(branchWeighting.getWeighting());

				meter.UpdateProgressBarValue(branch_count);

			});

			variableGroupTimeSliceDataWeighting.addWeighting(variableGroupBranchesAndLeavesWeighting.getWeighting());

		});

		if (CheckCancelled()) { return; }

		weighting.addWeighting(variableGroupTimeSliceDataWeighting.getWeighting());

	});

	// Now count a good upper limit for the total umber of *consolidated* K-ad combinations.
	// See comments above.
	for (auto const & branch_and_leaves_combo : branches_and_leaves_set)
	{
		if (CheckCancelled()) { break; }

		// How many leaves?
		size_t total_number_cols = branch_and_leaves_combo.size() - 4; // -4 to account for the time columns
		size_t total_number_leaf_cols = total_number_cols - number_branch_columns;
		int total_number_leaves = 0;

		if (number_primary_variable_group_single_leaf_columns > 0)
		{
			total_number_leaves = static_cast<int>(total_number_leaf_cols) / number_primary_variable_group_single_leaf_columns;
		}

		newgene_cpp_int number_branch_combinations = 1; // covers K > numberLeaves condition, and numberLeaves == 0 condition

		if (K <= total_number_leaves && total_number_leaves > 0)
		{
			number_branch_combinations = BinomialCoefficient(total_number_leaves, K);
		}

		weighting_consolidated.addWeighting(number_branch_combinations);
	}

	InstanceDataVector<calculate_consolidated_total_number_rows_tag>().swap(temp_branches_and_leaves);
	PurgeTags<calculate_consolidated_total_number_rows_tag>();
	ClearTopLevelTag<tag__calculate_consolidated_total_number_rows>();
	ClearTopLevelTag<tag__calculate_consolidated_total_number_rows__instance_vector>();

}

bool KadSampler::AddNewTimeSlice(int const & variable_group_number, Branch const & branch, TimeSliceLeaf const & newTimeSliceLeaf)
{
	VariableGroupTimeSliceData variableGroupTimeSliceData;
	VariableGroupBranchesAndLeavesVector<hits_tag> & variableGroupBranchesAndLeavesVector = *variableGroupTimeSliceData.branches_and_leaves;
	VariableGroupBranchesAndLeaves newVariableGroupBranch(variable_group_number);
	Branches<hits_tag> & newBranchesAndLeaves = newVariableGroupBranch.branches;
	newBranchesAndLeaves.insert(branch);
	Branch const & current_branch = *(newBranchesAndLeaves.find(branch));

	bool added = current_branch.InsertLeaf(newTimeSliceLeaf.second); // add Leaf to the set of Leaves attached to the new Branch

	// Even if the leaf was not added due to Limit DMU functionality,
	// we must add the branch itself,
	// so that in the future if the same branch is added, we have maintained
	// the "a bad DMU had been attempted to be added" flag.
	variableGroupBranchesAndLeavesVector.push_back(newVariableGroupBranch);
	timeSlices[newTimeSliceLeaf.first] = variableGroupTimeSliceData;

	return added;
}

void KadSampler::PrepareRandomNumbers(std::int64_t how_many)
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

	boost::random::mt19937 engine(static_cast<std::int32_t>(std::time(0)));
	boost::random::uniform_int_distribution<newgene_cpp_int> distribution(weighting.getWeightingRangeStart(), weighting.getWeightingRangeEnd());

	// Check out this clean, simple approach:
	// The available row samples are discretely numbered.
	// So duplicates are rejected here, guaranteeing unbiased
	// choice of branches (each branch corresponding to a
	// window of discrete values), including no-replacement.
	bool reverse_mode = false;

	void * ptr = RandomVectorPool::malloc();
	auto remaining_ = new(ptr)
	FastVectorCppInt(); // let pointer drop off stack without deleting because this will trigger deletion of elements; let boost pool manage for more rapid deletion
	auto & remaining = *remaining_;

	FastVectorCppInt::iterator remainingPtr = remaining.begin();

	ptr = RandomSetPool::malloc();
	auto tmp_random_numbers_ = new(ptr)
	FastSetCppInt(); // let pointer drop off stack without deleting because this will trigger deletion of elements; let boost pool manage for more rapid deletion
	auto & tmp_random_numbers = *tmp_random_numbers_;

	ProgressBarMeter meter(messager, "Generated %1% out of %2% random numbers", how_many);

	while (tmp_random_numbers.size() < static_cast<size_t>(how_many))
	{

		// Check if we've consumed over 50% of the random numbers available
		if (reverse_mode)
		{

			tmp_random_numbers.insert(*std::move_iterator<decltype(remainingPtr)>(remainingPtr));

			++remainingPtr;

			if (remainingPtr == remaining.end())
			{
				break;
			}

		}
		else if (newgene_cpp_int(tmp_random_numbers.size()) > weighting.getWeighting() / 2)
		{
			// Over 50% of the available numbers are already consumed.
			// It must be a "somewhat" small number of available numbers.

			// Begin pulling numbers randomly from the remaining set available.
			for (newgene_cpp_int n = 0; n < weighting.getWeighting(); ++n)
			{
				if (tmp_random_numbers.count(n) == 0)
				{
					remaining.push_back(n);
				}
			}

			if (remaining.size() == 0)
			{
				boost::format msg("No random numbers generated.");
				throw NewGeneException() << newgene_error_description(msg.str());
			}

			std::random_shuffle(remaining.begin(), remaining.end(), [&](size_t max_random)
			{
				std::uniform_int_distribution<size_t> remaining_distribution(0, max_random - 1);
				size_t which_remaining_random_number = remaining_distribution(engine);
				return which_remaining_random_number;
			});

			// Prepare reverse mode, but do not populate a new random number
			remainingPtr = remaining.begin();
			reverse_mode = true;
		}
		else
		{
			tmp_random_numbers.insert(distribution(engine));
		}

		meter.UpdateProgressBarValue(tmp_random_numbers.size());

	}

	random_numbers.insert(random_numbers.begin(), tmp_random_numbers.cbegin(), tmp_random_numbers.cend());


	// Optimization - profiler shows a vast time spent in lower_bound elsewhere,
	// so optimize by keeping the random numbers sorted and process them in order.
	if (false)
	{
		std::random_shuffle(random_numbers.begin(), random_numbers.end(), [&](size_t max_random)
		{
			std::uniform_int_distribution<size_t> remaining_distribution(0, max_random - 1);
			size_t which_remaining_random_number = remaining_distribution(engine);
			return which_remaining_random_number;
		});
	}


	random_number_iterator = random_numbers.cbegin();

}

void KadSampler::GenerateAllOutputRows(int const K, Branch const & branch)
{

	std::int64_t which_time_unit_full_sampling__MINUS_ONE = -1;  // -1 means "full sampling for branch" - no need to break down into time units (which have identical full sets)

	branch.hits[which_time_unit_full_sampling__MINUS_ONE].clear();
	branch.remaining[which_time_unit_full_sampling__MINUS_ONE].clear();

	static int saved_range = -1;

	BranchOutputRow<remaining_tag> single_leaf_combination;

	bool skip = false;

	bool dmu_limited = false;

	if (K >= branch.numberLeaves())
	{

		skip = true;

		if (K > 1 && K > branch.numberLeaves() && branch.has_excluded_leaves)
		{
			dmu_limited = true;
		}

		if (!dmu_limited)
		{
			for (int n = 0; n < static_cast<int>(branch.numberLeaves()); ++n)
			{
				single_leaf_combination.Insert(n);
			}

			branch.remaining[which_time_unit_full_sampling__MINUS_ONE].push_back(single_leaf_combination);

			++number_rows_generated;
		}

	}

	if (K <= 0 || dmu_limited)
	{
		return;
	}

	if (!skip)
	{
		PopulateAllLeafCombinations(which_time_unit_full_sampling__MINUS_ONE, K, branch);
	}

	if (CheckCancelled()) { return; }

	// If Limit DMU's prevented anything from being generated
	// (even if this function wasn't previously exited in this case),
	// then nothing will be inserted here - begin() == end()
	branch.hits[which_time_unit_full_sampling__MINUS_ONE].insert(branch.remaining[which_time_unit_full_sampling__MINUS_ONE].begin(),
			branch.remaining[which_time_unit_full_sampling__MINUS_ONE].end());

	// No! Will be cleared by the memory pool en-bulk after generation of output rows
	//branch.remaining[which_time_unit].clear();

}

void KadSampler::GenerateRandomKad(newgene_cpp_int random_number, int const K, Branch const & branch)
{

	random_number -= branch.weighting.getWeightingRangeStart();

	std::int64_t which_time_unit = -1;

	if (time_granularity != TIME_GRANULARITY__NONE)
	{
		BOOST_ASSERT_MSG(branch.number_branch_combinations > 0, "The number of branch combinations is 0 or less!");
		newgene_cpp_int which_time_unit_ = random_number / branch.number_branch_combinations;
		which_time_unit = which_time_unit_.convert_to<std::int64_t>();
	}
	else
	{
		BOOST_ASSERT_MSG(random_number < branch.number_branch_combinations, "Generated random number is greater than the number of branch combinations when the time granularity is none!");
	}

	static int saved_range = -1;
	static std::mt19937 engine(static_cast<std::int32_t>(std::time(0)));

	BranchOutputRow<remaining_tag> test_leaf_combination;

	bool skip = false;

	bool dmu_limited = false;

	if (K >= branch.numberLeaves())
	{
		skip = true;

		if (K > 1 && K > branch.numberLeaves() && branch.has_excluded_leaves)
		{
			// This test should be redundant, as the weighting for the branch
			// in case of DMU limiting should be 0 and therefore the random number
			// should never land on this branch.
			// But leave the relevant code in place as an FYI.

			if (true)
			{
				boost::format msg("Random K-ad being requested from a branch which should have 0 weight!");
				throw NewGeneException() << newgene_error_description(msg.str());
			}

			dmu_limited = true;
		}

		if (!dmu_limited)
		{
			for (int n = 0; n < branch.numberLeaves(); ++n)
			{
				test_leaf_combination.Insert(n);
			}
		}
	}

	if (K <= 0 || dmu_limited)
	{
		return;
	}

	if (!skip)
	{

		BOOST_ASSERT_MSG(newgene_cpp_int(branch.hits[which_time_unit].size()) < branch.number_branch_combinations,
						 "The number of hits is as large as the number of combinations for a branch.  Invalid!");

		// skip any leaf combinations returned from previous random numbers - count will be 1 if previously hit for this time unit
		// THIS is where random selection WITH REMOVAL is implemented (along with the fact that the random numbers generated are also with removal)
		while (test_leaf_combination.Empty() || branch.hits[which_time_unit].count(test_leaf_combination))
		{

			if (CheckCancelled()) { break; }

			test_leaf_combination.Clear();

			if (newgene_cpp_int(branch.hits[which_time_unit].size()) > branch.number_branch_combinations / 2)
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

				// Forced to use std::list, rather than std::vector,
				// because performance of Boost Pool in populating the "remaining" data structure
				// in some scenarios is horrible.
				// In any case, due to the "erase", below, it's not clear that a vector is any better than a list, anyways

				//test_leaf_combination = branch.remaining[which_time_unit][which_remaining_leaf_combination];
				//auto remainingPtr = branch.remaining[which_time_unit].begin() + which_remaining_leaf_combination;

				auto remainingPtr = branch.remaining[which_time_unit].begin();

				for (size_t n = 0; n < which_remaining_leaf_combination; ++n)
				{
					if (CheckCancelled()) { break; }

					++remainingPtr;
				}

				if (CheckCancelled()) { break; }

				test_leaf_combination = *remainingPtr;

				branch.remaining[which_time_unit].erase(remainingPtr);

			}
			else
			{

				std::vector<int> remaining_leaves;

				// "remaining_leaves" is initialized to include ALL leaves
				for (int n = 0; n < branch.numberLeaves(); ++n)
				{
					if (CheckCancelled()) { break; }

					remaining_leaves.push_back(n);
				}

				if (CheckCancelled()) { break; }

				// Pull random leaves, one at a time, to create the random row
				while (test_leaf_combination.Size() < static_cast<size_t>(K))
				{

					if (CheckCancelled()) { break; }

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

					// ************************************************************************************************ //
					// This block will never be called unless there are MORE THAN ONE leaf slot
					// for the PRIMARY VARIABLE GROUP
					// (i.e., there must be actual leaves in addition to branches;
					// not just a branch + 1 empty leaf representing that branch)
					// ************************************************************************************************ //

					test_leaf_combination.Insert(index_of_leaf);
				}

				if (CheckCancelled()) { break; }

			}

		}

	}

	if (CheckCancelled()) { return; }

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
	// this is handled by storing a map of every *time unit* (corresponding to the primary variable group)
	// and all leaf combinations that have been hit for that time unit.
	// The time unit is defined by the time granularity of the unit of analysis associated
	// with the primary variable group.

	auto insert_result = branch.hits[which_time_unit].insert(test_leaf_combination);
	bool inserted = insert_result.second;

	#	ifdef _DEBUG
	static size_t bytes_allocated = 0;
	#	endif

	if (inserted)
	{
		++random_rows_added;

		#		ifdef _DEBUG

		if (false)
		{
			bytes_allocated += sizeof(test_leaf_combination);

			if (random_rows_added % 10000 == 0)
			{
				std::string sizedata;
				getMySize();
				mySize.spitSizes(sizedata);
				boost::format mytxt("%1% calls that inserted an output row; %2% bytes allocated in this way.  Size of AllWeightings: %3%");
				mytxt % random_rows_added % bytes_allocated % sizedata;
				messager.AppendKadStatusText(mytxt.str(), nullptr);
			}
		}

		#		endif

	}

}

void KadSampler::PopulateAllLeafCombinations(std::int64_t const & which_time_unit, int const K, Branch const & branch)
{

	// ************************************************************************************************ //
	// This function will never be called unless there are MORE THAN ONE leaf slot
	// for the PRIMARY VARIABLE GROUP
	// (i.e., there must be actual leaves in addition to branches;
	// not just a branch + 1 empty leaf representing that branch)
	// ************************************************************************************************ //

	newgene_cpp_int total_added = 0;

	branch.remaining[which_time_unit].clear();
	std::vector<int> position;

	for (int n = 0; n < K; ++n)
	{
		position.push_back(n);
	}

	if (branch.number_branch_combinations > 1000)
	{
		messager.SetPerformanceLabel((boost::format("A branch has %1% rows that need to be generated... Looks like this branch will take a while...") % boost::lexical_cast<std::string>
									  (branch.number_branch_combinations)).str());
	}

	while (total_added < branch.number_branch_combinations)
	{

		if (CheckCancelled()) { break; }

		AddPositionToRemaining(which_time_unit, position, branch);
		bool succeeded = IncrementPosition(K, position, branch);

		BOOST_ASSERT_MSG(succeeded || (total_added + 1) == branch.number_branch_combinations, "Invalid logic in position incrementer in sampler!");

		++total_added;

	}

	if (branch.number_branch_combinations > 100000)
	{
		messager.SetPerformanceLabel("");
	}

	number_rows_generated += total_added;

}

void KadSampler::AddPositionToRemaining(std::int64_t const & which_time_unit, std::vector<int> const & position, Branch const & branch)
{

	// ************************************************************************************************ //
	// This function will never be called unless there are MORE THAN ONE leaf slot
	// for the PRIMARY VARIABLE GROUP
	// (i.e., there must be actual leaves in addition to branches;
	// not just a branch + 1 empty leaf representing that branch)
	// ************************************************************************************************ //

	BranchOutputRow<remaining_tag> new_remaining;
	std::for_each(position.cbegin(), position.cend(), [&](int const position_index)
	{

		if (CheckCancelled()) { return; }

		// ************************************************************************************************ //
		// This function will never be called unless there are MORE THAN ONE leaf slot
		// for the PRIMARY VARIABLE GROUP
		// (i.e., there must be actual leaves in addition to branches;
		// not just a branch + 1 empty leaf representing that branch)
		// ************************************************************************************************ //

		new_remaining.Insert(position_index);

	});

	if (CheckCancelled()) { return; }

	if (branch.hits[which_time_unit].count(new_remaining) == 0)
	{
		branch.remaining[which_time_unit].emplace_back(new_remaining);
	}

}

bool KadSampler::IncrementPosition(int const K, std::vector<int> & position, Branch const & branch)
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

int KadSampler::IncrementPositionManageSubK(int const K, int const subK_, std::vector<int> & position, Branch const & branch)
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

newgene_cpp_int KadSampler::BinomialCoefficient(int const N, int const K)
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

	newgene_cpp_int numerator { 1 };
	newgene_cpp_int denominator { 1 };

	for (int n = N; n > N - K; --n)
	{
		numerator *= newgene_cpp_int(n);
	}

	for (int n = 1; n <= K; ++n)
	{
		denominator *= newgene_cpp_int(n);
	}

	return numerator / denominator;

}

void KadSampler::ClearBranchCaches()
{

	//ProgressBarMeter meter(messager, "Processed %1% of %2% time slices", timeSlices.size());

	std::int64_t current_loop_iteration = 0;
	std::for_each(timeSlices.begin(), timeSlices.end(), [&](TimeSlices<hits_tag>::value_type  & timeSliceData)
	{

		if (CheckCancelled()) { return; }

		VariableGroupBranchesAndLeavesVector<hits_tag> & variableGroupBranchesAndLeavesVector = *timeSliceData.second.branches_and_leaves;

		// For now, assume only one variable group
		if (variableGroupBranchesAndLeavesVector.size() > 1)
		{
			boost::format msg("Only one top-level variable group is currently supported for the random and full sampler in ConsolidateHits().");
			throw NewGeneException() << newgene_error_description(msg.str());
		}

		VariableGroupBranchesAndLeaves & variableGroupBranchesAndLeaves = variableGroupBranchesAndLeavesVector[0];
		Branches<hits_tag> & branchesAndLeaves = variableGroupBranchesAndLeaves.branches;

		std::for_each(branchesAndLeaves.begin(), branchesAndLeaves.end(), [&](Branch const & branch)
		{

			if (CheckCancelled()) { return; }

			// Do not delete!  Let the Boost Pool system handle this memory

			// ****************************************************************************************************************** //
			// "helper_lookup__from_child_key_set__to_matching_output_rows" CAN NEVER HAVE MAP KEYS
			// THAT ARE BLANKS -
			// every child [branch + leaf] MUST map to real values
			// ****************************************************************************************************************** //
			branch.helper_lookup__from_child_key_set__to_matching_output_rows = nullptr;

		});

		++current_loop_iteration;

		//meter.UpdateProgressBarValue(current_loop_iteration);

	});

	PurgeTags<child_dmu_lookup_tag>();

}

void KadSampler::ResetBranchCaches(int const child_variable_group_number, bool const reset_child_dmu_lookup)
{

	if (reset_child_dmu_lookup)
	{
		ClearBranchCaches();
	}

	if (CheckCancelled()) { return; }

	ProgressBarMeter meter(messager, "Processed %1% of %2% time slices", timeSlices.size());
	std::int64_t current_loop_iteration = 0;

	current_child_variable_group_being_merged = child_variable_group_number;
	std::for_each(timeSlices.begin(), timeSlices.end(), [&](TimeSlices<hits_tag>::value_type  & timeSliceData)
	{

		if (CheckCancelled()) { return; }

		timeSliceData.second.ResetBranchCachesSingleTimeSlice(*this, reset_child_dmu_lookup);

		++current_loop_iteration;
		meter.UpdateProgressBarValue(current_loop_iteration);

	});
	current_child_variable_group_being_merged = -1;

}

void PrimaryKeysGroupingMultiplicityOne::ConstructChildCombinationCache(KadSampler & allWeightings, int const variable_group_number, bool const force,
		bool const is_consolidating) const
{

	// ************************************************************************************************************************************************** //
	// This function builds a cache:
	// A single map for this entire branch that
	// maps a CHILD primary key set (i.e., a single row of child data,
	// including the child's 0 or more branch DMU keys and the 0 or 1 child leaf DMU key)
	// to a particular output row in a particular time unit in this branch,
	// for rapid lookup later.  The map also includes the *child* leaf number in the *output*
	// for the given row.
	// (I.e., the *input* is just a single child row of data with at most only one child leaf,
	//  but this single row of monadic child input data maps to a particular k-value
	//  in the corresponding output row, because child data can appear multiple times
	//  for a single output row.)
	// ************************************************************************************************************************************************** //

	// ****************************************************************************************************************** //
	// "helper_lookup__from_child_key_set__to_matching_output_rows" CAN NEVER HAVE MAP KEYS
	// THAT ARE BLANKS -
	// every child [branch + leaf] MUST map to real values
	// ****************************************************************************************************************** //
	if (force || helper_lookup__from_child_key_set__to_matching_output_rows == nullptr)
	{

		// The cache has yet to be filled, or we are specifically being requested to refresh it

		// ****************************************************************************************************************** //
		// "helper_lookup__from_child_key_set__to_matching_output_rows" CAN NEVER HAVE MAP KEYS
		// THAT ARE BLANKS -
		// every child [branch + leaf] MUST map to real values
		// ****************************************************************************************************************** //
		if (helper_lookup__from_child_key_set__to_matching_output_rows == nullptr)
		{
			// This scenario occurs before a child variable group is being merged.
			// To save memory, the cache from any previous child variable group merges
			// is discarded (elsewhere) from the Memory Pool, and recreated here.

			// ****************************************************************************************************************** //
			// "helper_lookup__from_child_key_set__to_matching_output_rows" CAN NEVER HAVE MAP KEYS
			// THAT ARE BLANKS -
			// every child [branch + leaf] MUST map to real values
			// ****************************************************************************************************************** //
			helper_lookup__from_child_key_set__to_matching_output_rows = InstantiateUsingTopLevelObjectsPool<tag__fast__lookup__from_child_dmu_set__to__output_rows<child_dmu_lookup_tag>>();
		}
		else
		{
			// This scenario occurs when PruneTimeUnits is being called,
			// i.e., a time slice is being split apart, merged, or consolidated,
			// so the child DMU lookup cache needs to be rebuilt,
			// since BranchOutputRows have been copied into a new instance,
			// invalidating all previous pointers.

			// ****************************************************************************************************************** //
			// "helper_lookup__from_child_key_set__to_matching_output_rows" CAN NEVER HAVE MAP KEYS
			// THAT ARE BLANKS -
			// every child [branch + leaf] MUST map to real values
			// ****************************************************************************************************************** //
			helper_lookup__from_child_key_set__to_matching_output_rows->clear();
		}

		ChildDMUInstanceDataVector<child_dmu_lookup_tag> child_hit_vector__child_branch_components__currently_always_maps_to_primary_branch_components;
		ChildDMUInstanceDataVector<child_dmu_lookup_tag> potential_future__child_hit_vector;

		// Some child branch columns can map to leaves of the top-level UOA.
		// If there is no leaf data for some of these top-level leaf columns (which can happen in the K>N case),
		// there cannot possibly be a match for the corresponding child branch columns
		// for the given output row in the "hits" vector (irrelevant to the time unit in that vector).
		// Set the following flag to capture this case.
		bool branch_in_child__cannot_match_all_leaf_slots_in_primary = false;

		std::for_each(hits.cbegin(), hits.cend(), [&](decltype(hits)::value_type const & time_unit_output_rows)
		{

			if (CheckCancelled()) { return; }

			// ***************************************************************************************** //
			// We have one time unit entry within this time slice for the given branch.
			// We proceed to build the cache
			// ***************************************************************************************** //

			if (branch_in_child__cannot_match_all_leaf_slots_in_primary)
			{
				// No luck for this output row for any child leaf.
				// Try the next row.
				return;
			}

			// All branch data is the same for each time unit.
			bool first_time_in_branch = true;

			for (auto outputRowPtr = time_unit_output_rows.second.cbegin(); outputRowPtr != time_unit_output_rows.second.cend(); ++outputRowPtr)
			{

				if (CheckCancelled()) { return; }

				// ******************************************************************************************************** //
				// We have a new output row we're dealing with.
				// This is a single output row corresponding to:
				// - A single time slice
				// - A single primary (top-level) branch within that time slice (this object)
				// - A single time unit within that time slice
				// - A single output row in that time unit in that time slice for that top-level branch
				//
				// Now, from our own data (there is no child data yet), let's find all of the child data that
				// *could* match on this row for the current child variable group under construction
				// (given by "variable_group_number".
				// (There could be more than one match of child data in this row.)
				// ******************************************************************************************************** //

				BranchOutputRow<hits_tag> const & outputRow = *outputRowPtr;

				if (first_time_in_branch)
				{

					child_hit_vector__child_branch_components__currently_always_maps_to_primary_branch_components.clear();

					// First in the "child DMU" metadata vector is the metadata for the child's BRANCH DMU's
					std::for_each(allWeightings.mappings_from_child_branch_to_primary[variable_group_number].cbegin(),
								  allWeightings.mappings_from_child_branch_to_primary[variable_group_number].cend(), [&](ChildToPrimaryMapping const & childToPrimaryMapping)
					{

						if (CheckCancelled()) { return; }

						// We have the next DMU data in the sequence of DMU's for the child branch/leaf (we're working on the branch)

						switch (childToPrimaryMapping.mapping)
						{

							case CHILD_TO_PRIMARY_MAPPING__MAPS_TO_BRANCH:
								{

									// The next DMU in the child branch's DMU sequence maps to a branch in the top-level DMU sequence
									child_hit_vector__child_branch_components__currently_always_maps_to_primary_branch_components.push_back(DMUInstanceData(
												primary_keys[childToPrimaryMapping.index_of_column_within_top_level_branch_or_single_leaf]));

								}
								break;

							case CHILD_TO_PRIMARY_MAPPING__MAPS_TO_LEAF:
								{

									// The next DMU in the child branch's DMU sequence maps to a leaf in the top-level DMU sequence

									// ************************************************************************** //
									//
									// This is currently unsupported
									//
									// If it ever becomes supported, we have to do this calculation
									// EVERY TIME THROUGH THE LOOP
									// (i.e., for every output row associated with the primary group's branch),
									// rather than just the first time through the loop
									//
									// ************************************************************************** //
									if (true)
									{
										boost::format msg("Logic error: It is currently unsupported for a child variable group's branch column to map to a primary variable group leaf column.");
										throw NewGeneException() << newgene_error_description(msg.str());
									}

									// leaf_number tells us which leaf in the top-level DMU
									// index tells us which index in that leaf

									if (childToPrimaryMapping.leaf_number_in_top_level_group__only_applicable_when_child_key_column_points_to_top_level_column_that_is_in_top_level_leaf >= static_cast<int>
										(outputRow.primary_leaves_cache.size()))
									{
										// EVERY output row selects the same number of leaves and places them into its primary_leaves_cache.
										// So EVERY output row's primary_leaves_cache is the same size.

										// If one output row has fewer leaves in its primary_leaves_cache than is sufficient to fill
										// the necessary leaf slots required to form the branch slots for the child,
										// then all output rows also don't.
										branch_in_child__cannot_match_all_leaf_slots_in_primary = true;
										break;
									}

									if (leaves_cache[outputRow.primary_leaves_cache[childToPrimaryMapping.leaf_number_in_top_level_group__only_applicable_when_child_key_column_points_to_top_level_column_that_is_in_top_level_leaf]].primary_keys.size()
										== 0)
									{
										// This is the K=1 case - the matching leaf of the *top-level* UOA
										// has no primary keys.  This is a logic error, as we should never match
										// a "leaf" in the top-level UOA in this case.
										//
										// To confirm this is a legitimate logic error, see "OutputModel::OutputGenerator::KadSamplerFillDataForChildGroups()",
										// in particular the following lines:
										// --> // if (full_kad_key_info.total_outer_multiplicity__for_the_current_dmu_category__corresponding_to_the_uoa_corresponding_to_top_level_variable_group == 1)
										// --> // {
										// --> //     is_current_index_a_top_level_primary_group_branch = true;
										// --> // }

										boost::format msg("Logic error: attempting to match child branch data to a leaf in the top-level unit of analysis when K=1.  There can be no leaves when K=1.");
										throw NewGeneException() << newgene_error_description(msg.str());
									}

									// ************************************************************************************** //
									// ************************************************************************************** //
									//
									// The following index lookup is one of the trickiest in the entire application.
									//
									//
									// Note that "leaves_cache" belongs GLOBALLY to the PRIMARY VARIABLE GROUP branch object.
									//
									// And that "outputRow.primary_leaves_cache" is just a fast lookup into the same data,
									// with the same index into the data structure via iterating, as "outputRow.primary_leaves".
									//
									// And that "childToPrimaryMapping.leaf_number_in_top_level_group__only_applicable_when_child_key_column_points_to_top_level_column_that_is_in_top_level_leaf"
									// is the index into the above OUTPUT-ROW SPECIFIC leaf cache.
									//
									// And that the value stored in the OUTPUT-ROW SPECIFIC leaf cache
									// is, itself, just an INDEX into the GLOBAL leaf cache "leaved_cache" noted above.
									//
									// Once the actual cached leaf in the GLOBAL leaf cache "leaves_cache" is retrieved via
									// "leaves_cache[outputRow.primary_leaves_cache[childToPrimaryMapping.leaf_number_in_top_level_group__only_applicable_when_child_key_column_points_to_top_level_column_that_is_in_top_level_leaf]]",
									// the primary key columns of this leaf (because a single leaf can have multiple primary key columns)
									// is stored in the "primary_keys" data member.
									//
									// How do we know which INTERNAL column of DMU instance data inside the individual leaf to retrieve?
									// That is stored in "childToPrimaryMapping.index_of_column_within_top_level_branch_or_single_leaf".
									//
									// If you put all of the above pieces together, you find that the following line of code
									// stores the DMU instance data corresponding to the proper internal column inside the
									// PRIMARY VARIABLE GROUP leaf
									// at the proper index inside the global PRIMARY VARIABLE GROUP leaf cache,
									// in the "child_hit_vector__child_branch_components__currently_always_maps_to_primary_branch_components"
									//
									// ************************************************************************************** //
									// ************************************************************************************** //
									child_hit_vector__child_branch_components__currently_always_maps_to_primary_branch_components.push_back(DMUInstanceData(
												leaves_cache[outputRow.primary_leaves_cache[childToPrimaryMapping.leaf_number_in_top_level_group__only_applicable_when_child_key_column_points_to_top_level_column_that_is_in_top_level_leaf]].primary_keys[childToPrimaryMapping.index_of_column_within_top_level_branch_or_single_leaf]));

								}
								break;

							default:
								{}
								break;

						}

					});

				}

				if (CheckCancelled()) { break; }

				first_time_in_branch = false;

				// In the "child DMU" metadata vector is the metadata for the child's LEAF DMU's
				int child_leaf_index_crossing_multiple_child_leaves = 0;
				int child_leaf_index_within_a_single_child_leaf = 0;
				int current_child_leaf_number = 0;
				bool missing_top_level_leaf = false;
				potential_future__child_hit_vector.clear();
				potential_future__child_hit_vector.insert(potential_future__child_hit_vector.begin(),
						child_hit_vector__child_branch_components__currently_always_maps_to_primary_branch_components.begin(),
						child_hit_vector__child_branch_components__currently_always_maps_to_primary_branch_components.end());
				std::for_each(allWeightings.mappings_from_child_leaf_to_primary[variable_group_number].cbegin(),
							  allWeightings.mappings_from_child_leaf_to_primary[variable_group_number].cend(), [&](ChildToPrimaryMapping const & childToPrimaryMapping)
				{

					if (CheckCancelled()) { return; }

					// ************************************************************************************************************** //
					// We have a SINGLE COLUMN -
					// a single child LEAF (out of potentially more than one),
					// and within that child leaf a single COLUMN (out of potentially more than one)
					// ************************************************************************************************************** //

					// We have the next DMU data in the sequence of DMU's for the child branch/leaf (we're working on the leaf)

					switch (childToPrimaryMapping.mapping)
					{

						case CHILD_TO_PRIMARY_MAPPING__MAPS_TO_BRANCH:
							{

								// **************************************************************************************** //
								// TODO: This can be moved into the section that only performs the calculation once,
								// rather than performing it for every output row...
								// **************************************************************************************** //

								// The next DMU in the child leaf's DMU sequence maps to a branch in the top-level DMU sequence
								potential_future__child_hit_vector.push_back(DMUInstanceData(primary_keys[childToPrimaryMapping.index_of_column_within_top_level_branch_or_single_leaf]));

							}
							break;

						case CHILD_TO_PRIMARY_MAPPING__MAPS_TO_LEAF:
							{

								// leaf_number tells us which leaf
								// index tells us which index in that leaf

								// The next DMU in the child leaf's DMU sequence maps to a leaf in the top-level DMU sequence

								if (childToPrimaryMapping.leaf_number_in_top_level_group__only_applicable_when_child_key_column_points_to_top_level_column_that_is_in_top_level_leaf >= static_cast<int>
									(outputRow.primary_leaves_cache.size()))
								{
									// The current child leaf maps to a top-level leaf that has no data in this output row.
									// We therefore cannot match.

									// *********************************************************************************************** //
									// NOTE THAT ALL OUTPUT ROWS FOR A GIVEN PRIMARY BRANCH
									// contain the same number of leaves in their "primary_leaves_cache",
									// so if ONE output row is missing a top level leaf at what would be
									// a given index in the potential_future__child_hit_vector, then ALL output rows will be
									// missing a top level leaf at what would be the same index,
									// so the indexing will remain consistent and correct
									// (i.e., each index in potential_future__child_hit_vector will always correspond to the
									// same output column for this primary variable group branch and time slice).
									// *********************************************************************************************** //

									missing_top_level_leaf = true;
									break;
								}

								if (outputRow.primary_leaves_cache[childToPrimaryMapping.leaf_number_in_top_level_group__only_applicable_when_child_key_column_points_to_top_level_column_that_is_in_top_level_leaf]
									>= static_cast<int>(leaves_cache.size()))
								{
									boost::format msg("Logic error: Output rows saved with the branch point to leaf indexes that do not exist in the branch!");
									throw NewGeneException() << newgene_error_description(msg.str());
								}

								// We should never map to a leaf with no DMU's (an empty leaf) -
								//
								// neither a "Limit DMU's" scenario will ever add an empty leaf
								//   (and this possibility is only present when there is > 1 leaf slot;
								//    otherwise it's a branch-only scenario in which case in the "Limit DMU's" scenario
								//    a branch with any leaves in the restricted list will NOT be added
								//    to the list of branches and this function will not be called),
								// nor a "branch-only primary variable group" scenario will ever
								// cause a branch column to *be mapped to* by a leaf lookup
								if (leaves_cache[outputRow.primary_leaves_cache[childToPrimaryMapping.leaf_number_in_top_level_group__only_applicable_when_child_key_column_points_to_top_level_column_that_is_in_top_level_leaf]].primary_keys.size()
									== 0)
								{


									// *********************************************************************************************** //
									// Note: The following comments apply including the "Limit DMU's" functionality
									// *********************************************************************************************** //

									// This is the K=1 case - the matching leaf of the *top-level* UOA
									// has no primary keys.  This is a logic error, as we should never match
									// a "leaf" in the top-level UOA in this case.
									//
									// To confirm this is a legitimate logic error, see "OutputModel::OutputGenerator::KadSamplerFillDataForChildGroups()",
									// in particular the following lines:
									// --> // if (full_kad_key_info.total_outer_multiplicity__for_the_current_dmu_category__corresponding_to_the_uoa_corresponding_to_top_level_variable_group == 1)
									// --> // {
									// --> //     is_current_index_a_top_level_primary_group_branch = true;
									// --> // }

									boost::format msg("Logic error: attempting to match child leaf data to a leaf in the top-level unit of analysis when K=1");
									throw NewGeneException() << newgene_error_description(msg.str());
								}

								// In the branch-only scenario (for the primary variable group),
								// this block will never be reached because in this block
								// we map to LEAF in the primary variable group.
								//
								// So this is a scenario with > 1 leaf in the primary variable group.
								// In which case all leaves are valid
								// (no restricted leaves from the "Limit DMU's" settings will ever be added).
								potential_future__child_hit_vector.push_back(DMUInstanceData(
											leaves_cache[outputRow.primary_leaves_cache[childToPrimaryMapping.leaf_number_in_top_level_group__only_applicable_when_child_key_column_points_to_top_level_column_that_is_in_top_level_leaf]].primary_keys[childToPrimaryMapping.index_of_column_within_top_level_branch_or_single_leaf]));

							}
							break;

						default:
							{}
							break;

					}

					if (CheckCancelled()) { return; }

					++child_leaf_index_crossing_multiple_child_leaves;
					++child_leaf_index_within_a_single_child_leaf;

					if (child_leaf_index_within_a_single_child_leaf == allWeightings.childInternalToOneLeafColumnCountForDMUWithMultiplicityGreaterThan1[variable_group_number])
					{

						// missing_top_level_leaf is only possible
						// if there are > 1 top level leaf slots
						if (!missing_top_level_leaf)
						{

							// ****************************************************************************************************************** //
							// "helper_lookup__from_child_key_set__to_matching_output_rows" CAN NEVER HAVE MAP KEYS
							// THAT ARE BLANKS -
							// every child [branch + leaf] MUST map to real values
							// ****************************************************************************************************************** //
							if (helper_lookup__from_child_key_set__to_matching_output_rows == nullptr)
							{
								boost::format msg("Null child DMU key lookup cache in merge.");
								throw NewGeneException() << newgene_error_description(msg.str());
							}

							// ****************************************************************************************************************** //
							// Note!
							// A possibly DIFFERENT "potential_future__child_hit_vector" set of keys
							// is being added here
							// WITHIN THE SAME OUTPUT ROW
							//
							// I.e., we are accessing a possibly DIFFERENT vector in the next line of code
							// than in other iterations of this loop.
							// ****************************************************************************************************************** //
							//
							// ****************************************************************************************************************** //
							// "helper_lookup__from_child_key_set__to_matching_output_rows" CAN NEVER HAVE MAP KEYS
							// THAT ARE BLANKS -
							// every child [branch + leaf] MUST map to real values
							// ****************************************************************************************************************** //
							(*helper_lookup__from_child_key_set__to_matching_output_rows)[potential_future__child_hit_vector][&outputRow].push_back(current_child_leaf_number);
						}

						++current_child_leaf_number;
						child_leaf_index_within_a_single_child_leaf = 0;
						missing_top_level_leaf = false;
						potential_future__child_hit_vector.clear();
						potential_future__child_hit_vector.insert(potential_future__child_hit_vector.begin(),
								child_hit_vector__child_branch_components__currently_always_maps_to_primary_branch_components.begin(),
								child_hit_vector__child_branch_components__currently_always_maps_to_primary_branch_components.end());
					}

				});

				if (CheckCancelled()) { break; }

				// Cover the case where there are no child-leaf-to-primary mappings,
				// only child-branch-to-primary mappings
				//
				// This means the child is all branch and no leaf.
				// Equivalently, it means that this set of child DMU keys
				// should point to this particular output row, and within that row
				// it should point to "leaf #0".
				if (allWeightings.mappings_from_child_leaf_to_primary[variable_group_number].size() == 0)
				{
					if (helper_lookup__from_child_key_set__to_matching_output_rows == nullptr)
					{
						// Note: The DMU lookup includes both the branch (if any) and the leaf (if any) DMU keys
						boost::format msg("Null child DMU key lookup cache in merge.");
						throw NewGeneException() << newgene_error_description(msg.str());
					}

					// ****************************************************************************************************************** //
					// Note:
					// There is only one possible child that can match on this output row,
					// and its "leaf index" is always 0.
					// ****************************************************************************************************************** //
					//
					// ****************************************************************************************************************** //
					// "helper_lookup__from_child_key_set__to_matching_output_rows" CAN NEVER HAVE MAP KEYS
					// THAT ARE BLANKS -
					// every child [branch + leaf] MUST map to real values
					// ****************************************************************************************************************** //
					(*helper_lookup__from_child_key_set__to_matching_output_rows)[potential_future__child_hit_vector][&outputRow].push_back(
						0); // When there are no leaf DMU slots, there is always one leaf of index 0 with an empty DMU list
				}

			}

		});

	}

}

void KadSampler::PrepareRandomSamples(int const K)
{

	TimeSlices<hits_tag>::const_iterator timeSlicePtr = timeSlices.cbegin();
	newgene_cpp_int currentMapElementHighEndWeight = timeSlicePtr->second.weighting.getWeightingRangeEnd();

	std::int32_t random_rows_generated = 0;

	ProgressBarMeter meter(messager, std::string("Generated %1% out of %2% randomly selected K-adic combinations..."), static_cast<std::int32_t>(random_numbers.size()));

	while (true)
	{

		if (CheckCancelled()) { break; }

		if (random_number_iterator == random_numbers.cend())
		{
			break;
		}

		newgene_cpp_int const & random_number = *random_number_iterator;

		bool optimization = true;

		// Optimization: The incoming random numbers are sorted.
		// Profiling shows that even in Release mode,
		// well over 90% of the time in the selection and creation
		// of random rows is spent in this block.
		// Optimization here is critical.
		if (optimization)
		{

			while (random_number > currentMapElementHighEndWeight)
			{

				if (CheckCancelled()) { break; }

				// ************************************************************ //
				// Limit DMU functionality is properly handled here
				//
				// Note that elements with a weighting of 0 always have their
				// "end" value set to 1 less than their "start" value.
				// That is what makes this work.
				//
				// For example, if the first element/s have a weighting of 0,
				// they will all have [0,-1] for their [start, end] values.
				// Suppose the first element with non-zero weight has a weight of 1.
				// Its range will be [0,0], and the FOLLOWING element will have
				// a start value of 1: [1,x].
				// (If it has a weight of 0, it will have a range [1,0]; and so on.)
				// ************************************************************ //

				// The current map element is still good!
				++timeSlicePtr;

				// optimization: No safety check on iterator.  We assume all random numbers are within the map.
				currentMapElementHighEndWeight = timeSlicePtr->second.weighting.getWeightingRangeEnd();

			}

			if (CheckCancelled()) { break; }

		}
		else
		{

			// no optimization - the random number could lie in any map element


			// ******************************************************* //
			// Note: "Limit DMU" functionality should be properly handled here,
			// but it's untested since this block is disabled.
			// See comments above
			// ******************************************************* //

			BOOST_ASSERT_MSG(random_number >= 0 && random_number < weighting.getWeighting() && weighting.getWeightingRangeStart() == 0
							 && weighting.getWeightingRangeEnd() == weighting.getWeighting() - 1, "Invalid weights in RetrieveNextBranchAndLeaves().");

			TimeSlices<hits_tag>::const_iterator timeSlicePtr = std::lower_bound(timeSlices.cbegin(), timeSlices.cend(),
					random_number, [&](TimeSlices<hits_tag>::value_type const & timeSliceData,
									   newgene_cpp_int const & test_random_number)
			{
				VariableGroupTimeSliceData const & testVariableGroupTimeSliceData = timeSliceData.second;

				if (testVariableGroupTimeSliceData.weighting.getWeightingRangeEnd() < test_random_number)
				{
					return true;
				}

				return false;
			});

		}

		if (CheckCancelled()) { break; }

		TimeSlice const & timeSlice = timeSlicePtr->first;
		VariableGroupTimeSliceData const & variableGroupTimeSliceData = timeSlicePtr->second;
		VariableGroupBranchesAndLeavesVector<hits_tag> const & variableGroupBranchesAndLeavesVector = *variableGroupTimeSliceData.branches_and_leaves;

		// For now, assume only one variable group
		if (variableGroupBranchesAndLeavesVector.size() > 1)
		{
			boost::format msg("Only one top-level variable group is currently supported for the random and full sampler.");
			throw NewGeneException() << newgene_error_description(msg.str());
		}

		VariableGroupBranchesAndLeaves const & variableGroupBranchesAndLeaves = variableGroupBranchesAndLeavesVector[0];

		Branches<hits_tag> const & branches = variableGroupBranchesAndLeaves.branches;

		// Pick a branch randomly (with weight!)
		Branches<hits_tag>::const_iterator branchesPtr = std::lower_bound(branches.cbegin(), branches.cend(), random_number, [&](Branch const & testBranch,
				newgene_cpp_int const & test_random_number)
		{

			// ************************************************************ //
			// Limit DMU functionality should be properly handled here.
			// See comments in analogous block for time slices, above.
			// ************************************************************ //

			if (testBranch.weighting.getWeightingRangeEnd() < test_random_number)
			{
				return true;
			}

			return false;
		});

		if (CheckCancelled()) { break; }

		const Branch & new_branch = *branchesPtr;

		// random_number is now an actual *index* to which combination of leaves in this VariableGroupTimeSliceData
		GenerateRandomKad(random_number, K, new_branch);

		++random_number_iterator;

		++random_rows_generated;
		meter.UpdateProgressBarValue(random_rows_generated);

	}

}

void KadSampler::PrepareFullSamples(int const K)
{

	number_rows_generated = 0;
	ProgressBarMeter meter(messager, std::string("Generated %1% out of %2% K-adic combinations"), weighting.getWeighting());
	std::for_each(timeSlices.cbegin(), timeSlices.cend(), [&](decltype(timeSlices)::value_type const & timeSlice)
	{

		if (CheckCancelled()) { return; }

		TimeSlice const & the_slice = timeSlice.first;
		VariableGroupTimeSliceData const & variableGroupTimeSliceData = timeSlice.second;
		VariableGroupBranchesAndLeavesVector<hits_tag> const & variableGroupBranchesAndLeaves = *variableGroupTimeSliceData.branches_and_leaves;

		std::for_each(variableGroupBranchesAndLeaves.cbegin(), variableGroupBranchesAndLeaves.cend(), [&](VariableGroupBranchesAndLeaves const & variableGroupBranchesAndLeaves)
		{
			if (CheckCancelled()) { return; }

			std::for_each(variableGroupBranchesAndLeaves.branches.cbegin(), variableGroupBranchesAndLeaves.branches.cend(), [&](Branch const & branch)
			{
				if (CheckCancelled()) { return; }

				GenerateAllOutputRows(K, branch);
				number_rows_generated += branch.number_branch_combinations;
				meter.UpdateProgressBarValue(number_rows_generated);
			});

		});

	});

}

void KadSampler::ConsolidateRowsWithinBranch(Branch const & branch, std::int64_t & current_rows, ProgressBarMeter & meter)
{

	if (time_granularity == TIME_GRANULARITY__NONE)
	{
		// The data is already stored in a single time unit within this branch at index -1
		return;
	}

	// Optimization: no need to clear -1, and besides with the current algorithm, it's never filled if we are in this function in any case
	//branch.hits[-1].clear();

	// Optimization required: first, reserve memory
	std::int64_t reserve_size = 0;
	std::for_each(branch.hits.begin(), branch.hits.end(), [&](decltype(branch.hits)::value_type & hit)
	{
		if (CheckCancelled()) { return; }

		if (hit.first != -1)
		{
			reserve_size += hit.second.size();
		}
	});

	if (CheckCancelled()) { return; }

	// Optimization required: first, reserve memory
	// ... profiler shows that 99% of time in "consolidating rows" routine
	//     is spend expanding the vector in the boost-managed memory pool,
	//     and with standard-allocator backed pool, heap becomes so fragmented
	//     when running 1,000,000 rows that we crash
	branch.hits_consolidated.reserve(static_cast<size_t>(reserve_size));

	// Now consolidate the output rows from the time-unit subslices into a single sorted vector
	std::for_each(branch.hits.begin(), branch.hits.end(), [&](decltype(branch.hits)::value_type & hit)
	{
		if (CheckCancelled()) { return; }

		if (hit.first != -1)
		{
			for (auto iter = std::begin(hit.second); iter != std::end(hit.second); ++iter)
			{
				if (CheckCancelled()) { break; }

				// Profiler shows that about half the time in the "consolidating rows" phase
				// is spent creating new memory here.

				// Optimization!  Profiler shows that over 95% of time in the "consolidating rows" routine
				// is *now* spent inserting into the "Boost memory-pool backed" hits[-1].
				// This is terrible performance, so we must use a std::set.
				//branch.hits[-1].insert(std::move(const_cast<BranchOutputRow &>(*iter)));
				branch.hits_consolidated.emplace_back(std::move(const_cast<BranchOutputRow<hits_tag> &>(*iter)));
				//branch.hits_consolidated.emplace_back(*iter);

				// Even after the std::move, above, the Boost memory pool deallocation of the space for the object itself
				// is requiring 90%+ of the time for the "consolidating rows" routine.
				// Therefore, leave the (empty) elements in place, and later when looping through consolidated rows
				// just skip all but the -1 index of branch.hits.
				// We'll gladly pay the cost in memory in exchange for rapidly speeding up the "consolidating rows" stage.

				// No! Performance hit
				//hit.second.erase(iter++);

				++current_rows;
				meter.UpdateProgressBarValue(current_rows);
			}

			if (CheckCancelled()) { return; }

			// Nope.  Causes MASSIVE hang for many minutes to handle memory model using memory pool - intended for rapid creates, but not rapid deletes
			//hit.second.clear();

		}
	});

	if (CheckCancelled()) { return; }

	std::sort(branch.hits_consolidated.begin(), branch.hits_consolidated.end());
	branch.hits_consolidated.erase(std::unique(branch.hits_consolidated.begin(), branch.hits_consolidated.end()), branch.hits_consolidated.end());
	//branch.consolidated_hits_end_index = std::unique(branch.hits_consolidated.begin(), branch.hits_consolidated.end()); // Do not erase!! Optimization

	// ditto above
	//branch.hits.erase(++branch.hits.begin(), branch.hits.end());

}

void VariableGroupTimeSliceData::ResetBranchCachesSingleTimeSlice(KadSampler & allWeightings, bool const reset_child_dmu_lookup)
{

	VariableGroupBranchesAndLeavesVector<hits_tag> & variableGroupBranchesAndLeavesVector = *branches_and_leaves;

	// For now, assume only one variable group
	if (variableGroupBranchesAndLeavesVector.size() > 1)
	{
		boost::format msg("Only one top-level variable group is currently supported for the random and full sampler in ConsolidateHits().");
		throw NewGeneException() << newgene_error_description(msg.str());
	}

	VariableGroupBranchesAndLeaves & variableGroupBranchesAndLeaves = variableGroupBranchesAndLeavesVector[0];
	Branches<hits_tag> & branchesAndLeaves = variableGroupBranchesAndLeaves.branches;

	std::for_each(branchesAndLeaves.begin(), branchesAndLeaves.end(), [&](Branch const & branch)
	{

		if (allWeightings.CheckCancelled()) { return; }

		branch.ResetLeafCache();

		if (reset_child_dmu_lookup && allWeightings.current_child_variable_group_being_merged != -1)
		{
			branch.ConstructChildCombinationCache(allWeightings, allWeightings.current_child_variable_group_being_merged, true, false);
		}

	});

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
		sdata += TimeRange::convertMsSinceEpochToString(time_slice.getStart());
	}

	sdata += "</DATETIME_START>";

	sdata += "<DATETIME_END>";

	if (!endinf)
	{
		sdata += TimeRange::convertMsSinceEpochToString(time_slice.getEnd());
	}

	sdata += "</DATETIME_END>";

}

void SpitBranch(std::string & sdata, Branch const & branch)
{
	sdata += "<BRANCH>";

	sdata += "<PRIMARY_KEYS>";
	SpitKeys(sdata, branch.primary_keys);
	sdata += "</PRIMARY_KEYS>";

	sdata += "<HITS>";
	SpitHits(sdata, branch.hits);
	sdata += "</HITS>";

	sdata += "<CONSOLIDATED_HIT>";
	SpitHit<fast_branch_output_row_vector_huge<hits_consolidated_tag>, hits_consolidated_tag>(sdata, -1, branch.hits_consolidated);
	sdata += "</CONSOLIDATED_HIT>";

	if (branch.helper_lookup__from_child_key_set__to_matching_output_rows != nullptr)
	{
		sdata += "<CHILD_KEY_LOOKUP_TO_QUICKLY_DETERMINE_IF_ANY_PARTICULAR_CHILD_KEYSET_EXISTS_FOR_ANY_OUTPUT_ROW_FOR_THIS_BRANCH>";
		SpitChildLookup(sdata, *(branch.helper_lookup__from_child_key_set__to_matching_output_rows));
		sdata += "</CHILD_KEY_LOOKUP_TO_QUICKLY_DETERMINE_IF_ANY_PARTICULAR_CHILD_KEYSET_EXISTS_FOR_ANY_OUTPUT_ROW_FOR_THIS_BRANCH>";
	}
	else
	{
		sdata += "<CHILD_KEY_LOOKUP_TO_QUICKLY_DETERMINE_IF_ANY_PARTICULAR_CHILD_KEYSET_EXISTS_FOR_ANY_OUTPUT_ROW_FOR_THIS_BRANCH/>";
	}

	// Not yet tied into Boost memory pool infrastructure in same way - NewGene has never slowed down for lack of this yet
	//
	//if (branch.helper_lookup__from_child_key_set__to_matching_output_rows_consolidating != nullptr)
	//{
	sdata += "<CHILD_KEY_LOOKUP_TO_QUICKLY_DETERMINE_IF_ANY_PARTICULAR_CHILD_KEYSET_EXISTS_FOR_ANY_OUTPUT_ROW_FOR_THIS_BRANCH__CONSOLIDATION_PHASE>";
	SpitChildLookup(sdata, branch.helper_lookup__from_child_key_set__to_matching_output_rows_consolidating);
	sdata += "</CHILD_KEY_LOOKUP_TO_QUICKLY_DETERMINE_IF_ANY_PARTICULAR_CHILD_KEYSET_EXISTS_FOR_ANY_OUTPUT_ROW_FOR_THIS_BRANCH__CONSOLIDATION_PHASE>";
	//}

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
	sdata += weighting.getWeightingString();
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
	sdata += boost::lexical_cast<std::string>
			 (childToPrimaryMapping.leaf_number_in_top_level_group__only_applicable_when_child_key_column_points_to_top_level_column_that_is_in_top_level_leaf);
	sdata += "</leaf_number_in_top_level_group__only_applicable_when_child_key_column_points_to_top_level_column_that_is_in_top_level_leaf>";

}

void SpitAllWeightings(KadSampler const & allWeightings, std::string const & file_name_appending_string)
{

	std::fstream file_;

	boost::format filenametxt("all_weightings.%1%.xml");
	filenametxt % file_name_appending_string;
	file_.open(filenametxt.str(), std::ofstream::out | std::ios::trunc);

	if (!file_.is_open())
	{
		std::string theerr = strerror(errno);
		return;
	}

	std::string sdata_;
	std::string * sdata = &sdata_;

	*sdata += "<ALL_WEIGHTINGS>";

	allWeightings.getMySize();

	*sdata += "<ALL_WEIGHTINGS_TOTAL_SIZE>";
	allWeightings.mySize.spitSizes(*sdata);
	*sdata += "</ALL_WEIGHTINGS_TOTAL_SIZE>";

	*sdata += "<TIME_GRANULARITY>";
	*sdata += GetTimeGranularityText(allWeightings.time_granularity);
	*sdata += "</TIME_GRANULARITY>";

	*sdata += "<NUMBER_CHILD_VARIABLE_GROUPS>";
	*sdata += boost::lexical_cast<std::string>(allWeightings.numberChildVariableGroups);
	*sdata += "</NUMBER_CHILD_VARIABLE_GROUPS>";

	file_.write(sdata_.c_str(), sdata_.size());
	sdata_.clear();

	*sdata += "<childInternalToOneLeafColumnCountForDMUWithMultiplicityGreaterThan1>";
	std::for_each(allWeightings.childInternalToOneLeafColumnCountForDMUWithMultiplicityGreaterThan1.cbegin(),
				  allWeightings.childInternalToOneLeafColumnCountForDMUWithMultiplicityGreaterThan1.cend(), [&](decltype(
							  allWeightings.childInternalToOneLeafColumnCountForDMUWithMultiplicityGreaterThan1)::value_type const & oneChildGroup)
	{
		*sdata += "<child_group>";

		*sdata += "<child_variable_group_index>";
		*sdata += boost::lexical_cast<std::string>(boost::lexical_cast<std::int32_t>(oneChildGroup.first));
		*sdata += "</child_variable_group_index>";
		*sdata += "<column_count_for_child_dmu_with_child_multiplicity_greater_than_1>";
		*sdata += boost::lexical_cast<std::string>(boost::lexical_cast<std::int32_t>(oneChildGroup.second));
		*sdata += "</column_count_for_child_dmu_with_child_multiplicity_greater_than_1>";

		*sdata += "</child_group>";
	});
	*sdata += "</childInternalToOneLeafColumnCountForDMUWithMultiplicityGreaterThan1>";

	file_.write(sdata_.c_str(), sdata_.size());
	sdata_.clear();

	*sdata += "<CHILD_COLUMN_TO_TOP_LEVEL_COLUMN_KEY_MAPPINGS>";

	for (int c = 0; c < allWeightings.numberChildVariableGroups; ++c)
	{
		*sdata += "<CHILD_GROUP>";

		*sdata += "<CHILD_GROUP_INDEX>";
		*sdata += boost::lexical_cast<std::string>(c);
		*sdata += "</CHILD_GROUP_INDEX>";

		*sdata += "<CHILD_GROUP_COLUMN_MAPPINGS>";
		*sdata += "<BRANCH_MAPPINGS>";
		file_.write(sdata_.c_str(), sdata_.size());
		sdata_.clear();

		std::for_each(allWeightings.mappings_from_child_branch_to_primary.cbegin(),
					  allWeightings.mappings_from_child_branch_to_primary.cend(), [&](decltype(allWeightings.mappings_from_child_branch_to_primary)::value_type const & oneChildGroupBranchMappings)
		{
			if (oneChildGroupBranchMappings.first == c)
			{
				std::for_each(oneChildGroupBranchMappings.second.cbegin(), oneChildGroupBranchMappings.second.cend(), [&](ChildToPrimaryMapping const & childToPrimaryMapping)
				{
					file_.write(sdata_.c_str(), sdata_.size());
					sdata_.clear();

					*sdata += "<BRANCH_MAPPING>";
					SpitChildToPrimaryKeyColumnMapping(*sdata, childToPrimaryMapping);
					*sdata += "</BRANCH_MAPPING>";
					file_.write(sdata_.c_str(), sdata_.size());
					sdata_.clear();

				});
			}
		});
		*sdata += "</BRANCH_MAPPINGS>";
		*sdata += "<LEAF_MAPPINGS>";

		std::for_each(allWeightings.mappings_from_child_leaf_to_primary.cbegin(),
					  allWeightings.mappings_from_child_leaf_to_primary.cend(), [&](decltype(allWeightings.mappings_from_child_leaf_to_primary)::value_type const & oneChildGroupLeafMappings)
		{
			if (oneChildGroupLeafMappings.first == c)
			{
				std::for_each(oneChildGroupLeafMappings.second.cbegin(), oneChildGroupLeafMappings.second.cend(), [&](ChildToPrimaryMapping const & childToPrimaryMapping)
				{
					file_.write(sdata_.c_str(), sdata_.size());
					sdata_.clear();

					*sdata += "<LEAF_MAPPING>";
					SpitChildToPrimaryKeyColumnMapping(*sdata, childToPrimaryMapping);
					*sdata += "</LEAF_MAPPING>";

					file_.write(sdata_.c_str(), sdata_.size());
					sdata_.clear();

				});
			}
		});
		*sdata += "</LEAF_MAPPINGS>";
		*sdata += "</CHILD_GROUP_COLUMN_MAPPINGS>";

		*sdata += "</CHILD_GROUP>";
	}

	file_.write(sdata_.c_str(), sdata_.size());
	sdata_.clear();


	*sdata += "</CHILD_COLUMN_TO_TOP_LEVEL_COLUMN_KEY_MAPPINGS>";

	file_.write(sdata_.c_str(), sdata_.size());
	sdata_.clear();

	*sdata += "<DATA_CACHE_PRIMARY>";
	SpitDataCache(*sdata, allWeightings.dataCache);
	*sdata += "</DATA_CACHE_PRIMARY>";

	file_.write(sdata_.c_str(), sdata_.size());
	sdata_.clear();

	*sdata += "<DATA_CACHES_TOP_LEVEL_NON_PRIMARY>";
	SpitDataCaches(*sdata, allWeightings.otherTopLevelCache);
	*sdata += "</DATA_CACHES_TOP_LEVEL_NON_PRIMARY>";

	file_.write(sdata_.c_str(), sdata_.size());
	sdata_.clear();

	*sdata += "<DATA_CACHES_CHILDREN>";
	SpitDataCaches(*sdata, allWeightings.childCache);
	*sdata += "</DATA_CACHES_CHILDREN>";

	file_.write(sdata_.c_str(), sdata_.size());
	sdata_.clear();

	*sdata += "<TIME_SLICES>";
	*sdata += "<TIME_SLICES_MAP_ITSELF>";
	*sdata += boost::lexical_cast<std::string>(sizeof(allWeightings.timeSlices));
	*sdata += "</TIME_SLICES_MAP_ITSELF>";
	std::for_each(allWeightings.timeSlices.cbegin(), allWeightings.timeSlices.cend(), [&](decltype(allWeightings.timeSlices)::value_type const & timeSlice)
	{

		*sdata += "<TIME_SLICE>";

		*sdata += "<TIME_SLICE_MAP_ITSELF>";
		*sdata += boost::lexical_cast<std::string>(sizeof(timeSlice));
		*sdata += "</TIME_SLICE_MAP_ITSELF>";

		TimeSlice const & the_slice = timeSlice.first;
		VariableGroupTimeSliceData const & variableGroupTimeSliceData = timeSlice.second;
		VariableGroupBranchesAndLeavesVector<hits_tag> const & variableGroupBranchesAndLeaves = *variableGroupTimeSliceData.branches_and_leaves;

		file_.write(sdata_.c_str(), sdata_.size());
		sdata_.clear();

		*sdata += "<TIME>";
		SpitTimeSlice(*sdata, the_slice);
		*sdata += "</TIME>";

		file_.write(sdata_.c_str(), sdata_.size());
		sdata_.clear();

		*sdata += "<WEIGHTING>";
		SpitWeighting(*sdata, variableGroupTimeSliceData.weighting);
		*sdata += "</WEIGHTING>";

		*sdata += "<VARIABLE_GROUPS_BRANCHES_AND_LEAVES>";
		std::for_each(variableGroupBranchesAndLeaves.cbegin(), variableGroupBranchesAndLeaves.cend(), [&](VariableGroupBranchesAndLeaves const & variableGroupBranchesAndLeaves)
		{
			*sdata += "<VARIABLE_GROUP_BRANCHES_AND_LEAVES>";

			*sdata += "<VARIABLE_GROUP_BRANCHES_AND_LEAVES_MAP_ITSELF>";
			*sdata += boost::lexical_cast<std::string>(sizeof(variableGroupBranchesAndLeaves));
			*sdata += "</VARIABLE_GROUP_BRANCHES_AND_LEAVES_MAP_ITSELF>";

			*sdata += "<VARIABLE_GROUP_NUMBER>";
			*sdata += boost::lexical_cast<std::string>(variableGroupBranchesAndLeaves.variable_group_number);
			*sdata += "</VARIABLE_GROUP_NUMBER>";

			*sdata += "<WEIGHTING>";
			SpitWeighting(*sdata, variableGroupBranchesAndLeaves.weighting);
			*sdata += "</WEIGHTING>";

			file_.write(sdata_.c_str(), sdata_.size());
			sdata_.clear();

			*sdata += "<BRANCHES_WITH_LEAVES>";
			std::for_each(variableGroupBranchesAndLeaves.branches.cbegin(), variableGroupBranchesAndLeaves.branches.cend(), [&](Branch const & branch)
			{
				*sdata += "<BRANCH_WITH_LEAVES>";

				*sdata += "<BRANCH_WITH_LEAVES_MAP_ITSELF>";
				*sdata += boost::lexical_cast<std::string>(sizeof(branch));
				*sdata += "</BRANCH_WITH_LEAVES_MAP_ITSELF>";

				file_.write(sdata_.c_str(), sdata_.size());
				sdata_.clear();

				SpitBranch(*sdata, branch);

				file_.write(sdata_.c_str(), sdata_.size());
				sdata_.clear();

				branch.SpitLeaves(*sdata);

				file_.write(sdata_.c_str(), sdata_.size());
				sdata_.clear();

				*sdata += "</BRANCH_WITH_LEAVES>";
			});
			*sdata += "</BRANCHES_WITH_LEAVES>";

			file_.write(sdata_.c_str(), sdata_.size());
			sdata_.clear();

			*sdata += "</VARIABLE_GROUP_BRANCHES_AND_LEAVES>";

			file_.write(sdata_.c_str(), sdata_.size());
			sdata_.clear();

		});
		*sdata += "</VARIABLE_GROUPS_BRANCHES_AND_LEAVES>";

		*sdata += "</TIME_SLICE>";

	});
	*sdata += "</TIME_SLICES>";

	file_.write(sdata_.c_str(), sdata_.size());
	sdata_.clear();

	*sdata += "<WEIGHTING>";
	SpitWeighting(*sdata, allWeightings.weighting);
	*sdata += "</WEIGHTING>";

	*sdata += "</ALL_WEIGHTINGS>";

	file_.write(sdata_.c_str(), sdata_.size());
	sdata_.clear();

	file_.close();

}

void SpitLeaves(std::string & sdata, Branch const & branch)
{
	sdata += "<LEAVES>";
	branch.ResetLeafCache();
	branch.SpitLeaves(sdata);
	sdata += "</LEAVES>";
}

//#endif

void VariableGroupTimeSliceData::PruneTimeUnits(KadSampler & allWeightings, TimeSlice const & originalTimeSlice, TimeSlice const & currentTimeSlice, bool const consolidate_rows,
		bool const random_sampling)
{

	// ********************************************************************************************** //
	// This function is called when time slices (and corresponding output row data) are SPLIT.
	//
	// Note that the opposite case - when time slices are MERGED -
	// is always in the context of CONSOLIDATED output,
	// a scenario where precisely the "hits" data is wiped out and consolidated
	// so that this function would be irrelevant (and is not called).
	//
	// So again, in the context of this function, we are only slicing time slices, never merging.
	// Furthermore, the calling code guarantees that time slices are only shrunk from both sides
	// (or left the same), never expanded.
	// So it is guaranteed that "originalTimeSlice" fully covers "currentTimeSlice"
	// (or matches it on one and/or the other edge in time).
	//
	// This function takes the current time slice (this instance)
	// and recalculates its "hits" data, throwing away "hits" time units that
	// are outside the range indicated by "current time slice" in relation to "original time slice".
	//
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
	// are successfully output, with the weighting taken care of by the width of the slice.
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
	// Note that we call it "pruning" time slices, but really the calling function
	// before calling this function first "splits"
	// existing time slices into multiple pieces,
	// each a non-overlapping sub-piece of the original (never expanding)
	// and each with an exact copy of the original piece's "hits" data, and then the calling function
	// calls "prune" (this function) on each of the relevant sub-pieces
	// (those that are not being thrown away).
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

		// But do reset the child DMU key lookup.  This will be needed regardless.
		ResetBranchCachesSingleTimeSlice(allWeightings, true);
		return;
	}

	if (CheckCancelled()) { return; }

	if (originalTimeSlice.hasTimeGranularity() && !currentTimeSlice.hasTimeGranularity())
	{
		boost::format msg("Attempting to prune time units when the new time slice has no time granularity, while the original does!");
		throw NewGeneException() << newgene_error_description(msg.str());
	}

	// special cases of no time granularity
	if (allWeightings.time_granularity == TIME_GRANULARITY__NONE)
	{
		// We just keep all our hits.  They're already in the -1 index.
		// Let the new data merge into us later, after this function is exited,
		// in "MergeNewDataIntoTimeSlice()".
		ResetBranchCachesSingleTimeSlice(allWeightings, true);
		return;
	}

	if (CheckCancelled()) { return; }

	// We *leave the data available* no matter how small the time slice is,
	// and let the output routine ensure that the time widths properly weight the slices.
	//
	// [.............][.............][.............][.............][.............][.............][.............][.............]
	//     |       |____________________________A____________________________|    |______________B_____________|    |         |
	//
	// ^                                                                                                                      ^
	// |                                                                                                                      |
	// a                                                                                                                      b
	//     ^                                                                                                        ^
	//     |                                                                                                        |
	//     c                                                                                                        d
	//
	// In this example, each [.............] corresponds to a time unit with some output rows -
	// on average, the dots each represent an output row.
	//
	// Each [.............] corresponds to a time range that is precisely equal to
	// the time granularity corresponding to the primary variable group's unit of analysis,
	// and furthermore, the [.............] are always guaranteed to be aligned in absolute
	// coordinates on even multiples of this time unit (starting at the Unix timestamp 0 point,
	// which is the start of day on Jan 1, 1970 (i.e. the turn of midnight the previous day leading
	// into Jan 1, 1970).
	//
	// Note that each element of the "hits" vector corresponds to a single [.............].
	// In particular, note that in the FULL SAMPLING (i.e., NOT random sampling) case,
	// the rows in each [.............] are identical (that is how the time slices are constructed -
	// i.e., whenever a leaf is added to a sub-fraction of a time slice, the time slice is first split,
	// and only then is the new leaf - which creates a new set of rows as it is combined with all other leaves -
	// added).  But for RANDOM sampling, only a random selection of rows appears in each [.............],
	// and so the rows in each [.............] are different.
	//
	// The "original" time slice might be evenly aligned, represented by [a,b],
	//  (it will *always* be evenly aligned when this function is called while
	//   processing time slices for the primary variable group),
	// or it could be that the original time slice is NOT evenly aligned
	//  (represented by [c,d]).
	//
	// The "current" time slice will be some arbitrary slice that is fully contained
	// by the "original" time slice (represented by the two examples, A and B).
	// Note: one or both edges of the "current" slice could match the corresponding edge/s
	// of the "original" time slice, but will never exceed the corresponding edge of the
	// original time slice (on the left or right).
	//
	// B is an example of a "current" time slice that evenly covers two time units.
	//
	// A is an example of a "current" time slice that
	// covers a sliver of the full time unit at both the left and right.
	// After this function exits, A will contain ALL of the output rows
	// from the left time unit sliver
	// (in the first entry of its "hits" vector),
	// and all of the output rows from the right time unit sliver
	// (in the last entry of its "hits" vector),
	// as well as all of the output rows from both fully-enclosed and full-sized
	// middle two time units (it the middle elements of its "hits" vector,
	// in this case, two middle elements).
	//
	// You can see that the algorithm that outputs the data will simply output all rows
	// corresponding to each "sliver" piece,
	// but the sliver will have a very narrow range, so the weighting still works out.
	//
	// For this algorithm to work, it is critical that the PRIMARY variable group always arrive in multiples
	// of the time unit in absolute measure, and evenly aligned.
	//
	// The DATA IMPORT routine, in conjunction with the time granularity
	// of the unit of analysis, assure this.
	//
	// But merges with non-primary variable groups (both top-level and child) can have any time granularity
	// (even if it's smaller), just so long as they themselves are aligned on
	// absolute boundaries corresponding to their own time granularities.
	//
	// It's all about counting.

	std::int64_t start = currentTimeSlice.getStart();
	std::int64_t end = currentTimeSlice.getEnd();

	fast__int64__to__fast_branch_output_row_set<remaining_tag> new_hits;
	VariableGroupBranchesAndLeavesVector<hits_tag> const & variableGroupBranchesAndLeavesVector = *branches_and_leaves;
	std::for_each(variableGroupBranchesAndLeavesVector.cbegin(), variableGroupBranchesAndLeavesVector.cend(), [&](VariableGroupBranchesAndLeaves const & variableGroupBranchesAndLeaves)
	{

		if (CheckCancelled()) { return; }

		std::for_each(variableGroupBranchesAndLeaves.branches.cbegin(), variableGroupBranchesAndLeaves.branches.cend(), [&](Branch const & current_branch)
		{

			if (CheckCancelled()) { return; }

			// ************************************************************** //
			// Single branch
			// ************************************************************** //

			new_hits.clear();

			auto & hits = current_branch.hits;

			std::int64_t hit_number = 0;
			std::int64_t new_hit_number = 0;

			// granulated output, full sampling
			currentTimeSlice.loop_through_time_units(allWeightings.time_granularity, boost::function<void(std::int64_t const, std::int64_t const)>([&](std::int64_t const time_to_use_for_start,
					std::int64_t const time_to_use_for_end)
			{
				if (CheckCancelled()) { return; }

				bool overlaps = false;

				if (time_to_use_for_start < end && time_to_use_for_end > start)
				{
					overlaps = true;
				}

				if (overlaps)
				{
					new_hits[new_hit_number].insert(hits[hit_number].cbegin(), hits[hit_number].cend());
					++new_hit_number;
				}

				++hit_number;
			}));

			if (CheckCancelled()) { return; }

			hits.clear();
			std::for_each(new_hits.cbegin(), new_hits.cend(), [&](fast__int64__to__fast_branch_output_row_set<remaining_tag>::value_type const & new_hit)
			{
				if (CheckCancelled()) { return; }

				hits[new_hit.first].insert(new_hit.second.cbegin(), new_hit.second.cend());
			});

			if (CheckCancelled()) { return; }

			current_branch.ValidateOutputRowLeafIndexes();

		});

	});

	if (CheckCancelled()) { return; }

	ResetBranchCachesSingleTimeSlice(allWeightings, true);

}

void BindTermToInsertStatement(sqlite3_stmt * insert_random_sample_stmt, InstanceData const & data, int bindIndex)
{
	bind_visitor visitor(insert_random_sample_stmt, bindIndex);
	boost::apply_visitor(visitor, data);
}

void KadSampler::getMySize() const
{

	memset(&mySize, '\0', sizeof(mySize));

	mySize.sizePod += sizeof(*this);
	mySize.totalSize += mySize.sizePod;

	// random_numbers
	//mySize.sizeRandomNumbers += sizeof(random_numbers);

	// disable for now - random numbers are purged after use
	if (false)
	{
		for (auto const & random_number : random_numbers)
		{
			mySize.sizeRandomNumbers += sizeof(random_number);
		}
	}

	mySize.totalSize += mySize.sizeRandomNumbers;

	//consolidated_rows
	//mySize.sizeConsolidatedRows += sizeof(consolidated_rows);
	mySize.numberMapNodes += consolidated_rows.size();

	for (auto const & mergedTimeSliceRow : consolidated_rows)
	{
		mySize.sizeConsolidatedRows += sizeof(mergedTimeSliceRow);
		getInstanceDataVectorUsage(mySize.sizeConsolidatedRows, mergedTimeSliceRow.output_row, false);
	}

	mySize.totalSize += mySize.sizeConsolidatedRows;

	// mappings_from_child_branch_to_primary
	//mySize.sizeMappingsFromChildBranchToPrimary += sizeof(mappings_from_child_branch_to_primary);
	getChildToBranchColumnMappingsUsage(mySize.sizeMappingsFromChildBranchToPrimary, mappings_from_child_branch_to_primary);
	mySize.totalSize += mySize.sizeMappingsFromChildBranchToPrimary;

	// mappings_from_child_leaf_to_primary
	//mySize.sizeMappingFromChildLeafToPrimary += sizeof(mappings_from_child_leaf_to_primary);
	getChildToBranchColumnMappingsUsage(mySize.sizeMappingFromChildLeafToPrimary, mappings_from_child_leaf_to_primary);
	mySize.totalSize += mySize.sizeMappingFromChildLeafToPrimary;

	// childInternalToOneLeafColumnCountForDMUWithMultiplicityGreaterThan1
	//mySize.size_childInternalToOneLeafColumnCountForDMUWithMultiplicityGreaterThan1 += sizeof(childInternalToOneLeafColumnCountForDMUWithMultiplicityGreaterThan1);
	mySize.numberMapNodes += childInternalToOneLeafColumnCountForDMUWithMultiplicityGreaterThan1.size();

	for (auto const & single_childInternalToOneLeafColumnCountForDMUWithMultiplicityGreaterThan1 : childInternalToOneLeafColumnCountForDMUWithMultiplicityGreaterThan1)
	{
		//mySize.size_childInternalToOneLeafColumnCountForDMUWithMultiplicityGreaterThan1 += sizeof(single_childInternalToOneLeafColumnCountForDMUWithMultiplicityGreaterThan1);
		mySize.size_childInternalToOneLeafColumnCountForDMUWithMultiplicityGreaterThan1 += sizeof(single_childInternalToOneLeafColumnCountForDMUWithMultiplicityGreaterThan1.first);
		mySize.size_childInternalToOneLeafColumnCountForDMUWithMultiplicityGreaterThan1 += sizeof(single_childInternalToOneLeafColumnCountForDMUWithMultiplicityGreaterThan1.second);
	}

	mySize.totalSize += mySize.size_childInternalToOneLeafColumnCountForDMUWithMultiplicityGreaterThan1;

	// dataCache
	//mySize.sizeDataCache += sizeof(dataCache);
	getDataCacheUsage(mySize.sizeDataCache, dataCache);
	mySize.totalSize += mySize.sizeDataCache;

	// otherTopLevelCache
	//mySize.sizeOtherTopLevelCache += sizeof(otherTopLevelCache);
	mySize.numberMapNodes += otherTopLevelCache.size();

	for (auto const & single_otherTopLevelCache : otherTopLevelCache)
	{
		mySize.sizeOtherTopLevelCache += sizeof(single_otherTopLevelCache);
		getDataCacheUsage(mySize.sizeOtherTopLevelCache, single_otherTopLevelCache.second);
	}

	mySize.totalSize += mySize.sizeOtherTopLevelCache;

	// childCache
	//mySize.sizeChildCache += sizeof(childCache);
	mySize.numberMapNodes += childCache.size();

	for (auto const & single_childCache : childCache)
	{
		mySize.sizeChildCache += sizeof(single_childCache);
		getDataCacheUsage(mySize.sizeChildCache, single_childCache.second);
	}

	mySize.totalSize += mySize.sizeChildCache;

	// TimeSlices<hits_tag>
	mySize.sizeTimeSlices += sizeof(timeSlices);

	for (auto const & timeSlice : timeSlices)
	{
		mySize.sizeTimeSlices += sizeof(timeSlice.first);
		mySize.sizeTimeSlices += sizeof(timeSlice.second);

		// branches_and_leaves is a vector
		auto const & branches_and_leaves = *timeSlice.second.branches_and_leaves;

		for (auto const & variableGroupBranchesAndLeaves : branches_and_leaves)
		{

			// variableGroupBranchesAndLeaves is an object of a class

			mySize.sizeTimeSlices += sizeof(variableGroupBranchesAndLeaves);

			// branches is a set
			Branches<hits_tag> const & branches = variableGroupBranchesAndLeaves.branches;

			mySize.numberMapNodes += branches.size();

			for (auto const & branch : branches)
			{

				// branch is an object of a class

				mySize.sizeTimeSlices += sizeof(branch);

				getInstanceDataVectorUsage(mySize.sizeTimeSlices, branch.primary_keys, false);

				auto const & hits = branch.hits;
				auto const & remaining = branch.remaining;
				auto const & helper_lookup__from_child_key_set__to_matching_output_rows = branch.helper_lookup__from_child_key_set__to_matching_output_rows;
				auto const & leaves = branch.leaves;
				auto const & leaves_cache = branch.leaves_cache;

				// // Already calculated in sizeof(branch)
				//mySize.sizeTimeSlices += sizeof(hits);
				//mySize.sizeTimeSlices += sizeof(remaining);
				//mySize.sizeTimeSlices += sizeof(helper_lookup__from_child_key_set__to_matching_output_rows);
				//mySize.sizeTimeSlices += sizeof(leaves);
				//mySize.sizeTimeSlices += sizeof(leaves_cache);


				// leaves_cache is a vector

				// leaves_cache
				for (auto const & leaf : leaves_cache)
				{
					mySize.sizeTimeSlices += sizeof(leaf);
					getLeafUsage(mySize.sizeTimeSlices, leaf);
				}


				// leaves is a set
				mySize.numberMapNodes += leaves.size();

				// leaves
				for (auto const & leaf : leaves)
				{
					mySize.sizeTimeSlices += sizeof(leaf);
					getLeafUsage(mySize.sizeTimeSlices, leaf);
				}

				if (helper_lookup__from_child_key_set__to_matching_output_rows != nullptr)
				{

					// helper_lookup__from_child_key_set__to_matching_output_rows is a map
					mySize.numberMapNodes += helper_lookup__from_child_key_set__to_matching_output_rows->size();

					for (auto const & child_lookup_from_child_data_to_rows : *helper_lookup__from_child_key_set__to_matching_output_rows)
					{
						mySize.sizeTimeSlices += sizeof(child_lookup_from_child_data_to_rows.first);
						mySize.sizeTimeSlices += sizeof(child_lookup_from_child_data_to_rows.second);

						auto const & childDMUInstanceDataVector = child_lookup_from_child_data_to_rows.first;
						auto const & outputRowsToChildDataMap = child_lookup_from_child_data_to_rows.second;

						getInstanceDataVectorUsage(mySize.sizeTimeSlices, childDMUInstanceDataVector, false);

						mySize.numberMapNodes += outputRowsToChildDataMap.size();

						for (auto const & outputRowToChildDataMap : outputRowsToChildDataMap)
						{
							mySize.sizeTimeSlices += sizeof(outputRowToChildDataMap.first);
							mySize.sizeTimeSlices += sizeof(outputRowToChildDataMap.second);

							// child_leaf_indices is a vector
							auto const & child_leaf_indices = outputRowToChildDataMap.second;

							for (auto const & child_leaf_index : child_leaf_indices)
							{
								// POD
								mySize.sizeTimeSlices += sizeof(child_leaf_index);
							}
						}
					}

				}

				// ditto ... "remaining" is purged immediately after use, so don't bother
				if (false)
				{
					// remaining is a map
					mySize.numberMapNodes += remaining.size();

					for (auto const & remaining_rows : remaining)
					{
						auto const & remaining_time_unit = remaining_rows.first;
						auto const & remainingOutputRows = remaining_rows.second;

						// remainingOutputRows is a vector

						mySize.sizeTimeSlices += sizeof(remaining_time_unit);
						mySize.sizeTimeSlices += sizeof(remainingOutputRows);

						for (auto const & outputRow : remainingOutputRows)
						{
							// outputRow is an object that is an instance of the BranchOutputRow class
							getSizeOutputRow(mySize.sizeTimeSlices, outputRow);
						}
					}
				}

				// hits is a map
				mySize.numberMapNodes += hits.size();

				for (auto const & time_unit_hit : hits)
				{
					mySize.sizeTimeSlices += sizeof(time_unit_hit.first);
					mySize.sizeTimeSlices += sizeof(time_unit_hit.second);

					// outputRows is a vector
					auto const & outputRows = time_unit_hit.second;

					for (auto const & outputRow : outputRows)
					{
						// outputRow is an object that is an instance of the BranchOutputRow class
						getSizeOutputRow(mySize.sizeTimeSlices, outputRow);
					}
				}
			}
		}

	}

	mySize.totalSize += mySize.sizeTimeSlices;

}

void KadSampler::getLeafUsage(size_t & usage, Leaf const & leaf) const
{
	//usage += sizeof(leaf);

	getInstanceDataVectorUsage(usage, leaf.primary_keys, false);

	auto const & other_top_level_indices_into_raw_data = leaf.other_top_level_indices_into_raw_data;

	for (auto const & single_other_top_level_indices_into_raw_data : other_top_level_indices_into_raw_data)
	{
		usage += sizeof(single_other_top_level_indices_into_raw_data.first);
		usage += sizeof(single_other_top_level_indices_into_raw_data.second);
	}
}

void KadSampler::getDataCacheUsage(size_t & usage, DataCache<hits_tag> const & dataCache) const
{
	mySize.numberMapNodes += dataCache.size();

	for (auto const & rowIdToData : dataCache)
	{
		usage += sizeof(rowIdToData.first);
		usage += sizeof(rowIdToData.second);
		getInstanceDataVectorUsage(usage, rowIdToData.second, false);
	}
}

void KadSampler::getChildToBranchColumnMappingsUsage(size_t & usage, fast_int_to_childtoprimarymappingvector<hits_tag> const & childToBranchColumnMappings) const
{
	mySize.numberMapNodes += childToBranchColumnMappings.size();

	for (auto const & single_mappings_from_child_branch_to_primary : childToBranchColumnMappings)
	{
		usage += sizeof(single_mappings_from_child_branch_to_primary.first);
		usage += sizeof(single_mappings_from_child_branch_to_primary.second);

		auto const & childToPrimaryMappingVector = single_mappings_from_child_branch_to_primary.second;

		for (auto const & childToPrimaryMapping : childToPrimaryMappingVector)
		{
			usage += sizeof(childToPrimaryMapping);
		}
	}
}

void KadSampler::Clear()
{

	messager.SetPerformanceLabel("Cleaning up a bit; please be patient...");

	create_output_row_visitor::data = nullptr;

	if (insert_random_sample_stmt)
	{
		sqlite3_finalize(insert_random_sample_stmt);
		insert_random_sample_stmt = nullptr;
	}

	purge_pool<newgene_cpp_int_tag, sizeof(boost::multiprecision::limb_type const)>();
	purge_pool<newgene_cpp_int_tag, sizeof(newgene_cpp_int)>();
	purge_pool<newgene_cpp_int_random_tag, sizeof(boost::multiprecision::limb_type const)>();
	purge_pool<newgene_cpp_int_random_tag, sizeof(newgene_random_cpp_int)>();

	RandomVectorPool::purge_memory();
	RandomSetPool::purge_memory();

	ClearTopLevelTag<tag__fast__int64__to__fast_branch_output_row_vector>();
	ClearTopLevelTag<tag__fast__int64__to__fast_branch_output_row_list>();
	ClearTopLevelTag<tag__fast__lookup__from_child_dmu_set__to__output_rows>();
	ClearTopLevelTag<tag__saved_historic_rows>();
	ClearTopLevelTag<tag__ongoing_consolidation_vector>();
	ClearTopLevelTag<tag__ongoing_consolidation>();
	ClearTopLevelTag<tag__ongoing_merged_rows>();
	ClearTopLevelTag<tag__calculate_consolidated_total_number_rows>();
	ClearTopLevelTag<tag__calculate_consolidated_total_number_rows__instance_vector>();
	ClearTopLevelTag<tag__branches_and_leaves>();

	PurgeTags<boost::pool_allocator_tag>();
	PurgeTags<boost::fast_pool_allocator_tag>();
	PurgeTags<hits_tag>();
	PurgeTags<hits_consolidated_tag>();
	PurgeTags<saved_historic_rows_tag>();
	PurgeTags<ongoing_consolidation_tag>();
	PurgeTags<ongoing_merged_rows_tag>();
	PurgeTags<remaining_tag>();
	PurgeTags<child_dmu_lookup_tag>();
	PurgeTags<calculate_consolidated_total_number_rows_tag>();

	messager.SetPerformanceLabel("");

	ClearRemaining();
}

void KadSampler::ClearRemaining()
{
	PurgeTags<remaining_tag>();
	//std::for_each(timeSlices.begin(), timeSlices.end(), [&](TimeSlices<hits_tag>::value_type  & timeSliceData)
	//{
	//	VariableGroupBranchesAndLeavesVector<hits_tag> & variableGroupBranchesAndLeavesVector = *timeSliceData.second.branches_and_leaves;
	//	VariableGroupBranchesAndLeaves & variableGroupBranchesAndLeaves = variableGroupBranchesAndLeavesVector[0];
	//	Branches<hits_tag> & branchesAndLeaves = variableGroupBranchesAndLeaves.branches;
	//	std::for_each(branchesAndLeaves.begin(), branchesAndLeaves.end(), [&](Branch const & branch)
	//	{
	//		fast__int64__to__fast_branch_output_row_vector<remaining_tag>().swap(branch.remaining);
	//	});
	//});
}
