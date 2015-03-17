#include "../UIActionManager.h"

#ifndef Q_MOC_RUN
	#include <boost/scope_exit.hpp>
#endif
#include "../../Project/InputProject.h"
#include "../../Project/OutputProject.h"
#include "../../UIData/DataChanges.h"
#include "../../UIAction/ActionChanges.h"

/************************************************************************/
// ACTION_GENERATE_OUTPUT
/************************************************************************/
void UIActionManager::DoGenerateOutput(Messager & messager__, WidgetActionItemRequest_ACTION_GENERATE_OUTPUT const & action_request, OutputProject & project)
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
		return;
	}

	OutputModel & output_model = project.model();
	InputModel & input_model = project.model().getInputModel();

	switch (action_request.reason)
	{
		case WIDGET_ACTION_ITEM_REQUEST_REASON__DO_ACTION:
			{

				DataChangeMessage change_response(&project);

				bool & is_cancelled = OutputModel::OutputGenerator::cancelled;
				BOOST_SCOPE_EXIT(&is_cancelled)
				{
					is_cancelled = false;
				} BOOST_SCOPE_EXIT_END

				for_each(action_request.items->cbegin(), action_request.items->cend(), [&input_model, &output_model, &messager__, &project,
						 &change_response](InstanceActionItem const & instanceActionItem)
				{

					if (!instanceActionItem.second)
					{
						return;
					}

					// ************************************* //
					// Retrieve data sent by user interface
					// ************************************* //
					//WidgetActionItem const & actionItem = *instanceActionItem.second;
					//WidgetActionItem__GenerateOutput const & actionItemGenerateOutput = static_cast<WidgetActionItem__GenerateOutput const &>(actionItem);

					// ***************************************** //
					// Prepare data to send back to user interface
					// ***************************************** //
					DATA_CHANGE_TYPE type = DATA_CHANGE_TYPE__OUTPUT_MODEL__GENERATE_OUTPUT;
					DATA_CHANGE_INTENTION intention = DATA_CHANGE_INTENTION__NONE;
					DataChange change(type, intention);
					change.SetPacket(std::make_shared<DataChangePacket_GenerateOutput>());
					change_response.changes.push_back(change);

					// ***************************************** //
					// Generate output
					// ***************************************** //
					OutputModel::OutputGenerator output_generator(messager__, output_model, project);

					try
					{
						output_generator.GenerateOutput(change_response);
					}
					catch (boost::exception & e)
					{
						if (std::string const * error_desc = boost::get_error_info<newgene_error_description>(e))
						{
							boost::format msg(error_desc->c_str());
							messager__.AppendKadStatusText(msg.str().c_str(), &output_generator);
						}
						else
						{
							std::string the_error = boost::diagnostic_information(e);
							boost::format msg("Error: %1%");
							msg % the_error.c_str();
							messager__.AppendKadStatusText(msg.str().c_str(), &output_generator);
						}
					}
					catch (std::exception & e)
					{
						boost::format msg("Exception thrown: %1%");
						msg % e.what();
						messager__.AppendKadStatusText(msg.str().c_str(), &output_generator);
					}

					if (!output_generator.done)
					{
						messager__.UpdateStatusBarText("", &output_generator);

						if (OutputModel::OutputGenerator::cancelled)
						{
							messager__.AppendKadStatusText("Operation cancelled.", &output_generator);
						}
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

				messager__.EmitChangeMessage(change_response);

			}
			break;

		default:
			break;
	}

}
