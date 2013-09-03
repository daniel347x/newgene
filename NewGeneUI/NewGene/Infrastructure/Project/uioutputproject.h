#ifndef UIOUTPUTPROJECT_H
#define UIOUTPUTPROJECT_H

#include "uiproject.h"
#include "../Settings/uioutputprojectsettings.h"
#include "../../../../NewGeneBackEnd/Settings/InputModelSettings.h"
#include "../../../../NewGeneBackEnd/Settings/OutputModelSettings.h"
#include "uioutputmodel.h"
#include <memory>
#include "../Settings/uioutputmodelsettings.h"
#include "outputprojectworkqueue.h"

class UIOutputProject : public QObject, public UIProject<OutputProject, UIOutputProjectSettings, UIOutputModelSettings, UIOutputModel, UI_OUTPUT_PROJECT>
{

		Q_OBJECT

	public:
		UIOutputProject(std::shared_ptr<UIOutputProjectSettings> const & project_settings,
						std::shared_ptr<UIOutputModelSettings> const & model_settings,
						std::shared_ptr<UIOutputModel> const & model,
						QObject * parent = NULL)
			: QObject(parent)
			, UIProject(project_settings, model_settings, model)
			, messager(this)
			, number_timerange_widgets_created(0)
		{

		}

		void UpdateConnections();
		void DoRefreshAllWidgets();
		void PassChangeMessageToWidget(NewGeneWidget * widget, DataChangeMessage const & change_message);

	signals:
		void RefreshAllWidgets();

	public slots:
		void DataChangeMessageSlot(WidgetChangeMessages);
		void SignalMessageBox(STD_STRING);
		bool QuestionMessageBox(STD_STRING);

	public:

		UIMessagerOutputProject messager;

		bool is_model_equivalent(UIMessager & messager, UIOutputModel * model);

		int number_timerange_widgets_created;

	protected:

		WorkQueueManager<UI_OUTPUT_PROJECT> * InstantiateWorkQueue(void * ui_object, bool isPool2_ = false)
		{
			OutputProjectWorkQueue * work_queue = new OutputProjectWorkQueue();
			work_queue->SetUIObject(reinterpret_cast<UIOutputProject*>(ui_object));
			work_queue->SetConnections();
			return work_queue;
		}

};

#endif // UIOUTPUTPROJECT_H
