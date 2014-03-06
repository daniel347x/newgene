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

class NewGeneGenerateOutput;
class UIInputProject;

class UIOutputProject : public QObject, public UIProject<OutputProject, UIOutputProjectSettings, UIOutputModelSettings, UIOutputModel, UI_OUTPUT_PROJECT>
{

		Q_OBJECT

	public:
		UIOutputProject(std::shared_ptr<UIOutputProjectSettings> const & project_settings,
						std::shared_ptr<UIOutputModelSettings> const & model_settings,
						std::shared_ptr<UIOutputModel> const & model,
						QObject * mainWindowObject_,
						QObject * parent,
						UIMessagerOutputProject & messager_,
						UIInputProject * inp);

		void UpdateConnections();
		void DoRefreshAllWidgets();
		void PassChangeMessageToWidget(NewGeneWidget * widget, DataChangeMessage const & change_message);

	signals:
		void RefreshAllWidgets();

	public slots:
		void DataChangeMessageSlot(WidgetChangeMessages);
		void SignalMessageBox(STD_STRING);
		bool QuestionMessageBox(STD_STRING, STD_STRING); // title, question text

	public:

		QObject * mainWindowObject;
		UIMessagerOutputProject & messager;

		bool is_model_equivalent(UIMessager & messager, UIOutputModel * model);

		int number_timerange_widgets_created;

		NewGeneGenerateOutput * output_pane;

		UIInputProject * getUIInputProject()
		{
			return _inp;
		}

		void setUIInputProject(UIInputProject * inp)
		{
			_inp = inp;
		}

	protected:

		WorkQueueManager<UI_OUTPUT_PROJECT> * InstantiateWorkQueue(void * ui_object, bool = false)
		{
			OutputProjectWorkQueue * work_queue = new OutputProjectWorkQueue();
			work_queue->SetUIObject(reinterpret_cast<UIOutputProject*>(ui_object));
			work_queue->SetConnections();
			return work_queue;
		}

		UIInputProject * _inp;

};

#endif // UIOUTPUTPROJECT_H
