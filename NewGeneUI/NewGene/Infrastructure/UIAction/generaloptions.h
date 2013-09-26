#ifndef GENERALOPTIONS_H
#define GENERALOPTIONS_H

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

class RandomSamplingCountPerStageChange : public DoOutputAction<ACTION_RANDOM_SAMPLING_COUNT_PER_STAGE_CHANGE>
{

	public:

		RandomSamplingCountPerStageChange(WidgetActionItemRequest_ACTION_DO_RANDOM_SAMPLING_CHANGE & action_request_, OutputProjectWorkQueue * queue_)
			: DoOutputAction<ACTION_RANDOM_SAMPLING_COUNT_PER_STAGE_CHANGE>(static_cast<WidgetActionItemRequest<ACTION_RANDOM_SAMPLING_COUNT_PER_STAGE_CHANGE>>(action_request_), queue_)
		{

		}

		void operator()()
		{
			UIMessagerSingleShot messager(queue->get()->messager);
			uiactionManagerUI().getBackendManager().DoRandomSamplingCountPerStageChange(messager.get(), action_request, queue->get()->backend());
		}

};

#endif // GENERALOPTIONS_H
