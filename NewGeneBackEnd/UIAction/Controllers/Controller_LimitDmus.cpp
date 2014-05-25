#include "../UIActionManager.h"

#include "../../Project/InputProject.h"
#include "../../Project/OutputProject.h"
#include "../../UIData/DataChanges.h"
#include "../../UIAction/ActionChanges.h"
#ifndef Q_MOC_RUN
#	include <boost/scope_exit.hpp>
#endif

#include <algorithm>
#include <iterator>

/************************************************************************/
// ACTION_KAD_COUNT_CHANGE
/************************************************************************/
void UIActionManager::DoLimitDmusChange(Messager & messager, WidgetActionItemRequest_ACTION_LIMIT_DMU_MEMBERS_CHANGE const & action_request, OutputProject & project)
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

				DataChangeMessage change_response(&project);

				for (auto const & instanceActionItem : *action_request.items)
				{

					Executor executor(input_model.getDb());

					if (!instanceActionItem.second)
					{
						return;
					}

					WidgetInstanceIdentifier const & dmu_category = instanceActionItem.first;

					if (!dmu_category.uuid)
					{
						return;
					}

					// ************************************* //
					// Retrieve data sent by user interface
					// ************************************* //
					WidgetActionItem const & actionItem = *instanceActionItem.second;
					WidgetActionItem__WidgetInstanceIdentifiers_Plus_String const & actionItemLimitDmuInfo = static_cast<WidgetActionItem__WidgetInstanceIdentifiers_Plus_String const &>(actionItem);
					WidgetInstanceIdentifiers dmu_set_members__to_add = actionItemLimitDmuInfo.getValue();

					// ***************************************** //
					// Prepare data to send back to user interface
					// ***************************************** //
					DATA_CHANGE_TYPE type = DATA_CHANGE_TYPE__OUTPUT_MODEL__LIMIT_DMUS_CHANGE;
					DATA_CHANGE_INTENTION intention = DATA_CHANGE_INTENTION__RESET_ALL;

					WidgetInstanceIdentifiers dmu_set_members__all = input_model.t_dmu_setmembers.getIdentifiers(*dmu_category.uuid);
					WidgetInstanceIdentifiers dmu_set_members__current_limited = output_model.t_limit_dmus_set_members.getIdentifiers(*dmu_category.code);
					std::sort(dmu_set_members__all.begin(), dmu_set_members__all.end());
					std::sort(dmu_set_members__current_limited.begin(), dmu_set_members__current_limited.end());
					std::sort(dmu_set_members__to_add.begin(), dmu_set_members__to_add.end());

					WidgetInstanceIdentifiers dmu_set_members__new_limited;
					std::set_union(dmu_set_members__current_limited.cbegin(), dmu_set_members__current_limited.cend(), dmu_set_members__to_add.cbegin(), dmu_set_members__to_add.cend(),
								   std::inserter(dmu_set_members__new_limited, dmu_set_members__new_limited.begin()));

					WidgetInstanceIdentifiers dmu_set_members__not_limited;
					std::set_difference(dmu_set_members__all.cbegin(), dmu_set_members__all.cend(), dmu_set_members__new_limited.cbegin(), dmu_set_members__new_limited.cend(),
										std::inserter(dmu_set_members__not_limited, dmu_set_members__not_limited.begin()));

					DataChange change(type, intention, dmu_category, dmu_set_members__not_limited);
					change.vector_of_identifiers = dmu_set_members__new_limited;
					change_response.changes.push_back(change);

					// ***************************************** //
					// Update database and cache
					// ***************************************** //
					for (auto const & dmu_set_member_to_add : dmu_set_members__to_add)
					{
						output_model.t_limit_dmus_set_members.AddDmuMember(output_model.getDb(), output_model, input_model, dmu_category, *dmu_set_member_to_add.uuid);
					}

					executor.success();

				}

				messager.EmitChangeMessage(change_response);

			}
			break;

		case WIDGET_ACTION_ITEM_REQUEST_REASON__REMOVE_ITEMS:
			{

				DataChangeMessage change_response(&project);

				for (auto const & instanceActionItem : *action_request.items)
				{

					Executor executor(input_model.getDb());

					if (!instanceActionItem.second)
					{
						return;
					}

					WidgetInstanceIdentifier const & dmu_category = instanceActionItem.first;

					if (!dmu_category.uuid)
					{
						return;
					}

					// ************************************* //
					// Retrieve data sent by user interface
					// ************************************* //
					WidgetActionItem const & actionItem = *instanceActionItem.second;
					WidgetActionItem__WidgetInstanceIdentifiers_Plus_String const & actionItemLimitDmuInfo = static_cast<WidgetActionItem__WidgetInstanceIdentifiers_Plus_String const &>(actionItem);
					WidgetInstanceIdentifiers dmu_set_members__to_remove = actionItemLimitDmuInfo.getValue();

					// ***************************************** //
					// Prepare data to send back to user interface
					// ***************************************** //
					DATA_CHANGE_TYPE type = DATA_CHANGE_TYPE__OUTPUT_MODEL__LIMIT_DMUS_CHANGE;
					DATA_CHANGE_INTENTION intention = DATA_CHANGE_INTENTION__RESET_ALL;

					WidgetInstanceIdentifiers dmu_set_members__all = input_model.t_dmu_setmembers.getIdentifiers(*dmu_category.uuid);
					WidgetInstanceIdentifiers dmu_set_members__current_limited = output_model.t_limit_dmus_set_members.getIdentifiers(*dmu_category.code);
					std::sort(dmu_set_members__all.begin(), dmu_set_members__all.end());
					std::sort(dmu_set_members__current_limited.begin(), dmu_set_members__current_limited.end());
					std::sort(dmu_set_members__to_remove.begin(), dmu_set_members__to_remove.end());

					WidgetInstanceIdentifiers dmu_set_members__new_limited;
					std::set_difference(dmu_set_members__current_limited.cbegin(), dmu_set_members__current_limited.cend(), dmu_set_members__to_remove.cbegin(), dmu_set_members__to_remove.cend(),
								   std::inserter(dmu_set_members__new_limited, dmu_set_members__new_limited.begin()));

					WidgetInstanceIdentifiers dmu_set_members__not_limited;
					std::set_difference(dmu_set_members__all.cbegin(), dmu_set_members__all.cend(), dmu_set_members__new_limited.cbegin(), dmu_set_members__new_limited.cend(),
										std::inserter(dmu_set_members__not_limited, dmu_set_members__not_limited.begin()));

					DataChange change(type, intention, dmu_category, dmu_set_members__not_limited);
					change.vector_of_identifiers = dmu_set_members__new_limited;
					change_response.changes.push_back(change);

					// ***************************************** //
					// Update database and cache
					// ***************************************** //
					for (auto const & dmu_set_member_to_add : dmu_set_members__to_remove)
					{
						output_model.t_limit_dmus_set_members.AddDmuMember(output_model.getDb(), output_model, input_model, dmu_category, *dmu_set_member_to_add.uuid);
					}

					executor.success();

				}

				messager.EmitChangeMessage(change_response);

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
