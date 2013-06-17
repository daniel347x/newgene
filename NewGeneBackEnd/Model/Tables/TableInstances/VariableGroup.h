#ifndef VARIABLEGROUP_H
#define VARIABLEGROUP_H

#include "../Table.h"

class Table_VariableGroupIdentifier : public Table<TABLE_TYPE_VARIABLE_GROUP_IDENTIFIER>
{

	public:

		Table_VariableGroupIdentifier()
			: Table<TABLE_TYPE_VARIABLE_GROUP_IDENTIFIER>()
		{

		}

};

class Table_VariableIdentifier : public Table<TABLE_TYPE_VARIABLE_IDENTIFIER>
{

public:

	Table_VariableIdentifier()
		: Table<TABLE_TYPE_VARIABLE_IDENTIFIER>()
	{

	}

};

#endif
