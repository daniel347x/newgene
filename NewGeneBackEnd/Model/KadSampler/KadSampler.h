#ifndef RANDOMSAMPLING_NEWGENE_H
#define RANDOMSAMPLING_NEWGENE_H

#include <cstdint>
#include <vector>
#include <set>
#include <map>
#include <tuple>
#include <memory>
#include <stdio.h>
#ifndef Q_MOC_RUN
#	include <boost/multiprecision/number.hpp>
#	include <boost/multiprecision/cpp_int.hpp>
#	include <boost/multiprecision/cpp_dec_float.hpp>
#	include <boost/variant.hpp>
#	include <boost/pool/pool_alloc.hpp>
#	include "boost/date_time/posix_time/posix_time.hpp"
#endif
#include "../../Utilities/NewGeneException.h"
#include "../../Utilities/TimeRangeHelper.h"
#include "../../sqlite/sqlite-amalgamation-3071700/sqlite3.h"
#include "../TimeGranularity.h"
#include "../../Messager/Messager.h"
#include "../../Utilities/FastMap.h"
#include "../../Utilities/NewGeneMemoryPool.h"

typedef std::basic_string<char, std::char_traits<char>, boost::pool_allocator<char>> fast_string;

template <typename MEMORY_TAG>
using fast_short_to_short_map = FastMapMemoryTag<std::int16_t, std::int16_t, MEMORY_TAG>;

typedef boost::variant<std::int32_t, double, fast_string> InstanceData;
typedef boost::variant<std::int32_t, double, fast_string> DMUInstanceData;
typedef boost::variant<std::int32_t, double, fast_string> SecondaryInstanceData;

// ******************************************************************************************* //
//
// MEMORY_TAG is the tag - just an empty, uniquely-named struct - used by the internals
// of Boost Pool to mark different global memory pools.
// 
// Whenever the code instantiates an instance of any of the classes below,
// the code (namely, us) gets to choose which pool in which to store the object in memory.
//
// The benefit of using Boost Pool is that our code can free a block of memory (pool) wholesale,
// without calling the destructor for any of the objects in the pool, which is a major
// profiler-demonstrated optimization.
//
// By controlling which pool each object is created in, NewGene has the ability to 
// have fine-grain control over which objects are freed at any given time.
//
// This code has pool-enabled a variety of different classes, for convenience.
// Any class templatized with the MEMORY_TAG parameter is pool-enabled.
//
// ******************************************************************************************* //
template <typename MEMORY_TAG>
using InstanceDataVector = FastVectorMemoryTag<InstanceData, MEMORY_TAG>;

template <typename MEMORY_TAG>
using DMUInstanceDataVector  = InstanceDataVector<MEMORY_TAG>;

template <typename MEMORY_TAG>
using ChildDMUInstanceDataVector = InstanceDataVector<MEMORY_TAG>;

template <typename MEMORY_TAG>
using SecondaryInstanceDataVector = InstanceDataVector<MEMORY_TAG>;

template <typename MEMORY_TAG>
using fast_short_vector = FastVectorMemoryTag<std::int16_t, MEMORY_TAG>;

template <typename MEMORY_TAG>
using fast_int_vector = FastVectorMemoryTag<int, MEMORY_TAG>;

template <typename MEMORY_TAG>
using fast_int_set = FastSetMemoryTag<int, MEMORY_TAG>;

template <typename MEMORY_TAG>
using fast_short_to_int_map = FastMapMemoryTag<std::int16_t, std::int32_t, MEMORY_TAG>;

template <typename MEMORY_TAG>
using fast_short_to_int_map__loaded = FastMapMemoryTag<std::int16_t, std::int32_t, MEMORY_TAG>; // known memory allocation hog that can crash in a somewhat fragmented heap, so throttle it way down by forcing small maximum block sizes but that won't crash

template <typename MEMORY_TAG>
using fast__short__to__fast_short_to_int_map__loaded = FastMapMemoryTag<std::int16_t, fast_short_to_int_map__loaded<MEMORY_TAG>, MEMORY_TAG>;

// Row ID -> secondary data for that row for a given (unspecified) leaf
template <typename MEMORY_TAG>
using DataCache = FastMapMemoryTag<std::int32_t, SecondaryInstanceDataVector<MEMORY_TAG>, MEMORY_TAG>;

template <typename MEMORY_TAG>
using fast_short_to_data_cache_map = FastMapMemoryTag<std::int16_t, DataCache<MEMORY_TAG>, MEMORY_TAG>;


// For the below classes, instead of templatizing the class on MEMORY_TAG (which represents an empty, uniquely named struct),
// we just explicitly use a particular struct as the tag.
struct newgene_randomvector_tag {};
typedef boost::singleton_pool<newgene_randomvector_tag, sizeof(FastVectorCppInt)>
RandomVectorPool;

struct newgene_randomset_tag {};
typedef boost::singleton_pool<newgene_randomset_tag, sizeof(FastSetCppInt)>
RandomSetPool;

template <typename TOPLEVEL_POOL_TAG>
using TopLevelObjectsPool = boost::singleton_pool<TOPLEVEL_POOL_TAG, sizeof(TOPLEVEL_POOL_TAG::type)>;



class KadSampler;

class TimeSlice
{

	public:

		TimeSlice()
			: time_start { 0 }
			, time_end { 0 }
			, none { true }
			, minus_infinity { true }
			, plus_infinity { true }

		{
			CheckForAndSetNoTimeRangeGranularity();
			Validate();
		}

		TimeSlice(std::int64_t const time_start_, std::int64_t const time_end_)
			: time_start { time_start_ }
			, time_end { time_end_ }
			, none { false }
			, minus_infinity { false }
			, plus_infinity { false }
		{
			CheckForAndSetNoTimeRangeGranularity();
			Validate();
		}

		TimeSlice(TimeSlice const & rhs)
			: time_start { rhs.time_start }
			, time_end { rhs.time_end }
			, none { rhs.none }
			, minus_infinity { rhs.minus_infinity }
			, plus_infinity { rhs.plus_infinity }
		{
			CheckForAndSetNoTimeRangeGranularity();
			Validate();
		}

		std::string toStringStart() const
		{
			bool do_calc = false;
			if (none)
			{
				if (!minus_infinity)
				{
					do_calc = true;
				}
			}
			else
			{
				do_calc = true;
			}

			if (do_calc)
			{
				return TimeRange::convertMsSinceEpochToString(time_start, true);
			}

			return std::string();
		}

		std::string toStringEnd() const
		{
			bool do_calc = false;
			if (none)
			{
				if (!plus_infinity)
				{
					do_calc = true;
				}
			}
			else
			{
				do_calc = true;
			}

			if (do_calc)
			{
				return TimeRange::convertMsSinceEpochToString(time_end, true);
			}

			return std::string();
		}

		void Merge(TimeSlice const & rhs)
		{

			if (none)
			{
				if (rhs.none)
				{
					if (minus_infinity && plus_infinity)
					{
						// accept our values - we encompass, or are equal to, the incoming time slice
					}
					else if (minus_infinity && !plus_infinity)
					{
						if (rhs.minus_infinity && rhs.plus_infinity)
						{
							// accept the RHS values - the RHS encompasses us
							plus_infinity = true;
							time_end = 0;
						}
						else if (rhs.minus_infinity && !rhs.plus_infinity)
						{
							// Both us and RHS are infinite at the left, but cropped at the right.
							// Take the larger of the right values.
							time_end = std::max(time_end, rhs.time_end);
						}
						else if (!rhs.minus_infinity && rhs.plus_infinity)
						{
							// We are cropped at the right;
							// RHS is cropped at the left.

							// Confirm that we overlap, with no gap between us.
							if (time_end < rhs.time_start)
							{
								boost::format
								msg("Logic error merging time slices!  Both current time slice and RHS time slice are identified as having no time granularity, but current time slice is cropped at the right and RHS time slice is cropped at the left, and there is a gap between them.");
								throw NewGeneException() << newgene_error_description(msg.str());
							}

							// Set ourselves back to infinite, which is the merging of ourself with RHS.
							plus_infinity = true;
							time_end = 0;

						}
						else
						{
							boost::format msg("Logic error merging time slices!  RHS time slice is identified as having no time granularity, but both endpoints are finite.");
							throw NewGeneException() << newgene_error_description(msg.str());
						}
					}
					else if (!minus_infinity && plus_infinity)
					{
						if (rhs.minus_infinity && rhs.plus_infinity)
						{
							// accept the RHS values - the RHS encompasses us
							minus_infinity = true;
							time_start = 0;
						}
						else if (rhs.minus_infinity && !rhs.plus_infinity)
						{
							// Both us and RHS are infinite,
							// but we are cropped at the left,
							// and RHS is cropped at the right.

							// Confirm that we overlap, with no gap between us.
							if (time_start > rhs.time_end)
							{
								boost::format
								msg("Logic error merging time slices!  Both current time slice and RHS time slice are identified as having no time granularity, but current time slice is cropped at the left and RHS time slice is cropped at the right, and there is a gap between them.");
								throw NewGeneException() << newgene_error_description(msg.str());
							}

							// Make ourselves infinite again, to merge with RHS
							minus_infinity = true;
							time_start = 0;
						}
						else if (!rhs.minus_infinity && rhs.plus_infinity)
						{
							// We are cropped at the left;
							// RHS is cropped at the left.

							// Take the smaller of the left values.
							time_start = std::min(time_start, rhs.time_start);
						}
						else
						{
							boost::format msg("Logic error merging time slices!  RHS time slice is identified as having no time granularity, but both endpoints are finite.");
							throw NewGeneException() << newgene_error_description(msg.str());
						}
					}
					else
					{
						boost::format msg("Logic error merging time slices!  Current time slice is identified as having no time granularity, but both endpoints are finite.");
						throw NewGeneException() << newgene_error_description(msg.str());
					}
				}
				else
				{

					// We are infinite.  RHS is not.

					if (minus_infinity && plus_infinity)
					{
						// nothing to do - remain infinite
					}
					else if (minus_infinity && !plus_infinity)
					{
						// We are cropped at the right

						// Check that we overlap with RHS
						if (time_end < rhs.time_start)
						{
							boost::format
							msg("Logic error merging time slices!  Current time slice is identified as having no time granularity (and is cropped at the right), and RHS is identified as having time granularity,  and its left edge is to the right of the current time slice's right edge.");
							throw NewGeneException() << newgene_error_description(msg.str());
						}

						// Take the larger of the right values.
						time_end = std::max(time_end, rhs.time_end);
					}
					else if (!minus_infinity && plus_infinity)
					{
						// We are cropped at the left

						// Check that we overlap with RHS
						if (time_start > rhs.time_end)
						{
							boost::format
							msg("Logic error merging time slices!  Current time slice is identified as having no time granularity (and is cropped at the left), and RHS is identified as having time granularity,  and its right edge is to the left of the current time slice's left edge.");
							throw NewGeneException() << newgene_error_description(msg.str());
						}

						// Take the smaller of the left values.
						time_start = std::min(time_start, rhs.time_start);
					}
					else
					{
						boost::format msg("Logic error merging time slices!  Current time slice is identified as having no time granularity, but both endpoints are finite.");
						throw NewGeneException() << newgene_error_description(msg.str());
					}

				}
			}
			else if (rhs.none)
			{

				// We are not infinite, but RHS is

				// Utilize the above logic
				TimeSlice dummy = rhs;
				dummy.Merge(*this);
				*this = dummy;

			}
			else
			{

				// Normal case: Neither LHS nor RHS are infinite

				// Make sure we overlap

				if (time_start > rhs.time_end)
				{
					boost::format msg("Logic error merging time slices!  Current time slice is to the right of RHS, with a gap between.");
					throw NewGeneException() << newgene_error_description(msg.str());
				}

				if (time_end < rhs.time_start)
				{
					boost::format msg("Logic error merging time slices!  Current time slice is to the left of RHS, with a gap between.");
					throw NewGeneException() << newgene_error_description(msg.str());
				}

				// Take the larger of the right values.
				time_end = std::max(time_end, rhs.time_end);

				// Take the smaller of the left values.
				time_start = std::min(time_start, rhs.time_start);

			}

			Validate();

		}

		void CheckForAndSetNoTimeRangeGranularity()
		{
			if (time_start == 0 && time_end == 0)
			{
				// this means we really have no time range granularity

				none = true;
				minus_infinity = true;
				plus_infinity = true;
			}
		}

		std::int64_t getWidth() const
		{
			if (none && (minus_infinity || plus_infinity))
			{
				boost::format msg("Attempting to get the width of an infinite time slice!");
				throw NewGeneException() << newgene_error_description(msg.str());
			}

			return getEnd() - getStart();
		}

		void loop_through_time_units(TIME_GRANULARITY const & time_granularity, boost::function<void(std::int64_t const, std::int64_t const)> & misc_function) const
		{
			// ************************************************************************** //
			// This function is called in the case where ALL we need to do is 
			// determine the time widths (starting & ending values) of the
			// sub-time-units in each TimeSlice - and then call a
			// miscellaneous function with these values for each such sub-time-unit.
			// 
			// Note that all such time widths of sub-time-units (each corresponding
			// to an entry in the "hits" array of the given branch) will have
			// the width of the basic time unit of the time granularity of the 
			// unit of analysis associated with the primary variable group,
			// EXCEPT possibly for the FIRST and LAST sub-time-units, which may
			// be cropped.
			// ************************************************************************** //

			// Just do the calculation of how many
			// absolute total time units are overlapped by the current time slice.
			// Start at the beginning, and loop through.
			std::int64_t current_time_start = time_start;

			if (time_granularity == TIME_GRANULARITY__NONE)
			{
				misc_function(time_start, time_end);
			}
			else
			{
				while (current_time_start < time_end)
				{
					std::int64_t time_to_use_for_start = current_time_start;
					std::int64_t current_start_time_incremented_by_1_ms = time_to_use_for_start + 1;
					std::int64_t time_start_aligned_higher = TimeRange::determineAligningTimestamp(current_start_time_incremented_by_1_ms, time_granularity, TimeRange::ALIGN_MODE_UP);
					std::int64_t time_to_use_for_end = time_start_aligned_higher;
					if (time_to_use_for_end > time_end) { time_to_use_for_end = time_end; }

					misc_function(time_to_use_for_start, time_to_use_for_end);

					current_time_start = time_to_use_for_end;
				}
			}

		};

		// ***************************************************************************** //
		// Return the number of time slots corresponding to the given time unit
		// ***************************************************************************** //
		std::int64_t WidthForWeighting(TIME_GRANULARITY const & time_granularity) const
		{

			if (none)
			{
				// This function is only called for the primary variable group.
				// In this case, it's all-or-none:
				// Either *all* rows have no time granularity, or none do.
				// Assume, therefore, that if any call to this function
				// hits this block, all calls will for this run.
				// This is a scenario where each branch has only one time slice
				// and it should have a weight of 1.
				return 1;
			}

			int number_of_time_units = 0;
			loop_through_time_units(time_granularity, boost::function<void(std::int64_t const, std::int64_t const)>([&](std::int64_t const time_to_use_for_start, std::int64_t const time_to_use_for_end)
			{
				++number_of_time_units;
			}));

			return number_of_time_units;

		}

		//static long double WidthForWeighting(std::int64_t const leftVal, std::int64_t const rightVal, std::int64_t const ms_per_unit_time)
		//{
		//	std::int64_t absolute = rightVal - leftVal;
		//	return WidthForWeighting(absolute, ms_per_unit_time);
		//}

		//static long double WidthForWeighting(std::int64_t const absolute, std::int64_t const ms_per_unit_time)
		//{
		//	return boost::lexical_cast<long double>(absolute) / boost::lexical_cast<long double>(ms_per_unit_time);
		//}

		TimeSlice & operator=(TimeSlice const & rhs)
		{
			if (&rhs == this)
			{
				return *this;
			}

			time_start = rhs.time_start;
			time_end = rhs.time_end;
			none = rhs.none;
			minus_infinity = rhs.minus_infinity;
			plus_infinity = rhs.plus_infinity;

			CheckForAndSetNoTimeRangeGranularity();

			Validate();

			return *this;
		}

		void Reshape(std::int64_t const & new_start, std::int64_t const & new_end)
		{

			setStart(new_start, false);
			setEnd(new_end, false);

			CheckForAndSetNoTimeRangeGranularity();

			Validate();

		}

		bool operator<(TimeSlice const & rhs) const
		{

			if (none)
			{
				if (rhs.none)
				{
					if (minus_infinity && rhs.minus_infinity)
					{
						if (plus_infinity && rhs.plus_infinity) { return false; } // we're the same
						else if (plus_infinity && !rhs.plus_infinity) { return false; } // I'm bigger
						else if (!plus_infinity && rhs.plus_infinity) { return true; } // I'm smaller
						else { return time_end < rhs.time_end; }
					}
					else if (minus_infinity && !rhs.minus_infinity) { return true; } // I'm smaller
					else if (!minus_infinity && rhs.minus_infinity) { return false; } // I'm bigger
					else
					{
						if (time_start < rhs.time_start) { return true; }
						else if (time_start > rhs.time_start) { return false; }
						else
						{
							if (plus_infinity && rhs.plus_infinity) { return false; } // we're the same
							else if (plus_infinity && !rhs.plus_infinity) { return false; } // I'm bigger
							else if (!plus_infinity && rhs.plus_infinity) { return true; } // I'm smaller
							else { return time_end < rhs.time_end; }
						}
					}
				}
				else
				{
					if (!minus_infinity)
					{
						if (time_start < rhs.time_start) { return true; } // I'm smaller
						else if (time_start > rhs.time_start) { return false; } // I'm bigger
						else { return false; } // I'm bigger: This is the case where we start at the same left edge, but I go out to infinity and rhs has a finite right edge
					}
					else { return true; } // I'm smaller - my left edge is at negative infinity, but the rhs's left edge is finite
				}
			}
			else
			{
				if (rhs.none)
				{
					return !(rhs < *this); // utilize the above logic
				}
			}

			// Normal case follows

			if (time_start < rhs.time_start) { return true; }
			else if (time_start > rhs.time_start) { return false; }
			else if (time_end < rhs.time_end)     { return true; }
			else if (time_end > rhs.time_end)     { return false; }

			return false;

		}

		inline void Validate() const
		{
			bool valid = true;

			if (none)
			{
				if (!minus_infinity && !plus_infinity)
				{
					valid = false;
				}
			}
			else if (time_end <= time_start)
			{
				valid = false;
			}

			if (!valid)
			{
				boost::format msg("Invalid time slice!");
				throw NewGeneException() << newgene_error_description(msg.str());
			}
		}

		bool DoesOverlap(TimeSlice const & rhs) const
		{
			// Checks if the two time slices overlap

			if (none)
			{

				if (rhs.none)
				{

					if (minus_infinity && plus_infinity)
					{
						return true;
					}
					else if (minus_infinity && !plus_infinity)
					{
						if (rhs.minus_infinity && rhs.plus_infinity)
						{
							return true;
						}
						else if (rhs.minus_infinity && !rhs.plus_infinity)
						{
							return true;
						}
						else if (!rhs.minus_infinity && rhs.plus_infinity)
						{
							return time_end >= rhs.time_start;
						}
						else
						{
							boost::format msg("Logic error in TimeSlice::DoesOverlap!  RHS is declared as having no time granularity, but it is cropped on both sides");
							throw NewGeneException() << newgene_error_description(msg.str());
						}
					}
					else if (!minus_infinity && plus_infinity)
					{
						if (rhs.minus_infinity && rhs.plus_infinity)
						{
							return true;
						}
						else if (rhs.minus_infinity && !rhs.plus_infinity)
						{
							return time_start <= rhs.time_end;
						}
						else if (!rhs.minus_infinity && rhs.plus_infinity)
						{
							return true;
						}
						else
						{
							boost::format msg("Logic error in TimeSlice::DoesOverlap!  RHS is declared as having no time granularity, but it is cropped on both sides");
							throw NewGeneException() << newgene_error_description(msg.str());
						}
					}
					else
					{
						boost::format msg("Logic error in TimeSlice::DoesOverlap!  Current time slice is declared as having no time granularity, but it is cropped on both sides");
						throw NewGeneException() << newgene_error_description(msg.str());
					}
				}
				else
				{

					// We are infinite, but RHS is finite

					if (minus_infinity && plus_infinity)
					{
						return true;
					}
					else if (minus_infinity && !plus_infinity)
					{
						return time_end >= rhs.time_start;
					}
					else if (!minus_infinity && plus_infinity)
					{
						return time_start <= rhs.time_end;
					}
					else
					{
						boost::format msg("Logic error in TimeSlice::DoesOverlap!  Current time slice is declared as having no time granularity, but it is cropped on both sides");
						throw NewGeneException() << newgene_error_description(msg.str());
					}

				}

			}
			else if (rhs.none)
			{
				// Utilize above logic
				return rhs.DoesOverlap(*this);
			}

			// normal case

			if (time_end >= rhs.time_start && time_start <= rhs.time_end)
			{
				return true;
			}

			return false;

		}

		inline bool IsEndTimeGreaterThanRhsStartTime(TimeSlice const & rhs) const
		{

			bool my_right_edge_is_infinite = false;

			if (none && plus_infinity)
			{
				my_right_edge_is_infinite = true;
			}

			if (my_right_edge_is_infinite)
			{
				return true;
			}

			bool rhs_left_edge_is_infinite = false;

			if (rhs.none && rhs.minus_infinity)
			{
				rhs_left_edge_is_infinite = true;
			}

			if (rhs_left_edge_is_infinite)
			{
				return true;
			}

			// normal case

			if (time_end > rhs.time_start)
			{
				return true;
			}

			return false;

		}

		bool hasTimeGranularity() const
		{
			return !none;
		}

		bool startsAtNegativeInfinity() const
		{
			if (none)
			{
				if (minus_infinity)
				{
					return true;
				}
			}

			return false;
		}

		bool endsAtPlusInfinity() const
		{
			if (none)
			{
				if (plus_infinity)
				{
					return true;
				}
			}

			return false;
		}

		void setStart(std::int64_t const & time_start_, bool const validate = true)
		{
			time_start = time_start_;

			if (none)
			{
				minus_infinity = false;

				if (!plus_infinity)
				{
					// we had no time granularity,
					// but now we've been "snipped finite" from both directions
					none = false;
				}
			}

			CheckForAndSetNoTimeRangeGranularity();

			if (validate)
			{
				Validate();
			}
		}

		void setEnd(std::int64_t const & time_end_, bool const validate = true)
		{
			time_end = time_end_;

			if (none)
			{
				plus_infinity = false;

				if (!minus_infinity)
				{
					// we had no time granularity,
					// but now we've been "snipped finite" from both directions
					none = false;
				}
			}

			CheckForAndSetNoTimeRangeGranularity();

			if (validate)
			{
				Validate();
			}
		}


		// Less than checks
		bool IsStartLessThanRHSStart(TimeSlice const & rhs) const
		{
			if (none && minus_infinity)
			{
				if (rhs.none && rhs.minus_infinity)
				{
					return false;
				}

				return true;
			}
			else if (rhs.none && rhs.minus_infinity)
			{
				return false;
			}

			return time_start < rhs.time_start;
		}

		bool IsStartLessThanRHSEnd(TimeSlice const & rhs) const
		{
			if (none && minus_infinity)
			{
				return true;
			}
			else if (rhs.none && rhs.plus_infinity)
			{
				return true;
			}

			return time_start < rhs.time_end;
		}

		bool IsEndLessThanRHSStart(TimeSlice const & rhs) const
		{
			if (none && plus_infinity)
			{
				return false;
			}
			else if (rhs.none && rhs.minus_infinity)
			{
				return false;
			}

			return time_end < rhs.time_start;
		}

		bool IsEndLessThanRHSEnd(TimeSlice const & rhs) const
		{
			if (none && plus_infinity)
			{
				return false;
			}
			else if (rhs.none && rhs.plus_infinity)
			{
				return true;
			}

			return time_end < rhs.time_end;
		}


		// Less than or equal to checks
		bool IsStartLessThanOrEqualToRHSStart(TimeSlice const & rhs) const
		{
			return IsStartLessThanRHSStart(rhs) || IsStartEqualToRHSStart(rhs);
		}

		bool IsStartLessThanOrEqualToRHSEnd(TimeSlice const & rhs) const
		{
			return IsStartLessThanRHSEnd(rhs) || IsStartEqualToRHSEnd(rhs);
		}

		bool IsEndLessThanOrEqualToRHSStart(TimeSlice const & rhs) const
		{
			return IsEndLessThanRHSStart(rhs) || IsEndEqualToRHSStart(rhs);
		}

		bool IsEndLessThanOrEqualToRHSEnd(TimeSlice const & rhs) const
		{
			return IsEndLessThanRHSEnd(rhs) || IsEndEqualToRHSEnd(rhs);
		}


		// Equality checks
		bool IsStartEqualToRHSStart(TimeSlice const & rhs) const
		{
			return !IsStartLessThanRHSStart(rhs) && !rhs.IsStartLessThanRHSStart(*this);
		}

		bool IsStartEqualToRHSEnd(TimeSlice const & rhs) const
		{
			return !IsStartLessThanRHSEnd(rhs) && !rhs.IsEndLessThanRHSStart(*this);
		}

		bool IsEndEqualToRHSStart(TimeSlice const & rhs) const
		{
			return !IsEndLessThanRHSStart(rhs) && !rhs.IsStartLessThanRHSEnd(*this);
		}

		bool IsEndEqualToRHSEnd(TimeSlice const & rhs) const
		{
			return !IsEndLessThanRHSEnd(rhs) && !rhs.IsEndLessThanRHSEnd(*this);
		}



		// Greater than checks
		bool IsStartGreaterThanRHSStart(TimeSlice const & rhs) const
		{
			return rhs.IsStartLessThanRHSStart(*this);
		}

		bool IsStartGreaterThanRHSEnd(TimeSlice const & rhs) const
		{
			return rhs.IsEndLessThanRHSStart(*this);
		}

		bool IsEndGreaterThanRHSStart(TimeSlice const & rhs) const
		{
			return rhs.IsStartLessThanRHSEnd(*this);
		}

		bool IsEndGreaterThanRHSEnd(TimeSlice const & rhs) const
		{
			return rhs.IsEndLessThanRHSEnd(*this);
		}


		// Greater than or equal to checks
		bool IsStartGreaterThanOrEqualToRHSStart(TimeSlice const & rhs) const
		{
			return !IsStartLessThanRHSStart(rhs);
		}

		bool IsStartGreaterThanOrEqualToRHSEnd(TimeSlice const & rhs) const
		{
			return !IsStartLessThanRHSEnd(rhs);
		}

		bool IsEndGreaterThanOrEqualToRHSStart(TimeSlice const & rhs) const
		{
			return !IsEndLessThanRHSStart(rhs);
		}

		bool IsEndGreaterThanOrEqualToRHSEnd(TimeSlice const & rhs) const
		{
			return !IsEndLessThanRHSEnd(rhs);
		}


		std::int64_t getStart() const
		{
			if (none && minus_infinity)
			{
				boost::format msg("Attempting to get start time of time slice with no time range granularity!");
				throw NewGeneException() << newgene_error_description(msg.str());
			}

			return time_start;
		}

		std::int64_t getEnd() const
		{
			if (none && plus_infinity)
			{
				boost::format msg("Attempting to get start time of time slice with no time range granularity!");
				throw NewGeneException() << newgene_error_description(msg.str());
			}

			return time_end;
		}

	private:

		std::int64_t time_start;
		std::int64_t time_end;
		bool none;
		bool minus_infinity;
		bool plus_infinity;

};

class Weighting
{

	public:

		static int how_many_weightings;

		Weighting()
		{
			++how_many_weightings;
			InternalSetWeighting();
		}

		Weighting(Weighting const & rhs)
			: weighting { rhs.weighting }
		, weighting_range_start { rhs.weighting_range_start }
		, weighting_range_end { rhs.weighting_range_end }
		{
			++how_many_weightings;
			InternalSetWeighting();
		}

		~Weighting()
		{
			--how_many_weightings;
		}

		Weighting & operator=(Weighting const & rhs)
		{
			if (&rhs == this)
			{
				return *this;
			}

			weighting = rhs.weighting;
			weighting_range_start = rhs.weighting_range_start;
			InternalSetWeighting();
			return *this;
		}

		void setWeighting(newgene_cpp_int const & weighting_)
		{
			weighting = weighting_;
			weighting_range_end = weighting_range_start + weighting - 1;
			InternalSetWeighting();
		}

		void setWeightingRangeStart(newgene_cpp_int const & weighting_range_start_)
		{
			weighting_range_start = weighting_range_start_;
			weighting_range_end = weighting_range_start + weighting - 1;
			InternalSetWeighting();
		}

		void addWeighting(newgene_cpp_int const & weighting_to_add)
		{
			weighting += weighting_to_add;
			InternalSetWeighting();
		}

		std::string getWeightingString() const
		{
			return boost::lexical_cast<std::string>(weighting);
		}

		newgene_cpp_int const & getWeighting() const
		{
			return weighting;
		}

		newgene_cpp_int const & getWeightingRangeStart() const
		{
			return weighting_range_start;
		}

		newgene_cpp_int const & getWeightingRangeEnd() const
		{
			return weighting_range_end;
		}

	private:

		newgene_cpp_int weighting;
		newgene_cpp_int weighting_range_start;
		newgene_cpp_int weighting_range_end;
#	ifdef _DEBUG
		//std::string weighting_string;
		//std::string weighting_range_start_string;
		//std::string weighting_range_end_string;
#	endif

		void InternalSetWeighting()
		{
			weighting_range_end = weighting_range_start + weighting - 1;
#		ifdef _DEBUG
			//weighting_string = weighting.str();
			//weighting_range_start_string = weighting_range_start.str();
			//weighting_range_end_string = weighting_range_end.str();
#		endif
		}

};

template<typename TOPLEVEL_POOL_TAG, typename... Params>
typename TOPLEVEL_POOL_TAG::type * InstantiateUsingTopLevelObjectsPool(Params... args)
{
	void * ptr = TopLevelObjectsPool<TOPLEVEL_POOL_TAG>::malloc();
	auto typed_ptr = new(ptr)(typename TOPLEVEL_POOL_TAG::type)(args...);
	return typed_ptr;
}

template<typename TOPLEVEL_POOL_TAG>
void DeleteUsingTopLevelObjectsPool(typename TOPLEVEL_POOL_TAG::type * ptr)
{
	TopLevelObjectsPool<TOPLEVEL_POOL_TAG>::free(ptr);
}

enum CHILD_TO_PRIMARY_MAPPING
{
	CHILD_TO_PRIMARY_MAPPING__UNKNOWN
	, CHILD_TO_PRIMARY_MAPPING__MAPS_TO_BRANCH
	, CHILD_TO_PRIMARY_MAPPING__MAPS_TO_LEAF
};

struct ChildToPrimaryMapping
{
	ChildToPrimaryMapping(CHILD_TO_PRIMARY_MAPPING const mapping_, int const index_, int const leaf_number_ = -1)
		: mapping(mapping_), index_of_column_within_top_level_branch_or_single_leaf(index_),
		  leaf_number_in_top_level_group__only_applicable_when_child_key_column_points_to_top_level_column_that_is_in_top_level_leaf(leaf_number_) {}

	CHILD_TO_PRIMARY_MAPPING mapping;
	int index_of_column_within_top_level_branch_or_single_leaf;
	int leaf_number_in_top_level_group__only_applicable_when_child_key_column_points_to_top_level_column_that_is_in_top_level_leaf;

	static std::string MappingToText(CHILD_TO_PRIMARY_MAPPING const mapping)
	{
		std::string result;

		switch (mapping)
		{
			case CHILD_TO_PRIMARY_MAPPING__MAPS_TO_BRANCH:
				{ result = "CHILD_TO_PRIMARY_MAPPING__MAPS_TO_BRANCH"; }
				break;

			case CHILD_TO_PRIMARY_MAPPING__MAPS_TO_LEAF:
				{ result = "CHILD_TO_PRIMARY_MAPPING__MAPS_TO_LEAF"; }
				break;

			default:
				{ result = "CHILD_TO_PRIMARY_MAPPING__UNKNOWN"; }
				break;
		}

		return result;
	}
};

template <typename MEMORY_TAG>
using fast_vector_childtoprimarymapping = FastVectorMemoryTag<ChildToPrimaryMapping, MEMORY_TAG>;

template <typename MEMORY_TAG>
using fast_int_to_childtoprimarymappingvector = FastMapMemoryTag<int, fast_vector_childtoprimarymapping<MEMORY_TAG>, MEMORY_TAG>;

enum VARIABLE_GROUP_MERGE_MODE
{
	VARIABLE_GROUP_MERGE_MODE__UNKNOWN
	, VARIABLE_GROUP_MERGE_MODE__PRIMARY
	, VARIABLE_GROUP_MERGE_MODE__TOP_LEVEL
	, VARIABLE_GROUP_MERGE_MODE__CHILD
};

// ******************************************************************************************* //
// The following class is used as the base class for the "Leaf" and "Branch" classes, below
// (they aren't actually called "Leaf" and "Branch", but they are indicated as such in comments
//  and variable names throughout the code).
//
// This class is a wrapper over a vector of DMU instance data
// (i.e., for Country, we might have {2, 200, 300} corresponding to an instance of this class).
// ******************************************************************************************* //
class PrimaryKeysGrouping
{

	public:

		PrimaryKeysGrouping(DMUInstanceDataVector<hits_tag> const & dmuInstanceDataVector)
			: primary_keys(dmuInstanceDataVector)
		{}

		PrimaryKeysGrouping(PrimaryKeysGrouping const & rhs)
			: primary_keys(rhs.primary_keys)
		{}

		PrimaryKeysGrouping & operator=(PrimaryKeysGrouping const & rhs)
		{
			if (&rhs == this)
			{
				return *this;
			}

			primary_keys = rhs.primary_keys;
			return *this;
		}

		PrimaryKeysGrouping & operator=(PrimaryKeysGrouping const && rhs)
		{
			if (&rhs == this)
			{
				return *this;
			}

			primary_keys = std::move(rhs.primary_keys);
			return *this;
		}

		DMUInstanceDataVector<hits_tag> primary_keys;

		bool operator<(PrimaryKeysGrouping const & rhs) const
		{

			if (primary_keys.size() != rhs.primary_keys.size())
			{
				boost::format msg("Number of DMU's is different in PrimaryKeysGrouping::operator<()!");
				throw NewGeneException() << newgene_error_description(msg.str());
			}

			size_t nDmus = primary_keys.size();

			for (size_t n = 0; n < nDmus; ++n)
			{
				bool is_current_less_than = boost::apply_visitor(less_than_visitor(), primary_keys[n], rhs.primary_keys[n]);

				if (is_current_less_than)
				{
					return true;
				}
				else
				{
					bool is_current_greater_than = boost::apply_visitor(less_than_visitor(), rhs.primary_keys[n], primary_keys[n]);

					if (is_current_greater_than)
					{
						return false;
					}
					else
					{
						// so far, equal; continue
					}
				}
			}

			// equal in all DMU's, so return false
			return false;

		}

	protected:

		class less_than_visitor : public boost::static_visitor<bool>
		{

			public:

				template <typename T, typename U>
				bool operator()(const T &, const U &) const
				{
					boost::format msg("DMU's are of different types in PrimaryKeysGrouping::less_than_visitor()!");
					throw NewGeneException() << newgene_error_description(msg.str());
				}

				template <typename T>
				bool operator()(T const & lhs, T const & rhs) const
				{
					return lhs < rhs;
				}

		};

};

class size_of_visitor : public boost::static_visitor<size_t>
{

	public:

		template <typename T>
		size_t operator()(T const & rhs) const
		{
			return sizeof(rhs);
		}

};

// ******************************************************************************************************************************************************************** //
//
// "Leaf"
//
// See detailed comments inside the "Branch" class declaration, further down in this file.
//
// ******************************************************************************************************************************************************************** //
class PrimaryKeysGroupingMultiplicityGreaterThanOne : public PrimaryKeysGrouping
{

	public:

		PrimaryKeysGroupingMultiplicityGreaterThanOne()
			: PrimaryKeysGrouping { DMUInstanceDataVector<hits_tag>() }
			, index_into_raw_data{ 0 }
			, has_excluded_dmu_member{false}
		{
		}

		PrimaryKeysGroupingMultiplicityGreaterThanOne(DMUInstanceDataVector<hits_tag> const & dmuInstanceDataVector, std::int32_t const & index_into_raw_data_ = 0, bool const has_excluded_dmu_member_ = false)
			: PrimaryKeysGrouping(dmuInstanceDataVector)
			, index_into_raw_data { index_into_raw_data_ }
			, has_excluded_dmu_member{ has_excluded_dmu_member_ }
		{
		}

		PrimaryKeysGroupingMultiplicityGreaterThanOne(PrimaryKeysGroupingMultiplicityGreaterThanOne const & rhs)
			: PrimaryKeysGrouping(rhs)
			, index_into_raw_data ( rhs.index_into_raw_data )
			, other_top_level_indices_into_raw_data ( rhs.other_top_level_indices_into_raw_data )
			, has_excluded_dmu_member ( rhs.has_excluded_dmu_member )
		{
		}

		PrimaryKeysGroupingMultiplicityGreaterThanOne(PrimaryKeysGroupingMultiplicityGreaterThanOne const && rhs)
			: PrimaryKeysGrouping(std::move(rhs))
			, index_into_raw_data ( rhs.index_into_raw_data )
			, other_top_level_indices_into_raw_data ( std::move(rhs.other_top_level_indices_into_raw_data) )
			, has_excluded_dmu_member ( rhs.has_excluded_dmu_member )
		{
		}

		PrimaryKeysGroupingMultiplicityGreaterThanOne & operator=(PrimaryKeysGroupingMultiplicityGreaterThanOne const & rhs)
		{
			if (&rhs == this)
			{
				return *this;
			}

			PrimaryKeysGrouping::operator=(rhs);
			index_into_raw_data = rhs.index_into_raw_data;
			other_top_level_indices_into_raw_data = rhs.other_top_level_indices_into_raw_data;
			has_excluded_dmu_member = rhs.has_excluded_dmu_member;
			return *this;
		}

		PrimaryKeysGroupingMultiplicityGreaterThanOne & operator=(PrimaryKeysGroupingMultiplicityGreaterThanOne const && rhs)
		{
			if (&rhs == this)
			{
				return *this;
			}

			PrimaryKeysGrouping::operator=(std::move(rhs));
			index_into_raw_data = rhs.index_into_raw_data;
			other_top_level_indices_into_raw_data = std::move(rhs.other_top_level_indices_into_raw_data);
			has_excluded_dmu_member = rhs.has_excluded_dmu_member;
			return *this;
		}

		~PrimaryKeysGroupingMultiplicityGreaterThanOne()
		{
		}

		std::int32_t index_into_raw_data; // For the primary top-level variable group - the index of this leaf into the secondary data cache

		// The variable group index for this map will always skip the index of the primary top-level variable group - that value is stored in the above variable.
		mutable fast_short_to_int_map<hits_tag>
		other_top_level_indices_into_raw_data; // For the non-primary top-level variable groups - the index of this leaf into the secondary data cache (mapped by variable group index)

		bool has_excluded_dmu_member;

};

template <typename MEMORY_TAG>
class BranchOutputRow
{

	public:

		BranchOutputRow() {}

		template <typename MEMORY_TAG_RHS>
		BranchOutputRow(BranchOutputRow<MEMORY_TAG_RHS> const & rhs)
			: primary_leaves(rhs.primary_leaves.cbegin(), rhs.primary_leaves.cend())
		{
			for (auto & rhs_map : rhs.child_indices_into_raw_data)
			{
				child_indices_into_raw_data[rhs_map.first].insert(rhs_map.second.cbegin(), rhs_map.second.cend());
			}
			SaveCache();
		}

		//template <typename MEMORY_TAG_RHS>
		//BranchOutputRow(BranchOutputRow<MEMORY_TAG_RHS> && rhs)

		//	// No need to use a move iterator - these are just ints, and the move syntax is tedious because of the need for const_cast and move_iterator, resulting in no benefit because these are just int's
		//	//: primary_leaves(std::move_iterator(rhs.primary_leaves.begin()), std::move_iterator(rhs.primary_leaves.end()))
		//	: primary_leaves(rhs.primary_leaves.cbegin(), rhs.primary_leaves.cend())

		//	// ditto
		//	//, primary_leaves_cache(std::move_iterator(rhs.primary_leaves_cache.begin()), std::move_iterator(rhs.primary_leaves_cache.end()))
		//	, primary_leaves_cache(rhs.primary_leaves_cache.cbegin(), rhs.primary_leaves_cache.cend())
		//{
		//	for (auto & rhs_map : rhs.child_indices_into_raw_data)
		//	{
		//		for (auto & rhs_vec : rhs_map.second)
		//		{
		//			// move does nothing here, since it's an int, but if we could, we would...
		//			// this is the best that can be done for disparate types
		//			child_indices_into_raw_data[rhs_map.first][rhs_vec.first] = std::move(rhs_vec.second);
		//		}
		//	}
		//	//SaveCache(); // already moved from rhs
		//}

		// specialize for own type
		BranchOutputRow(BranchOutputRow<MEMORY_TAG> && rhs)

			// No need to use a move iterator - these are just ints, and the move syntax is tedious because of the need for const_cast and move_iterator, resulting in no benefit because these are just int's
			//: primary_leaves(std::move_iterator(rhs.primary_leaves.begin()), std::move_iterator(rhs.primary_leaves.end()))
			: primary_leaves(rhs.primary_leaves.cbegin(), rhs.primary_leaves.cend())

			// ditto
			//, primary_leaves_cache(std::move_iterator(rhs.primary_leaves_cache.begin()), std::move_iterator(rhs.primary_leaves_cache.end()))
			, primary_leaves_cache(rhs.primary_leaves_cache.cbegin(), rhs.primary_leaves_cache.cend())
		{
			child_indices_into_raw_data = std::move(rhs.child_indices_into_raw_data);
			//SaveCache(); // already moved from rhs
		}

		template <typename MEMORY_TAG_RHS>
		BranchOutputRow & operator=(BranchOutputRow<MEMORY_TAG_RHS> const & rhs)
		{
			if (this == &rhs)
			{
				return *this;
			}

			primary_leaves.clear();
			primary_leaves.insert(rhs.primary_leaves.cbegin(), rhs.primary_leaves.cend());
			child_indices_into_raw_data.clear();
			std::copy(rhs.child_indices_into_raw_data.cbegin(), rhs.child_indices_into_raw_data.cend(), child_indices_into_raw_data.begin());
			SaveCache();
			return *this;
		}

		template <typename MEMORY_TAG_RHS>
		BranchOutputRow & operator=(BranchOutputRow<MEMORY_TAG_RHS> && rhs)
		{
			if (this == &rhs)
			{
				return *this;
			}

			primary_leaves.clear();
			
			// Set of int's: no need to do a move, which requires tedious const_cast and move_iterator syntax and won't result in any benefit because they're just integers being moved
			primary_leaves.insert(rhs.primary_leaves.cbegin(), rhs.primary_leaves.cend());

			child_indices_into_raw_data.clear();
			for (auto & rhs_map : rhs.child_indices_into_raw_data)
			{
				for (auto & rhs_vec : rhs_map.second)
				{
					// move does nothing here, since it's an int, but if we could, we would...
					// this is the best that can be done for disparate types
					child_indices_into_raw_data[rhs_map.first][rhs_vec.first] = std::move(rhs_vec.second);
				}
			}
			SaveCache();
			return *this;
		}

		// Specialize for own type
		BranchOutputRow & operator=(BranchOutputRow<MEMORY_TAG> && rhs)
		{
			if (this == &rhs)
			{
				return *this;
			}

			primary_leaves.clear();

			// Set of int's: no need to do a move, which requires tedious const_cast and move_iterator syntax and won't result in any benefit because they're just integers being moved
			primary_leaves.insert(rhs.primary_leaves.cbegin(), rhs.primary_leaves.cend());

			child_indices_into_raw_data = std::move(rhs.child_indices_into_raw_data);

			SaveCache();
			return *this;
		}

		// Destructor to debug
		~BranchOutputRow() {}

		template <typename MEMORY_TAG_RHS>
		bool operator==(BranchOutputRow<MEMORY_TAG_RHS> const & rhs) const
		{
			return (!(*this < rhs) && !(rhs < *this));
		}

		template <typename MEMORY_TAG_RHS>
		bool operator!=(BranchOutputRow<MEMORY_TAG_RHS> const & rhs) const
		{
			return (!(*this == rhs));
		}

		template <typename MEMORY_TAG_RHS>
		bool operator<(BranchOutputRow<MEMORY_TAG_RHS> const & rhs) const
		{
			return primary_leaves < rhs.primary_leaves;
		}

		void Insert(int const index_of_leaf)
		{
			primary_leaves.insert(index_of_leaf);
			SaveCache();
		}

		size_t Size() const
		{
			return primary_leaves.size();
		}

		bool Empty() const
		{
			return primary_leaves.empty();
		}

		void Clear()
		{
			primary_leaves.clear();
			SaveCache();
		}

#	ifndef _DEBUG
		//	private: // for debugging convenience, make public; but be sure it builds when private
#	endif

		// ******************************************************************* //
		// Index into the branch's Leaf set.
		// This *uniquely* defines the row.
		// Any two BranchOutputRow's in the same branch
		// with the same set of primary leaves
		// ... are guaranteed to have identical
		// secondary data for all variable groups.
		// This includes the primary top-level variable group,
		// the non-primary top-level variable groups,
		// and all child variable groups.
		// ******************************************************************* //
		fast_int_set<MEMORY_TAG> primary_leaves;

	public:

		fast_int_vector<MEMORY_TAG> primary_leaves_cache; // for optimized lookup only

		// Map from child variable group ID to:
		// Map from child leaf index to:
		// index into child variable group's raw data cache (stored in the AllWeightings instance)
		mutable fast__short__to__fast_short_to_int_map__loaded<MEMORY_TAG> child_indices_into_raw_data;

	private:

		void SaveCache()
		{
			primary_leaves_cache.clear();
			primary_leaves_cache.insert(primary_leaves_cache.begin(), primary_leaves.begin(), primary_leaves.end());
		}

};

typedef PrimaryKeysGroupingMultiplicityGreaterThanOne Leaf;

template <typename MEMORY_TAG>
using fast_leaf_vector = FastVectorMemoryTag<Leaf, MEMORY_TAG>;

template <typename MEMORY_TAG>
using Leaves = FastSetMemoryTag<Leaf, MEMORY_TAG>;

// ************************************************************************************************************************************************************************* //
// Special-case!
// The following is ONLY used by Branch::remaining to optimize random selection of rows when the number of random numbers approaches the total number of branch combinations.
// And in some scenarios, the use of Boost Pool is horrendous.
// So, in this case, use the standard allocator.
// Beware if the following data type is ever used for anything else!!!
// ************************************************************************************************************************************************************************* //
//
// No - actually, we will use the Boost Pool here...
//
template <typename MEMORY_TAG>
using fast_branch_output_row_vector_____currently_only_used_for_Branch_remaining = FastVectorMemoryTag<BranchOutputRow<MEMORY_TAG>, MEMORY_TAG>;
//template <typename MEMORY_TAG>
//using fast_branch_output_row_vector_____currently_only_used_for_Branch_remaining = std::vector<BranchOutputRow<MEMORY_TAG>>;

template <typename MEMORY_TAG>
using fast_branch_output_row_list_____currently_only_used_for_Branch_remaining = FastListMemoryTag<BranchOutputRow<MEMORY_TAG>, MEMORY_TAG>;

template <typename MEMORY_TAG>
using fast_branch_output_row_vector_huge = FastVectorMemoryTag<BranchOutputRow<MEMORY_TAG>, MEMORY_TAG>;

template <typename MEMORY_TAG>
using fast_branch_output_row_set = FastSetMemoryTag<BranchOutputRow<MEMORY_TAG>, MEMORY_TAG>;

// ************************************************************************************************************************************************************************* //
// ************************************************************************************************************************************************************************* //
// Warning and notice!
// "BranchOutputRow<child_dmu_lookup_tag> const *" is used as a key value in the data structure, below,
// rather than MEMORY_TAG.
// The reason is that this is a POINTER to such a location in memory.
// The POINTER itself is part of a map node which WILL be stored in the memory pool indicated by MEMORY_TAG.
// ************************************************************************************************************************************************************************* //
// ************************************************************************************************************************************************************************* //
template <typename MEMORY_TAG>
using fast_branch_output_row_ptr__to__fast_short_vector = FastMapMemoryTag<BranchOutputRow<hits_tag> const *, fast_short_vector<MEMORY_TAG>, MEMORY_TAG>;

template <typename MEMORY_TAG>
using fast__int64__to__fast_branch_output_row_set = FastMapMemoryTag<std::int64_t, fast_branch_output_row_set<MEMORY_TAG>, MEMORY_TAG>;

template <typename MEMORY_TAG>
using fast__int64__to__fast_branch_output_row_vector = FastMapMemoryTag<std::int64_t, fast_branch_output_row_vector_____currently_only_used_for_Branch_remaining<MEMORY_TAG>, MEMORY_TAG>;

template <typename MEMORY_TAG>
using fast__int64__to__fast_branch_output_row_list = FastMapMemoryTag<std::int64_t, fast_branch_output_row_list_____currently_only_used_for_Branch_remaining<MEMORY_TAG>, MEMORY_TAG>;

template <typename MEMORY_TAG>
using fast__lookup__from_child_dmu_set__to__output_rows = FastMapMemoryTag<ChildDMUInstanceDataVector<MEMORY_TAG>, fast_branch_output_row_ptr__to__fast_short_vector<MEMORY_TAG>, MEMORY_TAG>;

// Supports using a separate, templatized memory pool "child_dmu_lookup_tag" to store the performance-intensive-on-delete
// "helper_lookup__from_child_key_set__to_matching_output_rows" data structure,
// while allowing binary_search on the ChildDMUInstanceDataVector<MEMORY_TAG> key of the map
// against a ChildDMUInstanceDataVector<MEMORY_TAG> from a different memory pool.
// The only way to support this is to provide an operator<() available for std::binary_search,
// which must accept the full std::map::value_type as arguments (i.e., a std::pair<K const, V>),
// even though all we are using to do the comparison is simply the key.
template <typename MEMORY_TAG_LHS, typename MEMORY_TAG_RHS>
bool operator<(std::pair<ChildDMUInstanceDataVector<MEMORY_TAG_LHS> const, fast_branch_output_row_ptr__to__fast_short_vector<MEMORY_TAG_LHS>> const & lhs, std::pair<ChildDMUInstanceDataVector<MEMORY_TAG_RHS> const, fast_branch_output_row_ptr__to__fast_short_vector<MEMORY_TAG_RHS>> const & rhs)
{
	size_t lhs_size = lhs.first.size();
	size_t rhs_size = rhs.first.size();
	size_t min_size = std::min(lhs_size, rhs_size);
	for (size_t n = 0; n < min_size; ++n)
	{
		if (lhs.first[n] < rhs.first[n])
		{
			return true;
		}
		else
		if (rhs.first[n] < lhs.first[n])
		{
			return false;
		}
	}
	if (lhs_size < rhs_size)
	{
		return true;
	}
	return false;
}

// We also need operator<() not just to compare two MAP elements in the existing helper_lookup__from_child_key_set__to_matching_output_rows map,
// but ALSO to compare a test ChildDMUInstanceDataVector<MEMORY_TAG> on the left or right against such a map element for use in std::binary_search.
// See comments above.
template <typename MEMORY_TAG_LHS, typename MEMORY_TAG_RHS>
bool operator<(std::pair<ChildDMUInstanceDataVector<MEMORY_TAG_LHS> const, fast_branch_output_row_ptr__to__fast_short_vector<MEMORY_TAG_LHS>> const & lhs, ChildDMUInstanceDataVector<MEMORY_TAG_RHS> const & rhs)
{
	size_t lhs_size = lhs.first.size();
	size_t rhs_size = rhs.size();
	size_t min_size = std::min(lhs_size, rhs_size);
	for (size_t n = 0; n < min_size; ++n)
	{
		if (lhs.first[n] < rhs[n])
		{
			return true;
		}
		else
		if (rhs[n] < lhs.first[n])
		{
			return false;
		}
	}
	if (lhs_size < rhs_size)
	{
		return true;
	}
	return false;
}

// We also need operator<() not just to compare two MAP elements in the existing helper_lookup__from_child_key_set__to_matching_output_rows map,
// but ALSO to compare a test ChildDMUInstanceDataVector<MEMORY_TAG> on the left or right against such a map element for use in std::binary_search.
// See comments above.
template <typename MEMORY_TAG_LHS, typename MEMORY_TAG_RHS>
bool operator<(ChildDMUInstanceDataVector<MEMORY_TAG_LHS> const & lhs, std::pair<ChildDMUInstanceDataVector<MEMORY_TAG_RHS> const, fast_branch_output_row_ptr__to__fast_short_vector<MEMORY_TAG_RHS>> const & rhs)
{
	size_t lhs_size = lhs.size();
	size_t rhs_size = rhs.first.size();
	size_t min_size = std::min(lhs_size, rhs_size);
	for (size_t n = 0; n < min_size; ++n)
	{
		if (lhs[n] < rhs.first[n])
		{
			return true;
		}
		else
		if (rhs.first[n] < lhs[n])
		{
			return false;
		}
	}
	if (lhs_size < rhs_size)
	{
		return true;
	}
	return false;
}

template<typename MEMORY_TAG>
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
	std::for_each(leaf.other_top_level_indices_into_raw_data.cbegin(),
		leaf.other_top_level_indices_into_raw_data.cend(), [&](fast_short_to_int_map<MEMORY_TAG>::value_type const & leafindicesintorawdata)
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

template<typename T, typename MEMORY_TAG>
void SpitSetOfOutputRows(std::string & sdata, T const & setOfRows)
{
	sdata += "<SET_OF_ROWS>";

	sdata += "<SET_OF_ROWS_MAP_ITSELF>";
	sdata += boost::lexical_cast<std::string>(sizeof(setOfRows));
	sdata += "</SET_OF_ROWS_MAP_ITSELF>";

	int index = 0;
	std::for_each(setOfRows.cbegin(), setOfRows.cend(), [&](BranchOutputRow<MEMORY_TAG> const & row)
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

template<typename T, typename MEMORY_TAG>
void SpitHit(std::string & sdata, std::int64_t const time_unit, T const & hit)
{

	sdata += "<TIME_UNIT>";

	sdata += "<TIME_UNIT_MAP_ITSELF>";
	sdata += boost::lexical_cast<std::string>(sizeof(std::pair<std::int64_t const, MEMORY_TAG>));
	sdata += "</TIME_UNIT_MAP_ITSELF>";

	sdata += "<TIME_UNIT_INDEX>";
	sdata += boost::lexical_cast<std::string>(time_unit);
	sdata += "</TIME_UNIT_INDEX>";
	sdata += "<OUTPUT_ROWS>";
	SpitSetOfOutputRows<decltype(hit), MEMORY_TAG>(sdata, hit);
	sdata += "</OUTPUT_ROWS>";

	sdata += "</TIME_UNIT>";

}

// ******************************************************************************************************************************************************************** //
// ******************************************************************************************************************************************************************** //
// ******************************************************************************************************************************************************************** //
// ******************************************************************************************************************************************************************** //
// ******************************************************************************************************************************************************************** //
//
// Output data to XML for debugging.
//
// It is highly recommended to call this function willy-nilly for debugging purposes,
// in the code while the output is being progressively generated.
// It will shed a spotlight on how the data is stored!
// 
// But warning: The XML file so generated becomes very large very quickly, so only
// call the function when debugging with small data sets.
//
// Please use an XML visualization program, such as XMLSpy, to view this file easily.
//
// ******************************************************************************************************************************************************************** //
// ******************************************************************************************************************************************************************** //
// ******************************************************************************************************************************************************************** //
// ******************************************************************************************************************************************************************** //
// ******************************************************************************************************************************************************************** //
//
void SpitAllWeightings(KadSampler const & allWeightings, std::string const & file_name_appending_string);
//
// ******************************************************************************************************************************************************************** //
// ******************************************************************************************************************************************************************** //
// ******************************************************************************************************************************************************************** //


// ******************************************************************************************************************************************************************** //
// SpitAllWeightings() is the important one... just use that for debugging.
// Internally, it calls the functions below.
// ******************************************************************************************************************************************************************** //

template <typename MEMORY_TAG>
void SpitKeys(std::string & sdata, DMUInstanceDataVector<MEMORY_TAG> const & dmu_keys)
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

template <typename MEMORY_TAG>
void SpitDataCache(std::string & sdata, DataCache<MEMORY_TAG> const & dataCache)
{
	sdata += "<DATA_CACHE>";
	std::for_each(dataCache.cbegin(), dataCache.cend(), [&](DataCache<hits_tag>::value_type const & dataEntry)
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

template <typename MEMORY_TAG>
void SpitDataCaches(std::string & sdata, fast_short_to_data_cache_map<MEMORY_TAG> const & dataCaches)
{
	sdata += "<DATA_CACHES>";
	std::for_each(dataCaches.cbegin(), dataCaches.cend(), [&](fast_short_to_data_cache_map<MEMORY_TAG>::value_type const & dataEntry)
	{
		sdata += "<DATA_CACHE_NUMBER>";
		sdata += boost::lexical_cast<std::string>(dataEntry.first);
		sdata += "</DATA_CACHE_NUMBER>";
		SpitDataCache(sdata, dataEntry.second);
	});
	sdata += "</DATA_CACHES>";
}

template<typename MEMORY_TAG>
void SpitHits(std::string & sdata, fast__int64__to__fast_branch_output_row_set<MEMORY_TAG> const & hits)
{
	sdata += "<TIME_UNITS>";

	sdata += "<TIME_UNITS_MAP_ITSELF>";
	sdata += boost::lexical_cast<std::string>(sizeof(hits));
	sdata += "</TIME_UNITS_MAP_ITSELF>";

	std::for_each(hits.cbegin(), hits.cend(), [&](fast__int64__to__fast_branch_output_row_set<MEMORY_TAG>::value_type const & hitsEntry)
	{
		SpitHit<decltype(hitsEntry.second), MEMORY_TAG>(sdata, hitsEntry.first, hitsEntry.second);
	});

	sdata += "</TIME_UNITS>";
}

template <typename MEMORY_TAG>
void SpitOutputRow(std::string & sdata, BranchOutputRow<MEMORY_TAG> const & row)
{
	sdata += "<ROW>";

	sdata += "<ROW_OBJECT_ITSELF>";
	sdata += boost::lexical_cast<std::string>(sizeof(row));
	sdata += "</ROW_OBJECT_ITSELF>";

	sdata += "<INDICES_FOR_THIS_SINGLE_ROW_POINTING_INTO_LEAF_SET_FOR_THIS_BRANCH>";
	std::for_each(row.primary_leaves.cbegin(), row.primary_leaves.cend(), [&](int const & leafindex)
	{
		sdata += "<INDEX_POINTING_INTO_LEAF_SET>";
		sdata += boost::lexical_cast<std::string>(leafindex);
		sdata += "</INDEX_POINTING_INTO_LEAF_SET>";
	});
	sdata += "</INDICES_FOR_THIS_SINGLE_ROW_POINTING_INTO_LEAF_SET_FOR_THIS_BRANCH>";

	sdata += "<CHILD_SECONDARY_DATA_CORRESPONDING_TO_THIS_OUTPUT_ROW>";
	sdata += "<CHILD_SECONDARY_DATA_CORRESPONDING_TO_THIS_OUTPUT_ROW_MAP_ITSELF>";
	sdata += boost::lexical_cast<std::string>(sizeof(row.child_indices_into_raw_data));
	sdata += "</CHILD_SECONDARY_DATA_CORRESPONDING_TO_THIS_OUTPUT_ROW_MAP_ITSELF>";
	std::for_each(row.child_indices_into_raw_data.cbegin(), row.child_indices_into_raw_data.cend(), [&](fast__short__to__fast_short_to_int_map__loaded<MEMORY_TAG>::value_type const & childindices)
	{
		sdata += "<SPECIFIC_VARIABLE_GROUP_CHILD_SECONDARY_DATA>";
		sdata += "<SPECIFIC_VARIABLE_GROUP_CHILD_SECONDARY_DATA_OBJECT_ITSELF>";
		sdata += boost::lexical_cast<std::string>(sizeof(childindices));
		sdata += "</SPECIFIC_VARIABLE_GROUP_CHILD_SECONDARY_DATA_OBJECT_ITSELF>";
		sdata += "<VARIABLE_GROUP_NUMBER>";
		sdata += boost::lexical_cast<std::string>(childindices.first);
		sdata += "</VARIABLE_GROUP_NUMBER>";
		std::for_each(childindices.second.cbegin(), childindices.second.cend(), [&](fast_short_to_int_map<MEMORY_TAG>::value_type const & childleaves)
		{
			sdata += "<SINGLE_LEAF_OF_CHILD_DATA_FOR_THIS_OUTPUT_ROW>";
			sdata += "<SINGLE_LEAF_OF_CHILD_DATA_FOR_THIS_OUTPUT_ROW_OBJECT_ITSELF>";
			sdata += boost::lexical_cast<std::string>(sizeof(childleaves));
			sdata += "</SINGLE_LEAF_OF_CHILD_DATA_FOR_THIS_OUTPUT_ROW_OBJECT_ITSELF>";
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

template <typename MEMORY_TAG>
void SpitChildLookup(std::string & sdata, fast__lookup__from_child_dmu_set__to__output_rows<MEMORY_TAG> const & helperLookup)
{
	sdata += "<LIST_OF_CHILD_KEYLISTS_THAT_MATCH_SOMETHING_IN_THIS_BRANCH>";
	std::for_each(helperLookup.cbegin(), helperLookup.cend(), [&](fast__lookup__from_child_dmu_set__to__output_rows<MEMORY_TAG>::value_type const & helper)
	{
		sdata += "<CHILD_KEYLIST_THAT_MATCHES_SOMETHING_IN_THIS_BRANCH>";

		sdata += "<CHILD_DMU_KEYS>";
		SpitKeys(sdata, helper.first);
		sdata += "</CHILD_DMU_KEYS>";

		sdata += "<ROWS_CONTAINING_DATA_THAT_INCLUDES_THE_GIVEN_CHILD_KEYS>";
		std::for_each(helper.second.cbegin(), helper.second.cend(), [&](fast_branch_output_row_ptr__to__fast_short_vector<MEMORY_TAG>::value_type const & helperrow)
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

void SpitWeighting(std::string & sdata, Weighting const & weighting);
void SpitTimeSlice(std::string & sdata, TimeSlice const & time_slice);
void SpitChildToPrimaryKeyColumnMapping(std::string & sdata, ChildToPrimaryMapping const & childToPrimaryMapping);

template <typename MEMORY_TAG>
struct tag__fast__int64__to__fast_branch_output_row_vector
{
	typedef fast__int64__to__fast_branch_output_row_vector<MEMORY_TAG> type;
};

template <typename MEMORY_TAG>
struct tag__fast__int64__to__fast_branch_output_row_list
{
	typedef fast__int64__to__fast_branch_output_row_list<MEMORY_TAG> type;
};

template <typename MEMORY_TAG>
struct tag__fast__lookup__from_child_dmu_set__to__output_rows
{
	typedef fast__lookup__from_child_dmu_set__to__output_rows<MEMORY_TAG> type;
};

// ******************************************************************************************************************************************************************** //
//
// "Branch"
//
// See detailed comments inside this class declaration.
//
// Note: The branch represented by this class corresponds to 
// a single time slice (but the branch itself has no information
// about its corresponding time slice)
//
// ******************************************************************************************************************************************************************** //
class PrimaryKeysGroupingMultiplicityOne : public PrimaryKeysGrouping
{

	public:

		PrimaryKeysGroupingMultiplicityOne()
			: PrimaryKeysGrouping{ DMUInstanceDataVector<hits_tag>() }
			, remaining_(InstantiateUsingTopLevelObjectsPool<tag__fast__int64__to__fast_branch_output_row_list<remaining_tag>>())
			, remaining(*remaining_)
			, helper_lookup__from_child_key_set__to_matching_output_rows(nullptr)
			, has_excluded_leaves(false)
		{}

		PrimaryKeysGroupingMultiplicityOne(DMUInstanceDataVector<hits_tag> const & dmuInstanceDataVector)
			: PrimaryKeysGrouping(dmuInstanceDataVector)
			, remaining_(InstantiateUsingTopLevelObjectsPool<tag__fast__int64__to__fast_branch_output_row_list<remaining_tag>>())
			, remaining(*remaining_)
			, helper_lookup__from_child_key_set__to_matching_output_rows(nullptr)
			, has_excluded_leaves(false)
		{
		}

		PrimaryKeysGroupingMultiplicityOne(PrimaryKeysGroupingMultiplicityOne const & rhs)
			: PrimaryKeysGrouping(rhs)
			, weighting ( rhs.weighting )
			, hits ( rhs.hits )
			//, remaining { rhs.remaining } // NO! Do not copy!  Branches are NEVER copied while "remaining" is in use, and the memory is guaranteed to have been invalidated by the memory pool manager at any point a branch is copied
			, remaining_(InstantiateUsingTopLevelObjectsPool<tag__fast__int64__to__fast_branch_output_row_list<remaining_tag>>())
			, remaining(*remaining_)
			, number_branch_combinations ( rhs.number_branch_combinations )
			, leaves ( rhs.leaves )
			, leaves_cache ( rhs.leaves_cache )
			, helper_lookup__from_child_key_set__to_matching_output_rows(nullptr)
			, has_excluded_leaves(rhs.has_excluded_leaves)
		{
		}

		PrimaryKeysGroupingMultiplicityOne(PrimaryKeysGroupingMultiplicityOne && rhs)
			: PrimaryKeysGrouping(rhs)
			, weighting ( rhs.weighting )
			, hits ( rhs.hits )
			//, remaining { rhs.remaining } // NO! Do not copy!  Branches are NEVER copied while "remaining" is in use, and the memory is guaranteed to have been invalidated by the memory pool manager at any point a branch is copied
			, remaining_ (InstantiateUsingTopLevelObjectsPool<tag__fast__int64__to__fast_branch_output_row_list<remaining_tag>>())
			, remaining (*remaining_)
			, number_branch_combinations ( rhs.number_branch_combinations )
			, leaves ( rhs.leaves )
			, leaves_cache ( rhs.leaves_cache )
			, helper_lookup__from_child_key_set__to_matching_output_rows(nullptr)
			, has_excluded_leaves(rhs.has_excluded_leaves)
		{
			// Confirm this is never called
			boost::format msg("Branch rvalue constructor called!");
			throw NewGeneException() << newgene_error_description(msg.str());
		}

		PrimaryKeysGroupingMultiplicityOne & operator=(PrimaryKeysGroupingMultiplicityOne const & rhs)
		{
			if (&rhs == this)
			{
				return *this;
			}

			PrimaryKeysGrouping::operator=(rhs);
			weighting = rhs.weighting;
			hits = rhs.hits;
			//remaining = rhs.remaining; // NO! Do not copy!  Branches are NEVER copied while "remaining" is in use, and the memory is guaranteed to have been invalidated by the memory pool manager at any point a branch is copied
			number_branch_combinations = rhs.number_branch_combinations;
			leaves = rhs.leaves;
			leaves_cache = rhs.leaves_cache;
			hits_consolidated = rhs.hits_consolidated;
			helper_lookup__from_child_key_set__to_matching_output_rows = nullptr;
			has_excluded_leaves = rhs.has_excluded_leaves;
			return *this;
		}

		PrimaryKeysGroupingMultiplicityOne & operator=(PrimaryKeysGroupingMultiplicityOne const && rhs)
		{

			// Confirm this is never called
			boost::format msg("Branch rvalue assignment operator called!");
			throw NewGeneException() << newgene_error_description(msg.str());

			if (false)
			{
				if (&rhs == this)
				{
					return *this;
				}

				PrimaryKeysGrouping::operator=(std::move(rhs));
				weighting = rhs.weighting;
				hits = std::move(rhs.hits);
				//remaining = std::move(rhs.remaining); // NO! Do not copy!  Branches are NEVER copied while "remaining" is in use, and the memory is guaranteed to have been invalidated by the memory pool manager at any point a branch is copied
				number_branch_combinations = std::move(rhs.number_branch_combinations);
				leaves = std::move(rhs.leaves);
				leaves_cache = std::move(rhs.leaves_cache);
				hits_consolidated = std::move(rhs.hits_consolidated);
				helper_lookup__from_child_key_set__to_matching_output_rows = nullptr;
				has_excluded_leaves = rhs.has_excluded_leaves;
			}

			return *this;
		}

		void SpitLeaves(std::string & sdata) const
		{
			int index = 0;
			std::for_each(leaves_cache.cbegin(), leaves_cache.cend(), [&](Leaf const & leaf)
			{
				sdata += "<LEAF>";
				sdata += "<LEAF_NUMBER>";
				sdata += boost::lexical_cast<std::string>(index);
				sdata += "</LEAF_NUMBER>";
				sdata += "<LEAF_DATA>";
				SpitLeaf<hits_tag>(sdata, leaf);
				sdata += "</LEAF_DATA>";
				++index;
				sdata += "</LEAF>";
			});
		}

		// The following must be MUTABLE
		// because the BRANCH is used as the KEY for various maps...
		// But the mutable data is not part of the operator<(), so it is safe

		// Weighting for this branch: This is the lowest-level, calculated value, with unit granularity according to the primary variable group.
		// It is the product of the number of branch combinations and the number of time units in this time slice.
		mutable Weighting weighting;
		mutable bool has_excluded_leaves;


		// ******************************************************************************************************** //
		// ******************************************************************************************************** //
		// The following is the official location of the randomly generated rows
		// ******************************************************************************************************** //
		// Leaf combinations hit by the random generator.
		//
		// Map from time unit to a set of leaf combinations hit for that time units
		// Time unit index is 0-based
		//
		mutable fast__int64__to__fast_branch_output_row_set<hits_tag> hits;
		mutable fast_branch_output_row_vector_huge<hits_consolidated_tag> hits_consolidated;
		//
		// ******************************************************************************************************** //
		// ******************************************************************************************************** //


		// Used for optimization purposes only during random sampling construction of output rows
		mutable fast__int64__to__fast_branch_output_row_list<remaining_tag> * remaining_; // Let the Boost pool manage this
		mutable fast__int64__to__fast_branch_output_row_list<remaining_tag> & remaining;

		// **************************************************************************************** //
		// **************************************************************************************** //
		// Indices into cached secondary data tables for child groups.
		//
		// Overview:
		//
		// There is a single top-level *primary* variable group,
		// with 0, 1, or more top-level *non-primary* variable groups.
		// (A "top-level" group is one whose UOA is a superset of all non-top-level VG's,
		//  and equal to all other top-level VG's.  By "superset" is meant the DMU
		//  columns - not the time range granularity or secondary data columns.)
		//
		// Each branch corresponds to a single combination of specific primary key data values
		// ... for those primary keys of multiplicity 1
		// ... for the UOA corresponding to the (single) primary top-level variable group.
		// Each leaf corresponds to a single combination of specific primary key data values
		// ... for those primary keys of multiplicity greater than 1
		// ... for the UOA corresponding to the primary top-level variable group.
		// ... For the multiplicity = 1 case (i.e., K=1)), there will be a single output row
		// ... for each branch which is represented by a single EMPTY leaf,
		// ... because all DMU's exist in the branch.
		// ... (This empty leaf will be empty only in terms of its DMU's.
		// ...  It will still contain other leaf-related data that is independent of the number of
		// ...  DMU's in the leaf - specifically, an index into *secondary* data.
		// ...  This is equivalent to identifying a single arbitrary DMU as the DMU involved
		// ...  in the K-ad, but with K=1.)
		// Each row of output data has one branch, and multiple leaves (one leaf per multiplicity)
		//     (but see exception above for the K=1 case).
		// The set of leaves per row is stored across individual time unit entries within this branch
		// ... (where duplicates can occur in the branch, because if the same row (i.e., combination of leaves)
		// ...  appears in multiple time unit entries in the same time slice,
		// ...  that row will appear only once in the output for the time slice).
		// Each leaf represents a single set of secondary column data.
		// Example: UOA "MID, CTY, CTY", with K-ad "MID, CTY, CTY, CTY, CTY":
		// ... has branch DMU "MID", and leave DMU "CTY, CTY".
		// ... Each branch has a single value of a MID, such as "MID = 257" or "MID = 258".
		// ... Each leaf has a single value for the "CTY, CTY" pair corresponding to the UOA,
		// ... ... such as "CTY_1 = 2, CTY_2 = 20".
		// ... Each row (or "hit") contains the value from the current (single) branch,
		// ... ... and the values from two leaves (i.e., two pairs of countries, or 4 countries total).
		//
		// There can also be child variable groups.  These "children" include non-primary
		// top-level variable groups.
		//
		// Each child variable group's UOA is a subset of the UOA of the primary variable group.
		// The full set of primary keys for this child variable group's UOA includes a subset of
		// ... the primary keys for the primary variable group's branch, and a subset of
		// ... the primary keys for the primary variable group's leaves (including ALL the leaves).
		// The child variable group also has 0, 1, or more leaves.
		// ... Each child leaf corresponds to a separate set of secondary child variable group
		// ... column data that appears in every row of output.
		// Example: For the above UOA "MID, CTY, CTY" and K-ad "MID, CTY, CTY, CTY, CTY",
		// ... a child group could have UOA "MID, CTY".
		// For every row of output, this child group has four leaves, one for each country.
		// ... (Multiple *sets* of leaves per row, as opposed to *one* **set** of leaves per row,
		// ... is prohibited by data validation.  The latter would be a scenario such as
		// ... primary UOA = "MID, MID, CTY, CTY" and child UOA = "MID, CTY", a case in which
		// ... both MID and CTY have multiple sets of data for a single row -
		// ... this is an example of the prohibited case.)
		// Each such child variable group *leaf* represents a single set of the child's secondary column data,
		// as well as the value of the leaf DMU's.
		//
		// The actual raw data for the SECONDARY columns - the data values themselves -
		// ... are stored in the AllWeightings' "dataCache" (for primary top-level VG)
		// ... in the AllWeightings' "otherTopLevelCache" (for non-primary top-level VG's),
		// ... and in the AllWeightings' "childCache" (for true child VG's).

		// **************************************************************************************** //
		// **************************************************************************************** //
		// With these comments in mind:
		// The following data structure is a helper index that maps the *FULL* child DMU set
		// (including the single child leaf, if any)
		// representing an incoming row of data from a child variable group raw data table
		// (including only child primary keys and selected child variables in the incoming row)
		// - this corresponds to incoming child variable group (NOT non-primary top-level variable group) -
		// to output rows for that variable group.
		// The output rows are represented by a map from the actual pointer pointing to a specific output row,
		// to a vector of child leaf numbers in the output row matching the single incoming child leaf.
		// This is how NewGene quickly locates the rows that match incoming child variable groups,
		// so that it can set an index in the BranchOutputRow pointing to an entry in the Leaf cache
		// for the child variable group, which NewGene will use to retrieve the correct child data
		// when it writes the given row to the output.
		// **************************************************************************************** //
		// **************************************************************************************** //
		//
		// ****************************************************************************************************************** //
		// "helper_lookup__from_child_key_set__to_matching_output_rows" CAN NEVER HAVE MAP KEYS
		// THAT ARE BLANKS -
		// every child [branch + leaf] MUST map to real values
		// ****************************************************************************************************************** //
		mutable fast__lookup__from_child_dmu_set__to__output_rows<child_dmu_lookup_tag> * helper_lookup__from_child_key_set__to_matching_output_rows;

		// Cache for use with "consolidating rows" phase, if it is ever necessary!
		// Currently, this will never be filled, because we have no need to perform the lookup after rows have been consolidated,
		// and the profiler shows a major hit during the consolidating of rows in managing this cache, so disable it.
		mutable fast__lookup__from_child_dmu_set__to__output_rows<hits_consolidated_tag> helper_lookup__from_child_key_set__to_matching_output_rows_consolidating;

		void ConstructChildCombinationCache(KadSampler & allWeightings, int const variable_group_number, bool const force,
											bool const is_consolidating = false) const; // Populate the above data structure

		bool InsertLeaf(Leaf const & leaf) const
		{
			if (leaf.has_excluded_dmu_member)
			{
				this->has_excluded_leaves = true;
				return false;
			}
			leaves.insert(leaf);
			ResetLeafCache();
			return true;
		}

		void ClearLeaves()
		{
			leaves.clear();
			ResetLeafCache();
		}

		Leaf & getLeafAtIndex(int const & leaf_index)
		{
			return leaves_cache[leaf_index];
		}

		Leaf const & getLeafAtIndex(int const & leaf_index) const
		{
			return leaves_cache[leaf_index];
		}

		int numberLeaves() const
		{
			return static_cast<int>(leaves.size());
		}

		bool doesLeafExist(Leaf const & leaf) const
		{
			auto const leafPtr = leaves.find(leaf);

			if (leafPtr == leaves.cend())
			{
				return false;
			}

			return true;
		}

		void setTopGroupIndexIntoRawData(Leaf const & existingLeaf, int const variable_group_number, std::int32_t const other_top_level_index_into_raw_data) const
		{
			// The following does not need to be a reference - it was added during a paranoid debugging session, but it doesn't hurt
			auto const & leafPtr = leaves.find(existingLeaf);
			leafPtr->other_top_level_indices_into_raw_data[variable_group_number] = other_top_level_index_into_raw_data;
			ResetLeafCache();
		}

	private:
	public: // debugging access

		mutable Leaves<hits_tag> leaves;

	private:
	public: // debugging access

		// *********************************************************************************** //
		// Every branch ALREADY has a std::set<Leaf>,
		// so we're already one-to-one with each branch.
		// (To see this, track through the TimeSlices data structure,
		//  which breaks the data first into branches, then into leaves.
		//  Also, note that a set of leaves is a set of SPECIFIC DATA VALUES,
		//  so it must be stored branch-by-branch.
		//  Example: For UOA with DMU's "MID-CTY", and with K=2 on CTY:
		//      Branch #1 (MID = 257): Leaves are CTY=2, CTY=20, CTY=220, ...
		//      ... resulting in a set of many K-ads for this branch (each a single output row).
		//      Branch #2 (MID = 37): Leaves are CTY=2, CTY=20, CTY=21 (this is just made up data)
		//      ... resulting in a set of 3 K-ads for this branch (each a single output row).
		//  The example makes clear that each branch contains a unique set of leaves.
		// Therefore, we are "only" doubling the memory required
		// by using the following cache, which is stored within each branch
		// (in addition to having the leaves stored inside the "timeSlices" data member
		//  of the "AllWeightings" object, where the leaves are nonetheless broken down by branch.)
		// *********************************************************************************** //
		//
		// Cache the leaves for this branch.  This is a CACHE only.
		// The official location of the leaves are contained in the AllWeightings instance.
		// This cache stores only the subset of leaves
		mutable fast_leaf_vector<hits_tag> leaves_cache;

	public:

		void ValidateOutputRowLeafIndexes() const
		{
#			ifdef _DEBUG
			std::for_each(hits.cbegin(), hits.cend(), [&](decltype(hits)::value_type const & hitsEntry)
			{
				auto const & hits = hitsEntry.second;
				std::for_each(hits.cbegin(), hits.cend(), [&](BranchOutputRow<hits_tag> const & outputRow)
				{
					std::for_each(outputRow.primary_leaves.cbegin(), outputRow.primary_leaves.cend(), [&](int const & index_into_leaf_cache)
					{
						if (index_into_leaf_cache > static_cast<int>(leaves_cache.size()))
						{
							boost::format msg("Output rows in branch have invalid leaf cache index!");
							throw NewGeneException() << newgene_error_description(msg.str());
						}
					});
				});
			});
#			endif
		}

		void ResetLeafCache() const
		{
			leaves_cache.clear();
			leaves_cache.insert(leaves_cache.begin(), leaves.cbegin(), leaves.cend());
			ValidateOutputRowLeafIndexes();
		}

	public:

		mutable newgene_cpp_int number_branch_combinations;

};

typedef PrimaryKeysGroupingMultiplicityOne Branch;

void SpitBranch(std::string & sdata, Branch const & branch);

// ******************************************************************************************************** //
// ******************************************************************************************************** //
// Each time-slice has one of these
// ******************************************************************************************************** //
// (Only one, since currently only one primary top-level variable group is supported)
//

template <typename MEMORY_TAG>
using Branches = FastSetMemoryTag<Branch, MEMORY_TAG>;
//
// ******************************************************************************************************** //
// ******************************************************************************************************** //


// Thin wrapper for all of the branches (and their leaves)
// associated with any given time slice.
// This class does not contain any information about its associated time slice.
class VariableGroupBranchesAndLeaves
{

	public:

		VariableGroupBranchesAndLeaves(int const & variable_group_number_)
			: variable_group_number(variable_group_number_)
		{}

		int variable_group_number; // unused: Always the single primary top-level variable group identifier
		Branches<hits_tag> branches;
		Weighting weighting; // sum over all branches and leaves

		bool operator==(int const & rhs) const
		{
			if (variable_group_number == rhs)
			{
				return true;
			}

			return false;
		}

};

template <typename MEMORY_TAG>
using VariableGroupBranchesAndLeavesVector = FastVectorMemoryTag<VariableGroupBranchesAndLeaves, MEMORY_TAG>;

template <typename MEMORY_TAG>
struct tag__branches_and_leaves
{
	typedef VariableGroupBranchesAndLeavesVector<MEMORY_TAG> type;
};

// Instances of this class wrap all data associated with a given time slice.
class VariableGroupTimeSliceData
{

	public:

		VariableGroupTimeSliceData()
			: branches_and_leaves{ InstantiateUsingTopLevelObjectsPool<tag__branches_and_leaves<hits_tag>>() }
		{
		}

		VariableGroupTimeSliceData(VariableGroupTimeSliceData const & rhs)
			: branches_and_leaves{ InstantiateUsingTopLevelObjectsPool<tag__branches_and_leaves<hits_tag>>() }
			, weighting(rhs.weighting)
		{
			*branches_and_leaves = *rhs.branches_and_leaves;
		}

		VariableGroupTimeSliceData(VariableGroupTimeSliceData && rhs)
			: branches_and_leaves(rhs.branches_and_leaves)
			, weighting(rhs.weighting)
		{
			rhs.branches_and_leaves = nullptr;
		}

		~VariableGroupTimeSliceData()
		{
			if (!when_destructing_do_not_delete)
			{
				if (branches_and_leaves != nullptr)
				{
					DeleteUsingTopLevelObjectsPool<tag__branches_and_leaves<hits_tag>>(branches_and_leaves);
					branches_and_leaves = nullptr;
				}
			}
		}

		VariableGroupTimeSliceData & operator=(VariableGroupTimeSliceData const & rhs)
		{
			if (this == &rhs)
			{
				return *this;
			}
			*branches_and_leaves = *rhs.branches_and_leaves;
			weighting = rhs.weighting;
			return *this;
		}

		VariableGroupTimeSliceData & operator=(VariableGroupTimeSliceData && rhs)
		{
			if (this == &rhs)
			{
				return *this;
			}
			branches_and_leaves = rhs.branches_and_leaves;
			rhs.branches_and_leaves = nullptr;
			weighting = rhs.weighting;
			return *this;
		}

		// Currently, only one primary variable group is supported,
		// so this vector always has (currently) just one entry.
		// (Each entry contains possibly multiple branches,
		//  each with possibly multiple leaves)
		VariableGroupBranchesAndLeavesVector<hits_tag> * branches_and_leaves;
		Weighting weighting; // sum over all branches and leaves in all variable groups

		void ResetBranchCachesSingleTimeSlice(KadSampler & allWeightings, bool const reset_child_dmu_lookup);
		void PruneTimeUnits(KadSampler & allWeightings, TimeSlice const & originalTimeSlice, TimeSlice const & currentTimeSlice,
							bool const consolidate_rows, bool const random_sampling);

		static bool when_destructing_do_not_delete;
};

template <typename MEMORY_TAG>
using TimeSlices = FastMapMemoryTag<TimeSlice, VariableGroupTimeSliceData, MEMORY_TAG>;

typedef std::pair<TimeSlice, Leaf> TimeSliceLeaf;

class bind_visitor : public boost::static_visitor<>
{

	public:

		bind_visitor(sqlite3_stmt * stmt_, int const bindIndex_)
			: stmt(stmt_)
			, bindIndex(bindIndex_)
		{}

		void operator()(std::int32_t const & data)
		{
			sqlite3_bind_int64(stmt, bindIndex, data);
		}

		void operator()(double const & data)
		{
			sqlite3_bind_double(stmt, bindIndex, data);
		}

		void operator()(fast_string const & data)
		{
			bool use_converted { false };
			static fast_string converted_value;
			converted_value = data;
			if (converted_value.find(",") != std::string::npos)
			{
				// Comma in data: must surround entire field with double quotes
				// Note: NewGene currently does not support double quote inside field
				converted_value = ("\"" + converted_value + "\"");
				use_converted = true;
			}

			if (use_converted)
			{
				sqlite3_bind_text(stmt, bindIndex, converted_value.c_str(), static_cast<int>(data.size()), SQLITE_STATIC);
			}
			else
			{
				sqlite3_bind_text(stmt, bindIndex, data.c_str(), static_cast<int>(data.size()), SQLITE_STATIC);
			}
		}

		sqlite3_stmt * stmt;
		int const bindIndex;

};

void BindTermToInsertStatement(sqlite3_stmt * insert_random_sample_stmt, InstanceData const & data, int bindIndex);

class create_output_row_visitor : public boost::static_visitor<>
{

	public:

		enum MODE
		{
			CREATE_ROW_MODE__NONE = 0
			, CREATE_ROW_MODE__OUTPUT_FILE = 1
			, CREATE_ROW_MODE__INSTANCE_DATA_VECTOR = 2
			, CREATE_ROW_MODE__PREPARED_STATEMENT = 4
		};

		create_output_row_visitor(bool & first_)
			: first(first_)
		{}

		template <typename T>
		void operator()(const T & data_value) const
		{

			if (mode & CREATE_ROW_MODE__OUTPUT_FILE)
			{

				bool use_converted { false };
				converted_value = boost::lexical_cast<fast_string>(data_value);
				if (converted_value.find(",") != std::string::npos)
				{
					// Comma in data: must surround entire field with double quotes
					// Note: NewGene currently does not support double quote inside field
					converted_value = ("\"" + converted_value + "\"");
					use_converted = true;
				}

				if (!first)
				{
					(*output_file) << ",";

#					ifdef _DEBUG
						row_in_process += ",";
#					endif
				}

#				ifdef _DEBUG
					else
					{
						row_in_process.clear();
					}

#				endif

				if (use_converted)
				{
					(*output_file) << converted_value;
				}
				else
				{
					(*output_file) << data_value;
				}

#				ifdef _DEBUG
					row_in_process += boost::lexical_cast<std::string>(converted_value);
					//int m = 0;
					//if (row_in_process == "257,2,255,2,300,1918")
					//{
					//	int n = 0;
					//}
#				endif

			}

			if (mode & CREATE_ROW_MODE__INSTANCE_DATA_VECTOR)
			{
				bool use_converted { false };
				converted_value = boost::lexical_cast<fast_string>(data_value);
				if (converted_value.find(",") != std::string::npos)
				{
					// Comma in data: must surround entire field with double quotes
					// Note: NewGene currently does not support double quote inside field
					converted_value = ("\"" + converted_value + "\"");
					use_converted = true;
				}

				if (use_converted)
				{
					data->push_back(converted_value);
				}
				else
				{
					data->push_back(data_value);
				}
			}

			if (mode & CREATE_ROW_MODE__PREPARED_STATEMENT)
			{
				BindTermToInsertStatement(insert_stmt, converted_value, (*bind_index)++);
			}

			first = false;

		}

		static std::string row_in_process;
		static fast_string converted_value;
		static std::fstream * output_file;
		static InstanceDataVector<hits_tag> * data;
		static int * bind_index;
		static sqlite3_stmt * insert_stmt;
		static int mode;
		bool & first;

};

extern bool MergedTimeSliceRow_RHS_wins;

template <typename MEMORY_TAG>
class MergedTimeSliceRow
{

	public:

		MergedTimeSliceRow()
			: empty(true)
		{}

		template <typename MEMORY_TAG_RHS>
		MergedTimeSliceRow(TimeSlice const & ts, InstanceDataVector<MEMORY_TAG_RHS> const & row)
			: time_slice(ts)
			, empty(false)
		{
			output_row.insert(output_row.begin(), row.cbegin(), row.cend());
		}

		MergedTimeSliceRow(MergedTimeSliceRow<MEMORY_TAG> const & rhs)
		{
			// The optimizing compiler sometimes
			// re-uses an existing object instead
			// of constructing a new one in a non-trivial way.
			// When this constructor is called in an optimizing circumstance,
			// the compiler really means it.
			//
			// The code has already been roughly tested and
			// we know that in the Debug build, there are
			// never issues revealed with use of the "RHS_wins"
			// flag, because the error condition in operator=()
			// (where a throw occurs) is never reached
			// ... except in release mode...
			// ... so we know it's OK to set RHS_wins
			// true here, since it covers all possible
			// cases correctly (albeit it covers the
			// different cases correctly for different reasons,
			// which is unintuitive).
			//
			// Obviously this code is not thread-safe,
			// but that would be probably easily fixed
			// were it to become important.
			// For now - only a single output project
			// work queue thread will ever access this function.
			bool oldRHSWins = MergedTimeSliceRow_RHS_wins;
			MergedTimeSliceRow_RHS_wins = true;
			*this = rhs;
			MergedTimeSliceRow_RHS_wins = oldRHSWins;
		}

		template <typename MEMORY_TAG_RHS>
		MergedTimeSliceRow(MergedTimeSliceRow<MEMORY_TAG_RHS> const & rhs)
		{
			// Ditto comments in above ctor
			bool oldRHSWins = MergedTimeSliceRow_RHS_wins;
			MergedTimeSliceRow_RHS_wins = true;
			*this = rhs;
			MergedTimeSliceRow_RHS_wins = oldRHSWins;
		}

		template <typename MEMORY_TAG_RHS>
		bool operator<(MergedTimeSliceRow<MEMORY_TAG_RHS> const & rhs) const
		{
			size_t lhs_size = output_row.size();
			size_t rhs_size = rhs.output_row.size();
			size_t min_size = std::min(lhs_size, rhs_size);
			for (size_t n = 0; n < min_size; ++n)
			{
				if (output_row[n] < rhs.output_row[n])
				{
					return true;
				}
				else
				if (rhs.output_row[n] < output_row[n])
				{
					return false;
				}
			}
			if (lhs_size < rhs_size)
			{
				return true;
			}
			return false;
		}

		template <typename MEMORY_TAG_RHS>
		MergedTimeSliceRow<MEMORY_TAG> & operator=(MergedTimeSliceRow<MEMORY_TAG_RHS> const & rhs)
		{
			if ((void * const)(this) == (void * const)(&rhs))
			{
				return *this;
			}

			if (MergedTimeSliceRow_RHS_wins)
			{
				time_slice = rhs.time_slice;
				output_row.clear();
				output_row.insert(output_row.begin(), rhs.output_row.cbegin(), rhs.output_row.cend());
				empty = rhs.empty;
				return *this;
			}

			if (empty)
			{
				// Just accept RHS
				if (rhs.empty)
				{
					// do nothing
					return *this;
				}

				time_slice = rhs.time_slice;
				output_row.clear();
				output_row.insert(output_row.begin(), rhs.output_row.cbegin(), rhs.output_row.cend());
				empty = rhs.empty;

				return *this;
			}

			if (output_row.size() != rhs.output_row.size())
			{
				boost::format msg("Logic error merging MergedTimeSliceRow!  The merge should only occur for rows with identical primary keys");
				throw NewGeneException() << newgene_error_description(msg.str());
			}

			for (size_t n = 0; n < output_row.size(); ++n)
			{
				// See http://stackoverflow.com/a/1046317/368896 for why there is no != in Boost Variant
				if (!(output_row[n] == rhs.output_row[n]))
				{
					boost::format msg("Logic error merging MergedTimeSliceRow!  The merge should only occur for rows with identical primary keys");
					throw NewGeneException() << newgene_error_description(msg.str());
				}
			}

			if (rhs.empty)
			{
				// nothing to do
				return *this;
			}

			time_slice.Merge(rhs.time_slice);

			return *this;
		}

		TimeSlice time_slice;
		InstanceDataVector<MEMORY_TAG> output_row;

		bool empty; // When we merge, should we automatically set ourselves to the other?  Used to support default ctors for STL containers

};

template <typename MEMORY_TAG>
struct tag__saved_historic_rows
{
	typedef FastSetMemoryTag<MergedTimeSliceRow<MEMORY_TAG>, MEMORY_TAG> type;
};

template <typename MEMORY_TAG>
struct tag__ongoing_merged_rows
{
	typedef FastSetMemoryTag<MergedTimeSliceRow<MEMORY_TAG>, MEMORY_TAG> type;
};

template <typename MEMORY_TAG>
struct tag__ongoing_consolidation
{
	typedef FastSetMemoryTag<MergedTimeSliceRow<MEMORY_TAG>, MEMORY_TAG> type;
};

template <typename MEMORY_TAG>
struct tag__ongoing_consolidation_vector
{
	typedef FastVectorMemoryTag<MergedTimeSliceRow<MEMORY_TAG>, MEMORY_TAG> type;
};

template <typename MEMORY_TAG>
struct tag__calculate_consolidated_total_number_rows
{
	typedef FastSetMemoryTag<InstanceDataVector<MEMORY_TAG>, MEMORY_TAG, boost::function<bool(InstanceDataVector<calculate_consolidated_total_number_rows_tag> const & lhs, InstanceDataVector<calculate_consolidated_total_number_rows_tag> const & rhs)>> type;
};

template <typename MEMORY_TAG>
struct tag__calculate_consolidated_total_number_rows__instance_vector
{
	typedef InstanceDataVector<MEMORY_TAG> type;
};

template <typename MEMORY_TAG>
class SortMergedRowsByTimeThenKeys
{

	public:

		bool operator()(MergedTimeSliceRow<MEMORY_TAG> const & lhs, MergedTimeSliceRow<MEMORY_TAG> const & rhs)
		{

			if (lhs.time_slice < rhs.time_slice)
			{
				return true;
			}

			if (rhs.time_slice < lhs.time_slice)
			{
				return false;
			}

			// Equal in the time slice parameter, so go with data

			return lhs.output_row < rhs.output_row;

		}

};

template <typename MEMORY_TAG>
using fast__mergedtimeslicerow_set = FastSetMemoryTag<MergedTimeSliceRow<MEMORY_TAG>, MEMORY_TAG, SortMergedRowsByTimeThenKeys<MEMORY_TAG>>;

class KadSampler
{

	public:

		KadSampler(Messager & messager_);
		~KadSampler();

	public:

		// The main time slice data
		TimeSlices<hits_tag> timeSlices;
		Weighting weighting; // sum over all time slices
		Weighting weighting_consolidated; // weighting of consolidated output (not separated into time units that correspond to the time granularity of the primary variable group; i.e., if a COW MID lasts over 6 years for a given set of 3 countries for dyadic output, the contribution to weighting_consolidateed will be just 3 (the number of K-ads), rather than 3 * the number of days or years of the MID.  This is useful for full sampling mode where the data is consolidated.

	public:

		struct SizeOfSampler
		{
			SizeOfSampler()
				: numberMapNodes { 0 }
				, sizePod { 0 }
				, sizeTimeSlices { 0 }
				, sizeDataCache { 0 }
				, sizeOtherTopLevelCache { 0 }
				, sizeChildCache { 0 }
				, sizeMappingsFromChildBranchToPrimary { 0 }
				, sizeMappingFromChildLeafToPrimary { 0 }
				, size_childInternalToOneLeafColumnCountForDMUWithMultiplicityGreaterThan1 { 0 }
				, sizeConsolidatedRows { 0 }
				, sizeRandomNumbers { 0 }
				, totalSize { 0 }
			{}

			size_t numberMapNodes;
			size_t sizePod;
			size_t sizeTimeSlices;
			size_t sizeDataCache;
			size_t sizeOtherTopLevelCache;
			size_t sizeChildCache;
			size_t sizeMappingsFromChildBranchToPrimary;
			size_t sizeMappingFromChildLeafToPrimary;
			size_t size_childInternalToOneLeafColumnCountForDMUWithMultiplicityGreaterThan1;
			size_t sizeConsolidatedRows;
			size_t sizeRandomNumbers;
			size_t totalSize;

			void spitSizes(std::string & sdata)
			{

				sdata += "<TOTAL_SIZE>";
				sdata += boost::lexical_cast<std::string>(totalSize);
				sdata += "</TOTAL_SIZE>";

				sdata += "<TOTAL_NUMBER_MAP_NODES>";
				sdata += boost::lexical_cast<std::string>(numberMapNodes);
				sdata += "</TOTAL_NUMBER_MAP_NODES>";

				sdata += "<SIZE_INSTANCE>";
				sdata += boost::lexical_cast<std::string>(sizePod);
				sdata += "</SIZE_INSTANCE>";

				sdata += "<SIZE_TIME_SLICES>";
				sdata += boost::lexical_cast<std::string>(sizeTimeSlices);
				sdata += "</SIZE_TIME_SLICES>";

				sdata += "<SIZE_DATA_CACHE>";
				sdata += boost::lexical_cast<std::string>(sizeDataCache);
				sdata += "</SIZE_DATA_CACHE>";

				sdata += "<SIZE_OTHER_TOP_LEVEL_CACHES>";
				sdata += boost::lexical_cast<std::string>(sizeOtherTopLevelCache);
				sdata += "</SIZE_OTHER_TOP_LEVEL_CACHES>";

				sdata += "<SIZE_CHILD_CACHES>";
				sdata += boost::lexical_cast<std::string>(sizeChildCache);
				sdata += "</SIZE_CHILD_CACHES>";

				sdata += "<SIZE_MAPPING_FROM_CHILD_BRANCH_TO_PRIMARY>";
				sdata += boost::lexical_cast<std::string>(sizeMappingsFromChildBranchToPrimary);
				sdata += "</SIZE_MAPPING_FROM_CHILD_BRANCH_TO_PRIMARY>";

				sdata += "<SIZE_MAPPING_FROM_CHILD_LEAF_TO_PRIMARY>";
				sdata += boost::lexical_cast<std::string>(sizeMappingFromChildLeafToPrimary);
				sdata += "</SIZE_MAPPING_FROM_CHILD_LEAF_TO_PRIMARY>";

				sdata += "<SIZE_childInternalToOneLeafColumnCountForDMUWithMultiplicityGreaterThan1>";
				sdata += boost::lexical_cast<std::string>(size_childInternalToOneLeafColumnCountForDMUWithMultiplicityGreaterThan1);
				sdata += "</SIZE_childInternalToOneLeafColumnCountForDMUWithMultiplicityGreaterThan1>";

				sdata += "<SIZE_CONSOLIDATED_ROWS>";
				sdata += boost::lexical_cast<std::string>(sizeConsolidatedRows);
				sdata += "</SIZE_CONSOLIDATED_ROWS>";

				sdata += "<SIZE_RANDOM_NUMBERS>";
				sdata += boost::lexical_cast<std::string>(sizeRandomNumbers);
				sdata += "</SIZE_RANDOM_NUMBERS>";

			}
		};

		mutable SizeOfSampler mySize;
		void getMySize() const;

		// Cache of secondary data: One cache for the primary top-level variable group, and a set of caches for all other variable groups (the non-primary top-level groups, and the child groups)
		DataCache<hits_tag> dataCache; // caches secondary key data for the primary variable group, required to create final results in a fashion that can be migrated (partially) to disk via LIFO to support huge monadic input datasets used in the construction of kads
		fast_short_to_data_cache_map<hits_tag> otherTopLevelCache; // Ditto, but for non-primary top-level variable groups
		fast_short_to_data_cache_map<hits_tag> childCache; // Ditto, but for child variable groups

		// For each child variable group, a vector of mapping from the child key columns to the top-level key columns
		fast_int_to_childtoprimarymappingvector<hits_tag> mappings_from_child_branch_to_primary;
		fast_int_to_childtoprimarymappingvector<hits_tag> mappings_from_child_leaf_to_primary;
		fast_short_to_short_map<hits_tag> childInternalToOneLeafColumnCountForDMUWithMultiplicityGreaterThan1;
		int numberChildVariableGroups;
		TIME_GRANULARITY time_granularity; // The time granularity of the primary variable group
		std::int64_t random_rows_added;
		Messager & messager;

		int current_child_variable_group_being_merged; // temporary helper variable

		// final output in case of consolidated row output
		fast__mergedtimeslicerow_set<hits_consolidated_tag> consolidated_rows;

	public:

		sqlite3_stmt * insert_random_sample_stmt;

		// Returns "added", "continue handling slice", and "next map iterator"
		std::tuple<bool, bool, TimeSlices<hits_tag>::iterator> HandleIncomingNewBranchAndLeaf(Branch const & branch, TimeSliceLeaf & timeSliceLeaf, int const & variable_group_number,
				VARIABLE_GROUP_MERGE_MODE const merge_mode, bool const consolidate_rows, bool const random_sampling,
				TimeSlices<hits_tag>::iterator mapIterator_ = TimeSlices<hits_tag>::iterator(), bool const useIterator = false);
		void CalculateWeightings(int const K);
		void PrepareRandomNumbers(std::int64_t how_many);
		void PrepareRandomSamples(int const K);
		void PrepareFullSamples(int const K);
		bool RetrieveNextBranchAndLeaves(int const K);
		void PopulateAllLeafCombinations(std::int64_t const & which_time_unit, int const K, Branch const & branch);
		void ClearBranchCaches();
		void ResetBranchCaches(int const child_variable_group_number, bool const reset_child_dmu_lookup);
		void ConsolidateRowsWithinBranch(Branch const & branch, std::int64_t & current_rows, ProgressBarMeter & meter);
		void getChildToBranchColumnMappingsUsage(size_t & usage, fast_int_to_childtoprimarymappingvector<hits_tag> const & childToBranchColumnMappings) const;
		void getDataCacheUsage(size_t & usage, DataCache<hits_tag> const & dataCache) const;

		template <typename MEMORY_TAG>
		void getInstanceDataVectorUsage(size_t & usage, InstanceDataVector<MEMORY_TAG> const & instanceDataVector, bool const includeSelf) const
		{
			if (includeSelf)
			{
				usage += sizeof(instanceDataVector);
			}

			for (auto const & instanceData : instanceDataVector)
			{
				// Boost Variant is stack-based
				usage += sizeof(instanceData);
				//usage += boost::apply_visitor(size_of_visitor(), instanceData);
			}
		}

		void getLeafUsage(size_t & usage, Leaf const & leaf) const;

		template <typename MEMORY_TAG>
		void getSizeOutputRow(size_t & usage, BranchOutputRow<MEMORY_TAG> const & outputRow) const
		{
			usage += sizeof(outputRow);

			// primary_leaves is a set
			auto const & primary_leaves = outputRow.primary_leaves;
			mySize.numberMapNodes += primary_leaves.size();

			// primary_leaves_cache is a vector
			auto const & primary_leaves_cache = outputRow.primary_leaves_cache;

			// child_indices_into_raw_data is a map
			auto const & child_indices_into_raw_data = outputRow.child_indices_into_raw_data;
			mySize.numberMapNodes += child_indices_into_raw_data.size();

			for (auto const & primary_leaf : primary_leaves)
			{
				usage += sizeof(primary_leaf);
			}

			for (auto const & primary_leaf : primary_leaves_cache)
			{
				usage += sizeof(primary_leaf);
			}

			for (auto const & single_child_indices_into_raw_data : child_indices_into_raw_data)
			{
				usage += sizeof(single_child_indices_into_raw_data.first);
				usage += sizeof(single_child_indices_into_raw_data.second);

				// the_child_lookup_map is a map from POD to POD
				auto const & the_child_lookup_map = single_child_indices_into_raw_data.second;
				mySize.numberMapNodes += the_child_lookup_map.size();

				for (auto const & the_child_lookup_map_entry : the_child_lookup_map)
				{
					usage += sizeof(the_child_lookup_map_entry.first);
					usage += sizeof(the_child_lookup_map_entry.second);
				}
			}
		}

		void Clear(); // ditto

		void ClearTopLevelTag();

		void ClearRemaining();

		template <typename TAG>
		void PurgeTags()
		{
			purge_pool<TAG, sizeof(fast_short_to_int_map__loaded<TAG>::value_type const)>();
			purge_pool<TAG, sizeof(std::pair<fast__short__to__fast_short_to_int_map__loaded<TAG>::key_type, fast__short__to__fast_short_to_int_map__loaded<TAG>::mapped_type> const)>();
			purge_pool<TAG, sizeof(fast__int64__to__fast_branch_output_row_vector<TAG>::value_type const)>();
			purge_pool<TAG, sizeof(fast__int64__to__fast_branch_output_row_list<TAG>::value_type const)>();
			purge_pool<TAG, sizeof(fast_branch_output_row_ptr__to__fast_short_vector<TAG>::value_type const)>();
			purge_pool<TAG, sizeof(fast__lookup__from_child_dmu_set__to__output_rows<TAG>::value_type const)>();
			purge_pool<TAG, sizeof(fast_int_set<TAG>::value_type const)>();
			purge_pool<TAG, sizeof(fast_branch_output_row_vector_huge<TAG>::value_type const)>();
			purge_pool<TAG, sizeof(fast_branch_output_row_vector_____currently_only_used_for_Branch_remaining<TAG>::value_type const)>();
			purge_pool<TAG, sizeof(BranchOutputRow<TAG>)>();
			purge_pool<TAG, sizeof(fast_int_vector<TAG>::value_type const)>();
			purge_pool<TAG, sizeof(fast__int64__to__fast_branch_output_row_set<TAG>::value_type const)>();
			purge_pool<TAG, sizeof(fast_branch_output_row_set<TAG>::value_type const)>();
			purge_pool<TAG, sizeof(fast_short_to_short_map<TAG>::value_type const)>();
			purge_pool<TAG, sizeof(InstanceDataVector<TAG>::value_type const)>();
			purge_pool<TAG, sizeof(DMUInstanceDataVector<TAG>::value_type const)>();
			purge_pool<TAG, sizeof(ChildDMUInstanceDataVector<TAG>::value_type const)>();
			purge_pool<TAG, sizeof(SecondaryInstanceDataVector<TAG>::value_type const)>();
			purge_pool<TAG, sizeof(DataCache<TAG>::value_type const)>();
			purge_pool<TAG, sizeof(fast_short_to_data_cache_map<TAG>::value_type const)>();
			purge_pool<TAG, sizeof(InstanceData)>();
			purge_pool<TAG, sizeof(fast_short_vector<TAG>::value_type const)>();
			purge_pool<TAG, sizeof(fast_vector_childtoprimarymapping<TAG>::value_type const)>();
			purge_pool<TAG, sizeof(int)>();
			purge_pool<TAG, sizeof(char)>();
			purge_pool<TAG, sizeof(fast_leaf_vector<TAG>::value_type const)>();
			purge_pool<TAG, sizeof(Leaf)>();
			purge_pool<TAG, sizeof(ChildToPrimaryMapping)>();
			purge_pool<TAG, sizeof(MergedTimeSliceRow<TAG>)>();
			purge_pool<TAG, sizeof(VariableGroupBranchesAndLeaves)>();
			purge_pool<TAG, sizeof(VariableGroupBranchesAndLeavesVector<TAG>::value_type const)>();
			purge_pool<TAG, sizeof(fast__mergedtimeslicerow_set<TAG>::value_type const)>();
			purge_pool<TAG, sizeof(fast_short_to_int_map<TAG>::value_type const)>();
			purge_pool<TAG, sizeof(Leaves<TAG>::value_type const)>();
			purge_pool<TAG, sizeof(Leaf)>();
			purge_pool<TAG, sizeof(fast_int_to_childtoprimarymappingvector<TAG>::value_type const)>();
			purge_pool<TAG, sizeof(Branch)>();
			purge_pool<TAG, sizeof(Branches<TAG>::value_type const)>();
			purge_pool<TAG, sizeof(TimeSlices<TAG>::value_type const)>();
			purge_pool<TAG, sizeof(DataCache<TAG>::value_type const)>();
		}

		template <template <typename MEMORY_TAG> class TOP_LEVEL_TAG_STRUCT>
		void ClearTopLevelTag()
		{
			TopLevelObjectsPool<TOP_LEVEL_TAG_STRUCT<boost::pool_allocator_tag>>::purge_memory();
			TopLevelObjectsPool<TOP_LEVEL_TAG_STRUCT<boost::fast_pool_allocator_tag>>::purge_memory();
			TopLevelObjectsPool<TOP_LEVEL_TAG_STRUCT<hits_tag>>::purge_memory();
			TopLevelObjectsPool<TOP_LEVEL_TAG_STRUCT<hits_consolidated_tag>>::purge_memory();
			TopLevelObjectsPool<TOP_LEVEL_TAG_STRUCT<remaining_tag>>::purge_memory();
			TopLevelObjectsPool<TOP_LEVEL_TAG_STRUCT<child_dmu_lookup_tag>>::purge_memory();
			TopLevelObjectsPool<TOP_LEVEL_TAG_STRUCT<saved_historic_rows_tag>>::purge_memory();
			TopLevelObjectsPool<TOP_LEVEL_TAG_STRUCT<ongoing_merged_rows_tag>>::purge_memory();
			TopLevelObjectsPool<TOP_LEVEL_TAG_STRUCT<calculate_consolidated_total_number_rows_tag>>::purge_memory();
		}

	protected:

		bool HandleTimeSliceNormalCase(bool & added, Branch const & branch, TimeSliceLeaf & timeSliceLeaf, TimeSlices<hits_tag>::iterator & mapElementPtr, int const & variable_group_number,
									   VARIABLE_GROUP_MERGE_MODE const merge_mode, bool const consolidate_rows, bool const random_sampling);

		bool AddNewTimeSlice(int const & variable_group_number, Branch const & branch, TimeSliceLeaf const & newTimeSliceLeaf);

		// Breaks an existing map entry into two pieces and returns an iterator to both.
		void SliceMapEntry(TimeSlices<hits_tag>::iterator const & existingMapElementPtr, std::int64_t const middle, TimeSlices<hits_tag>::iterator & newMapElementLeftPtr,
			TimeSlices<hits_tag>::iterator & newMapElementRightPtr, bool const consolidate_rows, bool const random_sampling);

		// Breaks an existing map entry into three pieces and returns an iterator to the middle piece.
		void SliceMapEntry(TimeSlices<hits_tag>::iterator const & existingMapElementPtr, std::int64_t const left, std::int64_t const right, TimeSlices<hits_tag>::iterator & newMapElementMiddlePtr, bool const consolidate_rows, bool const random_sampling);

		// Slices off the left part of the "incoming_slice" TimeSliceLeaf and returns it in the "new_left_slice" TimeSliceLeaf.
		// The "incoming_slice" TimeSliceLeaf is adjusted to become equal to the remaining part on the right.
		void SliceOffLeft(TimeSliceLeaf & incoming_slice, std::int64_t const slicePoint, TimeSliceLeaf & new_left_slice);

		// Merge time slice data into a map element
		bool MergeNewDataIntoTimeSlice(Branch const & branch, TimeSliceLeaf const & timeSliceLeaf, TimeSlices<hits_tag>::iterator & mapElementPtr, int const & variable_group_number,
									   VARIABLE_GROUP_MERGE_MODE const merge_mode);

		void GenerateRandomKad(newgene_cpp_int random_number, int const K, Branch const & branch);
		void GenerateAllOutputRows(int const K, Branch const & branch);

		static bool is_map_entry_end_time_greater_than_new_time_slice_start_time(TimeSliceLeaf const & new_time_slice_, TimeSlices<hits_tag>::value_type const & map_entry_)
		{

			TimeSlice const & new_time_slice = new_time_slice_.first;
			TimeSlice const & map_entry = map_entry_.first;

			return map_entry.IsEndTimeGreaterThanRhsStartTime(new_time_slice);

		}

		FastVectorCppInt random_numbers;

	private:

		void AddPositionToRemaining(std::int64_t const & which_time_unit, std::vector<int> const & position, Branch const & branch);
		bool IncrementPosition(int const K, std::vector<int> & position, Branch const & branch);
		int IncrementPositionManageSubK(int const K, int const subK, std::vector<int> & position, Branch const & branch);

		newgene_cpp_int BinomialCoefficient(int const N, int const K);

	public:

		InstanceDataVector<hits_tag> create_output_row_visitor_global_data_cache;
		newgene_cpp_int number_rows_generated;
		FastVectorCppInt::const_iterator random_number_iterator;

		int number_branch_columns;
		int number_primary_variable_group_single_leaf_columns;
		bool debuggingflag;

		std::int64_t rowsWritten;
};

template <typename TAG, int SIZE>
void purge_pool()
{
	// The Pool allocator is subtle.
	// We provide the type T to the standard containers as the template parameter;
	// viz. std::set<int, std::less<int>, boost::fast_pool_allocator<int>>
	// (T here being 'int'),
	// but that does ***NOT*** mean that the memory allocated by the underlying pool
	// will be sizeof(T).
	// Instead, the type T is available to the standard containers as the 'value_type'
	// of the allocator that is passed as a template argument.
	// The standard containers may then adjust the ACTUAL number of bytes they allocate
	// per item in the container to be more than this - and they do.
	// Therefore, the ACTUAL underlying pool - boost::singleton_pool<> -
	// will NOT be a pool corresponding to sizeof(T).
	// It will be a pool corresponding to a larger value.
	//
	// Unfortunately, the standard does not specify what the actual size of allocation is
	// for items in the container.
	//
	// However, it is quite safe to assume that it will be 4, 8, or perhaps no more than 12 or 16
	// bytes in excess of sizeof(T).
	//
	// Therefore, we simply loop through all reasonable possibilities for the size of a node.
	// This is a negligible operation if there is no pool at the given size.

	// Smallest possible size of the internal node in bytes, followed by additional byte guesses up to 48 additional bytes
	boost::singleton_pool < TAG, SIZE - 20, boost::default_user_allocator_malloc_free, boost::details::pool::null_mutex >::purge_memory();
	boost::singleton_pool < TAG, SIZE - 19, boost::default_user_allocator_malloc_free, boost::details::pool::null_mutex >::purge_memory();
	boost::singleton_pool < TAG, SIZE - 18, boost::default_user_allocator_malloc_free, boost::details::pool::null_mutex >::purge_memory();
	boost::singleton_pool < TAG, SIZE - 17, boost::default_user_allocator_malloc_free, boost::details::pool::null_mutex >::purge_memory();
	boost::singleton_pool < TAG, SIZE - 16, boost::default_user_allocator_malloc_free, boost::details::pool::null_mutex >::purge_memory();
	boost::singleton_pool < TAG, SIZE - 15, boost::default_user_allocator_malloc_free, boost::details::pool::null_mutex >::purge_memory();
	boost::singleton_pool < TAG, SIZE - 14, boost::default_user_allocator_malloc_free, boost::details::pool::null_mutex >::purge_memory();
	boost::singleton_pool < TAG, SIZE - 13, boost::default_user_allocator_malloc_free, boost::details::pool::null_mutex >::purge_memory();
	boost::singleton_pool < TAG, SIZE - 12, boost::default_user_allocator_malloc_free, boost::details::pool::null_mutex >::purge_memory();
	boost::singleton_pool < TAG, SIZE - 11, boost::default_user_allocator_malloc_free, boost::details::pool::null_mutex >::purge_memory();
	boost::singleton_pool < TAG, SIZE - 10, boost::default_user_allocator_malloc_free, boost::details::pool::null_mutex >::purge_memory();
	boost::singleton_pool < TAG, SIZE - 9, boost::default_user_allocator_malloc_free, boost::details::pool::null_mutex >::purge_memory();
	boost::singleton_pool < TAG, SIZE - 8, boost::default_user_allocator_malloc_free, boost::details::pool::null_mutex >::purge_memory();
	boost::singleton_pool < TAG, SIZE - 7, boost::default_user_allocator_malloc_free, boost::details::pool::null_mutex >::purge_memory();
	boost::singleton_pool < TAG, SIZE - 6, boost::default_user_allocator_malloc_free, boost::details::pool::null_mutex >::purge_memory();
	boost::singleton_pool < TAG, SIZE - 5, boost::default_user_allocator_malloc_free, boost::details::pool::null_mutex >::purge_memory();
	boost::singleton_pool < TAG, SIZE - 4, boost::default_user_allocator_malloc_free, boost::details::pool::null_mutex >::purge_memory();
	boost::singleton_pool < TAG, SIZE - 3, boost::default_user_allocator_malloc_free, boost::details::pool::null_mutex >::purge_memory();
	boost::singleton_pool < TAG, SIZE - 2, boost::default_user_allocator_malloc_free, boost::details::pool::null_mutex >::purge_memory();
	boost::singleton_pool < TAG, SIZE - 1, boost::default_user_allocator_malloc_free, boost::details::pool::null_mutex >::purge_memory();
	boost::singleton_pool < TAG, SIZE + 0, boost::default_user_allocator_malloc_free, boost::details::pool::null_mutex >::purge_memory();
	boost::singleton_pool < TAG, SIZE + 1, boost::default_user_allocator_malloc_free, boost::details::pool::null_mutex >::purge_memory();
	boost::singleton_pool < TAG, SIZE + 2, boost::default_user_allocator_malloc_free, boost::details::pool::null_mutex >::purge_memory();
	boost::singleton_pool < TAG, SIZE + 3, boost::default_user_allocator_malloc_free, boost::details::pool::null_mutex >::purge_memory();
	boost::singleton_pool < TAG, SIZE + 4, boost::default_user_allocator_malloc_free, boost::details::pool::null_mutex >::purge_memory();
	boost::singleton_pool < TAG, SIZE + 5, boost::default_user_allocator_malloc_free, boost::details::pool::null_mutex >::purge_memory();
	boost::singleton_pool < TAG, SIZE + 6, boost::default_user_allocator_malloc_free, boost::details::pool::null_mutex >::purge_memory();
	boost::singleton_pool < TAG, SIZE + 7, boost::default_user_allocator_malloc_free, boost::details::pool::null_mutex >::purge_memory();
	boost::singleton_pool < TAG, SIZE + 8, boost::default_user_allocator_malloc_free, boost::details::pool::null_mutex >::purge_memory();
	boost::singleton_pool < TAG, SIZE + 9, boost::default_user_allocator_malloc_free, boost::details::pool::null_mutex >::purge_memory();
	boost::singleton_pool < TAG, SIZE + 10, boost::default_user_allocator_malloc_free, boost::details::pool::null_mutex >::purge_memory();
	boost::singleton_pool < TAG, SIZE + 11, boost::default_user_allocator_malloc_free, boost::details::pool::null_mutex >::purge_memory();
	boost::singleton_pool < TAG, SIZE + 12, boost::default_user_allocator_malloc_free, boost::details::pool::null_mutex >::purge_memory();
	boost::singleton_pool < TAG, SIZE + 13, boost::default_user_allocator_malloc_free, boost::details::pool::null_mutex >::purge_memory();
	boost::singleton_pool < TAG, SIZE + 14, boost::default_user_allocator_malloc_free, boost::details::pool::null_mutex >::purge_memory();
	boost::singleton_pool < TAG, SIZE + 15, boost::default_user_allocator_malloc_free, boost::details::pool::null_mutex >::purge_memory();
	boost::singleton_pool < TAG, SIZE + 16, boost::default_user_allocator_malloc_free, boost::details::pool::null_mutex >::purge_memory();
	boost::singleton_pool < TAG, SIZE + 17, boost::default_user_allocator_malloc_free, boost::details::pool::null_mutex >::purge_memory();
	boost::singleton_pool < TAG, SIZE + 18, boost::default_user_allocator_malloc_free, boost::details::pool::null_mutex >::purge_memory();
	boost::singleton_pool < TAG, SIZE + 19, boost::default_user_allocator_malloc_free, boost::details::pool::null_mutex >::purge_memory();
	boost::singleton_pool < TAG, SIZE + 20, boost::default_user_allocator_malloc_free, boost::details::pool::null_mutex >::purge_memory();
	boost::singleton_pool < TAG, SIZE + 21, boost::default_user_allocator_malloc_free, boost::details::pool::null_mutex >::purge_memory();
	boost::singleton_pool < TAG, SIZE + 22, boost::default_user_allocator_malloc_free, boost::details::pool::null_mutex >::purge_memory();
	boost::singleton_pool < TAG, SIZE + 23, boost::default_user_allocator_malloc_free, boost::details::pool::null_mutex >::purge_memory();
	boost::singleton_pool < TAG, SIZE + 24, boost::default_user_allocator_malloc_free, boost::details::pool::null_mutex >::purge_memory();
	boost::singleton_pool < TAG, SIZE + 25, boost::default_user_allocator_malloc_free, boost::details::pool::null_mutex >::purge_memory();
	boost::singleton_pool < TAG, SIZE + 26, boost::default_user_allocator_malloc_free, boost::details::pool::null_mutex >::purge_memory();
	boost::singleton_pool < TAG, SIZE + 27, boost::default_user_allocator_malloc_free, boost::details::pool::null_mutex >::purge_memory();
	boost::singleton_pool < TAG, SIZE + 28, boost::default_user_allocator_malloc_free, boost::details::pool::null_mutex >::purge_memory();
	boost::singleton_pool < TAG, SIZE + 29, boost::default_user_allocator_malloc_free, boost::details::pool::null_mutex >::purge_memory();
	boost::singleton_pool < TAG, SIZE + 30, boost::default_user_allocator_malloc_free, boost::details::pool::null_mutex >::purge_memory();
	boost::singleton_pool < TAG, SIZE + 31, boost::default_user_allocator_malloc_free, boost::details::pool::null_mutex >::purge_memory();
	boost::singleton_pool < TAG, SIZE + 32, boost::default_user_allocator_malloc_free, boost::details::pool::null_mutex >::purge_memory();
	boost::singleton_pool < TAG, SIZE + 33, boost::default_user_allocator_malloc_free, boost::details::pool::null_mutex >::purge_memory();
	boost::singleton_pool < TAG, SIZE + 34, boost::default_user_allocator_malloc_free, boost::details::pool::null_mutex >::purge_memory();
	boost::singleton_pool < TAG, SIZE + 35, boost::default_user_allocator_malloc_free, boost::details::pool::null_mutex >::purge_memory();
	boost::singleton_pool < TAG, SIZE + 36, boost::default_user_allocator_malloc_free, boost::details::pool::null_mutex >::purge_memory();
	boost::singleton_pool < TAG, SIZE + 37, boost::default_user_allocator_malloc_free, boost::details::pool::null_mutex >::purge_memory();
	boost::singleton_pool < TAG, SIZE + 38, boost::default_user_allocator_malloc_free, boost::details::pool::null_mutex >::purge_memory();
	boost::singleton_pool < TAG, SIZE + 39, boost::default_user_allocator_malloc_free, boost::details::pool::null_mutex >::purge_memory();
	boost::singleton_pool < TAG, SIZE + 40, boost::default_user_allocator_malloc_free, boost::details::pool::null_mutex >::purge_memory();
	boost::singleton_pool < TAG, SIZE + 41, boost::default_user_allocator_malloc_free, boost::details::pool::null_mutex >::purge_memory();
	boost::singleton_pool < TAG, SIZE + 42, boost::default_user_allocator_malloc_free, boost::details::pool::null_mutex >::purge_memory();
	boost::singleton_pool < TAG, SIZE + 43, boost::default_user_allocator_malloc_free, boost::details::pool::null_mutex >::purge_memory();
	boost::singleton_pool < TAG, SIZE + 44, boost::default_user_allocator_malloc_free, boost::details::pool::null_mutex >::purge_memory();
	boost::singleton_pool < TAG, SIZE + 45, boost::default_user_allocator_malloc_free, boost::details::pool::null_mutex >::purge_memory();
	boost::singleton_pool < TAG, SIZE + 46, boost::default_user_allocator_malloc_free, boost::details::pool::null_mutex >::purge_memory();
	boost::singleton_pool < TAG, SIZE + 47, boost::default_user_allocator_malloc_free, boost::details::pool::null_mutex >::purge_memory();
	boost::singleton_pool < TAG, SIZE + 48, boost::default_user_allocator_malloc_free, boost::details::pool::null_mutex >::purge_memory();

}

#endif
