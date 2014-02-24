#include "../UIActionManager.h"

#include "../../Project/InputProject.h"
#include "../../Project/OutputProject.h"
#include "../../UIData/DataChanges.h"
#include "../../UIAction/ActionChanges.h"
#ifndef Q_MOC_RUN
#	include <boost/scope_exit.hpp>
#endif

/************************************************************************/
// ACTION_ADD_DMU
// ACTION_DELETE_DMU
// ACTION_ADD_DMU_MEMBERS
// ACTION_DELETE_DMU_MEMBERS
// ACTION_REFRESH_DMUS_FROM_FILE
/************************************************************************/

void UIActionManager::AddDMU(Messager & messager, WidgetActionItemRequest_ACTION_ADD_DMU const & action_request, InputProject & project)
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
		boost::format msg("There are no new DMU categories to add.");
		messager.ShowMessageBox(msg.str());
		return;
	}

	InputModel & input_model = project.model();

	if (!action_request.items || action_request.items->size() == 0)
	{
		boost::format msg("There are no new DMU categories to add.");
		messager.ShowMessageBox(msg.str());
		return;
	}

	switch (action_request.reason)
	{

	case WIDGET_ACTION_ITEM_REQUEST_REASON__ADD_ITEMS:
		{

			DataChangeMessage change_response(&project);

			for_each(action_request.items->cbegin(), action_request.items->cend(), [&input_model, &messager, &change_response](InstanceActionItem const & instanceActionItem)
			{
				if (!instanceActionItem.second)
				{
					boost::format msg("Missing a new DMU category.");
					messager.ShowMessageBox(msg.str());
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
					messager.ShowMessageBox(msg.str());
					return;
				}

				std::string proposed_new_dmu = dmu_strings[0];
				std::string new_dmu_description = dmu_strings[1];

				bool dmu_already_exists = input_model.t_dmu_category.Exists(input_model.getDb(), input_model, proposed_new_dmu);
				if (dmu_already_exists)
				{
					boost::format msg("The DMU category '%1%' already exists.");
					msg % boost::to_upper_copy(proposed_new_dmu);
					messager.ShowMessageBox(msg.str());
					return;
				}

				bool dmu_successfully_created = input_model.t_dmu_category.CreateNewDMU(input_model.getDb(), input_model, proposed_new_dmu, new_dmu_description);

				if (!dmu_successfully_created)
				{
					boost::format msg("Unable to execute INSERT statement to create a new DMU category.");
					throw NewGeneException() << newgene_error_description(msg.str());
				}

				std::string new_dmu(proposed_new_dmu);

				boost::format msg("DMU category '%1%' successfully created.");
				msg % boost::to_upper_copy(proposed_new_dmu);
				messager.ShowMessageBox(msg.str());

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

			});

			messager.EmitChangeMessage(change_response);
	
		}
			break;

		default:
			break;

	}

}

void UIActionManager::DeleteDMU(Messager & messager, WidgetActionItemRequest_ACTION_DELETE_DMU const & action_request, InputProject & project)
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

			for_each(action_request.items->cbegin(), action_request.items->cend(), [&input_model, &messager, &change_response](InstanceActionItem const & instanceActionItem)
			{

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

				boost::format msg("DMU category '%1%' successfully deleted.");
				msg % boost::to_upper_copy(dmu_to_delete_code);
				messager.ShowMessageBox(msg.str());

			});


			messager.EmitChangeMessage(change_response);

		}
			break;

		default:
			break;

	}

}

void UIActionManager::DeleteDMUOutput(Messager & messager, WidgetActionItemRequest_ACTION_DELETE_DMU const & action_request, OutputProject & project)
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

			DataChangeMessage change_response(&project);

			for_each(action_request.items->cbegin(), action_request.items->cend(), [&output_model, &input_model, &messager, &change_response](InstanceActionItem const & instanceActionItem)
			{

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
								output_model.t_variables_selected_identifiers.RemoveAllfromVG(output_model.getDb(), *vg.code);
							}
						});
					}
				});

			});

			messager.EmitChangeMessage(change_response);

		}
		break;

		default:
			break;

	}

}

void UIActionManager::AddDMUMembers(Messager & messager, WidgetActionItemRequest_ACTION_ADD_DMU_MEMBERS const & action_request, InputProject & project)
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
		case WIDGET_ACTION_ITEM_REQUEST_REASON__ADD_ITEMS:
		{

			DataChangeMessage change_response(&project);

			std::string result_msg("The following DMU members have been added:");
			result_msg += "\n";

			for_each(action_request.items->cbegin(), action_request.items->cend(), [&result_msg, &input_model, &messager, &change_response](InstanceActionItem const & instanceActionItem)
			{

				WidgetInstanceIdentifier dmu_category = instanceActionItem.first;
				if (!dmu_category.uuid || dmu_category.uuid->empty())
				{
					boost::format msg("Missing the associated DMU category.");
					messager.ShowMessageBox(msg.str());
					return;
				}

				if (!instanceActionItem.second)
				{
					boost::format msg("Missing a new DMU member.");
					messager.ShowMessageBox(msg.str());
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
					messager.ShowMessageBox(msg.str());
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
					messager.ShowMessageBox(msg.str());
					return;
				}

				WidgetInstanceIdentifier dmu_member = input_model.t_dmu_setmembers.CreateNewDmuMember(input_model.getDb(), input_model, dmu_category, proposed_new_dmu_member_uuid, proposed_new_dmu_member_code, proposed_new_dmu_member_description);

				if (dmu_member.IsEmpty())
				{
					boost::format msg("Unable to execute INSERT statement to create a new DMU member.");
					throw NewGeneException() << newgene_error_description(msg.str());
				}

				boost::format msg("%1% (%2%)\n");
				msg % Table_DMU_Instance::GetDmuMemberDisplayText(dmu_member) % Table_DMU_Identifier::GetDmuCategoryDisplayText(dmu_category);
				result_msg += msg.str();



				// ***************************************** //
				// Prepare data to send back to user interface
				// ***************************************** //
				DATA_CHANGE_TYPE type = DATA_CHANGE_TYPE__INPUT_MODEL__DMU_MEMBERS_CHANGE;
				DATA_CHANGE_INTENTION intention = DATA_CHANGE_INTENTION__ADD;
				DataChange change(type, intention, dmu_member, WidgetInstanceIdentifiers());

				change_response.changes.push_back(change);

			});

			boost::format msg("%1%");
			msg % result_msg;
			messager.ShowMessageBox(msg.str());

			messager.EmitChangeMessage(change_response);

		}

			break;
		case WIDGET_ACTION_ITEM_REQUEST_REASON__REMOVE_ITEMS:
		{

		}

			break;
		case WIDGET_ACTION_ITEM_REQUEST_REASON__UPDATE_ITEMS:
		{

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

		case WIDGET_ACTION_ITEM_REQUEST_REASON__ADD_ITEMS:
		{

		}
			break;

		case WIDGET_ACTION_ITEM_REQUEST_REASON__REMOVE_ITEMS:
		{

			DataChangeMessage change_response(&project);

			for_each(action_request.items->cbegin(), action_request.items->cend(), [&input_model, &messager, &change_response](InstanceActionItem const & instanceActionItem)
			{

				WidgetInstanceIdentifier dmu_member = instanceActionItem.first;

				if (!dmu_member.uuid || dmu_member.uuid->empty())
				{
					boost::format msg("Missing the DMU member to delete.");
					messager.ShowMessageBox(msg.str());
					return;
				}

				input_model.t_dmu_setmembers.DeleteDmuMember(input_model.getDb(), input_model, dmu_member, change_response);

				//	// ***************************************** //
				//	// Prepare data to send back to user interface
				//	// ***************************************** //
				//	DATA_CHANGE_TYPE type = DATA_CHANGE_TYPE__INPUT_MODEL__DMU_MEMBERS_CHANGE;
				//	DATA_CHANGE_INTENTION intention = DATA_CHANGE_INTENTION__REMOVE;
				//	DataChange change(type, intention, dmu_member, WidgetInstanceIdentifiers());
				//	change_response.changes.push_back(change);

			});

			//messager.EmitChangeMessage(change_response);

		}
			break;

		case WIDGET_ACTION_ITEM_REQUEST_REASON__UPDATE_ITEMS:
		{

		}
			break;

		default:
			break;

	}

}

void UIActionManager::RefreshDMUsFromFile(Messager & messager, WidgetActionItemRequest_ACTION_REFRESH_DMUS_FROM_FILE const & action_request, InputProject & project)
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

	InputModel & output_model = project.model();

	switch (action_request.reason)
	{
		case WIDGET_ACTION_ITEM_REQUEST_REASON__ADD_ITEMS:
		{

		}
			break;
		case WIDGET_ACTION_ITEM_REQUEST_REASON__REMOVE_ITEMS:
		{

		}
			break;
		case WIDGET_ACTION_ITEM_REQUEST_REASON__UPDATE_ITEMS:
		{

			DataChangeMessage change_response(&project);

			// ***************************************** //
			// Prepare data to send back to user interface
			// ***************************************** //
			DATA_CHANGE_TYPE type = DATA_CHANGE_TYPE__INPUT_MODEL__DMU_MEMBERS_CHANGE;
			DATA_CHANGE_INTENTION intention = DATA_CHANGE_INTENTION__ADD;
			WidgetInstanceIdentifiers child_identifiers;
			DataChange change(type, intention, WidgetInstanceIdentifier(), child_identifiers);
			change.SetPacket(std::make_shared<DataChangePacket_int>(77));
			change_response.changes.push_back(change);

			messager.EmitChangeMessage(change_response);

		}
			break;

		default:
			break;

	}

}
