#ifndef VGMANAGEMENT_H
#define VGMANAGEMENT_H

#include "../../../../NewGeneBackEnd/UIAction/ActionWidgets.h"
#include "../Project/inputprojectworkqueue.h"
#include "../Project/outputprojectworkqueue.h"
#include "../Messager/uimessager.h"
#include "../Messager/uimessagersingleshot.h"
#include "../UIAction/uiuiactionmanager.h"
#include "uiaction.h"
#include "../Project/uiinputproject.h"
#include "../Project/uioutputproject.h"

class CreateVG_ : public DoInputAction<ACTION_CREATE_VG>
{

	public:

		CreateVG_(WidgetActionItemRequest_ACTION_CREATE_VG & action_request_, InputProjectWorkQueue * queue_)
			: DoInputAction<ACTION_CREATE_VG>(static_cast<WidgetActionItemRequest_ACTION_CREATE_VG>(action_request_), queue_)
		{

		}

		void operator()()
		{
			UIMessagerSingleShot messager(queue->get()->messager);
			uiactionManagerUI().getBackendManager().CreateVG(messager.get(), action_request, queue->get()->backend());
		}

};

class DeleteVG_ : public DoInputAction<ACTION_DELETE_VG>
{

	public:

		DeleteVG_(WidgetActionItemRequest_ACTION_DELETE_VG & action_request_, InputProjectWorkQueue * queue_)
			: DoInputAction<ACTION_DELETE_VG>(static_cast<WidgetActionItemRequest_ACTION_DELETE_VG>(action_request_), queue_)
		{

		}

		void operator()()
		{
			UIMessagerSingleShot messager(queue->get()->messager);
			uiactionManagerUI().getBackendManager().DeleteVG(messager.get(), action_request, queue->get()->backend());
		}

};

class RefreshVG_ : public DoInputAction<ACTION_REFRESH_VG>
{

	public:

		RefreshVG_(WidgetActionItemRequest_ACTION_REFRESH_VG & action_request_, InputProjectWorkQueue * queue_)
			: DoInputAction<ACTION_REFRESH_VG>(static_cast<WidgetActionItemRequest_ACTION_REFRESH_VG>(action_request_), queue_)
		{

		}

		void operator()()
		{
			UIMessagerSingleShot messager(queue->get()->messager);
			uiactionManagerUI().getBackendManager().RefreshVG(messager.get(), action_request, queue->get()->backend());
		}

};

class DeleteVG_Output : public DoOutputAction<ACTION_DELETE_VG>
{

	public:

		DeleteVG_Output(WidgetActionItemRequest_ACTION_DELETE_VG & action_request_, OutputProjectWorkQueue * queue_)
			: DoOutputAction<ACTION_DELETE_VG>(static_cast<WidgetActionItemRequest_ACTION_DELETE_VG>(action_request_), queue_)
		{

		}

		void operator()()
		{
			UIMessagerSingleShot messager(queue->get()->messager);
			uiactionManagerUI().getBackendManager().DeleteVGOutput(messager.get(), action_request, queue->get()->backend());
		}

};

#endif // VGMANAGEMENT_H
