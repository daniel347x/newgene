#include "KAdCount.h"
#include "../../InputModel.h"
#include "../../OutputModel.h"
#include "../../../sqlite/sqlite-amalgamation-3071700/sqlite3.h"

std::string const Table_KAD_COUNT::KAD_COUNT__DMU_CATEGORY_STRING_CODE = "DMU_CATEGORY_STRING_CODE";
std::string const Table_KAD_COUNT::KAD_COUNT__COUNT = "COUNT";
std::string const Table_KAD_COUNT::KAD_COUNT__FLAGS = "FLAGS";

void Table_KAD_COUNT::Load(sqlite3 * db, OutputModel * output_model_, InputModel * input_model_)
{

	if (!output_model_ || !input_model_)
	{
		return;
	}

	std::lock_guard<std::recursive_mutex> data_lock(data_mutex);

	identifiers.clear();

	sqlite3_stmt * stmt = NULL;
	std::string sql("SELECT * FROM KAD_COUNT");
	sqlite3_prepare_v2(db, sql.c_str(), static_cast<int>(sql.size()) + 1, &stmt, NULL);

	if (stmt == NULL)
	{
		return;
	}

	int step_result = 0;

	while ((step_result = sqlite3_step(stmt)) == SQLITE_ROW)
	{

		// ****************************************************************************************//
		// Use codes as foreign keys, not NewGeneUUID's, so that this output model can be shared with others
		// ****************************************************************************************//
		char const * code_dmu_category = reinterpret_cast<char const *>(sqlite3_column_text(stmt, INDEX__KAD_COUNT__DMU_CATEGORY_STRING_CODE));
		int const kad_count = sqlite3_column_int(stmt, INDEX__KAD_COUNT__COUNT);

		//char const * flags = reinterpret_cast<char const *>(sqlite3_column_text(stmt, INDEX__KAD_COUNT__FLAGS));
		if (code_dmu_category && strlen(code_dmu_category))
		{
			WidgetInstanceIdentifier identifier; // DMU
			bool found_parent = input_model_->t_dmu_category.getIdentifierFromStringCode(code_dmu_category, identifier);

			if (found_parent && identifier.uuid && identifier.uuid->size() > 0)
			{
				identifiers.push_back(std::make_pair(identifier, kad_count));
			}
		}

	}

	if (stmt)
	{
		sqlite3_finalize(stmt);
		stmt = nullptr;
	}
}

bool Table_KAD_COUNT::Update(sqlite3 * db, OutputModel & output_model_, InputModel & input_model_, DataChangeMessage & change_message)
{

	std::lock_guard<std::recursive_mutex> data_lock(data_mutex);

	//Executor theExecutor(db);

	std::for_each(change_message.changes.cbegin(), change_message.changes.cend(), [&db, &input_model_, this](DataChange const & change)
	{
		switch (change.change_type)
		{
			case DATA_CHANGE_TYPE::DATA_CHANGE_TYPE__OUTPUT_MODEL__KAD_COUNT_CHANGE:
				{
					switch (change.change_intention)
					{
						case DATA_CHANGE_INTENTION__ADD:
						case DATA_CHANGE_INTENTION__REMOVE:
							{
								// Should never receive this.
							}
							break;

						case DATA_CHANGE_INTENTION__UPDATE:
							{
								// This is the OUTPUT model changing.
								// "Add" means to simply add an item that is CHECKED (previously unchecked) -
								// NOT to add a new variable.  That would be input model change type.

								if (change.child_identifiers.size() == 0)
								{
									return; // from lambda
								}

								std::for_each(change.child_identifiers.cbegin(), change.child_identifiers.cend(), [&db, &input_model_, &change, this](WidgetInstanceIdentifier const & child_identifier)
								{
									if (child_identifier.code && child_identifier.code->size() != 0)
									{
										if (change.change_intention == DATA_CHANGE_INTENTION__UPDATE)
										{
											DataChangePacket_int * packet = static_cast<DataChangePacket_int *>(change.getPacket());

											if (packet)
											{
												bool found = false;
												std::for_each(this->identifiers.begin(), this->identifiers.end(), [&found, &packet, &child_identifier](WidgetInstanceIdentifier_Int_Pair & cache_identifier)
												{
													if (boost::iequals(*child_identifier.uuid, *cache_identifier.first.uuid))
													{
														found = true;

														if (packet->getValue() != cache_identifier.second)
														{
															cache_identifier.second = packet->getValue();
														}

														return; // from lambda
													}
												});

												if (!found)
												{
													// Must add - the DMU has no entry in the output model database's KAd selection table yet
													WidgetInstanceIdentifier identifier_new;
													bool found_parent = input_model_.t_dmu_category.getIdentifierFromStringCode(*child_identifier.code, identifier_new);

													if (found_parent && identifier_new.uuid && identifier_new.uuid->size() > 0)
													{
														identifiers.push_back(std::make_pair(identifier_new, packet->getValue()));
													}

													Add(db, *child_identifier.code, packet->getValue());
												}
												else
												{
													Modify(db, *child_identifier.code, packet->getValue());
												}
											}
											else
											{
												// error condition ... todo
											}
										}
									}
								});

							}

						case DATA_CHANGE_INTENTION__RESET_ALL:
							{
								// Ditto above.
							}
							break;

						default:
							break;
					}
				}
				break;

			default:
				break;
		}
	});

	//theExecutor.success();

	//return theExecutor.succeeded();
	return true;

}

void Table_KAD_COUNT::Add(sqlite3 * db, std::string const & dmu_category_code, int const value_)
{

	std::lock_guard<std::recursive_mutex> data_lock(data_mutex);

	std::string sqlAdd("INSERT INTO KAD_COUNT (");
	sqlAdd += KAD_COUNT__DMU_CATEGORY_STRING_CODE;
	sqlAdd += ",";
	sqlAdd += KAD_COUNT__COUNT;
	sqlAdd += ",";
	sqlAdd += KAD_COUNT__FLAGS;
	sqlAdd += ") VALUES ('";
	sqlAdd += dmu_category_code;
	sqlAdd += "',";
	sqlAdd += boost::lexical_cast<std::string>(value_);
	sqlAdd += ",'')";
	sqlite3_stmt * stmt = NULL;
	sqlite3_prepare_v2(db, sqlAdd.c_str(), static_cast<int>(sqlAdd.size()) + 1, &stmt, NULL);

	if (stmt == NULL)
	{
		return;
	}

	sqlite3_step(stmt);

}

void Table_KAD_COUNT::Remove(sqlite3 * db, std::string const & dmu_category_code)
{

	std::lock_guard<std::recursive_mutex> data_lock(data_mutex);

	std::string sqlRemove("DELETE FROM KAD_COUNT WHERE ");
	sqlRemove += KAD_COUNT__DMU_CATEGORY_STRING_CODE;
	sqlRemove += "='";
	sqlRemove += dmu_category_code;
	sqlRemove += "'";
	sqlite3_stmt * stmt = NULL;
	sqlite3_prepare_v2(db, sqlRemove.c_str(), static_cast<int>(sqlRemove.size()) + 1, &stmt, NULL);

	if (stmt == NULL)
	{
		return;
	}

	sqlite3_step(stmt);

	if (stmt)
	{
		sqlite3_finalize(stmt);
		stmt = nullptr;
	}

	// Remove from cache

	size_t count = identifiers.size();
	int index_to_remove = -1;

	for (size_t n = 0; n < count; ++n)
	{
		std::pair<WidgetInstanceIdentifier, int> const & test_pair = identifiers[n];

		if (test_pair.first.code || boost::iequals(*test_pair.first.code, dmu_category_code))
		{
			index_to_remove = n;
			break;
		}
	}

	if (index_to_remove != -1)
	{
		identifiers.erase(identifiers.begin() + index_to_remove);
	}

}

void Table_KAD_COUNT::Modify(sqlite3 * db, std::string const & dmu_category_code, int const value_)
{

	std::lock_guard<std::recursive_mutex> data_lock(data_mutex);

	std::string sqlAdd("UPDATE KAD_COUNT SET ");
	sqlAdd += KAD_COUNT__COUNT;
	sqlAdd += "=";
	sqlAdd += std::to_string(value_);
	sqlAdd += " WHERE ";
	sqlAdd += KAD_COUNT__DMU_CATEGORY_STRING_CODE;
	sqlAdd += "='";
	sqlAdd += dmu_category_code;
	sqlAdd += "'";
	sqlite3_stmt * stmt = NULL;
	sqlite3_prepare_v2(db, sqlAdd.c_str(), static_cast<int>(sqlAdd.size()) + 1, &stmt, NULL);

	if (stmt == NULL)
	{
		return;
	}

	sqlite3_step(stmt);

	if (stmt)
	{
		sqlite3_finalize(stmt);
		stmt = nullptr;
	}

}
