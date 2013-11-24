#include "TableManager.h"

TableManager TableManager::tableManager;
	
bool TableManager::TableExists(sqlite3 * db, std::string table_name)
{

	if (db)
	{
		std::string sql_exists;
		sql_exists += "SELECT name FROM sqlite_master WHERE type='table' AND name='";
		sql_exists += table_name;
		sql_exists += "';";

		sqlite3_stmt * stmt = NULL;
		sqlite3_prepare_v2(db, sql_exists.c_str(), static_cast<int>(sql_exists.size()) + 1, &stmt, NULL);
		if (stmt == NULL)
		{
			return false;
		}
		int step_result = 0;
		while ((step_result = sqlite3_step(stmt)) == SQLITE_ROW)
		{
			if (stmt)
			{
				sqlite3_finalize(stmt);
				stmt = nullptr;
			}
			return true;
		}
		if (stmt)
		{
			sqlite3_finalize(stmt);
			stmt = nullptr;
		}
	}

	return false;

}

std::string TableManager::EscapeDatabaseStringField(std::string const field)
{
	size_t the_length = field.size();
	if (the_length == 0)
	{
		return field;
	}
	char * escaped_string = new char[the_length * 2 + 1];
	escaped_string[0] = '\0';
	size_t current_input_index = 0;
	size_t current_output_index = 0;
	char const * const input = field.c_str();
	while (input[current_input_index] != '\0')
	{
		if (input[current_input_index] == '\'')
		{
			escaped_string = strcat(escaped_string, "\\'");
			++current_output_index;
			++current_output_index;
		}
		else
		{
			escaped_string[current_output_index] = input[current_input_index];
			++current_output_index;
		}
		++current_input_index;
	}
	std::string return_string(escaped_string);
	delete [] escaped_string;
	escaped_string = nullptr;

	return return_string;
}

TableManager & tableManager()
{
	return TableManager::tableManager;
}
