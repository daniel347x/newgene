#ifndef FIELDS_H
#define FIELDS_H

#include "FieldTypes.h"

template<FIELD_TYPE THE_FIELD_TYPE>
class FieldValue
{
public:
	FieldValue()
	{

	}
	FieldValue(typename FieldTypeTraits<THE_FIELD_TYPE>::type & value_)
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
	virtual FIELD_TYPE GetType() = 0;
	virtual std::string GetName() = 0;
};

template <FIELD_TYPE THE_FIELD_TYPE>
class Field : public BaseField
{

public:

	Field(std::string const field_name, FieldValue<THE_FIELD_TYPE> const & field_value)
		: BaseField()
		, data(std::make_tuple(THE_FIELD_TYPE, field_name, field_value))

	Field(FieldData<field_type>::type const & data_)
		: BaseField()
		, data_(data_)
	{

	}

	inline FIELD_TYPE GetType()
	{
		return THE_FIELD_TYPE;
	}

	inline std::string GetName()
	{
		return data.get<1>(data);
	}

	inline typename FieldTypeTraits<THE_FIELD_TYPE>::type GetValue()
	{
		return data.get<2>(data);
	}

	inline void SetValue(typename FieldTypeTraits<THE_FIELD_TYPE>::type & value_)
	{
		data.set<2>(value_);
	}

	typename FieldData<THE_FIELD_TYPE>::type const data;

};

#endif
