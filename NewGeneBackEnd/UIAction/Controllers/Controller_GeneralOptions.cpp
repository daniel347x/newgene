#include "../UIActionManager.h"

#ifndef Q_MOC_RUN
#	include <boost/scope_exit.hpp>
#endif
#include "../../Project/InputProject.h"
#include "../../Project/OutputProject.h"
#include "../../UIData/DataChanges.h"
#include "../../UIAction/ActionChanges.h"

/************************************************************************/
// ACTION_DO_RANDOM_SAMPLING_CHANGE
/************************************************************************/
void UIActionManager::DoDoRandomSamplingChange(Messager & messager, WidgetActionItemRequest_ACTION_DO_RANDOM_SAMPLING_CHANGE const & action_request, OutputProject & project)
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

					// ************************************* //
					// Retrieve data sent by user interface
					// ************************************* //
					WidgetActionItem const & actionItem = *instanceActionItem.second;
					WidgetActionItem__Checkbox const & actionItemCheckbox = static_cast<WidgetActionItem__Checkbox const &>(actionItem);

					// ***************************************** //
					// Prepare data to send back to user interface
					// ***************************************** //
					DATA_CHANGE_TYPE type = DATA_CHANGE_TYPE__OUTPUT_MODEL__DO_RANDOM_SAMPLING_CHANGE;
					DATA_CHANGE_INTENTION intention = DATA_CHANGE_INTENTION__UPDATE;
					DataChange change(type, intention);
					change.SetPacket(std::make_shared<DataChangePacket_bool>(actionItemCheckbox.isChecked()));
					change_response.changes.push_back(change);

					// ***************************************** //
					// Update database and cache
					// ***************************************** //
					output_model.t_general_options.UpdateDoRandomSampling(output_model.getDb(), output_model, input_model, change_response);

					executor.success();

				});

				messager.EmitChangeMessage(change_response);

			}
			break;
		}

}

/************************************************************************/
// ACTION_RANDOM_SAMPLING_COUNT_PER_STAGE_CHANGE
/************************************************************************/
void UIActionManager::DoRandomSamplingCountPerStageChange(Messager & messager, WidgetActionItemRequest_ACTION_RANDOM_SAMPLING_COUNT_PER_STAGE_CHANGE const & action_request, OutputProject & project)
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

					// ************************************* //
					// Retrieve data sent by user interface
					// ************************************* //
					WidgetActionItem const & actionItem = *instanceActionItem.second;
					WidgetActionItem__Int64 const & actionItemInt64 = static_cast<WidgetActionItem__Int64 const &>(actionItem);

					// ***************************************** //
					// Prepare data to send back to user interface
					// ***************************************** //
					DATA_CHANGE_TYPE type = DATA_CHANGE_TYPE__OUTPUT_MODEL__RANDOM_SAMPLING_COUNT_PER_STAGE_CHANGE;
					DATA_CHANGE_INTENTION intention = DATA_CHANGE_INTENTION__UPDATE;
					DataChange change(type, intention);
					change.SetPacket(std::make_shared<DataChangePacket_int64>(actionItemInt64.getValue()));
					change_response.changes.push_back(change);

					// ***************************************** //
					// Update database and cache
					// ***************************************** //
					output_model.t_general_options.UpdateKadSamplerCountPerStage(output_model.getDb(), output_model, input_model, change_response);

					executor.success();

				});

				messager.EmitChangeMessage(change_response);

			}
			break;
		}

}

/************************************************************************/
// ACTION_CONSOLIDATE_ROWS_CHANGE
/************************************************************************/
void UIActionManager::DoConsolidateRowsChange(Messager & messager, WidgetActionItemRequest_ACTION_CONSOLIDATE_ROWS_CHANGE const & action_request, OutputProject & project)
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

					// ************************************* //
					// Retrieve data sent by user interface
					// ************************************* //
					WidgetActionItem const & actionItem = *instanceActionItem.second;
					WidgetActionItem__Checkbox const & actionItemCheckbox = static_cast<WidgetActionItem__Checkbox const &>(actionItem);

					// ***************************************** //
					// Prepare data to send back to user interface
					// ***************************************** //
					DATA_CHANGE_TYPE type = DATA_CHANGE_TYPE__OUTPUT_MODEL__CONSOLIDATE_ROWS_CHANGE;
					DATA_CHANGE_INTENTION intention = DATA_CHANGE_INTENTION__UPDATE;
					DataChange change(type, intention);
					change.SetPacket(std::make_shared<DataChangePacket_bool>(actionItemCheckbox.isChecked()));
					change_response.changes.push_back(change);

					// ***************************************** //
					// Update database and cache
					// ***************************************** //
					output_model.t_general_options.UpdateConsolidateRows(output_model.getDb(), output_model, input_model, change_response);

					executor.success();

				});

				messager.EmitChangeMessage(change_response);

			}
			break;
		}

}

/************************************************************************/
// ACTION_DISPLAY_ABSOLUTE_TIME_COLUMNS_CHANGE
/************************************************************************/
void UIActionManager::DoDisplayAbsoluteTimeColumnsChange(Messager & messager, WidgetActionItemRequest_ACTION_DISPLAY_ABSOLUTE_TIME_COLUMNS_CHANGE const & action_request, OutputProject & project)
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

					// ************************************* //
					// Retrieve data sent by user interface
					// ************************************* //
					WidgetActionItem const & actionItem = *instanceActionItem.second;
					WidgetActionItem__Checkbox const & actionItemCheckbox = static_cast<WidgetActionItem__Checkbox const &>(actionItem);

					// ***************************************** //
					// Prepare data to send back to user interface
					// ***************************************** //
					DATA_CHANGE_TYPE type = DATA_CHANGE_TYPE__OUTPUT_MODEL__DISPLAY_ABSOLUTE_TIME_COLUMNS;
					DATA_CHANGE_INTENTION intention = DATA_CHANGE_INTENTION__UPDATE;
					DataChange change(type, intention);
					change.SetPacket(std::make_shared<DataChangePacket_bool>(actionItemCheckbox.isChecked()));
					change_response.changes.push_back(change);

					// ***************************************** //
					// Update database and cache
					// ***************************************** //
					output_model.t_general_options.UpdateDisplayAbsoluteTimeColumns(output_model.getDb(), output_model, input_model, change_response);

					executor.success();

				});

				messager.EmitChangeMessage(change_response);

			}
			break;
		}

}
