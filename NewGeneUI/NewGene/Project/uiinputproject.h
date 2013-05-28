#ifndef UIINPUTPROJECT_H
#define UIINPUTPROJECT_H

#include "uiproject.h"
#include "../Settings/uiinputprojectsettings.h"
#include "uiinputmodel.h"

class UIInputProject : public QObject, public UIProject<InputProject, UIInputProjectSettings, UIInputModel>
{

		Q_OBJECT

	public:
		UIInputProject(UIMessager & messager, boost::filesystem::path const path_to_settings = boost::filesystem::path(), QObject * parent = NULL)
			: QObject(parent)
			, UIProject(messager, path_to_settings)
		{

		}

	signals:

	public slots:

};

#endif // UIINPUTPROJECT_H
