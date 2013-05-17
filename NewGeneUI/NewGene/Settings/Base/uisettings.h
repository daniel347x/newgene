#ifndef UISETTINGS_H
#define UISETTINGS_H

#include <QObject>
#include "globals.h"
#include "../../../NewGeneBackEnd/Settings/Settings.h"

class UISettings : public QObject
{

		Q_OBJECT

	public:

		Settings * getBackendSettings()
		{
			return backend_settings.get();
		}

		void resetBackendSettings(Settings * backend_settings_)
		{
			backend_settings.reset(backend_settings_);
		}

	signals:

	public slots:

	protected:

		virtual Settings * CreateBackendSettings(boost::filesystem::path const path_to_settings) = 0; // from file
		virtual Settings * CreateDefaultBackendSettings() = 0; // default

		std::unique_ptr<Settings> backend_settings;


};

#endif // UISETTINGS_H
