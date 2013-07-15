#ifndef KADCOUNTCHANGE_H
#define KADCOUNTCHANGE_H

#include "../../../../NewGeneBackEnd/UIAction/ActionWidgets.h"
#include "../Project/inputprojectworkqueue.h"
#include "../Project/outputprojectworkqueue.h"
#include "../Messager/uimessager.h"
#include "../Messager/uimessagersingleshot.h"
#include "../UIAction/uiuiactionmanager.h"
#include "uiaction.h"
#include "../Project/uiinputproject.h"
#include "../Project/uioutputproject.h"

class KAdCountChange : public DoOutputAction<ACTION_KAD_COUNT_CHANGE>
{

	public:

		KAdCountChange(WidgetActionItemRequest_ACTION_KAD_COUNT_CHANGE & action_request_, OutputProjectWorkQueue * queue_)
			: DoOutputAction<ACTION_KAD_COUNT_CHANGE>(static_cast<WidgetActionItemRequest<ACTION_KAD_COUNT_CHANGE>>(action_request_), queue_)
		{

		}

		void operator()()
		{
			UIMessagerSingleShot messager(queue->get()->messager);
			uiactionManagerUI().getBackendManager().DoKAdCountChange(messager.get(), action_request, queue->get()->backend());
		}

};

#endif // KADCOUNTCHANGE_H
