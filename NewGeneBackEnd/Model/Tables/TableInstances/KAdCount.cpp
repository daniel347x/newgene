#include "KAdCount.h"
#include "../../InputModel.h"
#include "../../OutputModel.h"
#include "../../../sqlite/sqlite-amalgamation-3071700/sqlite3.h"

std::string const Table_KAD_COUNT::KAD_COUNT__DMU_CATEGORY_STRING_CODE = "DMU_CATEGORY_UUID";
std::string const Table_KAD_COUNT::KAD_COUNT__COUNT = "COUNT";

void Table_KAD_COUNT::Load(sqlite3 * db, OutputModel * output_model_, InputModel * input_model_)
{

	if (!input_model_)
	{
		return;
	}

	std::lock_guard<std::recursive_mutex> data_lock(data_mutex);

	identifiers.clear();

	sqlite3_stmt * stmt = NULL;
	std::string sql("SELECT * FROM KAD_COUNT");	
	sqlite3_prepare_v2(db, sql.c_str(), sql.size() + 1, &stmt, NULL);
	if (stmt == NULL)
	{
		return;
	}
	int step_result = 0;
	while ((step_result = sqlite3_step(stmt)) == SQLITE_ROW)
	{

		// ****************************************************************************************//
		// Use codes as foreign keys, not UUID's, so that this output model can be shared with others
		// ****************************************************************************************//
		char const * code_dmu_category = reinterpret_cast<char const *>(sqlite3_column_text(stmt, INDEX__KAD_COUNT__DMU_CATEGORY_STRING_CODE));
		int const kad_count = reinterpret_cast<int>(sqlite3_column_text(stmt, INDEX__KAD_COUNT__COUNT));
		if (code_dmu_category && strlen(code_dmu_category))
		{
			WidgetInstanceIdentifier identifier;
			bool found_parent = input_model_->t_dmu_category.getIdentifierFromStringCode(code_dmu_category, identifier);
			if (found_parent && identifier.uuid && identifier.uuid->size() > 0)
			{
				identifiers.push_back(std::make_pair(identifier, kad_count));
			}
		}

	}
}
