#ifndef _FASTMAP_NEWGENE_H
#define _FASTMAP_NEWGENE_H

#include <utility>
#include <set>
#ifndef Q_MOC_RUN
#	include <boost/pool/pool_alloc.hpp>
#	include <boost/container/flat_map.hpp>
#	include <boost/container/flat_set.hpp>
#endif

//#include "MemoryPool/MemoryPool.h"
//#include "NewGeneMemoryPool.h"

#include <map>
#include <vector>
#include <set>

template<typename K>
using FastVector = std::vector<K, boost::pool_allocator<K, boost::default_user_allocator_malloc_free, boost::details::pool::null_mutex>>;

template<typename K, typename V, class Comp_ = std::less<K>>
using FastMap = std::map<K, V, Comp_, boost::fast_pool_allocator<std::pair<K const, V>, boost::default_user_allocator_malloc_free, boost::details::pool::null_mutex>>;

template<typename K, typename V, class Comp_ = std::less<K>>
using FastMap4096 = std::map<K, V, Comp_, boost::fast_pool_allocator<std::pair<K const, V>, boost::default_user_allocator_malloc_free, boost::details::pool::null_mutex, 4194304, 4194304>>;

//template<typename K, typename V, class Comp_ = std::less<K>>
//using FastMap = std::map<K, V, Comp_, NewGeneMemoryPoolAllocator<std::pair<K const, V>>>;

//template<typename K, typename V, class Comp_ = std::less<K>>
//using FastMap = std::map<K, V, Comp_, MemoryPool<std::pair<K const, V>>;

template<typename K, typename V, class Comp_ = std::less<K>>
using FastMapFlat = boost::container::flat_map<K, V, Comp_, boost::fast_pool_allocator<std::pair<K const, V>, boost::default_user_allocator_malloc_free, boost::details::pool::null_mutex>>;

template<typename K, class Comp_ = std::less<K>>
using FastSet = std::set<K, Comp_, boost::fast_pool_allocator<K, boost::default_user_allocator_malloc_free, boost::details::pool::null_mutex>>;

template<typename K, class Comp_ = std::less<K>>
using FastSet4096 = std::set<K, Comp_, boost::fast_pool_allocator<K, boost::default_user_allocator_malloc_free, boost::details::pool::null_mutex, 1048576, 1048576>>;

//template<typename K, class Comp_ = std::less<K>>
//using FastSet = std::set<K, Comp_, NewGeneMemoryPoolAllocator<K>>;

//template<typename K, class Comp_ = std::less<K>>
//using FastSet = std::set<K, Comp_, MemoryPool<K>>;

template<typename K, class Comp_ = std::less<K>>
using FastSetFlat = boost::container::flat_set<K, Comp_, boost::fast_pool_allocator<K, boost::default_user_allocator_malloc_free, boost::details::pool::null_mutex>>;



// Deprecated and unused

template<typename K_, typename V_, class Comp_ = std::less<K_>>
class FastMap_
{

public:

	typedef std::pair<K_ const, V_> element_type;

	struct MyMutableKeyValue
	{
		MyMutableKeyValue(K_ const & key, V_ const & value)
		: me(key, value)
		{}
		mutable element_type me;
		K_ const & first() const { return me.first;  }
		V_ & second() const { return me.second; }
	};

	struct FastMapComparator
	{
		bool operator()(MyMutableKeyValue const & lhs, MyMutableKeyValue const & rhs)
		{
			return Comp_()(lhs.first, rhs.first);
		}
	};

	//typedef std::set<MyMutableKeyValue, FastMapComparator, boost::fast_pool_allocator<MyMutableKeyValue, boost::default_user_allocator_malloc_free>> FastSet;
	typedef std::set<MyMutableKeyValue, FastMapComparator, boost::fast_pool_allocator<MyMutableKeyValue>> FastSet;

	FastSet fastSet;

	typedef typename FastSet::iterator iterator;
	typedef typename FastSet::const_iterator const_iterator;
	typedef typename FastSet::reverse_iterator reverse_iterator;
	typedef typename FastSet::const_reverse_iterator const_reverse_iterator;
	typedef MyMutableKeyValue value_type;

	iterator begin() { return fastSet.begin(); }
	iterator end() { return fastSet.end(); }
	const_iterator begin() const { return fastSet.begin(); }
	const_iterator end() const { return fastSet.end(); }
	const_iterator cbegin() const { return fastSet.cbegin(); }
	const_iterator cend() const { return fastSet.cend(); }
	reverse_iterator rbegin() { return fastSet.rbegin(); }
	reverse_iterator rend() { return fastSet.rend(); }
	const_reverse_iterator rbegin() const { return fastSet.rbegin(); }
	const_reverse_iterator rend() const { return fastSet.rend(); }
	const_reverse_iterator crbegin() const { return fastSet.crbegin(); }
	const_reverse_iterator crend() const { return fastSet.crend(); }

	void clear()
	{
		fastSet.clear();
	}

	bool empty()
	{
		return fastSet.empty();
	}

	size_t size() const
	{
		return fastSet.size();
	}

	iterator find(K_ const & key)
	{
		auto found = std::lower_bound(fastSet.begin(), fastSet.end(), key, [&](MyMutableKeyValue const & element, K_ const & key_)
		{
			return Comp_()(element.first, key);
		});
		if (found == fastSet.end())
		{
			return found;
		}
		if (!Comp_()(found->first, key) && !Comp_()(key, found->first))
		{
			return found;
		}
		return fastSet.end();
	}

	const_iterator find(K_ const & key) const
	{
		auto found = std::lower_bound(fastSet.cbegin(), fastSet.cend(), key, [&](MyMutableKeyValue const & element, K_ const & key_)
		{
			return Comp_()(element.first, key);
		});
		if (found == fastSet.cend())
		{
			return found;
		}
		if (!Comp_()(found->first, key) && !Comp_()(key, found->first))
		{
			return found;
		}
		return fastSet.cend();
	}

	iterator erase(const_iterator position)
	{
		return fastSet.erase(position);
	}

	iterator erase(const_iterator first, const_iterator last)
	{
		return fastSet.erase(first, last);
	}

	V_ & operator[](K_ const & key)
	{
		auto found = find(key);
		if (found == fastSet.end())
		{
			// item does not yet exist, so create one
			auto inserted = fastSet.emplace(key, element_type::second_type());
			return (inserted.first)->second;
		}
		return found->second;
	}

};

#endif
