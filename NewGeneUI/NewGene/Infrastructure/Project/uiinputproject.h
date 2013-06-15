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
		UIInputProject(UIMessager * messager,
					   std::shared_ptr<UIInputProjectSettings> const & project_settings,
					   std::shared_ptr<UIInputModelSettings> const & model_settings,
					   std::shared_ptr<UIInputModel> const & model,
					   QObject * parent = NULL)
			: QObject(parent)
			, UIProject(messager, project_settings, model_settings, model)
		{

		}

	signals:

	public slots:

	protected:

		virtual WorkQueueManager<UI_INPUT_PROJECT> * InstantiateWorkQueue()
		{
			return new InputProjectWorkQueue();
		}

};

#endif // UIINPUTPROJECT_H
