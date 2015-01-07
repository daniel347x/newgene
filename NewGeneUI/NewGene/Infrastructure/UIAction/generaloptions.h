#ifndef GENERALOPTIONS_2_H
#define GENERALOPTIONS_2_H

#include "../../../../NewGeneBackEnd/UIAction/ActionWidgets.h"
#include "../Project/inputprojectworkqueue.h"
#include "../Project/outputprojectworkqueue.h"
#include "../Messager/uimessager.h"
#include "../Messager/uimessagersingleshot.h"
#include "../UIAction/uiuiactionmanager.h"
#include "uiaction.h"
#include "../Project/uiinputproject.h"
#include "../Project/uioutputproject.h"

class DoRandomSamplingChange : public DoOutputAction<ACTION_DO_RANDOM_SAMPLING_CHANGE>
{

	public:

		DoRandomSamplingChange(WidgetActionItemRequest_ACTION_DO_RANDOM_SAMPLING_CHANGE & action_request_, OutputProjectWorkQueue * queue_)
			: DoOutputAction<ACTION_DO_RANDOM_SAMPLING_CHANGE>(static_cast<WidgetActionItemRequest<ACTION_DO_RANDOM_SAMPLING_CHANGE>>(action_request_), queue_)
		{

		}

		void operator()()
		{
			UIMessagerSingleShot messager(queue->get()->messager);
			uiactionManagerUI().getBackendManager().DoDoRandomSamplingChange(messager.get(), action_request, queue->get()->backend());
		}

};

class KadSamplerCountPerStageChange : public DoOutputAction<ACTION_RANDOM_SAMPLING_COUNT_PER_STAGE_CHANGE>
{

	public:

		KadSamplerCountPerStageChange(WidgetActionItemRequest_ACTION_RANDOM_SAMPLING_COUNT_PER_STAGE_CHANGE & action_request_, OutputProjectWorkQueue * queue_)
			: DoOutputAction<ACTION_RANDOM_SAMPLING_COUNT_PER_STAGE_CHANGE>(static_cast<WidgetActionItemRequest<ACTION_RANDOM_SAMPLING_COUNT_PER_STAGE_CHANGE>>(action_request_), queue_)
		{

		}

		void operator()()
		{
			UIMessagerSingleShot messager(queue->get()->messager);
			uiactionManagerUI().getBackendManager().DoRandomSamplingCountPerStageChange(messager.get(), action_request, queue->get()->backend());
		}

};

class DoConsolidateRowsChange : public DoOutputAction<ACTION_CONSOLIDATE_ROWS_CHANGE>
{

	public:

		DoConsolidateRowsChange(WidgetActionItemRequest_ACTION_CONSOLIDATE_ROWS_CHANGE & action_request_, OutputProjectWorkQueue * queue_)
			: DoOutputAction<ACTION_CONSOLIDATE_ROWS_CHANGE>(static_cast<WidgetActionItemRequest<ACTION_CONSOLIDATE_ROWS_CHANGE>>(action_request_), queue_)
		{

		}

		void operator()()
		{
			UIMessagerSingleShot messager(queue->get()->messager);
			uiactionManagerUI().getBackendManager().DoConsolidateRowsChange(messager.get(), action_request, queue->get()->backend());
		}

};

class DoDisplayAbsoluteTimeColumnsChange : public DoOutputAction<ACTION_DISPLAY_ABSOLUTE_TIME_COLUMNS_CHANGE>
{

	public:

		DoDisplayAbsoluteTimeColumnsChange(WidgetActionItemRequest_ACTION_DISPLAY_ABSOLUTE_TIME_COLUMNS_CHANGE & action_request_, OutputProjectWorkQueue * queue_)
			: DoOutputAction<ACTION_DISPLAY_ABSOLUTE_TIME_COLUMNS_CHANGE>(static_cast<WidgetActionItemRequest<ACTION_DISPLAY_ABSOLUTE_TIME_COLUMNS_CHANGE>>(action_request_), queue_)
		{

		}

		void operator()()
		{
			UIMessagerSingleShot messager(queue->get()->messager);
			uiactionManagerUI().getBackendManager().DoDisplayAbsoluteTimeColumnsChange(messager.get(), action_request, queue->get()->backend());
		}

};

#endif // GENERALOPTIONS_H
