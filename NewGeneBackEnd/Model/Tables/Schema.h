#ifndef SCHEMA_H
#define SCHEMA_H

#include <utility>
#include <string>
#include <vector>

#include "FieldTypes.h"

class SchemaEntry
{

	public:

		SchemaEntry()
		{
		}

	FIELD_TYPE field_type;
	std::string field_name;

};

typedef std::vector<SchemaEntry> SchemaVector;

class Schema
{

	public:

		Schema();

	public:

		SchemaVector schema;

};

#endif
