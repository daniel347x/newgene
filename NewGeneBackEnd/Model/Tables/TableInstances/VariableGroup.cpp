#include "VariableGroup.h"
#include "../../../sqlite/sqlite-amalgamation-3071700/sqlite3.h"

#ifndef Q_MOC_RUN
#	include <boost/algorithm/string.hpp>
#endif
#include "../../../Utilities/UUID.h"
#include "../../InputModel.h"

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
std::string const Table_VG_SET_MEMBER::VG_SET_MEMBER_SEQUENCE_NUMBER = "VG_SET_MEMBER_SEQUENCE_NUMBER";
std::string const Table_VG_SET_MEMBER::VG_SET_MEMBER_NOTES1 = "VG_SET_MEMBER_NOTES1";
std::string const Table_VG_SET_MEMBER::VG_SET_MEMBER_NOTES2 = "VG_SET_MEMBER_NOTES2";
std::string const Table_VG_SET_MEMBER::VG_SET_MEMBER_NOTES3 = "VG_SET_MEMBER_NOTES3";
std::string const Table_VG_SET_MEMBER::VG_SET_MEMBER_FK_VG_CATEGORY_UUID = "VG_SET_MEMBER_FK_VG_CATEGORY_UUID";
std::string const Table_VG_SET_MEMBER::VG_SET_MEMBER_FLAGS = "VG_SET_MEMBER_FLAGS";

void Table_VG_CATEGORY::Load(sqlite3 * db, InputModel * input_model_)
{

	std::lock_guard<std::recursive_mutex> data_lock(data_mutex);

	identifiers.clear();

	sqlite3_stmt * stmt = NULL;
	std::string sql("SELECT * FROM VG_CATEGORY");
	sqlite3_prepare_v2(db, sql.c_str(), static_cast<int>(sql.size()) + 1, &stmt, NULL);

	if (stmt == NULL)
	{
		return;
	}

	int step_result = 0;

	while ((step_result = sqlite3_step(stmt)) == SQLITE_ROW)
	{
		char const * uuid = reinterpret_cast<char const *>(sqlite3_column_text(stmt, INDEX__VG_CATEGORY_UUID));
		char const * code = reinterpret_cast<char const *>(sqlite3_column_text(stmt, INDEX__VG_CATEGORY_STRING_CODE));
		char const * longhand = reinterpret_cast<char const *>(sqlite3_column_text(stmt, INDEX__VG_CATEGORY_STRING_LONGHAND));
		char const * notes1 = reinterpret_cast<char const *>(sqlite3_column_text(stmt, INDEX__VG_CATEGORY_NOTES1));
		char const * notes2 = reinterpret_cast<char const *>(sqlite3_column_text(stmt, INDEX__VG_CATEGORY_NOTES2));
		char const * notes3 = reinterpret_cast<char const *>(sqlite3_column_text(stmt, INDEX__VG_CATEGORY_NOTES3));
		char const * fk_uoa_uuid = reinterpret_cast<char const *>(sqlite3_column_text(stmt, INDEX__VG_CATEGORY_FK_UOA_CATEGORY_UUID));
		char const * flags = reinterpret_cast<char const *>(sqlite3_column_text(stmt, INDEX__VG_CATEGORY_FLAGS));

		if (uuid && /* strlen(uuid) == UUID_LENGTH && */ code && strlen(code) && longhand && strlen(longhand) && fk_uoa_uuid /* && strlen(fk_uoa_uuid) == UUID_LENGTH */)
		{
			WidgetInstanceIdentifier uoa_identifier = input_model_->t_uoa_category.getIdentifier(fk_uoa_uuid);
			WidgetInstanceIdentifier vg_category_identifier(uuid, uoa_identifier, code, longhand, 0, flags, uoa_identifier.time_granularity, MakeNotes(notes1, notes2, notes3));
			identifiers.push_back(vg_category_identifier);
		}
	}

	if (stmt)
	{
		sqlite3_finalize(stmt);
		stmt = nullptr;
	}

}

bool Table_VG_CATEGORY::DeleteVG(sqlite3 * db, InputModel * input_model_, WidgetInstanceIdentifier const & vg, DataChangeMessage & change_message)
{

	Executor theExecutor(db);

	if (!db)
	{
		return false;
	}

	if (!input_model_)
	{
		return false;
	}

	if (!vg.code || vg.code->empty() || !vg.uuid || vg.uuid->empty())
	{
		return false;
	}

	bool already_exists = ExistsByCode(db, *input_model_, *vg.code);

	if (!already_exists)
	{
		boost::format msg("The variable group %1% does not exist.");
		msg % *vg.code;
		throw NewGeneException() << newgene_error_description(msg.str());
	}

	already_exists = ExistsByUuid(db, *input_model_, *vg.uuid);

	if (!already_exists)
	{
		boost::format msg("The variable group %1% does not exist.");
		msg % *vg.code;
		throw NewGeneException() << newgene_error_description(msg.str());
	}

	already_exists = Exists(db, *input_model_, vg);

	if (!already_exists)
	{
		boost::format msg("The variable group %1% does not exist.");
		msg % *vg.code;
		throw NewGeneException() << newgene_error_description(msg.str());
	}

	std::for_each(input_model_->t_vgp_data_vector.begin(), input_model_->t_vgp_data_vector.end(), [&](std::unique_ptr<Table_VariableGroupData> & vg_instance_table)
	{
		if (vg_instance_table)
		{
			if (boost::iequals(*vg.code, vg_instance_table->vg_category_string_code))
			{
				vg_instance_table->DeleteDataTable(db, input_model_);
			}
		}
	});

	input_model_->t_vgp_data_vector.erase
	(
		std::remove_if(
			input_model_->t_vgp_data_vector.begin(),
			input_model_->t_vgp_data_vector.end(),
			std::bind(
				[&](std::unique_ptr<Table_VariableGroupData> & vg_instance_table, WidgetInstanceIdentifier const & vg_to_delete_)
	{
		bool vg_matches = false;

		if (vg_instance_table && boost::iequals(vg_instance_table->vg_category_string_code, *vg_to_delete_.code))
		{
			vg_matches = true;
		}

		return vg_matches;
	},
	std::placeholders::_1,
	vg
			)
		),
	input_model_->t_vgp_data_vector.end()
	);

	sqlite3_stmt * stmt = NULL;
	std::string sql("DELETE FROM VG_CATEGORY WHERE VG_CATEGORY_UUID = '");
	sql += *vg.uuid;
	sql += "'";
	sqlite3_prepare_v2(db, sql.c_str(), static_cast<int>(sql.size()) + 1, &stmt, NULL);

	if (stmt == NULL)
	{
		boost::format msg("Unable to prepare DELETE statement to delete the VG.");
		throw NewGeneException() << newgene_error_description(msg.str());
	}

	int step_result = 0;
	step_result = sqlite3_step(stmt);

	if (step_result != SQLITE_DONE)
	{
		boost::format msg("Unable to execute DELETE statement to delete the VG: %1%");
		msg % sqlite3_errstr(step_result);
		throw NewGeneException() << newgene_error_description(msg.str());
	}

	if (stmt)
	{
		sqlite3_finalize(stmt);
		stmt = nullptr;
	}

	// Remove from cache for table VG_SET_MEMBER
	input_model_->t_vgp_setmembers.DeleteVG(db, input_model_, vg);

	// Remove from our cache
	identifiers.erase(std::remove_if(identifiers.begin(), identifiers.end(), std::bind(&WidgetInstanceIdentifier::IsEqual, std::placeholders::_1,
									 WidgetInstanceIdentifier::EQUALITY_CHECK_TYPE__UUID, vg)), identifiers.end());

	// ***************************************** //
	// Prepare data to send back to user interface
	// ***************************************** //
	DATA_CHANGE_TYPE type = DATA_CHANGE_TYPE__INPUT_MODEL__VG_CHANGE;
	DATA_CHANGE_INTENTION intention = DATA_CHANGE_INTENTION__REMOVE;
	DataChange change(type, intention, vg, WidgetInstanceIdentifiers());
	change_message.changes.push_back(change);

	theExecutor.success();

	return theExecutor.succeeded();

}

WidgetInstanceIdentifiers Table_VG_CATEGORY::RetrieveVGsFromUOA(sqlite3 * db, InputModel * input_model_, UUID const & uuid)
{

	if (!db)
	{
		return WidgetInstanceIdentifiers();
	}

	if (!input_model_)
	{
		return WidgetInstanceIdentifiers();
	}

	WidgetInstanceIdentifier uoa = input_model_->t_uoa_category.getIdentifier(uuid);

	if (uoa.IsEmpty())
	{
		return WidgetInstanceIdentifiers();
	}

	WidgetInstanceIdentifiers vgs;

	sqlite3_stmt * stmt = NULL;
	std::string sql("SELECT * FROM VG_CATEGORY WHERE VG_CATEGORY_FK_UOA_CATEGORY_UUID = '");
	sql += uuid;
	sql += "'";
	sqlite3_prepare_v2(db, sql.c_str(), static_cast<int>(sql.size()) + 1, &stmt, NULL);

	if (stmt == NULL)
	{
		boost::format msg("Unable to retrieve variable groups associated with unit of analysis.");
		throw NewGeneException() << newgene_error_description(msg.str());
	}

	int step_result = 0;

	while ((step_result = sqlite3_step(stmt)) == SQLITE_ROW)
	{
		char const * uuid_vg = reinterpret_cast<char const *>(sqlite3_column_text(stmt, INDEX__VG_CATEGORY_UUID));
		WidgetInstanceIdentifier vg = getIdentifier(uuid_vg);
		vgs.push_back(vg);
	}

	if (stmt)
	{
		sqlite3_finalize(stmt);
		stmt = nullptr;
	}

	return vgs;

}

bool Table_VG_CATEGORY::CreateNewVG(sqlite3 * db, InputModel & input_model, std::string const & vg_code_, std::string const & vg_description,
									WidgetInstanceIdentifier const & uoa_to_use)
{

	std::lock_guard<std::recursive_mutex> data_lock(data_mutex);

	Executor theExecutor(db);

	if (!uoa_to_use.uuid)
	{
		boost::format msg("Bad UOA in attempted creation of VG");
		throw NewGeneException() << newgene_error_description(msg.str());
	}

	std::string vg_code = boost::trim_copy(vg_code_);
	boost::to_upper(vg_code);

	bool already_exists = ExistsByCode(db, input_model, vg_code);

	if (already_exists)
	{
		return false;
	}

	std::string new_uuid(boost::to_upper_copy(newUUID(false)));
	sqlite3_stmt * stmt = NULL;
	std::string
	sql("INSERT INTO VG_CATEGORY (VG_CATEGORY_UUID, VG_CATEGORY_STRING_CODE, VG_CATEGORY_STRING_LONGHAND, VG_CATEGORY_NOTES1, VG_CATEGORY_NOTES2, VG_CATEGORY_NOTES3, VG_CATEGORY_FK_UOA_CATEGORY_UUID, VG_CATEGORY_FLAGS) VALUES ('");
	sql += new_uuid;
	sql += "', ?, ?, '', '', '', ?, '')";
	sqlite3_prepare_v2(db, sql.c_str(), static_cast<int>(sql.size()) + 1, &stmt, NULL);

	if (stmt == NULL)
	{
		boost::format msg("Unable to prepare INSERT statement to create a new variable group.");
		throw NewGeneException() << newgene_error_description(msg.str());
	}

	sqlite3_bind_text(stmt, 1, vg_code.c_str(), -1, SQLITE_TRANSIENT);
	sqlite3_bind_text(stmt, 2, vg_description.c_str(), -1, SQLITE_TRANSIENT);
	sqlite3_bind_text(stmt, 3, uoa_to_use.uuid->c_str(), -1, SQLITE_TRANSIENT);
	int step_result = 0;
	step_result = sqlite3_step(stmt);

	if (step_result != SQLITE_DONE)
	{
		boost::format msg("Unable to execute INSERT statement to create a new VG: %1%");
		msg % sqlite3_errstr(step_result);
		throw NewGeneException() << newgene_error_description(msg.str());
	}

	if (stmt)
	{
		sqlite3_finalize(stmt);
		stmt = nullptr;
	}

	std::string flags;
	WidgetInstanceIdentifier vg_category_identifier(new_uuid, vg_code, vg_description, 0, flags.c_str(), TIME_GRANULARITY__NONE, MakeNotes(std::string(), std::string(),
			std::string()));
	vg_category_identifier.identifier_parent = std::make_shared<WidgetInstanceIdentifier>(uoa_to_use);
	identifiers.push_back(vg_category_identifier);

	// Do not create the table here.  Wait for them to import data, and create the table then.

	theExecutor.success();

	return theExecutor.succeeded();

}

bool Table_VG_CATEGORY::Exists(sqlite3 * db, InputModel & input_model_, WidgetInstanceIdentifier const & vg, bool const also_confirm_using_cache)
{

	std::lock_guard<std::recursive_mutex> data_lock(data_mutex);

	if (!vg.uuid || !vg.code)
	{
		return false;
	}

	sqlite3_stmt * stmt = NULL;
	std::string sql("SELECT COUNT(*) FROM VG_CATEGORY WHERE VG_CATEGORY_UUID = '");
	sql += *vg.uuid;
	sql += "'";
	sqlite3_prepare_v2(db, sql.c_str(), static_cast<int>(sql.size()) + 1, &stmt, NULL);

	if (stmt == NULL)
	{
		boost::format msg("Unable to prepare SELECT statement to search for an existing VG.");
		throw NewGeneException() << newgene_error_description(msg.str());
	}

	int step_result = 0;
	bool exists = false;

	while ((step_result = sqlite3_step(stmt)) == SQLITE_ROW)
	{
		int existing_vg_count = sqlite3_column_int(stmt, 0);

		if (existing_vg_count == 1)
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
		auto found = std::find_if(identifiers.cbegin(), identifiers.cend(), std::bind(&WidgetInstanceIdentifier::IsEqual, std::placeholders::_1,
								  WidgetInstanceIdentifier::EQUALITY_CHECK_TYPE__UUID, vg));
		bool exists_in_cache = (found != identifiers.cend());

		if (exists != exists_in_cache)
		{
			boost::format msg("Cache of VGs is out-of-sync.");
			throw NewGeneException() << newgene_error_description(msg.str());
		}
	}

	return exists;

}

bool Table_VG_CATEGORY::ExistsByUuid(sqlite3 * db, InputModel & input_model_, std::string const & vg_uuid, bool const also_confirm_using_cache)
{

	std::lock_guard<std::recursive_mutex> data_lock(data_mutex);

	sqlite3_stmt * stmt = NULL;
	std::string sql("SELECT COUNT(*) FROM VG_CATEGORY WHERE VG_CATEGORY_UUID = '");
	sql += vg_uuid;
	sql += "'";
	sqlite3_prepare_v2(db, sql.c_str(), static_cast<int>(sql.size()) + 1, &stmt, NULL);

	if (stmt == NULL)
	{
		boost::format msg("Unable to prepare SELECT statement to search for an existing VG.");
		throw NewGeneException() << newgene_error_description(msg.str());
	}

	int step_result = 0;
	bool exists = false;

	while ((step_result = sqlite3_step(stmt)) == SQLITE_ROW)
	{
		int existing_vg_count = sqlite3_column_int(stmt, 0);

		if (existing_vg_count == 1)
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
		WidgetInstanceIdentifier vg(vg_uuid, "", "", 0);
		auto found = std::find_if(identifiers.cbegin(), identifiers.cend(), std::bind(&WidgetInstanceIdentifier::IsEqual, std::placeholders::_1,
								  WidgetInstanceIdentifier::EQUALITY_CHECK_TYPE__UUID, vg));
		bool exists_in_cache = (found != identifiers.cend());

		if (exists != exists_in_cache)
		{
			boost::format msg("Cache of VGs is out-of-sync.");
			throw NewGeneException() << newgene_error_description(msg.str());
		}
	}

	return exists;

}

bool Table_VG_CATEGORY::ExistsByCode(sqlite3 * db, InputModel & input_model_, std::string const & vg_code_, bool const also_confirm_using_cache)
{

	std::lock_guard<std::recursive_mutex> data_lock(data_mutex);

	std::string vg_code(vg_code_);
	boost::trim(vg_code);
	boost::to_upper(vg_code);

	if (vg_code.empty())
	{
		return false;
	}

	sqlite3_stmt * stmt = NULL;
	std::string sql("SELECT COUNT(*) FROM VG_CATEGORY WHERE VG_CATEGORY_STRING_CODE = '");
	sql += vg_code;
	sql += "'";
	sqlite3_prepare_v2(db, sql.c_str(), static_cast<int>(sql.size()) + 1, &stmt, NULL);

	if (stmt == NULL)
	{
		boost::format msg("Unable to prepare SELECT statement to search for an existing VG.");
		throw NewGeneException() << newgene_error_description(msg.str());
	}

	int step_result = 0;
	bool exists = false;

	while ((step_result = sqlite3_step(stmt)) == SQLITE_ROW)
	{
		int existing_vg_count = sqlite3_column_int(stmt, 0);

		if (existing_vg_count == 1)
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
		WidgetInstanceIdentifier vg(vg_code);
		auto found = std::find_if(identifiers.cbegin(), identifiers.cend(), std::bind(&WidgetInstanceIdentifier::IsEqual, std::placeholders::_1,
								  WidgetInstanceIdentifier::EQUALITY_CHECK_TYPE__STRING_CODE, vg));
		bool exists_in_cache = (found != identifiers.cend());

		if (exists != exists_in_cache)
		{
			boost::format msg("Cache of VGs is out-of-sync.");
			throw NewGeneException() << newgene_error_description(msg.str());
		}
	}

	return exists;

}

std::string Table_VG_CATEGORY::GetVgDisplayText(WidgetInstanceIdentifier const & vg)
{

	if (!vg.uuid || vg.uuid->empty() || !vg.code || vg.code->empty() || !vg.identifier_parent)
	{
		boost::format msg("Bad VG in GetVgDisplayText().");
		throw NewGeneException() << newgene_error_description(msg.str());
	}

	bool has_description = false;

	if (vg.longhand && !vg.longhand->empty())
	{
		has_description = true;
	}

	std::string displayText;

	if (has_description)
	{
		displayText += *vg.longhand;
		displayText += " (";
		displayText += *vg.code;
		displayText += ")";
	}
	else
	{
		displayText += *vg.code;
	}

	WidgetInstanceIdentifiers dmu_categories;

	if ((*vg.identifier_parent).foreign_key_identifiers)
	{
		dmu_categories = *(*vg.identifier_parent).foreign_key_identifiers;
	}

	displayText += " (Corresponds to UOA: ";
	displayText += Table_UOA_Identifier::GetUoaCategoryDisplayText(*vg.identifier_parent, dmu_categories);
	displayText += ")";

	return displayText;

}

void Table_VG_SET_MEMBER::Load(sqlite3 * db, InputModel * input_model_)
{

	std::lock_guard<std::recursive_mutex> data_lock(data_mutex);

	identifiers_map.clear();

	sqlite3_stmt * stmt = NULL;
	std::string sql("SELECT * FROM VG_SET_MEMBER");
	sqlite3_prepare_v2(db, sql.c_str(), static_cast<int>(sql.size()) + 1, &stmt, NULL);

	if (stmt == NULL)
	{
		return;
	}

	int step_result = 0;

	while ((step_result = sqlite3_step(stmt)) == SQLITE_ROW)
	{
		char const * uuid = reinterpret_cast<char const *>(sqlite3_column_text(stmt, INDEX__VG_SET_MEMBER_UUID));
		char const * code = reinterpret_cast<char const *>(sqlite3_column_text(stmt, INDEX__VG_SET_MEMBER_STRING_CODE));
		char const * longhand = reinterpret_cast<char const *>(sqlite3_column_text(stmt, INDEX__VG_SET_MEMBER_STRING_LONGHAND));
		int const seqnumber = sqlite3_column_int(stmt, INDEX__VG_SET_MEMBER_SEQUENCE_NUMBER);
		char const * notes1 = reinterpret_cast<char const *>(sqlite3_column_text(stmt, INDEX__VG_SET_MEMBER_NOTES1));
		char const * notes2 = reinterpret_cast<char const *>(sqlite3_column_text(stmt, INDEX__VG_SET_MEMBER_NOTES2));
		char const * notes3 = reinterpret_cast<char const *>(sqlite3_column_text(stmt, INDEX__VG_SET_MEMBER_NOTES3));
		char const * fk_vg_uuid = reinterpret_cast<char const *>(sqlite3_column_text(stmt, INDEX__VG_SET_MEMBER_FK_VG_CATEGORY_UUID));
		char const * flags = reinterpret_cast<char const *>(sqlite3_column_text(stmt, INDEX__VG_SET_MEMBER_FLAGS));

		if (uuid && /* strlen(uuid) == UUID_LENGTH && */ code && strlen(code) && longhand && strlen(longhand) && fk_vg_uuid /* && strlen(fk_vg_uuid) == UUID_LENGTH */)
		{
			WidgetInstanceIdentifier vg_category_identifier = input_model_->t_vgp_identifiers.getIdentifier(fk_vg_uuid);
			identifiers_map[fk_vg_uuid].push_back(WidgetInstanceIdentifier(uuid, vg_category_identifier, code, longhand, seqnumber, flags, vg_category_identifier.time_granularity,
												  MakeNotes(notes1, notes2, notes3)));
		}
	}

	if (stmt)
	{
		sqlite3_finalize(stmt);
		stmt = nullptr;
	}
}

bool Table_VG_SET_MEMBER::AddNewVGTableEntries(sqlite3 * db, InputModel * input_model_, WidgetInstanceIdentifier const & variable_group, ImportDefinition const & import_definition, std::string & errorMsg)
{

	int sequence_number = 0;
	std::for_each(import_definition.output_schema.schema.cbegin(),
				  import_definition.output_schema.schema.cend(), [&](SchemaEntry const & table_schema_entry)
	{

		std::string sql;
		std::string new_uuid(boost::to_upper_copy(newUUID(false)));
		sql += "INSERT INTO VG_SET_MEMBER (VG_SET_MEMBER_UUID, VG_SET_MEMBER_STRING_CODE, VG_SET_MEMBER_STRING_LONGHAND, VG_SET_MEMBER_SEQUENCE_NUMBER, VG_SET_MEMBER_NOTES1, VG_SET_MEMBER_NOTES2, VG_SET_MEMBER_NOTES3, VG_SET_MEMBER_FK_VG_CATEGORY_UUID, VG_SET_MEMBER_FLAGS, VG_SET_MEMBER_DATA_TYPE)";
		sql += " VALUES ('";
		sql += new_uuid;
		sql += "', '";
		sql += table_schema_entry.field_name;
		sql += "', '";
		sql += table_schema_entry.field_description;
		sql += "', ";
		sql += boost::lexical_cast<std::string>(sequence_number);
		sql += ", '', '', '', '";
		sql += *variable_group.uuid;
		sql += "', '', '";

		switch (table_schema_entry.field_type)
		{
			case FIELD_TYPE_INT32:
			case FIELD_TYPE_UINT32:
				{
					sql += "INT32";
				}
				break;

			case FIELD_TYPE_INT64:
			case FIELD_TYPE_UINT64:
				{
					sql += "INT64";
				}
				break;

			case FIELD_TYPE_STRING_FIXED:
			case FIELD_TYPE_STRING_VAR:
			case FIELD_TYPE_UUID:
			case FIELD_TYPE_UUID_FOREIGN:
			case FIELD_TYPE_STRING_CODE:
			case FIELD_TYPE_STRING_LONGHAND:
			case FIELD_TYPE_TIME_RANGE:
			case FIELD_TYPE_NOTES_1:
			case FIELD_TYPE_NOTES_2:
			case FIELD_TYPE_NOTES_3:
				{
					sql += "STRING";
				}
				break;

			case FIELD_TYPE_TIMESTAMP:
				{
					sql += "INT64";
				}
				break;

			case FIELD_TYPE_FLOAT:
				{
					sql += "FLOAT";
				}
				break;

			default:
				{
					sql += "STRING";
				}
				break;
		}

		sql += "')";
		sqlite3_stmt * stmt = NULL;
		int err = sqlite3_prepare_v2(db, sql.c_str(), static_cast<int>(sql.size()) + 1, &stmt, NULL);

		if (stmt == NULL)
		{
			boost::format msg("Cannot create SQL \"%1%\" in VG import: %2%");
			msg % sql % sqlite3_errstr(err);
			throw NewGeneException() << newgene_error_description(msg.str());
		}

		int step_result = 0;

		if ((step_result = sqlite3_step(stmt)) != SQLITE_DONE)
		{
			boost::format msg("Cannot execute SQL \"%1%\" in VG import: %2%");
			msg % sql % sqlite3_errstr(step_result);
			throw NewGeneException() << newgene_error_description(msg.str());
		}

		if (stmt)
		{
			sqlite3_finalize(stmt);
			stmt = nullptr;
		}

		++sequence_number;
	});

	return true;

}

bool Table_VG_SET_MEMBER::DeleteVG(sqlite3 * db, InputModel * input_model_, WidgetInstanceIdentifier const & vg)
{

	if (!vg.uuid || vg.uuid->empty())
	{
		return false;
	}

	// Just delete from the cache (cascading delete handles this in the DB)
	identifiers_map.erase(*vg.uuid);

	return true;

}
