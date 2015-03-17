#include "../UIActionManager.h"

#ifndef Q_MOC_RUN
	#include <boost/scope_exit.hpp>
#endif
#include "../../Project/InputProject.h"
#include "../../Project/OutputProject.h"
#include "../../UIData/DataChanges.h"
#include "../../UIAction/ActionChanges.h"

/************************************************************************/
// ACTION_VARIABLE_GROUP_SET_MEMBER_SELECTION_CHANGED
/************************************************************************/
void UIActionManager::DoVariableGroupSetMemberSelectionChange(Messager & messager,
		WidgetActionItemRequest_ACTION_VARIABLE_GROUP_SET_MEMBER_SELECTION_CHANGED const & action_request, OutputProject & project)
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

					Executor executor(input_model.getDb());

					if (!instanceActionItem.second)
					{
						return;
					}

					// identifier is the VG_SET_MEMBER
					// its parent is the VG_CATEGORY identifier
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
					NewGeneUUID itemUUID = *identifier.uuid;
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

					executor.success();

				});

				messager.EmitChangeMessage(change_response);

			}
			break;

		default:
			break;
	}

}
