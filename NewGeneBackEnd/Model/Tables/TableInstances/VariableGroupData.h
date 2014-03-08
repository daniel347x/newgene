#ifndef VARIABLEGROUPDATA_H
#define VARIABLEGROUPDATA_H

#include "../../../globals.h"
#include "../Table.h"

class Table_VariableGroupData : public Table<TABLE__VG_INPUT_DATA, TABLE_INSTANCE_IDENTIFIER_CONTAINER_TYPE__NONE>
{

public:

	Table_VariableGroupData(std::string const & vg_category_string_code_)
		: Table<TABLE__VG_INPUT_DATA, TABLE_INSTANCE_IDENTIFIER_CONTAINER_TYPE__NONE>(Table_basemost::TABLE_MODEL_TYPE__INPUT_MODEL)
		, vg_category_string_code(vg_category_string_code_)
	{
		table_name = Table_VariableGroupData::TableNameFromVGCode(vg_category_string_code);
	}

	bool BuildImportDefinition(sqlite3 * db, InputModel * input_model_, WidgetInstanceIdentifier const & vg, std::vector<std::string> const & timeRangeCols, std::vector<std::pair<WidgetInstanceIdentifier, std::string>> const & dmusAndCols, boost::filesystem::path const & filepathname, TIME_GRANULARITY const & the_time_granularity, ImportDefinition & definition, std::string & errorMsg);
	bool ImportStart(sqlite3 * db, WidgetInstanceIdentifier const & identifier, ImportDefinition const & import_definition, OutputModel * output_model_, InputModel * input_model_);
	bool ImportEnd(sqlite3 * db, WidgetInstanceIdentifier const & identifier, ImportDefinition const & import_definition, OutputModel * output_model_, InputModel * input_model_);

	bool DeleteDataTable(sqlite3 * db, InputModel * input_model_);
	bool DeleteDmuMemberRows(sqlite3 * db, InputModel * input_model_, WidgetInstanceIdentifier const & dmu_member, std::string const & column_name);

	static std::string TableNameFromVGCode(std::string variable_group_code);
	static std::string ViewNameFromCount(int const view_number);
	static std::string ViewNameFromCountTemp(int const view_number, int const multiplicity_number);
	static std::string ViewNameFromCountTempTimeRanged(int const view_number, int const multiplicity_number);
	static std::string JoinViewNameFromCount(int const join_number);
	static std::string JoinViewNameWithTimeRangesFromCount(int const join_number);

	static Table_VariableGroupData * GetInstanceTableFromTableName(sqlite3 * db, InputModel * input_model_, std::string const & table_name);

	std::string vg_category_string_code;

};

class Table_VariableGroupMetadata_PrimaryKeys : public Table<TABLE__VG_INPUT_DATA_METADATA_PRIMARY_KEYS, TABLE_INSTANCE_IDENTIFIER_CONTAINER_TYPE__MAP>
{

public:

	static std::string const VG_DATA_TABLE_NAME;
	static std::string const VG_DATA_TABLE_PRIMARY_KEY_COLUMN_NAME;
	static std::string const VG_DATA_TABLE_FK_DMU_CATEGORY_CODE;
	static std::string const VG_DATA_TABLE_PRIMARY_KEY_SEQUENCE_NUMBER;
	static std::string const VG_DATA_TABLE_PRIMARY_KEY_FIELD_TYPE;

	enum COLUMN_INDEX
	{
		  INDEX__VG_DATA_TABLE_NAME = 0
		, INDEX__VG_DATA_TABLE_PRIMARY_KEY_COLUMN_NAME
		, INDEX__VG_DATA_TABLE_FK_DMU_CATEGORY_CODE
		, INDEX__VG_DATA_TABLE_PRIMARY_KEY_SEQUENCE_NUMBER
		, INDEX__VG_DATA_TABLE_PRIMARY_KEY_FIELD_TYPE
	};

public:

	Table_VariableGroupMetadata_PrimaryKeys()
		: Table<TABLE__VG_INPUT_DATA_METADATA_PRIMARY_KEYS, TABLE_INSTANCE_IDENTIFIER_CONTAINER_TYPE__MAP>(Table_basemost::TABLE_MODEL_TYPE__INPUT_MODEL)
	{

	}

	void Load(sqlite3 * db, InputModel * input_model_);

	bool AddDataTable(sqlite3 * db, InputModel * input_model_, WidgetInstanceIdentifier const & vg, std::vector<std::tuple<WidgetInstanceIdentifier, std::string, FIELD_TYPE>> const & primary_keys_info, std::string errorMsg);
	bool DeleteDataTable(sqlite3 * db, InputModel * input_model_, std::string const & table_name);

	// The first element of the pair is the WidgetInstanceIdentifier corresponding to the DMU category associated with the primary key column
	std::vector<std::pair<WidgetInstanceIdentifier, std::vector<std::string>>> GetColumnNamesCorrespondingToPrimaryKeys(sqlite3 * db, InputModel * input_model_, std::string const & table_name);

};


class Table_VariableGroupMetadata_DateTimeColumns : public Table<TABLE__VG_INPUT_DATA_METADATA_DATETIME_COLUMNS, TABLE_INSTANCE_IDENTIFIER_CONTAINER_TYPE__MAP>
{

public:

	static std::string const VG_DATA_TABLE_NAME;
	static std::string const VG_DATA_TABLE_DATETIME_START_COLUMN_NAME;
	static std::string const VG_DATA_TABLE_DATETIME_END_COLUMN_NAME;
	static std::string const VG_DATA_FK_VG_CATEGORY_UUID;

	enum DATETIME_INDEX
	{
		  INDEX__VG_DATA_TABLE_NAME = 0
		, INDEX__VG_DATA_TABLE_DATETIME_START_COLUMN_NAME
		, INDEX__VG_DATA_TABLE_DATETIME_END_COLUMN_NAME
		, INDEX__VG_DATA_TABLE_FK_VG_UUID
	};

public:

	Table_VariableGroupMetadata_DateTimeColumns()
		: Table<TABLE__VG_INPUT_DATA_METADATA_DATETIME_COLUMNS, TABLE_INSTANCE_IDENTIFIER_CONTAINER_TYPE__MAP>(Table_basemost::TABLE_MODEL_TYPE__INPUT_MODEL)
	{

	}

	void Load(sqlite3 * db, InputModel * input_model_);

	bool AddDataTable(sqlite3 * db, InputModel * input_model_, WidgetInstanceIdentifier const & vg, std::string errorMsg);
	bool DeleteDataTable(sqlite3 * db, InputModel * input_model_, std::string const & table_name);

	static std::string DefaultDatetimeStartColumnName;
	static std::string DefaultDatetimeEndColumnName;

};

#endif
