#ifndef TABLETYPES_H
#define TABLETYPES_H

#include <vector>

enum FIELD_TYPE
{
	FIELD_TYPE_UNKNOWN = 0
	, FIELD_TYPE_INT32
	, FIELD_TYPE_INT64
	, FIELD_TYPE_UINT32
	, FIELD_TYPE_UINT64
	, FIELD_TYPE_STRING_FIXED
	, FIELD_TYPE_STRING_VAR
	, FIELD_TYPE_UUOA
};

enum TABLE_TYPES
{

	  TABLE_TYPE_NONE = 0

	, TABLE_TYPE_DMU
	, TABLE_TYPE_CMU
	, TABLE_TYPE_UOA
	, TABLE_TYPE_UOA_ELEMENTS
	, TABLE_TYPE_VARIABLE_GROUP

	, TABLE_TYPE_LAST

};

template<TABLE_TYPES TABLE_TYPE>
struct TableTypeTraits
{
	static std::vector<FIELD_TYPE> const types;
	static std::vector<std::string> const names;
};
template<TABLE_TYPES TABLE_TYPE>
std::vector<FIELD_TYPE> const TableTypeTraits<TABLE_TYPE>::types;
template<TABLE_TYPES TABLE_TYPE>
std::vector<std::string> const TableTypeTraits<TABLE_TYPE>::names;

template<>
std::vector<FIELD_TYPE> const TableTypeTraits<TABLE_TYPE_DMU>::types {FIELD_TYPE_UUOA, FIELD_TYPE_STRING_VAR, FIELD_TYPE_STRING_VAR};
template<>
std::vector<std::string> const TableTypeTraits<TABLE_TYPE_DMU>::names {"DMU_UUID", "DMU" "DESC"};

template<>
std::vector<FIELD_TYPE> const TableTypeTraits<TABLE_TYPE_CMU>::types {FIELD_TYPE_UUOA, FIELD_TYPE_STRING_VAR, FIELD_TYPE_STRING_VAR, FIELD_TYPE_STRING_VAR, FIELD_TYPE_STRING_VAR};
template<>
std::vector<std::string> const TableTypeTraits<TABLE_TYPE_CMU>::names {"CMU_UUID", "CMU", "DESC", "DMU1", "DMU2"};

template<>
std::vector<FIELD_TYPE> const TableTypeTraits<TABLE_TYPE_UOA>::types {FIELD_TYPE_UUOA, FIELD_TYPE_STRING_VAR, FIELD_TYPE_STRING_VAR};
template<>
std::vector<std::string> const TableTypeTraits<TABLE_TYPE_UOA>::names {"UOA_UUID", "UOA", "DESC"};

template<>
std::vector<FIELD_TYPE> const TableTypeTraits<TABLE_TYPE_UOA_ELEMENTS>::types {FIELD_TYPE_UUOA, FIELD_TYPE_UUOA};
template<>
std::vector<std::string> const TableTypeTraits<TABLE_TYPE_UOA_ELEMENTS>::names {"UOA_UUID", "DMU_UUID"};

template<>
std::vector<FIELD_TYPE> const TableTypeTraits<TABLE_TYPE_VARIABLE_GROUP>::types {FIELD_TYPE_UUOA, FIELD_TYPE_STRING_VAR, FIELD_TYPE_STRING_VAR, FIELD_TYPE_UUOA};
template<>
std::vector<std::string> const TableTypeTraits<TABLE_TYPE_VARIABLE_GROUP>::names {"VGP_UUID", "VGP", "DESC", "UOA_UUID"};

#endif
