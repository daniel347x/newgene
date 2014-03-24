#ifndef RANDOMSAMPLING_NEWGENE_H
#define RANDOMSAMPLING_NEWGENE_H

#include <cstdint>
#include <vector>
#include <set>
#include <map>
#ifndef Q_MOC_RUN
#	include <boost/multiprecision/cpp_int.hpp>
#	include <boost/variant.hpp>
#endif
#include "../../Utilities/NewGeneException.h"

typedef boost::variant<std::int32_t, std::int64_t, double, std::string> DMU;

class TimeSlice
{

	public:

		TimeSlice()
			: time_start{ 0 }
			, time_end  { 0 }
		{}

		TimeSlice(TimeSlice const & rhs)
			: time_start{ rhs.time_start }
			, time_end  { rhs.time_end }
		{}

		bool operator<(TimeSlice const & rhs) const
		{
			if (time_start < rhs.time_start)
			{
				return true;
			}
			else
			if (time_end < rhs.time_end)
			{
				return true;
			}
			return false;
		}

		inline bool Validate() const
		{
			if (time_end <= time_start)
			{
				return false;
			}
			return true;
		}

		inline bool IsStartTimeGreaterThanRhsEndTime(TimeSlice const & rhs) const
		{
			if (time_start > rhs.time_end)
			{
				return true;
			}
			return false;
		}

		inline bool IsEndTimeLessThanRhsEndTime(TimeSlice const & rhs) const
		{
			if (time_end < rhs.time_end)
			{
				return true;
			}
			return false;
		}

		std::int64_t time_start;
		std::int64_t time_end;

};

class Weighting
{

	public:

		Weighting()
			: weighting{ 0 }
			, weighting_range_start{ 0 }
			, weighting_range_end{ 0 }
		{}

		Weighting(Weighting const & rhs)
			: weighting{ rhs.weighting }
			, weighting_range_start{ rhs.weighting_range_start }
			, weighting_range_end{ rhs.weighting_range_end }
		{}

		boost::multiprecision::cpp_int weighting;
		boost::multiprecision::cpp_int weighting_range_start;
		boost::multiprecision::cpp_int weighting_range_end;

};

class PrimaryKeysGrouping
{

	public:

		PrimaryKeysGrouping()
		{}

		PrimaryKeysGrouping(PrimaryKeysGrouping const & rhs)
			: primary_keys(rhs.primary_keys)
		{}

		std::vector<DMU> primary_keys;

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
			bool operator()(T const & lhs, T const & rhs)
			{
				return lhs < rhs;
			}

		};

};

// "Leaf"
class PrimaryKeysGroupingMultiplicityGreaterThanOne : public PrimaryKeysGrouping
{

	public:

		PrimaryKeysGroupingMultiplicityGreaterThanOne()
			: PrimaryKeysGrouping()
			, index_into_raw_data{ 0 }
		{}

		PrimaryKeysGroupingMultiplicityGreaterThanOne(PrimaryKeysGroupingMultiplicityGreaterThanOne const & rhs)
			: PrimaryKeysGrouping(rhs)
			, index_into_raw_data{ rhs.index_into_raw_data }
		{}

		std::int64_t index_into_raw_data;

};

// "Branch"
class PrimaryKeysGroupingMultiplicityOne : public PrimaryKeysGrouping
{

	public:
		Weighting weighting; // Weighting for this branch: This is the lowest-level, calculated value

};

typedef PrimaryKeysGroupingMultiplicityGreaterThanOne Leaf;
typedef std::set<Leaf> Leaves;
typedef PrimaryKeysGroupingMultiplicityOne Branch;
typedef std::map<Branch, Leaves> BranchesAndLeaves;

class VariableGroupBranchesAndLeaves
{

	public:

		VariableGroupBranchesAndLeaves(std::string const & variable_group_name_)
			: variable_group_name(variable_group_name_)
		{}

		std::string variable_group_name;
		BranchesAndLeaves branches_and_leaves;
		Weighting weighting; // sum over all branches and leaves

		bool operator==(VariableGroupBranchesAndLeaves const & rhs) const
		{
			if (variable_group_name == rhs.variable_group_name)
			{
				return true;
			}
			return false;
		}

};

typedef std::vector<VariableGroupBranchesAndLeaves> VariableGroupBranchesAndLeavesVector;

class VariableGroupTimeSliceData
{

	public:

		VariableGroupBranchesAndLeavesVector branches_and_leaves;
		Weighting weighting; // sum over all branches and leaves in all variable groups

};

typedef std::map<TimeSlice, VariableGroupTimeSliceData> TimeSlices;

typedef std::pair<TimeSlice, Leaf> TimeSliceLeaf;

class AllWeightings
{

	public:

		TimeSlices timeSlices;
		Weighting weighting; // sum over all time slices

		void AddLeafToTimeSlices(TimeSliceLeaf const & timeSliceLeaf);

		void CalculateWeightings();

	protected:

		bool HandleTimeSliceNormalCase(TimeSliceLeaf & timeSliceLeaf, TimeSlices::iterator & mapElementPtr);
		// breaks an existing map entry into two pieces and returns an iterator to both
		void SliceMapEntry(std::int64_t const middle, TimeSlices::iterator & newMapElementLeftPtr, TimeSlices::iterator & newMapElementRightPtr);
		// breaks an existing map entry into three pieces and returns an iterator to the middle piece
		void SliceMapEntry(std::int64_t const left, std::int64_t const right, TimeSlices::iterator & newMapElementMiddlePtr);

		bool is_rhs_start_time_greater_than_lhs_end_time(TimeSlices::value_type const & lhs, TimeSliceLeaf const & rhs)
		{

			// The start time comparison compares the start time of 'rhs' to the map elements ('lhs') using std::upper_bound against the end time of the map's time slices
			TimeSlice const & lhs_slice = lhs.first;
			TimeSlice const & rhs_slice = rhs.first;

			// deliberate reversal of labels
			return rhs_slice.IsStartTimeGreaterThanRhsEndTime(lhs_slice);

		}

		bool is_rhs_end_time_less_than_lhs_end_time(TimeSlices::value_type const & lhs, TimeSliceLeaf const & rhs)
		{

			// The end time comparison compares the end time of 'rhs' to the map elements ('lhs') using std::lower_bound against the end time of the map's time slices
			TimeSlice const & lhs_slice = lhs.first;
			TimeSlice const & rhs_slice = rhs.first;

			// deliberate reversal of labels
			return rhs_slice.IsEndTimeLessThanRhsEndTime(lhs_slice);

		}

};

#endif
