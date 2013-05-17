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

		class UIOnlySettings_base
		{

			protected:

		};

		class _impl_base
		{

			protected:

				template<typename SETTINGS_ENUM, typename SETTING_CLASS>
				class _UIRelatedImpl_base : public SettingsRepository<SETTINGS_ENUM, SETTING_CLASS>
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
						// The BACKEND possesses and maintains the
						// std::vector<BackendSetting> _backend_settings;
						// ***********************************************************************
						std::unique_ptr<BACKEND_SETTINGS_CLASS> _backend_settings;

				};

		};


	private:

		void init();

};

#endif // UISETTINGS_H
