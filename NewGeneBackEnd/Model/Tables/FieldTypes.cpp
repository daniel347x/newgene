#include "FieldTypes.h"

#ifndef Q_MOC_RUN
	#include <boost/lexical_cast.hpp>
	#include <boost/format.hpp>
#endif
#include "../../Utilities/NewGeneException.h"

// Handle string default initializiations
FieldTypeTraits<FIELD_TYPE_STRING_FIXED>::type const FieldTypeTraits<FIELD_TYPE_STRING_FIXED>::default_;
FieldTypeTraits<FIELD_TYPE_STRING_VAR>::type const FieldTypeTraits<FIELD_TYPE_STRING_VAR>::default_;
FieldTypeTraits<FIELD_TYPE_FLOAT>::type const FieldTypeTraits<FIELD_TYPE_FLOAT>::default_ = 0.0;
FieldTypeTraits<FIELD_TYPE_UUID>::type const FieldTypeTraits<FIELD_TYPE_UUID>::default_;
FieldTypeTraits<FIELD_TYPE_UUID_FOREIGN>::type const FieldTypeTraits<FIELD_TYPE_UUID_FOREIGN>::default_;
FieldTypeTraits<FIELD_TYPE_STRING_CODE>::type const FieldTypeTraits<FIELD_TYPE_STRING_CODE>::default_;
FieldTypeTraits<FIELD_TYPE_STRING_LONGHAND>::type const FieldTypeTraits<FIELD_TYPE_STRING_LONGHAND>::default_;
FieldTypeTraits<FIELD_TYPE_NOTES_1>::type const FieldTypeTraits<FIELD_TYPE_NOTES_1>::default_;
FieldTypeTraits<FIELD_TYPE_NOTES_2>::type const FieldTypeTraits<FIELD_TYPE_NOTES_2>::default_;
FieldTypeTraits<FIELD_TYPE_NOTES_3>::type const FieldTypeTraits<FIELD_TYPE_NOTES_3>::default_;
FieldTypeTraits<FIELD_TYPE_DMU_MEMBER_UUID>::type const FieldTypeTraits<FIELD_TYPE_DMU_MEMBER_UUID>::default_;
FieldTypeTraits<FIELD_TYPE_DMU_MEMBER_UUID_STRING>::type const FieldTypeTraits<FIELD_TYPE_DMU_MEMBER_UUID_STRING>::default_;
FieldTypeTraits<FIELD_TYPE_DMU_MEMBER_CODE>::type const FieldTypeTraits<FIELD_TYPE_DMU_MEMBER_CODE>::default_;
FieldTypeTraits<FIELD_TYPE_DMU_MEMBER_DESCRIPTION>::type const FieldTypeTraits<FIELD_TYPE_DMU_MEMBER_DESCRIPTION>::default_;
FieldTypeTraits<FIELD_TYPE_FK_TO_DMU_CATEGORY_UUID>::type const FieldTypeTraits<FIELD_TYPE_FK_TO_DMU_CATEGORY_UUID>::default_;
FieldTypeTraits<FIELD_TYPE_DATETIME_STRING>::type const FieldTypeTraits<FIELD_TYPE_DATETIME_STRING>::default_;
FieldTypeTraits<FIELD_TYPE_DMU_PRIMARY_KEY_AND_DATETIME_STRING>::type const FieldTypeTraits<FIELD_TYPE_DMU_PRIMARY_KEY_AND_DATETIME_STRING>::default_;

// Handle int default initialization
// Clang on OS X can't handle in-declaration defaults, thanks much
FieldTypeTraits<FIELD_TYPE_INT32>::type const FieldTypeTraits<FIELD_TYPE_INT32>::default_ = 0;
FieldTypeTraits<FIELD_TYPE_INT64>::type const FieldTypeTraits<FIELD_TYPE_INT64>::default_ = 0;
FieldTypeTraits<FIELD_TYPE_UINT32>::type const FieldTypeTraits<FIELD_TYPE_UINT32>::default_ = 0;
FieldTypeTraits<FIELD_TYPE_UINT64>::type const FieldTypeTraits<FIELD_TYPE_UINT64>::default_ = 0;
FieldTypeTraits<FIELD_TYPE_TIMESTAMP>::type const FieldTypeTraits<FIELD_TYPE_TIMESTAMP>::default_ = 0;
FieldTypeTraits<FIELD_TYPE_TIME_RANGE>::type const FieldTypeTraits<FIELD_TYPE_TIME_RANGE>::default_ = 0;
FieldTypeTraits<FIELD_TYPE_DMU_MEMBER_UUID_NUMERIC>::type const FieldTypeTraits<FIELD_TYPE_DMU_MEMBER_UUID_NUMERIC>::default_ = 0;
FieldTypeTraits<FIELD_TYPE_TIME_RANGE_OUTPUT_START_DATETIME>::type const FieldTypeTraits<FIELD_TYPE_TIME_RANGE_OUTPUT_START_DATETIME>::default_ = 0;
FieldTypeTraits<FIELD_TYPE_TIME_RANGE_OUTPUT_END_DATETIME>::type const FieldTypeTraits<FIELD_TYPE_TIME_RANGE_OUTPUT_END_DATETIME>::default_ = 0;
FieldTypeTraits<FIELD_TYPE_DAY>::type const FieldTypeTraits<FIELD_TYPE_DAY>::default_ = 0;
FieldTypeTraits<FIELD_TYPE_MONTH>::type const FieldTypeTraits<FIELD_TYPE_MONTH>::default_ = 0;
FieldTypeTraits<FIELD_TYPE_YEAR>::type const FieldTypeTraits<FIELD_TYPE_YEAR>::default_ = 0;
FieldTypeTraits<FIELD_TYPE_DMU_PRIMARY_KEY_AND_DAY>::type const FieldTypeTraits<FIELD_TYPE_DMU_PRIMARY_KEY_AND_DAY>::default_ = 0;
FieldTypeTraits<FIELD_TYPE_DMU_PRIMARY_KEY_AND_MONTH>::type const FieldTypeTraits<FIELD_TYPE_DMU_PRIMARY_KEY_AND_MONTH>::default_ = 0;
FieldTypeTraits<FIELD_TYPE_DMU_PRIMARY_KEY_AND_YEAR>::type const FieldTypeTraits<FIELD_TYPE_DMU_PRIMARY_KEY_AND_YEAR>::default_ = 0;

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

bool IsFieldTypeTimeRange(FIELD_TYPE const & field_type)
{

	bool returnVal = false;

	switch (field_type)
	{

		case FIELD_TYPE_DAY:
		case FIELD_TYPE_MONTH:
		case FIELD_TYPE_YEAR:
		case FIELD_TYPE_DATETIME_STRING:
		case FIELD_TYPE_DMU_PRIMARY_KEY_AND_DAY:
		case FIELD_TYPE_DMU_PRIMARY_KEY_AND_MONTH:
		case FIELD_TYPE_DMU_PRIMARY_KEY_AND_YEAR:
		case FIELD_TYPE_DMU_PRIMARY_KEY_AND_DATETIME_STRING:
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

void BindSqlField(sqlite3_stmt * stmt, int & bind_index, std::pair<FIELD_TYPE, std::string> const & field)
{

	bool handled = false;

	if (IsFieldTypeInt32(field.first))
	{
		sqlite3_bind_int(stmt, bind_index++, boost::lexical_cast<std::int32_t>(field.second.c_str()));
		handled = true;
	}
	else if (IsFieldTypeInt64(field.first))
	{
		sqlite3_bind_int64(stmt, bind_index++, boost::lexical_cast<std::int64_t>(field.second.c_str()));
		handled = true;
	}
	else if (IsFieldTypeFloat(field.first))
	{
		sqlite3_bind_double(stmt, bind_index++, boost::lexical_cast<double>(field.second.c_str()));
		handled = true;
	}
	else if (IsFieldTypeString(field.first))
	{
		sqlite3_bind_text(stmt, bind_index++, field.second.c_str(), static_cast<int>(field.second.size()), SQLITE_STATIC);
		handled = true;
	}

	if (!handled)
	{
		boost::format msg("Invalid field type when binding data for import");
		throw NewGeneException() << newgene_error_description(msg.str());
	}

}
