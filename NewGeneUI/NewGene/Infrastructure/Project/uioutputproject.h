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
		UIOutputProject(UIMessager * messager,
						std::shared_ptr<UIOutputProjectSettings> const & project_settings,
						std::shared_ptr<UIOutputModelSettings> const & model_settings,
						std::shared_ptr<UIOutputModel> const & model,
						QObject * parent = NULL)
			: QObject(parent)
			, UIProject(messager, project_settings, model_settings, model)
		{

		}

	signals:

	public slots:

	protected:

		virtual WorkQueueManager<UI_OUTPUT_PROJECT> * InstantiateWorkQueue()
		{
			return new OutputProjectWorkQueue();
		}

};

#endif // UIOUTPUTPROJECT_H
