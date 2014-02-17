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

		}
			break;

		default:
			break;

	}

	DataChangeMessage change_response(&project);

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
