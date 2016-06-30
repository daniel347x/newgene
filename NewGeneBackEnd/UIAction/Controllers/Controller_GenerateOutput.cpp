#include "../UIActionManager.h"

#ifndef Q_MOC_RUN
	#include <boost/scope_exit.hpp>
#endif
#include "../../Project/InputProject.h"
#include "../../Project/OutputProject.h"
#include "../../UIData/DataChanges.h"
#include "../../UIAction/ActionChanges.h"

bool SingleGeneratorRun(OutputModel::OutputGenerator & generator, RunMetadata * pMetadata = nullptr)
{
	bool retVal = true;

	try
	{
		DataChangeMessage dummy;
		generator.GenerateOutput(dummy, pMetadata);
	}
	catch (boost::exception & e)
	{
		if (std::string const * error_desc = boost::get_error_info<newgene_error_description>(e))
		{
			boost::format msg(error_desc->c_str());
			generator.messager.AppendKadStatusText(msg.str().c_str(), &generator);
		}
		else
		{
			std::string the_error = boost::diagnostic_information(e);
			boost::format msg("Error: %1%");
			msg % the_error.c_str();
			generator.messager.AppendKadStatusText(msg.str().c_str(), &generator);
		}

		retVal = false;
	}
	catch (std::exception & e)
	{
		boost::format msg("Exception thrown: %1%");
		msg % e.what();
		generator.messager.AppendKadStatusText(msg.str().c_str(), &generator);
		retVal = false;
	}

	if (!generator.done)
	{
		if (generator.mode & OutputGeneratorMode::TAIL_RUN)
		{
			generator.messager.UpdateStatusBarText("", &generator);

			if (OutputModel::OutputGenerator::cancelled)
			{
				generator.messager.AppendKadStatusText("Operation cancelled.", &generator);
			}

			retVal = false;
		}
	}

	return retVal;
}

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

					bool granular_mode { true };

					if (granular_mode)
					{
						// Perform multiple runs, once per time unit, closing the file and clearing all memory between runs
						// ... to optimize, so that memory does not run out for long stretches of time (i.e., many days, months, years, etc.)

						// First - a run in 'gather time range' mode

						// You must modify the generator code to support this.  Return both the time range, the time granularity,
						// AND whether random sampling or consolidate rows is selected..  If so, switch to full mode.

						// Second - split the time range into units

						// Third - run a loop over the time range units

						// No need to modify the generator code for this

						// In this loop the first received 'head', the last receives 'tail', and any that are not first and not last receive 'middle';
						// set the time range values temporarily to those in the single time range, and then call the generator

						// Fourth, go through the generator code and modify text spit out to file/screen to match the mode being run

						// Also add new log/screen comments identifying whether 'time granular optimized' mode is being run, or regular mode

						bool doContinue = false;
						RunMetadata metadata;
						metadata.runIndex = 0;
						{
							OutputModel::OutputGenerator output_generator(messager__, output_model, project, OutputGeneratorMode::GATHER_TIME_RANGE);
							doContinue = SingleGeneratorRun(output_generator, &metadata);
						}

						if (doContinue)
						{
							if (metadata.isConsolidateRows || metadata.isRandomSampling || metadata.time_granularity == TIME_GRANULARITY::TIME_GRANULARITY__NONE)
							{
								// time-unit by time-unit runs are disabled for random sampling and consolidate rows mode
								granular_mode = false;
							}
							else
							{
								// Loop through time units, performing one full run for that given time unit for each time unit
								RunMetadata origMetadata = metadata;
								TimeSlice fullTimeRange(metadata.timerange_start, metadata.timerange_end);
								int loopCount = 0;
								int highestLoopCount = 0;
								fullTimeRange.loop_through_time_units(metadata.time_granularity, boost::function<void(int64_t, int64_t)>([&highestLoopCount](std::int64_t, std::int64_t)
								{
									++highestLoopCount;
								}));

								fullTimeRange.loop_through_time_units(metadata.time_granularity, boost::function<void(int64_t, int64_t)>([&doContinue, &messager__, &output_model, &project, &metadata, &loopCount,
																	  &highestLoopCount](std::int64_t start, std::int64_t end)
								{
									++loopCount;
									++metadata.runIndex;

									if (doContinue)
									{
										metadata.timerange_start = start;
										metadata.timerange_end = end;

										OutputModel::OutputGenerator output_generator(messager__, output_model, project, 0);
										output_generator.mode = 0;

										if (loopCount == 1)
										{
											output_generator.mode |= OutputGeneratorMode::HEADER_RUN;
										}

										if (loopCount == highestLoopCount)
										{
											output_generator.mode |= OutputGeneratorMode::TAIL_RUN;
										}

										if (output_generator.mode == 0)
										{
											output_generator.mode = OutputGeneratorMode::MIDDLE_RUN;
										}

										doContinue = SingleGeneratorRun(output_generator, &metadata);
									}
								}));
							}
						}
					}

					if (!granular_mode)
					{

						// Ignore metadata!  This is just a test run to make certain is_generating_output is not set!
						RunMetadata metadata;
						OutputModel::OutputGenerator output_generator(messager__, output_model, project, OutputGeneratorMode::GATHER_TIME_RANGE);
						bool doContinue = SingleGeneratorRun(output_generator, &metadata);

						if (doContinue)
						{
							// All data (i.e., over all days, months, years, etc.) is processed in a single run - all data, intermediate internal data, and results
							// must fit in RAM
							SingleGeneratorRun(OutputModel::OutputGenerator(messager__, output_model, project, OutputGeneratorMode::HEADER_RUN | OutputGeneratorMode::TAIL_RUN));
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
