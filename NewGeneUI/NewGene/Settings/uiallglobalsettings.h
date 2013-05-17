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


	signals:

	public slots:

	protected:

		class UIOnlySettings : public UIOnlySettings_base<GLOBAL_SETTINGS_UI, UIGlobalSetting>
		{

			public:

				UIOnlySettings() : UIOnlySettings_base()
				{

				}

				UIOnlySettings(boost::filesystem::path const path_to_settings) : UIOnlySettings_base()
				{

				}

		};

		class _impl : public _impl_base<GlobalSettings, UIOnlySettings>
		{

			public:

				_impl() : _impl_base()
				{
					CreateInternalImplementations();
				}

				_impl(boost::filesystem::path const path_to_settings) : _impl_base()
				{
					CreateInternalImplementations(path_to_settings);
				}

				class _UIRelatedImpl : public _UIRelatedImpl_base
				{

					public:

						_UIRelatedImpl() : _UIRelatedImpl_base()
						{

						}

						_UIRelatedImpl(boost::filesystem::path const path_to_settings) : _UIRelatedImpl_base()
						{

						}

				};

				class _BackendRelatedImpl : public _BackendRelatedImpl_base
				{

					public:

						_BackendRelatedImpl() : _BackendRelatedImpl_base()
						{

						}

						_BackendRelatedImpl(boost::filesystem::path const path_to_settings) : _BackendRelatedImpl_base()
						{

						}

				};

			protected:

				void CreateInternalImplementations();
				void CreateInternalImplementations(boost::filesystem::path const path_to_settings)
;\
		};

		void CreateImplementation();
		void CreateImplementation(boost::filesystem::path const path_to_settings);
		std::unique_ptr<_impl> __impl;

};

#endif // UIGLOBALSETTINGS_H
