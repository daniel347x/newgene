#ifndef FIELDTYPES_H
#define FIELDTYPES_H

#include <cstdint>
#include <string>
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

class NameOrIndex
{

	public:

		enum NAME_OR_INDEX
		{
			  NAME
			, INDEX
		};

		NameOrIndex(NAME_OR_INDEX const name_or_index_, int const index_)
			: name_or_index(name_or_index_)
			, index(index)
		{
		}

		NameOrIndex(NAME_OR_INDEX const name_or_index_, std::string const name_)
			: name_or_index(name_or_index_)
			, name(name_)
			, index(-1)
		{
		}

		NAME_OR_INDEX name_or_index;
		int index;
		std::string name;

};

typedef std::pair<NameOrIndex, FIELD_TYPE> FieldTypeEntry;
typedef std::vector<FieldTypeEntry> FieldTypeEntries;

#endif
