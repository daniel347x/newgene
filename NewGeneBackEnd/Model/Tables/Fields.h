#ifndef FIELDS_H
#define FIELDS_H

#include "FieldTypes.h"
#include <tuple>
#include <memory>

#ifndef Q_MOC_RUN
#	include <boost/format.hpp>
#	include <boost/spirit/home/support/string_traits.hpp>
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
	virtual FIELD_TYPE GetType() const { return FIELD_TYPE_UNKNOWN; };
	virtual std::string GetName() const { return ""; };
	virtual std::int64_t const & GetInt64Ref() const { boost::format msg("Calling data access on base class field!"); throw NewGeneException() << newgene_error_description(msg.str()); }
	virtual std::int32_t const & GetInt32Ref() const { boost::format msg("Calling data access on base class field!"); throw NewGeneException() << newgene_error_description(msg.str()); }
	virtual double const & GetDoubleRef() const { boost::format msg("Calling data access on base class field!"); throw NewGeneException() << newgene_error_description(msg.str()); }
	virtual std::string const & GetStringRef() const { boost::format msg("Calling data access on base class field!"); throw NewGeneException() << newgene_error_description(msg.str()); }
	virtual std::int64_t GetInt64() const { boost::format msg("Calling data access on base class field!"); throw NewGeneException() << newgene_error_description(msg.str()); }
	virtual std::int32_t GetInt32() const { boost::format msg("Calling data access on base class field!"); throw NewGeneException() << newgene_error_description(msg.str()); }
	virtual double GetDouble() const { boost::format msg("Calling data access on base class field!"); throw NewGeneException() << newgene_error_description(msg.str()); }
	virtual std::string GetString() const { boost::format msg("Calling data access on base class field!"); throw NewGeneException() << newgene_error_description(msg.str()); }
	BaseField(bool)
	{

	}
private:
    BaseField(BaseField const &) {}
};

template <FIELD_TYPE THE_FIELD_TYPE>
class Field : public BaseField
{

public:

	Field<THE_FIELD_TYPE>(std::string const field_name, FieldValue<THE_FIELD_TYPE> const & field_value = FieldValue<THE_FIELD_TYPE>(FieldTypeTraits<THE_FIELD_TYPE>::default_))
		: BaseField(true)
		, data(std::make_tuple(THE_FIELD_TYPE, field_name, field_value))
	{

	}

	FIELD_TYPE GetType() const
	{
		return THE_FIELD_TYPE;
	}

	std::string GetName() const
	{
		return std::get<1>(data);
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

	// Compile-time reflection will cause the following function to not be compiled
	// if the data type corresponding to this derived class is not of the correct type to support this function
	// (SFINAE)
	// Note regarding SFINAE: See http://stackoverflow.com/a/6972771/368896 -
	// Stage 1 of compilation: The class is instantiated with a brick-and-mortar type substituted for THE_FIELD_TYPE;
	//     at this time the template function here is NOT instantiated, but rather PREPARED for instantiation by
	//     substituting the default type of INNER_FIELD_TYPE to THE_FIELD_TYPE
	// Stage 2 of compilation: When the member function is called, INNER_FIELD_TYPE is substituted into the function
	//     in order to locate a valid overload.  This substitution will fail in the case indicated by the test.
	//     This will result in a compile error.  However, the function should never BE called unless the underlying
	//     data type matches, because the CALLING function calls "IsFieldTypeInt64(GetType())" (etc) before calling this function.
	// Note that if this were NOT a template function, then it would always be instantiated and would result in a compile error.
	template<FIELD_TYPE INNER_FIELD_TYPE = THE_FIELD_TYPE>
	typename std::enable_if<std::is_integral<typename FieldTypeTraits<INNER_FIELD_TYPE>::type>::value, std::int64_t>::type
	const & GetInt64Ref() const
	{
		return boost::lexical_cast<std::int64_t const &>(std::get<2>(data).value);
	}

	template<FIELD_TYPE INNER_FIELD_TYPE = THE_FIELD_TYPE>
	typename std::enable_if<std::is_integral<typename FieldTypeTraits<INNER_FIELD_TYPE>::type>::value, std::int32_t>::type
	const & GetInt32Ref() const
	{
		return boost::lexical_cast<std::int32_t const &>(std::get<2>(data).value);
	}

	template<FIELD_TYPE INNER_FIELD_TYPE = THE_FIELD_TYPE>
	typename std::enable_if<std::is_floating_point<typename FieldTypeTraits<INNER_FIELD_TYPE>::type>::value, double>::type
	const & GetDoubleRef() const
	{
		return boost::lexical_cast<double const &>(std::get<2>(data).value);
	}

	template<FIELD_TYPE INNER_FIELD_TYPE = THE_FIELD_TYPE>
	typename std::enable_if<boost::spirit::traits::is_string<typename FieldTypeTraits<INNER_FIELD_TYPE>::type>::value, std::string>::type
	const & GetStringRef() const
	{
		if (!IsFieldTypeString(GetType()))
		{
			boost::format msg("Trying to retrieve invalid data type from field!");
			throw NewGeneException() << newgene_error_description(msg.str());
		}
		return boost::lexical_cast<std::string const &>(std::get<2>(data).value);
	}

	template<FIELD_TYPE INNER_FIELD_TYPE = THE_FIELD_TYPE>
	typename std::enable_if<std::is_integral<typename FieldTypeTraits<INNER_FIELD_TYPE>::type>::value, std::int64_t>::type
	GetInt64() const
	{
		if (!IsFieldTypeInt(GetType()))
		{
			boost::format msg("Trying to retrieve invalid data type from field!");
			throw NewGeneException() << newgene_error_description(msg.str());
		}
		return boost::lexical_cast<std::int64_t>(std::get<2>(data).value);
	}

	template<FIELD_TYPE INNER_FIELD_TYPE = THE_FIELD_TYPE>
	typename std::enable_if<std::is_integral<typename FieldTypeTraits<INNER_FIELD_TYPE>::type>::value, std::int32_t>::type
	GetInt32() const
	{
		if (!IsFieldTypeInt(GetType()))
		{
			boost::format msg("Trying to retrieve invalid data type from field!");
			throw NewGeneException() << newgene_error_description(msg.str());
		}
		return boost::lexical_cast<std::int32_t>(std::get<2>(data).value);
	}

	template<FIELD_TYPE INNER_FIELD_TYPE = THE_FIELD_TYPE>
	typename std::enable_if<std::is_floating_point<typename FieldTypeTraits<INNER_FIELD_TYPE>::type>::value, double>::type
	GetDouble() const
	{
		if (!IsFieldTypeFloat(GetType()))
		{
			boost::format msg("Trying to retrieve invalid data type from field!");
			throw NewGeneException() << newgene_error_description(msg.str());
		}
		return boost::lexical_cast<double>(std::get<2>(data).value);
	}

	template<FIELD_TYPE INNER_FIELD_TYPE = THE_FIELD_TYPE>
	typename std::enable_if<boost::spirit::traits::is_string<typename FieldTypeTraits<INNER_FIELD_TYPE>::type>::value, std::string>::type
	GetString() const
	{
		if (!IsFieldTypeString(GetType()))
		{
			boost::format msg("Trying to retrieve invalid data type from field!");
			throw NewGeneException() << newgene_error_description(msg.str());
		}
		return boost::lexical_cast<std::string>(std::get<2>(data).value);
	}


	typename FieldData<THE_FIELD_TYPE>::type data;


private:
	 
    Field<THE_FIELD_TYPE>(Field<THE_FIELD_TYPE> const &) {}

};

#endif
