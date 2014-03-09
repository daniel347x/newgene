#ifndef SCHEMA_H
#define SCHEMA_H

#include <utility>
#include <string>
#include <vector>
#include <memory>

#include "FieldTypes.h"
#include "../../Utilities/WidgetIdentifier.h"

#ifndef Q_MOC_RUN
#	include <boost/dynamic_bitset.hpp>
#endif

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
			, field_description(rhs.field_description)
			, dmu_category_string_code(rhs.dmu_category_string_code)
            , required(rhs.required)
        {
		}

		bool IsPrimaryKey() const
		{
			if (!dmu_category_string_code || dmu_category_string_code->empty())
			{
				return false;
			}
			return true;
		}

		FIELD_TYPE field_type;
		std::string field_name;
		std::shared_ptr<std::string> dmu_category_string_code; // primary key field
		//WidgetInstanceIdentifier dmu_category; // when needed, bring this in
		std::string field_description;
		bool required;

};

typedef std::vector<SchemaEntry> SchemaVector;

class Schema
{

	public:

		Schema();
		Schema(Schema const & rhs);

		void ReorderAccToColumnNames(std::vector<std::string> const & colnames);

	public:

		SchemaVector schema;
		boost::dynamic_bitset<> validcols;

};

#endif
