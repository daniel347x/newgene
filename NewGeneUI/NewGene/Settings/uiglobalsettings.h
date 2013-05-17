#ifndef UIGLOBALSETTINGS_H
#define UIGLOBALSETTINGS_H

#include <QObject>
#include "uisettings.h"
#include "../../../NewGeneBackEnd/Settings/GlobalSettings.h"

class UIGlobalSettings : public UISettings
{

		Q_OBJECT

	public:

		explicit UIGlobalSettings( QObject * parent = 0 );

		virtual Settings * LoadBackendSettings();

	signals:

	public slots:

	protected:

		//virtual void CreateBackendSettings();

};

#endif // UIGLOBALSETTINGS_H
