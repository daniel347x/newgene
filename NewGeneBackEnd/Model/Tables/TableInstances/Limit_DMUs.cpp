#include "Limit_DMUs.h"
#include "../../../sqlite/sqlite-amalgamation-3071700/sqlite3.h"
#include "../../OutputModel.h"
#include "../../../Utilities/UUID.h"
//#ifndef Q_MOC_RUN
//#	include <boost/format.hpp>
//#endif

std::string const Table__Limit_DMUS__Categories::LIMIT_DMUS__DMU_CATEGORY_STRING_CODE = "LIMIT_DMUS__DMU_CATEGORY_STRING_CODE";

std::string const Table__Limit_DMUs__Elements::LIMIT_DMUS__DMU_CATEGORY_STRING_CODE = "LIMIT_DMUS__DMU_CATEGORY_STRING_CODE";
std::string const Table__Limit_DMUs__Elements::LIMIT_DMUS__DMU_SET_MEMBER_UUID = "LIMIT_DMUS__DMU_SET_MEMBER_UUID";

void Table__Limit_DMUS__Categories::Load(sqlite3 * db, OutputModel * output_model_, InputModel * input_model_)
{

	if (!output_model_ || !input_model_)
	{
		return;
	}

	std::lock_guard<std::recursive_mutex> data_lock(data_mutex);

	identifiers.clear();

	sqlite3_stmt * stmt = NULL;
	std::string sql("SELECT * FROM LIMIT_DMUS__CATEGORIES");
	sqlite3_prepare_v2(db, sql.c_str(), static_cast<int>(sql.size()) + 1, &stmt, NULL);
	if (stmt == NULL)
	{
		return;
	}
	int step_result = 0;
	while ((step_result = sqlite3_step(stmt)) == SQLITE_ROW)
	{
		char const * dmu_category_code = reinterpret_cast<char const *>(sqlite3_column_text(stmt, INDEX__LIMIT_DMUS__DMU_CATEGORY_STRING_CODE));
		if (dmu_category_code && /* strlen(dmu_category_code) == UUID_LENGTH && */ strlen(dmu_category_code))
		{
			WidgetInstanceIdentifier dmu_category;
			input_model_->t_dmu_category.getIdentifierFromStringCode(dmu_category_code, dmu_category);
			if (dmu_category.IsEmpty())
			{
				continue;
			}
			identifiers.push_back(dmu_category);
		}
	}
	if (stmt)
	{
		sqlite3_finalize(stmt);
		stmt = nullptr;
	}

	Sort();

}

bool Table__Limit_DMUS__Categories::Exists(sqlite3 * db, OutputModel & output_model_, InputModel & input_model_, std::string const & dmu_category_code, bool const also_confirm_using_cache)
{

	std::lock_guard<std::recursive_mutex> data_lock(data_mutex);

	sqlite3_stmt * stmt = NULL;
	std::string sql("SELECT COUNT(*) FROM `LIMIT_DMUS__CATEGORIES` WHERE LIMIT_DMUS__DMU_CATEGORY_STRING_CODE = '");
	sql += dmu_category_code;
	sql += "'";
	int error_or_success_code = sqlite3_prepare_v2(db, sql.c_str(), static_cast<int>(sql.size()) + 1, &stmt, NULL);
	if (stmt == NULL)
	{
		boost::format msg("Unable to prepare SELECT statement to search for an existing DMU category in the Limit DMUs categories table: %1%");
		msg % sqlite3_errstr(error_or_success_code);
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
		if (getIdentifierFromStringCode(dmu_category_code, WidgetInstanceIdentifier()) != exists)
		{
			boost::format msg("Cache of DMU categories is out-of-sync.");
			throw NewGeneException() << newgene_error_description(msg.str());
		}

	}

	return exists;

}

bool Table__Limit_DMUS__Categories::AddDMU(sqlite3 * db, OutputModel & output_model_, InputModel & input_model_, std::string const & dmu_category_code)
{

	std::lock_guard<std::recursive_mutex> data_lock(data_mutex);

	//Executor theExecutor(db);

	bool already_exists = Exists(db, output_model_, input_model_, dmu_category_code);
	if (already_exists)
	{
		return false;
	}

	WidgetInstanceIdentifier dmu_category;
	input_model_.t_dmu_category.getIdentifierFromStringCode(dmu_category_code, dmu_category);
	if (dmu_category.IsEmpty())
	{
		boost::format msg("Cannot add a non-existent DMU category to the Limit DMUs categories table: %1%");
		msg % dmu_category_code.c_str();
		throw NewGeneException() << newgene_error_description(msg.str());
	}

	sqlite3_stmt * stmt = NULL;
	std::string sql("INSERT INTO LIMIT_DMUS__CATEGORIES VALUES ('");
	sql += dmu_category_code;
	sql += ")";
	sqlite3_prepare_v2(db, sql.c_str(), static_cast<int>(sql.size()) + 1, &stmt, NULL);
	if (stmt == NULL)
	{
		boost::format msg("Unable to prepare INSERT statement to create a new DMU category in the Limit DMUs categories table.");
		throw NewGeneException() << newgene_error_description(msg.str());
	}
	int step_result = 0;
	step_result = sqlite3_step(stmt);
	if (step_result != SQLITE_DONE)
	{
		boost::format msg("Unable to execute INSERT statement to create a new DMU category in the Limit DMUs categories table: %1%");
		msg % sqlite3_errstr(step_result);
		throw NewGeneException() << newgene_error_description(msg.str());
	}
	if (stmt)
	{
		sqlite3_finalize(stmt);
		stmt = nullptr;
	}

	identifiers.push_back(dmu_category);

	Sort();

	//theExecutor.success();

	//return theExecutor.succeeded();
	return true;

}

bool Table__Limit_DMUS__Categories::RemoveDMU(sqlite3 * db, OutputModel & output_model_, InputModel & input_model_, WidgetInstanceIdentifier & dmu_category, DataChangeMessage & change_message)
{

	std::lock_guard<std::recursive_mutex> data_lock(data_mutex);

	//Executor theExecutor(db);

	if (!dmu_category.code || !dmu_category.uuid)
	{
		return false;
	}

	bool already_exists = Exists(db, output_model_, input_model_, *dmu_category.code);
	if (!already_exists)
	{
		return false;
	}

	sqlite3_stmt * stmt = NULL;
	std::string sql("DELETE FROM LIMIT_DMUS__CATEGORIES WHERE LIMIT_DMUS__DMU_CATEGORY_STRING_CODE = ?");
	int err = sqlite3_prepare_v2(db, sql.c_str(), static_cast<int>(sql.size()) + 1, &stmt, NULL);
	if (stmt == NULL)
	{
		boost::format msg("Unable to prepare DELETE statement to delete a DMU category from the Limit DMUs category table: %1%");
		msg % sqlite3_errstr(err);
		throw NewGeneException() << newgene_error_description(msg.str());
	}
	sqlite3_bind_text(stmt, 1, dmu_category.code->c_str(), -1, SQLITE_TRANSIENT);
	int step_result = 0;
	step_result = sqlite3_step(stmt);
	if (step_result != SQLITE_DONE)
	{
		boost::format msg("Unable to execute DELETE statement to delete a DMU category from the Limit DMUs category table: %1%");
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
	identifiers.erase(std::remove_if(identifiers.begin(), identifiers.end(), std::bind(&WidgetInstanceIdentifier::IsEqual, std::placeholders::_1, WidgetInstanceIdentifier::EQUALITY_CHECK_TYPE__UUID_PLUS_STRING_CODE, dmu_category)), identifiers.end());

	// ***************************************** //
	// Prepare data to send back to user interface
	// ***************************************** //
	//DATA_CHANGE_TYPE type = DATA_CHANGE_TYPE__INPUT_MODEL__DMU_CHANGE;
	//DATA_CHANGE_INTENTION intention = DATA_CHANGE_INTENTION__REMOVE;
	//DataChange change(type, intention, dmu, WidgetInstanceIdentifiers());
	//change_message.changes.push_back(change);

	//theExecutor.success();

	//return theExecutor.succeeded();
	return true;

}

void Table__Limit_DMUs__Elements::Load(sqlite3 * db, OutputModel * output_model_, InputModel * input_model_)
{

	if (!output_model_ || !input_model_)
	{
		return;
	}

	std::lock_guard<std::recursive_mutex> data_lock(data_mutex);

	identifiers_map.clear();

	sqlite3_stmt * stmt = NULL;
	std::string sql("SELECT * FROM LIMIT_DMUS__ELEMENTS");
	sqlite3_prepare_v2(db, sql.c_str(), static_cast<int>(sql.size()) + 1, &stmt, NULL);
	if (stmt == NULL)
	{
		return;
	}
	int step_result = 0;

	while ((step_result = sqlite3_step(stmt)) == SQLITE_ROW)
	{
		char const * parent_dmu_category_code = reinterpret_cast<char const *>(sqlite3_column_text(stmt, INDEX__LIMIT_DMUS__DMU_CATEGORY_STRING_CODE));
		char const * dmu_set_member_uuid = reinterpret_cast<char const *>(sqlite3_column_text(stmt, INDEX__LIMIT_DMUS__DMU_SET_MEMBER_UUID));
		if (parent_dmu_category_code && dmu_set_member_uuid && strlen(parent_dmu_category_code) && strlen(dmu_set_member_uuid))
		{
			WidgetInstanceIdentifier parent_dmu_category;
			input_model_->t_dmu_category.getIdentifierFromStringCode(parent_dmu_category_code, parent_dmu_category);
			if (parent_dmu_category.IsEmpty())
			{
				continue;
			}
			WidgetInstanceIdentifier dmu_set_member = input_model_->t_dmu_setmembers.getIdentifier(dmu_set_member_uuid, *parent_dmu_category.uuid);
			if (dmu_set_member.IsEmpty())
			{
				continue;
			}
			identifiers_map[parent_dmu_category_code].push_back(dmu_set_member);
		}
	}

	if (stmt)
	{
		sqlite3_finalize(stmt);
		stmt = nullptr;
	}

}

bool Table__Limit_DMUs__Elements::Exists(sqlite3 * db, OutputModel & output_model_, InputModel & input_model_, WidgetInstanceIdentifier const & dmu_category, std::string const & dmu_member_uuid, bool const also_confirm_using_cache)
{

	std::lock_guard<std::recursive_mutex> data_lock(data_mutex);

	if (!dmu_category.uuid || dmu_category.uuid->empty() || !dmu_category.code || dmu_category.code->empty() || dmu_member_uuid.empty())
	{
		return false;
	}

	sqlite3_stmt * stmt = NULL;
	std::string sql("SELECT COUNT(*) FROM LIMIT_DMUS__ELEMENTS WHERE LIMIT_DMUS__DMU_CATEGORY_STRING_CODE = '");
	sql += *dmu_category.code;
	sql += "' AND LIMIT_DMUS__DMU_SET_MEMBER_UUID = '";
	sql += dmu_member_uuid;
	sql += "'";
	sqlite3_prepare_v2(db, sql.c_str(), static_cast<int>(sql.size()) + 1, &stmt, NULL);
	if (stmt == NULL)
	{
		boost::format msg("Unable to prepare SELECT statement to search for an existing DMU member from the Limit DMUs category table.");
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
		if (!getIdentifier(*dmu_category.code, dmu_member_uuid).IsEmpty() == exists)
		{
			boost::format msg("Cache of the Limit DMUs member table is out-of-sync.");
			throw NewGeneException() << newgene_error_description(msg.str());
		}
	}

	return exists;

}

WidgetInstanceIdentifier Table__Limit_DMUs__Elements::AddDmuMember(sqlite3 * db, OutputModel & output_model_, InputModel & input_model_, WidgetInstanceIdentifier const & dmu_category, std::string const & dmu_member_uuid)
{

	std::lock_guard<std::recursive_mutex> data_lock(data_mutex);

	//Executor theExecutor(db);

	bool already_exists = Exists(db, output_model_, input_model_, dmu_category, dmu_member_uuid);
	if (already_exists)
	{
		return getIdentifier(dmu_member_uuid, *dmu_category.code);
	}

	WidgetInstanceIdentifier dmu_set_member = input_model_.t_dmu_setmembers.getIdentifier(dmu_member_uuid, *dmu_category.uuid);
	if (dmu_set_member.IsEmpty())
	{
		boost::format msg("Unable to locate existing DMU set member while attempting to insert a new DMU set member in the Limit DMU set members table");
		throw NewGeneException() << newgene_error_description(msg.str());
	}

	std::string new_uuid(dmu_member_uuid);
	sqlite3_stmt * stmt = NULL;
	std::string sql("INSERT INTO LIMIT_DMUS__ELEMENTS (LIMIT_DMUS__DMU_CATEGORY_STRING_CODE, LIMIT_DMUS__DMU_SET_MEMBER_UUID) VALUES (");
	sql += "'";
	sql += *dmu_category.code;
	sql += "', '";
	sql += dmu_member_uuid;
	sql += "')";
	int err = sqlite3_prepare_v2(db, sql.c_str(), static_cast<int>(sql.size()) + 1, &stmt, NULL);
	if (stmt == NULL)
	{
		boost::format msg("Unable to prepare INSERT statement to create a new DMU member for the Limit DMUs member table: %1%");
		msg % sqlite3_errstr(err);
		throw NewGeneException() << newgene_error_description(msg.str());
	}
	int step_result = 0;
	step_result = sqlite3_step(stmt);
	if (step_result != SQLITE_DONE)
	{
		boost::format msg("Unable to execute INSERT statement to create a new DMU member in the Limit DMUs member table: %1%");
		msg % sqlite3_errstr(step_result);
		throw NewGeneException() << newgene_error_description(msg.str());
	}
	if (stmt)
	{
		sqlite3_finalize(stmt);
		stmt = nullptr;
	}

	// Append to cache
	identifiers_map[*dmu_category.code].push_back(dmu_set_member);

	//theExecutor.success();

	return dmu_set_member;

}

bool Table__Limit_DMUs__Elements::RemoveDmuMember(sqlite3 * db, OutputModel & output_model_, InputModel & input_model_, WidgetInstanceIdentifier const & dmu_category, std::string const & dmu_member_uuid)
{

	std::lock_guard<std::recursive_mutex> data_lock(data_mutex);

	//Executor theExecutor(db);

	bool already_exists = Exists(db, output_model_, input_model_, dmu_category, dmu_member_uuid);
	if (!already_exists)
	{
		return false;
	}

	sqlite3_stmt * stmt = NULL;
	boost::format sql("DELETE FROM LIMIT_DMUS__ELEMENTS WHERE LIMIT_DMUS__DMU_CATEGORY_STRING_CODE = '%1%' AND LIMIT_DMUS__DMU_SET_MEMBER_UUID = '%2%'");
	sql % *dmu_category.code % dmu_member_uuid;
	int err = sqlite3_prepare_v2(db, sql.str().c_str(), static_cast<int>(sql.str().size()) + 1, &stmt, NULL);
	if (stmt == NULL)
	{
		boost::format msg("Unable to prepare DELETE statement to delete DMU member from the Limit DMUs member table: %1%");
		msg % sqlite3_errstr(err);
		throw NewGeneException() << newgene_error_description(msg.str());
	}
	int step_result = 0;
	step_result = sqlite3_step(stmt);
	if (step_result != SQLITE_DONE)
	{
		boost::format msg("Unable to execute DELETE statement to delete DMU member from the Limit DMUs member table: %1%");
		msg % sqlite3_errstr(err);
		throw NewGeneException() << newgene_error_description(msg.str());
	}
	if (stmt)
	{
		sqlite3_finalize(stmt);
		stmt = nullptr;
	}

	// Remove from cache
	identifiers_map[*dmu_category.code].erase(std::remove_if(identifiers_map[*dmu_category.code].begin(), identifiers_map[*dmu_category.code].end(), [&](WidgetInstanceIdentifier & test_dmu_member)
	{
		if (test_dmu_member.IsEqual(WidgetInstanceIdentifier::EQUALITY_CHECK_TYPE__UUID, dmu_member_uuid))
		{
			return true;
		}
		return false;
	}), identifiers_map[*dmu_category.code].end());

	//theExecutor.success();

	//return theExecutor.succeeded();
	return true;

}
