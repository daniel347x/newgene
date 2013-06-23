#include "../../../globals.h"
#include "VariableGroup.h"
#include "../../../sqlite/sqlite-amalgamation-3071700/sqlite3.h"

std::string const Table_VG_CATEGORY::VG_CATEGORY_UUID = "VG_CATEGORY_UUID";
std::string const Table_VG_CATEGORY::VG_CATEGORY_STRING_CODE = "VG_CATEGORY_STRING_CODE";
std::string const Table_VG_CATEGORY::VG_CATEGORY_STRING_LONGHAND = "VG_CATEGORY_STRING_LONGHAND";
std::string const Table_VG_CATEGORY::VG_CATEGORY_NOTES1 = "VG_CATEGORY_NOTES1";
std::string const Table_VG_CATEGORY::VG_CATEGORY_NOTES2 = "VG_CATEGORY_NOTES2";
std::string const Table_VG_CATEGORY::VG_CATEGORY_NOTES3 = "VG_CATEGORY_NOTES3";
std::string const Table_VG_CATEGORY::VG_CATEGORY_FK_UOA_CATEGORY_UUID = "VG_CATEGORY_FK_UOA_CATEGORY_UUID";
std::string const Table_VG_CATEGORY::VG_CATEGORY_FLAGS = "VG_CATEGORY_FLAGS";

std::string const Table_VG_SET_MEMBER::VG_SET_MEMBER_UUID = "VG_SET_MEMBER_UUID";
std::string const Table_VG_SET_MEMBER::VG_SET_MEMBER_STRING_CODE = "VG_SET_MEMBER_STRING_CODE";
std::string const Table_VG_SET_MEMBER::VG_SET_MEMBER_STRING_LONGHAND = "VG_SET_MEMBER_STRING_LONGHAND";
std::string const Table_VG_SET_MEMBER::VG_SET_MEMBER_NOTES1 = "VG_SET_MEMBER_NOTES1";
std::string const Table_VG_SET_MEMBER::VG_SET_MEMBER_NOTES2 = "VG_SET_MEMBER_NOTES2";
std::string const Table_VG_SET_MEMBER::VG_SET_MEMBER_NOTES3 = "VG_SET_MEMBER_NOTES3";
std::string const Table_VG_SET_MEMBER::VG_SET_MEMBER_FK_VG_CATEGORY_UUID = "VG_SET_MEMBER_FK_VG_CATEGORY_UUID";
std::string const Table_VG_SET_MEMBER::VG_SET_MEMBER_FLAGS = "VG_SET_MEMBER_FLAGS";

void Table_VG_CATEGORY::Load(sqlite3 * db)
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
	//DataInstanceIdentifiers identifiers;
	while ((step_result = sqlite3_step(stmt)) == SQLITE_ROW)
	{
		char const * variable_group_code = reinterpret_cast<char const *>(sqlite3_column_text(stmt, INDEX__VG_CATEGORY_STRING_CODE));
		char const * variable_group_name = reinterpret_cast<char const *>(sqlite3_column_text(stmt, INDEX__VG_CATEGORY_STRING_LONGHAND));
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
