#ifndef TABLE_MANAGER_H
#define TABLE_MANAGER_H

#include "Import/Import.h"
#include "Table.h"
#include "TableInstances/DMU.h"
#include "TableInstances/CMU.h"
#include "TableInstances/UOA.h"
#include "TableInstances/VariableGroup.h"
#include "TableInstances/VariablesSelected.h"
#include "TableInstances/GeneralOptions.h"
#include "TableInstances/KAdCount.h"
#include "TableInstances/TimeRange.h"
#include "TableInstances/VariableGroupData.h"
#include "TableInstances/Limit_DMUs.h"
#include <vector>

class TableManager
{

public:

	TableManager()
	{

	}

	bool TableExists(sqlite3 * db, std::string table_name);
	std::string EscapeDatabaseStringField(std::string const field);

	static TableManager tableManager;

};

TableManager & tableManager();

#endif
