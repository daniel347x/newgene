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
				WidgetActionItem__String const & actionItemString = static_cast<WidgetActionItem__String const &>(actionItem);
				std::string proposed_new_dmu = actionItemString.getValue();

				bool dmu_already_exists = input_model.t_dmu_category.Exists(input_model.getDb(), input_model, proposed_new_dmu);
				if (dmu_already_exists)
				{
					boost::format msg("The DMU category '%1%' already exists.");
					msg % boost::to_upper_copy(proposed_new_dmu);
					messager.ShowMessageBox(msg.str());
					return;
				}

				// ***************************************** //
				// Prepare data to send back to user interface
				// ***************************************** //
				DATA_CHANGE_TYPE type = DATA_CHANGE_TYPE__OUTPUT_MODEL__VG_CATEGORY_SET_MEMBER_SELECTION;
				DATA_CHANGE_INTENTION intention = DATA_CHANGE_INTENTION__ADD;
				if (!checked)
				{
					intention = DATA_CHANGE_INTENTION__REMOVE;
				}
				WidgetInstanceIdentifiers child_identifiers;
				child_identifiers.push_back(identifier);
				DataChange change(type, intention, WidgetInstanceIdentifier(*identifier.identifier_parent), child_identifiers);
				change_response.changes.push_back(change);

				// ***************************************** //
				// Update database and cache
				// ***************************************** //
				output_model.t_variables_selected_identifiers.Update(output_model.getDb(), output_model, input_model, change_response);

				// ***************************************** //
				// Use updated cache info to set further info
				// in change_response
				// ***************************************** //
				std::set<WidgetInstanceIdentifier> active_dmus = output_model.t_variables_selected_identifiers.GetActiveDMUs(&output_model, &input_model);
				change_response.changes.back().set_of_identifiers = active_dmus;

			});

			// ***************************************** //
			// Prepare data to send back to user interface
			// ***************************************** //
			DATA_CHANGE_TYPE type = DATA_CHANGE_TYPE__INPUT_MODEL__DMU_CHANGE;
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
			DATA_CHANGE_TYPE type = DATA_CHANGE_TYPE__INPUT_MODEL__DMU_CHANGE;
			DATA_CHANGE_INTENTION intention = DATA_CHANGE_INTENTION__REMOVE;
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
			DATA_CHANGE_INTENTION intention = DATA_CHANGE_INTENTION__REMOVE;
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
