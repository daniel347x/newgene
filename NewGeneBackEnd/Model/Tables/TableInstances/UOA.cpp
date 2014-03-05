#include "UOA.h"
#include "../../../sqlite/sqlite-amalgamation-3071700/sqlite3.h"

#ifndef Q_MOC_RUN
#	include <boost/algorithm/string.hpp>
#endif
#include "../../InputModel.h"
#include "../../../Utilities/UUID.h"

#include <unordered_set>

std::string const Table_UOA_Identifier::UOA_CATEGORY_UUID = "UOA_CATEGORY_UUID";
std::string const Table_UOA_Identifier::UOA_CATEGORY_STRING_CODE = "UOA_CATEGORY_STRING_CODE";
std::string const Table_UOA_Identifier::UOA_CATEGORY_STRING_LONGHAND = "UOA_CATEGORY_STRING_LONGHAND";
std::string const Table_UOA_Identifier::UOA_CATEGORY_TIME_GRANULARITY = "UOA_CATEGORY_TIME_GRANULARITY";
std::string const Table_UOA_Identifier::UOA_CATEGORY_NOTES1 = "UOA_CATEGORY_NOTES1";
std::string const Table_UOA_Identifier::UOA_CATEGORY_NOTES2 = "UOA_CATEGORY_NOTES2";
std::string const Table_UOA_Identifier::UOA_CATEGORY_NOTES3 = "UOA_CATEGORY_NOTES3";
std::string const Table_UOA_Identifier::UOA_CATEGORY_FLAGS = "UOA_CATEGORY_FLAGS";

std::string const Table_UOA_Member::UOA_CATEGORY_LOOKUP_FK_UOA_CATEGORY_UUID = "UOA_CATEGORY_LOOKUP_FK_UOA_CATEGORY_UUID";
std::string const Table_UOA_Member::UOA_CATEGORY_LOOKUP_SEQUENCE_NUMBER = "UOA_CATEGORY_LOOKUP_SEQUENCE_NUMBER";
std::string const Table_UOA_Member::UOA_CATEGORY_LOOKUP_FK_DMU_CATEGORY_UUID = "UOA_CATEGORY_LOOKUP_FK_DMU_CATEGORY_UUID";

void Table_UOA_Identifier::Load(sqlite3 * db, InputModel * input_model_)
{

	std::lock_guard<std::recursive_mutex> data_lock(data_mutex);

	identifiers.clear();

	sqlite3_stmt * stmt = NULL;
	std::string sql("SELECT * FROM UOA_CATEGORY");	
	sqlite3_prepare_v2(db, sql.c_str(), static_cast<int>(sql.size()) + 1, &stmt, NULL);
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
		TIME_GRANULARITY time_granularity = static_cast<TIME_GRANULARITY>(static_cast<int>(sqlite3_column_int(stmt, INDEX__UOA_CATEGORY_TIME_GRANULARITY)));
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
	if (stmt)
	{
		sqlite3_finalize(stmt);
		stmt = nullptr;
	}

}

WidgetInstanceIdentifiers Table_UOA_Identifier::RetrieveDMUCategories(sqlite3 const * db, InputModel * input_model_, UUID const & uuid)
{

	if (!db)
	{
		return WidgetInstanceIdentifiers();
	}

	if (!input_model_)
	{
		return WidgetInstanceIdentifiers();
	}

	WidgetInstanceIdentifier uoa = getIdentifier(uuid);
	if (uoa.IsEmpty())
	{
		return WidgetInstanceIdentifiers();
	}

	WidgetInstanceIdentifiers dmu_categories = input_model_->t_uoa_setmemberlookup.getIdentifiers(uuid);

	return dmu_categories;

}

Table_UOA_Identifier::DMU_Counts Table_UOA_Identifier::RetrieveDMUCounts(sqlite3 const * db, InputModel * input_model_, UUID const & uuid)
{

	if (!db)
	{
		return DMU_Counts();
	}

	if (!input_model_)
	{
		return DMU_Counts();
	}

	WidgetInstanceIdentifier uoa = getIdentifier(uuid);
	if (uoa.IsEmpty())
	{
		return DMU_Counts();
	}

	WidgetInstanceIdentifiers dmu_categories = input_model_->t_uoa_setmemberlookup.getIdentifiers(uuid);

	DMU_Counts dmu_counts;
	std::for_each(dmu_categories.cbegin(), dmu_categories.cend(), [&dmu_counts](WidgetInstanceIdentifier const & dmu_category)
	{
		bool found = false;
		std::for_each(dmu_counts.begin(), dmu_counts.end(), [&dmu_category, &found](DMU_Plus_Count & dmu_plus_count)
		{
			if (found)
			{
				return; // from lambda
			}
			if (dmu_plus_count.first.IsEqual(WidgetInstanceIdentifier::EQUALITY_CHECK_TYPE__STRING_CODE, dmu_category))
			{
				found = true;
				++dmu_plus_count.second;
				return; // from lambda
			}
		});
		if (!found)
		{
			dmu_counts.push_back(std::make_pair(dmu_category, 1));
		}
	});

	return dmu_counts;

}

bool Table_UOA_Identifier::Exists(sqlite3 * db, InputModel & input_model_, WidgetInstanceIdentifier const & uoa, bool const also_confirm_using_cache)
{

	std::lock_guard<std::recursive_mutex> data_lock(data_mutex);

	if (!uoa.uuid)
	{
		return false;
	}

	sqlite3_stmt * stmt = NULL;
	std::string sql("SELECT COUNT(*) FROM UOA_CATEGORY WHERE UOA_CATEGORY_UUID = '");
	sql += *uoa.uuid;
	sql += "'";
	sqlite3_prepare_v2(db, sql.c_str(), static_cast<int>(sql.size()) + 1, &stmt, NULL);
	if (stmt == NULL)
	{
		boost::format msg("Unable to prepare SELECT statement to search for an existing UOA.");
		throw NewGeneException() << newgene_error_description(msg.str());
	}
	int step_result = 0;
	bool exists = false;
	while ((step_result = sqlite3_step(stmt)) == SQLITE_ROW)
	{
		int existing_uoa_count = sqlite3_column_int(stmt, 0);
		if (existing_uoa_count == 1)
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
		auto found = std::find_if(identifiers.cbegin(), identifiers.cend(), std::bind(&WidgetInstanceIdentifier::IsEqual, std::placeholders::_1, WidgetInstanceIdentifier::EQUALITY_CHECK_TYPE__UUID, uoa));
		bool exists_in_cache = (found != identifiers.cend());
		if (exists != exists_in_cache)
		{
			boost::format msg("Cache of UOAs is out-of-sync.");
			throw NewGeneException() << newgene_error_description(msg.str());
		}
	}

	return exists;

}

bool Table_UOA_Identifier::ExistsByCode(sqlite3 * db, InputModel & input_model_, std::string const & uoa_code, bool const also_confirm_using_cache)
{

	std::lock_guard<std::recursive_mutex> data_lock(data_mutex);

	std::string proposed_uoa_code(uoa_code);
	boost::trim(proposed_uoa_code);

	if (proposed_uoa_code.empty())
	{
		return false;
	}

	sqlite3_stmt * stmt = NULL;
	std::string sql("SELECT COUNT(*) FROM UOA_CATEGORY WHERE UOA_CATEGORY_STRING_CODE = '");
	sql += proposed_uoa_code;
	sql += "'";
	sqlite3_prepare_v2(db, sql.c_str(), static_cast<int>(sql.size()) + 1, &stmt, NULL);
	if (stmt == NULL)
	{
		boost::format msg("Unable to prepare SELECT statement to search for an existing UOA code.");
		throw NewGeneException() << newgene_error_description(msg.str());
	}
	int step_result = 0;
	bool exists = false;
	while ((step_result = sqlite3_step(stmt)) == SQLITE_ROW)
	{
		int existing_uoa_count = sqlite3_column_int(stmt, 0);
		if (existing_uoa_count == 1)
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
		auto found = std::find_if(identifiers.cbegin(), identifiers.cend(), std::bind(&WidgetInstanceIdentifier::IsEqual, std::placeholders::_1, WidgetInstanceIdentifier::EQUALITY_CHECK_TYPE__STRING_CODE, proposed_uoa_code));
		bool exists_in_cache = (found != identifiers.cend());
		if (exists != exists_in_cache)
		{
			boost::format msg("Cache of UOAs is out-of-sync.");
			throw NewGeneException() << newgene_error_description(msg.str());
		}
	}

	return exists;

}

bool Table_UOA_Identifier::DeleteUOA(sqlite3 * db, InputModel & input_model_, WidgetInstanceIdentifier const & uoa, DataChangeMessage & change_message)
{

	std::lock_guard<std::recursive_mutex> data_lock(data_mutex);

	Executor theExecutor(db);

	if (!uoa.uuid)
	{
		return false;
	}

	bool already_exists = Exists(db, input_model_, uoa);
	if (!already_exists)
	{
		return false;
	}

	// Before removing the UOA itself,
	// pull up any associated variable group instance data tables
	// (which for robustness of potential user data in error do not enforce FK's)
	// and remove them
	WidgetInstanceIdentifiers vgs_to_delete = input_model_.t_vgp_identifiers.RetrieveVGsFromUOA(db, &input_model_, *uoa.uuid);
	std::for_each(vgs_to_delete.cbegin(), vgs_to_delete.cend(), [&](WidgetInstanceIdentifier const & vg_to_delete)
	{
		input_model_.t_vgp_identifiers.DeleteVG(db, &input_model_, vg_to_delete, change_message);
	});

	sqlite3_stmt * stmt = NULL;
	std::string sql("DELETE FROM UOA_CATEGORY WHERE UOA_CATEGORY_UUID = '");
	sql += *uoa.uuid;
	sql += "'";
	sqlite3_prepare_v2(db, sql.c_str(), static_cast<int>(sql.size()) + 1, &stmt, NULL);
	if (stmt == NULL)
	{
		boost::format msg("Unable to prepare DELETE statement to delete a UOA.");
		throw NewGeneException() << newgene_error_description(msg.str());
	}
	int step_result = 0;
	step_result = sqlite3_step(stmt);
	if (step_result != SQLITE_DONE)
	{
		boost::format msg("Unable to execute DELETE statement to delete a UOA: %1%");
		msg % sqlite3_errstr(step_result);
		throw NewGeneException() << newgene_error_description(msg.str());
	}
	if (stmt)
	{
		sqlite3_finalize(stmt);
		stmt = nullptr;
	}

	// Remove from cache for table UOA_CATEGORY_LOOKUP
	input_model_.t_uoa_setmemberlookup.DeleteUOA(db, input_model_, uoa);

	// Remove from our cache
	identifiers.erase(std::remove_if(identifiers.begin(), identifiers.end(), std::bind(&WidgetInstanceIdentifier::IsEqual, std::placeholders::_1, WidgetInstanceIdentifier::EQUALITY_CHECK_TYPE__UUID, uoa)), identifiers.end());

	// ***************************************** //
	// Prepare data to send back to user interface
	// ***************************************** //
	DATA_CHANGE_TYPE type = DATA_CHANGE_TYPE__INPUT_MODEL__UOA_CHANGE;
	DATA_CHANGE_INTENTION intention = DATA_CHANGE_INTENTION__REMOVE;
	DataChange change(type, intention, uoa, WidgetInstanceIdentifiers());
	change_message.changes.push_back(change);

	theExecutor.success();

	return theExecutor.succeeded();

}

bool Table_UOA_Identifier::CreateNewUOA(sqlite3 * db, InputModel & input_model, std::string const & new_uoa_code, std::string const & uoa_description, WidgetInstanceIdentifiers const & dmu_categories, TIME_GRANULARITY const & time_granularity)
{

	std::lock_guard<std::recursive_mutex> data_lock(data_mutex);

	Executor theExecutor(db);

	std::string uoa_code = boost::trim_copy(new_uoa_code);

	bool already_exists = ExistsByCode(db, input_model, uoa_code);
	if (already_exists)
	{
		return false;
	}

	std::string new_uuid(boost::to_upper_copy(newUUID(false)));
	sqlite3_stmt * stmt = NULL;
	std::string sql("INSERT INTO UOA_CATEGORY (UOA_CATEGORY_UUID, UOA_CATEGORY_STRING_CODE, UOA_CATEGORY_STRING_LONGHAND, UOA_CATEGORY_TIME_GRANULARITY, UOA_CATEGORY_NOTES1, UOA_CATEGORY_NOTES2, UOA_CATEGORY_NOTES3, UOA_CATEGORY_FLAGS) VALUES ('");
	sql += new_uuid;
	sql += "', ?, ?, ?, '', '', '', '')";
	sqlite3_prepare_v2(db, sql.c_str(), static_cast<int>(sql.size()) + 1, &stmt, NULL);
	if (stmt == NULL)
	{
		boost::format msg("Unable to prepare INSERT statement to create a new unit of analysis.");
		throw NewGeneException() << newgene_error_description(msg.str());
	}
	std::string new_uoa(boost::to_upper_copy(uoa_code));
	sqlite3_bind_text(stmt, 1, new_uoa.c_str(), -1, SQLITE_TRANSIENT);
	sqlite3_bind_text(stmt, 2, uoa_description.c_str(), -1, SQLITE_TRANSIENT);
	sqlite3_bind_int(stmt, 3, (int)time_granularity);
	int step_result = 0;
	step_result = sqlite3_step(stmt);
	if (step_result != SQLITE_DONE)
	{
		boost::format msg("Unable to execute INSERT statement to create a new UOA: %1%");
		msg % sqlite3_errstr(step_result);
		throw NewGeneException() << newgene_error_description(msg.str());
	}
	if (stmt)
	{
		sqlite3_finalize(stmt);
		stmt = nullptr;
	}

	std::string flags;
	WidgetInstanceIdentifier UOA_category_identifier(new_uuid, new_uoa, std::string(), 0, flags.c_str(), TIME_GRANULARITY__NONE, MakeNotes(std::string(), std::string(), std::string()));
	identifiers.push_back(UOA_category_identifier);
	Sort();

	bool added_dmu_category_lookups = input_model.t_uoa_setmemberlookup.CreateNewUOA(db, input_model, new_uuid, dmu_categories);
	if (!added_dmu_category_lookups)
	{
		boost::format msg("Unable to create new UOA-DMU category entries in lookup table: %1%");
		msg % sqlite3_errstr(step_result);
		throw NewGeneException() << newgene_error_description(msg.str());
	}

	theExecutor.success();

	return theExecutor.succeeded();

}

std::string Table_UOA_Identifier::GetUoaCategoryDisplayText(WidgetInstanceIdentifier const & uoa_category, WidgetInstanceIdentifiers const & dmu_categories)
{

	if (!uoa_category.uuid || uoa_category.uuid->empty())
	{
		boost::format msg("Bad unit of analysis in model.");
		throw NewGeneException() << newgene_error_description(msg.str());
	}

	bool has_code = false;
	bool has_description = false;

	std::string displayText;

	if (uoa_category.code && !uoa_category.code->empty())
	{
		has_code = true;
	}

	if (uoa_category.longhand && !uoa_category.longhand->empty())
	{
		has_description = true;
	}

	if (has_code || has_description)
	{
		if (!has_code)
		{
			displayText += *uoa_category.longhand;
			//displayText += " (";
			//displayText += *uoa_category.uuid;
			//displayText += ")";
		}
		else if (!has_description)
		{
			displayText += *uoa_category.code;
			//displayText += " (";
			//displayText += *uoa_category.uuid;
			//displayText += ")";
		}
		else
		{
			displayText += *uoa_category.longhand;
			displayText += " (";
			displayText += *uoa_category.code;
			//displayText += " - ";
			//displayText += *uoa_category.uuid;
			displayText += ")";
		}
	}
	else
	{
		displayText += *uoa_category.uuid;
	}


	// Now the DMU categories

	displayText += " (";
	bool first = true;
	std::for_each(dmu_categories.cbegin(), dmu_categories.cend(), [&](WidgetInstanceIdentifier const & dmu_category)
	{
		if (!dmu_category.code || dmu_category.code->empty())
		{
			boost::format msg("Bad DMU category member of a UOA.");
			throw NewGeneException() << newgene_error_description(msg.str());
		}
		if (!first)
		{
			displayText += ", ";
		}
		first = false;
		displayText += *dmu_category.code;
	});
	displayText += ")";

	return displayText;

}

void Table_UOA_Member::Load(sqlite3 * db, InputModel * input_model_)
{

	std::lock_guard<std::recursive_mutex> data_lock(data_mutex);

	identifiers_map.clear();

	sqlite3_stmt * stmt = NULL;
	std::string sql("SELECT * FROM UOA_CATEGORY_LOOKUP");
	sqlite3_prepare_v2(db, sql.c_str(), static_cast<int>(sql.size()) + 1, &stmt, NULL);
	if (stmt == NULL)
	{
		return;
	}
	int step_result = 0;

	while ((step_result = sqlite3_step(stmt)) == SQLITE_ROW)
	{
		char const * uuid = reinterpret_cast<char const *>(sqlite3_column_text(stmt, INDEX__UOA_CATEGORY_LOOKUP_FK_UOA_CATEGORY_UUID));
		int const sequence_number = sqlite3_column_int(stmt, INDEX__UOA_CATEGORY_LOOKUP_SEQUENCE_NUMBER);
		char const * dmu_uuid = reinterpret_cast<char const *>(sqlite3_column_text(stmt, INDEX__UOA_CATEGORY_LOOKUP_FK_DMU_CATEGORY_UUID));
		if (uuid && /* strlen(uuid) == UUID_LENGTH && */ dmu_uuid /* && strlen(dmu_uuid) == UUID_LENGTH */)
		{
			WidgetInstanceIdentifier the_dmu_identifier = input_model_->t_dmu_category.getIdentifier(dmu_uuid);
			the_dmu_identifier.sequence_number_or_count = sequence_number;
			identifiers_map[uuid].push_back(the_dmu_identifier);

			// Todo: Add sort by sequence number
		}
	}

	if (stmt)
	{
		sqlite3_finalize(stmt);
		stmt = nullptr;
	}

}

WidgetInstanceIdentifiers Table_UOA_Member::RetrieveUOAsGivenDMU(sqlite3 * db, InputModel * input_model_, WidgetInstanceIdentifier const & dmu)
{

	if (!db)
	{
		return WidgetInstanceIdentifiers();
	}

	if (!input_model_)
	{
		return WidgetInstanceIdentifiers();
	}

	// No hash function available; can't use unordered_set
	//std::unordered_set<WidgetInstanceIdentifier> uoa_set;
	WidgetInstanceIdentifiers uoas;
	std::for_each(identifiers_map.cbegin(), identifiers_map.cend(), [&](std::pair<UUID const &, WidgetInstanceIdentifiers> const & mapping_from_uoa_to_dmus)
	{
		UUID const uuid(mapping_from_uoa_to_dmus.first);
		WidgetInstanceIdentifiers const & dmus(mapping_from_uoa_to_dmus.second);
		std::for_each(dmus.cbegin(), dmus.cend(), [&](WidgetInstanceIdentifier const & dmu_candidate)
		{
			if (dmu.IsEqual(WidgetInstanceIdentifier::EQUALITY_CHECK_TYPE__UUID_PLUS_STRING_CODE, dmu_candidate))
			{
				WidgetInstanceIdentifier uoa(input_model_->t_uoa_category.getIdentifier(uuid));
				if (std::find_if(uoas.cbegin(), uoas.cend(), std::bind(&WidgetInstanceIdentifier::IsEqual, std::placeholders::_1, WidgetInstanceIdentifier::EQUALITY_CHECK_TYPE__UUID, uoa)) == uoas.cend()) {
					uoas.push_back(uoa);
				}
			}
		});
	});

	return uoas;

}

bool Table_UOA_Member::DeleteUOA(sqlite3 * db, InputModel & input_model_, WidgetInstanceIdentifier const & uoa)
{

	if (!uoa.uuid || uoa.uuid->empty())
	{
		return false;
	}

	// Just delete from the cache
	identifiers_map.erase(*uoa.uuid);

	return true;

}

bool Table_UOA_Member::CreateNewUOA(sqlite3 * db, InputModel & input_model, std::string const & uoa_uuid, WidgetInstanceIdentifiers dmu_categories)
{

	std::lock_guard<std::recursive_mutex> data_lock(data_mutex);

	Executor theExecutor(db);

	int sequence_number = 0;
	std::for_each(dmu_categories.cbegin(), dmu_categories.cend(), [&](WidgetInstanceIdentifier const & dmu_category)
	{

		if (!dmu_category.uuid || dmu_category.uuid->empty())
		{
			boost::format msg("Bad DMU category member of a UOA.");
			throw NewGeneException() << newgene_error_description(msg.str());
		}

		++sequence_number;

		sqlite3_stmt * stmt = NULL;
		std::string sql("INSERT INTO UOA_CATEGORY_LOOKUP (UOA_CATEGORY_LOOKUP_FK_UOA_CATEGORY_UUID, UOA_CATEGORY_LOOKUP_SEQUENCE_NUMBER, UOA_CATEGORY_LOOKUP_FK_DMU_CATEGORY_UUID) VALUES ('");
		sql += uoa_uuid;
		sql += "', ";
		sql += boost::lexical_cast<std::string>(sequence_number);
		sql += ", '";
		sql += *dmu_category.uuid;
		sql += "')";
		sqlite3_prepare_v2(db, sql.c_str(), static_cast<int>(sql.size()) + 1, &stmt, NULL);
		if (stmt == NULL)
		{
			boost::format msg("Unable to prepare INSERT statement to create a new UOA-DMU category mapping.");
			throw NewGeneException() << newgene_error_description(msg.str());
		}
		int step_result = 0;
		step_result = sqlite3_step(stmt);
		if (step_result != SQLITE_DONE)
		{
			boost::format msg("Unable to execute INSERT statement to create a new UOA-DMU category mapping: %1%");
			msg % sqlite3_errstr(step_result);
			throw NewGeneException() << newgene_error_description(msg.str());
		}
		if (stmt)
		{
			sqlite3_finalize(stmt);
			stmt = nullptr;
		}
	});

	theExecutor.success();

	return theExecutor.succeeded();

}
