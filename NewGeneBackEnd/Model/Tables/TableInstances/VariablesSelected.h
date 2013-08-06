#ifndef VARIABLESSELECTED_H
#define VARIABLESSELECTED_H

#include "../../../globals.h"
#include "../Table.h"

class Table_VARIABLES_SELECTED : public Table<TABLE__VG_SET_MEMBER_SELECTED, TABLE_INSTANCE_IDENTIFIER_CONTAINER_TYPE__MAP>
{

public:

	static std::string const VG_SET_MEMBER_STRING_CODE;
	static std::string const VG_CATEGORY_STRING_CODE;

	// Vector of: pair<Variable Group identifier, Selected Variables in the group>
	typedef std::vector<std::pair<WidgetInstanceIdentifier, WidgetInstanceIdentifiers>> VariableGroup_To_VariableSelections_Vector;

	// Variable Group identifier => Selected Variables in the group
	typedef std::map<WidgetInstanceIdentifier, WidgetInstanceIdentifiers> VariableGroup_To_VariableSelections_Map;

	// UOA identifier => Variable Groups with the same UOA (and their selected variables)
	typedef std::map<WidgetInstanceIdentifier, VariableGroup_To_VariableSelections_Map> UOA_To_Variables_Map;

	enum COLUMN_INDEX
	{
		  INDEX__VG_SET_MEMBER_STRING_CODE = 0
		, INDEX__VG_CATEGORY_STRING_CODE
	};

public:

	Table_VARIABLES_SELECTED()
		: Table<TABLE__VG_SET_MEMBER_SELECTED, TABLE_INSTANCE_IDENTIFIER_CONTAINER_TYPE__MAP>(Table_basemost::TABLE_MODEL_TYPE__OUTPUT_MODEL)
	{

	}

	void Load(sqlite3 * db, OutputModel * output_model_, InputModel * input_model_);
	bool Update(sqlite3 * db, OutputModel & output_model_, InputModel & input_model_, DataChangeMessage & change_message);

	UOA_To_Variables_Map GetSelectedVariablesByUOA(sqlite3 * db, OutputModel * output_model_, InputModel * input_model_);


private:

	void Add(sqlite3 * db, std::string const & vg_set_member_code, std::string const & vg_category_code);
	void Remove(sqlite3 * db, std::string const & vg_set_member_code, std::string const & vg_category_code);

};

#endif
