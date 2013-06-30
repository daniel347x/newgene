#ifndef UIACTION_H
#define UIACTION_H

#include "../../../../NewGeneBackEnd/UIAction/ActionWidgets.h"
#include "../Project/inputprojectworkqueue.h"
#include "../Project/outputprojectworkqueue.h"
#include "../Messager/uimessager.h"
#include "../Messager/uimessagersingleshot.h"
#include "../UIAction/uiuiactionmanager.h"

template<WIDGET_ACTIONS WIDGET_ACTION>
class DoAction
{

	public:

		DoAction(WidgetActionItemRequest<WIDGET_ACTION> const & action_request_)
			: action_request(action_request_)
		{

		}

		DoAction(DoAction const & rhs)
			: action_request(rhs.action_request)
		{

		}

		virtual void operator()() {};

		WidgetActionItemRequest<WIDGET_ACTION> const action_request;


};

template<WIDGET_ACTIONS WIDGET_ACTION>
class DoInputAction : public DoAction<WIDGET_ACTION>
{

	public:

		DoInputAction(WidgetActionItemRequest<WIDGET_ACTION> const & action_request_, InputProjectWorkQueue * queue_)
			: DoAction<WIDGET_ACTION>(action_request_)
			, queue(queue_)
		{

		}

		DoInputAction(DoInputAction const & rhs)
			: DoAction<WIDGET_ACTION>(rhs)
			, queue(rhs.queue)
		{

		}

		void operator()()
		{

		}

	protected:

		InputProjectWorkQueue * queue;

};

template<WIDGET_ACTIONS WIDGET_ACTION>
class DoOutputAction : public DoAction<WIDGET_ACTION>
{

	public:

		DoOutputAction(WidgetActionItemRequest<WIDGET_ACTION> const & action_request_, OutputProjectWorkQueue * queue_)
			: DoAction<WIDGET_ACTION>(action_request_)
			, queue(queue_)
		{

		}

		DoOutputAction(DoOutputAction const & rhs)
			: DoAction<WIDGET_ACTION>(rhs)
			, queue(rhs.queue)
		{

		}

		void operator()()
		{

		}

	protected:

		OutputProjectWorkQueue * queue;

};


#endif // UIACTION_H
