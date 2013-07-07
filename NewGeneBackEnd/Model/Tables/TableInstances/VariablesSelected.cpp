#include "VariablesSelected.h"
#include "../../InputModel.h"
#include "../../../sqlite/sqlite-amalgamation-3071700/sqlite3.h"

std::string const Table_VARIABLES_SELECTED::VG_SET_MEMBER_STRING_CODE = "VG_SET_MEMBER_STRING_CODE";
std::string const Table_VARIABLES_SELECTED::VG_CATEGORY_STRING_CODE = "VG_CATEGORY_STRING_CODE";

void Table_VARIABLES_SELECTED::Load(sqlite3 * db, InputModel * input_model_)
{

	if (!input_model_)
	{
		return;
	}

	std::lock_guard<std::recursive_mutex> data_lock(data_mutex);

	identifiers_map.clear();

	sqlite3_stmt * stmt = NULL;
	std::string sql("SELECT * FROM VG_SET_MEMBERS_SELECTED");	
	sqlite3_prepare_v2(db, sql.c_str(), sql.size() + 1, &stmt, NULL);
	if (stmt == NULL)
	{
		return;
	}
	int step_result = 0;
	while ((step_result = sqlite3_step(stmt)) == SQLITE_ROW)
	{
		char const * code_variable = reinterpret_cast<char const *>(sqlite3_column_text(stmt, INDEX__VG_SET_MEMBER_STRING_CODE));
		char const * code_variable_group = reinterpret_cast<char const *>(sqlite3_column_text(stmt, INDEX__VG_CATEGORY_STRING_CODE));
		if (code_variable && strlen(code_variable) && code_variable_group && strlen(code_variable_group))
		{

			// TODO: Could also use fkeys (foreign_keys) here

			// Get the UUID (identifier) of the VG_CATEGORY instance from the input model
			WidgetInstanceIdentifier identifier_parent;
			bool found_parent = input_model_->t_vgp_identifiers.getIdentifierFromStringCode(code_variable_group, identifier_parent);
			if (found_parent && identifier_parent.uuid)
			{
				WidgetInstanceIdentifier identifier;
				bool found = input_model_->t_vgp_setmembers.getIdentifierFromStringCodeAndParentUUID(code_variable, *identifier_parent.uuid, identifier);
				if (found && identifier.uuid)
				{
					identifiers_map[*identifier.uuid].push_back(identifier);
				}
			}
		}
	}
}
