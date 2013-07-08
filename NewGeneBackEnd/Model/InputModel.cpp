#include "InputModel.h"
#include "../sqlite/sqlite-amalgamation-3071700/sqlite3.h"

void InputModel::LoadTables()
{

	Model::LoadTables();

	if (db != nullptr)
	{
		t_dmu_category.Load(db, this);
		t_dmu_setmembers.Load(db, this);
		t_uoa_category.Load(db, this);
		t_uoa_setmemberlookup.Load(db, this);
		t_vgp_identifiers.Load(db, this);
		t_vgp_setmembers.Load(db, this);
	}

}
