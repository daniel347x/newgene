#include "DMU.h"
#include "../../../sqlite/sqlite-amalgamation-3071700/sqlite3.h"

#ifndef Q_MOC_RUN
#	include <boost/algorithm/string.hpp>
#endif
#include "../../InputModel.h"

std::string const Table_DMU_Identifier::DMU_CATEGORY_UUID = "DMU_CATEGORY_UUID";
std::string const Table_DMU_Identifier::DMU_CATEGORY_STRING_CODE = "DMU_CATEGORY_STRING_CODE";
std::string const Table_DMU_Identifier::DMU_CATEGORY_STRING_LONGHAND = "DMU_CATEGORY_STRING_LONGHAND";
std::string const Table_DMU_Identifier::DMU_CATEGORY_NOTES1 = "DMU_CATEGORY_NOTES1";
std::string const Table_DMU_Identifier::DMU_CATEGORY_NOTES2 = "DMU_CATEGORY_NOTES2";
std::string const Table_DMU_Identifier::DMU_CATEGORY_NOTES3 = "DMU_CATEGORY_NOTES3";
std::string const Table_DMU_Identifier::DMU_CATEGORY_FLAGS = "DMU_CATEGORY_FLAGS";

std::string const Table_DMU_Instance::DMU_SET_MEMBER_UUID = "DMU_SET_MEMBER_UUID";
std::string const Table_DMU_Instance::DMU_SET_MEMBER_STRING_CODE = "DMU_SET_MEMBER_STRING_CODE";
std::string const Table_DMU_Instance::DMU_SET_MEMBER_STRING_LONGHAND = "DMU_SET_MEMBER_STRING_LONGHAND";
std::string const Table_DMU_Instance::DMU_SET_MEMBER_NOTES1 = "DMU_SET_MEMBER_NOTES1";
std::string const Table_DMU_Instance::DMU_SET_MEMBER_NOTES2 = "DMU_SET_MEMBER_NOTES2";
std::string const Table_DMU_Instance::DMU_SET_MEMBER_NOTES3 = "DMU_SET_MEMBER_NOTES3";
std::string const Table_DMU_Instance::DMU_SET_MEMBER_FK_DMU_CATEGORY_UUID = "DMU_SET_MEMBER_FK_DMU_CATEGORY_UUID";
std::string const Table_DMU_Instance::DMU_SET_MEMBER_FLAGS = "DMU_SET_MEMBER_FLAGS";

void Table_DMU_Identifier::Load(sqlite3 * db, InputModel * input_model_)
{

	std::lock_guard<std::recursive_mutex> data_lock(data_mutex);

	identifiers.clear();

	sqlite3_stmt * stmt = NULL;
	std::string sql("SELECT * FROM DMU_CATEGORY");
	sqlite3_prepare_v2(db, sql.c_str(), static_cast<int>(sql.size()) + 1, &stmt, NULL);
	if (stmt == NULL)
	{
		return;
	}
	int step_result = 0;
	while ((step_result = sqlite3_step(stmt)) == SQLITE_ROW)
	{
		char const * uuid = reinterpret_cast<char const *>(sqlite3_column_text(stmt, INDEX__DMU_CATEGORY_UUID));
		char const * code = reinterpret_cast<char const *>(sqlite3_column_text(stmt, INDEX__DMU_CATEGORY_STRING_CODE));
		char const * longhand = reinterpret_cast<char const *>(sqlite3_column_text(stmt, INDEX__DMU_CATEGORY_STRING_LONGHAND));
		char const * notes1 = reinterpret_cast<char const *>(sqlite3_column_text(stmt, INDEX__DMU_CATEGORY_NOTES1));
		char const * notes2 = reinterpret_cast<char const *>(sqlite3_column_text(stmt, INDEX__DMU_CATEGORY_NOTES2));
		char const * notes3 = reinterpret_cast<char const *>(sqlite3_column_text(stmt, INDEX__DMU_CATEGORY_NOTES3));
		char const * flags = reinterpret_cast<char const *>(sqlite3_column_text(stmt, INDEX__DMU_CATEGORY_FLAGS));
		if (uuid && /* strlen(uuid) == UUID_LENGTH && */ code && strlen(code) && longhand && strlen(longhand))
		{
			WidgetInstanceIdentifier DMU_category_identifier(uuid, code, longhand, 0, flags, TIME_GRANULARITY__NONE, MakeNotes(notes1, notes2, notes3));
			identifiers.push_back(DMU_category_identifier);
		}
	}
	if (stmt)
	{
		sqlite3_finalize(stmt);
		stmt = nullptr;
	}
}

void Table_DMU_Instance::Load(sqlite3 * db, InputModel * input_model_)
{

	std::lock_guard<std::recursive_mutex> data_lock(data_mutex);

	identifiers_map.clear();

	sqlite3_stmt * stmt = NULL;
	std::string sql("SELECT * FROM DMU_SET_MEMBER");
	sqlite3_prepare_v2(db, sql.c_str(), static_cast<int>(sql.size()) + 1, &stmt, NULL);
	if (stmt == NULL)
	{
		return;
	}
	int step_result = 0;

	while ((step_result = sqlite3_step(stmt)) == SQLITE_ROW)
	{
		char const * uuid = reinterpret_cast<char const *>(sqlite3_column_text(stmt, INDEX__DMU_SET_MEMBER_UUID));
		char const * code = reinterpret_cast<char const *>(sqlite3_column_text(stmt, INDEX__DMU_SET_MEMBER_STRING_CODE));
		char const * longhand = reinterpret_cast<char const *>(sqlite3_column_text(stmt, INDEX__DMU_SET_MEMBER_STRING_LONGHAND));
		char const * notes1 = reinterpret_cast<char const *>(sqlite3_column_text(stmt, INDEX__DMU_SET_MEMBER_NOTES1));
		char const * notes2 = reinterpret_cast<char const *>(sqlite3_column_text(stmt, INDEX__DMU_SET_MEMBER_NOTES2));
		char const * notes3 = reinterpret_cast<char const *>(sqlite3_column_text(stmt, INDEX__DMU_SET_MEMBER_NOTES3));
		char const * fk_DMU_uuid = reinterpret_cast<char const *>(sqlite3_column_text(stmt, INDEX__DMU_SET_MEMBER_FK_DMU_CATEGORY_UUID));
		char const * flags = reinterpret_cast<char const *>(sqlite3_column_text(stmt, INDEX__DMU_SET_MEMBER_FLAGS));
		if (uuid && fk_DMU_uuid)
		{
			std::string code_string;
			if (code)
			{
				code_string = code;
			}
			std::string longhand_string;
			if (longhand)
			{
				longhand_string = longhand;
			}
			std::string flags_string;
			if (flags)
			{
				flags_string = flags;
			}
			std::string notes_string_1;
			if (notes1)
			{
				notes_string_1 = notes1;
			}
			std::string notes_string_2;
			if (notes2)
			{
				notes_string_2 = notes2;
			}
			std::string notes_string_3;
			if (notes3)
			{
				notes_string_3 = notes3;
			}
			identifiers_map[fk_DMU_uuid].push_back(WidgetInstanceIdentifier(uuid, input_model_->t_dmu_category.getIdentifier(fk_DMU_uuid), code_string.c_str(), longhand_string.c_str(), 0, flags_string.c_str(), TIME_GRANULARITY__NONE, MakeNotes(notes_string_1.c_str(), notes_string_2.c_str(), notes_string_3.c_str())));
		}
	}

	if (stmt)
	{
		sqlite3_finalize(stmt);
		stmt = nullptr;
	}
}
