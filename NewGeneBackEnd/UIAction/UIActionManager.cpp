#include "UIActionManager.h"

#include "../Project/InputProject.h"
#include "../Project/OutputProject.h"
#include "../UIData/DataChanges.h"
#include "../UIAction/ActionChanges.h"

/************************************************************************/
// ACTION_VARIABLE_GROUP_SET_MEMBER_SELECTION_CHANGED
/************************************************************************/
void UIActionManager::DoVariableGroupSetMemberSelectionChange(Messager & messager, WidgetActionItemRequest_ACTION_VARIABLE_GROUP_SET_MEMBER_SELECTION_CHANGED const & action_request, OutputProject & project)
{

	if (!action_request.items)
	{
		return;
	}
	
	OutputModel & output_model = project.model();
	InputModel & input_model = project.model().getInputModel();

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

			for_each(action_request.items->cbegin(), action_request.items->cend(), [&input_model, &output_model, &messager, &change_response](InstanceActionItem const & instanceActionItem)
			{
				if (!instanceActionItem.second)
				{
					return;
				}
				WidgetInstanceIdentifier const & identifier = instanceActionItem.first;
				if (!identifier.uuid)
				{
					return;
				}

				// ************************************* //
				// Retrieve data sent by user interface
				// ************************************* //
				WidgetActionItem const & actionItem = *instanceActionItem.second;
				WidgetActionItem__Checkbox const & actionItemCheckbox = static_cast<WidgetActionItem__Checkbox const &>(actionItem);
				UUID itemUUID = *identifier.uuid;
				std::string itemCode;
				if (identifier.code)
				{
					itemCode = *identifier.code;
				}
				bool checked = false;
				if (actionItemCheckbox.isChecked())
				{
					checked = true;
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
				DataChange change(type, intention, WidgetInstanceIdentifier(*identifier.uuid_parent), child_identifiers);
				change_response.changes.push_back(change);

				//boost::format msg("UUID = %1%, code = %2%, checked = %3%");
				//msg % itemUUID % itemCode % checked;
				//messager.ShowMessageBox(msg.str());
			});

			messager.EmitChangeMessage(change_response);

		}
		break;
	}

	//messager.EmitOutputWidgetDataRefresh(variable_groups);

	//messager.ShowMessageBox("Crazy crap");

}
