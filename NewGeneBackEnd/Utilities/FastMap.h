#ifndef _FASTMAP_NEWGENE_H
#define _FASTMAP_NEWGENE_H

#include <utility>
#include <set>
#ifndef Q_MOC_RUN
#	include <boost/pool/pool_alloc.hpp>
#	include <boost/container/flat_map.hpp>
#	include <boost/container/flat_set.hpp>
#endif
#include "../Messager/Messager.h"

// These didn't work out
//#include "MemoryPool/MemoryPool.h"
//#include "NewGeneMemoryPool.h"

#include <map>
#include <vector>
#include <set>

struct remaining_tag {};
struct hits_tag {};
struct hits_consolidated_tag {};


// Vectors

template<typename K>
using FastVector = std::vector<K, boost::pool_allocator<K, boost::default_user_allocator_malloc_free, boost::pool_allocator_tag, boost::details::pool::null_mutex>>;

template<typename K>
using FastVectorCppInt = std::vector<K, boost::pool_allocator<K, boost::default_user_allocator_malloc_free, newgene_cpp_int_tag, boost::details::pool::null_mutex>>;

template<typename K>
using FastVectorRemaining = std::vector<K, boost::pool_allocator<K, boost::default_user_allocator_malloc_free, remaining_tag, boost::details::pool::null_mutex>>;

template<typename K>
using FastVectorHits = std::vector<K, boost::pool_allocator<K, boost::default_user_allocator_malloc_free, hits_tag, boost::details::pool::null_mutex>>;

template<typename K>
using FastVectorHitsConsolidated = std::vector<K, boost::pool_allocator<K, boost::default_user_allocator_malloc_free, hits_consolidated_tag, boost::details::pool::null_mutex>>;

template <typename K, typename MEMORY_TAG>
using FastVectorMemoryTag = std::vector<K, boost::pool_allocator<K, boost::default_user_allocator_malloc_free, MEMORY_TAG, boost::details::pool::null_mutex>>;




// Maps

template<typename K, typename V, class Comp_ = std::less<K>>
using FastMap = std::map<K, V, Comp_, boost::fast_pool_allocator<std::pair<K, V>, boost::default_user_allocator_malloc_free, boost::fast_pool_allocator_tag, boost::details::pool::null_mutex>>;

template<typename K, typename V, class Comp_ = std::less<K>>
using FastMapCppInt = std::map<K, V, Comp_, boost::fast_pool_allocator<std::pair<K, V>, boost::default_user_allocator_malloc_free, newgene_cpp_int_tag, boost::details::pool::null_mutex>>;

template<typename K, typename V, class Comp_ = std::less<K>>
using FastMapRemaining = std::map<K, V, Comp_, boost::fast_pool_allocator<std::pair<K, V>, boost::default_user_allocator_malloc_free, remaining_tag, boost::details::pool::null_mutex>>;

template<typename K, typename V, class Comp_ = std::less<K>>
using FastMapHits = std::map<K, V, Comp_, boost::fast_pool_allocator<std::pair<K, V>, boost::default_user_allocator_malloc_free, hits_tag, boost::details::pool::null_mutex>>;

template<typename K, typename V, class Comp_ = std::less<K>>
using FastMapHitsConsolidated = std::map<K, V, Comp_, boost::fast_pool_allocator<std::pair<K, V>, boost::default_user_allocator_malloc_free, hits_consolidated_tag, boost::details::pool::null_mutex>>;

template<typename K, typename V, typename MEMORY_TAG, class Comp_ = std::less<K>>
using FastMapMemoryTag = std::map<K, V, Comp_, boost::fast_pool_allocator<std::pair<K, V>, boost::default_user_allocator_malloc_free, MEMORY_TAG, boost::details::pool::null_mutex>>;

//template<typename K, typename V, class Comp_ = std::less<K>>
//using FastMap = std::map<K const, V, Comp_>;

template<typename K, typename V, class Comp_ = std::less<K>>
using FastMapLoaded = std::map<K, V, Comp_, boost::fast_pool_allocator<std::pair<K const, V>, boost::default_user_allocator_malloc_free, boost::fast_pool_allocator_tag, boost::details::pool::null_mutex>>;

//template<typename K, typename V, class Comp_ = std::less<K>>
//using FastMap = std::map<K const, V, Comp_, NewGeneMemoryPoolAllocator<std::pair<K const, V>>>;

//template<typename K, typename V, class Comp_ = std::less<K>>
//using FastMap = std::map<K const, V, Comp_, MemoryPool<std::pair<K const, V>>;

template<typename K, typename V, class Comp_ = std::less<K>>
using FastMapFlat =
boost::container::flat_map<K, V, Comp_, boost::fast_pool_allocator<std::pair<K, V>, boost::default_user_allocator_malloc_free, boost::fast_pool_allocator_tag, boost::details::pool::null_mutex>>;



// Sets

template<typename K, class Comp_ = std::less<K>>
using FastSet = std::set<K, Comp_, boost::fast_pool_allocator<K, boost::default_user_allocator_malloc_free, boost::fast_pool_allocator_tag, boost::details::pool::null_mutex>>;

template<typename K, class Comp_ = std::less<K>>
using FastSetCppInt = std::set<K, Comp_, boost::fast_pool_allocator<K, boost::default_user_allocator_malloc_free, newgene_cpp_int_tag, boost::details::pool::null_mutex>>;

template<typename K, class Comp_ = std::less<K>>
using FastSetRemaining = std::set<K, Comp_, boost::fast_pool_allocator<K, boost::default_user_allocator_malloc_free, remaining_tag, boost::details::pool::null_mutex>>;

template<typename K, class Comp_ = std::less<K>>
using FastSetHits = std::set<K, Comp_, boost::fast_pool_allocator<K, boost::default_user_allocator_malloc_free, hits_tag, boost::details::pool::null_mutex>>;

template<typename K, class Comp_ = std::less<K>>
using FastSetHitsConsolidated = std::set<K, Comp_, boost::fast_pool_allocator<K, boost::default_user_allocator_malloc_free, hits_consolidated_tag, boost::details::pool::null_mutex>>;

template<typename K, typename MEMORY_TAG, class Comp_ = std::less<K>>
using FastSetMemoryTag = std::set<K, Comp_, boost::fast_pool_allocator<K, boost::default_user_allocator_malloc_free, MEMORY_TAG, boost::details::pool::null_mutex>>;

//template<typename K, class Comp_ = std::less<K>>
//using FastSet = std::set<K const, Comp_>;

//template<typename K, class Comp_ = std::less<K>>
//using FastSetLoaded = std::set<K const, Comp_, boost::fast_pool_allocator<K, boost::default_user_allocator_malloc_free, boost::fast_pool_allocator_tag, boost::details::pool::null_mutex>>;

//template<typename K, class Comp_ = std::less<K>>
//using FastSet = std::set<K const, Comp_, NewGeneMemoryPoolAllocator<K>>;

//template<typename K, class Comp_ = std::less<K>>
//using FastSet = std::set<K const, Comp_, MemoryPool<K>>;

template<typename K, class Comp_ = std::less<K>>
using FastSetFlat = boost::container::flat_set<K, Comp_, boost::fast_pool_allocator<K, boost::default_user_allocator_malloc_free, boost::fast_pool_allocator_tag, boost::details::pool::null_mutex>>;

#endif
