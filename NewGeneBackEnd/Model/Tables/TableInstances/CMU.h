#ifndef CMU_H
#define CMU_H

#include "../Table.h"

class Table_CMU_Identifier : public Table<TABLE__CMU_IDENTIFIER>
{

	public:

		Table_CMU_Identifier()
			: Table<TABLE__CMU_IDENTIFIER>()
		{

		}

};

class Table_CMU_Instance : public Table<TABLE__CMU_INSTANCE>
{

public:

	Table_CMU_Instance()
		: Table<TABLE__CMU_INSTANCE>()
	{

	}

};

#endif
