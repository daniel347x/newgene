#include "RandomSampling.h"

void AllWeightings::AddLeafToTimeSlices(TimeSliceLeaf & timeSliceLeaf)
{

	if (!timeSliceLeaf.first.Validate())
	{
		return;
	}

	auto startSlice = std::upper_bound(timeSlices.cbegin(), timeSlices.cend(), timeSliceLeaf, is_rhs_start_time_greater_than_lhs_end_time);
	auto endSlice   = std::lower_bound(timeSlices.cbegin(), timeSlices.cend(), timeSliceLeaf, is_rhs_end_time_less_than_lhs_end_time);

}

void AllWeightings::CalculateWeightings()
{

}
