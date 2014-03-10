#ifndef FIELDS_H
#define FIELDS_H

#include "FieldTypes.h"
#include <tuple>
#include <memory>

#ifndef Q_MOC_RUN
#	include <boost/format.hpp>
#	include <boost/spirit/home/support/string_traits.hpp>
#	include <boost/lexical_cast.hpp>
#endif

#include "../../Utilities/NewGeneException.h"

template<FIELD_TYPE THE_FIELD_TYPE>
class FieldValue
{
	public:
		FieldValue()
		{

		}
		FieldValue(typename FieldTypeTraits<THE_FIELD_TYPE>::type const & value_)
			: value(value_)
		{

		}
		typename FieldTypeTraits<THE_FIELD_TYPE>::type value;
};

template <FIELD_TYPE THE_FIELD_TYPE>
struct FieldData
{
	typedef std::tuple<FIELD_TYPE, std::string const, FieldValue<THE_FIELD_TYPE>> type;
};

class BaseField
{

	public:

		BaseField(FIELD_TYPE const the_field_type, std::string const & the_field_name)
			: field_type(the_field_type)
			, field_name(the_field_name)
		{
		}

		FIELD_TYPE GetType() const { return field_type; }

		std::string GetName() const { return field_name; }

		std::int64_t const & GetInt64Ref() const;
		std::int32_t const & GetInt32Ref() const;
		std::string const & GetStringRef() const;
		std::int64_t GetInt64() const;
		std::int32_t GetInt32() const;
		double GetDouble() const;
		std::string GetString() const;

		void SetValueInt64(std::int64_t const & val);
		void SetValueInt32(std::int32_t const & val);
		void SetValueDouble(double const & val);
		void SetValueString(std::string const & val);

	protected:

		BaseField(BaseField const &) : field_type(FIELD_TYPE_UNKNOWN), field_name(std::string()) {}
		FIELD_TYPE const field_type;
		std::string const field_name;

};

template <FIELD_TYPE THE_FIELD_TYPE>
class Field : public BaseField
{

	public:

		Field<THE_FIELD_TYPE>(std::string const & field_name, FieldValue<THE_FIELD_TYPE> const & field_value = FieldValue<THE_FIELD_TYPE>(FieldTypeTraits<THE_FIELD_TYPE>::default_))
			: BaseField(THE_FIELD_TYPE, field_name)
			, data(std::make_tuple(field_type, field_name, field_value))
		{

		}

		inline typename FieldTypeTraits<THE_FIELD_TYPE>::type GetValue() const
		{
			return std::get<2>(data).value;
		}

		inline typename FieldTypeTraits<THE_FIELD_TYPE>::type const & GetValueReference() const
		{
			return std::get<2>(data).value;
		}

		inline typename FieldTypeTraits<THE_FIELD_TYPE>::type & GetValueReference()
		{
			return std::get<2>(data).value;
		}

		inline void SetValue(typename FieldTypeTraits<THE_FIELD_TYPE>::type const & value_)
		{
			std::get<2>(data).value = value_;
		}


		typename FieldData<THE_FIELD_TYPE>::type data;


	private:

		Field<THE_FIELD_TYPE>(Field<THE_FIELD_TYPE> const &) {}

};

static std::string GetFieldDataTypeAsString(FIELD_TYPE const & field_type)
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

static std::string GetSqlLiteFieldDataTypeAsString(FIELD_TYPE const & field_type)
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

static void FieldFactory(FIELD_TYPE field_type, std::string field_name, std::shared_ptr<BaseField> & field)
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
	else
	if (theField.GetType() == FIELD_TYPE_DMU_MEMBER_DESCRIPTION)
	{
		std::string test_field = theField.GetStringRef();
		valid = Validation::ValidateDmuMemberDescription(test_field, errorMsg);
		if (valid && test_field != theField.GetStringRef())
		{
			theField.SetValueString(test_field);
		}
	}
	else
	if (theField.GetType() == FIELD_TYPE_DMU_MEMBER_UUID)
	{
		std::string test_field = theField.GetStringRef();
		valid = Validation::ValidateDmuMemberUUID(test_field, false, errorMsg);
		if (valid && test_field != theField.GetStringRef())
		{
			theField.SetValueString(test_field);
		}
	}
	else
	if (theField.GetType() == FIELD_TYPE_DMU_MEMBER_UUID_NUMERIC)
	{
		std::string test_field = theField.GetStringRef();
		valid = Validation::ValidateDmuMemberUUID(test_field, true, errorMsg);
	}
	else
	if (theField.GetType() == FIELD_TYPE_DMU_MEMBER_UUID_STRING)
	{
		std::string test_field = theField.GetStringRef();
		valid = Validation::ValidateDmuMemberUUID(test_field, false, errorMsg);
		if (valid && test_field != theField.GetStringRef())
		{
			theField.SetValueString(test_field);
		}
	}
	else
	if (IsFieldTypeString(theField.GetType()))
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

#endif
