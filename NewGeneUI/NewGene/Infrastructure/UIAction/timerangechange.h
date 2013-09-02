#ifndef TIMERANGECHANGE_H
#define TIMERANGECHANGE_H

#include "../../../../NewGeneBackEnd/UIAction/ActionWidgets.h"
#include "../Project/inputprojectworkqueue.h"
#include "../Project/outputprojectworkqueue.h"
#include "../Messager/uimessager.h"
#include "../Messager/uimessagersingleshot.h"
#include "../UIAction/uiuiactionmanager.h"
#include "uiaction.h"
#include "../Project/uiinputproject.h"
#include "../Project/uioutputproject.h"

class TimeRangeChange : public DoOutputAction<ACTION_DATETIME_RANGE_CHANGE>
{

	public:

		TimeRangeChange(WidgetActionItemRequest_ACTION_DATETIME_RANGE_CHANGE & action_request_, OutputProjectWorkQueue * queue_)
			: DoOutputAction<ACTION_DATETIME_RANGE_CHANGE>(static_cast<WidgetActionItemRequest<ACTION_DATETIME_RANGE_CHANGE>>(action_request_), queue_)
		{

		}

		void operator()()
		{
			UIMessagerSingleShot messager(queue->get()->messager);
			uiactionManagerUI().getBackendManager().DoTimeRangeChange(messager.get(), action_request, queue->get()->backend());
		}

};

#endif // TIMERANGECHANGE_H
