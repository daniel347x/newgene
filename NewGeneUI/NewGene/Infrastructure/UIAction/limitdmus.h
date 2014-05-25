#ifndef LIMITDMUS_H
#define LIMITDMUS_H

#include "../../../../NewGeneBackEnd/UIAction/ActionWidgets.h"
#include "../Project/inputprojectworkqueue.h"
#include "../Project/outputprojectworkqueue.h"
#include "../Messager/uimessager.h"
#include "../Messager/uimessagersingleshot.h"
#include "../UIAction/uiuiactionmanager.h"
#include "uiaction.h"
#include "../Project/uiinputproject.h"
#include "../Project/uioutputproject.h"

class LimitDMUs : public DoOutputAction<ACTION_LIMIT_DMU_MEMBERS_CHANGE>
{

    public:

        LimitDMUs(WidgetActionItemRequest_ACTION_LIMIT_DMU_MEMBERS_CHANGE & action_request_, OutputProjectWorkQueue * queue_)
            : DoOutputAction<ACTION_LIMIT_DMU_MEMBERS_CHANGE>(static_cast<WidgetActionItemRequest<ACTION_LIMIT_DMU_MEMBERS_CHANGE>>(action_request_), queue_)
        {

        }

        void operator()()
        {
            UIMessagerSingleShot messager(queue->get()->messager);
            uiactionManagerUI().getBackendManager().DoLimitDmusChange(messager.get(), action_request, queue->get()->backend());
        }

};

#endif // LIMITDMUS_H
