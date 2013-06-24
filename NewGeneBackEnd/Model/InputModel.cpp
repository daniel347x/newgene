#include "InputModel.h"
#include "../sqlite/sqlite-amalgamation-3071700/sqlite3.h"

void InputModel::LoadTables()
{

	Model::LoadTables();

	if (db != nullptr)
	{
		t_vgp_identifiers.Load(db);
		t_vgp_setmembers.Load(db);
	}

}
