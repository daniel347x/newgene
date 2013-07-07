#ifndef VARIABLESSELECTED_H
#define VARIABLESSELECTED_H

#include "../../../globals.h"
#include "../Table.h"

class Table_VARIABLES_SELECTED : public Table<TABLE__VG_SET_MEMBER_SELECTED>
{

public:

	static std::string const VG_SET_MEMBER_STRING_CODE;
	static std::string const VG_CATEGORY_STRING_CODE;

	enum COLUMN_INDEX
	{
		  INDEX__VG_SET_MEMBER_STRING_CODE = 0
		, INDEX__VG_CATEGORY_STRING_CODE
	};

public:

	Table_VARIABLES_SELECTED()
		: Table<TABLE__VG_SET_MEMBER_SELECTED>()
	{

	}

	void Load(sqlite3 * db, InputModel * input_model_);

	WidgetInstanceIdentifiers getIdentifiers(UUID const & uuid)
	{
		std::lock_guard<std::recursive_mutex> data_lock(data_mutex);
		return identifiers_map[uuid];
	}

protected:

	std::map<UUID, WidgetInstanceIdentifiers> identifiers_map;

};

#endif
