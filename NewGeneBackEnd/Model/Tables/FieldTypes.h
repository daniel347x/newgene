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

	, FIELD_TYPE_FLOAT

	// Special - typically for internal NewGene use, and not available in the user interface directly,
	// but only as translated from user interface options such as "primary key" being selected for a column
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

	// DMU's
	, FIELD_TYPE_DMU_MEMBER_UUID
	, FIELD_TYPE_DMU_MEMBER_UUID_NUMERIC
	, FIELD_TYPE_DMU_MEMBER_UUID_STRING
	, FIELD_TYPE_DMU_MEMBER_CODE
	, FIELD_TYPE_DMU_MEMBER_DESCRIPTION
	, FIELD_TYPE_FK_TO_DMU_CATEGORY_UUID

	// Time range
	, FIELD_TYPE_TIME_RANGE_OUTPUT_START_DATETIME
	, FIELD_TYPE_TIME_RANGE_OUTPUT_END_DATETIME

	, FIELD_TYPE_DAY
	, FIELD_TYPE_MONTH
	, FIELD_TYPE_YEAR
	, FIELD_TYPE_TIMERANGE_STRING

	, FIELD_TYPE_DMU_PRIMARY_KEY_AND_DAY
	, FIELD_TYPE_DMU_PRIMARY_KEY_AND_MONTH
	, FIELD_TYPE_DMU_PRIMARY_KEY_AND_YEAR
	, FIELD_TYPE_DMU_PRIMARY_KEY_AND_TIMERANGE_STRING

};

bool IsFieldTypeInt32(FIELD_TYPE const & field_type)
{

	bool returnVal = false;

	switch (field_type)
	{

		case FIELD_TYPE_INT32:
		case FIELD_TYPE_UINT32:
		case FIELD_TYPE_DMU_MEMBER_UUID_NUMERIC:
		case FIELD_TYPE_DAY:
		case FIELD_TYPE_MONTH:
		case FIELD_TYPE_YEAR:
		case FIELD_TYPE_DMU_PRIMARY_KEY_AND_DAY:
		case FIELD_TYPE_DMU_PRIMARY_KEY_AND_MONTH:
		case FIELD_TYPE_DMU_PRIMARY_KEY_AND_YEAR:
		{
			returnVal = true;
		}
		break;

		default:
		{
			returnVal = false;
		}
		break;

	}

	return returnVal;

}

bool IsFieldTypeInt64(FIELD_TYPE const & field_type)
{

	bool returnVal = false;

	switch (field_type)
	{

		case FIELD_TYPE_INT64:
		case FIELD_TYPE_UINT64:
		case FIELD_TYPE_TIMESTAMP:
		case FIELD_TYPE_TIME_RANGE_OUTPUT_START_DATETIME:
		case FIELD_TYPE_TIME_RANGE_OUTPUT_END_DATETIME:
		{
			returnVal = true;
		}
		break;

		default:
		{
			returnVal = false;
		}
		break;

	}

	return returnVal;

}

bool IsFieldTypeInt(FIELD_TYPE const & field_type)
{

	bool returnVal = false;

	if (IsFieldTypeInt32(field_type))
	{
		returnVal = true;
	}

	if (IsFieldTypeInt64(field_type))
	{
		returnVal = true;
	}

	return returnVal;

}

bool IsFieldTypeFloat(FIELD_TYPE const & field_type)
{

	bool returnVal = false;

	switch (field_type)
	{

		case FIELD_TYPE_FLOAT:
		{
			returnVal = true;
		}
		break;

		default:
		{
			returnVal = false;
		}
		break;

	}

	return returnVal;

}

bool IsFieldTypeString(FIELD_TYPE const & field_type)
{

	bool returnVal = true;

	if (IsFieldTypeInt(field_type))
	{
		returnVal = false;
	}
	if (IsFieldTypeFloat(field_type))
	{
		returnVal = false;
	}

	return returnVal;

}

template<FIELD_TYPE THE_FIELD_TYPE>
struct FieldTypeTraits
{
	typedef void * type;
	static type const default_;
};
template<FIELD_TYPE THE_FIELD_TYPE>
typename FieldTypeTraits<THE_FIELD_TYPE>::type const FieldTypeTraits<THE_FIELD_TYPE>::default_ = nullptr;

template<>
struct FieldTypeTraits<FIELD_TYPE_INT32>
{
	typedef std::int32_t type;
	static type const default_ = 0;
};

template<>
struct FieldTypeTraits<FIELD_TYPE_INT64>
{
	typedef std::int64_t type;
	static type const default_ = 0;
};

template<>
struct FieldTypeTraits<FIELD_TYPE_UINT32>
{
	typedef std::uint32_t type;
	static type const default_ = 0;
};

template<>
struct FieldTypeTraits<FIELD_TYPE_UINT64>
{
	typedef std::uint64_t type;
	static type const default_ = 0;
};

template<>
struct FieldTypeTraits<FIELD_TYPE_STRING_FIXED>
{
	// TODO: make this somehow fixed size at initialization?
	typedef std::string type;
	static type const default_;
};

template<>
struct FieldTypeTraits<FIELD_TYPE_STRING_VAR>
{
	typedef std::string type;
	static type const default_;
};

template<>
struct FieldTypeTraits<FIELD_TYPE_FLOAT>
{
	typedef double type;
	static type const default_;
};

template<>
struct FieldTypeTraits<FIELD_TYPE_TIMESTAMP>
{
	typedef std::int64_t type;
	static type const default_ = 0;
};

template<>
struct FieldTypeTraits<FIELD_TYPE_UUID>
{
	typedef std::string type;
	static type const default_;
};

template<>
struct FieldTypeTraits<FIELD_TYPE_UUID_FOREIGN>
{
	typedef std::string type;
	static type const default_;
};

template<>
struct FieldTypeTraits<FIELD_TYPE_STRING_CODE>
{
	typedef std::string type;
	static type const default_;
};

template<>
struct FieldTypeTraits<FIELD_TYPE_STRING_LONGHAND>
{
	typedef std::string type;
	static type const default_;
};

template<>
struct FieldTypeTraits<FIELD_TYPE_TIME_RANGE>
{
	typedef std::uint64_t type;
	static type const default_ = 0;
};

template<>
struct FieldTypeTraits<FIELD_TYPE_NOTES_1>
{
	typedef std::string type;
	static type const default_;
};

template<>
struct FieldTypeTraits<FIELD_TYPE_NOTES_2>
{
	typedef std::string type;
	static type const default_;
};

template<>
struct FieldTypeTraits<FIELD_TYPE_NOTES_3>
{
	typedef std::string type;
	static type const default_;
};

template<>
struct FieldTypeTraits<FIELD_TYPE_FIELD_TYPE>
{
	typedef FIELD_TYPE type;
	static type const default_ = FIELD_TYPE_UNKNOWN;
};

template<>
struct FieldTypeTraits<FIELD_TYPE_DMU_MEMBER_UUID>
{
	typedef std::string type;
	static type const default_;
};

template<>
struct FieldTypeTraits<FIELD_TYPE_DMU_MEMBER_UUID_NUMERIC>
{
	typedef std::int32_t type;
	static type const default_ = 0;
};

template<>
struct FieldTypeTraits<FIELD_TYPE_DMU_MEMBER_UUID_STRING>
{
	typedef std::string type;
	static type const default_;
};

template<>
struct FieldTypeTraits<FIELD_TYPE_DMU_MEMBER_CODE>
{
	typedef std::string type;
	static type const default_;
};

template<>
struct FieldTypeTraits<FIELD_TYPE_DMU_MEMBER_DESCRIPTION>
{
	typedef std::string type;
	static type const default_;
};

template<>
struct FieldTypeTraits<FIELD_TYPE_FK_TO_DMU_CATEGORY_UUID>
{
	typedef std::string type;
	static type const default_;
};

template<>
struct FieldTypeTraits<FIELD_TYPE_TIME_RANGE_OUTPUT_START_DATETIME>
{
	typedef std::int64_t type;
	static type const default_ = 0;
};

template<>
struct FieldTypeTraits<FIELD_TYPE_TIME_RANGE_OUTPUT_END_DATETIME>
{
	typedef std::int64_t type;
	static type const default_ = 0;
};

template<>
struct FieldTypeTraits<FIELD_TYPE_DAY>
{
	typedef std::int32_t type;
	static type const default_ = 0;
};

template<>
struct FieldTypeTraits<FIELD_TYPE_MONTH>
{
	typedef std::int32_t type;
	static type const default_ = 0;
};

template<>
struct FieldTypeTraits<FIELD_TYPE_YEAR>
{
	typedef std::int32_t type;
	static type const default_ = 0;
};

template<>
struct FieldTypeTraits<FIELD_TYPE_TIMERANGE_STRING>
{
	typedef std::string type;
	static type const default_ = 0;
};

template<>
struct FieldTypeTraits<FIELD_TYPE_DMU_PRIMARY_KEY_AND_DAY>
{
	typedef std::int32_t type;
	static type const default_ = 0;
};

template<>
struct FieldTypeTraits<FIELD_TYPE_DMU_PRIMARY_KEY_AND_MONTH>
{
	typedef std::int32_t type;
	static type const default_ = 0;
};

template<>
struct FieldTypeTraits<FIELD_TYPE_DMU_PRIMARY_KEY_AND_YEAR>
{
	typedef std::int32_t type;
	static type const default_ = 0;
};

template<>
struct FieldTypeTraits<FIELD_TYPE_DMU_PRIMARY_KEY_AND_TIMERANGE_STRING>
{
	typedef std::string type;
	static type const default_ = 0;
};

#endif
