#ifndef _FASTMAP_NEWGENE_H
#define _FASTMAP_NEWGENE_H

#include <utility>
#include <set>
#ifndef Q_MOC_RUN
#	include <boost/pool/pool_alloc.hpp>
#	include <boost/container/flat_map.hpp>
#	include <boost/container/flat_set.hpp>
#endif

// These didn't work out
//#include "MemoryPool/MemoryPool.h"
//#include "NewGeneMemoryPool.h"

#include <map>
#include <vector>
#include <set>

template<typename K>
using FastVector = std::vector<K, boost::pool_allocator<K, boost::default_user_allocator_malloc_free, boost::details::pool::null_mutex>>;

template<typename K, typename V, class Comp_ = std::less<K>>
using FastMap = std::map<K, V, Comp_, boost::fast_pool_allocator<std::pair<K const, V>, boost::default_user_allocator_malloc_free, boost::details::pool::null_mutex>>;

//template<typename K, typename V, class Comp_ = std::less<K>>
//using FastMap = std::map<K, V, Comp_>;

template<typename K, typename V, class Comp_ = std::less<K>>
using FastMapLoaded = std::map<K, V, Comp_, boost::fast_pool_allocator<std::pair<K const, V>, boost::default_user_allocator_malloc_free, boost::details::pool::null_mutex>>;
//using FastMap4096 = std::map<K, V, Comp_, boost::fast_pool_allocator<std::pair<K const, V>, boost::default_user_allocator_malloc_free, boost::details::pool::null_mutex, 4194304, 4194304>>;

//template<typename K, typename V, class Comp_ = std::less<K>>
//using FastMap = std::map<K, V, Comp_, NewGeneMemoryPoolAllocator<std::pair<K const, V>>>;

//template<typename K, typename V, class Comp_ = std::less<K>>
//using FastMap = std::map<K, V, Comp_, MemoryPool<std::pair<K const, V>>;

template<typename K, typename V, class Comp_ = std::less<K>>
using FastMapFlat =
	boost::container::flat_map<K, V, Comp_, boost::fast_pool_allocator<std::pair<K const, V>, boost::default_user_allocator_malloc_free, boost::details::pool::null_mutex>>;

template<typename K, class Comp_ = std::less<K>>
using FastSet = std::set<K, Comp_, boost::fast_pool_allocator<K, boost::default_user_allocator_malloc_free, boost::details::pool::null_mutex>>;

//template<typename K, class Comp_ = std::less<K>>
//using FastSet = std::set<K, Comp_>;

//template<typename K, class Comp_ = std::less<K>>
//using FastSetLoaded = std::set<K, Comp_, boost::fast_pool_allocator<K, boost::default_user_allocator_malloc_free, boost::details::pool::null_mutex>>;

//template<typename K, class Comp_ = std::less<K>>
//using FastSet = std::set<K, Comp_, NewGeneMemoryPoolAllocator<K>>;

//template<typename K, class Comp_ = std::less<K>>
//using FastSet = std::set<K, Comp_, MemoryPool<K>>;

template<typename K, class Comp_ = std::less<K>>
using FastSetFlat = boost::container::flat_set<K, Comp_, boost::fast_pool_allocator<K, boost::default_user_allocator_malloc_free, boost::details::pool::null_mutex>>;

#endif
