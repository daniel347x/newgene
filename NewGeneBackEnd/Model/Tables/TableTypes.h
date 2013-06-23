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

	, TABLE_TYPE_DMU_IDENTIFIER
	, TABLE_TYPE_DMU_INSTANCE
	, TABLE_TYPE_CMU_IDENTIFIER
	, TABLE_TYPE_CMU_INSTANCE
	, TABLE_TYPE_UOA_IDENTIFIER
	, TABLE_TYPE_UOA_MEMBER
	, TABLE__VG_CATEGORY
	, TABLE__VG_SET_MEMBER

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

// DMU_IDENTIFIER
// Example table entries:
// COUNTRY
// ALLIANCE
template<>
std::vector<FIELD_TYPE> const TableTypeTraits<TABLE_TYPE_DMU_IDENTIFIER>::types {FIELD_TYPE_UUID, FIELD_TYPE_STRING_CODE, FIELD_TYPE_STRING_LONGHAND, FIELD_TYPE_NOTES_1, FIELD_TYPE_NOTES_2, FIELD_TYPE_NOTES_3};
template<>
std::vector<std::string> const TableTypeTraits<TABLE_TYPE_DMU_IDENTIFIER>::names {"DMU_IDENTIFIER_UUID", "DMU_IDENTIFIER_STRING_CODE", "DMU_IDENTIFIER_STRING_LONGHAND", "NOTES1", "NOTES2", "NOTES3"};

// DMU_INSTANCE : keys on DMU_IDENTIFIER
// Example table entries:
// DMU_IDENTIFIER = Country: USA
// DMU_IDENTIFIER = Country: RUSSIA
// DMU_IDENTIFIER = Alliance: NATO
// DMU_IDENTIFIER = Alliance: EUROPEAN_ALLIANCE
template<>
std::vector<FIELD_TYPE> const TableTypeTraits<TABLE_TYPE_DMU_INSTANCE>::types {FIELD_TYPE_UUID, FIELD_TYPE_UUID_FOREIGN, FIELD_TYPE_STRING_CODE, FIELD_TYPE_STRING_LONGHAND, FIELD_TYPE_NOTES_1, FIELD_TYPE_NOTES_2, FIELD_TYPE_NOTES_3};
template<>
std::vector<std::string> const TableTypeTraits<TABLE_TYPE_DMU_INSTANCE>::names {"DMU_INSTANCE_UUID", "DMU_IDENTIFIER_UUID", "DMU_INSTANCE_STRING_CODE", "DMU_INSTANCE_STRING_LONGHAND", "NOTES1", "NOTES2", "NOTES3"};

// CMU_IDENTIFIER - keys on two DMU_IDENTIFIERs
// (Always two DMU's)
// Example table entries:
// Country, Alliance
template<>
std::vector<FIELD_TYPE> const TableTypeTraits<TABLE_TYPE_CMU_IDENTIFIER>::types {FIELD_TYPE_UUID, FIELD_TYPE_UUID_FOREIGN, FIELD_TYPE_UUID_FOREIGN, FIELD_TYPE_STRING_CODE, FIELD_TYPE_STRING_LONGHAND, FIELD_TYPE_NOTES_1, FIELD_TYPE_NOTES_2, FIELD_TYPE_NOTES_3};
template<>
std::vector<std::string> const TableTypeTraits<TABLE_TYPE_CMU_IDENTIFIER>::names {"CMU_IDENTIFIER_UUID", "DMU_IDENTIFIER_UUID_1", "DMU_IDENTIFIER_UUID_2", "CMU_IDENTIFIER_STRING_CODE", "CMU_IDENTIFIER_STRING_LONGHAND", "NOTES1", "NOTES2", "NOTES3"};

// CMU_INSTANCE
// Example table entries:
// CMU = Country_Alliance: USA, NATO
// CMU = Country_Alliance: ENGLAND, NATO
template<>
std::vector<FIELD_TYPE> const TableTypeTraits<TABLE_TYPE_CMU_INSTANCE>::types {FIELD_TYPE_UUID, FIELD_TYPE_UUID_FOREIGN, FIELD_TYPE_UUID_FOREIGN, FIELD_TYPE_UUID_FOREIGN};
template<>
std::vector<std::string> const TableTypeTraits<TABLE_TYPE_CMU_INSTANCE>::names {"CMU_INSTANCE_UUID", "CMU_IDENTIFIER_UUID", "DMU_INSTANCE_UUID_1", "DMU_INSTANCE_UUID_2"};

// UOA_IDENTIFIER
// Example table entries:
// "Country_Alliance_NGO (minute)", minute
// "Country_Alliance_NGO (year)", year
// "Country_Alliance_GO", year
template<>
std::vector<FIELD_TYPE> const TableTypeTraits<TABLE_TYPE_UOA_IDENTIFIER>::types {FIELD_TYPE_UUID, FIELD_TYPE_STRING_CODE, FIELD_TYPE_STRING_LONGHAND, FIELD_TYPE_TIME_RANGE, FIELD_TYPE_NOTES_1, FIELD_TYPE_NOTES_2, FIELD_TYPE_NOTES_3};
template<>
std::vector<std::string> const TableTypeTraits<TABLE_TYPE_UOA_IDENTIFIER>::names {"UOA_IDENTIFIER_UUID", "UOA_IDENTIFIER_STRING_CODE", "UOA_IDENTIFIER_STRING_LONGHAND", "TIME_GRANULARITY", "NOTES1", "NOTES2", "NOTES3"};

// UOA_MEMBER - keys on UOA_IDENTIFIER, DMU_IDENTIFIER.
// Example table entries:
// UOA_IDENTIFIER = "Country_Alliance_NGO (minute)", DMU_IDENTIFIER = Country
// UOA_IDENTIFIER = "Country_Alliance_NGO (minute)": DMU_IDENTIFIER = Alliance
// UOA_IDENTIFIER = "Country_Alliance_NGO (minute)": DMU_IDENTIFIER = NGO
template<>
std::vector<FIELD_TYPE> const TableTypeTraits<TABLE_TYPE_UOA_MEMBER>::types {FIELD_TYPE_UUID, FIELD_TYPE_UUID_FOREIGN, FIELD_TYPE_UUID_FOREIGN};
template<>
std::vector<std::string> const TableTypeTraits<TABLE_TYPE_UOA_MEMBER>::names {"UOA_MEMBER_UUID", "UOA_IDENTIFIER_UUID", "DMU_IDENTIFIER_UUID"};

// VARIABLE_GROUP_IDENTIFIER - keys on UOA_IDENTIFIER
// Example table entries:
// UOA = "Country_Alliance_NGO (minute)": "Sum of Assistance Provided"
// UOA = "Country_Alliance_NGO (minute)": "Highest Individual Assistance Provided"
// UOA = "Country_Alliance_GO (year)": "Assistance Provided"
template<>
std::vector<FIELD_TYPE> const TableTypeTraits<TABLE__VG_CATEGORY>::types {FIELD_TYPE_UUID, FIELD_TYPE_UUID_FOREIGN, FIELD_TYPE_STRING_CODE, FIELD_TYPE_STRING_LONGHAND, FIELD_TYPE_NOTES_1, FIELD_TYPE_NOTES_2, FIELD_TYPE_NOTES_3};
template<>
std::vector<std::string> const TableTypeTraits<TABLE__VG_CATEGORY>::names {"VARIABLE_GROUP_IDENTIFIER_UUID", "UOA_IDENTIFIER_UUID", "VARIABLE_GROUP_IDENTIFIER_STRING_CODE", "VARIABLE_GROUP_IDENTIFIER_STRING_LONGHAND", "NOTES1", "NOTES2", "NOTES3"};

// VARIABLE_IDENTIFIER - keys on VARIABLE_GROUP_IDENTIFIER
// Example table entries:
// Variable group identifier = "Sum of Assistance Provided": Happiness
// Variable group identifier = "Sum of Assistance Provided": Controversy
// Variable group identifier = "Highest Individual Assistance Provided": Name of donor
// Variable group identifier = "Highest Individual Assistance Provided": Status of donor
template<>
std::vector<FIELD_TYPE> const TableTypeTraits<TABLE__VG_SET_MEMBER>::types {FIELD_TYPE_UUID, FIELD_TYPE_UUID_FOREIGN, FIELD_TYPE_FIELD_TYPE, FIELD_TYPE_STRING_CODE, FIELD_TYPE_STRING_LONGHAND, FIELD_TYPE_NOTES_1, FIELD_TYPE_NOTES_2, FIELD_TYPE_NOTES_3};
template<>
std::vector<std::string> const TableTypeTraits<TABLE__VG_SET_MEMBER>::names {"VARIABLE_IDENTIFIER_UUID", "VARIABLE_GROUP_IDENTIFIER_UUID", "VARIABLE_IDENTIFIER_FIELD_TYPE", "VARIABLE_IDENTIFIER_STRING_CODE", "VARIABLE_IDENTIFIER_STRING_LONGHAND", "NOTES1", "NOTES2", "NOTES3"};


// The following approach would be used if new tables were not to be dynamically created
// ... it would be horrendously inefficient
#if 0

enum TABLE_TYPES
{

	  TABLE_TYPE_NONE = 0

	, TABLE_TYPE_DMU_IDENTIFIER
	, TABLE_TYPE_DMU_INSTANCE
	, TABLE_TYPE_CMU_IDENTIFIER
	, TABLE_TYPE_CMU_INSTANCE
	, TABLE_TYPE_UOA_IDENTIFIER
	, TABLE_TYPE_UOA_MEMBER
	, TABLE_TYPE_UOA_INSTANCE
	, TABLE_TYPE_VARIABLE_GROUP_IDENTIFIER
	, TABLE_TYPE_VARIABLE_IDENTIFIER
	, TABLE_TYPE_VARIABLE_INSTANCE

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

// DMU_IDENTIFIER
// Example table entries:
// COUNTRY
// ALLIANCE
template<>
std::vector<FIELD_TYPE> const TableTypeTraits<TABLE_TYPE_DMU_IDENTIFIER>::types {FIELD_TYPE_UUOA, FIELD_TYPE_STRING_VAR, FIELD_TYPE_STRING_VAR};
template<>
std::vector<std::string> const TableTypeTraits<TABLE_TYPE_DMU_IDENTIFIER>::names {"DMU_UUID", "DMU" "DESC"};

// DMU_INSTANCE : keys on DMU_IDENTIFIER
// Example table entries:
// DMU = Country: USA
// DMU = Country: RUSSIA
// DMU = Alliance: NATO
// DMU = Alliance: EUROPEAN_ALLIANCE
template<>
std::vector<FIELD_TYPE> const TableTypeTraits<TABLE_TYPE_DMU_INSTANCE>::types {FIELD_TYPE_UUOA, FIELD_TYPE_STRING_VAR, FIELD_TYPE_STRING_VAR};
template<>
std::vector<std::string> const TableTypeTraits<TABLE_TYPE_DMU_INSTANCE>::names {"DMU_UUID", "DMU" "DESC"};

// CMU_IDENTIFIER - keys on two DMU_IDENTIFIERs
// (Always two DMU's)
// Example table entries:
// DMU's = Country, Alliance: "Country_Alliance" (name not required)
template<>
std::vector<FIELD_TYPE> const TableTypeTraits<TABLE_TYPE_CMU_IDENTIFIER>::types {FIELD_TYPE_UUOA, FIELD_TYPE_STRING_VAR, FIELD_TYPE_STRING_VAR, FIELD_TYPE_STRING_VAR, FIELD_TYPE_STRING_VAR};
template<>
std::vector<std::string> const TableTypeTraits<TABLE_TYPE_CMU_IDENTIFIER>::names {"CMU_UUID", "CMU", "DESC", "DMU1", "DMU2"};

// CMU_INSTANCE
// Example table entries:
// CMU = Country_Alliance: USA, NATO
// CMU = Country_Alliance: ENGLAND, NATO
template<>
std::vector<FIELD_TYPE> const TableTypeTraits<TABLE_TYPE_CMU_INSTANCE>::types {FIELD_TYPE_UUOA, FIELD_TYPE_STRING_VAR, FIELD_TYPE_STRING_VAR, FIELD_TYPE_STRING_VAR, FIELD_TYPE_STRING_VAR};
template<>
std::vector<std::string> const TableTypeTraits<TABLE_TYPE_CMU_INSTANCE>::names {"CMU_UUID", "CMU", "DESC", "DMU1", "DMU2"};

// UOA_IDENTIFIER
// Example table entries:
// "Country_Alliance_NGO (minute)", minute
// "Country_Alliance_NGO (year)", year
// "Country_Alliance_GO", year
template<>
std::vector<FIELD_TYPE> const TableTypeTraits<TABLE_TYPE_UOA_IDENTIFIER>::types {FIELD_TYPE_UUOA, FIELD_TYPE_STRING_VAR, FIELD_TYPE_STRING_VAR};
template<>
std::vector<std::string> const TableTypeTraits<TABLE_TYPE_UOA_IDENTIFIER>::names {"UOA_UUID", "UOA", "DESC"};

// UOA_MEMBER - keys on UOA_IDENTIFIER, DMU_IDENTIFIER.
// Example table entries:
// UOA_IDENTIFIER = "Country_Alliance_NGO (minute)", DMU_IDENTIFIER = Country
// UOA_IDENTIFIER = "Country_Alliance_NGO (minute)": DMU_IDENTIFIER = Alliance
// UOA_IDENTIFIER = "Country_Alliance_NGO (minute)": DMU_IDENTIFIER = NGO
template<>
std::vector<FIELD_TYPE> const TableTypeTraits<TABLE_TYPE_UOA_MEMBER>::types {FIELD_TYPE_UUOA, FIELD_TYPE_STRING_VAR, FIELD_TYPE_STRING_VAR};
template<>
std::vector<std::string> const TableTypeTraits<TABLE_TYPE_UOA_MEMBER>::names {"UOA_UUID", "UOA", "DESC"};

// UOA_INSTANCE - keys on UOA_MEMBER, Time range, DMU_INSTANCE.
// The UUID is ***NOT*** unique in this table; instead, all DMU_IDENTIFIER's correspond to the same UUID in this table.
// Example table entries:
// UOA_MEMBER = "Country_Alliance_NGO (minute)", DMU_IDENTIFIER = Country, Time range = xxx - xxx, DMU_INSTANCE = USA
// UOA_MEMBER = "Country_Alliance_NGO (minute)": DMU_IDENTIFIER = Alliance, Time range = xxx - xxx, DMU_INSTANCE = NATO
// UOA_MEMBER = "Country_Alliance_NGO (minute)": DMU_IDENTIFIER = NGO, Time range = xxx - xxx, DMU_INSTANCE = RedCross
template<>
std::vector<FIELD_TYPE> const TableTypeTraits<TABLE_TYPE_UOA_MEMBER>::types {FIELD_TYPE_UUOA, FIELD_TYPE_STRING_VAR, FIELD_TYPE_STRING_VAR};
template<>
std::vector<std::string> const TableTypeTraits<TABLE_TYPE_UOA_MEMBER>::names {"UOA_UUID", "UOA", "DESC"};

// VARIABLE_GROUP_IDENTIFIER - keys on UOA_IDENTIFIER
// Example table entries:
// UOA = "Country_Alliance_NGO (minute)": "Sum of Assistance Provided"
// UOA = "Country_Alliance_NGO (minute)": "Highest Individual Assistance Provided"
// UOA = "Country_Alliance_GO (year)": "Assistance Provided"
template<>
std::vector<FIELD_TYPE> const TableTypeTraits<TABLE_TYPE_VARIABLE_GROUP_IDENTIFIER>::types {FIELD_TYPE_UUOA, FIELD_TYPE_UUOA};
template<>
std::vector<std::string> const TableTypeTraits<TABLE_TYPE_VARIABLE_GROUP_IDENTIFIER>::names {"UOA_UUID", "DMU_UUID"};

// VARIABLE_IDENTIFIER - keys on VARIABLE_GROUP_IDENTIFIER
// Example table entries:
// Variable group identifier = "Sum of Assistance Provided": Happiness
// Variable group identifier = "Sum of Assistance Provided": Controversy
// Variable group identifier = "Highest Individual Assistance Provided": Name of donor
// Variable group identifier = "Highest Individual Assistance Provided": Status of donor
template<>
std::vector<FIELD_TYPE> const TableTypeTraits<TABLE_TYPE_VARIABLE_INSTANCE>::types {FIELD_TYPE_UUOA, FIELD_TYPE_STRING_VAR, FIELD_TYPE_STRING_VAR, FIELD_TYPE_UUOA};
template<>
std::vector<std::string> const TableTypeTraits<TABLE_TYPE_VARIABLE_INSTANCE>::names {"VGP_UUID", "VGP", "DESC", "UOA_UUID"};

// VARIABLE_INSTANCE - keys on VARIABLE_IDENTIFIER, UOA_MEMBER (UOA_MEMBER is NOT unique either in its table, or in this table)
// Example table entries:
template<>
std::vector<FIELD_TYPE> const TableTypeTraits<TABLE_TYPE_VARIABLE_INSTANCE>::types {FIELD_TYPE_UUOA, FIELD_TYPE_STRING_VAR, FIELD_TYPE_STRING_VAR, FIELD_TYPE_UUOA};
template<>
std::vector<std::string> const TableTypeTraits<TABLE_TYPE_VARIABLE_INSTANCE>::names {"VGP_UUID", "VGP", "DESC", "UOA_UUID"};

#endif

#endif
