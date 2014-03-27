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

		variableGroupTimeSliceDataWeighting.weighting_range_start = currentWeighting;

		// We know there's only one variable group currently supported, but include the loop as a reminder that
		// we may support multiple variable groups in the random sampler in the future.
		std::for_each(variableGroupBranchesAndLeavesVector.begin(), variableGroupBranchesAndLeavesVector.end(), [&](VariableGroupBranchesAndLeaves & variableGroupBranchesAndLeaves)
		{
			
			BranchesAndLeaves & branchesAndLeaves = variableGroupBranchesAndLeaves.branches_and_leaves;
			Weighting & variableGroupBranchesAndLeavesWeighting = variableGroupBranchesAndLeaves.weighting;
			variableGroupBranchesAndLeavesWeighting.weighting_range_start = currentWeighting;

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
				if (K <= numberLeaves)
				{
					//branch.number_branch_combinations = boost::math::binomial_coefficient<boost::multiprecision::cpp_int>(numberLeaves, K);
					branch.number_branch_combinations = BinomialCoefficient(numberLeaves, K);
				}

				// clear the hit cache
				branch.hit.clear();

				branchWeighting.weighting = timeSlice.Width() * branch.number_branch_combinations;
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
	boost::random::mt19937 engine(static_cast<std::int32_t>(std::time(0)));
	boost::random::uniform_int_distribution<boost::multiprecision::cpp_int> distribution(weighting.weighting_range_start, weighting.weighting_range_end);
	while (random_numbers.size() < static_cast<size_t>(how_many))
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

	boost::multiprecision::cpp_int const & random_number = *random_number_iterator;

	std::string val1 = random_number.str();
	std::string val2 = weighting.weighting.str();
	std::string val3 = weighting.weighting_range_start.str();
	std::string val4 = weighting.weighting_range_end.str();

	BOOST_ASSERT_MSG(random_number >= 0 && random_number < weighting.weighting && weighting.weighting_range_start == 0 && weighting.weighting_range_end == weighting.weighting - 1, "Invalid weights in RetrieveNextBranchAndLeaves().");

	TimeSlices::const_iterator timeSlicePtr = std::lower_bound(timeSlices.cbegin(), timeSlices.cend(), random_number, [&](std::pair<TimeSlice, VariableGroupTimeSliceData> const & timeSliceData, boost::multiprecision::cpp_int const & test_random_number)
	{
		VariableGroupTimeSliceData const & testVariableGroupTimeSliceData = timeSliceData.second;
		if (testVariableGroupTimeSliceData.weighting.weighting_range_end < test_random_number)
		{
			return true;
		}
		return false;
	});

	if (timeSlicePtr == timeSlices.cend())
	{
		int debug___ = 0;
	}

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

	const Branch & new_branch = branchesAndLeavesPtr->first;

	if (new_branch.primary_keys != branch.primary_keys)
	{
		branch.remaining.clear();
	}

	branch = new_branch;

	Leaves const & tmp_leaves = branchesAndLeavesPtr->second;

	std::string val1 = random_number.str();
	std::string val2 = boost::multiprecision::cpp_int((random_number - branch.weighting.weighting_range_start) / boost::multiprecision::cpp_int(timeSlice.Width())).str();

	// random_number should be between 0 and the binomial coefficient representing the number of combinations of K leaves out of the total number of leaves
	//BOOST_ASSERT_MSG((random_number - branch.weighting.weighting_range_start) < 0 || (random_number - branch.weighting.weighting_range_start) / boost::multiprecision::cpp_int(timeSlice.Width()) >= boost::math::binomial_coefficient<boost::multiprecision::cpp_int>(tmp_leaves.size(), K), "Random index is outside [0. binomial coefficient)");
	BOOST_ASSERT_MSG((random_number - branch.weighting.weighting_range_start) < 0 || boost::multiprecision::cpp_int((random_number - branch.weighting.weighting_range_start) / boost::multiprecision::cpp_int(timeSlice.Width())) >= BinomialCoefficient(tmp_leaves.size(), K), "Random index is outside [0. binomial coefficient)");

	// random_number is now an actual *index* to which combination of leaves in this VariableGroupTimeSliceData;
	leaves = GetLeafCombination(K, branch, tmp_leaves);

	++random_number_iterator;
	return true;

}

Leaves AllWeightings::GetLeafCombination(int const K, Branch const & branch, Leaves const & leaves)
{

	static int saved_range = -1;
	static std::mt19937 engine(static_cast<std::int32_t>(std::time(0)));

	if (static_cast<size_t>(K) >= leaves.size())
	{
		return leaves;
	}

	if (K <= 0)
	{
		return Leaves();
	}

	BOOST_ASSERT_MSG(boost::multiprecision::cpp_int(branch.hit.size()) < branch.number_branch_combinations, "The number of hits is as large as the number of combinations for a branch.  Invalid!");

	std::set<int> test_leaf_combination;

	// skip any leaf combinations returned from previous random numbers
	while (test_leaf_combination.empty() || branch.hit.count(test_leaf_combination))
	{

		test_leaf_combination.clear();

		if (boost::multiprecision::cpp_int(branch.hit.size()) > branch.number_branch_combinations / 2)
		{
			// There are so many requests that it is more efficient to populate a list with all the remaining possibilities,
			// and then pick randomly from that
			
			// A previous call may have populated "remaining"

			if (branch.remaining.size() == 0)
			{
				PopulateAllLeafCombinations(K, branch, leaves);
			}

			std::uniform_int_distribution<size_t> distribution(0, branch.remaining.size() - 1);
			size_t which_remaining_leaf_combination = distribution(engine);

			test_leaf_combination = branch.remaining[which_remaining_leaf_combination];

			branch.remaining.erase(std::remove(std::begin(branch.remaining), std::end(branch.remaining), test_leaf_combination), std::end(branch.remaining));

		}
		else
		{

			std::vector<int> remaining_leaves;
			for (size_t n = 0; n < leaves.size(); ++n)
			{
				remaining_leaves.push_back(n);
			}

			// Fill it up, with no duplicates
			while (test_leaf_combination.size() < static_cast<size_t>(K))
			{
				std::uniform_int_distribution<size_t> distribution(0, remaining_leaves.size() - 1);
				size_t index_of_index = distribution(engine);
				int index_of_leaf = remaining_leaves[index_of_index];
				remaining_leaves.erase(std::remove(std::begin(remaining_leaves), std::end(remaining_leaves), index_of_leaf), std::end(remaining_leaves));
				test_leaf_combination.insert(index_of_leaf);
			}

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
			++current_index_to_use_ptr;
		}
		++current_index;
	});

	BOOST_ASSERT_MSG(leaf_combination.size() == K, "Number of leaves generated does not equal K.");

	return leaf_combination;

}

void AllWeightings::PopulateAllLeafCombinations(int const K, Branch const & branch, Leaves const & leaves)
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

		AddPositionToRemaining(position, branch);
		bool succeeded = IncrementPosition(K, position, leaves);

		BOOST_ASSERT_MSG(succeeded || (total_added + 1) == branch.number_branch_combinations, "Invalid logic in position incrementer in sampler!");

		++total_added;

	}	

}

void AllWeightings::AddPositionToRemaining(std::vector<int> const & position, Branch const & branch)
{

	std::set<int> new_remaining;
	std::for_each(position.cbegin(), position.cend(), [&](int const position_index)
	{
		new_remaining.insert(position_index);
	});

	if (branch.hit.count(new_remaining) == 0)
	{
		branch.remaining.push_back(new_remaining);
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
