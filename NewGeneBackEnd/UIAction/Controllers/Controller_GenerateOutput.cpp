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

					{
						std::lock_guard<std::recursive_mutex> guard(OutputModel::OutputGenerator::is_generating_output_mutex);

						if (OutputModel::OutputGenerator::is_generating_output)
						{
							messager__.ShowMessageBox("Another k-ad output generation operation is in progress.  Please wait for that operation to complete first.");
							return;
						}

						OutputModel::OutputGenerator::is_generating_output = true;
					}

					messager__.SetRunStatus(RUN_STATUS__RUNNING);

					bool cancelled { false };
					bool failed { false };
					bool markedAsDone { false };

					RunMetadata metadata;
					metadata.runIndex = 0;
					metadata.isGranular = true;
					metadata.rows = 0;
					bool doContinue { false };
					{
						OutputModel::OutputGenerator output_generator(messager__, output_model, project, OutputGeneratorMode::GATHER_TIME_RANGE);
						doContinue = SingleGeneratorRun(output_generator, &metadata);
					}

					if (metadata.isConsolidateRows || metadata.isRandomSampling || metadata.time_granularity == TIME_GRANULARITY::TIME_GRANULARITY__NONE)
					{
						// time-unit by time-unit runs are disabled for random sampling and consolidate rows mode
						metadata.isGranular = false;
					}

					std::int64_t rows = 0;

					if (doContinue)
					{
						if (metadata.isGranular)
						{
							// Loop through time units, performing one full run for that given time unit for each time unit
							TimeSlice fullTimeRange(metadata.timerange_start, metadata.timerange_end);
							int loopCount = 0;
							int highestLoopCount = 0;
							fullTimeRange.loop_through_time_units(metadata.time_granularity, boost::function<void(int64_t, int64_t)>([&highestLoopCount](std::int64_t, std::int64_t)
							{
								++highestLoopCount;
							}));

							fullTimeRange.loop_through_time_units(metadata.time_granularity, boost::function<void(int64_t, int64_t)>([&doContinue, &messager__, &output_model, &project, &metadata, &loopCount,
																  &highestLoopCount, &markedAsDone, &cancelled, &failed, &rows](std::int64_t start, std::int64_t end)
							{
								++loopCount;
								++metadata.runIndex;

								if (doContinue)
								{
									metadata.timerange_start = start;
									metadata.timerange_end = end;
									metadata.rows = 0;

									OutputModel::OutputGenerator output_generator(messager__, output_model, project, 0 /* mode - will be set below */);
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
									rows += metadata.rows;

									if (output_generator.failed)
									{
										failed = true;
										doContinue = false;
									}

									if (output_generator.CheckCancelled())
									{
										cancelled = true;
										doContinue = false;
									}

									if (output_generator.done && (output_generator.mode & OutputGeneratorMode::TAIL_RUN))
									{
										markedAsDone = true;
									}
								}
							}));

							if (!markedAsDone)
							{
								messager__.UpdateStatusBarText("", nullptr);
							}
						}
						else
						{
							// All data (i.e., over all days, months, years, etc.) is processed in a single run - all data, intermediate internal data, and results
							// must fit in RAM
							++metadata.runIndex; // unused, but if it is ever used in the future, this is a reminder to increment runIndex
							metadata.rows = 0;

							OutputModel::OutputGenerator output_generator(messager__, output_model, project, OutputGeneratorMode::HEADER_RUN | OutputGeneratorMode::TAIL_RUN);
							SingleGeneratorRun(output_generator, &metadata);
							rows += metadata.rows;

							if (output_generator.failed)
							{
								failed = true;
							}

							if (output_generator.CheckCancelled())
							{
								cancelled = true;
							}

							if (output_generator.done && (output_generator.mode & OutputGeneratorMode::TAIL_RUN))
							{
								markedAsDone = true;
							}
						}
					}

					messager__.AppendKadStatusText((boost::format("Wrote %1% total rows") % rows).str().c_str(), nullptr);

					if (failed)
					{
						messager__.AppendKadStatusText((boost::format("*****************************************************")).str().c_str(), nullptr);
						messager__.AppendKadStatusText((boost::format("*****************************************************")).str().c_str(), nullptr);
						messager__.AppendKadStatusText("Operation failed.", nullptr);
					}
					else if (cancelled)
					{
						messager__.AppendKadStatusText((boost::format("*****************************************************")).str().c_str(), nullptr);
						messager__.AppendKadStatusText((boost::format("*****************************************************")).str().c_str(), nullptr);
						messager__.AppendKadStatusText("Operation cancelled.", nullptr);
					}
					else if (!markedAsDone)
					{
						messager__.AppendKadStatusText((boost::format("*****************************************************")).str().c_str(), nullptr);
						messager__.AppendKadStatusText((boost::format("*****************************************************")).str().c_str(), nullptr);
						messager__.AppendKadStatusText("Operation did not complete.", nullptr);
					}
					else
					{
						if (metadata.isGranular)
						{
							messager__.AppendKadStatusText((boost::format("*****************************************************")).str().c_str(), nullptr);
							messager__.AppendKadStatusText((boost::format("*****************************************************")).str().c_str(), nullptr);
							messager__.AppendKadStatusText("k-ad routine is complete.", nullptr);
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

					OutputModel::OutputGenerator::is_generating_output = false;
					messager__.SetPerformanceLabel("");
					messager__.SetRunStatus(RUN_STATUS__NOT_RUNNING);
				});

				messager__.EmitChangeMessage(change_response);

			}

			break;

		default:
			break;
	}

}
