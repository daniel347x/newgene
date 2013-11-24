#ifndef FIELDS_H
#define FIELDS_H

#include "FieldTypes.h"
#include <tuple>
#include <memory>

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
	BaseField(bool dummy)
	{

	}
private:
	BaseField(BaseField const & rhs) {}
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

	typename FieldData<THE_FIELD_TYPE>::type data;

private:
	 
	Field<THE_FIELD_TYPE>(Field<THE_FIELD_TYPE> const & rhs) {}

};

#endif
