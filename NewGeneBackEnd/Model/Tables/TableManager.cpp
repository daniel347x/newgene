#include "TableManager.h"

TableManager TableManager::tableManager;
	
bool TableManager::TableExists(sqlite3 * db, std::string table_name)
{

	if (db)
	{
		std::string sql_exists("SELECT ");
		sql_exists += table_name;
		sql_exists += " FROM sqlite_master WHERE type='table' AND name='table_name';";

		sqlite3_stmt * stmt = NULL;
		sqlite3_prepare_v2(db, sql_exists.c_str(), sql_exists.size() + 1, &stmt, NULL);
		if (stmt == NULL)
		{
			return false;
		}
		int step_result = 0;
		while ((step_result = sqlite3_step(stmt)) == SQLITE_ROW)
		{
			return true;
		}
	}

	return false;

}
