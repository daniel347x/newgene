#ifndef UIINPUTPROJECT_H
#define UIINPUTPROJECT_H

#include "uiproject.h"
#include "../Settings/uiinputprojectsettings.h"
#include "uiinputmodel.h"

class UIInputProject : public QObject, public UIProject<InputProject, UIInputProjectSettings, UIInputModel>
{

		Q_OBJECT

	public:
		UIInputProject(UIMessager & messager, UIInputProjectSettings * settings, UIInputModel * model, QObject * parent = NULL)
			: QObject(parent)
			, UIProject(messager, settings, model)
		{

		}

	signals:

	public slots:

};

#endif // UIINPUTPROJECT_H
