#include "VariableGroup.h"
#include "../../../sqlite/sqlite-amalgamation-3071700/sqlite3.h"

#ifndef Q_MOC_RUN
#	include <boost/algorithm/string.hpp>
#endif
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
		if (uuid && /* strlen(uuid) == UUID_LENGTH && */ code && strlen(code) && longhand && strlen(longhand) && fk_uoa_uuid /* && strlen(fk_uoa_uuid) == UUID_LENGTH */ )
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

	if (!vg.code)
	{
		return false;
	}

	std::for_each(input_model_->t_vgp_data_vector.begin(), input_model_->t_vgp_data_vector.end(), [&](std::unique_ptr<Table_VariableGroupData> & vg_instance_table)
	{
		if (vg_instance_table)
		{
			if (boost::iequals(*vg.code, vg_instance_table->vg_category_string_code))
			{
				vg_instance_table->DeleteDataTable(db, input_model_, change_message);
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

std::string Table_VG_CATEGORY::GetVgDisplayText(WidgetInstanceIdentifier const & vg)
{
	
	if (!vg.uuid || vg.uuid->empty() || !vg.code || vg.code->empty())
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
		if (uuid && /* strlen(uuid) == UUID_LENGTH && */ code && strlen(code) && longhand && strlen(longhand) && fk_vg_uuid /* && strlen(fk_vg_uuid) == UUID_LENGTH */ )
		{
			WidgetInstanceIdentifier vg_category_identifier = input_model_->t_vgp_identifiers.getIdentifier(fk_vg_uuid);
			identifiers_map[fk_vg_uuid].push_back(WidgetInstanceIdentifier(uuid, vg_category_identifier, code, longhand, seqnumber, flags, vg_category_identifier.time_granularity, MakeNotes(notes1, notes2, notes3)));
		}
	}

	if (stmt)
	{
		sqlite3_finalize(stmt);
		stmt = nullptr;
	}
}
