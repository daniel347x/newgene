#ifndef VARIABLEGROUPSETMEMBERSELECTIONCHANGE_H
#define VARIABLEGROUPSETMEMBERSELECTIONCHANGE_H

#include "../../../../NewGeneBackEnd/UIAction/ActionWidgets.h"
#include "../Project/inputprojectworkqueue.h"
#include "../Project/outputprojectworkqueue.h"
#include "../Messager/uimessager.h"
#include "../Messager/uimessagersingleshot.h"
#include "../UIAction/uiuiactionmanager.h"
#include "uiaction.h"
#include "../Project/uiinputproject.h"
#include "../Project/uioutputproject.h"

class VariableGroupSetMemberSelectionChange : public DoOutputAction<ACTION_VARIABLE_GROUP_SET_MEMBER_SELECTION_CHANGED>
{

	public:

		VariableGroupSetMemberSelectionChange(WidgetActionItemRequest_ACTION_VARIABLE_GROUP_SET_MEMBER_SELECTION_CHANGED & action_request_, OutputProjectWorkQueue * queue_)
			: DoOutputAction<ACTION_VARIABLE_GROUP_SET_MEMBER_SELECTION_CHANGED>(static_cast<WidgetActionItemRequest<ACTION_VARIABLE_GROUP_SET_MEMBER_SELECTION_CHANGED>>(action_request_),
					queue_)
		{

		}

		void operator()()
		{
			UIMessagerSingleShot messager(queue->get()->messager);
			uiactionManagerUI().getBackendManager().DoVariableGroupSetMemberSelectionChange(messager.get(), action_request, queue->get()->backend());
		}

};

#endif // VARIABLEGROUPSETMEMBERSELECTIONCHANGE_H
