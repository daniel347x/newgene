#ifndef DMUMANAGEMENT_H
#define DMUMANAGEMENT_H

#include "../../../../NewGeneBackEnd/UIAction/ActionWidgets.h"
#include "../Project/inputprojectworkqueue.h"
#include "../Project/outputprojectworkqueue.h"
#include "../Messager/uimessager.h"
#include "../Messager/uimessagersingleshot.h"
#include "../UIAction/uiuiactionmanager.h"
#include "uiaction.h"
#include "../Project/uiinputproject.h"
#include "../Project/uioutputproject.h"

class AddDMU_ : public DoInputAction<>
{

	public:

		AddDMU_(WidgetActionItemRequest_ACTION_ADD_DMU & action_request_, InputProjectWorkQueue * queue_)
			: DoOutputAction<>(static_cast<WidgetActionItemRequest_ACTION_ADD_DMU>(action_request_), queue_)
		{

		}

		void operator()()
		{
			UIMessagerSingleShot messager(queue->get()->messager);
			uiactionManagerUI().getBackendManager().AddDMU(messager.get(), action_request, queue->get()->backend());
		}

};

class DeleteDMU_ : public DoInputAction<>
{

	public:

		DeleteDMU_(WidgetActionItemRequest_ACTION_DELETE_DMU & action_request_, InputProjectWorkQueue * queue_)
			: DoOutputAction<>(static_cast<WidgetActionItemRequest_ACTION_DELETE_DMU>(action_request_), queue_)
		{

		}

		void operator()()
		{
			UIMessagerSingleShot messager(queue->get()->messager);
			uiactionManagerUI().getBackendManager().DeleteDMU(messager.get(), action_request, queue->get()->backend());
		}

};

class AddDMUMembers_ : public DoInputAction<>
{

	public:

		AddDMUMembers_(WidgetActionItemRequest_ACTION_ADD_DMU_MEMBERS & action_request_, InputProjectWorkQueue * queue_)
			: DoOutputAction<>(static_cast<WidgetActionItemRequest_ACTION_ADD_DMU_MEMBERS>(action_request_), queue_)
		{

		}

		void operator()()
		{
			UIMessagerSingleShot messager(queue->get()->messager);
			uiactionManagerUI().getBackendManager().AddDMUMembers(messager.get(), action_request, queue->get()->backend());
		}

};

class DeleteDMUMembers_ : public DoInputAction<>
{

	public:

		DeleteDMUMembers_(WidgetActionItemRequest_ACTION_DELETE_DMU_MEMBERS & action_request_, InputProjectWorkQueue * queue_)
			: DoOutputAction<>(static_cast<WidgetActionItemRequest_ACTION_DELETE_DMU_MEMBERS>(action_request_), queue_)
		{

		}

		void operator()()
		{
			UIMessagerSingleShot messager(queue->get()->messager);
			uiactionManagerUI().getBackendManager().DeleteDMUMembers(messager.get(), action_request, queue->get()->backend());
		}

};

class RefreshDMUsFromFile_ : public DoInputAction<>
{

	public:

		RefreshDMUsFromFile_(WidgetActionItemRequest_ACTION_REFRESH_DMUS_FROM_FILE & action_request_, InputProjectWorkQueue * queue_)
			: DoOutputAction<>(static_cast<WidgetActionItemRequest_ACTION_REFRESH_DMUS_FROM_FILE>(action_request_), queue_)
		{

		}

		void operator()()
		{
			UIMessagerSingleShot messager(queue->get()->messager);
			uiactionManagerUI().getBackendManager().RefreshDMUsFromFile(messager.get(), action_request, queue->get()->backend());
		}

};


#endif // DMUMANAGEMENT_H
