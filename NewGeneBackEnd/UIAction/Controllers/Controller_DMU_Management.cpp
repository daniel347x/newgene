#include "../UIActionManager.h"

#ifndef Q_MOC_RUN
	#include <boost/scope_exit.hpp>
#endif
#include "../../Project/InputProject.h"
#include "../../Project/OutputProject.h"
#include "../../UIData/DataChanges.h"
#include "../../UIAction/ActionChanges.h"
#include "../../Utilities/TimeRangeHelper.h"
#include "../../Project/ProjectManager.h"

/************************************************************************/
// ACTION_ADD_DMU
// ACTION_DELETE_DMU
// ACTION_ADD_DMU_MEMBERS
// ACTION_DELETE_DMU_MEMBERS
// ACTION_REFRESH_DMUS_FROM_FILE
/************************************************************************/

void UIActionManager::AddDMU(Messager & messager__, WidgetActionItemRequest_ACTION_ADD_DMU const & action_request, InputProject & project)
{

	if (FailIfBusy(messager__))
	{
		return;
	}

	BOOST_SCOPE_EXIT(this_)
	{
		this_->EndFailIfBusy();
	} BOOST_SCOPE_EXIT_END

	if (!action_request.items)
	{
		boost::format msg("There are no new DMU categories to add.");
		messager__.ShowMessageBox(msg.str());
		return;
	}

	InputModel & input_model = project.model();

	if (!action_request.items || action_request.items->size() == 0)
	{
		boost::format msg("There are no new DMU categories to add.");
		messager__.ShowMessageBox(msg.str());
		return;
	}

	switch (action_request.reason)
	{

		case WIDGET_ACTION_ITEM_REQUEST_REASON__ADD_ITEMS:
			{

				DataChangeMessage change_response(&project);

				for_each(action_request.items->cbegin(), action_request.items->cend(), [&input_model, &messager__, &change_response](InstanceActionItem const & instanceActionItem)
				{

					Executor executor(input_model.getDb());

					if (!instanceActionItem.second)
					{
						boost::format msg("Missing a new DMU category.");
						messager__.ShowMessageBox(msg.str());
						return;
					}

					// ************************************* //
					// Retrieve data sent by user interface
					// ************************************* //
					WidgetActionItem const & actionItem = *instanceActionItem.second;
					WidgetActionItem__StringVector const & actionItemString = static_cast<WidgetActionItem__StringVector const &>(actionItem);
					std::vector<std::string> dmu_strings = actionItemString.getValue();

					if (dmu_strings.size() != 2)
					{
						boost::format msg("A DMU category name, and descriptive text, are required.");
						messager__.ShowMessageBox(msg.str());
						return;
					}

					std::string proposed_new_dmu = dmu_strings[0];
					std::string new_dmu_description = dmu_strings[1];

					bool dmu_already_exists = input_model.t_dmu_category.Exists(input_model.getDb(), input_model, proposed_new_dmu);

					if (dmu_already_exists)
					{
						boost::format msg("The DMU category '%1%' already exists.");
						msg % boost::to_upper_copy(proposed_new_dmu);
						messager__.ShowMessageBox(msg.str());
						return;
					}

					bool dmu_successfully_created = input_model.t_dmu_category.CreateNewDMU(input_model.getDb(), input_model, proposed_new_dmu, new_dmu_description);

					if (!dmu_successfully_created)
					{
						boost::format msg("Unable to execute INSERT statement to create a new DMU category.");
						throw NewGeneException() << newgene_error_description(msg.str());
					}

					std::string new_dmu(proposed_new_dmu);

					// UI will provide dialog confirmation
					//boost::format msg("DMU category '%1%' successfully created.");
					//msg % boost::to_upper_copy(proposed_new_dmu);
					//messager.ShowMessageBox(msg.str());

					// ***************************************** //
					// Prepare data to send back to user interface
					// ***************************************** //

					WidgetInstanceIdentifier newIdentifier;
					bool found_newly_created_dmu = input_model.t_dmu_category.getIdentifierFromStringCode(new_dmu, newIdentifier);

					if (!found_newly_created_dmu)
					{
						boost::format msg("Unable to find newly created DMU.");
						throw NewGeneException() << newgene_error_description(msg.str());
					}

					WidgetInstanceIdentifiers dmu_members = input_model.t_dmu_setmembers.getIdentifiers(*newIdentifier.uuid);

					DATA_CHANGE_TYPE type = DATA_CHANGE_TYPE__INPUT_MODEL__DMU_CHANGE;
					DATA_CHANGE_INTENTION intention = DATA_CHANGE_INTENTION__ADD;
					DataChange change(type, intention, newIdentifier, dmu_members);

					change_response.changes.push_back(change);

					executor.success();

				});

				messager__.EmitChangeMessage(change_response);

			}
			break;

		default:
			break;

	}

}

void UIActionManager::DeleteDMU(Messager & messager, WidgetActionItemRequest_ACTION_DELETE_DMU const & action_request, InputProject & project)
{

	if (!action_request.items)
	{
		return;
	}

	InputModel & input_model = project.model();

	switch (action_request.reason)
	{

		case WIDGET_ACTION_ITEM_REQUEST_REASON__REMOVE_ITEMS:
			{

				DataChangeMessage change_response(&project);

				std::for_each(action_request.items->cbegin(), action_request.items->cend(), [&](InstanceActionItem const & instanceActionItem)
				{

					ProjectManager & project_manager = projectManager();
					std::string errorMsg;
					bool proceed = project_manager.LetMeRunTask(ProjectManager::PROJECT_TYPE__INPUT, instanceActionItem.second->id, std::string("delete_dmu"), errorMsg);

					if (!proceed)
					{
						boost::format msg("Error deleting DMU: %1%");
						msg % errorMsg.c_str();
						messager.ShowMessageBox(msg.str());
						return;
					}

					BOOST_SCOPE_EXIT_ALL(&)
					{
						bool success = project_manager.TaskCompleted(ProjectManager::PROJECT_TYPE__INPUT, instanceActionItem.second->id, std::string("delete_dmu"), errorMsg);

						if (!success)
						{
							boost::format msg("Error deleting DMU: %1%. Please restart NewGene.");
							msg % errorMsg.c_str();
							messager.ShowMessageBox(msg.str());
						}
					};

					if (this->FailIfBusy(messager))
					{
						return;
					}

					BOOST_SCOPE_EXIT_ALL(&, this)
					{
						this->EndFailIfBusy();
					};

					Executor executor(input_model.getDb());

					WidgetInstanceIdentifier dmu = instanceActionItem.first;

					if (!dmu.code || !dmu.uuid)
					{
						boost::format msg("Missing the DMU category to delete.");
						messager.ShowMessageBox(msg.str());
						return;
					}

					// ************************************* //
					// Retrieve data sent by user interface
					// ************************************* //
					std::string dmu_to_delete_code = *dmu.code;
					std::string dmu_to_delete_uuid = *dmu.uuid;

					bool dmu_already_exists = input_model.t_dmu_category.Exists(input_model.getDb(), input_model, dmu_to_delete_code);

					if (!dmu_already_exists)
					{
						boost::format msg("The DMU category '%1%' is already absent.");
						msg % boost::to_upper_copy(dmu_to_delete_code);
						messager.ShowMessageBox(msg.str());
						return;
					}

					bool dmu_successfully_deleted = input_model.t_dmu_category.DeleteDMU(input_model.getDb(), input_model, dmu, change_response);

					if (!dmu_successfully_deleted)
					{
						boost::format msg("Unable to delete the DMU category.");
						throw NewGeneException() << newgene_error_description(msg.str());
					}

					// User request: do not display the following confirmation dialog.  It is redundant.
					//boost::format msg("DMU category '%1%' successfully deleted.");
					//msg % boost::to_upper_copy(dmu_to_delete_code);
					//messager.ShowMessageBox(msg.str());

					executor.success();

				});


				messager.EmitChangeMessage(change_response);

			}
			break;

		default:
			break;

	}

}

void UIActionManager::DeleteDMUOutput(Messager & messager__, WidgetActionItemRequest_ACTION_DELETE_DMU const & action_request, OutputProject & project)
{

	if (!action_request.items)
	{
		return;
	}

	OutputModel & output_model = project.model();
	InputModel & input_model = output_model.getInputModel();

	switch (action_request.reason)
	{

		case WIDGET_ACTION_ITEM_REQUEST_REASON__REMOVE_ITEMS:
			{

				DataChangeMessage change_response(&project);

				for_each(action_request.items->cbegin(), action_request.items->cend(), [this, &output_model, &input_model, &messager__, &change_response](
							 InstanceActionItem const & instanceActionItem)
				{

					ProjectManager & project_manager = projectManager();
					std::string errorMsg;
					bool proceed = project_manager.LetMeRunTask(ProjectManager::PROJECT_TYPE__OUTPUT, instanceActionItem.second->id, std::string("delete_dmu"), errorMsg);

					if (!proceed)
					{
						boost::format msg("Error deleting DMU: %1%");
						msg % errorMsg.c_str();
						messager__.ShowMessageBox(msg.str());
						return;
					}

					BOOST_SCOPE_EXIT_ALL(&)
					{
						bool success = project_manager.TaskCompleted(ProjectManager::PROJECT_TYPE__OUTPUT, instanceActionItem.second->id, std::string("delete_dmu"), errorMsg);

						if (!success)
						{
							boost::format msg("Error deleting DMU: %1%. Please restart NewGene.");
							msg % errorMsg.c_str();
							messager__.ShowMessageBox(msg.str());
						}
					};

					if (this->FailIfBusy(messager__))
					{
						return;
					}

					BOOST_SCOPE_EXIT_ALL(&, this)
					{
						this->EndFailIfBusy();
					};

					Executor executor(input_model.getDb());

					WidgetInstanceIdentifier dmu = instanceActionItem.first;

					if (!dmu.code || !dmu.uuid)
					{
						// Error should already be handled in input model function
						return;
					}

					// ************************************* //
					// Retrieve data sent by user interface
					// ************************************* //
					std::string dmu_to_delete_code = *dmu.code;
					std::string dmu_to_delete_uuid = *dmu.uuid;

					// The INPUT model does the bulk of the deleting,
					// and informs the UI.
					// Here, we just want to clear a couple things in the output database.

					output_model.t_kad_count.Remove(output_model.getDb(), *dmu.code);

					WidgetInstanceIdentifiers uoas = input_model.t_uoa_setmemberlookup.RetrieveUOAsGivenDMU(input_model.getDb(), &input_model, dmu);
					std::for_each(uoas.cbegin(), uoas.cend(), [&](WidgetInstanceIdentifier const & uoa)
					{
						if (uoa.uuid)
						{
							WidgetInstanceIdentifiers vgs(input_model.t_vgp_identifiers.RetrieveVGsFromUOA(input_model.getDb(), &input_model, *uoa.uuid));
							std::for_each(vgs.cbegin(), vgs.cend(), [&](WidgetInstanceIdentifier const & vg)
							{
								if (vg.code)
								{
									output_model.t_variables_selected_identifiers.RemoveAllfromVG(output_model.getDb(), vg);
								}
							});
						}
					});

					// ***************************************** //
					// Prepare data to send back to user interface
					// ***************************************** //
					// DMU's with any variables selected - this might have changed
					DATA_CHANGE_TYPE type = DATA_CHANGE_TYPE__OUTPUT_MODEL__ACTIVE_DMU_CHANGE;
					DATA_CHANGE_INTENTION intention = DATA_CHANGE_INTENTION__UPDATE;
					DataChange change(type, intention, WidgetInstanceIdentifier(), WidgetInstanceIdentifiers());
					change_response.changes.push_back(change);
					// ***************************************** //
					// Use updated cache info to set further info
					// in change_response
					// ***************************************** //
					std::set<WidgetInstanceIdentifier> active_dmus = output_model.t_variables_selected_identifiers.GetActiveDMUs(&output_model, &input_model);
					change_response.changes.back().set_of_identifiers = active_dmus;


					// Now remove the DMU from the "Limit DMU's" category table
					output_model.t_limit_dmus_categories.RemoveDMU(output_model.getDb(), output_model, input_model, dmu);

					// Now remove the corresponding DMU members from the "Limit DMU's" members table
					// empty string for last arg means "delete all members for this DMU category"
					output_model.t_limit_dmus_set_members.RemoveDmuMember(output_model.getDb(), output_model, input_model, dmu, std::string());

					// ******************************************************************************************************** //
					// No need to send this message - the Limit DMU's tab already responds to the INPUT model message change,
					// which is always sent
					// ******************************************************************************************************** //
					//type = DATA_CHANGE_TYPE__OUTPUT_MODEL__INPUT_DMU_OR_DMU_MEMBER_CHANGE;
					//intention = DATA_CHANGE_INTENTION__NONE;
					//DataChange change2(type, intention, WidgetInstanceIdentifier(), WidgetInstanceIdentifiers());
					//change_response.changes.push_back(change2);

					executor.success();

				});

				messager__.EmitChangeMessage(change_response);

			}
			break;

		default:
			break;

	}

}

void UIActionManager::AddDMUMembers(Messager & messager__, WidgetActionItemRequest_ACTION_ADD_DMU_MEMBERS const & action_request, InputProject & project)
{

	if (FailIfBusy(messager__))
	{
		return;
	}

	BOOST_SCOPE_EXIT(this_)
	{
		this_->EndFailIfBusy();
	} BOOST_SCOPE_EXIT_END

	if (!action_request.items)
	{
		return;
	}

	InputModel & input_model = project.model();

	switch (action_request.reason)
	{

		case WIDGET_ACTION_ITEM_REQUEST_REASON__ADD_ITEMS:
			{

				DataChangeMessage change_response(&project);

				std::string result_msg("The following DMU members have been added:");
				result_msg += "\n";

				for_each(action_request.items->cbegin(), action_request.items->cend(), [&result_msg, &input_model, &messager__, &change_response](InstanceActionItem const & instanceActionItem)
				{

					Executor executor(input_model.getDb());

					WidgetInstanceIdentifier dmu_category = instanceActionItem.first;

					if (!dmu_category.uuid || dmu_category.uuid->empty())
					{
						boost::format msg("Missing the associated DMU category.");
						messager__.ShowMessageBox(msg.str());
						return;
					}

					if (!instanceActionItem.second)
					{
						boost::format msg("Missing a new DMU member.");
						messager__.ShowMessageBox(msg.str());
						return;
					}

					// ************************************* //
					// Retrieve data sent by user interface
					// ************************************* //
					WidgetActionItem const & actionItem = *instanceActionItem.second;
					WidgetActionItem__StringVector const & actionItemString = static_cast<WidgetActionItem__StringVector const &>(actionItem);
					std::vector<std::string> dmu_strings = actionItemString.getValue();

					if (dmu_strings.size() != 3)
					{
						boost::format msg("A DMU member code, name and descriptive text are required.");
						messager__.ShowMessageBox(msg.str());
						return;
					}

					std::string proposed_new_dmu_member_uuid = dmu_strings[0];
					std::string proposed_new_dmu_member_code = dmu_strings[1];
					std::string proposed_new_dmu_member_description = dmu_strings[2];

					bool dmu_member_already_exists = input_model.t_dmu_setmembers.Exists(input_model.getDb(), input_model, dmu_category, proposed_new_dmu_member_uuid);

					if (dmu_member_already_exists)
					{
						boost::format msg("The DMU member '%1%' already exists for '%2%'.");
						msg % boost::to_upper_copy(proposed_new_dmu_member_uuid) % boost::to_upper_copy(*dmu_category.code);
						messager__.ShowMessageBox(msg.str());
						return;
					}

					WidgetInstanceIdentifier dmu_member = input_model.t_dmu_setmembers.CreateNewDmuMember(input_model.getDb(), input_model, dmu_category, proposed_new_dmu_member_uuid,
														  proposed_new_dmu_member_code, proposed_new_dmu_member_description);

					if (dmu_member.IsEmpty())
					{
						boost::format msg("Unable to execute INSERT statement to create a new DMU member.");
						throw NewGeneException() << newgene_error_description(msg.str());
					}

					boost::format msg("%1% (%2%)\n");
					msg % Table_DMU_Instance::GetDmuMemberDisplayText(dmu_member) % Table_DMU_Identifier::GetDmuCategoryDisplayText(dmu_category);
					result_msg += msg.str();



					//// ***************************************** //
					//// Prepare data to send back to user interface
					//// ***************************************** //
					//DATA_CHANGE_TYPE type = DATA_CHANGE_TYPE__INPUT_MODEL__DMU_MEMBERS_CHANGE;
					//DATA_CHANGE_INTENTION intention = DATA_CHANGE_INTENTION__ADD;
					//DataChange change(type, intention, dmu_member, WidgetInstanceIdentifiers());

					// ***************************************** //
					// Prepare data to send back to user interface
					// ***************************************** //

					WidgetInstanceIdentifiers dmu_members = input_model.t_dmu_setmembers.getIdentifiers(*dmu_category.uuid);

					DATA_CHANGE_TYPE type = DATA_CHANGE_TYPE__INPUT_MODEL__DMU_MEMBERS_CHANGE;
					DATA_CHANGE_INTENTION intention = DATA_CHANGE_INTENTION__RESET_ALL;
					DataChange change(type, intention, dmu_category, dmu_members);

					change_response.changes.push_back(change);

					executor.success();

				});

				EndFailIfBusy();

				boost::format msg("%1%");
				msg % result_msg;
				messager__.ShowMessageBox(msg.str());

				messager__.EmitChangeMessage(change_response);

			}
			break;

		default:
			break;

	}

}

void UIActionManager::DeleteDMUMembers(Messager & messager, WidgetActionItemRequest_ACTION_DELETE_DMU_MEMBERS const & action_request, InputProject & project)
{

	if (FailIfBusy(messager))
	{
		return;
	}

	BOOST_SCOPE_EXIT(this_)
	{
		this_->EndFailIfBusy();
	} BOOST_SCOPE_EXIT_END

	if (!action_request.items)
	{
		return;
	}

	InputModel & input_model = project.model();

	switch (action_request.reason)
	{

		case WIDGET_ACTION_ITEM_REQUEST_REASON__REMOVE_ITEMS:
			{

				DataChangeMessage change_response(&project);

				std::string result_msg("The following DMU members have been removed:");
				result_msg += "\n";

				for_each(action_request.items->cbegin(), action_request.items->cend(), [&result_msg, &input_model, &messager, &change_response](InstanceActionItem const & instanceActionItem)
				{

					Executor executor(input_model.getDb());

					WidgetInstanceIdentifier const & dmu_member = instanceActionItem.first;

					if (!dmu_member.uuid || dmu_member.uuid->empty() || !dmu_member.identifier_parent)
					{
						boost::format msg("Missing the DMU member to delete.");
						messager.ShowMessageBox(msg.str());
						return;
					}

					WidgetInstanceIdentifier const & dmu_category = *dmu_member.identifier_parent;

					boost::format msg("%1% (from %2%)\n");
					msg % Table_DMU_Instance::GetDmuMemberDisplayText(dmu_member) % Table_DMU_Identifier::GetDmuCategoryDisplayText(dmu_category);
					result_msg += msg.str();

					input_model.t_dmu_setmembers.DeleteDmuMember(input_model.getDb(), input_model, dmu_member);


					//// ***************************************** //
					//// Prepare data to send back to user interface
					//// ***************************************** //
					//DATA_CHANGE_TYPE type = DATA_CHANGE_TYPE__INPUT_MODEL__DMU_MEMBERS_CHANGE;
					//DATA_CHANGE_INTENTION intention = DATA_CHANGE_INTENTION__REMOVE;
					//DataChange change(type, intention, dmu_member, WidgetInstanceIdentifiers());

					// ***************************************** //
					// Prepare data to send back to user interface
					// ***************************************** //

					WidgetInstanceIdentifiers dmu_members = input_model.t_dmu_setmembers.getIdentifiers(*dmu_category.uuid);

					DATA_CHANGE_TYPE type = DATA_CHANGE_TYPE__INPUT_MODEL__DMU_MEMBERS_CHANGE;
					DATA_CHANGE_INTENTION intention = DATA_CHANGE_INTENTION__RESET_ALL;
					DataChange change(type, intention, dmu_category, dmu_members);

					change_response.changes.push_back(change);

					executor.success();

				});

				EndFailIfBusy();

				boost::format msg("%1%");
				msg % result_msg;
				messager.ShowMessageBox(msg.str());

				messager.EmitChangeMessage(change_response);

			}
			break;

		default:
			break;

	}

}

void UIActionManager::DeleteDMUMembersOutput(Messager & messager, WidgetActionItemRequest_ACTION_DELETE_DMU_MEMBERS const & action_request, OutputProject & project)
{

	if (FailIfBusy(messager))
	{
		return;
	}

	BOOST_SCOPE_EXIT(this_)
	{
		this_->EndFailIfBusy();
	} BOOST_SCOPE_EXIT_END

	if (!action_request.items)
	{
		return;
	}

	OutputModel & output_model = project.model();
	InputModel & input_model = output_model.getInputModel();

	switch (action_request.reason)
	{

		case WIDGET_ACTION_ITEM_REQUEST_REASON__REMOVE_ITEMS:
			{

				//DataChangeMessage change_response(&project);

				for_each(action_request.items->cbegin(), action_request.items->cend(), [&input_model, &output_model, &messager](InstanceActionItem const & instanceActionItem)
				{

					Executor executor(input_model.getDb());

					WidgetInstanceIdentifier const & dmu_member = instanceActionItem.first;
					WidgetInstanceIdentifier const & dmu_category = *dmu_member.identifier_parent;

					// Now remove the corresponding DMU members from the "Limit DMU's" members table
					output_model.t_limit_dmus_set_members.RemoveDmuMember(output_model.getDb(), output_model, input_model, dmu_category, *dmu_member.uuid);

					// ******************************************************************************************************** //
					// No need to send this message - the Limit DMU's tab already responds to the INPUT model message change,
					// which is always sent
					// ******************************************************************************************************** //
					//DATA_CHANGE_TYPE type = DATA_CHANGE_TYPE__OUTPUT_MODEL__INPUT_DMU_OR_DMU_MEMBER_CHANGE;
					//DATA_CHANGE_INTENTION intention = DATA_CHANGE_INTENTION__NONE;
					//DataChange change(type, intention, WidgetInstanceIdentifier(), WidgetInstanceIdentifiers());
					//change_response.changes.push_back(change);

					executor.success();

				});

				//messager.EmitChangeMessage(change_response);

			}
			break;

		default:
			break;

	}

}

void UIActionManager::RefreshDMUsFromFile(Messager & messager__, WidgetActionItemRequest_ACTION_REFRESH_DMUS_FROM_FILE const & action_request, InputProject & project)
{

	if (FailIfBusy(messager__))
	{
		return;
	}

	BOOST_SCOPE_EXIT(this_)
	{
		this_->EndFailIfBusy();
	} BOOST_SCOPE_EXIT_END

	if (!action_request.items)
	{
		return;
	}

	{
		std::lock_guard<std::recursive_mutex> guard(Importer::is_performing_import_mutex);

		if (Importer::is_performing_import)
		{
			boost::format msg("Another import operation is in progress.  Please wait for that operation to complete first.");
			throw NewGeneException() << newgene_error_description(msg.str());
		}
	}

	InputModel & input_model = project.model();

	switch (action_request.reason)
	{

		case WIDGET_ACTION_ITEM_REQUEST_REASON__DO_ACTION:
			{

				DataChangeMessage change_response(&project);

				for_each(action_request.items->cbegin(), action_request.items->cend(), [&input_model, &messager__, &change_response](InstanceActionItem const & instanceActionItem)
				{

					WidgetInstanceIdentifier dmu_category = instanceActionItem.first;

					if (!dmu_category.code || !dmu_category.uuid || dmu_category.code->empty() || dmu_category.uuid->empty())
					{
						boost::format msg("Missing the DMU category to refresh.");
						messager__.ShowMessageBox(msg.str());
						return;
					}

					if (!instanceActionItem.second)
					{
						boost::format msg("Missing DMU refresh information.");
						messager__.ShowMessageBox(msg.str());
						return;
					}

					// ************************************* //
					// Retrieve data sent by user interface
					// ************************************* //
					WidgetActionItem const & actionItem = *instanceActionItem.second;
					WidgetActionItem__StringVector_Plus_Int const & actionItemStringInt = static_cast<WidgetActionItem__StringVector_Plus_Int const &>(actionItem);
					std::vector<std::string> dmu_refresh_strings = actionItemStringInt.getValue();
					bool do_refresh_not_plain_insert = actionItemStringInt.getIntValue() > 0 ? true : false;

					std::string dmu_refresh_file_pathname = dmu_refresh_strings[0];
					std::vector<std::string> dmu_refresh_column_labels(dmu_refresh_strings.cbegin() + 1, dmu_refresh_strings.cend());

					bool success = input_model.t_dmu_setmembers.RefreshFromFile(input_model.getDb(), input_model, dmu_category, boost::filesystem::path(dmu_refresh_file_pathname),
								   dmu_refresh_column_labels, messager__, do_refresh_not_plain_insert);

					if (!success)
					{
						return;
					}

					std::string cancelAddendum;

					if (Importer::cancelled)
					{
						cancelAddendum = " (until cancelled)";
					}

					if (input_model.t_dmu_setmembers.badreadlines > 0 || input_model.t_dmu_setmembers.badwritelines > 0)
					{
						if (input_model.t_dmu_setmembers.badreadlines > 0 && input_model.t_dmu_setmembers.badwritelines > 0)
						{
							if (do_refresh_not_plain_insert)
							{
								// Handle incoming data row-by-row, distinguishing between inserts and updates
								boost::format
								msg("DMU category '%1%' refreshed %5% lines from file%4% (%6% written to, %7% updated in database), but %2% rows failed when being read from the input file and %3% rows failed to be written to the database.  See the \"newgene.import.log\" file for details.");
								msg % Table_DMU_Identifier::GetDmuCategoryDisplayText(dmu_category)
								% boost::lexical_cast<std::string>(input_model.t_dmu_setmembers.badreadlines)
								% boost::lexical_cast<std::string>(input_model.t_dmu_setmembers.badwritelines)
								% cancelAddendum
								% boost::lexical_cast<std::string>(input_model.t_dmu_setmembers.goodreadlines)
								% boost::lexical_cast<std::string>(input_model.t_dmu_setmembers.goodwritelines)
								% boost::lexical_cast<std::string>(input_model.t_dmu_setmembers.goodupdatelines);
								messager__.ShowMessageBox(msg.str());
							}
							else
							{
								// Bulk INSERT OR REPLACE mode - we do not currently distinguish between inserts and updates
								boost::format
								msg("DMU category '%1%' refreshed %5% lines from file%4% (%6% written to and/or updated in database), but %2% rows failed when being read from the input file and %3% rows failed to be written to the database.  See the \"newgene.import.log\" file for details.");
								msg % Table_DMU_Identifier::GetDmuCategoryDisplayText(dmu_category)
								% boost::lexical_cast<std::string>(input_model.t_dmu_setmembers.badreadlines)
								% boost::lexical_cast<std::string>(input_model.t_dmu_setmembers.badwritelines)
								% cancelAddendum
								% boost::lexical_cast<std::string>(input_model.t_dmu_setmembers.goodreadlines)
								% boost::lexical_cast<std::string>(input_model.t_dmu_setmembers.goodwritelines + input_model.t_dmu_setmembers.goodupdatelines);
								messager__.ShowMessageBox(msg.str());
							}
						}
						else if (input_model.t_dmu_setmembers.badreadlines == 0 && input_model.t_dmu_setmembers.badwritelines > 0)
						{
							if (do_refresh_not_plain_insert)
							{
								// Handle incoming data row-by-row, distinguishing between inserts and updates
								boost::format
								msg("DMU category '%1%' refreshed %4% lines from file%3% (%5% written to, %6% updated in database), but %2% rows failed to be written to the database.  See the \"newgene.import.log\" file for details.");
								msg % Table_DMU_Identifier::GetDmuCategoryDisplayText(dmu_category)
								% boost::lexical_cast<std::string>(input_model.t_dmu_setmembers.badwritelines)
								% cancelAddendum
								% boost::lexical_cast<std::string>(input_model.t_dmu_setmembers.goodreadlines)
								% boost::lexical_cast<std::string>(input_model.t_dmu_setmembers.goodwritelines)
								% boost::lexical_cast<std::string>(input_model.t_dmu_setmembers.goodupdatelines);
								messager__.ShowMessageBox(msg.str());
							}
							else
							{
								// Bulk INSERT OR REPLACE mode - we do not currently distinguish between inserts and updates
								boost::format
								msg("DMU category '%1%' refreshed %4% lines from file%3% (%5% written to and/or updated in database), but %2% rows failed to be written to the database.  See the \"newgene.import.log\" file for details.");
								msg % Table_DMU_Identifier::GetDmuCategoryDisplayText(dmu_category)
								% boost::lexical_cast<std::string>(input_model.t_dmu_setmembers.badwritelines)
								% cancelAddendum
								% boost::lexical_cast<std::string>(input_model.t_dmu_setmembers.goodreadlines)
								% boost::lexical_cast<std::string>(input_model.t_dmu_setmembers.goodwritelines + input_model.t_dmu_setmembers.goodupdatelines);
								messager__.ShowMessageBox(msg.str());
							}
						}
						else if (input_model.t_dmu_setmembers.badreadlines > 0 && input_model.t_dmu_setmembers.badwritelines == 0)
						{
							if (do_refresh_not_plain_insert)
							{
								// Handle incoming data row-by-row, distinguishing between inserts and updates
								boost::format
								msg("DMU category '%1%' refreshed %4% lines from file%3% (%5% written to, %6% updated in database), but %2% rows failed when being read from the input file.  See the \"newgene.import.log\" file for details.");
								msg % Table_DMU_Identifier::GetDmuCategoryDisplayText(dmu_category)
								% boost::lexical_cast<std::string>(input_model.t_dmu_setmembers.badreadlines)
								% cancelAddendum
								% boost::lexical_cast<std::string>(input_model.t_dmu_setmembers.goodreadlines)
								% boost::lexical_cast<std::string>(input_model.t_dmu_setmembers.goodwritelines)
								% boost::lexical_cast<std::string>(input_model.t_dmu_setmembers.goodupdatelines);
								messager__.ShowMessageBox(msg.str());
							}
							else
							{
								// Bulk INSERT OR REPLACE mode - we do not currently distinguish between inserts and updates
								boost::format
								msg("DMU category '%1%' refreshed %4% lines from file%3% (%5% written to and/or updated in database), but %2% rows failed when being read from the input file.  See the \"newgene.import.log\" file for details.");
								msg % Table_DMU_Identifier::GetDmuCategoryDisplayText(dmu_category)
								% boost::lexical_cast<std::string>(input_model.t_dmu_setmembers.badreadlines)
								% cancelAddendum
								% boost::lexical_cast<std::string>(input_model.t_dmu_setmembers.goodreadlines)
								% boost::lexical_cast<std::string>(input_model.t_dmu_setmembers.goodwritelines + input_model.t_dmu_setmembers.goodupdatelines);
								messager__.ShowMessageBox(msg.str());
							}
						}
					}
					else
					{
						if (! Importer::CheckCancelled() && (input_model.t_dmu_setmembers.goodreadlines != input_model.t_dmu_setmembers.goodwritelines + input_model.t_dmu_setmembers.goodupdatelines))
						{
							boost::format
							msg("During import of DMU members, with no read or write failures, nonetheless the number of successful lines read from input file (%1%) does not match the number of successful lines written to (%2%) and updated in (%3%) the database.");
							msg % boost::lexical_cast<std::string>(input_model.t_dmu_setmembers.goodreadlines) % boost::lexical_cast<std::string>(input_model.t_dmu_setmembers.goodwritelines) %
							boost::lexical_cast<std::string>(input_model.t_dmu_setmembers.goodupdatelines);
							throw NewGeneException() << newgene_error_description(msg.str());
						}

						if (do_refresh_not_plain_insert)
						{
							// Handle incoming data row-by-row, distinguishing between inserts and updates
							boost::format msg("DMU '%1%' successfully read %3% DMU members from file (%4% updated and %5% inserted)%2%.");
							msg % Table_DMU_Identifier::GetDmuCategoryDisplayText(dmu_category)
							% cancelAddendum
							% boost::lexical_cast<std::string>(input_model.t_dmu_setmembers.goodreadlines)
							% boost::lexical_cast<std::string>(input_model.t_dmu_setmembers.goodupdatelines)
							% boost::lexical_cast<std::string>(input_model.t_dmu_setmembers.goodwritelines);
							messager__.ShowMessageBox(msg.str());
						}
						else
						{
							// Bulk INSERT OR REPLACE mode - we do not currently distinguish between inserts and updates
							boost::format msg("DMU '%1%' successfully read %3% DMU members from file (%4% written to database)%2%.");
							msg % Table_DMU_Identifier::GetDmuCategoryDisplayText(dmu_category)
							% cancelAddendum
							% boost::lexical_cast<std::string>(input_model.t_dmu_setmembers.goodreadlines)
							% boost::lexical_cast<std::string>(input_model.t_dmu_setmembers.goodupdatelines + input_model.t_dmu_setmembers.goodwritelines);
							messager__.ShowMessageBox(msg.str());
						}
					}

					// ***************************************** //
					// Prepare data to send back to user interface
					// ***************************************** //

					WidgetInstanceIdentifiers dmu_members = input_model.t_dmu_setmembers.getIdentifiers(*dmu_category.uuid);

					DATA_CHANGE_TYPE type = DATA_CHANGE_TYPE__INPUT_MODEL__DMU_MEMBERS_CHANGE;
					DATA_CHANGE_INTENTION intention = DATA_CHANGE_INTENTION__RESET_ALL;
					DataChange change(type, intention, dmu_category, dmu_members);

					change_response.changes.push_back(change);

				});

				messager__.EmitChangeMessage(change_response);

			}
			break;

		default:
			break;

	}

}
