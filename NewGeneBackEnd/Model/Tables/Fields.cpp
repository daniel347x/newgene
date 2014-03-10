#include "Fields.h"

std::int64_t const & BaseField::GetInt64Ref() const
{

	if (!IsFieldTypeInt64(field_type))
	{
		boost::format msg("Field data type is not convertible to int64.");
		throw NewGeneException() << newgene_error_description(msg.str());
	}

	switch (field_type)
	{

		case FIELD_TYPE_INT64:
			{
				Field<FIELD_TYPE_INT64> const & myself = static_cast<Field<FIELD_TYPE_INT64> const &>(*this);
				return static_cast<std::int64_t const &>(myself.GetValueReference());
			}
			break;

		case FIELD_TYPE_UINT64:
			{
				Field<FIELD_TYPE_UINT64> const & myself = static_cast<Field<FIELD_TYPE_UINT64> const &>(*this);
				return static_cast<std::int64_t const &>(myself.GetValueReference());
			}
			break;

		case FIELD_TYPE_TIMESTAMP:
			{
				Field<FIELD_TYPE_TIMESTAMP> const & myself = static_cast<Field<FIELD_TYPE_TIMESTAMP> const &>(*this);
				return static_cast<std::int64_t const &>(myself.GetValueReference());
			}
			break;

		case FIELD_TYPE_TIME_RANGE_OUTPUT_START_DATETIME:
			{
				Field<FIELD_TYPE_TIME_RANGE_OUTPUT_START_DATETIME> const & myself = static_cast<Field<FIELD_TYPE_TIME_RANGE_OUTPUT_START_DATETIME> const &>(*this);
				return static_cast<std::int64_t const &>(myself.GetValueReference());
			}
			break;

		case FIELD_TYPE_TIME_RANGE_OUTPUT_END_DATETIME:
			{
				Field<FIELD_TYPE_TIME_RANGE_OUTPUT_END_DATETIME> const & myself = static_cast<Field<FIELD_TYPE_TIME_RANGE_OUTPUT_END_DATETIME> const &>(*this);
				return static_cast<std::int64_t const &>(myself.GetValueReference());
			}
			break;

		default:
			{
				boost::format msg("Unknown field data type.");
				throw NewGeneException() << newgene_error_description(msg.str());
			}
			break;

	}

	static std::int64_t dummy = 0;
	return dummy;

}

std::int32_t const & BaseField::GetInt32Ref() const
{

	if (!IsFieldTypeInt32(field_type))
	{
		boost::format msg("Field data type is not convertible to int32.");
		throw NewGeneException() << newgene_error_description(msg.str());
	}

	switch (field_type)
	{

		case FIELD_TYPE_INT32:
			{
				Field<FIELD_TYPE_INT32> const & myself = static_cast<Field<FIELD_TYPE_INT32> const &>(*this);
				return static_cast<std::int32_t const &>(myself.GetValueReference());
			}
			break;

		case FIELD_TYPE_UINT32:
			{
				Field<FIELD_TYPE_UINT32> const & myself = static_cast<Field<FIELD_TYPE_UINT32> const &>(*this);
				return static_cast<std::int32_t const &>(myself.GetValueReference());
			}
			break;

		case FIELD_TYPE_DMU_MEMBER_UUID_NUMERIC:
			{
				Field<FIELD_TYPE_DMU_MEMBER_UUID_NUMERIC> const & myself = static_cast<Field<FIELD_TYPE_DMU_MEMBER_UUID_NUMERIC> const &>(*this);
				return static_cast<std::int32_t const &>(myself.GetValueReference());
			}
			break;

		case FIELD_TYPE_DAY:
			{
				Field<FIELD_TYPE_DAY> const & myself = static_cast<Field<FIELD_TYPE_DAY> const &>(*this);
				return static_cast<std::int32_t const &>(myself.GetValueReference());
			}
			break;

		case FIELD_TYPE_MONTH:
			{
				Field<FIELD_TYPE_MONTH> const & myself = static_cast<Field<FIELD_TYPE_MONTH> const &>(*this);
				return static_cast<std::int32_t const &>(myself.GetValueReference());
			}
			break;

		case FIELD_TYPE_YEAR:
			{
				Field<FIELD_TYPE_YEAR> const & myself = static_cast<Field<FIELD_TYPE_YEAR> const &>(*this);
				return static_cast<std::int32_t const &>(myself.GetValueReference());
			}
			break;

		case FIELD_TYPE_DMU_PRIMARY_KEY_AND_DAY:
			{
				Field<FIELD_TYPE_DMU_PRIMARY_KEY_AND_DAY> const & myself = static_cast<Field<FIELD_TYPE_DMU_PRIMARY_KEY_AND_DAY> const &>(*this);
				return static_cast<std::int32_t const &>(myself.GetValueReference());
			}
			break;

		case FIELD_TYPE_DMU_PRIMARY_KEY_AND_MONTH:
			{
				Field<FIELD_TYPE_DMU_PRIMARY_KEY_AND_MONTH> const & myself = static_cast<Field<FIELD_TYPE_DMU_PRIMARY_KEY_AND_MONTH> const &>(*this);
				return static_cast<std::int32_t const &>(myself.GetValueReference());
			}
			break;

		case FIELD_TYPE_DMU_PRIMARY_KEY_AND_YEAR:
			{
				Field<FIELD_TYPE_DMU_PRIMARY_KEY_AND_YEAR> const & myself = static_cast<Field<FIELD_TYPE_DMU_PRIMARY_KEY_AND_YEAR> const &>(*this);
				return static_cast<std::int32_t const &>(myself.GetValueReference());
			}
			break;

		default:
			{
				boost::format msg("Unknown field data type.");
				throw NewGeneException() << newgene_error_description(msg.str());
			}
			break;

	}

	static std::int32_t dummy = 0;
	return dummy;

}

std::string const & BaseField::GetStringRef() const
{

	if (!IsFieldTypeString(field_type))
	{
		boost::format msg("Field data type is not convertible to a string.");
		throw NewGeneException() << newgene_error_description(msg.str());
	}

	switch (field_type)
	{

		case FIELD_TYPE_STRING_FIXED:
			{
				Field<FIELD_TYPE_STRING_FIXED> const & myself = static_cast<Field<FIELD_TYPE_STRING_FIXED> const &>(*this);
				return myself.GetValueReference();
			}
			break;

		case FIELD_TYPE_STRING_VAR:
			{
				Field<FIELD_TYPE_STRING_VAR> const & myself = static_cast<Field<FIELD_TYPE_STRING_VAR> const &>(*this);
				return myself.GetValueReference();
			}
			break;

		case FIELD_TYPE_UUID:
			{
				Field<FIELD_TYPE_UUID> const & myself = static_cast<Field<FIELD_TYPE_UUID> const &>(*this);
				return myself.GetValueReference();
			}
			break;

		case FIELD_TYPE_UUID_FOREIGN:
			{
				Field<FIELD_TYPE_UUID_FOREIGN> const & myself = static_cast<Field<FIELD_TYPE_UUID_FOREIGN> const &>(*this);
				return myself.GetValueReference();
			}
			break;

		case FIELD_TYPE_STRING_CODE:
			{
				Field<FIELD_TYPE_STRING_CODE> const & myself = static_cast<Field<FIELD_TYPE_STRING_CODE> const &>(*this);
				return myself.GetValueReference();
			}
			break;

		case FIELD_TYPE_STRING_LONGHAND:
			{
				Field<FIELD_TYPE_STRING_LONGHAND> const & myself = static_cast<Field<FIELD_TYPE_STRING_LONGHAND> const &>(*this);
				return myself.GetValueReference();
			}
			break;

		case FIELD_TYPE_NOTES_1:
			{
				Field<FIELD_TYPE_NOTES_1> const & myself = static_cast<Field<FIELD_TYPE_NOTES_1> const &>(*this);
				return myself.GetValueReference();
			}
			break;

		case FIELD_TYPE_NOTES_2:
			{
				Field<FIELD_TYPE_NOTES_2> const & myself = static_cast<Field<FIELD_TYPE_NOTES_2> const &>(*this);
				return myself.GetValueReference();
			}
			break;

		case FIELD_TYPE_NOTES_3:
			{
				Field<FIELD_TYPE_NOTES_3> const & myself = static_cast<Field<FIELD_TYPE_NOTES_3> const &>(*this);
				return myself.GetValueReference();
			}
			break;

		case FIELD_TYPE_DMU_MEMBER_UUID:
			{
				Field<FIELD_TYPE_DMU_MEMBER_UUID> const & myself = static_cast<Field<FIELD_TYPE_DMU_MEMBER_UUID> const &>(*this);
				return myself.GetValueReference();
			}
			break;

		case FIELD_TYPE_DMU_MEMBER_UUID_STRING:
			{
				Field<FIELD_TYPE_DMU_MEMBER_UUID_STRING> const & myself = static_cast<Field<FIELD_TYPE_DMU_MEMBER_UUID_STRING> const &>(*this);
				return myself.GetValueReference();
			}
			break;

		case FIELD_TYPE_DMU_MEMBER_CODE:
			{
				Field<FIELD_TYPE_DMU_MEMBER_CODE> const & myself = static_cast<Field<FIELD_TYPE_DMU_MEMBER_CODE> const &>(*this);
				return myself.GetValueReference();
			}
			break;

		case FIELD_TYPE_DMU_MEMBER_DESCRIPTION:
			{
				Field<FIELD_TYPE_DMU_MEMBER_DESCRIPTION> const & myself = static_cast<Field<FIELD_TYPE_DMU_MEMBER_DESCRIPTION> const &>(*this);
				return myself.GetValueReference();
			}
			break;

		case FIELD_TYPE_FK_TO_DMU_CATEGORY_UUID:
			{
				Field<FIELD_TYPE_FK_TO_DMU_CATEGORY_UUID> const & myself = static_cast<Field<FIELD_TYPE_FK_TO_DMU_CATEGORY_UUID> const &>(*this);
				return myself.GetValueReference();
			}
			break;

		case FIELD_TYPE_TIMERANGE_STRING:
			{
				Field<FIELD_TYPE_TIMERANGE_STRING> const & myself = static_cast<Field<FIELD_TYPE_TIMERANGE_STRING> const &>(*this);
				return myself.GetValueReference();
			}
			break;

		case FIELD_TYPE_DMU_PRIMARY_KEY_AND_TIMERANGE_STRING:
			{
				Field<FIELD_TYPE_DMU_PRIMARY_KEY_AND_TIMERANGE_STRING> const & myself = static_cast<Field<FIELD_TYPE_DMU_PRIMARY_KEY_AND_TIMERANGE_STRING> const &>(*this);
				return myself.GetValueReference();
			}
			break;

		default:
			{
				boost::format msg("Unknown field data type.");
				throw NewGeneException() << newgene_error_description(msg.str());
			}
			break;

	}

	static std::string dummy;
	return dummy;

}

std::int64_t BaseField::GetInt64() const
{

	if (!IsFieldTypeInt64(field_type))
	{
		boost::format msg("Field data type is not convertible to int64.");
		throw NewGeneException() << newgene_error_description(msg.str());
	}

	switch (field_type)
	{

		case FIELD_TYPE_INT64:
			{
				Field<FIELD_TYPE_INT64> const & myself = static_cast<Field<FIELD_TYPE_INT64> const &>(*this);
				return static_cast<std::int64_t const &>(myself.GetValueReference());
			}
			break;

		case FIELD_TYPE_UINT64:
			{
				Field<FIELD_TYPE_UINT64> const & myself = static_cast<Field<FIELD_TYPE_UINT64> const &>(*this);
				return static_cast<std::int64_t const &>(myself.GetValueReference());
			}
			break;

		case FIELD_TYPE_TIMESTAMP:
			{
				Field<FIELD_TYPE_TIMESTAMP> const & myself = static_cast<Field<FIELD_TYPE_TIMESTAMP> const &>(*this);
				return static_cast<std::int64_t const &>(myself.GetValueReference());
			}
			break;

		case FIELD_TYPE_TIME_RANGE_OUTPUT_START_DATETIME:
			{
				Field<FIELD_TYPE_TIME_RANGE_OUTPUT_START_DATETIME> const & myself = static_cast<Field<FIELD_TYPE_TIME_RANGE_OUTPUT_START_DATETIME> const &>(*this);
				return static_cast<std::int64_t const &>(myself.GetValueReference());
			}
			break;

		case FIELD_TYPE_TIME_RANGE_OUTPUT_END_DATETIME:
			{
				Field<FIELD_TYPE_TIME_RANGE_OUTPUT_END_DATETIME> const & myself = static_cast<Field<FIELD_TYPE_TIME_RANGE_OUTPUT_END_DATETIME> const &>(*this);
				return static_cast<std::int64_t const &>(myself.GetValueReference());
			}
			break;

		default:
			{
				boost::format msg("Unknown field data type.");
				throw NewGeneException() << newgene_error_description(msg.str());
			}
			break;

	}

	return 0;

}

std::int32_t BaseField::GetInt32() const
{

	if (!IsFieldTypeInt32(field_type))
	{
		boost::format msg("Field data type is not convertible to int32.");
		throw NewGeneException() << newgene_error_description(msg.str());
	}

	switch (field_type)
	{

		case FIELD_TYPE_INT32:
			{
				Field<FIELD_TYPE_INT32> const & myself = static_cast<Field<FIELD_TYPE_INT32> const &>(*this);
				return static_cast<std::int32_t const &>(myself.GetValueReference());
			}
			break;

		case FIELD_TYPE_UINT32:
			{
				Field<FIELD_TYPE_UINT32> const & myself = static_cast<Field<FIELD_TYPE_UINT32> const &>(*this);
				return static_cast<std::int32_t const &>(myself.GetValueReference());
			}
			break;

		case FIELD_TYPE_DMU_MEMBER_UUID_NUMERIC:
			{
				Field<FIELD_TYPE_DMU_MEMBER_UUID_NUMERIC> const & myself = static_cast<Field<FIELD_TYPE_DMU_MEMBER_UUID_NUMERIC> const &>(*this);
				return static_cast<std::int32_t const &>(myself.GetValueReference());
			}
			break;

		case FIELD_TYPE_DAY:
			{
				Field<FIELD_TYPE_DAY> const & myself = static_cast<Field<FIELD_TYPE_DAY> const &>(*this);
				return static_cast<std::int32_t const &>(myself.GetValueReference());
			}
			break;

		case FIELD_TYPE_MONTH:
			{
				Field<FIELD_TYPE_MONTH> const & myself = static_cast<Field<FIELD_TYPE_MONTH> const &>(*this);
				return static_cast<std::int32_t const &>(myself.GetValueReference());
			}
			break;

		case FIELD_TYPE_YEAR:
			{
				Field<FIELD_TYPE_YEAR> const & myself = static_cast<Field<FIELD_TYPE_YEAR> const &>(*this);
				return static_cast<std::int32_t const &>(myself.GetValueReference());
			}
			break;

		case FIELD_TYPE_DMU_PRIMARY_KEY_AND_DAY:
			{
				Field<FIELD_TYPE_DMU_PRIMARY_KEY_AND_DAY> const & myself = static_cast<Field<FIELD_TYPE_DMU_PRIMARY_KEY_AND_DAY> const &>(*this);
				return static_cast<std::int32_t const &>(myself.GetValueReference());
			}
			break;

		case FIELD_TYPE_DMU_PRIMARY_KEY_AND_MONTH:
			{
				Field<FIELD_TYPE_DMU_PRIMARY_KEY_AND_MONTH> const & myself = static_cast<Field<FIELD_TYPE_DMU_PRIMARY_KEY_AND_MONTH> const &>(*this);
				return static_cast<std::int32_t const &>(myself.GetValueReference());
			}
			break;

		case FIELD_TYPE_DMU_PRIMARY_KEY_AND_YEAR:
			{
				Field<FIELD_TYPE_DMU_PRIMARY_KEY_AND_YEAR> const & myself = static_cast<Field<FIELD_TYPE_DMU_PRIMARY_KEY_AND_YEAR> const &>(*this);
				return static_cast<std::int32_t const &>(myself.GetValueReference());
			}
			break;

		default:
			{
				boost::format msg("Unknown field data type.");
				throw NewGeneException() << newgene_error_description(msg.str());
			}
			break;

	}

	return 0;

}

double BaseField::GetDouble() const
{

	if (!IsFieldTypeFloat(field_type))
	{
		boost::format msg("Field data type is not convertible to double.");
		throw NewGeneException() << newgene_error_description(msg.str());
	}

	switch (field_type)
	{

		case FIELD_TYPE_FLOAT:
			{
				Field<FIELD_TYPE_FLOAT> const & myself = static_cast<Field<FIELD_TYPE_FLOAT> const &>(*this);
				return static_cast<double const &>(myself.GetValueReference());
			}
			break;

		default:
			{
				boost::format msg("Unknown field data type.");
				throw NewGeneException() << newgene_error_description(msg.str());
			}
			break;

	}

	return 0.0;

}

std::string BaseField::GetString() const
{

	if (!IsFieldTypeString(field_type))
	{
		boost::format msg("Field data type is not convertible to a string.");
		throw NewGeneException() << newgene_error_description(msg.str());
	}

	switch (field_type)
	{

		case FIELD_TYPE_STRING_FIXED:
			{
				Field<FIELD_TYPE_STRING_FIXED> const & myself = static_cast<Field<FIELD_TYPE_STRING_FIXED> const &>(*this);
				return myself.GetValueReference();
			}
			break;

		case FIELD_TYPE_STRING_VAR:
			{
				Field<FIELD_TYPE_STRING_VAR> const & myself = static_cast<Field<FIELD_TYPE_STRING_VAR> const &>(*this);
				return myself.GetValueReference();
			}
			break;

		case FIELD_TYPE_UUID:
			{
				Field<FIELD_TYPE_UUID> const & myself = static_cast<Field<FIELD_TYPE_UUID> const &>(*this);
				return myself.GetValueReference();
			}
			break;

		case FIELD_TYPE_UUID_FOREIGN:
			{
				Field<FIELD_TYPE_UUID_FOREIGN> const & myself = static_cast<Field<FIELD_TYPE_UUID_FOREIGN> const &>(*this);
				return myself.GetValueReference();
			}
			break;

		case FIELD_TYPE_STRING_CODE:
			{
				Field<FIELD_TYPE_STRING_CODE> const & myself = static_cast<Field<FIELD_TYPE_STRING_CODE> const &>(*this);
				return myself.GetValueReference();
			}
			break;

		case FIELD_TYPE_STRING_LONGHAND:
			{
				Field<FIELD_TYPE_STRING_LONGHAND> const & myself = static_cast<Field<FIELD_TYPE_STRING_LONGHAND> const &>(*this);
				return myself.GetValueReference();
			}
			break;

		case FIELD_TYPE_NOTES_1:
			{
				Field<FIELD_TYPE_NOTES_1> const & myself = static_cast<Field<FIELD_TYPE_NOTES_1> const &>(*this);
				return myself.GetValueReference();
			}
			break;

		case FIELD_TYPE_NOTES_2:
			{
				Field<FIELD_TYPE_NOTES_2> const & myself = static_cast<Field<FIELD_TYPE_NOTES_2> const &>(*this);
				return myself.GetValueReference();
			}
			break;

		case FIELD_TYPE_NOTES_3:
			{
				Field<FIELD_TYPE_NOTES_3> const & myself = static_cast<Field<FIELD_TYPE_NOTES_3> const &>(*this);
				return myself.GetValueReference();
			}
			break;

		case FIELD_TYPE_DMU_MEMBER_UUID:
			{
				Field<FIELD_TYPE_DMU_MEMBER_UUID> const & myself = static_cast<Field<FIELD_TYPE_DMU_MEMBER_UUID> const &>(*this);
				return myself.GetValueReference();
			}
			break;

		case FIELD_TYPE_DMU_MEMBER_UUID_STRING:
			{
				Field<FIELD_TYPE_DMU_MEMBER_UUID_STRING> const & myself = static_cast<Field<FIELD_TYPE_DMU_MEMBER_UUID_STRING> const &>(*this);
				return myself.GetValueReference();
			}
			break;

		case FIELD_TYPE_DMU_MEMBER_CODE:
			{
				Field<FIELD_TYPE_DMU_MEMBER_CODE> const & myself = static_cast<Field<FIELD_TYPE_DMU_MEMBER_CODE> const &>(*this);
				return myself.GetValueReference();
			}
			break;

		case FIELD_TYPE_DMU_MEMBER_DESCRIPTION:
			{
				Field<FIELD_TYPE_DMU_MEMBER_DESCRIPTION> const & myself = static_cast<Field<FIELD_TYPE_DMU_MEMBER_DESCRIPTION> const &>(*this);
				return myself.GetValueReference();
			}
			break;

		case FIELD_TYPE_FK_TO_DMU_CATEGORY_UUID:
			{
				Field<FIELD_TYPE_FK_TO_DMU_CATEGORY_UUID> const & myself = static_cast<Field<FIELD_TYPE_FK_TO_DMU_CATEGORY_UUID> const &>(*this);
				return myself.GetValueReference();
			}
			break;

		case FIELD_TYPE_TIMERANGE_STRING:
			{
				Field<FIELD_TYPE_TIMERANGE_STRING> const & myself = static_cast<Field<FIELD_TYPE_TIMERANGE_STRING> const &>(*this);
				return myself.GetValueReference();
			}
			break;

		case FIELD_TYPE_DMU_PRIMARY_KEY_AND_TIMERANGE_STRING:
			{
				Field<FIELD_TYPE_DMU_PRIMARY_KEY_AND_TIMERANGE_STRING> const & myself = static_cast<Field<FIELD_TYPE_DMU_PRIMARY_KEY_AND_TIMERANGE_STRING> const &>(*this);
				return myself.GetValueReference();
			}
			break;

		default:
			{
				boost::format msg("Unknown field data type.");
				throw NewGeneException() << newgene_error_description(msg.str());
			}
			break;

	}

	return std::string();

}

void BaseField::SetValueInt64(std::int64_t const & val)
{

	if (!IsFieldTypeInt64(field_type))
	{
		boost::format msg("Field data type is not convertible to int64.");
		throw NewGeneException() << newgene_error_description(msg.str());
	}

	switch (field_type)
	{

		case FIELD_TYPE_INT64:
			{
				Field<FIELD_TYPE_INT64> & myself = static_cast<Field<FIELD_TYPE_INT64> &>(*this);
				myself.SetValue(val);
			}
			break;

		case FIELD_TYPE_UINT64:
			{
				Field<FIELD_TYPE_UINT64> & myself = static_cast<Field<FIELD_TYPE_UINT64> &>(*this);
				myself.SetValue(val);
			}
			break;

		case FIELD_TYPE_TIMESTAMP:
			{
				Field<FIELD_TYPE_TIMESTAMP> & myself = static_cast<Field<FIELD_TYPE_TIMESTAMP> &>(*this);
				myself.SetValue(val);
			}
			break;

		case FIELD_TYPE_TIME_RANGE_OUTPUT_START_DATETIME:
			{
				Field<FIELD_TYPE_TIME_RANGE_OUTPUT_START_DATETIME> & myself = static_cast<Field<FIELD_TYPE_TIME_RANGE_OUTPUT_START_DATETIME> &>(*this);
				myself.SetValue(val);
			}
			break;

		case FIELD_TYPE_TIME_RANGE_OUTPUT_END_DATETIME:
			{
				Field<FIELD_TYPE_TIME_RANGE_OUTPUT_END_DATETIME> & myself = static_cast<Field<FIELD_TYPE_TIME_RANGE_OUTPUT_END_DATETIME> &>(*this);
				myself.SetValue(val);
			}
			break;

		default:
			{
				boost::format msg("Unknown field data type.");
				throw NewGeneException() << newgene_error_description(msg.str());
			}
			break;

	}

}

void BaseField::SetValueInt32(std::int32_t const & val)
{

	if (!IsFieldTypeInt32(field_type))
	{
		boost::format msg("Field data type is not convertible to int32.");
		throw NewGeneException() << newgene_error_description(msg.str());
	}

	switch (field_type)
	{

		case FIELD_TYPE_INT32:
			{
				Field<FIELD_TYPE_INT32> & myself = static_cast<Field<FIELD_TYPE_INT32> &>(*this);
				myself.SetValue(val);
			}
			break;

		case FIELD_TYPE_UINT32:
			{
				Field<FIELD_TYPE_UINT32> & myself = static_cast<Field<FIELD_TYPE_UINT32> &>(*this);
				myself.SetValue(val);
			}
			break;

		case FIELD_TYPE_DMU_MEMBER_UUID_NUMERIC:
			{
				Field<FIELD_TYPE_DMU_MEMBER_UUID_NUMERIC> & myself = static_cast<Field<FIELD_TYPE_DMU_MEMBER_UUID_NUMERIC> &>(*this);
				myself.SetValue(val);
			}
			break;

		case FIELD_TYPE_DAY:
			{
				Field<FIELD_TYPE_DAY> & myself = static_cast<Field<FIELD_TYPE_DAY> &>(*this);
				myself.SetValue(val);
			}
			break;

		case FIELD_TYPE_MONTH:
			{
				Field<FIELD_TYPE_MONTH> & myself = static_cast<Field<FIELD_TYPE_MONTH> &>(*this);
				myself.SetValue(val);
			}
			break;

		case FIELD_TYPE_YEAR:
			{
				Field<FIELD_TYPE_YEAR> & myself = static_cast<Field<FIELD_TYPE_YEAR> &>(*this);
				myself.SetValue(val);
			}
			break;

		case FIELD_TYPE_DMU_PRIMARY_KEY_AND_DAY:
			{
				Field<FIELD_TYPE_DMU_PRIMARY_KEY_AND_DAY> & myself = static_cast<Field<FIELD_TYPE_DMU_PRIMARY_KEY_AND_DAY> &>(*this);
				myself.SetValue(val);
			}
			break;

		case FIELD_TYPE_DMU_PRIMARY_KEY_AND_MONTH:
			{
				Field<FIELD_TYPE_DMU_PRIMARY_KEY_AND_MONTH> & myself = static_cast<Field<FIELD_TYPE_DMU_PRIMARY_KEY_AND_MONTH> &>(*this);
				myself.SetValue(val);
			}
			break;

		case FIELD_TYPE_DMU_PRIMARY_KEY_AND_YEAR:
			{
				Field<FIELD_TYPE_DMU_PRIMARY_KEY_AND_YEAR> & myself = static_cast<Field<FIELD_TYPE_DMU_PRIMARY_KEY_AND_YEAR> &>(*this);
				myself.SetValue(val);
			}
			break;

		default:
			{
				boost::format msg("Unknown field data type.");
				throw NewGeneException() << newgene_error_description(msg.str());
			}
			break;

	}

}

void BaseField::SetValueDouble(double const & val)
{

	if (!IsFieldTypeFloat(field_type))
	{
		boost::format msg("Field data type is not convertible to double.");
		throw NewGeneException() << newgene_error_description(msg.str());
	}

	switch (field_type)
	{

		case FIELD_TYPE_FLOAT:
			{
				Field<FIELD_TYPE_FLOAT> & myself = static_cast<Field<FIELD_TYPE_FLOAT> &>(*this);
				myself.SetValue(val);
			}
			break;

		default:
			{
				boost::format msg("Unknown field data type.");
				throw NewGeneException() << newgene_error_description(msg.str());
			}
			break;

	}

}

void BaseField::SetValueString(std::string const & val)
{

	if (!IsFieldTypeString(field_type))
	{
		boost::format msg("Field data type is not convertible to a string.");
		throw NewGeneException() << newgene_error_description(msg.str());
	}

	switch (field_type)
	{

		case FIELD_TYPE_STRING_FIXED:
			{
				Field<FIELD_TYPE_STRING_FIXED> & myself = static_cast<Field<FIELD_TYPE_STRING_FIXED> &>(*this);
				myself.SetValue(val);
			}
			break;

		case FIELD_TYPE_STRING_VAR:
			{
				Field<FIELD_TYPE_STRING_VAR> & myself = static_cast<Field<FIELD_TYPE_STRING_VAR> &>(*this);
				myself.SetValue(val);
			}
			break;

		case FIELD_TYPE_UUID:
			{
				Field<FIELD_TYPE_UUID> & myself = static_cast<Field<FIELD_TYPE_UUID> &>(*this);
				myself.SetValue(val);
			}
			break;

		case FIELD_TYPE_UUID_FOREIGN:
			{
				Field<FIELD_TYPE_UUID_FOREIGN> & myself = static_cast<Field<FIELD_TYPE_UUID_FOREIGN> &>(*this);
				myself.SetValue(val);
			}
			break;

		case FIELD_TYPE_STRING_CODE:
			{
				Field<FIELD_TYPE_STRING_CODE> & myself = static_cast<Field<FIELD_TYPE_STRING_CODE> &>(*this);
				myself.SetValue(val);
			}
			break;

		case FIELD_TYPE_STRING_LONGHAND:
			{
				Field<FIELD_TYPE_STRING_LONGHAND> & myself = static_cast<Field<FIELD_TYPE_STRING_LONGHAND> &>(*this);
				myself.SetValue(val);
			}
			break;

		case FIELD_TYPE_NOTES_1:
			{
				Field<FIELD_TYPE_NOTES_1> & myself = static_cast<Field<FIELD_TYPE_NOTES_1> &>(*this);
				myself.SetValue(val);
			}
			break;

		case FIELD_TYPE_NOTES_2:
			{
				Field<FIELD_TYPE_NOTES_2> & myself = static_cast<Field<FIELD_TYPE_NOTES_2> &>(*this);
				myself.SetValue(val);
			}
			break;

		case FIELD_TYPE_NOTES_3:
			{
				Field<FIELD_TYPE_NOTES_3> & myself = static_cast<Field<FIELD_TYPE_NOTES_3> &>(*this);
				myself.SetValue(val);
			}
			break;

		case FIELD_TYPE_DMU_MEMBER_UUID:
			{
				Field<FIELD_TYPE_DMU_MEMBER_UUID> & myself = static_cast<Field<FIELD_TYPE_DMU_MEMBER_UUID> &>(*this);
				myself.SetValue(val);
			}
			break;

		case FIELD_TYPE_DMU_MEMBER_UUID_STRING:
			{
				Field<FIELD_TYPE_DMU_MEMBER_UUID_STRING> & myself = static_cast<Field<FIELD_TYPE_DMU_MEMBER_UUID_STRING> &>(*this);
				myself.SetValue(val);
			}
			break;

		case FIELD_TYPE_DMU_MEMBER_CODE:
			{
				Field<FIELD_TYPE_DMU_MEMBER_CODE> & myself = static_cast<Field<FIELD_TYPE_DMU_MEMBER_CODE> &>(*this);
				myself.SetValue(val);
			}
			break;

		case FIELD_TYPE_DMU_MEMBER_DESCRIPTION:
			{
				Field<FIELD_TYPE_DMU_MEMBER_DESCRIPTION> & myself = static_cast<Field<FIELD_TYPE_DMU_MEMBER_DESCRIPTION> &>(*this);
				myself.SetValue(val);
			}
			break;

		case FIELD_TYPE_FK_TO_DMU_CATEGORY_UUID:
			{
				Field<FIELD_TYPE_FK_TO_DMU_CATEGORY_UUID> & myself = static_cast<Field<FIELD_TYPE_FK_TO_DMU_CATEGORY_UUID> &>(*this);
				myself.SetValue(val);
			}
			break;

		case FIELD_TYPE_TIMERANGE_STRING:
			{
				Field<FIELD_TYPE_TIMERANGE_STRING> & myself = static_cast<Field<FIELD_TYPE_TIMERANGE_STRING> &>(*this);
				myself.SetValue(val);
			}
			break;

		case FIELD_TYPE_DMU_PRIMARY_KEY_AND_TIMERANGE_STRING:
			{
				Field<FIELD_TYPE_DMU_PRIMARY_KEY_AND_TIMERANGE_STRING> & myself = static_cast<Field<FIELD_TYPE_DMU_PRIMARY_KEY_AND_TIMERANGE_STRING> &>(*this);
				myself.SetValue(val);
			}
			break;

		default:
			{
				boost::format msg("Unknown field data type.");
				throw NewGeneException() << newgene_error_description(msg.str());
			}
			break;

	}

}

std::string GetFieldDataTypeAsString(FIELD_TYPE const & field_type)
{

	std::string retStr;

	if (IsFieldTypeInt32(field_type))
	{
		retStr = "INT32";
	}
	else if (IsFieldTypeInt64(field_type))
	{
		retStr = "INT64";
	}
	else if (IsFieldTypeFloat(field_type))
	{
		retStr = "FLOAT";
	}
	else if (IsFieldTypeString(field_type))
	{
		retStr = "STRING";
	}
	else
	{
		boost::format msg("Invalid field type in GetFieldDataTypeAsString");
		throw NewGeneException() << newgene_error_description(msg.str());
	}

	return retStr;

}

std::string GetSqlLiteFieldDataTypeAsString(FIELD_TYPE const & field_type)
{

	std::string retStr;

	if (IsFieldTypeInt(field_type))
	{
		retStr = "INTEGER";
	}
	else if (IsFieldTypeFloat(field_type))
	{
		retStr = "REAL";
	}
	else if (IsFieldTypeString(field_type))
	{
		retStr = "TEXT";
	}
	else
	{
		boost::format msg("Invalid field type in GetSqlLiteFieldDataTypeAsString");
		throw NewGeneException() << newgene_error_description(msg.str());
	}

	return retStr;

}

void FieldFactory(FIELD_TYPE field_type, std::string field_name, std::shared_ptr<BaseField> & field)
{

	switch (field_type)
	{
		case FIELD_TYPE_INT32:
			{
				field = std::make_shared<Field<FIELD_TYPE_INT32>>(field_name);
			}
			break;

		case FIELD_TYPE_INT64:
			{
				field = std::make_shared<Field<FIELD_TYPE_INT64>>(field_name);
			}
			break;

		case FIELD_TYPE_UINT32:
			{
				field = std::make_shared<Field<FIELD_TYPE_UINT32>>(field_name);
			}
			break;

		case FIELD_TYPE_UINT64:
			{
				field = std::make_shared<Field<FIELD_TYPE_UINT64>>(field_name);
			}
			break;

		case FIELD_TYPE_STRING_FIXED:
			{
				field = std::make_shared<Field<FIELD_TYPE_STRING_FIXED>>(field_name);
			}
			break;

		case FIELD_TYPE_STRING_VAR:
			{
				field = std::make_shared<Field<FIELD_TYPE_STRING_VAR>>(field_name);
			}
			break;

		case FIELD_TYPE_FLOAT:
			{
				field = std::make_shared<Field<FIELD_TYPE_FLOAT>>(field_name);
			}
			break;

		case FIELD_TYPE_TIMESTAMP:
			{
				field = std::make_shared<Field<FIELD_TYPE_TIMESTAMP>>(field_name);
			}
			break;

		case FIELD_TYPE_UUID:
			{
				field = std::make_shared<Field<FIELD_TYPE_UUID>>(field_name);
			}
			break;

		case FIELD_TYPE_UUID_FOREIGN:
			{
				field = std::make_shared<Field<FIELD_TYPE_UUID_FOREIGN>>(field_name);
			}
			break;

		case FIELD_TYPE_STRING_CODE:
			{
				field = std::make_shared<Field<FIELD_TYPE_STRING_CODE>>(field_name);
			}
			break;

		case FIELD_TYPE_STRING_LONGHAND:
			{
				field = std::make_shared<Field<FIELD_TYPE_STRING_LONGHAND>>(field_name);
			}
			break;

		case FIELD_TYPE_TIME_RANGE:
			{
				field = std::make_shared<Field<FIELD_TYPE_TIME_RANGE>>(field_name);
			}
			break;

		case FIELD_TYPE_NOTES_1:
			{
				field = std::make_shared<Field<FIELD_TYPE_NOTES_1>>(field_name);
			}
			break;

		case FIELD_TYPE_NOTES_2:
			{
				field = std::make_shared<Field<FIELD_TYPE_NOTES_2>>(field_name);
			}
			break;

		case FIELD_TYPE_NOTES_3:
			{
				field = std::make_shared<Field<FIELD_TYPE_NOTES_3>>(field_name);
			}
			break;

		case FIELD_TYPE_DMU_MEMBER_UUID:
			{
				field = std::make_shared<Field<FIELD_TYPE_DMU_MEMBER_UUID>>(field_name);
			}
			break;

		case FIELD_TYPE_DMU_MEMBER_UUID_NUMERIC:
			{
				field = std::make_shared<Field<FIELD_TYPE_DMU_MEMBER_UUID_NUMERIC>>(field_name);
			}
			break;

		case FIELD_TYPE_DMU_MEMBER_UUID_STRING:
			{
				field = std::make_shared<Field<FIELD_TYPE_DMU_MEMBER_UUID_STRING>>(field_name);
			}
			break;

		case FIELD_TYPE_DMU_MEMBER_CODE:
			{
				field = std::make_shared<Field<FIELD_TYPE_DMU_MEMBER_CODE>>(field_name);
			}
			break;

		case FIELD_TYPE_DMU_MEMBER_DESCRIPTION:
			{
				field = std::make_shared<Field<FIELD_TYPE_DMU_MEMBER_DESCRIPTION>>(field_name);
			}
			break;

		case FIELD_TYPE_FK_TO_DMU_CATEGORY_UUID:
			{
				field = std::make_shared<Field<FIELD_TYPE_FK_TO_DMU_CATEGORY_UUID>>(field_name);
			}
			break;

		case FIELD_TYPE_TIME_RANGE_OUTPUT_START_DATETIME:
			{
				field = std::make_shared<Field<FIELD_TYPE_TIME_RANGE_OUTPUT_START_DATETIME>>(field_name);
			}
			break;

		case FIELD_TYPE_TIME_RANGE_OUTPUT_END_DATETIME:
			{
				field = std::make_shared<Field<FIELD_TYPE_TIME_RANGE_OUTPUT_END_DATETIME>>(field_name);
			}
			break;

		case FIELD_TYPE_DAY:
			{
				field = std::make_shared<Field<FIELD_TYPE_DAY>>(field_name);
			}
			break;

		case FIELD_TYPE_MONTH:
			{
				field = std::make_shared<Field<FIELD_TYPE_MONTH>>(field_name);
			}
			break;

		case FIELD_TYPE_YEAR:
			{
				field = std::make_shared<Field<FIELD_TYPE_YEAR>>(field_name);
			}
			break;

		case FIELD_TYPE_TIMERANGE_STRING:
			{
				field = std::make_shared<Field<FIELD_TYPE_TIMERANGE_STRING>>(field_name);
			}
			break;

		case FIELD_TYPE_DMU_PRIMARY_KEY_AND_DAY:
			{
				field = std::make_shared<Field<FIELD_TYPE_DMU_PRIMARY_KEY_AND_DAY>>(field_name);
			}
			break;

		case FIELD_TYPE_DMU_PRIMARY_KEY_AND_MONTH:
			{
				field = std::make_shared<Field<FIELD_TYPE_DMU_PRIMARY_KEY_AND_MONTH>>(field_name);
			}
			break;

		case FIELD_TYPE_DMU_PRIMARY_KEY_AND_YEAR:
			{
				field = std::make_shared<Field<FIELD_TYPE_DMU_PRIMARY_KEY_AND_YEAR>>(field_name);
			}
			break;

		case FIELD_TYPE_DMU_PRIMARY_KEY_AND_TIMERANGE_STRING:
			{
				field = std::make_shared<Field<FIELD_TYPE_DMU_PRIMARY_KEY_AND_TIMERANGE_STRING>>(field_name);
			}
			break;

		default:
			{
				boost::format msg("Invalid data type in FieldFactory.");
				throw NewGeneException() << newgene_error_description(msg.str());
			}
			break;

	}

}

bool ValidateFieldData(BaseField & theField)
{

	static std::string errorMsg;

	bool valid = true;

	if (theField.GetType() == FIELD_TYPE_DMU_MEMBER_CODE)
	{
		std::string test_field = theField.GetStringRef();
		valid = Validation::ValidateDmuCode(test_field, errorMsg);

		if (valid && test_field != theField.GetStringRef())
		{
			theField.SetValueString(test_field);
		}
	}
	else if (theField.GetType() == FIELD_TYPE_DMU_MEMBER_DESCRIPTION)
	{
		std::string test_field = theField.GetStringRef();
		valid = Validation::ValidateDmuMemberDescription(test_field, errorMsg);

		if (valid && test_field != theField.GetStringRef())
		{
			theField.SetValueString(test_field);
		}
	}
	else if (theField.GetType() == FIELD_TYPE_DMU_MEMBER_UUID)
	{
		std::string test_field = theField.GetStringRef();
		valid = Validation::ValidateDmuMemberUUID(test_field, false, errorMsg);

		if (valid && test_field != theField.GetStringRef())
		{
			theField.SetValueString(test_field);
		}
	}
	else if (theField.GetType() == FIELD_TYPE_DMU_MEMBER_UUID_NUMERIC)
	{
		std::string test_field = theField.GetStringRef();
		valid = Validation::ValidateDmuMemberUUID(test_field, true, errorMsg);
	}
	else if (theField.GetType() == FIELD_TYPE_DMU_MEMBER_UUID_STRING)
	{
		std::string test_field = theField.GetStringRef();
		valid = Validation::ValidateDmuMemberUUID(test_field, false, errorMsg);

		if (valid && test_field != theField.GetStringRef())
		{
			theField.SetValueString(test_field);
		}
	}
	else if (IsFieldTypeString(theField.GetType()))
	{
		// Perform validation on the field
		std::string test_field_name = theField.GetStringRef();
		valid = Validation::ValidateGenericStringField(test_field_name, errorMsg, false);

		if (valid && test_field_name != theField.GetStringRef())
		{
			theField.SetValueString(test_field_name);
		}
	}

	// Integers and floats have already been validated

	// Note that time fields - year, month, day - will be validated by the time range mapper,
	// which will throw an exception if they do not represent valid dates

	return valid;

}
