#include "UOA.h"
#include "../../../sqlite/sqlite-amalgamation-3071700/sqlite3.h"

#ifndef Q_MOC_RUN
#	include <boost/algorithm/string.hpp>
#endif
#include "../../InputModel.h"

std::string const Table_UOA_Identifier::UOA_CATEGORY_UUID = "UOA_CATEGORY_UUID";
std::string const Table_UOA_Identifier::UOA_CATEGORY_STRING_CODE = "UOA_CATEGORY_STRING_CODE";
std::string const Table_UOA_Identifier::UOA_CATEGORY_STRING_LONGHAND = "UOA_CATEGORY_STRING_LONGHAND";
std::string const Table_UOA_Identifier::UOA_CATEGORY_TIME_GRANULARITY = "UOA_CATEGORY_TIME_GRANULARITY";
std::string const Table_UOA_Identifier::UOA_CATEGORY_NOTES1 = "UOA_CATEGORY_NOTES1";
std::string const Table_UOA_Identifier::UOA_CATEGORY_NOTES2 = "UOA_CATEGORY_NOTES2";
std::string const Table_UOA_Identifier::UOA_CATEGORY_NOTES3 = "UOA_CATEGORY_NOTES3";
std::string const Table_UOA_Identifier::UOA_CATEGORY_FLAGS = "UOA_CATEGORY_FLAGS";

std::string const Table_UOA_Member::UOA_CATEGORY_LOOKUP_UOA_CATEGORY_UUID = "UOA_CATEGORY_LOOKUP_UOA_CATEGORY_UUID";
std::string const Table_UOA_Member::UOA_CATEGORY_LOOKUP_SEQUENCE_NUMBER = "UOA_CATEGORY_LOOKUP_SEQUENCE_NUMBER";
std::string const Table_UOA_Member::UOA_CATEGORY_LOOKUP_FK_DMU_CATEGORY_UUID = "UOA_CATEGORY_LOOKUP_FK_DMU_CATEGORY_UUID";

void Table_UOA_Identifier::Load(sqlite3 * db, InputModel * input_model_)
{

	std::lock_guard<std::recursive_mutex> data_lock(data_mutex);

	identifiers.clear();

	sqlite3_stmt * stmt = NULL;
	std::string sql("SELECT * FROM UOA_CATEGORY");	
	sqlite3_prepare_v2(db, sql.c_str(), sql.size() + 1, &stmt, NULL);
	if (stmt == NULL)
	{
		return;
	}
	int step_result = 0;
	while ((step_result = sqlite3_step(stmt)) == SQLITE_ROW)
	{
		char const * uuid = reinterpret_cast<char const *>(sqlite3_column_text(stmt, INDEX__UOA_CATEGORY_UUID));
		char const * code = reinterpret_cast<char const *>(sqlite3_column_text(stmt, INDEX__UOA_CATEGORY_STRING_CODE));
		char const * longhand = reinterpret_cast<char const *>(sqlite3_column_text(stmt, INDEX__UOA_CATEGORY_STRING_LONGHAND));
		TIME_GRANULARITY time_granularity = static_cast<TIME_GRANULARITY>(reinterpret_cast<int>(sqlite3_column_text(stmt, INDEX__UOA_CATEGORY_TIME_GRANULARITY)));
		char const * notes1 = reinterpret_cast<char const *>(sqlite3_column_text(stmt, INDEX__UOA_CATEGORY_NOTES1));
		char const * notes2 = reinterpret_cast<char const *>(sqlite3_column_text(stmt, INDEX__UOA_CATEGORY_NOTES2));
		char const * notes3 = reinterpret_cast<char const *>(sqlite3_column_text(stmt, INDEX__UOA_CATEGORY_NOTES3));
		char const * flags = reinterpret_cast<char const *>(sqlite3_column_text(stmt, INDEX__UOA_CATEGORY_FLAGS));
		if (uuid /* && */ /* strlen(uuid) == UUID_LENGTH && */ )
		{
			WidgetInstanceIdentifier uoa_category_identifier(uuid, code, longhand, 0, flags, time_granularity, MakeNotes(notes1, notes2, notes3));
			identifiers.push_back(uoa_category_identifier);
		}
	}
}

void Table_UOA_Member::Load(sqlite3 * db, InputModel * input_model_)
{

	std::lock_guard<std::recursive_mutex> data_lock(data_mutex);

	identifiers_map.clear();

	sqlite3_stmt * stmt = NULL;
	std::string sql("SELECT * FROM VG_SET_MEMBER");
	sqlite3_prepare_v2(db, sql.c_str(), sql.size() + 1, &stmt, NULL);
	if (stmt == NULL)
	{
		return;
	}
	int step_result = 0;

	while ((step_result = sqlite3_step(stmt)) == SQLITE_ROW)
	{
		char const * uuid = reinterpret_cast<char const *>(sqlite3_column_text(stmt, INDEX__UOA_CATEGORY_LOOKUP_UOA_CATEGORY_UUID));
		int const sequence_number = reinterpret_cast<int>(sqlite3_column_text(stmt, INDEX__UOA_CATEGORY_LOOKUP_SEQUENCE_NUMBER));
		char const * dmu_uuid = reinterpret_cast<char const *>(sqlite3_column_text(stmt, INDEX__UOA_CATEGORY_LOOKUP_FK_DMU_CATEGORY_UUID));
		if (uuid && /* strlen(uuid) == UUID_LENGTH && */ dmu_uuid /* && strlen(dmu_uuid) == UUID_LENGTH */ )
		{
			identifiers_map[uuid].push_back(WidgetInstanceIdentifier(uuid, input_model_->t_dmu_category.getIdentifier(dmu_uuid), "", "", sequence_number));
		}
	}
}
