#include "RandomSampling.h"

#ifndef Q_MOC_RUN
#	include <boost/scope_exit.hpp>
#	include <boost/math/special_functions/binomial.hpp>
#	include <boost/multiprecision/random.hpp>
#	include <boost/assert.hpp>
#endif
#include <random>
#include <functional>

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

void AllWeightings::HandleBranchAndLeaf(Branch const & branch, TimeSliceLeaf & newTimeSliceLeaf, int const & variable_group_number)
{

	TimeSlice const & newTimeSlice = newTimeSliceLeaf.first;

	if (!newTimeSlice.Validate())
	{
		return;
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
		// No entries in the 'timeSlices' map yet
		// Add the entire new time slice as the first entry in the map
		AddNewTimeSlice(variable_group_number, branch, newTimeSliceLeaf);
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
			// The new slice is entirely past the end of the map.
			// Add new map entry consisting solely of the new slice.
			AddNewTimeSlice(variable_group_number, branch, newTimeSliceLeaf);
		}
		else
		{

			// The start of the new slice is to the left of the end of the map

			TimeSlice const & startMapSlice = startMapSlicePtr->first;
			auto firstMapSlicePtr = timeSlices.begin();

			if (startMapSlicePtr == firstMapSlicePtr)
			{

				// The starting slice is the first element in the map.
				// This is special-case logic on the left.
				// It means that the new time slice MIGHT start to the left of the map.
				// Or it might not - it might start IN or at the left edge of
				// the first time slice in the map.

				if (newTimeSlice.time_start < startMapSlice.time_start)
				{

					// The new time slice starts to the left of the map.

					if (newTimeSlice.time_end <= startMapSlice.time_start)
					{
						// The entire new time slice is less than the first time slice in the map.
						// Add the entire new time slice as a unit to the beginning of the map.
						AddNewTimeSlice(variable_group_number, branch, newTimeSliceLeaf);
					}
					else
					{

						// The new time slice starts to the left of the map,
						// and ends inside or at the right edge of the first time slice in the map

						// Slice off the left-most piece of the new time slice and add it to the beginning of the map
						TimeSliceLeaf new_left_slice;
						SliceOffLeft(newTimeSliceLeaf, existing_start_slice->first.time_start, new_left_slice);
						AddNewTimeSlice(variable_group_number, branch, new_left_slice);

						// For the remainder of the slice, proceed with normal case
						normalCase = true;
						mapIterator = existing_start_slice;

					}

				}
				else
				{

					// The new time slice starts at the left edge, or to the right of the left edge,
					// of the first element in the map, but is to the left of the right edge
					// of the first element in the map.

					// This is actually the normal case - it turns out that it doesn't matter
					// that this was the first element in the map.
					normalCase = true;
					mapIterator = existing_start_slice;

				}

			}
			else
			{

				// Normal case: The new time slice starts inside
				// (or at the left edge of) an existing time slice in the map (not the first)
				normalCase = true;
				mapIterator = startMapSlicePtr;

			}

		}

	}

	if (normalCase)
	{
		for (;;)
		{
			bool no_more_time_slice = HandleTimeSliceNormalCase(branch, newTimeSliceLeaf, mapIterator, variable_group_number);
			if (no_more_time_slice)
			{
				break;
			}
			if (mapIterator == timeSlices.end())
			{
				// We have a chunk left but it's past the end of the map.
				// Add it solo to the end of the map.
				AddNewTimeSlice(variable_group_number, branch, newTimeSliceLeaf);
				break;
			}
		}
	}

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
// The function returns the following data:
// If the return value is true:
// - undefined value of time slice
// - iterator to the map entry whose left edge corresponds to the left edge
//   of the original time slice (but this is unused by the algorithm)
// If the return value is false:
// - The portion of the incoming slice that extends past the right edge
//   of the map entry
// - An iterator to the map entry whose left edge matches the left edge
//   of the leftover time slice to the right.
bool AllWeightings::HandleTimeSliceNormalCase(Branch const & branch, TimeSliceLeaf & newTimeSliceLeaf, TimeSlices::iterator & mapElementPtr, int const & variable_group_number)
{

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
			MergeTimeSliceDataIntoMap(branch, newTimeSliceLeaf, newMapElementLeftPtr, variable_group_number);

			mapElementPtr = newMapElementLeftPtr;

			newTimeSliceEatenCompletelyUp = true;

		}
		else if (newTimeSlice.time_end == mapElement.time_end)
		{

			// The new time slice exactly matches the first map element.

			// Merge the new time slice with the map element.

			MergeTimeSliceDataIntoMap(branch, newTimeSliceLeaf, mapElementPtr, variable_group_number);

			newTimeSliceEatenCompletelyUp = true;

		}
		else
		{

			// The new time slice starts at the left edge of the map element,
			// and extends past the right edge.

			// Slice the first part of the time slice off so that it
			// perfectly overlaps the map element.
			// Merge it with the map element.

			SliceOffLeft(newTimeSliceLeaf, mapElement.time_end, new_left_slice);
			MergeTimeSliceDataIntoMap(branch, new_left_slice, mapElementPtr, variable_group_number);

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
			MergeTimeSliceDataIntoMap(branch, newTimeSliceLeaf, newMapElementMiddlePtr, variable_group_number);

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
			MergeTimeSliceDataIntoMap(branch, newTimeSliceLeaf, newMapElementRightPtr, variable_group_number);

			mapElementPtr = newMapElementRightPtr;

			newTimeSliceEatenCompletelyUp = true;

		}
		else
		{

			// The new time slice starts in the map element,
			// but past its left edge.
			// The new time slice ends past the right edge of the map element.

			// Slice the left part of the time slice
			// and the right part of the map element
			// so that they are equal, and merge.

			SliceOffLeft(newTimeSliceLeaf, mapElement.time_end, new_left_slice);
			SliceMapEntry(mapElementPtr, newTimeSlice.time_start, newMapElementLeftPtr, newMapElementRightPtr);
			MergeTimeSliceDataIntoMap(branch, new_left_slice, newMapElementRightPtr, variable_group_number);

			mapElementPtr = ++newMapElementRightPtr;

		}

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
void AllWeightings::MergeTimeSliceDataIntoMap(Branch const & branch, TimeSliceLeaf const & timeSliceLeaf, TimeSlices::iterator & mapElementPtr, int const & variable_group_number)
{

	VariableGroupTimeSliceData & variableGroupTimeSliceData = mapElementPtr->second;
	VariableGroupBranchesAndLeavesVector & variableGroupBranchesAndLeavesVector = variableGroupTimeSliceData.branches_and_leaves;
	VariableGroupBranchesAndLeavesVector::iterator VariableGroupBranchesAndLeavesPtr = std::find(variableGroupBranchesAndLeavesVector.begin(), variableGroupBranchesAndLeavesVector.end(), variable_group_number);
	if (VariableGroupBranchesAndLeavesPtr == variableGroupBranchesAndLeavesVector.end())
	{
		// add new branch corresponding to this variable group
		VariableGroupBranchesAndLeaves newVariableGroupBranch(variable_group_number);
		BranchesAndLeaves & newBranchesAndLeaves = newVariableGroupBranch.branches_and_leaves;
		newBranchesAndLeaves[branch].emplace(timeSliceLeaf.second); // add Leaf to the set of Leaves attached to the new Branch
		variableGroupBranchesAndLeavesVector.push_back(newVariableGroupBranch);
	}
	else
	{
		// branch already exists for this variable group
		VariableGroupBranchesAndLeaves & variableGroupBranch = *VariableGroupBranchesAndLeavesPtr;
		BranchesAndLeaves & branchesAndLeaves = variableGroupBranch.branches_and_leaves;

		// *********************************************************************************** //
		// This is where multiple rows with duplicated primary keys 
		// and overlapping time range will be wiped out.
		// ... Only one row with given primary keys and time range is allowed.
		// *********************************************************************************** //
		branchesAndLeaves[branch].emplace(timeSliceLeaf.second); // add Leaf to the set of Leaves attached to the new Branch, if it doesn't already exist
	}

}

void AllWeightings::CalculateWeightings(int const K)
{

	boost::multiprecision::cpp_int currentWeighting = 0;

	std::for_each(timeSlices.begin(), timeSlices.end(), [&](std::pair<TimeSlice, VariableGroupTimeSliceData> & timeSliceEntry)
	{

		TimeSlice & timeSlice = timeSliceEntry.first;
		VariableGroupTimeSliceData & variableGroupTimeSliceData = timeSliceEntry.second;
		VariableGroupBranchesAndLeavesVector & variableGroupBranchesAndLeavesVector = variableGroupTimeSliceData.branches_and_leaves;
		Weighting & variableGroupTimeSliceDataWeighting = variableGroupTimeSliceData.weighting;

		if (variableGroupBranchesAndLeavesVector.size() > 1)
		{
			boost::format msg("Only one top-level variable group is currently supported for the random / full sampler.");
			throw NewGeneException() << newgene_error_description(msg.str());
		}

		variableGroupTimeSliceDataWeighting.weighting_range_start = currentWeighting;

		// We know there's only one variable group currently supported, but include the loop as a reminder that
		// we may support multiple variable groups in the random sampler in the future.
		std::for_each(variableGroupBranchesAndLeavesVector.begin(), variableGroupBranchesAndLeavesVector.end(), [&](VariableGroupBranchesAndLeaves & variableGroupBranchesAndLeaves)
		{
			
			BranchesAndLeaves & branchesAndLeaves = variableGroupBranchesAndLeaves.branches_and_leaves;
			Weighting & variableGroupBranchesAndLeavesWeighting = variableGroupBranchesAndLeaves.weighting;
			variableGroupBranchesAndLeavesWeighting.weighting_range_start = currentWeighting;

			std::for_each(branchesAndLeaves.begin(), branchesAndLeaves.end(), [&](std::pair<Branch, Leaves> & branchAndLeaves)
			{

				Branch & branch = branchAndLeaves.first;
				Leaves & leaves = branchAndLeaves.second;

				Weighting & branchWeighting = branch.weighting;

				// Count the leaves
				int numberLeaves = static_cast<int>(leaves.size());

				// The number of K-ad combinations for this branch is easily calculated.
				// It is just the binomial coefficient (assuming K <= N)

				boost::multiprecision::cpp_int number_branch_combinations = 1; // covers K > numberLeaves condition
				if (K <= numberLeaves)
				{
					number_branch_combinations = boost::math::binomial_coefficient<boost::multiprecision::cpp_int>(numberLeaves, K);
				}

				// clear the hit cache
				branch.hit.clear();

				branchWeighting.weighting = timeSlice.Width() * number_branch_combinations;
				branchWeighting.weighting_range_start = currentWeighting;
				branchWeighting.weighting_range_end = branchWeighting.weighting_range_start + branchWeighting.weighting - 1;
				currentWeighting += branchWeighting.weighting;

				variableGroupBranchesAndLeavesWeighting.weighting += branchWeighting.weighting;
				variableGroupBranchesAndLeavesWeighting.weighting_range_end += branchWeighting.weighting;

			});

			variableGroupBranchesAndLeavesWeighting.weighting_range_end -= 1;

			variableGroupTimeSliceDataWeighting.weighting_range_end += variableGroupBranchesAndLeavesWeighting.weighting;
			variableGroupTimeSliceDataWeighting.weighting += variableGroupBranchesAndLeavesWeighting.weighting;

		});

		variableGroupTimeSliceDataWeighting.weighting_range_end -= 1;

		weighting.weighting_range_end += variableGroupTimeSliceDataWeighting.weighting;
		weighting.weighting += variableGroupTimeSliceDataWeighting.weighting;

	});

	weighting.weighting_range_end -= 1;

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

	random_numbers.clear();
	boost::random::mt19937 engine(std::time(0));
	boost::random::uniform_int_distribution<boost::multiprecision::cpp_int> distribution(weighting.weighting_range_start, weighting.weighting_range_end);
	while (random_numbers.size < how_many)
	{
		random_numbers.insert(distribution(engine));
	}
	random_number_iterator = random_numbers.cbegin();

}

bool AllWeightings::RetrieveNextBranchAndLeaves(int const K, Branch & branch, Leaves & leaves, TimeSlice & time_slice)
{
	
	if (random_number_iterator == random_numbers.cend())
	{
		return false;
	}

	boost::multiprecision::cpp_int & random_number = *random_number_iterator;

	TimeSlices::const_iterator timeSlicePtr = std::lower_bound(timeSlices.cbegin(), timeSlices.cend(), random_number, [&](std::pair<TimeSlice, VariableGroupTimeSliceData> const & timeSliceData, boost::multiprecision::cpp_int const & test_random_number)
	{
		VariableGroupTimeSliceData const & testVariableGroupTimeSliceData = timeSliceData.second;
		if (testVariableGroupTimeSliceData.weighting.weighting_range_end < test_random_number)
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
		
	BranchesAndLeaves::const_iterator branchesAndLeavesPtr = std::lower_bound(branchesAndLeaves.cbegin(), branchesAndLeaves.cend(), random_number, [&](std::pair<Branch, Leaves> const & testBranchAndLeaves, boost::multiprecision::cpp_int const & test_random_number)
	{
		Branch const & testBranch = testBranchAndLeaves.first;
		if (testBranch.weighting.weighting_range_end < test_random_number)
		{
			return true;
		}
		return false;
	});

	// random_number should be between 0 and the binomial coefficient representing the number of combinations of K leaves out of the total number of leaves
	BOOST_ASSERT_MSG((random_number - branch.weighting.weighting_range_start) < 0 || (random_number - branch.weighting.weighting_range_start) / timeSlice.Width() >= boost::math::binomial_coefficient<boost::multiprecision::cpp_int>(leaves.size(), K), "Random index is outside [0. binomial coefficient)");

	Branch const & branch = branchesAndLeavesPtr->first;
	Leaves const & leaves = branchesAndLeavesPtr->second;

	// random_number should be between 0 and the binomial coefficient representing the number of combinations of K leaves out of the total number of leaves
	if (index < 0 || index >= boost::math::binomial_coefficient<boost::multiprecision::cpp_int>(leaves.size(), K))
	{
		boost::format msg("Random index out of range in the random sampler!");
		throw NewGeneException() << newgene_error_description(msg.str());
	}

	// random_number is now an actual *index* to which combination of leaves in this VariableGroupTimeSliceData;
	Leaves KAd = GetLeafCombination(K, branch, leaves);

	++random_number_iterator;
	return true;

}

Leaves AllWeightings::GetLeafCombination(int const K, Branch const & branch, Leaves const & leaves)
{

	static int saved_range = -1;
	static std::mt19937 engine(std::time(0));
	static std::uniform_int_distribution<int> distribution(0, 0);
	static std::function<int ()> rng;

	if (K >= leaves.size())
	{
		return leaves;
	}

	if (K <= 0)
	{
		return Leaves();
	}

	if (saved_range != leaves.size())
	{
		saved_range = leaves.size();
		std::uniform_int_distribution<int> tmp_distribution(0, saved_range - 1);
		distribution.param(tmp_distribution.param());
		rng = std::bind(distribution, engine);
	}

	std::set<int> test_leaf_combination;

	// skip any leaf combinations returned from previous random numbers
	while (test_leaf_combination.empty() || branch.hit.count(test_leaf_combination))
	{

		test_leaf_combination.clear();

		// Fill it up, with no duplicates
		while (test_leaf_combination.size() < K)
		{
			test_leaf_combination.insert(rng());
		}

	}

	branch.hit.insert(test_leaf_combination);

	Leaves leaf_combination;
	std::set<int>::const_iterator current_index_to_use_ptr = test_leaf_combination.cbegin();
	int current_index_to_use = *current_index_to_use_ptr;
	int current_index = 0;
	std::for_each(leaves.cbegin(), leaves.cend(), [&](Leaf const & leaf)
	{
		if (current_index_to_use_ptr == test_leaf_combination.cend())
		{
			return;
		}
		current_index_to_use = *current_index_to_use_ptr;
		if (current_index == current_index_to_use)
		{
			leaf_combination.insert(leaf);
		}
		++current_index;
		++current_index_to_use_ptr;
	});

	BOOST_ASSERT_MSG(leaf_combination.size() == K, "Number of leaves generated does not equal K.");

	return leaf_combination;

}
