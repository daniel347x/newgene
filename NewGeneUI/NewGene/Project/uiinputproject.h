#ifndef UIINPUTPROJECT_H
#define UIINPUTPROJECT_H

#include "uiproject.h"
#include "../Settings/allinputprojectsettings.h"
#include "uiinputmodel.h"

class UIInputProject : public QObject, public UIProject<InputProject, AllInputProjectSettings, UIInputModel>
{

		Q_OBJECT

	public:
		UIInputProject(Messager & messager, boost::filesystem::path const path_to_settings = boost::filesystem::path(), QObject * parent = NULL)
			: QObject(parent)
			, UIProject(messager, path_to_settings)
		{

		}

	signals:

	public slots:

};

#endif // UIINPUTPROJECT_H
