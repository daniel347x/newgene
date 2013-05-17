#ifndef UIPROJECTSETTINGS_H
#define UIPROJECTSETTINGS_H

#include <QObject>
#include "uiallsettings.h"
#include "../../../NewGeneBackEnd/Settings/ProjectSettings.h"

enum PROJECT_SETTINGS_UI
{
	PROJECT_SETTING_UI__LAST
};

class UIAllProjectSettings : public UIAllSettings
{

		Q_OBJECT

	public:

		explicit UIAllProjectSettings( QObject * parent = 0 );
		UIAllProjectSettings(boost::filesystem::path const path_to_settings, QObject * parent = NULL);

		void ProvideDefaultSettings();


	signals:

	public slots:

	protected:

		class UIOnlySettings : public UIAllSettings::UIOnlySettings_base
		{

			public:

		};

		class _impl : public UIAllSettings::_impl_base
		{

			public:

				class _UIRelatedImpl : public _UIRelatedImpl_base<PROJECT_SETTINGS_UI, UIProjectSetting>
				{

					public:

						void CreateDefaultUISettings();
						void CreateUISettings(boost::filesystem::path const path_to_settings);

				};

				class _BackendRelatedImpl : public _BackendRelatedImpl_base<ProjectSettings>
				{

					public:

						void CreateDefaultBackendSettings();
						void CreateBackendSettings(boost::filesystem::path const path_to_settings);

				};

		};

};

#endif // UIPROJECTSETTINGS_H
