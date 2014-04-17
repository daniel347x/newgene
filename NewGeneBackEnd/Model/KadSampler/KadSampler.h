#ifndef RANDOMSAMPLING_NEWGENE_H
#define RANDOMSAMPLING_NEWGENE_H

#include <cstdint>
#include <vector>
#include <set>
#include <map>
#include <tuple>
#include <memory>
#ifndef Q_MOC_RUN
#	include <boost/multiprecision/number.hpp>
#	include <boost/multiprecision/cpp_int.hpp>
#	include <boost/multiprecision/cpp_dec_float.hpp>
#	include <boost/variant.hpp>
#	include <boost/pool/pool_alloc.hpp>
#endif
#include "../../Utilities/NewGeneException.h"
#include "../../sqlite/sqlite-amalgamation-3071700/sqlite3.h"
#include "../TimeGranularity.h"
#include "../../Messager/Messager.h"
#include "../../Utilities/FastMap.h"
#include "../../Utilities/NewGeneMemoryPool.h"

typedef std::basic_string<char, std::char_traits<char>, boost::pool_allocator<char>> fast_string;

typedef FastMap<std::int16_t, std::int16_t> fast_short_to_short_map;
typedef boost::variant<std::int32_t, double, fast_string> InstanceData;
typedef boost::variant<std::int32_t, double, fast_string> DMUInstanceData;
typedef boost::variant<std::int32_t, double, fast_string> SecondaryInstanceData;

typedef FastVector<InstanceData> InstanceDataVector;
typedef InstanceDataVector DMUInstanceDataVector;
typedef InstanceDataVector ChildDMUInstanceDataVector;
typedef InstanceDataVector SecondaryInstanceDataVector;

typedef FastVector<std::int16_t> fast_short_vector;
typedef FastVector<int> fast_int_vector;
typedef FastSet<int> fast_int_set;
typedef FastMapLoaded<std::int16_t, std::int32_t> fast_short_to_int_map;
typedef FastMapLoaded<std::int16_t, std::int32_t>
fast_short_to_int_map__loaded; // known memory allocation hog that can crash in a somewhat fragmented heap, so throttle it way down by forcing small maximum block sizes but that won't crash
typedef FastMapLoaded<std::int16_t, fast_short_to_int_map__loaded> fast__short__to__fast_short_to_int_map__loaded;

// Row ID -> secondary data for that row for a given (unspecified) leaf
typedef FastMap<std::int32_t, SecondaryInstanceDataVector> DataCache;

typedef FastMap<std::int16_t, DataCache> fast_short_to_data_cache_map;

struct newgene_randomvector_tag {};
typedef boost::singleton_pool<newgene_randomvector_tag, sizeof(FastVector<newgene_cpp_int>)>
RandomVectorPool;

struct newgene_randomset_tag {};
typedef boost::singleton_pool<newgene_randomset_tag, sizeof(FastSet<newgene_cpp_int>)>
RandomSetPool;

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

		// ***************************************************************************** //
		// Return the number of time slots corresponding to the given time unit
		// ***************************************************************************** //
		std::int64_t WidthForWeighting(std::int64_t const ms_per_unit_time) const
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

			std::int64_t absolute = time_end - time_start;
			std::int64_t mod = absolute % ms_per_unit_time;
			std::int64_t val = absolute - mod;
			std::int64_t ret = val / ms_per_unit_time;

			if (mod < ms_per_unit_time / 2)
			{
				// round down
				return ret;
			}

			// round up
			return ret + 1;

		}

		static long double WidthForWeighting(std::int64_t const leftVal, std::int64_t const rightVal, std::int64_t const ms_per_unit_time)
		{
			std::int64_t absolute = rightVal - leftVal;
			return WidthForWeighting(absolute, ms_per_unit_time);
		}

		static long double WidthForWeighting(std::int64_t const absolute, std::int64_t const ms_per_unit_time)
		{
			return boost::lexical_cast<long double>(absolute) / boost::lexical_cast<long double>(ms_per_unit_time);
		}

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
		std::string weighting_string;
		std::string weighting_range_start_string;
		std::string weighting_range_end_string;
#	endif

		void InternalSetWeighting()
		{
			weighting_range_end = weighting_range_start + weighting - 1;
#		ifdef _DEBUG
			weighting_string = weighting.str();
			weighting_range_start_string = weighting_range_start.str();
			weighting_range_end_string = weighting_range_end.str();
#		endif
		}

};

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

typedef FastVector<ChildToPrimaryMapping> fast_vector_childtoprimarymapping;
typedef FastMap<int, fast_vector_childtoprimarymapping> fast_int_to_childtoprimarymappingvector;

enum VARIABLE_GROUP_MERGE_MODE
{
	VARIABLE_GROUP_MERGE_MODE__UNKNOWN
	, VARIABLE_GROUP_MERGE_MODE__PRIMARY
	, VARIABLE_GROUP_MERGE_MODE__TOP_LEVEL
	, VARIABLE_GROUP_MERGE_MODE__CHILD
};

class PrimaryKeysGrouping
{

	public:

		PrimaryKeysGrouping(DMUInstanceDataVector const & dmuInstanceDataVector)
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

		DMUInstanceDataVector primary_keys;

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

struct newgene_leaf_tag {};
typedef boost::singleton_pool<newgene_leaf_tag, sizeof(fast_short_to_int_map)>
LeafPool;

// "Leaf"
class PrimaryKeysGroupingMultiplicityGreaterThanOne : public PrimaryKeysGrouping
{

	public:

		PrimaryKeysGroupingMultiplicityGreaterThanOne()
			: PrimaryKeysGrouping { DMUInstanceDataVector() }
		, index_into_raw_data { 0 }
		, other_top_level_indices_into_raw_data_ { new(LeafPool::malloc())fast_short_to_int_map() }
		, other_top_level_indices_into_raw_data(*other_top_level_indices_into_raw_data_)
		{
		}

		PrimaryKeysGroupingMultiplicityGreaterThanOne(DMUInstanceDataVector const & dmuInstanceDataVector, std::int32_t const & index_into_raw_data_ = 0)
			: PrimaryKeysGrouping(dmuInstanceDataVector)
			, index_into_raw_data { index_into_raw_data_ }
		, other_top_level_indices_into_raw_data_ { new(LeafPool::malloc())fast_short_to_int_map() }
		, other_top_level_indices_into_raw_data { *other_top_level_indices_into_raw_data_ }
		{
		}

		PrimaryKeysGroupingMultiplicityGreaterThanOne(PrimaryKeysGroupingMultiplicityGreaterThanOne const & rhs)
			: PrimaryKeysGrouping(rhs)
			, index_into_raw_data { rhs.index_into_raw_data }
		, other_top_level_indices_into_raw_data_ { new(LeafPool::malloc())fast_short_to_int_map() }
		, other_top_level_indices_into_raw_data { *other_top_level_indices_into_raw_data_ }
		{
			other_top_level_indices_into_raw_data = rhs.other_top_level_indices_into_raw_data;
		}

		PrimaryKeysGroupingMultiplicityGreaterThanOne(PrimaryKeysGroupingMultiplicityGreaterThanOne const && rhs)
			: PrimaryKeysGrouping(std::move(rhs))
			, index_into_raw_data { rhs.index_into_raw_data }
		, other_top_level_indices_into_raw_data_ { new(LeafPool::malloc())fast_short_to_int_map() }
		, other_top_level_indices_into_raw_data { *other_top_level_indices_into_raw_data_ }
		{
			other_top_level_indices_into_raw_data = std::move(rhs.other_top_level_indices_into_raw_data);
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
			return *this;
		}

		~PrimaryKeysGroupingMultiplicityGreaterThanOne()
		{
			LeafPool::free(other_top_level_indices_into_raw_data_);
		}

		std::int32_t index_into_raw_data; // For the primary top-level variable group - the index of this leaf into the secondary data cache

		// The variable group index for this map will always skip the index of the primary top-level variable group - that value is stored in the above variable.
		mutable fast_short_to_int_map *
		other_top_level_indices_into_raw_data_; // For the non-primary top-level variable groups - the index of this leaf into the secondary data cache (mapped by variable group index)

		mutable fast_short_to_int_map &
		other_top_level_indices_into_raw_data; // For the non-primary top-level variable groups - the index of this leaf into the secondary data cache (mapped by variable group index)

};

struct newgene_branchoutputrow_tag {};
typedef boost::singleton_pool<newgene_branchoutputrow_tag, sizeof(fast__short__to__fast_short_to_int_map__loaded)>
BranchOutputRowPool;

class BranchOutputRow
{

	public:

		BranchOutputRow();
		BranchOutputRow(BranchOutputRow const & rhs);
		BranchOutputRow(BranchOutputRow && rhs);
		BranchOutputRow & operator=(BranchOutputRow const & rhs);
		BranchOutputRow & operator=(BranchOutputRow && rhs);

		// Destructor to debug
		~BranchOutputRow();

		bool operator==(BranchOutputRow const & rhs) const
		{
			return (!(*this < rhs) && !(rhs < *this));
		}

		bool operator!=(BranchOutputRow const & rhs) const
		{
			return (!(*this == rhs));
		}

		bool operator<(BranchOutputRow const & rhs) const
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
		fast_int_set primary_leaves;

	public:

		fast_int_vector primary_leaves_cache; // for optimized lookup only

		// Map from child variable group ID to:
		// Map from child leaf index to:
		// index into child variable group's raw data cache (stored in the AllWeightings instance)
		mutable fast__short__to__fast_short_to_int_map__loaded * child_indices_into_raw_data_;
		mutable fast__short__to__fast_short_to_int_map__loaded & child_indices_into_raw_data;

	private:

		void SaveCache()
		{
			primary_leaves_cache.clear();
			primary_leaves_cache.insert(primary_leaves_cache.begin(), primary_leaves.begin(), primary_leaves.end());
		}

};

typedef PrimaryKeysGroupingMultiplicityGreaterThanOne Leaf;

typedef FastVector<Leaf> fast_leaf_vector;
typedef FastSet<Leaf> Leaves;

typedef FastVector<BranchOutputRow> fast_branch_output_row_vector;
typedef FastVector<BranchOutputRow> fast_branch_output_row_vector_huge; // no difference at this point because code pre-allocates full size, so extra template param was removed
typedef FastSet<BranchOutputRow> fast_branch_output_row_set;

typedef FastMap<BranchOutputRow const *, fast_short_vector> fast_branch_output_row_ptr__to__fast_short_vector;

typedef FastMap<std::int64_t, fast_branch_output_row_set> fast__int64__to__fast_branch_output_row_set;
typedef FastMap<std::int64_t, fast_branch_output_row_vector> fast__int64__to__fast_branch_output_row_vector;

typedef FastMapFlat<ChildDMUInstanceDataVector, fast_branch_output_row_ptr__to__fast_short_vector> fast__lookup__from_child_dmu_set__to__output_rows;

// ******************************************************************************************************************************************************************** //
// Output data to XML for debugging
// ******************************************************************************************************************************************************************** //
void SpitAllWeightings(KadSampler const & allWeightings, std::string const & file_name_appending_string);
// ******************************************************************************************************************************************************************** //
// SpitAllWeightings() is the important one... just use that for debugging.
// Internally, it calls the functions below.
// ******************************************************************************************************************************************************************** //
void SpitKeys(std::string & sdata, FastVector<DMUInstanceData> const & dmu_keys);
void SpitDataCache(std::string & sdata, DataCache const & dataCache);
void SpitDataCaches(std::string & sdata, fast_short_to_data_cache_map const & dataCaches);
void SpitHits(std::string & sdata, fast__int64__to__fast_branch_output_row_set const & hits);
void SpitOutputRow(std::string & sdata, BranchOutputRow const & row);
void SpitChildLookup(std::string & sdata, fast__lookup__from_child_dmu_set__to__output_rows const & helperLookup);
void SpitLeaf(std::string & sdata, Leaf const & leaf);
void SpitWeighting(std::string & sdata, Weighting const & weighting);
void SpitTimeSlice(std::string & sdata, TimeSlice const & time_slice);
void SpitChildToPrimaryKeyColumnMapping(std::string & sdata, ChildToPrimaryMapping const & childToPrimaryMapping);

// "Branch"
class PrimaryKeysGroupingMultiplicityOne : public PrimaryKeysGrouping
{

	public:

		PrimaryKeysGroupingMultiplicityOne()
			: PrimaryKeysGrouping { DMUInstanceDataVector() }
		{}

		PrimaryKeysGroupingMultiplicityOne(DMUInstanceDataVector const & dmuInstanceDataVector)
			: PrimaryKeysGrouping(dmuInstanceDataVector)
		{
		}

		PrimaryKeysGroupingMultiplicityOne(PrimaryKeysGroupingMultiplicityOne const & rhs)
			: PrimaryKeysGrouping(rhs)
			, weighting { rhs.weighting }
		, hits { rhs.hits }
		, remaining { rhs.remaining }
		, number_branch_combinations { rhs.number_branch_combinations }
		, leaves { rhs.leaves }
		, leaves_cache { rhs.leaves_cache }
		{
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
			remaining = rhs.remaining;
			number_branch_combinations = rhs.number_branch_combinations;
			leaves = rhs.leaves;
			leaves_cache = rhs.leaves_cache;
			hits_consolidated = rhs.hits_consolidated;
			return *this;
		}

		PrimaryKeysGroupingMultiplicityOne & operator=(PrimaryKeysGroupingMultiplicityOne const && rhs)
		{
			if (&rhs == this)
			{
				return *this;
			}

			PrimaryKeysGrouping::operator=(std::move(rhs));
			weighting = rhs.weighting;
			hits = std::move(rhs.hits);
			remaining = std::move(rhs.remaining);
			number_branch_combinations = std::move(rhs.number_branch_combinations);
			leaves = std::move(rhs.leaves);
			leaves_cache = std::move(rhs.leaves_cache);
			hits_consolidated = std::move(rhs.hits_consolidated);
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
				SpitLeaf(sdata, leaf);
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


		// ******************************************************************************************************** //
		// ******************************************************************************************************** //
		// The following is the official location of the randomly generated rows
		// ******************************************************************************************************** //
		// Leaf combinations hit by the random generator.
		//
		// Map from time unit to a set of leaf combinations hit for that time units
		// Time unit index is 0-based
		//
		mutable fast__int64__to__fast_branch_output_row_set hits;
		mutable fast_branch_output_row_vector_huge hits_consolidated;
		//
		// ******************************************************************************************************** //
		// ******************************************************************************************************** //


		// Used for optimization purposes only during random sampling construction of output rows
		mutable fast__int64__to__fast_branch_output_row_vector remaining;

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
		mutable fast__lookup__from_child_dmu_set__to__output_rows helper_lookup__from_child_key_set__to_matching_output_rows;

		// Cache for use with "consolidating rows" phase, if it is every necessary!
		// Currently, this will never be filled, because we have no need to perform the lookup after rows have been consolidated,
		// and the profiler shows a major hit during the consolidating of rows in managing this cache, so disable it.
		mutable fast__lookup__from_child_dmu_set__to__output_rows helper_lookup__from_child_key_set__to_matching_output_rows_consolidating;

		void ConstructChildCombinationCache(KadSampler & allWeightings, int const variable_group_number, bool const force,
											bool const is_consolidating = false) const; // Populate the above data structure

		void InsertLeaf(Leaf const & leaf) const
		{
			leaves.insert(leaf);
			ResetLeafCache();
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
			auto const leafPtr = leaves.find(existingLeaf);
			leafPtr->other_top_level_indices_into_raw_data[variable_group_number] = other_top_level_index_into_raw_data;
			ResetLeafCache();
		}

	private:
	public: // debugging access

		mutable Leaves leaves;

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
		mutable fast_leaf_vector leaves_cache;

	public:

		void ValidateOutputRowLeafIndexes() const
		{
#			ifdef _DEBUG
			std::for_each(hits.cbegin(), hits.cend(), [&](decltype(hits)::value_type const & hitsEntry)
			{
				fast_branch_output_row_set const & hits = hitsEntry.second;
				std::for_each(hits.cbegin(), hits.cend(), [&](BranchOutputRow const & outputRow)
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
//typedef FastSet<Branch> Branches;
typedef FastSetFlat<Branch> Branches;
//
// ******************************************************************************************************** //
// ******************************************************************************************************** //



class VariableGroupBranchesAndLeaves
{

	public:

		VariableGroupBranchesAndLeaves(int const & variable_group_number_)
			: variable_group_number(variable_group_number_)
		{}

		int variable_group_number; // unused: Always the single primary top-level variable group identifier
		Branches branches;
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

typedef FastVector<VariableGroupBranchesAndLeaves> VariableGroupBranchesAndLeavesVector;

class VariableGroupTimeSliceData
{

	public:

		VariableGroupBranchesAndLeavesVector branches_and_leaves;
		Weighting weighting; // sum over all branches and leaves in all variable groups

		void ResetBranchCachesSingleTimeSlice(KadSampler & allWeightings, bool const reset_child_dmu_lookup);
		void PruneTimeUnits(KadSampler & allWeightings, TimeSlice const & originalTimeSlice, TimeSlice const & currentTimeSlice, std::int64_t const AvgMsperUnit,
							bool const consolidate_rows, bool const random_sampling);

};

typedef FastMap<TimeSlice, VariableGroupTimeSliceData> TimeSlices;

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
			sqlite3_bind_text(stmt, bindIndex, data.c_str(), static_cast<int>(data.size()), SQLITE_STATIC);
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

				if (!first)
				{
					(*output_file) << ",";

#				ifdef _DEBUG
					row_in_process += ",";
#				endif
				}

#			ifdef _DEBUG
				else
				{
					row_in_process.clear();
				}

#			endif

				(*output_file) << data_value;

#			ifdef _DEBUG
				row_in_process += boost::lexical_cast<std::string>(data_value);
				int m = 0;

				if (row_in_process == "257,2,255,2,300,1918")
				{
					int n = 0;
				}

#			endif

			}

			if (mode & CREATE_ROW_MODE__INSTANCE_DATA_VECTOR)
			{
				data->push_back(data_value);
			}

			if (mode & CREATE_ROW_MODE__PREPARED_STATEMENT)
			{
				BindTermToInsertStatement(insert_stmt, data_value, (*bind_index)++);
			}

			first = false;

		}

		static std::string row_in_process;
		static std::fstream * output_file;
		static InstanceDataVector * data;
		static int * bind_index;
		static sqlite3_stmt * insert_stmt;
		static int mode;
		bool & first;

};

class MergedTimeSliceRow
{

	public:

		MergedTimeSliceRow()
			: empty(true)
		{}

		MergedTimeSliceRow(TimeSlice const & ts, InstanceDataVector const & row)
			: time_slice(ts)
			, output_row(row)
			, empty(false)
		{}

		MergedTimeSliceRow(MergedTimeSliceRow const & rhs)
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
			bool oldRHSWins = RHS_wins;
			RHS_wins = true;
			*this = rhs;
			RHS_wins = oldRHSWins;
		}

		bool operator<(MergedTimeSliceRow const & rhs) const
		{
			return (output_row < rhs.output_row);
		}

		MergedTimeSliceRow & operator=(MergedTimeSliceRow const & rhs)
		{
			if (this == &rhs)
			{
				return *this;
			}

			if (RHS_wins)
			{
				time_slice = rhs.time_slice;
				output_row = rhs.output_row;
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
				output_row = rhs.output_row;
				empty = rhs.empty;

				return *this;
			}

			if (output_row != rhs.output_row)
			{
				boost::format msg("Logic error merging MergedTimeSliceRow!  The merge should only occur for rows with identical primary keys");
				throw NewGeneException() << newgene_error_description(msg.str());
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
		InstanceDataVector output_row;

		static bool RHS_wins; // controls how operator=() should behave.  Note: Only required because release-mode optimizer utilizes operator=() instead of ctor

	private:

		bool empty; // When we merge, should we automatically set ourselves to the other?  Used to support default ctors for STL containers

};

class SortMergedRowsByTimeThenKeys
{

	public:

		bool operator()(MergedTimeSliceRow const & lhs, MergedTimeSliceRow const & rhs)
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

typedef FastSet<MergedTimeSliceRow, SortMergedRowsByTimeThenKeys> fast__mergedtimeslicerow_set;

class KadSampler
{

	public:

		KadSampler(Messager & messager_);
		~KadSampler();

	public:

		// The main time slice data
		TimeSlices timeSlices;
		Weighting weighting; // sum over all time slices

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
		DataCache dataCache; // caches secondary key data for the primary variable group, required to create final results in a fashion that can be migrated (partially) to disk via LIFO to support huge monadic input datasets used in the construction of kads
		fast_short_to_data_cache_map otherTopLevelCache; // Ditto, but for non-primary top-level variable groups
		fast_short_to_data_cache_map childCache; // Ditto, but for child variable groups

		// For each child variable group, a vector of mapping from the child key columns to the top-level key columns
		fast_int_to_childtoprimarymappingvector mappings_from_child_branch_to_primary;
		fast_int_to_childtoprimarymappingvector mappings_from_child_leaf_to_primary;
		fast_short_to_short_map childInternalToOneLeafColumnCountForDMUWithMultiplicityGreaterThan1;
		int numberChildVariableGroups;
		TIME_GRANULARITY time_granularity;
		std::int64_t random_rows_added;
		Messager & messager;

		// final output in case of consolidated row output
		fast__mergedtimeslicerow_set consolidated_rows;

	public:

		sqlite3_stmt * insert_random_sample_stmt;

		// Returns "added", "continue handling slice", and "next map iterator"
		std::tuple<bool, bool, TimeSlices::iterator> HandleIncomingNewBranchAndLeaf(Branch const & branch, TimeSliceLeaf & timeSliceLeaf, int const & variable_group_number,
				VARIABLE_GROUP_MERGE_MODE const merge_mode, std::int64_t const AvgMsperUnit, bool const consolidate_rows, bool const random_sampling,
				TimeSlices::iterator mapIterator_ = TimeSlices::iterator(), bool const useIterator = false);
		void CalculateWeightings(int const K, std::int64_t const ms_per_unit_time);
		void PrepareRandomNumbers(std::int64_t how_many);
		void PrepareRandomSamples(int const K);
		void PrepareFullSamples(int const K);
		bool RetrieveNextBranchAndLeaves(int const K);
		void PopulateAllLeafCombinations(std::int64_t const & which_time_unit, int const K, Branch const & branch);
		void ResetBranchCaches(bool const reset_child_dmu_lookup);
		void ConsolidateRowsWithinBranch(Branch const & branch, std::int64_t & current_rows, ProgressBarMeter & meter);
		void getChildToBranchColumnMappingsUsage(size_t & usage, fast_int_to_childtoprimarymappingvector const & childToBranchColumnMappings) const;
		void getDataCacheUsage(size_t & usage, DataCache const & dataCache) const;
		void getInstanceDataVectorUsage(size_t & usage, InstanceDataVector const & instanceDataVector, bool const includeSelf = false) const;
		void getLeafUsage(size_t & usage, Leaf const & leaf) const;
		void getSizeOutputRow(size_t & usage, BranchOutputRow const & outputRow) const;
		void Clear(); // ditto

	protected:

		bool HandleTimeSliceNormalCase(bool & added, Branch const & branch, TimeSliceLeaf & timeSliceLeaf, TimeSlices::iterator & mapElementPtr, int const & variable_group_number,
									   VARIABLE_GROUP_MERGE_MODE const merge_mode, std::int64_t const AvgMsperUnit, bool const consolidate_rows, bool const random_sampling);

		void AddNewTimeSlice(int const & variable_group_number, Branch const & branch, TimeSliceLeaf const & newTimeSliceLeaf);

		// Breaks an existing map entry into two pieces and returns an iterator to both.
		void SliceMapEntry(TimeSlices::iterator const & existingMapElementPtr, std::int64_t const middle, TimeSlices::iterator & newMapElementLeftPtr,
						   TimeSlices::iterator & newMapElementRightPtr, std::int64_t const AvgMsperUnit, bool const consolidate_rows, bool const random_sampling);

		// Breaks an existing map entry into three pieces and returns an iterator to the middle piece.
		void SliceMapEntry(TimeSlices::iterator const & existingMapElementPtr, std::int64_t const left, std::int64_t const right, TimeSlices::iterator & newMapElementMiddlePtr,
						   std::int64_t const AvgMsperUnit, bool const consolidate_rows, bool const random_sampling);

		// Slices off the left part of the "incoming_slice" TimeSliceLeaf and returns it in the "new_left_slice" TimeSliceLeaf.
		// The "incoming_slice" TimeSliceLeaf is adjusted to become equal to the remaining part on the right.
		void SliceOffLeft(TimeSliceLeaf & incoming_slice, std::int64_t const slicePoint, TimeSliceLeaf & new_left_slice);

		// Merge time slice data into a map element
		bool MergeTimeSliceDataIntoMap(Branch const & branch, TimeSliceLeaf const & timeSliceLeaf, TimeSlices::iterator & mapElementPtr, int const & variable_group_number,
									   VARIABLE_GROUP_MERGE_MODE const merge_mode);

		void GenerateOutputRow(newgene_cpp_int random_number, int const K, Branch const & branch);
		void GenerateAllOutputRows(int const K, Branch const & branch);

		static bool is_map_entry_end_time_greater_than_new_time_slice_start_time(TimeSliceLeaf const & new_time_slice_, TimeSlices::value_type const & map_entry_)
		{

			TimeSlice const & new_time_slice = new_time_slice_.first;
			TimeSlice const & map_entry = map_entry_.first;

			return map_entry.IsEndTimeGreaterThanRhsStartTime(new_time_slice);

		}

		FastVectorCppInt<newgene_cpp_int> random_numbers;

	private:

		void AddPositionToRemaining(std::int64_t const & which_time_unit, std::vector<int> const & position, Branch const & branch);
		bool IncrementPosition(int const K, std::vector<int> & position, Branch const & branch);
		int IncrementPositionManageSubK(int const K, int const subK, std::vector<int> & position, Branch const & branch);

		newgene_cpp_int BinomialCoefficient(int const N, int const K);

	public:

		InstanceDataVector create_output_row_visitor_global_data_cache;
		newgene_cpp_int number_rows_generated;
		FastVector<newgene_cpp_int>::const_iterator random_number_iterator;

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
