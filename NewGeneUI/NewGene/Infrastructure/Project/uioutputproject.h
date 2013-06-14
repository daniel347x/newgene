#ifndef UIOUTPUTPROJECT_H
#define UIOUTPUTPROJECT_H

#include "uiproject.h"
#include "../Settings/uioutputprojectsettings.h"
#include "../../../../NewGeneBackEnd/Settings/InputModelSettings.h"
#include "../../../../NewGeneBackEnd/Settings/OutputModelSettings.h"
#include "uioutputmodel.h"
#include <memory>

class UIOutputProject : public QObject, public UIProject<OutputProject, UIOutputProjectSettings, OutputModelSettings, UIOutputModel>
{

		Q_OBJECT

	public:
		UIOutputProject(UIMessager * messager, std::shared_ptr<UIOutputProjectSettings> const & project_settings,
											   std::shared_ptr<UIOutputModel> const & model,
											   std::shared_ptr<OutputModelSettings> const & model_settings, QObject * parent = NULL)
			: QObject(parent)
			, UIProject(messager, project_settings, model_settings, model)
		{

		}

	signals:

	public slots:

	protected:

};

#endif // UIOUTPUTPROJECT_H
