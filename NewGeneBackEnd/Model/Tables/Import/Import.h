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

		FieldMapping()
		{

		}

		FieldMapping(FIELD_MAPPING_TYPE const field_mapping_type_)
			: field_mapping_type(field_mapping_type_)
		{

		}

		FieldMapping(FieldTypeEntry const & input_field_entry, FieldTypeEntry const & output_table_entry)
			: field_mapping_type(FIELD_MAPPING_TYPE__ONE_TO_ONE)
		{
			input_file_fields.push_back(input_field_entry);
			output_table_fields.push_back(output_table_entry);
		}

		FieldMapping(FieldMapping const & rhs)
			: field_mapping_type(rhs.field_mapping_type)
			, input_file_fields(rhs.input_file_fields)
			, output_table_fields(rhs.output_table_fields)
		{

		}

		virtual bool Validate() { return true; }

		FIELD_MAPPING_TYPE field_mapping_type;
		FieldTypeEntries input_file_fields;
		FieldTypeEntries output_table_fields;

};

class RowFieldMapping : public FieldMapping
{
	
	public:

		RowFieldMapping()
		{

		}

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
			TIME_RANGE_FIELD_MAPPING_TYPE__YEAR
		};

		TimeRangeFieldMapping()
		{

		}

		TimeRangeFieldMapping(TIME_RANGE_FIELD_MAPPING_TYPE const time_range_type_);
		
		bool Validate()
		{
			return true;
		}

		TIME_RANGE_FIELD_MAPPING_TYPE time_range_type;

};

class Table_basemost;

class ImportDefinition
{

	public:

		enum IMPORT_TYPE
		{
			  IMPORT_TYPE__INPUT_MODEL
			, IMPORT_TYPE__OUTPUT_MODEL
		};

		enum FORMAT_QUALIFIERS
		{
			  FORMAT_QUALIFIERS__STRINGS_ARE_DOUBLEQUOTED = 0X01
			, FORMAT_QUALIFIERS__STRINGS_ARE_SINGLEQUOTED = 0X02
			, FORMAT_QUALIFIERS__COMMA_DELIMITED = 0X04
			, FORMAT_QUALIFIERS__TAB_DELIMITED = 0X08
			, FORMAT_QUALIFIERS__BACKSLASH_ESCAPE_CHAR = 0x10
		};

		typedef std::vector<std::shared_ptr<FieldMapping>> ImportMappings;

		ImportDefinition();
		ImportDefinition(ImportDefinition const & rhs);

		bool IsEmpty();

		ImportMappings mappings;
		boost::filesystem::path input_file;
		bool first_row_is_header_row;
		Schema input_schema;
		Schema output_schema;
		int format_qualifiers;
		IMPORT_TYPE import_type;

};

#define MAX_LINE_SIZE 32768
class Table_basemost;
class Model_basemost;
class Importer
{

	public:

		typedef std::vector<std::shared_ptr<BaseField>> DataFields;
		typedef std::vector<DataFields> DataBlock;
		typedef bool (*TableImportCallbackFn)(Model_basemost * model_, Table_basemost * table_, DataBlock const & table_block, int const number_rows);

		static int const block_size = 1024;

		Importer(ImportDefinition const & import_definition_, Model_basemost * model_, Table_basemost * table_, TableImportCallbackFn table_write_callback_);

		bool DoImport();

	protected:
	
		ImportDefinition import_definition;
		DataBlock input_block;
		DataBlock output_block;
		Model_basemost * model;
		Table_basemost * table;
		TableImportCallbackFn table_write_callback;
		
	protected:
	
		bool ValidateMapping();
		void InitializeFields();
		int ReadBlockFromFile(std::fstream & data_file, char * line, char * parsedline);
		void RetrieveStringField(char const * current_line_ptr, char * parsed_line_ptr, bool & stop);

};

#endif
