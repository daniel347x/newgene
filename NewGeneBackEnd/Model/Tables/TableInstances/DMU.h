#ifndef DMU_H
#define DMU_H

#include "../Table.h"

class Table_DMU_Identifier : public Table<TABLE__DMU_IDENTIFIER>
{

public:

	Table_DMU_Identifier()
		: Table<TABLE__DMU_IDENTIFIER>()
	{

	}

};

class Table_DMU_Instance : public Table<TABLE__DMU_INSTANCE>
{

public:

	Table_DMU_Instance()
		: Table<TABLE__DMU_INSTANCE>()
	{

	}

};

#endif
