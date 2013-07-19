#include "VariablesSelected.h"
#include "../../InputModel.h"
#include "../../OutputModel.h"
#include "../../../sqlite/sqlite-amalgamation-3071700/sqlite3.h"

std::string const Table_VARIABLES_SELECTED::VG_SET_MEMBER_STRING_CODE = "VG_SET_MEMBER_STRING_CODE";
std::string const Table_VARIABLES_SELECTED::VG_CATEGORY_STRING_CODE = "VG_CATEGORY_STRING_CODE";

void Table_VARIABLES_SELECTED::Load(sqlite3 * db, OutputModel * output_model_, InputModel * input_model_)
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

		// ****************************************************************************************//
		// Use codes as foreign keys, not UUID's, so that this output model can be shared with others
		// ****************************************************************************************//
		char const * code_variable = reinterpret_cast<char const *>(sqlite3_column_text(stmt, INDEX__VG_SET_MEMBER_STRING_CODE));
		char const * code_variable_group = reinterpret_cast<char const *>(sqlite3_column_text(stmt, INDEX__VG_CATEGORY_STRING_CODE));
		if (code_variable && strlen(code_variable) && code_variable_group && strlen(code_variable_group))
		{
			WidgetInstanceIdentifier identifier_parent;
			bool found_parent = input_model_->t_vgp_identifiers.getIdentifierFromStringCode(code_variable_group, identifier_parent);
			if (found_parent && identifier_parent.uuid && identifier_parent.uuid->size() > 0)
			{
				WidgetInstanceIdentifier identifier;
				bool found = input_model_->t_vgp_setmembers.getIdentifierFromStringCodeAndParentUUID(code_variable, *identifier_parent.uuid, identifier);
				if (found && identifier.uuid && identifier.uuid->size())
				{
					identifiers_map[*identifier_parent.uuid].push_back(identifier);
				}
			}
		}

	}
}

bool Table_VARIABLES_SELECTED::Update(sqlite3 * db, OutputModel & output_model_, InputModel & input_model_, DataChangeMessage & change_message)
{

	std::lock_guard<std::recursive_mutex> data_lock(data_mutex);

	Executor theExecutor(db);
	
	std::for_each(change_message.changes.cbegin(), change_message.changes.cend(), [&db, &input_model_, this](DataChange const & change)
	{
		switch (change.change_type)
		{
		case DATA_CHANGE_TYPE::DATA_CHANGE_TYPE__OUTPUT_MODEL__VG_CATEGORY_SET_MEMBER_SELECTION:
			{
				switch (change.change_intention)
				{
				case DATA_CHANGE_INTENTION__ADD:
				case DATA_CHANGE_INTENTION__REMOVE:
					{
						// This is the OUTPUT model changing.
						// "Add" means to simply add an item that is CHECKED (previously unchecked) -
						// NOT to add a new variable.  That would be an input model change type.

						if (change.child_identifiers.size() == 0)
						{
							return; // from lambda
						}

						if (!change.parent_identifier.uuid || change.parent_identifier.uuid->size() == 0 || !change.parent_identifier.code || change.parent_identifier.code->size() == 0)
						{
							return; // from lambda
						}

						std::for_each(change.child_identifiers.cbegin(), change.child_identifiers.cend(), [&db, &input_model_, &change, this](WidgetInstanceIdentifier const & child_identifier)
						{

							if (!child_identifier.uuid || child_identifier.uuid->size() == 0 || !child_identifier.code || child_identifier.code->size() == 0)
							{
								return; // from lambda
							}

							// *************** //
							// VG_CATEGORY
							// *************** //

							std::map<UUID, WidgetInstanceIdentifiers>::const_iterator found_set = this->identifiers_map.find(*change.parent_identifier.uuid);
							if (found_set == this->identifiers_map.cend())
							{
								// no selections in this variable group category
								if (change.change_intention == DATA_CHANGE_INTENTION__ADD)
								{
									// add selection
									this->identifiers_map[*change.parent_identifier.uuid].push_back(child_identifier);
									Add(db, *child_identifier.code, *change.parent_identifier.code);
								}
								return; // from lambda
							}
							else
							{
								// selections exist in this variable group category
								WidgetInstanceIdentifiers current_identifiers = this->identifiers_map[*change.parent_identifier.uuid];
								int number_variables = current_identifiers.size();
								bool found = false;
								for (int n=0; n < number_variables; ++n)
								{

									// *************** //
									// VG_SET_MEMBER
									// *************** //

									WidgetInstanceIdentifier & current_test_identifier = current_identifiers[n];
									if (child_identifier.IsEqual(WidgetInstanceIdentifier::EQUALITY_CHECK_TYPE__UUID, current_test_identifier))
									{

										// The selection currently exists
										if (change.change_intention == DATA_CHANGE_INTENTION__REMOVE)
										{
											// remove selection
											current_identifiers.erase(current_identifiers.begin() + n, current_identifiers.begin() + n + 1);
											--n;
											--number_variables;
											found = true;
											Remove(db, *child_identifier.code, *change.parent_identifier.code);
											continue;
										}
									}
								}

								if (!found)
								{
									// Item not found - add it
									if (change.change_intention == DATA_CHANGE_INTENTION__ADD)
									{
										this->identifiers_map[*change.parent_identifier.uuid].push_back(child_identifier);
										Add(db, *child_identifier.code, *change.parent_identifier.code);
									}
								}

							}

						});

					}
					break;
				case DATA_CHANGE_INTENTION__UPDATE:
					{
						// Should never receive this.
					}
				case DATA_CHANGE_INTENTION__RESET_ALL:
					{
						// Ditto above.
					}
					break;
				}
			}
			break;
		}
	});

	theExecutor.success();

	return theExecutor.succeeded();

}

void Table_VARIABLES_SELECTED::Add(sqlite3 * db, std::string const & vg_set_member_code, std::string const & vg_category_code)
{
	std::string sqlAdd("INSERT INTO VG_SET_MEMBERS_SELECTED (");
	sqlAdd += VG_SET_MEMBER_STRING_CODE;
	sqlAdd += ",";
	sqlAdd += VG_CATEGORY_STRING_CODE;
	sqlAdd += ") VALUES ('";
	sqlAdd += vg_set_member_code;
	sqlAdd += "','";
	sqlAdd += vg_category_code;
	sqlAdd += "')";
	sqlite3_stmt * stmt = NULL;
	sqlite3_prepare_v2(db, sqlAdd.c_str(), sqlAdd.size() + 1, &stmt, NULL);
	if (stmt == NULL)
	{
		return;
	}
	sqlite3_step(stmt);
}

void Table_VARIABLES_SELECTED::Remove(sqlite3 * db, std::string const & vg_set_member_code, std::string const & vg_category_code)
{
	std::string sqlRemove("DELETE FROM VG_SET_MEMBERS_SELECTED WHERE ");
	sqlRemove += VG_SET_MEMBER_STRING_CODE;
	sqlRemove += "='";
	sqlRemove += vg_set_member_code;
	sqlRemove += "' AND ";
	sqlRemove += VG_CATEGORY_STRING_CODE;
	sqlRemove += "='";
	sqlRemove += vg_category_code;
	sqlRemove += "'";
	sqlite3_stmt * stmt = NULL;
	sqlite3_prepare_v2(db, sqlRemove.c_str(), sqlRemove.size() + 1, &stmt, NULL);
	if (stmt == NULL)
	{
		return;
	}
	sqlite3_step(stmt);
}
