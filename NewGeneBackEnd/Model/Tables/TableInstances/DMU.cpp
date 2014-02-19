#include "DMU.h"
#include "../../../sqlite/sqlite-amalgamation-3071700/sqlite3.h"

#ifndef Q_MOC_RUN
#	include <boost/algorithm/string.hpp>
#endif
#include "../../InputModel.h"
#include "../../../Utilities/UUID.h"

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

bool Table_DMU_Identifier::Exists(sqlite3 * db, InputModel & input_model_, std::string const & dmu, bool const also_confirm_using_cache)
{

	std::lock_guard<std::recursive_mutex> data_lock(data_mutex);

	std::string dmu_to_check(boost::to_upper_copy(dmu));

	sqlite3_stmt * stmt = NULL;
	std::string sql("SELECT COUNT(*) FROM DMU_CATEGORY WHERE DMU_CATEGORY_STRING_CODE = '");
	sql += dmu_to_check;
	sql += "'";
	sqlite3_prepare_v2(db, sql.c_str(), static_cast<int>(sql.size()) + 1, &stmt, NULL);
	if (stmt == NULL)
	{
		boost::format msg("Unable to prepare SELECT statement to search for an existing DMU category.");
		throw NewGeneException() << newgene_error_description(msg.str());
	}
	int step_result = 0;
	bool exists = false;
	while ((step_result = sqlite3_step(stmt)) == SQLITE_ROW)
	{
		int existing_dmu_count = sqlite3_column_int(stmt, 0);
		if (existing_dmu_count == 1)
		{
			exists = true;
		}
	}
	if (stmt)
	{
		sqlite3_finalize(stmt);
		stmt = nullptr;
	}

	if (also_confirm_using_cache)
	{
		// Safety check: Cache should match database
		auto found = std::find_if(identifiers.cbegin(), identifiers.cend(), std::bind(&WidgetInstanceIdentifier::IsEqual, std::placeholders::_1, WidgetInstanceIdentifier::EQUALITY_CHECK_TYPE__STRING_CODE, WidgetInstanceIdentifier(dmu_to_check)));
		bool exists_in_cache = (found != identifiers.cend());
		if (exists != exists_in_cache)
		{
			boost::format msg("Cache of DMU categories is out-of-sync.");
			throw NewGeneException() << newgene_error_description(msg.str());
		}
	}

	return exists;

}

bool Table_DMU_Identifier::CreateNewDMU(sqlite3 * db, InputModel & input_model_, std::string const & dmu, std::string const & dmu_description)
{

	std::lock_guard<std::recursive_mutex> data_lock(data_mutex);

	Executor theExecutor(db);

	bool already_exists = Exists(db, input_model_, dmu);
	if (already_exists)
	{
		return false;
	}

	std::string new_uuid(boost::to_upper_copy(newUUID(false)));
	sqlite3_stmt * stmt = NULL;
	std::string sql("INSERT INTO DMU_CATEGORY (DMU_CATEGORY_UUID, DMU_CATEGORY_STRING_CODE, DMU_CATEGORY_STRING_LONGHAND) VALUES ('");
	sql += new_uuid;
	sql += "', ?, ?)";
	sqlite3_prepare_v2(db, sql.c_str(), static_cast<int>(sql.size()) + 1, &stmt, NULL);
	if (stmt == NULL)
	{
		boost::format msg("Unable to prepare INSERT statement to create a new DMU category.");
		throw NewGeneException() << newgene_error_description(msg.str());
	}
	std::string new_dmu(boost::to_upper_copy(dmu));
	sqlite3_bind_text(stmt, 1, new_dmu.c_str(), -1, SQLITE_TRANSIENT);
	sqlite3_bind_text(stmt, 2, dmu_description.c_str(), -1, SQLITE_TRANSIENT);
	int step_result = 0;
	step_result = sqlite3_step(stmt);
	if (step_result != SQLITE_DONE)
	{
		boost::format msg("Unable to execute INSERT statement to create a new DMU category: %1%");
		msg % sqlite3_errstr(step_result);
		throw NewGeneException() << newgene_error_description(msg.str());
	}
	if (stmt)
	{
		sqlite3_finalize(stmt);
		stmt = nullptr;
	}

	std::string flags;
	WidgetInstanceIdentifier DMU_category_identifier(new_uuid, new_dmu, dmu_description, 0, flags.c_str(), TIME_GRANULARITY__NONE, MakeNotes(std::string(), std::string(), std::string()));
	identifiers.push_back(DMU_category_identifier);
	Sort();

	theExecutor.success();

	return theExecutor.succeeded();

}

bool Table_DMU_Identifier::DeleteDMU(sqlite3 * db, InputModel & input_model_, WidgetInstanceIdentifier & dmu)
{

	std::lock_guard<std::recursive_mutex> data_lock(data_mutex);

	Executor theExecutor(db);

	if (!dmu.code || !dmu.uuid)
	{
		return false;
	}

	bool already_exists = Exists(db, input_model_, *dmu.code);
	if (!already_exists)
	{
		return false;
	}

	// Since the schema doesn't allow FK pointing from UOA to UOA_LOOKUP,
	// we must reverse-map to the UOA's here and then remove those
	WidgetInstanceIdentifiers uoas = input_model_.t_uoa_setmemberlookup.RetrieveUOAsGivenDMU(db, &input_model_, dmu);
	std::for_each(uoas.cbegin(), uoas.cend(), [&](WidgetInstanceIdentifier const & uoa)
	{
		input_model_.t_uoa_category.DeleteUOA(db, input_model_, uoa);
	});

	sqlite3_stmt * stmt = NULL;
	std::string sql("DELETE FROM DMU_CATEGORY WHERE DMU_CATEGORY_UUID = ?");
	sqlite3_prepare_v2(db, sql.c_str(), static_cast<int>(sql.size()) + 1, &stmt, NULL);
	if (stmt == NULL)
	{
		boost::format msg("Unable to prepare DELETE statement to delete a DMU category.");
		throw NewGeneException() << newgene_error_description(msg.str());
	}
	sqlite3_bind_text(stmt, 1, dmu.uuid->c_str(), -1, SQLITE_TRANSIENT);
	int step_result = 0;
	step_result = sqlite3_step(stmt);
	if (step_result != SQLITE_DONE)
	{
		boost::format msg("Unable to execute DELETE statement to delete a DMU category: %1%");
		msg % sqlite3_errstr(step_result);
		throw NewGeneException() << newgene_error_description(msg.str());
	}
	if (stmt)
	{
		sqlite3_finalize(stmt);
		stmt = nullptr;
	}

	// Remove from cache
	std::string flags;
	identifiers.erase(std::remove_if(identifiers.begin(), identifiers.end(), std::bind(&WidgetInstanceIdentifier::IsEqual, std::placeholders::_1, WidgetInstanceIdentifier::EQUALITY_CHECK_TYPE__UUID_PLUS_STRING_CODE, dmu)), identifiers.end());

	theExecutor.success();

	return theExecutor.succeeded();

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
