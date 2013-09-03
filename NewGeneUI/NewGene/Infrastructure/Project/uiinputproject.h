#ifndef UIINPUTPROJECT_H
#define UIINPUTPROJECT_H

#include "uiproject.h"
#include "../Settings/uiinputprojectsettings.h"
#include "../../../../NewGeneBackEnd/Settings/InputModelSettings.h"
#include "uiinputmodel.h"
#include "../Settings/uiinputmodelsettings.h"
#include "inputprojectworkqueue.h"

class UIInputProject : public QObject, public UIProject<InputProject, UIInputProjectSettings, UIInputModelSettings, UIInputModel, UI_INPUT_PROJECT>
{

		Q_OBJECT

	public:
		UIInputProject(std::shared_ptr<UIInputProjectSettings> const & project_settings,
					   std::shared_ptr<UIInputModelSettings> const & model_settings,
					   std::shared_ptr<UIInputModel> const & model,
					   QObject * parent = NULL)
			: QObject(parent)
			, UIProject(project_settings, model_settings, model)
			, messager(this)
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

		UIMessagerInputProject messager;

		bool is_model_equivalent(UIMessager & messager, UIInputModel * model);

	protected:

		WorkQueueManager<UI_INPUT_PROJECT> * InstantiateWorkQueue(void * ui_object, bool isPool2_ = false)
		{
			InputProjectWorkQueue * work_queue = new InputProjectWorkQueue();
			work_queue->SetUIObject(reinterpret_cast<UIInputProject*>(ui_object));
			work_queue->SetConnections();
			return work_queue;
		}

};

#endif // UIINPUTPROJECT_H
