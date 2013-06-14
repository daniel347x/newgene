#ifndef UIINPUTPROJECT_H
#define UIINPUTPROJECT_H

#include "uiproject.h"
#include "../Settings/uiinputprojectsettings.h"
#include "../../../../NewGeneBackEnd/Settings/InputModelSettings.h"
#include "uiinputmodel.h"
#include "../Settings/uiinputmodelsettings.h"

class UIInputProject : public UIProject<InputProject, UIInputProjectSettings, UIInputModelSettings, UIInputModel>
{

		Q_OBJECT

	public:
		UIInputProject(UIMessager * messager,
					   std::shared_ptr<UIInputProjectSettings> const & project_settings,
					   std::shared_ptr<UIInputModelSettings> const & model_settings,
					   std::shared_ptr<UIInputModel> const & model,
					   QObject * parent = NULL)
			: UIProject(messager, project_settings, model_settings, model, parent)
		{

		}

	signals:

	public slots:

};

#endif // UIINPUTPROJECT_H
