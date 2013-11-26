#ifndef SCHEMA_H
#define SCHEMA_H

#include <utility>
#include <string>
#include <vector>
#include <memory>

#include "FieldTypes.h"

class SchemaEntry
{

	public:

		SchemaEntry()
		{
		}

		SchemaEntry(FIELD_TYPE const field_type_, std::string const & field_name_ = std::string(), bool const required_ = false)
			: field_type(field_type_)
			, field_name(field_name_)
			, required(required_)
		{
		}

		SchemaEntry(std::string const & dmu_category_string_code_, FIELD_TYPE const field_type_, std::string const & field_name_ = std::string(), bool const required_ = false)
			: field_type(field_type_)
			, field_name(field_name_)
			, dmu_category_string_code(std::make_shared<std::string>(dmu_category_string_code_))
            , required(required_)
        {
		}

		SchemaEntry(SchemaEntry const & rhs)
			: field_type(rhs.field_type)
			, field_name(rhs.field_name)
			, dmu_category_string_code(rhs.dmu_category_string_code)
            , required(rhs.required)
        {
		}

	FIELD_TYPE field_type;
	std::string field_name;
	std::shared_ptr<std::string> dmu_category_string_code; // primary key field
	bool required;

};

typedef std::vector<SchemaEntry> SchemaVector;

class Schema
{

	public:

		Schema();
		Schema(Schema const & rhs);

	public:

		SchemaVector schema;

};

#endif
