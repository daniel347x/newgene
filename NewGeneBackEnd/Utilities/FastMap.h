#ifndef _FASTMAP_NEWGENE_H
#define _FASTMAP_NEWGENE_H

#ifndef Q_MOC_RUN
	#include <boost/pool/pool_alloc.hpp>
	#include <boost/container/flat_map.hpp>
	#include <boost/container/flat_set.hpp>
#endif
#include <utility>
#include <set>
#include "../Messager/Messager.h"

// These didn't work out
//#include "MemoryPool/MemoryPool.h"
//#include "NewGeneMemoryPool.h"

#include <map>
#include <vector>
#include <set>
#include <list>

struct remaining_tag {};
struct hits_tag {};
struct hits_consolidated_tag {};
struct saved_historic_rows_tag {};
struct ongoing_merged_rows_tag {};
struct ongoing_consolidation_tag {};
struct child_dmu_lookup_tag {};
struct calculate_consolidated_total_number_rows_tag {};

// Vectors

typedef std::vector<newgene_random_cpp_int, boost::pool_allocator<newgene_random_cpp_int, boost::default_user_allocator_malloc_free, newgene_cpp_int_random_tag, boost::details::pool::null_mutex>>
		FastVectorCppInt;

template <typename K, typename MEMORY_TAG>
using FastVectorMemoryTag = std::vector<K, boost::pool_allocator<K, boost::default_user_allocator_malloc_free, MEMORY_TAG, boost::details::pool::null_mutex>>;



// Lists

template <typename K, typename MEMORY_TAG>
using FastListMemoryTag = std::list<K, boost::fast_pool_allocator<K, boost::default_user_allocator_malloc_free, MEMORY_TAG, boost::details::pool::null_mutex>>;



// Maps

//template<typename K, typename V, class Comp_ = std::less<K>>
//using FastMapCppInt = std::map<K, V, Comp_, boost::fast_pool_allocator<std::pair<K, V>, boost::default_user_allocator_malloc_free, newgene_cpp_int_tag, boost::details::pool::null_mutex>>;

template<typename K, typename V, typename MEMORY_TAG, class Comp_ = std::less<K>>
using FastMapMemoryTag =
	std::map<K, V, Comp_, boost::fast_pool_allocator<std::pair<K, V>, boost::default_user_allocator_malloc_free, MEMORY_TAG, boost::details::pool::null_mutex>>;

//template<typename K, typename V, class Comp_ = std::less<K>>
//using FastMapFlat =
//boost::container::flat_map<K, V, Comp_, boost::fast_pool_allocator<std::pair<K, V>, boost::default_user_allocator_malloc_free, boost::fast_pool_allocator_tag, boost::details::pool::null_mutex>>;



// Sets

typedef std::set<newgene_random_cpp_int, std::less<newgene_cpp_int>, boost::fast_pool_allocator<newgene_cpp_int, boost::default_user_allocator_malloc_free, newgene_cpp_int_random_tag, boost::details::pool::null_mutex>>
		FastSetCppInt;

template<typename K, typename MEMORY_TAG, class Comp_ = std::less<K>>
using FastSetMemoryTag = std::set<K, Comp_, boost::fast_pool_allocator<K, boost::default_user_allocator_malloc_free, MEMORY_TAG, boost::details::pool::null_mutex>>;

//template<typename K, class Comp_ = std::less<K>>
//using FastSetFlat = boost::container::flat_set<K, Comp_, boost::fast_pool_allocator<K, boost::default_user_allocator_malloc_free, boost::fast_pool_allocator_tag, boost::details::pool::null_mutex>>;

#endif
