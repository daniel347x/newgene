#ifndef CMU_H
#define CMU_H

#include "../Table.h"

class Table_CMU_Identifier : public Table<TABLE_TYPE_CMU_IDENTIFIER>
{

	public:

		Table_CMU_Identifier()
			: Table<TABLE_TYPE_CMU_IDENTIFIER>()
		{

		}

};

class Table_CMU_Instance : public Table<TABLE_TYPE_CMU_INSTANCE>
{

public:

	Table_CMU_Instance()
		: Table<TABLE_TYPE_CMU_INSTANCE>()
	{

	}

};

#endif
