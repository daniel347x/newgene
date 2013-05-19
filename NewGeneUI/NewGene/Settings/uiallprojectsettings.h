#ifndef UIPROJECTSETTINGS_H
#define UIPROJECTSETTINGS_H

#include <QObject>
#include "../../../NewGeneBackEnd/Utilities/NewGeneException.h"
#include "uiallsettings.h"
#include "../../../NewGeneBackEnd/Settings/ProjectSettings.h"

namespace PROJECT_SETTINGS_UI_NAMESPACE
{

	enum PROJECT_SETTINGS_UI
	{
		  SETTING_FIRST = 0
		, SETTING_LAST
	};

}

template<>
SettingInfo GetSettingInfoFromEnum<PROJECT_SETTINGS_UI_NAMESPACE::PROJECT_SETTINGS_UI>(Messager & messager, PROJECT_SETTINGS_UI_NAMESPACE::PROJECT_SETTINGS_UI const value_);

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

				boost::filesystem::path GetSettingsPath(Messager & messager, SettingInfo & setting_info);
				void SetMapEntry(Messager & messager, SettingInfo & setting_info, boost::property_tree::ptree & pt);
				UIProjectSetting * CloneSetting(Messager & messager, UIProjectSetting * current_setting, SettingInfo & setting_info) const;
				UIProjectSetting * NewSetting(Messager & messager, SettingInfo & setting_info, void const * setting_value_void);

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

		_impl_base<ProjectSettings, UIOnlySettings> & getImplementation()
		{
			if (!__impl)
			{
				boost::format msg( "UI Project settings implementation not yet constructed." );
				throw NewGeneException() << newgene_error_description( msg.str() );
			}
			return *(__impl.get());
		}

		void CreateImplementation(Messager & messager);
		void CreateImplementation(Messager & messager, boost::filesystem::path const path_to_settings);
		std::unique_ptr<_impl> __impl;


	public:

		UIOnlySettings & getUISettings();
		ProjectSettings & getBackendSettings();

};

#endif // UIPROJECTSETTINGS_H
