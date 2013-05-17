#ifndef UISETTINGS_H
#define UISETTINGS_H

#include <QObject>
#include "globals.h"
#include "../../../NewGeneBackEnd/Settings/Settings.h"
#include "../../../NewGeneBackEnd/Settings/SettingsRepository.h"
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

		template<typename SETTINGS_ENUM, typename SETTING_CLASS>
		class UIOnlySettings_base : public SettingsRepository<SETTINGS_ENUM, SETTING_CLASS>
		{

				// ***********************************************************************
				// Directory derive from SettingsRepository.
				// Therefore, we ourselves (through this base class)
				// maintain the UI-related settings.
				//
				// (Unlike the backend-related settings implementation, below,
				// which simply holds a pointer to a backend settings instance,
				// and it is the backend settings instance which derives from
				// SettingsRepository.)
				// ***********************************************************************

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

				};

				template<typename BACKEND_SETTINGS_CLASS>
				class _BackendRelatedImpl_base
				{

					protected:

						virtual void CreateDefaultBackendSettings() = 0;
						virtual void CreateBackendSettings(boost::filesystem::path const path_to_settings) = 0;

						// ***********************************************************************
						// The BACKEND settings class possesses and maintains the
						// std::map<BACKEND_ENUM, BackendSetting> _backend_settings;
						// ... it does so by deriving from SettingsRepository.
						//
						// We simply own a pointer to the backend-related settings class.
						// ***********************************************************************
						std::unique_ptr<BACKEND_SETTINGS_CLASS> _backend_settings;

				};

		};


	private:

		void init();

};

#endif // UISETTINGS_H
