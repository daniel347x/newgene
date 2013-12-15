#ifndef WIDGETDATAREFRESH_H
#define WIDGETDATAREFRESH_H

#include "../../../../NewGeneBackEnd/UIData/DataWidgets.h"
#include "../Project/inputprojectworkqueue.h"
#include "../Project/outputprojectworkqueue.h"
#include "../Messager/uimessager.h"
#include "../Messager/uimessagersingleshot.h"
#include "../UIData/uiuidatamanager.h"

template<DATA_WIDGETS DATA_WIDGET>
class DoRefreshWidget
{

	public:

		DoRefreshWidget(WidgetDataItemRequest<DATA_WIDGET> const & widget_refresh_request_)
			: widget_refresh_request(widget_refresh_request_)
		{

		}

		DoRefreshWidget(DoRefreshWidget const & rhs)
			: widget_refresh_request(rhs.widget_refresh_request)
		{

		}

		virtual void operator()() {};

		WidgetDataItemRequest<DATA_WIDGET> const widget_refresh_request;

};

template<DATA_WIDGETS DATA_WIDGET>
class DoRefreshInputWidget : public DoRefreshWidget<DATA_WIDGET>
{

	public:

		DoRefreshInputWidget(WidgetDataItemRequest<DATA_WIDGET> const & widget_refresh_request_, InputProjectWorkQueue * queue_)
			: DoRefreshWidget<DATA_WIDGET>(widget_refresh_request_)
			, queue(queue_)
		{

		}

		DoRefreshInputWidget(DoRefreshInputWidget const & rhs)
			: DoRefreshWidget<DATA_WIDGET>(rhs)
			, queue(rhs.queue)
		{

		}

		void operator()()
		{

		}

	protected:

		InputProjectWorkQueue * queue;

};

template<DATA_WIDGETS DATA_WIDGET>
class DoRefreshOutputWidget : public DoRefreshWidget<DATA_WIDGET>
{

	public:

		DoRefreshOutputWidget(WidgetDataItemRequest<DATA_WIDGET> const & widget_, OutputProjectWorkQueue * queue_)
			: DoRefreshWidget<DATA_WIDGET>(widget_)
			, queue(queue_)
		{

		}

		DoRefreshOutputWidget(DoRefreshOutputWidget const & rhs)
			: DoRefreshWidget<DATA_WIDGET>(rhs)
			, queue(rhs.queue)
		{

		}

		void operator()()
		{
			UIMessagerSingleShot messager(queue->get()->messager);
			uidataManagerUI().getBackendManager().DoRefreshOutputWidget(messager.get(), this->widget_refresh_request, queue->get()->backend());
		}

	protected:

		OutputProjectWorkQueue * queue;

};

#endif // WIDGETDATAREFRESH_H
