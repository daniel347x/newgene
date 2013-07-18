#ifndef TABLETYPES_H
#define TABLETYPES_H

#include <cstdint>
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
	, FIELD_TYPE_TIMESTAMP
	, FIELD_TYPE_UUID
	, FIELD_TYPE_UUID_FOREIGN
	, FIELD_TYPE_STRING_CODE
	, FIELD_TYPE_STRING_LONGHAND
	, FIELD_TYPE_TIME_RANGE
	, FIELD_TYPE_NOTES_1
	, FIELD_TYPE_NOTES_2
	, FIELD_TYPE_NOTES_3
	, FIELD_TYPE_FIELD_TYPE // Yes, a field type defined to represent a field type (must not itself be "FIELD_TYPE_FIELD_TYPE" or that would be circular)
};

template<FIELD_TYPE THE_FIELD_TYPE>
struct FieldTypeTraits
{
	typedef void * type;
};

template<>
struct FieldTypeTraits<FIELD_TYPE_INT32>
{
	typedef std::int32_t type;
};

template<>
struct FieldTypeTraits<FIELD_TYPE_INT64>
{
	typedef std::int64_t type;
};

template<>
struct FieldTypeTraits<FIELD_TYPE_UINT32>
{
	typedef std::uint32_t type;
};

template<>
struct FieldTypeTraits<FIELD_TYPE_UINT64>
{
	typedef std::uint64_t type;
};

template<>
struct FieldTypeTraits<FIELD_TYPE_STRING_FIXED>
{
	// TODO: make this somehow fixed size at initialization?
	typedef std::string type;
};

template<>
struct FieldTypeTraits<FIELD_TYPE_STRING_VAR>
{
	typedef std::string type;
};

template<>
struct FieldTypeTraits<FIELD_TYPE_TIMESTAMP>
{
	typedef std::uint64_t type;
};

template<>
struct FieldTypeTraits<FIELD_TYPE_UUID>
{
	typedef std::string type;
};

template<>
struct FieldTypeTraits<FIELD_TYPE_UUID_FOREIGN>
{
	typedef std::string type;
};

template<>
struct FieldTypeTraits<FIELD_TYPE_STRING_CODE>
{
	typedef std::string type;
};

template<>
struct FieldTypeTraits<FIELD_TYPE_STRING_LONGHAND>
{
	typedef std::string type;
};

template<>
struct FieldTypeTraits<FIELD_TYPE_TIME_RANGE>
{
	typedef std::string type;
};

template<>
struct FieldTypeTraits<FIELD_TYPE_NOTES_1>
{
	typedef std::string type;
};

template<>
struct FieldTypeTraits<FIELD_TYPE_NOTES_2>
{
	typedef std::string type;
};

template<>
struct FieldTypeTraits<FIELD_TYPE_NOTES_3>
{
	typedef std::string type;
};

template<>
struct FieldTypeTraits<FIELD_TYPE_FIELD_TYPE>
{
	typedef std::string type;
};


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

	// Output model tables
	, TABLE__VG_SET_MEMBER_SELECTED
	, TABLE__KAD_COUNT

	, TABLE_TYPE_LAST

};

#endif
