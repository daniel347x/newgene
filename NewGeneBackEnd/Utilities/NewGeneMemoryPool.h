#ifndef _MEMORYPOOL_NEWGENE_H
#define _MEMORYPOOL_NEWGENE_H

#include <map>
#include <cstdint>
#ifndef Q_MOC_RUN
#	include <boost/container/flat_map.hpp>
#	include <boost/container/flat_set.hpp>
#	include <boost/format.hpp>
#endif
#include <stdlib.h>
#include <time.h>
#include "NewGeneException.h"

#define MAX_NUMBER_BLOCKS 1000
#define BLOCK_ITEM_COUNT 4096
#define BYTEBITS_PER_BLOCK 512
#define FREE_WALK_MAX_STEPS 5
#define MAX_ITEM_SIZE_IN_BYTES 256

	class NewGenePool
	{

	public:

		static NewGenePool * getInstance(std::int32_t const theSize)
		{
			return existingMaps[theSize / 4 - 1];
		}

		static void InitializePools()
		{
			for (int n = 1; n <= MAX_ITEM_SIZE_IN_BYTES / 4; ++n)
			{
				existingMaps[n-1] = new NewGenePool(n * 4);
			}
		}

		static void ClearAllPools()
		{
			for (int n = 0; n < MAX_ITEM_SIZE_IN_BYTES / 4; ++n)
			{
				delete existingMaps[n];
			}
		}

		void deallocate(char * const ptr, size_t n)
		{
			if (n > 1)
			{
				boost::format msg("Cannot deallocate more than one item at a time!");
				throw NewGeneException() << newgene_error_description(msg.str());
			}
			if ((ptr >= previous_block_holding_deleted_item) && (ptr - previous_block_holding_deleted_item) / mySize < BLOCK_ITEM_COUNT)
			{
				// The current item being deleted is in the same block as the previous that was deleted
				previous_index_to_deleted_item = (ptr - previous_block_holding_deleted_item) / mySize;
				blockbits[previous_block_holding_deleted_item_index * BYTEBITS_PER_BLOCK + previous_index_to_deleted_item / 8] &= ~(static_cast<char>(0x01 << (previous_index_to_deleted_item % 8)));
			}
			else
			{
				// We have to find the block from which to delete
				auto found_block = std::lower_bound(&blocks[0], &blocks[current_block+1], ptr, [&](char * const & existing_test, char * const existing)
				{
					if ((existing >= existing_test) && (existing - existing_test) / mySize < BLOCK_ITEM_COUNT)
					{
						return true;
					}
					return false;
				});

				if (*found_block == nullptr || (ptr < *found_block) || (ptr - *found_block) / mySize >= BLOCK_ITEM_COUNT)
				{
					boost::format msg("Item being deallocated cannot be found!");
					throw NewGeneException() << newgene_error_description(msg.str());
				}

				previous_block_holding_deleted_item = *found_block;
				previous_index_to_deleted_item = (ptr - previous_block_holding_deleted_item) / mySize;
				previous_block_holding_deleted_item_index = blocks_sorted[previous_block_holding_deleted_item];
			}
		}

		char * allocate(size_t n)
		{
			static char * prev = nullptr;
			if (n > 1)
			{
				boost::format msg("Cannot allocate more than one item at a time!");
				throw NewGeneException() << newgene_error_description(msg.str());
			}
			char * ptr = CheckReturnFreeSlot();
			if (ptr)
			{
				if (prev == ptr)
				{
					int m = 0;
				}
				prev = ptr;
				return ptr; 
			}
			else
			{
				// take a few steps and see if there's a nearby match...
				for (int f = 0; f < FREE_WALK_MAX_STEPS; ++f)
				{
					++previous_index_to_deleted_item;
					if (previous_index_to_deleted_item == BLOCK_ITEM_COUNT)
					{
						previous_index_to_deleted_item = 0;
					}
					char * ptr = CheckReturnFreeSlot();
					if (ptr)
					{
						if (prev == ptr)
						{
							int m = 0;
						}
						prev = ptr;
						return ptr;
					}
				}
				// jump to a random block - defies any pattern of memory usage by an application
				previous_block_holding_deleted_item_index = rand() % (highest_block_index+1);
				previous_index_to_deleted_item = rand() % BLOCK_ITEM_COUNT;
				// again, take a few steps and see if there's a nearby match...
				for (int f = 0; f < FREE_WALK_MAX_STEPS; ++f)
				{
					++previous_index_to_deleted_item;
					if (previous_index_to_deleted_item == BLOCK_ITEM_COUNT)
					{
						previous_index_to_deleted_item = 0;
					}
					char * ptr = CheckReturnFreeSlot();
					if (ptr)
					{
						if (prev == ptr)
						{
							int m = 0;
						}
						prev = ptr;
						return ptr;
					}
				}
			}
			// No luck.  Add new item at end
			ptr = blocks[current_block] + current_block_available_index * mySize;
			++current_block_available_index;
			if (current_block_available_index == BLOCK_ITEM_COUNT)
			{
				++current_block;
				if (current_block == MAX_NUMBER_BLOCKS)
				{
					boost::format msg("Exceeded maximum number of blocks!");
					throw NewGeneException() << newgene_error_description(msg.str());
				}
				blocks[current_block] = new char[BLOCK_ITEM_COUNT * mySize];
				blocks_sorted[blocks[current_block]] = current_block; // reverse lookup to find the block index from the pointer to the data for the block
				++highest_block_index;
				current_block_available_index = 0;
			}
			if (ptr)
			{
				if (prev == ptr)
				{
					int m = 0;
				}
				prev = ptr;
				return ptr;
			}
			return ptr;
		}

	private:

		NewGenePool(std::int32_t const theSize)
			: mySize{ theSize }
			, current_block{ 0 }
			, current_block_available_index{ 0 }
			, previous_block_holding_deleted_item{ nullptr }
			, previous_block_holding_deleted_item_index{ 0 }
			, previous_index_to_deleted_item{ 0 }
			, highest_block_index{ 0 }
		{
			srand(static_cast<unsigned int>(time(NULL)));
			memset(blocks, '\0', MAX_NUMBER_BLOCKS + 1); // space for "end" block pointer which is always NULL
			memset(blockbits, '\0', BYTEBITS_PER_BLOCK * MAX_NUMBER_BLOCKS);
			blocks[current_block] = new char[BLOCK_ITEM_COUNT * mySize];
			previous_block_holding_deleted_item = blocks[current_block];
			blocks_sorted[blocks[current_block]] = current_block;
		}

		~NewGenePool()
		{
			for (std::int32_t n = 0; n < MAX_NUMBER_BLOCKS; ++n)
			{
				if (blocks[n])
				{
					delete [] blocks[n];
					blocks[n] = '\0';
				}
			}
		}

		char * NewGenePool::CheckReturnFreeSlot();

		std::int32_t mySize;
		char * blocks[MAX_NUMBER_BLOCKS + 1]; // space for "end" block pointer which is always NULL
		char blockbits[BYTEBITS_PER_BLOCK * MAX_NUMBER_BLOCKS];
		std::map<char*, std::int32_t> blocks_sorted;
		std::int32_t current_block;
		std::int32_t current_block_available_index;

		std::int32_t previous_block_holding_deleted_item_index;
		char * previous_block_holding_deleted_item;
		std::int32_t previous_index_to_deleted_item;

		std::int32_t highest_block_index;

		//static std::map<std::int32_t, NewGenePool *> existingMaps;
		static NewGenePool * existingMaps[MAX_ITEM_SIZE_IN_BYTES/4];

	};

	template <typename T>
	class NewGeneMemoryPoolAllocator
	{
	public:
		typedef T value_type;
		typedef value_type * pointer;
		typedef const value_type * const_pointer;
		typedef value_type & reference;
		typedef const value_type & const_reference;
		typedef typename size_t size_type;
		typedef typename std::int64_t difference_type;

	public:

		NewGeneMemoryPoolAllocator()
		{
			if (sizeof(T) > MAX_ITEM_SIZE_IN_BYTES)
			{
				boost::format msg("Item size too large!");
				throw NewGeneException() << newgene_error_description(msg.str());
			}
		}

		template <typename U>
		NewGeneMemoryPoolAllocator(const NewGeneMemoryPoolAllocator<U> &)
		{
		}

		template <typename U>
		struct rebind
		{
			typedef NewGeneMemoryPoolAllocator<U> other;
		};

		static pointer address(reference r)
		{
			return &r;
		}

		static const_pointer address(const_reference s)
		{
			return &s;
		}

		static size_type max_size()
		{
			return (std::numeric_limits<size_type>::max)();
		}

		static void construct(const pointer ptr, const value_type & t)
		{
			new (ptr)T(t);
		}

		static void destroy(const pointer ptr)
		{
			ptr->~T();
			(void)ptr; // avoid unused variable warning.
		}

		bool operator==(const NewGeneMemoryPoolAllocator &) const
		{
			return true;
		}

		bool operator!=(const NewGeneMemoryPoolAllocator &) const
		{
			return false;
		}

		static pointer allocate(const size_type n)
		{
			const pointer ret = reinterpret_cast<pointer>(NewGenePool::getInstance(sizeof(T))->allocate(n));
			if ((ret == 0) && n)
			{
				boost::format msg("Unable to obtain memory!");
				throw NewGeneException() << newgene_error_description(msg.str());
			}
			return ret;
		}

		static pointer allocate(const size_type n, const void * const)
		{
			return allocate(n);
		}

		static void deallocate(const pointer ptr, const size_type n)
		{
			NewGenePool::getInstance(sizeof(T))->deallocate(reinterpret_cast<char *const>(ptr), n);
		}

	};

	template<>
	class NewGeneMemoryPoolAllocator<void>
	{
	public:
		typedef void*       pointer;
		typedef const void* const_pointer;
		typedef void        value_type;

		//! \brief Nested class rebind allows for transformation from
		//! fast_pool_allocator<T> to fast_pool_allocator<U>.
		//!
		//! Nested class rebind allows for transformation from
		//! fast_pool_allocator<T> to fast_pool_allocator<U> via the member
		//! typedef other.
		template <class U> struct rebind
		{
			typedef NewGeneMemoryPoolAllocator<U> other;
		};
	};


#endif
