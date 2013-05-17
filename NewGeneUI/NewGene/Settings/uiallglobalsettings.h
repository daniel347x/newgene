#ifndef UIGLOBALSETTINGS_H
#define UIGLOBALSETTINGS_H

#include <QObject>
#include "uiallsettings.h"
#include "../../../NewGeneBackEnd/Settings/GlobalSettings.h"

enum GLOBAL_SETTINGS_UI
{
	GLOBAL_SETTING_UI__LAST
};

class UIAllGlobalSettings : public UIAllSettings
{

		Q_OBJECT

	public:

		explicit UIAllGlobalSettings( QObject * parent = 0 );
		UIAllGlobalSettings(boost::filesystem::path const path_to_settings, QObject * parent = NULL);

		void ProvideDefaultSettings();


	signals:

	public slots:

	protected:

		class UIOnlySettings : public UIAllSettings::UIOnlySettings_base<GLOBAL_SETTINGS_UI, UIGlobalSetting>
		{

			public:

		};

		class _impl : public UIAllSettings::_impl_base
		{

			public:

				class _UIRelatedImpl : public _UIRelatedImpl_base
				{

					public:

						void CreateDefaultUISettings();
						void CreateUISettings(boost::filesystem::path const path_to_settings);

				};

				class _BackendRelatedImpl : public _BackendRelatedImpl_base<GlobalSettings>
				{

					public:

						void CreateDefaultBackendSettings();
						void CreateBackendSettings(boost::filesystem::path const path_to_settings);

				};

		};

		void CreateImplementation();

};

#endif // UIGLOBALSETTINGS_H
