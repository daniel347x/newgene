#ifndef WIDGETDATAREFRESH_H
#define WIDGETDATAREFRESH_H

#include "../../../../NewGeneBackEnd/UIData/DataWidgets.h"

class InputProjectWorkQueue;
class OutputProjectWorkQueue;

template<DATA_WIDGETS WIDGET>
void WidgetDataRefresh()
{}

class DoRefreshWidget
{

	public:

		DoRefreshWidget(DATA_WIDGETS const widget_)
			: widget(widget_)
		{

		}

		DoRefreshWidget(DoRefreshWidget const & rhs)
			: widget(rhs.widget)
		{

		}

		virtual void operator()() {};

	protected:

		DATA_WIDGETS const widget;


};

class DoRefreshInputWidget : public DoRefreshWidget
{

	public:

		DoRefreshInputWidget(DATA_WIDGETS const widget_, InputProjectWorkQueue * queue_)
			: DoRefreshWidget(widget_)
			, queue(queue_)
		{

		}

		DoRefreshInputWidget(DoRefreshInputWidget const & rhs)
			: DoRefreshWidget(rhs)
			, queue(rhs.queue)
		{

		}

		void operator()();

	protected:

		InputProjectWorkQueue * queue;

};

class DoRefreshOutputWidget : public DoRefreshWidget
{

	public:

		DoRefreshOutputWidget(DATA_WIDGETS const widget_, OutputProjectWorkQueue * queue_)
			: DoRefreshWidget(widget_)
			, queue(queue_)
		{

		}

		DoRefreshOutputWidget(DoRefreshOutputWidget const & rhs)
			: DoRefreshWidget(rhs)
			, queue(rhs.queue)
		{

		}

		void operator()();

	protected:

		OutputProjectWorkQueue * queue;

};

#endif // WIDGETDATAREFRESH_H
