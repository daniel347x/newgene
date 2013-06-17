#ifndef OUTPUTPROJECTWORKQUEUE_H
#define OUTPUTPROJECTWORKQUEUE_H

#include "Base/outputprojectworkqueue_base.h"

class UIOutputProject;

class OutputProjectWorkQueue : public WorkQueueManager<UI_OUTPUT_PROJECT>
{

	public:

		explicit OutputProjectWorkQueue(QObject * parent = NULL);

		void SetUIObject(void * ui_output_object_)
		{
			outp = ui_output_object_;
		}

		void SetConnections();

		void EmitMessage(std::string msg);
		void EmitOutputWidgetDataRefresh(WidgetDataItem_VARIABLE_GROUPS_SCROLL_AREA & widgetData)
		{
			emit WidgetDataRefresh(widgetData);
		}
		void EmitOutputWidgetDataRefresh(WidgetDataItem_VARIABLE_GROUPS_TOOLBOX & widgetData)
		{
			emit WidgetDataRefresh(widgetData);
		}

		UIOutputProject * get();

	private:

		void * outp;

	protected:

	// ********************************* //
	// Slot Overrides
	// ********************************* //

		void TestSlot();
		void RefreshWidget(DATA_WIDGETS);

};

#endif // OUTPUTPROJECTWORKQUEUE_H
