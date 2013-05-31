#ifndef UIOUTPUTPROJECT_H
#define UIOUTPUTPROJECT_H

#include "uiproject.h"
#include "../Settings/uioutputprojectsettings.h"
#include "uioutputmodel.h"

class UIOutputProject : public QObject, public UIProject<OutputProject, UIOutputProjectSettings, UIOutputModel>
{

		Q_OBJECT

	public:
		UIOutputProject(UIMessager & messager, UIOutputProjectSettings * settings, UIOutputModel * model, QObject * parent = NULL)
			: QObject(parent)
			, UIProject(messager, settings, model)
		{

		}

	signals:

	public slots:

};

#endif // UIOUTPUTPROJECT_H
