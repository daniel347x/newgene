#include "RandomSampling.h"

void AllWeightings::AddLeafToTimeSlices(TimeSliceLeaf const & newTimeSliceLeaf)
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
	auto const & existing_start_slice          = timeSlices.cbegin();
	auto const & existing_one_past_end_slice   = timeSlices.cend();

	if (existing_start_slice == existing_one_past_end_slice)
	{
		
		// No entries in the 'timeSlices' map yet

		// Add the entire new time slice as the first entry in the map
	
	}
	else
	{
	
		auto startMapSlicePtr = std::upper_bound(timeSlices.begin(), timeSlices.end(), newTimeSliceLeaf, &AllWeightings::is_rhs_start_time_greater_than_lhs_end_time);
		bool start_of_new_slice_is_past_end_of_map = false;
		if (startMapSlicePtr == existing_one_past_end_slice)
		{
			start_of_new_slice_is_past_end_of_map = true;
		}

		if (start_of_new_slice_is_past_end_of_map)
		{
			// The new slice is entirely past the end of the map.
			// Add new map entry consisting solely of the new slice.
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

					}
					else
					{

						// The new time slice starts to the left of the map,
						// and ends inside or at the right edge of the first time slice in the map

						// Slice off the left-most piece of the new time slice and add it to the beginning of the map
						// and add as new map entry consisting solely of this slice

						// For the remainder of the slice, proceed with normal case

					}

				}
				else
				{

					// The new time slice starts at the left edge, or to the right of the left edge,
					// of the first element in the map, but is to the left of the right edge
					// of the first element in the map.

					// This is actually the normal case - it turns out that it doesn't matter
					// that this was the first element in the map.

				}

			}
			else
			{

				// Normal case: The new time slice starts inside
				// (or at the left edge of) an existing time slice in the map (not the first)

			}

		}

	}

}

bool AllWeightings::HandleTimeSliceNormalCase(TimeSliceLeaf & newTimeSliceLeaf, TimeSlices::iterator & mapElementPtr)
{

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
	// - Was the entire slice merged, or is there a part of the slice
	//   that extends beyond the right edge of the map entry?
	// The function returns the following data:
	// If the return value is TRUE:
	// - undefined values
	// If the return value is FALSE:
	// - The portion of the incoming slice that extends past the right edge
	//   of the map entry
	// - An iterator to the map entry to the right of the slice, if there is one,
	//   or an iterator to one past the end of the map if there isn't one ("end()").

	TimeSlice const & newTimeSlice = newTimeSliceLeaf.first;
	TimeSlice const & mapElement   = mapElementPtr->first;

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

			// 

			newTimeSliceEatenCompletelyUp = true;

		}
		else if (newTimeSlice.time_end == mapElement.time_end)
		{

			// The new time slice exactly matches the first map element.

			// Merge the new time slice with the map element.

			newTimeSliceEatenCompletelyUp = true;

		}
		else
		{

			// The new time slice starts at the left edge of the map element,
			// and extends past the right edge.

			// Slice the first part of the time slice off so that it
			// perfectly overlaps the map element.
			// Merge it with the map element.

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

			newTimeSliceEatenCompletelyUp = true;

		}
		else
		{

			// The new time slice starts in the map element,
			// but past its left edge.
			// The new time slice ends past the right edge of the map element.

			// Slice the left part of the time slice
			// and the right part of the map element
			// so that they are equal, and merge,

		}

	}

	if (newTimeSliceEatenCompletelyUp)
	{
		return true;
	}


	return false;

}

// breaks an existing map entry into two pieces and returns an iterator to both
void AllWeightings::SliceMapEntry(std::int64_t const middle, TimeSlices::iterator & newMapElementLeftPtr, TimeSlices::iterator & newMapElementRightPtr)
{

}

// breaks an existing map entry into three pieces and returns an iterator to the middle piece
void AllWeightings::SliceMapEntry(std::int64_t const left, std::int64_t const right, TimeSlices::iterator & newMapElementMiddlePtr)
{

}

void AllWeightings::CalculateWeightings()
{

}
