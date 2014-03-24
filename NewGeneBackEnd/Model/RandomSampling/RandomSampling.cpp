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
	
		// existing_start_slice guaranteed to represent an existing instance of a map element

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
					// of the first element in the map.

					// This is actually the normal case - it turns out that it doesn't matter
					// that this was the first element in the map.

				}

			}
			else
			{

				// Normal case: The new time slice starts inside
				// (or at the left edge of) an existing time slice in the map (not the first)

				// Call a function that takes the following arguments:
				// - An entry in the map
				// - A time slice
				// The time slice's left edge is guaranteed to be inside (not at the right edge of)
				// an existing (non-"end()") entry of the map, or at the left edge of this entry.
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
				// - Any portion of the incoming slice that extends past the right edge
				//   of the map entry
				// - An iterator to the map entry to the right of the slice, if there is one,
				//   or an iterator to one past the end of the map if there isn't one.

			}

		}

	}

}

void AllWeightings::CalculateWeightings()
{

}
