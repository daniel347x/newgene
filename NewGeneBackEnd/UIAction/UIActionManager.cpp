#include "UIActionManager.h"

#include "../Project/InputProject.h"
#include "../Project/OutputProject.h"

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
			for_each(action_request.items->cbegin(), action_request.items->cend(), [&input_model, &output_model, &messager](InstanceActionItem const & instanceActionItem)
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
				WidgetActionItem const & actionItem = *instanceActionItem.second;
				WidgetActionItem__Checkbox const & actionItemCheckbox = static_cast<WidgetActionItem__Checkbox const &>(actionItem);
				UUID itemUUID = *identifier.uuid;
				std::string itemCode;
				if (identifier.code)
				{
					itemCode = *identifier.code;
				}
				boost::format msg("UUID = %1%, code = %2%");
				msg % itemUUID % itemCode;
				messager.ShowMessageBox(msg.str());
			});
		}
		break;
	}

	//messager.EmitOutputWidgetDataRefresh(variable_groups);

	//messager.ShowMessageBox("Crazy crap");

}
