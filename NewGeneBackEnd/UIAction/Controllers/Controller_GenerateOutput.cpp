#include "../UIActionManager.h"

#include "../../Project/InputProject.h"
#include "../../Project/OutputProject.h"
#include "../../UIData/DataChanges.h"
#include "../../UIAction/ActionChanges.h"
#ifndef Q_MOC_RUN
#	include <boost/scope_exit.hpp>
#endif

/************************************************************************/
// ACTION_GENERATE_OUTPUT
/************************************************************************/
void UIActionManager::DoGenerateOutput(Messager & messager, WidgetActionItemRequest_ACTION_GENERATE_OUTPUT const & action_request, OutputProject & project)
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
	case WIDGET_ACTION_ITEM_REQUEST_REASON__DO_ACTION:
		{

			DataChangeMessage change_response(&project);

			for_each(action_request.items->cbegin(), action_request.items->cend(), [&input_model, &output_model, &messager, &project, &change_response](InstanceActionItem const & instanceActionItem)
			{
				if (!instanceActionItem.second)
				{
					return;
				}

				// ************************************* //
				// Retrieve data sent by user interface
				// ************************************* //
				WidgetActionItem const & actionItem = *instanceActionItem.second;
				WidgetActionItem__GenerateOutput const & actionItemGenerateOutput = static_cast<WidgetActionItem__GenerateOutput const &>(actionItem);

				// ***************************************** //
				// Prepare data to send back to user interface
				// ***************************************** //
				DATA_CHANGE_TYPE type = DATA_CHANGE_TYPE__OUTPUT_MODEL__GENERATE_OUTPUT;
				DATA_CHANGE_INTENTION intention = DATA_CHANGE_INTENTION__NONE;
				DataChange change(type, intention);
				change.SetPacket(std::make_shared<DataChangePacket_GenerateOutput>());
				change_response.changes.push_back(change);

				bool & is_cancelled = OutputModel::OutputGenerator::cancelled;
				BOOST_SCOPE_EXIT(&is_cancelled)
				{
					is_cancelled = false;
				} BOOST_SCOPE_EXIT_END

				// ***************************************** //
				// Generate output
				// ***************************************** //
				OutputModel::OutputGenerator output_generator(messager, output_model, project);
				output_generator.GenerateOutput(change_response);

				if (OutputModel::OutputGenerator::cancelled)
				{
					output_generator.messager.AppendKadStatusText("Operation cancelled.");
				}

#				if 0
				boost::format msg1("Number transactions begun: %1%");
				msg1 % OutputModel::OutputGenerator::number_transaction_begins;
				boost::format msg2("Number transactions ended: %1%");
				msg2 % OutputModel::OutputGenerator::number_transaction_ends;
				boost::format msg3("Number statements prepared: %1%");
				msg3 % OutputModel::OutputGenerator::SQLExecutor::number_statement_prepares;
				boost::format msg4("Number statements finalized: %1%");
				msg4 % OutputModel::OutputGenerator::SQLExecutor::number_statement_finalizes;

				messager.AppendKadStatusText(msg1.str());
				messager.AppendKadStatusText(msg2.str());
				messager.AppendKadStatusText(msg3.str());
				messager.AppendKadStatusText(msg4.str());
#				endif

			});

			messager.EmitChangeMessage(change_response);

		}
		break;
	}

}
