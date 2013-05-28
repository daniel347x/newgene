#ifndef UIOUTPUTPROJECT_H
#define UIOUTPUTPROJECT_H

#include "uiproject.h"
#include "../Settings/uioutputprojectsettings.h"
#include "uioutputmodel.h"

class UIOutputProject : public QObject, public UIProject<OutputProject, UIOutputProjectSettings, UIOutputModel>
{

		Q_OBJECT

	public:
		UIOutputProject(Messager & messager, boost::filesystem::path const path_to_settings = boost::filesystem::path(), QObject * parent = NULL)
			: QObject(parent)
			, UIProject(messager, path_to_settings)
		{

		}

	signals:

	public slots:

};

#endif // UIOUTPUTPROJECT_H
