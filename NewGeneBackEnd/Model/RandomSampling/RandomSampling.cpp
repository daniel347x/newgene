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
	
	}
	else
	{
	
		// existing_start_slice guaranteed to represent an existing instance of a map element

		auto startMapSlicePtr = std::upper_bound(timeSlices.begin(), timeSlices.end(), newTimeSliceLeaf, &AllWeightings::is_rhs_start_time_greater_than_lhs_end_time);
		//auto endMapSlicePtr   = std::lower_bound(timeSlices.begin(), timeSlices.end(), newTimeSliceLeaf, &AllWeightings::is_rhs_end_time_less_than_lhs_end_time);

		bool start_of_new_slice_is_past_end_of_map = false;
		//bool end_of_new_slice_is_past_end_of_map   = false;
		if (startMapSlicePtr == existing_one_past_end_slice)
		{
			start_of_new_slice_is_past_end_of_map = true;
		}
		//if (endMapSlicePtr == existing_one_past_end_slice)
		//{
		//	end_of_new_slice_is_past_end_of_map = true;
		//}

		//if (!start_of_new_slice_is_past_end_of_map && !end_of_new_slice_is_past_end_of_map)
		if (!start_of_new_slice_is_past_end_of_map)
		{

			// // Both the start of the new slice and the end of the new slice
			// // are to the left of the end of the map

			// The start of the new slice is to the left of the end of the map

			TimeSlice const & startMapSlice = startMapSlicePtr->first;
			//TimeSlice const & endMapSlice   = endMapSlicePtr  ->first; // unused

			auto firstMapSlicePtr = timeSlices.begin();

			if (startMapSlicePtr == firstMapSlicePtr)
			{

				// The starting slice is the first element in the map.
				// This is special-case logic on the left.
				// It means that the new time slice MIGHT start to the left of the map.
				// Or it might not - it might start IN or at the left edge of
				// the first time slice in the map.

				if (endMapSlicePtr == firstMapSlicePtr)
				{
					
					// The ending slice is also the first element in the map.
					
					if (newTimeSlice.time_start < startMapSlice.time_start)
					{

						// The new time slice starts to the left of the map.

						if (newTimeSlice.time_end <= startMapSlice.time_end)
						{

							// The entire new time slice is less than the first time slice in the map.
							// Add the entire new time slice as a unit to the beginning of the map.

						}
						else
						{

							// The new time slice starts to the left of the map,
							// and ends inside or at the right edge of the first time slice in the map

							// Slice off the left-most piece of the new time slice and add it to the beginning of the map
							// ... here

							if (newTimeSlice.time_end < startMapSlice.time_end)
							{

								// The new time slice starts to the left of the map, and stretches into the first map element.
								// The remainder of the new time slice fits inside the first time slice in the map,
								// with space left over in the first time slice in the map.

								// Slice the first time slice in the map into two pieces.
								// Merge the first piece with the new time slice.
								// Leave the second piece unchanged.

							}
							else
							{

								// The new time slice starts to the left of the map, and stretches into the first map element.
								// The remainder of the new time slice exactly matches the first time slice in the map.

								// Merge the new time slice with the first time slice in the map.

							}

						}

					}
					else
					{

						// The new time slice starts at the left edge or inside the first time slice of the map.

						if (newTimeSlice.time_start == startMapSlice.time_start)
						{

							// The new time slice starts at the left edge of the first time slice of the map.

							if (newTimeSlice.time_end < startMapSlice.time_end)
							{

								// The new time slice starts at the left edge of the first slice of the map,
								// and fits inside the first slice in the map,
								// with space left over in the first time slice in the map.

								// Slice the first time slice in the map into two pieces.
								// Merge the first piece with the new time slice.
								// Leave the second piece unchanged.

							}
							else
							{

								// The new time slice exactly matches the first slice in the map.

								// Merge the new time slice with the first time slice in the map.

							}

						}
						else
						{

							// The new time slice is inside the first time slice of the map,
							// but starts past the left edge.

							if (newTimeSlice.time_end < startMapSlice.time_end)
							{

								// The new time slice starts in the first time slice of the map,
								// but past the left edge.
								// The new time slice ends before the right edge of the first slice of the map.

								// Slice the first time slice in the map into three pieces.
								// The first is unchanged.
								// The second merges with the new time slice.
								// The third is unchanged.

							}
							else
							{

								// The new time slice starts in the first time slice of the map,
								// but past the left edge.
								// The new time slice ends at exactly the right edge of the first slice of the map.

								// Slice the first time slice in the map into two pieces.
								// The first is unchanged.
								// The second merges with the new time slice
								// (with the right edge equal to the right edge of the first slice of the map).

							}

						}

					}

				}
				else
				{

					// The ending slice exists, but is past the first element in the map.

					if (newTimeSlice.time_start < startMapSlice.time_start)
					{

						// The new slice starts to the left of the map,
						// and extends beyond the first slice of the map.

						// Cut away the leftmost piece of the new slice and add it to the start of the map.

						// Cut the next piece of the new slice to exactly match the first slice of the map,
						// and merge it.
						
						// Continue with the algorithm by calling a function with the initial condition:
						// - New slice's left edge matches the left edge of a given slice of the map

					}
					else
					{

						if (newTimeSlice.time_start == startMapSlice.time_start)
						{

							// The new slice starts at the left edge of the first entry in the map,
							// and extends beyond the first entry of the map.

							// Slice the new slice into two pieces.
							// The first piece already starts at exactly the left edge of the first entry in the map,
							// and after the new slice is cut, ends at exactly the right edge of the first entry of the map -
							// merge this with the full first slice of the map.

							// Continue with the algorithm by calling a function with the initial condition:
							// - New slice's left edge matches the left edge of a given entry in the map

						}
						else
						{

							// The new slice starts past the left edge of the first entry in the map,
							// but in the first entry of the map,
							// and extends beyond the first entry of the map.

							// Take the first entry in the map and slice it into two pieces.
							// Also, take the new slice and slice it into two pieces.
							// The left side of the first entry of the map remains as it was.
							// The right side of the first entry of the map merges with the left part of the new slice.
							
							// Continue with the algorithm by calling a function with the initial condition:
							// - New slice's left edge matches the left edge of a given entry in the map

						}

					}

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
				// the entry in the map, or at the left edge of the entry of the map.
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
		else
		if (!start_of_new_slice_is_past_end_of_map && end_of_new_slice_is_past_end_of_map)
		{

			// The start of the new slice is to the left of the end of the map,
			// and the end of the new slice is past the end of the map.

			TimeSlice const & startMapSlice = startMapSlicePtr->first;
			auto firstMapSlicePtr = timeSlices.begin();

			if (startMapSlicePtr == firstMapSlicePtr)
			{

				// The starting slice is the first element in the map.
				// This is special-case logic on the left.
				// It means that the new time slice MIGHT start to the left of the map.
				// Or it might not - it might start IN or at the left edge of
				// the first time slice in the map.

			}
			else
			{

				// 

			}

		}
		else
		{

			// Both the start of the new slice and the end of the new slice
			// are past the end of the map.

			// Add a new entry to the end of the map
			// consisting solely of the new slice.

		}

	}

}

void AllWeightings::CalculateWeightings()
{

}
