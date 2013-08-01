#ifndef TABLETYPES_H
#define TABLETYPES_H

#include <cstdint>
#include <vector>

enum TABLE_TYPES
{

	  TABLE_TYPE_NONE = 0

	// Input model tables
	, TABLE__DMU_IDENTIFIER
	, TABLE__DMU_INSTANCE
	, TABLE__CMU_IDENTIFIER
	, TABLE__CMU_INSTANCE
	, TABLE__UOA_IDENTIFIER
	, TABLE__UOA_MEMBER
	, TABLE__VG_CATEGORY
	, TABLE__VG_SET_MEMBER

	// Raw input data tables
	, TABLE__VG_INPUT_DATA // special case
	, TABLE__VG_INPUT_DATA_METADATA

	// Output model tables
	, TABLE__VG_SET_MEMBER_SELECTED
	, TABLE__KAD_COUNT

	, TABLE_TYPE_LAST

};

#endif
