#include "../UIActionManager.h"

#ifndef Q_MOC_RUN
#	include <boost/scope_exit.hpp>
#endif
#include "../../Project/InputProject.h"
#include "../../Project/OutputProject.h"
#include "../../UIData/DataChanges.h"
#include "../../UIAction/ActionChanges.h"
#include "../../Utilities/TimeRangeHelper.h"
#include "../../Project/ProjectManager.h"

/************************************************************************/
// ACTION_ADD_UOA
// ACTION_DELETE_UOA
/************************************************************************/

void UIActionManager::AddUOA(Messager & messager__, WidgetActionItemRequest_ACTION_ADD_UOA const & action_request, InputProject & project)
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
		boost::format msg("There are no new UOAs to add.");
		messager__.ShowMessageBox(msg.str());
		return;
	}

	InputModel & input_model = project.model();

	if (!action_request.items || action_request.items->size() == 0)
	{
		boost::format msg("There are no new UOAs to add.");
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
						boost::format msg("Missing the new UOA to create.");
						messager__.ShowMessageBox(msg.str());
						return;
					}

					// ************************************* //
					// Retrieve data sent by user interface
					// ************************************* //
					WidgetActionItem const & actionItem = *instanceActionItem.second;
					WidgetActionItem__WidgetInstanceIdentifiers_Plus_String_String_And_Int const & actionItemWsStringStringInt = static_cast<WidgetActionItem__WidgetInstanceIdentifiers_Plus_String_String_And_Int const &>(actionItem);
					WidgetInstanceIdentifiers const & dmu_categories = actionItemWsStringStringInt.getValue();
					std::string const & new_uoa_code = actionItemWsStringStringInt.getValueString();
					std::string const & uoa_description = actionItemWsStringStringInt.getValueString2();
					int const & the_time_granularity = actionItemWsStringStringInt.getValueInt();
					TIME_GRANULARITY time_granularity = (TIME_GRANULARITY)the_time_granularity;

					if (dmu_categories.size() == 0)
					{
						boost::format msg("At least one DMU category is required to create a new UOA.");
						messager__.ShowMessageBox(msg.str());
						return;
					}

					bool uoa_already_exists = input_model.t_uoa_category.ExistsByCode(input_model.getDb(), input_model, new_uoa_code);

					if (uoa_already_exists)
					{
						boost::format msg("The UOA code '%1%' already exists.");
						msg % boost::to_upper_copy(new_uoa_code);
						messager__.ShowMessageBox(msg.str());
						return;
					}

					bool uoa_successfully_created = input_model.t_uoa_category.CreateNewUOA(input_model.getDb(), input_model, new_uoa_code, uoa_description, dmu_categories, time_granularity);

					if (!uoa_successfully_created)
					{
						boost::format msg("Unable to execute INSERT statement to create a new UOA category.");
						throw NewGeneException() << newgene_error_description(msg.str());
					}

					boost::format msg("UOA '%1%' successfully created.");
					msg % boost::to_upper_copy(new_uoa_code);
					messager__.ShowMessageBox(msg.str());

					// ***************************************** //
					// Prepare data to send back to user interface
					// ***************************************** //

					WidgetInstanceIdentifier newIdentifier;
					bool found_newly_created_uoa = input_model.t_uoa_category.getIdentifierFromStringCode(new_uoa_code, newIdentifier);

					if (!found_newly_created_uoa || !newIdentifier.uuid || newIdentifier.uuid->empty())
					{
						boost::format msg("Unable to find newly created UOA.");
						throw NewGeneException() << newgene_error_description(msg.str());
					}

					DATA_CHANGE_TYPE type = DATA_CHANGE_TYPE__INPUT_MODEL__UOA_CHANGE;
					DATA_CHANGE_INTENTION intention = DATA_CHANGE_INTENTION__ADD;
					DataChange change(type, intention, newIdentifier, dmu_categories);

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

void UIActionManager::DeleteUOA(Messager & messager, WidgetActionItemRequest_ACTION_DELETE_UOA const & action_request, InputProject & project)
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

				for_each(action_request.items->cbegin(), action_request.items->cend(), [this, &input_model, &messager, &change_response](InstanceActionItem const & instanceActionItem)
				{

					ProjectManager & project_manager = projectManager();
					std::string errorMsg;
					bool proceed = project_manager.LetMeRunTask(ProjectManager::PROJECT_TYPE__INPUT, instanceActionItem.second->id, std::string("delete_uoa"), errorMsg);
					if (!proceed)
					{
						boost::format msg("Error deleting UOA: %1%");
						msg % errorMsg.c_str();
						messager.ShowMessageBox(msg.str());
						return;
					}
					BOOST_SCOPE_EXIT_ALL(&)
					{
						bool success = project_manager.TaskCompleted(ProjectManager::PROJECT_TYPE__INPUT, instanceActionItem.second->id, std::string("delete_uoa"), errorMsg);
						if (!success)
						{
							boost::format msg("Error deleting UOA: %1%. Please restart NewGene.");
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

					if (!instanceActionItem.second)
					{
						boost::format msg("The DMU identifiers are invalid.");
						messager.ShowMessageBox(msg.str());
						return;
					}

					WidgetInstanceIdentifier uoa_category = instanceActionItem.first;
					WidgetActionItem const & actionItem = *instanceActionItem.second;
					WidgetActionItem__WidgetInstanceIdentifiers const & actionItemDMUs = static_cast<WidgetActionItem__WidgetInstanceIdentifiers const &>(actionItem);
					WidgetInstanceIdentifiers dmu_categories = actionItemDMUs.getValue();

					if (!uoa_category.uuid || uoa_category.uuid->empty())
					{
						boost::format msg("Missing the UOA to delete.");
						messager.ShowMessageBox(msg.str());
						return;
					}

					// ************************************* //
					// Retrieve data sent by user interface
					// ************************************* //
					std::string uoa_to_delete_display_text = Table_UOA_Identifier::GetUoaCategoryDisplayText(uoa_category, dmu_categories);

					bool uoa_already_exists = input_model.t_uoa_category.Exists(input_model.getDb(), input_model, uoa_category);

					if (!uoa_already_exists)
					{
						boost::format msg("The UOA '%1%' is already absent.");
						msg % boost::to_upper_copy(uoa_to_delete_display_text);
						messager.ShowMessageBox(msg.str());
						return;
					}

					bool uoa_successfully_deleted = input_model.t_uoa_category.DeleteUOA(input_model.getDb(), input_model, uoa_category, change_response);

					if (!uoa_successfully_deleted)
					{
						boost::format msg("Unable to delete the UOA.");
						throw NewGeneException() << newgene_error_description(msg.str());
					}

					// User request: do not display the following confirmation dialog.  It is redundant.
					//boost::format msg("UOA '%1%' successfully deleted.");
					//msg % boost::to_upper_copy(uoa_to_delete_display_text);
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

void UIActionManager::DeleteUOAOutput(Messager & messager, WidgetActionItemRequest_ACTION_DELETE_UOA const & action_request, OutputProject & project)
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

				std::for_each(action_request.items->cbegin(), action_request.items->cend(), [this, &output_model, &input_model, &messager, &change_response](InstanceActionItem const & instanceActionItem)
				{

					ProjectManager & project_manager = projectManager();
					std::string errorMsg;
					bool proceed = project_manager.LetMeRunTask(ProjectManager::PROJECT_TYPE__OUTPUT, instanceActionItem.second->id, std::string("delete_uoa"), errorMsg);
					if (!proceed)
					{
						boost::format msg("Error deleting UOA: %1%");
						msg % errorMsg.c_str();
						messager.ShowMessageBox(msg.str());
						return;
					}
					BOOST_SCOPE_EXIT_ALL(&)
					{
						bool success = project_manager.TaskCompleted(ProjectManager::PROJECT_TYPE__OUTPUT, instanceActionItem.second->id, std::string("delete_uoa"), errorMsg);
						if (!success)
						{
							boost::format msg("Error deleting UOA: %1%. Please restart NewGene.");
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

					WidgetInstanceIdentifier uoa_category = instanceActionItem.first;
					if (!uoa_category.uuid || uoa_category.uuid->empty())
					{
						// Error should already be handled in input model function
						return;
					}

					WidgetInstanceIdentifiers vgs(input_model.t_vgp_identifiers.RetrieveVGsFromUOA(input_model.getDb(), &input_model, *uoa_category.uuid));
					std::for_each(vgs.cbegin(), vgs.cend(), [&](WidgetInstanceIdentifier const & vg)
					{
						if (vg.code)
						{
							output_model.t_variables_selected_identifiers.RemoveAllfromVG(output_model.getDb(), vg);
						}
					});

					// ***************************************** //
					// Prepare data to send back to user interface
					// ***************************************** //
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

					executor.success();

				});

				messager.EmitChangeMessage(change_response);

			}
			break;

		default:
			break;

	}

}
