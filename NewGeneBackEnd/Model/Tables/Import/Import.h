#ifndef IMPORT_H
#define IMPORT_H

#ifndef Q_MOC_RUN
#	include <boost/filesystem.hpp>
#	include <boost/date_time/local_time/local_time.hpp>
#	include <boost/date_time/gregorian/gregorian.hpp>
#endif
#include "../Fields.h"
#include "../Schema.h"
#include <vector>
#include <memory>
#include "../../../Utilities/WidgetIdentifier.h"
#include "../../../Messager/Messager.h"
#include "../../../sqlite/sqlite-amalgamation-3071700/sqlite3.h"

#include <atomic>
#include <mutex>

class NameOrIndex
{

	public:

		enum NAME_OR_INDEX
		{
			  NAME
			, INDEX
		};

		NameOrIndex()
			: name_or_index(NAME)
			, index(0)
		{

		}

		NameOrIndex(NameOrIndex const & rhs)
			: name_or_index(rhs.name_or_index)
			, index(rhs.index)
			, name(rhs.name)
		{

		}

		NameOrIndex(NAME_OR_INDEX const name_or_index_, int const index_)
			: name_or_index(name_or_index_)
			, index(index_)
		{
		}

		NameOrIndex(NAME_OR_INDEX const name_or_index_, std::string const name_)
			: name_or_index(name_or_index_)
			, index(-1)
            , name(name_)
        {
		}

		NAME_OR_INDEX name_or_index;
		int index;
		std::string name;

};

typedef std::pair<NameOrIndex, FIELD_TYPE> FieldTypeEntry;
typedef std::vector<FieldTypeEntry> FieldTypeEntries;
class BaseField;
typedef std::vector<std::shared_ptr<BaseField>> DataFields;
typedef std::vector<DataFields> DataBlock;
std::shared_ptr<BaseField> RetrieveDataField(FieldTypeEntry const & field_type_entry, DataFields const & data_fields);

class FieldMapping
{

	public:

		enum FIELD_MAPPING_TYPE
		{
			  FIELD_MAPPING_TYPE__UNKNOWN
			, FIELD_MAPPING_TYPE__ROW
			, FIELD_MAPPING_TYPE__ONE_TO_ONE
			, FIELD_MAPPING_TYPE__TIME_RANGE
			, FIELD_MAPPING_TYPE__HARD_CODED
		};

		FieldMapping()
			: field_mapping_type(FIELD_MAPPING_TYPE__UNKNOWN)
		{

		}

		FieldMapping(FIELD_MAPPING_TYPE const field_mapping_type_)
			: field_mapping_type(field_mapping_type_)
		{

		}

		FieldMapping(FIELD_MAPPING_TYPE const field_mapping_type_, FieldTypeEntry const & input_field_entry, FieldTypeEntry const & output_table_entry)
			: field_mapping_type(field_mapping_type_)
		{
			input_file_fields.push_back(input_field_entry);
			output_table_fields.push_back(output_table_entry);
		}

		FieldMapping(FieldTypeEntry const & field_entry, bool field_is_input)
			: field_mapping_type(FIELD_MAPPING_TYPE__UNKNOWN)
		{
			if (field_is_input)
			{
				input_file_fields.push_back(field_entry);
			}
			else
			{
				output_table_fields.push_back(field_entry);
			}
		}

		FieldMapping(FIELD_MAPPING_TYPE const field_mapping_type_, FieldTypeEntry const & field_entry, bool field_is_input)
			: field_mapping_type(field_mapping_type_)
		{
			if (field_is_input)
			{
				input_file_fields.push_back(field_entry);
			}
			else
			{
				output_table_fields.push_back(field_entry);
			}
		}

		FieldMapping(FieldTypeEntry const & input_field_entry, FieldTypeEntry const & output_table_entry)
			: field_mapping_type(FIELD_MAPPING_TYPE__UNKNOWN)
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
			: FieldMapping(FIELD_MAPPING_TYPE__ROW)
		{

		}

		RowFieldMapping(FIELD_MAPPING_TYPE const field_mapping_type_)
			: FieldMapping(field_mapping_type_)
		{

		}

		RowFieldMapping(FIELD_MAPPING_TYPE const field_mapping_type_, FieldTypeEntry const & input_field_entry, FieldTypeEntry const & output_table_entry)
			: FieldMapping(field_mapping_type_, input_field_entry, output_table_entry)
		{
		}

		RowFieldMapping(FieldTypeEntry const & input_field_entry, FieldTypeEntry const & output_table_entry)
			: FieldMapping(FIELD_MAPPING_TYPE__ROW, input_field_entry, output_table_entry)
		{
		}

		RowFieldMapping(FIELD_MAPPING_TYPE const field_mapping_type_, FieldTypeEntry const & field_entry, bool field_is_input)
			: FieldMapping(field_mapping_type_, field_entry, field_is_input)
		{
		}

		RowFieldMapping(RowFieldMapping const & rhs)
			: FieldMapping(rhs)
		{

		}

};

class OneToOneFieldMapping : public RowFieldMapping
{

	public:

		OneToOneFieldMapping()
			: RowFieldMapping(FIELD_MAPPING_TYPE__ONE_TO_ONE)
		{

		}

		OneToOneFieldMapping(FIELD_MAPPING_TYPE const field_mapping_type_)
			: RowFieldMapping(field_mapping_type_)
		{

		}

		OneToOneFieldMapping(FIELD_MAPPING_TYPE const field_mapping_type_, FieldTypeEntry const & input_field_entry, FieldTypeEntry const & output_table_entry)
			: RowFieldMapping(field_mapping_type_, input_field_entry, output_table_entry)
		{
		}

		OneToOneFieldMapping(FieldTypeEntry const & input_field_entry, FieldTypeEntry const & output_table_entry)
			: RowFieldMapping(FIELD_MAPPING_TYPE__ONE_TO_ONE, input_field_entry, output_table_entry)
		{
		}

		OneToOneFieldMapping(OneToOneFieldMapping const & rhs)
			: RowFieldMapping(rhs)
		{

		}

		bool Validate()
		{
			return true;
		}

};

class HardCodedFieldMapping : public RowFieldMapping
{

public:

	HardCodedFieldMapping()
		: RowFieldMapping(FIELD_MAPPING_TYPE__HARD_CODED)
	{
	}

	HardCodedFieldMapping(std::shared_ptr<BaseField> data_)
		: RowFieldMapping(FIELD_MAPPING_TYPE__HARD_CODED)
		, data(data_)
	{
	}

	HardCodedFieldMapping(FIELD_MAPPING_TYPE const field_mapping_type_, std::shared_ptr<BaseField> data_)
		: RowFieldMapping(field_mapping_type_)
		, data(data_)
	{
	}

	HardCodedFieldMapping(std::shared_ptr<BaseField> input_data_, FieldTypeEntry const & output_table_entry)
		: RowFieldMapping(FIELD_MAPPING_TYPE__HARD_CODED, output_table_entry, false)
		, data(input_data_)
	{
	}

	HardCodedFieldMapping(HardCodedFieldMapping const & rhs)
		: RowFieldMapping(rhs)
		, data(rhs.data)
	{
	}

	bool Validate()
	{
		return true;
	}

	std::shared_ptr<BaseField> data;

};

class TimeRangeFieldMapping : public RowFieldMapping
{

	public:

		// caches for time ranges
		static std::map<std::tuple<int, int, int, int, int, int>, std::pair<std::int64_t, std::int64_t>> ymd_ymd_int_mappings;
		static std::map<std::tuple<int, int, int>, std::pair<std::int64_t, std::int64_t>>                ymd_int_mappings;
		static std::map<std::tuple<int, int, int, int>, std::pair<std::int64_t, std::int64_t>>           ym_ym_int_mappings;
		static std::map<std::pair<int, int>, std::pair<std::int64_t, std::int64_t>>                      ym_int_mappings;
		static std::map<std::pair<int, int>, std::pair<std::int64_t, std::int64_t>>                      y_y_int_mappings;
		static std::map<int, std::pair<std::int64_t, std::int64_t>>                                      y_int_mappings;
		static std::map<std::pair<std::string, std::string>, std::pair<std::int64_t, std::int64_t>>      ymd_ymd_string_mappings;
		static std::map<std::string, std::pair<std::int64_t, std::int64_t>>                              ymd_string_mappings;
		static std::map<std::pair<std::string, std::string>, std::pair<std::int64_t, std::int64_t>>      ym_ym_string_mappings;
		static std::map<std::string, std::pair<std::int64_t, std::int64_t>>                              ym_string_mappings;
		static std::map<std::pair<std::string, std::string>, std::pair<std::int64_t, std::int64_t>>      y_y_string_mappings;
		static std::map<std::string, std::pair<std::int64_t, std::int64_t>>                              y_string_mappings;

	public:

		enum TIME_RANGE_FIELD_MAPPING_TYPE
		{

			  TIME_RANGE_FIELD_MAPPING_TYPE__START = 0


			  
			// *************************************************************************************** //  
			// Year mappings
			// *************************************************************************************** //  

			// Integer field
			// that is used for both the starting YEAR and ending YEAR
			// (with the ending year being set to the
			// zeroeth second of the year AFTER the specified ending year)
			, TIME_RANGE_FIELD_MAPPING_TYPE__INTS__YEAR__START_YEAR_ONLY

			// Two integer fields are provided,
			// one used for the starting YEAR and the other used for the ending YEAR
			// (with the ending year being set to the zeroeth
			// second of the year AFTER the specified ending year)
			, TIME_RANGE_FIELD_MAPPING_TYPE__INTS__YEAR__FROM__START_YEAR__TO__END_YEAR

			// A single string field is provided, containing a single date string,
			// that is used for both the starting YEAR and ending YEAR
			// (rounded to year, with the ending year being set to the
			// zeroeth second of the year AFTER the specified ending year)
			, TIME_RANGE_FIELD_MAPPING_TYPE__STRINGS__YEAR__START_YEAR_ONLY

			// Two string fields are provided, each containing a date string,
			// one used for the starting YEAR and the other used for the ending YEAR
			// (both rounded to year, with the ending year being set to the zeroeth
			// second of the year AFTER the specified ending year)
			, TIME_RANGE_FIELD_MAPPING_TYPE__STRINGS__YEAR__FROM__START_YEAR__TO__END_YEAR







			// *************************************************************************************** //  
			// Month mappings
			// *************************************************************************************** //  

			, TIME_RANGE_FIELD_MAPPING_TYPE__INTS__MONTH__START_MONTH_ONLY
			, TIME_RANGE_FIELD_MAPPING_TYPE__INTS__MONTH__FROM__START_MONTH__TO__END_MONTH
			, TIME_RANGE_FIELD_MAPPING_TYPE__STRINGS__MONTH__START_MONTH_ONLY
			, TIME_RANGE_FIELD_MAPPING_TYPE__STRINGS__MONTH__FROM__START_MONTH__TO__END_MONTH





			// *************************************************************************************** //  
			// Day mappings
			// *************************************************************************************** //  

			, TIME_RANGE_FIELD_MAPPING_TYPE__INTS__DAY__START_DAY_ONLY
			, TIME_RANGE_FIELD_MAPPING_TYPE__INTS__DAY__FROM__START_DAY__TO__END_DAY
			, TIME_RANGE_FIELD_MAPPING_TYPE__STRINGS__DAY__START_DAY_ONLY
			, TIME_RANGE_FIELD_MAPPING_TYPE__STRINGS__DAY__FROM__START_DAY__TO__END_DAY










			// *************************************************************************************** //  
			// Unused mappings
			// *************************************************************************************** //  

			// Special-case: Not used for NewGene application,
			// but only for internal special-coded loading of Maoz, etc. data
			// ... and not even used for that any more
			, TIME_RANGE_FIELD_MAPPING_TYPE__YEAR__RANGE__FROM__YR_MNTH_DAY

			// Special-case: Not used for NewGene application,
			// but only for internal special-coded loading of Maoz, etc. data
			// ... and not even used for that any more
			, TIME_RANGE_FIELD_MAPPING_TYPE__STRING_RANGE // accepts only one format



		};

		TimeRangeFieldMapping(TIME_RANGE_FIELD_MAPPING_TYPE const time_range_type_, FIELD_MAPPING_TYPE const field_mapping_type_)
			: RowFieldMapping(field_mapping_type_)
			, time_range_type(time_range_type_)
		{

		}

		TimeRangeFieldMapping(TIME_RANGE_FIELD_MAPPING_TYPE const time_range_type_, FIELD_MAPPING_TYPE const field_mapping_type_, FieldTypeEntry const & input_field_entry, FieldTypeEntry const & output_table_entry)
			: RowFieldMapping(field_mapping_type_, input_field_entry, output_table_entry)
			, time_range_type(time_range_type_)
		{
		}

		TimeRangeFieldMapping(TIME_RANGE_FIELD_MAPPING_TYPE const time_range_type_, FieldTypeEntry const & input_field_entry, FieldTypeEntry const & output_table_entry)
			: RowFieldMapping(FIELD_MAPPING_TYPE__TIME_RANGE, input_field_entry, output_table_entry)
			, time_range_type(time_range_type_)
		{
		}

		TimeRangeFieldMapping(TimeRangeFieldMapping const & rhs)
			: RowFieldMapping(rhs)
			, time_range_type(rhs.time_range_type)
		{

		}

		TimeRangeFieldMapping(TIME_RANGE_FIELD_MAPPING_TYPE const time_range_type_)
			: RowFieldMapping(FIELD_MAPPING_TYPE__TIME_RANGE)
			, time_range_type(time_range_type_)
		{

		}
		
		bool Validate()
		{
			return true;
		}

		void PerformMapping(DataFields const & input_data_fields, DataFields const & output_data_fields);

		static void ConvertStringToDate(int & year, int & month, int & day, std::string const & the_string);
		static int ConvertStringToDateFancy(boost::posix_time::ptime & the_time, std::string const & the_string, int const index_to_use);

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
			  FORMAT_QUALIFIERS__STRINGS_ARE_DOUBLEQUOTED                        = 0X01
			, FORMAT_QUALIFIERS__STRINGS_ARE_SINGLEQUOTED                        = 0X02
			, FORMAT_QUALIFIERS__STRINGS_ARE_EITHER_DOUBLEQUOTED_OR_SINGLEQUOTED = 0X04
			, FORMAT_QUALIFIERS__COMMA_DELIMITED                                 = 0X08
			, FORMAT_QUALIFIERS__TAB_DELIMITED                                   = 0X10
			, FORMAT_QUALIFIERS__BACKSLASH_ESCAPE_CHAR                           = 0x20
		};

		typedef std::vector<std::shared_ptr<FieldMapping>> ImportMappings;

		ImportDefinition();
		ImportDefinition(ImportDefinition const & rhs);
		virtual ~ImportDefinition();

		bool IsEmpty();

		ImportMappings mappings;
		boost::filesystem::path input_file;
		bool first_row_is_header_row;
		bool second_row_is_column_description_row;
		bool third_row_is_data_type_row;
		Schema input_schema;
		Schema output_schema;
		int format_qualifiers; // must be int in order to OR them together
		IMPORT_TYPE import_type;
		std::vector<std::tuple<WidgetInstanceIdentifier, std::string, FIELD_TYPE>> primary_keys_info;

		mutable sqlite3_stmt * stmt_update;
		mutable sqlite3_stmt * stmt_insert;

};

#define MAX_LINE_SIZE 32768
class Table_basemost;
class Model_basemost;
class Importer
{

	public:

		enum Mode
		{
			  INSERT_IN_BULK = 0
			, INSERT_OR_UPDATE
		};

	public:

		enum WHICH_IMPORT
		{
			  IMPORT_DMU_SET_MEMBER = 0
			, IMPORT_VG_INSTANCE_DATA
		};

	public:

		typedef bool(*TableImportCallbackFn)(Importer * importer, Model_basemost * model_, ImportDefinition & import_definition, Table_basemost * table_, DataBlock const & table_block, int const number_rows, long & linenum, long & badwritelines, long & goodwritelines, long & goodupdatelines, std::vector<std::string> & errors);

		static int const block_size_sqlite_limit = 500; // Maximum number of rows supported for block insert by SQLite
		static int const block_size_no_sqlite_limit = 1000; // It doesn't really help to increase this much
		int block_size;

		Importer(ImportDefinition const & import_definition_, Model_basemost * model_, Table_basemost * table_, Mode const mode_, WidgetInstanceIdentifier const & identifier_, TableImportCallbackFn table_write_callback_, WHICH_IMPORT const & which_import_, std::string & errorMsg);
		virtual ~Importer();

		bool DoImport(std::string & errorMsg, Messager & messager);

	protected:
	
		ImportDefinition import_definition;
		DataBlock input_block;
		DataBlock output_block;
		Model_basemost * model;
		Table_basemost * table;
		WidgetInstanceIdentifier identifier;
		TableImportCallbackFn table_write_callback;

	protected:
	
		bool ValidateMapping();
		void InitializeFields();
		int ReadBlockFromFile(std::fstream & data_file, char * line, char * parsedline, long & linenum, std::string & errorMsg, Messager & messager);
		void ReadFieldFromFile(char * & current_line_ptr, int & current_lines_read, int const & current_column_index, char * & parsed_line_ptr, bool & stop, SchemaEntry const & column, long const line, int const col, bool const is_final_col, std::string & errorMsg);

	public:

		static void InstantiateDataFieldInstance(FIELD_TYPE field_type, std::string field_name, DataFields &fields);
		static void ReadFieldFromFileStatic(char *& current_line_ptr, char *& parsed_line_ptr, bool & stop, SchemaEntry const & column, BaseField & theField, ImportDefinition const & import_definition, long const line, int const col, bool const is_final_col, std::string & errorMsg);
		static void SkipFieldInFile(char *& current_line_ptr, char *& parsed_line_ptr, bool & stop, ImportDefinition const & import_definition, long line, int col, bool const is_final_col, std::string & errorMsg);

	protected:

		static void ReadOneDataField(SchemaEntry const &column, BaseField & theField, char * & current_line_ptr, char * & parsed_line_ptr, bool & stop, ImportDefinition const & import_definition, long const line, int const col, std::string & errorMsg);
		static void RetrieveStringField(char * & current_line_ptr, char * & parsed_line_ptr, bool & stop, ImportDefinition const & import_definition, std::string & errorMsg);
		static void EatWhitespace(char * & current_line_ptr, ImportDefinition const & import_definition);
		static void EatSeparator(char * & current_line_ptr, ImportDefinition const & import_definition);

	public:

		Mode mode;
		WHICH_IMPORT which_import;
		std::vector<std::string> errors;
		long badreadlines;
		long badwritelines;
		long goodreadlines;
		long goodwritelines;
		long goodupdatelines;

	public:

		static std::recursive_mutex is_performing_import_mutex;
		static std::atomic<bool> is_performing_import;
		static bool cancelled;
		inline static bool CheckCancelled()
		{
			// No lock - not necessary for a boolean whose getting/setting is sequenced properly
			return cancelled; // opportunity for further checking here
		}

};

//static size_t GetStackUsage()
//{
//	MEMORY_BASIC_INFORMATION mbi;
//	VirtualQuery(&mbi, &mbi, sizeof(mbi));
//	// now mbi.AllocationBase = reserved stack memory base address
//
//	VirtualQuery(mbi.AllocationBase, &mbi, sizeof(mbi));
//	// now (mbi.BaseAddress, mbi.RegionSize) describe reserved (uncommitted) portion of the stack
//	// skip it
//
//	VirtualQuery((char*)mbi.BaseAddress + mbi.RegionSize, &mbi, sizeof(mbi));
//	// now (mbi.BaseAddress, mbi.RegionSize) describe the guard page
//	// skip it
//
//	VirtualQuery((char*)mbi.BaseAddress + mbi.RegionSize, &mbi, sizeof(mbi));
//	// now (mbi.BaseAddress, mbi.RegionSize) describe the committed (i.e. accessed) portion of the stack
//
//	return mbi.RegionSize;
//}

#endif
