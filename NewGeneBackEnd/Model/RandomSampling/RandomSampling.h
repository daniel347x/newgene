#ifndef RANDOMSAMPLING_NEWGENE_H
#define RANDOMSAMPLING_NEWGENE_H

#include <cstdint>
#include <vector>
#include <set>
#include <map>
#ifndef Q_MOC_RUN
#	include <boost/multiprecision/number.hpp>
#	include <boost/multiprecision/cpp_int.hpp>
#	include <boost/variant.hpp>
#endif
#include "../../Utilities/NewGeneException.h"
#include "../../sqlite/sqlite-amalgamation-3071700/sqlite3.h"

typedef boost::variant<std::int64_t, double, std::string> InstanceData;
typedef boost::variant<std::int64_t, double, std::string> DMUInstanceData;
typedef boost::variant<std::int64_t, double, std::string> SecondaryInstanceData;

typedef std::vector<DMUInstanceData> DMUInstanceDataVector;
typedef DMUInstanceDataVector ChildDMUInstanceDataVector;
typedef std::vector<SecondaryInstanceData> SecondaryInstanceDataVector;

// Row ID -> secondary data for that row
typedef std::map<std::int64_t, SecondaryInstanceDataVector> DataCache;

class AllWeightings;

class TimeSlice
{

	public:

		TimeSlice()
			: time_start{ 0 }
			, time_end  { 0 }
		{}

		TimeSlice(std::int64_t time_start_, std::int64_t time_end_)
			: time_start{ time_start_ }
			, time_end{ time_end_ }
		{}

		TimeSlice(TimeSlice const & rhs)
			: time_start{ rhs.time_start }
			, time_end  { rhs.time_end }
		{}

		std::int64_t Width(std::int64_t const ms_per_unit_time) const
		{
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

		TimeSlice & operator=(TimeSlice const & rhs)
		{
			if (&rhs == this)
			{
				return *this;
			}
			time_start = rhs.time_start;
			time_end = rhs.time_end;
			return *this;
		}

		void Reshape(std::int64_t const & new_start, std::int64_t const & new_end)
		{
			time_start = new_start;
			time_end = new_end;
		}

		bool operator<(TimeSlice const & rhs) const
		{
			if (time_start < rhs.time_start)
			{
				return true;
			}
			else
			if (time_start > rhs.time_start)
			{
				return false;
			}
			else
			if (time_end < rhs.time_end)
			{
				return true;
			}
			else
			if (time_end > rhs.time_end)
			{
				return false;
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

		inline bool IsEndTimeGreaterThanRhsStartTime(TimeSlice const & rhs) const
		{
			if (time_end > rhs.time_start)
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
		{
			InternalSetWeighting();
		}

		Weighting(Weighting const & rhs)
			: weighting{ rhs.weighting }
			, weighting_range_start{ rhs.weighting_range_start }
			, weighting_range_end{ rhs.weighting_range_end }
		{
			InternalSetWeighting();
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

		void setWeighting(boost::multiprecision::cpp_int const & weighting_)
		{
			weighting = weighting_;
			weighting_range_end = weighting_range_start + weighting - 1;
			InternalSetWeighting();
		}

		void setWeightingRangeStart(boost::multiprecision::cpp_int const & weighting_range_start_)
		{
			weighting_range_start = weighting_range_start_;
			weighting_range_end = weighting_range_start + weighting - 1;
			InternalSetWeighting();
		}

		void addWeighting(boost::multiprecision::cpp_int const & weighting_to_add)
		{
			weighting += weighting_to_add;
			InternalSetWeighting();
		}

		boost::multiprecision::cpp_int getWeighting() const
		{
			return weighting;
		}

		boost::multiprecision::cpp_int getWeightingRangeStart() const
		{
			return weighting_range_start;
		}

		boost::multiprecision::cpp_int getWeightingRangeEnd() const
		{
			return weighting_range_end;
		}

	private:
	
		boost::multiprecision::cpp_int weighting;
		boost::multiprecision::cpp_int weighting_range_start;
		boost::multiprecision::cpp_int weighting_range_end;
#		ifdef _DEBUG
		std::string weighting_string;
		std::string weighting_range_start_string;
		std::string weighting_range_end_string;
#		endif

		void InternalSetWeighting()
		{
			weighting_range_end = weighting_range_start + weighting - 1;
#			ifdef _DEBUG
			weighting_string = weighting.str();
			weighting_range_start_string = weighting_range_start.str();
			weighting_range_end_string = weighting_range_end.str();
#			endif
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
	: mapping(mapping_), index(index_), leaf_number(leaf_number_) {}

	CHILD_TO_PRIMARY_MAPPING mapping;
	int index;
	int leaf_number;
};

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

// "Leaf"
class PrimaryKeysGroupingMultiplicityGreaterThanOne : public PrimaryKeysGrouping
{

	public:

		PrimaryKeysGroupingMultiplicityGreaterThanOne()
			: PrimaryKeysGrouping{ DMUInstanceDataVector() }
			, index_into_raw_data{ 0 }
		{}

		PrimaryKeysGroupingMultiplicityGreaterThanOne(DMUInstanceDataVector const & dmuInstanceDataVector, std::int64_t const & index_into_raw_data_ = -1)
			: PrimaryKeysGrouping(dmuInstanceDataVector)
			, index_into_raw_data{ index_into_raw_data_ }
		{}

		PrimaryKeysGroupingMultiplicityGreaterThanOne(PrimaryKeysGroupingMultiplicityGreaterThanOne const & rhs)
			: PrimaryKeysGrouping(rhs)
			, index_into_raw_data{ rhs.index_into_raw_data }
		{}

		PrimaryKeysGroupingMultiplicityGreaterThanOne & operator=(PrimaryKeysGroupingMultiplicityGreaterThanOne const & rhs)
		{
			if (&rhs == this)
			{
				return *this;
			}
			PrimaryKeysGrouping::operator=(rhs);
			index_into_raw_data = rhs.index_into_raw_data;
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
			return *this;
		}

		std::int64_t index_into_raw_data; // For the primary top-level variable group - the index of this leaf into the secondary data cache

		// The variable group index for this map will always skip the index of the primary top-level variable group - that value is stored in the above variable.
		mutable std::map<int, std::int64_t> other_top_level_indices_into_raw_data; // For the non-primary top-level variable groups - the index of this leaf into the secondary data cache (mapped by variable group index)

};

class BranchOutputRow
{

	public:

		BranchOutputRow()
		{}

		BranchOutputRow(BranchOutputRow const & rhs)
			: primary_leaves(rhs.primary_leaves)
		{}

		BranchOutputRow(BranchOutputRow && rhs)
			: primary_leaves(std::move(rhs.primary_leaves))
		{}

		BranchOutputRow & operator=(BranchOutputRow const & rhs)
		{
			if (this == &rhs)
			{
				return *this;
			}
			primary_leaves = rhs.primary_leaves;
			return *this;
		}

		BranchOutputRow & operator=(BranchOutputRow && rhs)
		{
			if (this == &rhs)
			{
				return *this;
			}
			primary_leaves = std::move(rhs.primary_leaves);
			return *this;
		}

		bool operator==(BranchOutputRow const & rhs)
		{
			return primary_leaves == rhs.primary_leaves;
		}

		bool operator<(BranchOutputRow const & rhs) const
		{
			return primary_leaves < rhs.primary_leaves;
		}

		void Insert(int const index_of_leaf)
		{
			primary_leaves.insert(index_of_leaf);
			//primary_leaves.push_back(index_of_leaf);
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
		}

		// Index into the branch's Leaf set
		std::set<int> primary_leaves;

		void SaveCache()
		{
			primary_leaves_cache.clear();
			primary_leaves_cache.insert(primary_leaves_cache.begin(), primary_leaves.begin(), primary_leaves.end());
		}
		std::vector<int> primary_leaves_cache; // for optimized lookup only

		// Map from child variable group ID to:
		// Map from child leaf index to:
		// index into child variable group's raw data cache (stored in the AllWeightings instance)
		mutable std::map<int, std::map<int, std::int64_t>> child_indices_into_raw_data;

};

typedef PrimaryKeysGroupingMultiplicityGreaterThanOne Leaf;
typedef std::set<Leaf> Leaves;

// "Branch"
class PrimaryKeysGroupingMultiplicityOne : public PrimaryKeysGrouping
{

	public:

		PrimaryKeysGroupingMultiplicityOne()
			: PrimaryKeysGrouping{ DMUInstanceDataVector() }
			, number_branch_combinations{ 0 }
		{}

		PrimaryKeysGroupingMultiplicityOne(DMUInstanceDataVector const & dmuInstanceDataVector)
			: PrimaryKeysGrouping(dmuInstanceDataVector)
			, number_branch_combinations{ 0 }
		{}

		PrimaryKeysGroupingMultiplicityOne(PrimaryKeysGroupingMultiplicityOne const & rhs)
			: PrimaryKeysGrouping(rhs)
			, weighting{ rhs.weighting }
			, hits{ rhs.hits }
			, remaining{ rhs.remaining }
			, number_branch_combinations{ rhs.number_branch_combinations }
			, leaves_cache{ rhs.leaves_cache }
		{}

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
			leaves_cache = rhs.leaves_cache;
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
			hits = rhs.hits;
			remaining = rhs.remaining;
			number_branch_combinations = rhs.number_branch_combinations;
			leaves_cache = rhs.leaves_cache;
			return *this;
		}

		//void ConsolidateHits() const
		//{
		//	std::for_each(hits.begin(), hits.end(), [&](std::pair<boost::multiprecision::cpp_int const, std::set<BranchOutputRow>> & the_hits)
		//	{
		//		std::for_each(the_hits.second.begin(), the_hits.second.end(), [&](BranchOutputRow const & the_hit)
		//		{
		//			// Disabled pending further work to support "random selection of rows WITH removal".
		//			//hits_consolidated.insert(the_hit);
		//		});
		//	});
		//}


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
		// map from time unit to a set of leaf combinations hit for that time units
		//
		mutable std::map<boost::multiprecision::cpp_int, std::set<BranchOutputRow>> hits;
		//
		// ******************************************************************************************************** //
		// ******************************************************************************************************** //


		// Used for optimization purposes only
		mutable std::map<boost::multiprecision::cpp_int, std::vector<BranchOutputRow>> remaining;

		// Commented out for now
		//mutable std::vector<BranchOutputRow> hits_consolidated; // After-the-fact: Merge identical hits across time units within this branch

		// Indices into cached secondary data tables for child groups.
		// 
		// Overview:
		//
		// There is a single top-level primary variable group.
		//
		// Each branch corresponds to a single combination of specific primary key data values
		// ... for those primary keys of multiplicity 1
		// ... for the UOA corresponding to the (single) primary top-level variable group.
		// Each leaf corresponds to a single combination of specific primary key data values
		// ... for those primary keys of multiplicity greater than 1
		// ... for the UOA corresponding to the primary top-level variable group.
		// Each row of output data has one branch, and multiple leaves (one leaf per multiplicity).
		// The set of leaves per row is stored across individual time unit entries within this branch
		// ... (where duplicates can occur, because if the same row (i.e., combination of leaves)
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
		// There can also be child variable groups.
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
		// ... (Multiple *sets* of leaves per row, as opposed to *one* *set* of leaves per row,
		// ... is prohibited by data validation.  The latter would be a scenario such as
		// ... primary UOA = "MID, MID, CTY, CTY" and child UOA = "MID, CTY", a case in which
		// ... both MID and CTY have multiple sets of data for a single row - a prohibited case.)
		// Each such child variable group leaf represents a single set of secondary column data.
		//
		// The actual raw data for the SECONDARY columns - the data values themselves -
		// ... are stored in the AllWeightings' "dataCache" (for primary top-level VG)
		// ... and in the AllWeightings' "secondaryCache" (for non-primary top-level, and child, VG's).

		// **************************************************************************************** //
		// **************************************************************************************** //
		// This data structure is a helper index that maps the *FULL* child DMU set
		// (including the single child leaf, if any)
		// representing an incoming row of data from a child variable group raw data table
		// (including only child primary keys and selected child variables)
		// - this corresponds to incoming child variable group (NOT non-primary top-level variable group) -
		// to output rows for that variable group.
		// The output rows are represented by a map from the actual iterator to a specific output row,
		// to a vector of child leaf numbers in the output row matching the single incoming child leaf.
		// **************************************************************************************** //
		// **************************************************************************************** //
		mutable std::map<ChildDMUInstanceDataVector, std::map<BranchOutputRow const *, std::vector<int>>> helper_lookup__from_child_key_set__to_matching_output_rows;
		void ConstructChildCombinationCache(AllWeightings & allWeightings, std::vector<Leaf> & leaves_cache, int const variable_group_number, std::vector<ChildToPrimaryMapping> mappings_from_child_branch_to_primary = std::vector<ChildToPrimaryMapping>(), std::vector<ChildToPrimaryMapping> mappings_from_child_leaf_to_primary = std::vector<ChildToPrimaryMapping>(), int const number_columns_in_one_child_leaf = 0, bool const force = false) const; // Populate the above data structure

		// *********************************************************************************** //
		// Every branch ALREADY has a std::set<Leaf>,
		// so we're already one-to-one with each branch.
		// but it is NOT stored inside the branch itself.
		// Therefore, we are "only" doubling the memory required 
		// by using this cache.
		// *********************************************************************************** //
		//
		// Cache the leaves for this branch.  This is a CACHE only.
		mutable std::vector<Leaf> leaves_cache;
		void CreateLeafCache(Leaves const & leaves) const
		{
			leaves_cache.clear();
			leaves_cache.insert(leaves_cache.begin(), leaves.cbegin(), leaves.cend());
		}

		mutable boost::multiprecision::cpp_int number_branch_combinations;

};

typedef PrimaryKeysGroupingMultiplicityOne Branch;



// ******************************************************************************************************** //
// ******************************************************************************************************** //
// Each time slice has one of these
// ******************************************************************************************************** //
// (Only one, since currently only one primary top-level variable group is supported)
//
typedef std::map<Branch, Leaves> BranchesAndLeaves;
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
		BranchesAndLeaves branches_and_leaves;
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

		AllWeightings();
		~AllWeightings();

	public:

		// The main time slice data
		TimeSlices timeSlices;
		Weighting weighting; // sum over all time slices

	public:

		// Cache of secondary data: One cache for the primary top-level variable group, and a set of caches for all other variable groups (the non-primary top-level groups, and the child groups)
		DataCache dataCache; // caches secondary key data for the primary variable group, required to create final results in a fashion that can be migrated (partially) to disk via LIFO to support huge monadic input datasets used in the construction of kads
		std::map<int, DataCache> otherTopLevelCache; // Ditto, but for non-primary top-level variable groups
		std::map<int, DataCache> childCache; // Ditto, but for child variable groups

	public:

		sqlite3_stmt * insert_random_sample_stmt;

		bool HandleBranchAndLeaf(Branch const & branch, TimeSliceLeaf & timeSliceLeaf, int const & variable_group_number, VARIABLE_GROUP_MERGE_MODE const merge_mode, std::vector<ChildToPrimaryMapping> mappings_from_child_branch_to_primary = std::vector<ChildToPrimaryMapping>(), std::vector<ChildToPrimaryMapping> mappings_from_child_leaf_to_primary = std::vector<ChildToPrimaryMapping>());
		void CalculateWeightings(int const K, std::int64_t const ms_per_unit_time);
		void PrepareRandomNumbers(int how_many);
		bool RetrieveNextBranchAndLeaves(int const K, Branch & branch, Leaves & leaves, TimeSlice & time_slice, BranchOutputRow & outputRow);
		void PopulateAllLeafCombinations(boost::multiprecision::cpp_int const & which_time_unit, int const K, Branch const & branch, Leaves const & leaves);
		Leaves RetrieveLeafCombinationFromLeafIndices(BranchOutputRow & test_leaf_combination, Leaves const & leaves, int const K);
		void ResetBranchCaches(bool const empty_all = false);
		void ConsolidateHits(); // one-time pass to consolidate hits across time units within branches

	protected:

		bool HandleTimeSliceNormalCase(bool & added, Branch const & branch, TimeSliceLeaf & timeSliceLeaf, TimeSlices::iterator & mapElementPtr, int const & variable_group_number, VARIABLE_GROUP_MERGE_MODE const merge_mode, std::vector<ChildToPrimaryMapping> mappings_from_child_branch_to_primary = std::vector<ChildToPrimaryMapping>(), std::vector<ChildToPrimaryMapping> mappings_from_child_leaf_to_primary = std::vector<ChildToPrimaryMapping>());

		void AddNewTimeSlice(int const & variable_group_number, Branch const & branch, TimeSliceLeaf const &newTimeSliceLeaf);

		// Breaks an existing map entry into two pieces and returns an iterator to both.
		void SliceMapEntry(TimeSlices::iterator const & existingMapElementPtr, std::int64_t const middle, TimeSlices::iterator & newMapElementLeftPtr, TimeSlices::iterator & newMapElementRightPtr);

		// Breaks an existing map entry into three pieces and returns an iterator to the middle piece.
		void SliceMapEntry(TimeSlices::iterator const & existingMapElementPtr, std::int64_t const left, std::int64_t const right, TimeSlices::iterator & newMapElementMiddlePtr);

		// Slices off the left part of the "incoming_slice" TimeSliceLeaf and returns it in the "new_left_slice" TimeSliceLeaf.
		// The "incoming_slice" TimeSliceLeaf is adjusted to become equal to the remaining part on the right.
		void SliceOffLeft(TimeSliceLeaf & incoming_slice, std::int64_t const slicePoint, TimeSliceLeaf & new_left_slice);

		// Merge time slice data into a map element
		bool MergeTimeSliceDataIntoMap(Branch const & branch, TimeSliceLeaf const & timeSliceLeaf, TimeSlices::iterator & mapElementPtr, int const & variable_group_number, VARIABLE_GROUP_MERGE_MODE const merge_mode, std::vector<ChildToPrimaryMapping> mappings_from_child_branch_to_primary = std::vector<ChildToPrimaryMapping>(), std::vector<ChildToPrimaryMapping> mappings_from_child_leaf_to_primary = std::vector<ChildToPrimaryMapping>());

		Leaves GetLeafCombination(boost::multiprecision::cpp_int random_number, int const K, Branch const & branch, Leaves const & leaves);

		static bool is_map_entry_end_time_greater_than_new_time_slice_start_time(TimeSliceLeaf const & new_time_slice_ , TimeSlices::value_type const & map_entry_)
		{

			TimeSlice const & new_time_slice = new_time_slice_.first;
			TimeSlice const & map_entry = map_entry_.first;

			return map_entry.IsEndTimeGreaterThanRhsStartTime(new_time_slice);

		}

		std::set<boost::multiprecision::cpp_int> random_numbers;
		std::set<boost::multiprecision::cpp_int>::const_iterator random_number_iterator;

	private:

		void AddPositionToRemaining(boost::multiprecision::cpp_int const & which_time_unit, std::vector<int> const & position, Branch const & branch);
		bool IncrementPosition(int const K, std::vector<int> & position, Leaves const & leaves);
		int IncrementPositionManageSubK(int const K, int const subK, std::vector<int> & position, Leaves const & leaves);

		boost::multiprecision::cpp_int BinomialCoefficient(int const N, int const K);

};

#endif
