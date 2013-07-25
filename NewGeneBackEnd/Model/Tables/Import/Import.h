#ifndef IMPORT_H
#define IMPORT_H

#include "..\Fields.h"
#include "..\Schema.h"
#include <vector>
#include <memory>

#ifndef Q_MOC_RUN
#	include <boost/filesystem.hpp>
#endif

class FieldMapping
{

	public:

		enum FIELD_MAPPING_TYPE
		{
			FIELD_MAPPING_TYPE__TIME_RANGE
		};

		FieldTypeEntries input_file_fields;
		FieldTypeEntries output_table_fields;

};

class RowFieldMapping : public FieldMapping
{
};

class TimeRangeFieldMapping : public RowFieldMapping
{
	
	public:

		enum TIME_RANGE_FIELD_MAPPING_TYPE
		{
			TIME_RANGE_FIELD_MAPPING_TYPE__DAY_MONTH_YEAR
		};

		TimeRangeFieldMapping(TIME_RANGE_FIELD_MAPPING_TYPE const time_range_type_);
		TIME_RANGE_FIELD_MAPPING_TYPE time_range_type;
		
};

class ImportDefinition
{

	public:

		enum IMPORT_TYPE
		{
			  IMPORT_TYPE__INPUT_MODEL
			, IMPORT_TYPE__OUTPUT_MODEL
		};

		typedef std::vector<std::shared_ptr<FieldMapping>> ImportMappings;

		ImportDefinition();

		ImportMappings mappings;
		boost::filesystem::path input_file;
		IMPORT_TYPE input_or_output;
		std::string table_name;
		bool first_row_is_header_row;
		Schema input_schema;
		Schema output_schema;

};



#endif
