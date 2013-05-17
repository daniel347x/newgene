#ifndef UISETTINGS_H
#define UISETTINGS_H

#include <QObject>
#include "globals.h"
#include "../../../NewGeneBackEnd/Settings/Settings.h"
#include "uisetting.h"

class UIAllSettings : public QObject
{

		Q_OBJECT


	public:

		explicit UIAllSettings(QObject * parent = NULL);
		UIAllSettings(boost::filesystem::path const path_to_settings, QObject * parent = NULL);


	signals:

	public slots:


	protected:

		virtual void ProvideDefaultSettings() = 0;


	protected:

		class UIOnlySettings_base
		{

			protected:

		};

		class _impl_base
		{

			protected:

				class _UIRelatedImpl_base
				{

					protected:

						virtual void CreateDefaultUISettings() = 0;
						virtual void CreateUISettings(boost::filesystem::path const path_to_settings) = 0; // just overwrite whatever already exists in _ui_settings

						std::vector<UISetting> _ui_settings;

				};

				class _BackendRelatedImpl_base
				{

					protected:

						virtual void CreateDefaultBackendSettings() = 0;
						virtual void CreateBackendSettings(boost::filesystem::path const path_to_settings) = 0;

						// ***********************************************************************
						// The BACKEND possesses and maintains the
						// std::vector<BackendSetting> _backend_settings;
						// ***********************************************************************

				};

		};


	private:

		void init();

};

#endif // UISETTINGS_H
