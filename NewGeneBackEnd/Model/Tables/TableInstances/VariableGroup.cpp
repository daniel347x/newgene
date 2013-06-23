#include "VariableGroup.h"
#include "../../../sqlite/sqlite-amalgamation-3071700/sqlite3.h"

void Table_VariableGroupIdentifier::Load(sqlite3 * db)
{
	identifiers.clear();

	sqlite3_stmt * stmt = NULL;
	std::string sql("SELECT * FROM VG_CATEGORY");
	sqlite3_prepare_v2(db, sql.c_str(), sql.size() + 1, &stmt, NULL);
	if (stmt == NULL)
	{
		return;
	}
	int step_result = 0;
	while ((step_result = sqlite3_step(stmt)) == SQLITE_ROW)
	{
		char const * variable_group_code = reinterpret_cast<char const *>(sqlite3_column_text(stmt, 1));
		char const * variable_group_name = reinterpret_cast<char const *>(sqlite3_column_text(stmt, 2));
		if (variable_group_code && strlen(variable_group_code))
		{
			if (variable_group_name && strlen(variable_group_name) > 0)
			{
				std::string variable_group = variable_group_name;
				if (!variable_group.empty())
				{
					identifiers.push_back(variable_group);
				}
			}
		}
	}
}
