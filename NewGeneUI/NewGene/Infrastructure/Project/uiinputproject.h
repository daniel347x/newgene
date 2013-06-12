#ifndef UIINPUTPROJECT_H
#define UIINPUTPROJECT_H

#include "uiproject.h"
#include "../Settings/uiinputprojectsettings.h"
#include "../../../../NewGeneBackEnd/Settings/OutputModelSettings.h"
#include "uiinputmodel.h"

class UIInputProject : public QObject, public UIProject<InputProject, UIInputProjectSettings, InputModelSettings, UIInputModel>
{

		Q_OBJECT

	public:
		UIInputProject(UIMessager * messager, std::shared_ptr<UIInputProjectSettings> const & project_settings,
											  std::shared_ptr<UIInputModel> const & model,
											  std::shared_ptr<InputModelSettings> const & model_settings, QObject * parent = NULL)
			: QObject(parent)
			, UIProject(messager, project_settings, model_settings, model)
		{

		}

	signals:

	public slots:

};

#endif // UIINPUTPROJECT_H
