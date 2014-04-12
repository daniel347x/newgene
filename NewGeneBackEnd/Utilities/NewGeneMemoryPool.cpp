#include "NewGeneMemoryPool.h"

NewGenePool * NewGenePool::existingMaps[MAX_ITEM_SIZE_IN_BYTES/4];

char * NewGenePool::CheckReturnFreeSlot()
{

	if ((blockbits[previous_block_holding_deleted_item_index * BYTEBITS_PER_BLOCK + previous_index_to_deleted_item / 8] & (static_cast<char>((0x01 << (previous_index_to_deleted_item % 8))))) != 0)
	{
		// The bit is set: it is not a free slot
		return nullptr;
	}

	// The bit is unset: it is a free slot.  Set slot to not free, and return pointer.
	blockbits[previous_block_holding_deleted_item_index * BYTEBITS_PER_BLOCK + previous_index_to_deleted_item / 8] |= static_cast<char>((0x01 << (previous_index_to_deleted_item % 8)));
	char * ret = previous_block_holding_deleted_item + previous_index_to_deleted_item * mySize;
	++previous_index_to_deleted_item;
	if (previous_index_to_deleted_item == BLOCK_ITEM_COUNT)
	{
		previous_index_to_deleted_item = 0;
	}
	--free_slots[previous_block_holding_deleted_item_index];
	return ret;

}

char * NewGenePool::CheckReturnFreeSlotCurrent()
{

	if ((blockbits[current_block_index * BYTEBITS_PER_BLOCK + current_block_available_index / 8] & (static_cast<char>((0x01 << (current_block_available_index % 8))))) != 0)
	{
		// The bit is set: it is not a free slot
		return nullptr;
	}

	// The bit is unset: it is a free slot.  Set slot to not free, and return pointer.
	blockbits[current_block_index * BYTEBITS_PER_BLOCK + current_block_available_index / 8] |= static_cast<char>((0x01 << (current_block_available_index % 8)));
	char * ret = blocks[current_block_index] + current_block_available_index * mySize;
	++current_block_available_index;
	if (current_block_available_index == BLOCK_ITEM_COUNT)
	{
		current_block_available_index = 0;
	}
	--free_slots[current_block_index];
	return ret;

}
