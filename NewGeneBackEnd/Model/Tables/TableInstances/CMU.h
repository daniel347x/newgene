#ifndef CMU_H
#define CMU_H

#include "../Table.h"

class Table_CMU_Identifier : public Table<TABLE__CMU_IDENTIFIER, TABLE_INSTANCE_IDENTIFIER_CONTAINER_TYPE__VECTOR>
{

	public:

		Table_CMU_Identifier()
			: Table<TABLE__CMU_IDENTIFIER, TABLE_INSTANCE_IDENTIFIER_CONTAINER_TYPE__VECTOR>()
		{

		}

};

class Table_CMU_Instance : public Table<TABLE__CMU_INSTANCE, TABLE_INSTANCE_IDENTIFIER_CONTAINER_TYPE__VECTOR>
{

public:

	Table_CMU_Instance()
		: Table<TABLE__CMU_INSTANCE, TABLE_INSTANCE_IDENTIFIER_CONTAINER_TYPE__VECTOR>()
	{

	}

};

#endif
