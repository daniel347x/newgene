#ifndef UOAMANAGEMENT_H
#define UOAMANAGEMENT_H

#include "../../../../NewGeneBackEnd/UIAction/ActionWidgets.h"
#include "../Project/inputprojectworkqueue.h"
#include "../Project/outputprojectworkqueue.h"
#include "../Messager/uimessager.h"
#include "../Messager/uimessagersingleshot.h"
#include "../UIAction/uiuiactionmanager.h"
#include "uiaction.h"
#include "../Project/uiinputproject.h"
#include "../Project/uioutputproject.h"

class AddUOA_ : public DoInputAction<ACTION_ADD_UOA>
{

	public:

		AddUOA_(WidgetActionItemRequest_ACTION_ADD_UOA & action_request_, InputProjectWorkQueue * queue_)
			: DoInputAction<ACTION_ADD_UOA>(static_cast<WidgetActionItemRequest_ACTION_ADD_UOA>(action_request_), queue_)
		{

		}

		void operator()()
		{
			UIMessagerSingleShot messager(queue->get()->messager);
			uiactionManagerUI().getBackendManager().AddUOA(messager.get(), action_request, queue->get()->backend());
		}

};

class DeleteUOA_ : public DoInputAction<ACTION_DELETE_UOA>
{

	public:

		DeleteUOA_(WidgetActionItemRequest_ACTION_DELETE_UOA & action_request_, InputProjectWorkQueue * queue_)
			: DoInputAction<ACTION_DELETE_UOA>(static_cast<WidgetActionItemRequest_ACTION_DELETE_UOA>(action_request_), queue_)
		{

		}

		void operator()()
		{
			UIMessagerSingleShot messager(queue->get()->messager);
			uiactionManagerUI().getBackendManager().DeleteUOA(messager.get(), action_request, queue->get()->backend());
		}

};

class DeleteUOA_Output : public DoOutputAction<ACTION_DELETE_UOA>
{

	public:

		DeleteUOA_Output(WidgetActionItemRequest_ACTION_DELETE_UOA & action_request_, OutputProjectWorkQueue * queue_)
			: DoOutputAction<ACTION_DELETE_UOA>(static_cast<WidgetActionItemRequest_ACTION_DELETE_UOA>(action_request_), queue_)
		{

		}

		void operator()()
		{
			UIMessagerSingleShot messager(queue->get()->messager);
			uiactionManagerUI().getBackendManager().DeleteUOAOutput(messager.get(), action_request, queue->get()->backend());
		}

};

#endif // UOAMANAGEMENT_H
