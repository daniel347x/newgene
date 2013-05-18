#ifndef UIPROJECTSETTINGS_H
#define UIPROJECTSETTINGS_H

#include <QObject>
#include "uiallsettings.h"
#include "../../../NewGeneBackEnd/Settings/ProjectSettings.h"

namespace PROJECT_SETTINGS_UI_NAMESPACE
{

	enum PROJECT_SETTINGS_UI
	{
		SETTING_LAST
	};

}

class UIAllProjectSettings : public UIAllSettings
{

		Q_OBJECT

	public:

		explicit UIAllProjectSettings(Messager & messager, QObject * parent = NULL);
		UIAllProjectSettings(Messager & messager, boost::filesystem::path const path_to_settings, QObject * parent = NULL);


	signals:

	public slots:

	protected:

		class UIOnlySettings : public UIOnlySettings_base<PROJECT_SETTINGS_UI_NAMESPACE::PROJECT_SETTINGS_UI, UIProjectSetting>
		{

			public:

				UIOnlySettings(Messager & messager) : UIOnlySettings_base(messager)
				{

				}

				UIOnlySettings(Messager & messager, boost::filesystem::path const path_to_settings) : UIOnlySettings_base(messager, path_to_settings)
				{

				}

			protected:

				void LoadDefaultSettings(Messager & messager);

		};

		class _impl : public _impl_base<ProjectSettings, UIOnlySettings>
		{

			public:

				_impl(Messager & messager) : _impl_base(messager)
				{
					CreateInternalImplementations(messager);
				}

				_impl(Messager & messager, boost::filesystem::path const path_to_settings) : _impl_base(messager)
				{
					CreateInternalImplementations(messager, path_to_settings);
				}

				class _UIRelatedImpl : public _UIRelatedImpl_base
				{

					public:

						_UIRelatedImpl(Messager & messager) : _UIRelatedImpl_base(messager)
						{

						}

						_UIRelatedImpl(Messager & messager, boost::filesystem::path const path_to_settings) : _UIRelatedImpl_base(messager, path_to_settings)
						{

						}

				};

				class _BackendRelatedImpl : public _BackendRelatedImpl_base
				{

					public:

						_BackendRelatedImpl(Messager & messager) : _BackendRelatedImpl_base(messager)
						{

						}

						_BackendRelatedImpl(Messager & messager, boost::filesystem::path const path_to_settings) : _BackendRelatedImpl_base(messager, path_to_settings)
						{

						}

				};

			protected:

				void CreateInternalImplementations(Messager & messager);
				void CreateInternalImplementations(Messager & messager, boost::filesystem::path const path_to_settings);

		};

		void CreateImplementation(Messager & messager);
		void CreateImplementation(Messager & messager, boost::filesystem::path const path_to_settings);
		std::unique_ptr<_impl> __impl;

};

#endif // UIPROJECTSETTINGS_H
