#include "RandomSampling.h"

void AllWeightings::AddLeafToTimeSlices(TimeSliceLeaf & newTimeSliceLeaf)
{

	if (!newTimeSliceLeaf.first.Validate())
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
		auto endMapSlicePtr   = std::lower_bound(timeSlices.begin(), timeSlices.end(), newTimeSliceLeaf, &AllWeightings::is_rhs_end_time_less_than_lhs_end_time);

		TimeSlice const & startMapSlice = startMapSlicePtr->first;

	}

}

void AllWeightings::CalculateWeightings()
{

}
