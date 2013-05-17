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

	signals:

	public slots:

	protected:

		Settings * CreateBackendSettings(boost::filesystem::path const path_to_settings);
		Settings * CreateDefaultBackendSettings();

};

#endif // UIGLOBALSETTINGS_H
