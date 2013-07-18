#ifndef TABLE_MANAGER_H
#define TABLE_MANAGER_H

#include "Table.h"
#include "TableInstances/DMU.h"
#include "TableInstances/CMU.h"
#include "TableInstances/UOA.h"
#include "TableInstances/VariableGroup.h"
#include "TableInstances/VariablesSelected.h"
#include "TableInstances/KAdCount.h"
#include <vector>

class TableManager
{

public:

	TableManager()
	{

	}

	void LoadTable(TABLE_TYPES table_type);

	// Todo: fill in directed acyclic graph table dependencies

};

#endif
