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

	switch (field_type)
	{
		case FIELD_TYPE_INT32:
		case FIELD_TYPE_UINT32:
			{
				retStr = "INT32";
			}
			break;

		case FIELD_TYPE_INT64:
		case FIELD_TYPE_UINT64:
			{
				retStr = "INT64";
			}
			break;

		case FIELD_TYPE_STRING_FIXED:
		case FIELD_TYPE_STRING_VAR:
		case FIELD_TYPE_UUID:
		case FIELD_TYPE_UUID_FOREIGN:
		case FIELD_TYPE_STRING_CODE:
		case FIELD_TYPE_STRING_LONGHAND:
		case FIELD_TYPE_TIME_RANGE:
		case FIELD_TYPE_NOTES_1:
		case FIELD_TYPE_NOTES_2:
		case FIELD_TYPE_NOTES_3:
			{
				retStr = "STRING";
			}
			break;

		case FIELD_TYPE_TIMESTAMP:
			{
				retStr = "INT64";
			}
			break;

		case FIELD_TYPE_FLOAT:
			{
				retStr = "FLOAT";
			}
			break;

		default:
			{
				retStr = "STRING";
			}
			break;
	}


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
	}

}

#endif
