#ifndef UIPROJECTSETTINGS_H
#define UIPROJECTSETTINGS_H

#include <QObject>
#include "uisettings.h"
#include "../../../NewGeneBackEnd/Settings/ProjectSettings.h"

class UIProjectSettings : public UISettings
{

		Q_OBJECT

	public:

		explicit UIProjectSettings( QObject * parent = 0 );

		virtual Settings * LoadBackendSettings();

	signals:

	public slots:

	protected:

		//virtual void CreateBackendSettings();

};

#endif // UIPROJECTSETTINGS_H
