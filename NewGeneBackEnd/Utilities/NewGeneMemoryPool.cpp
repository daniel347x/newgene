#include "NewGeneMemoryPool.h"

NewGenePool * NewGenePool::existingMaps[MAX_ITEM_SIZE_IN_BYTES/4];

char * NewGenePool::CheckReturnFreeSlot()
{

	if ((blockbits[previous_block_holding_deleted_item_index + previous_index_to_deleted_item / 8] & (static_cast<char>((0x01 << (previous_index_to_deleted_item % 8))))) != 0)
	{
		// The bit is set: it is not a free slot
		return nullptr;
	}

	// The bit is unset: it is a free slot.  Set slot to not free, and return pointer.
	blockbits[previous_block_holding_deleted_item_index + previous_index_to_deleted_item / 8] |= static_cast<char>((0x01 << (previous_index_to_deleted_item % 8)));
	char * ret = previous_block_holding_deleted_item + previous_index_to_deleted_item * mySize;
	++previous_index_to_deleted_item;
	if (previous_index_to_deleted_item == BLOCK_ITEM_COUNT)
	{
		previous_index_to_deleted_item = 0;
	}
	return ret;

}
