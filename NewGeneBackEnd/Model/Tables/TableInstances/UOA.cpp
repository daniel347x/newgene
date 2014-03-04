#include "UOA.h"
#include "../../../sqlite/sqlite-amalgamation-3071700/sqlite3.h"

#ifndef Q_MOC_RUN
#	include <boost/algorithm/string.hpp>
#endif
#include "../../InputModel.h"

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
