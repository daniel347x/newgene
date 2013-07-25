#ifndef IMPORT_H
#define IMPORT_H

#include "..\Fields.h"
#include "..\Schema.h"
#include <vector>
#include <memory>

#ifndef Q_MOC_RUN
#	include <boost/filesystem.hpp>
#endif

class NameOrIndex
{

	public:

		enum NAME_OR_INDEX
		{
			  NAME
			, INDEX
		};

		NameOrIndex(NAME_OR_INDEX const name_or_index_, int const index_)
			: name_or_index(name_or_index_)
			, index(index)
		{
		}

		NameOrIndex(NAME_OR_INDEX const name_or_index_, std::string const name_)
			: name_or_index(name_or_index_)
			, name(name_)
			, index(-1)
		{
		}

		NAME_OR_INDEX name_or_index;
		int index;
		std::string name;

};

typedef std::pair<NameOrIndex, FIELD_TYPE> FieldTypeEntry;
typedef std::vector<FieldTypeEntry> FieldTypeEntries;

class FieldMapping
{

	public:

		enum FIELD_MAPPING_TYPE
		{
			  FIELD_MAPPING_TYPE__ONE_TO_ONE
			, FIELD_MAPPING_TYPE__TIME_RANGE
		};

		FIELD_MAPPING_TYPE field_mapping_type;
		FieldTypeEntries input_file_fields;
		FieldTypeEntries output_table_fields;

		virtual bool Validate() { return true; }

};

class RowFieldMapping : public FieldMapping
{
};

class OneToOneFieldMapping : public RowFieldMapping
{

	public:

		bool Validate()
		{
			return true;
		}

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
		
		bool Validate()
		{
			return true;
		}

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
		ImportDefinition(ImportDefinition const & rhs);

		ImportMappings mappings;
		boost::filesystem::path input_file;
		IMPORT_TYPE input_or_output;
		std::string table_name;
		bool first_row_is_header_row;
		Schema input_schema;
		Schema output_schema;

};

#define MAX_LINE_SIZE 32768
class Importer
{

	public:

		typedef std::vector<std::shared_ptr<BaseField>> DataFields;
		typedef std::vector<DataFields> DataBlock;

		static int const block_size = 1024;

		Importer(ImportDefinition const & import_definition_);

		bool DoImport();

		ImportDefinition import_definition;
		DataBlock input_block;
		DataBlock output_block;

	protected:

		bool ValidateMapping();
		void InitializeFields();
		void ReadBlockFromFile();
		void WriteBlockToTable();

};

#endif
