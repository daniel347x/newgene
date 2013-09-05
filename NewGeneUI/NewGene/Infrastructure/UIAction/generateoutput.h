#ifndef GENERATEOUTPUT_H
#define GENERATEOUTPUT_H

#include "../../../../NewGeneBackEnd/UIAction/ActionWidgets.h"
#include "../Project/inputprojectworkqueue.h"
#include "../Project/outputprojectworkqueue.h"
#include "../Messager/uimessager.h"
#include "../Messager/uimessagersingleshot.h"
#include "../UIAction/uiuiactionmanager.h"
#include "uiaction.h"
#include "../Project/uiinputproject.h"
#include "../Project/uioutputproject.h"

class GenerateOutput : public DoOutputAction<ACTION_GENERATE_OUTPUT>
{

	public:

		GenerateOutput(WidgetActionItemRequest_ACTION_GENERATE_OUTPUT & action_request_, OutputProjectWorkQueue * queue_)
			: DoOutputAction<ACTION_GENERATE_OUTPUT>(static_cast<WidgetActionItemRequest<ACTION_GENERATE_OUTPUT>>(action_request_), queue_)
		{

		}

		void operator()()
		{
			UIMessagerSingleShot messager(queue->get()->messager);
			messager.setMode(UIMessager::KAD_GENERATION);
			uiactionManagerUI().getBackendManager().DoGenerateOutput(messager.get(), action_request, queue->get()->backend());
		}

};

#endif // GENERATEOUTPUT_H
