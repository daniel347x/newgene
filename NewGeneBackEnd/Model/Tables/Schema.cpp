#include "Schema.h"

#include <algorithm>
#include <vector>

Schema::Schema()
{
}

Schema::Schema(Schema const & rhs)
	: schema(rhs.schema)
{
}

void Schema::ReorderAccToColumnNames(std::vector<std::string> const & colnames)
{

	// Sort our schema so that it matches the order of the columns in the input file
	std::sort(schema.begin(), schema.end(), [&colnames](SchemaEntry const & lhs, SchemaEntry const & rhs)
	{
		auto poslhs = std::find(colnames.cbegin(), colnames.cend(), lhs.field_name);
		auto posrhs = std::find(colnames.cbegin(), colnames.cend(), rhs.field_name);
		return poslhs < posrhs;
	});

	validcols.clear();
	std::for_each(colnames.cbegin(), colnames.cend(), [this](std::string const & colname)
	{
		auto pos = std::find_if(schema.cbegin(), schema.cend(), [this, &colname](SchemaEntry const & schema_entry)
		{
			if (schema_entry.field_name == colname)
			{
				return true;
			}

			return false;
		});

		if (pos == schema.cend())
		{
			// A column is present in the input file, but not present in our list of desired input columns
			validcols.push_back(false);
		}
		else
		{
			// We've found a column in the input file that is in our list of desired input columns
			validcols.push_back(true);
		}
	});

}
