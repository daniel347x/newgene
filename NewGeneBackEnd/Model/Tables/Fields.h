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

#endif
