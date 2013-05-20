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

		UIAllProjectSettings(Messager & messager, Project & project, boost::filesystem::path const path_to_settings = boost::filesystem::path(), QObject * parent = NULL);


	signals:

	public slots:

	protected:

		// The following is the equivalent of the backend's ProjectSettings class
		class UIOnlySettings : public UIOnlySettings_base<PROJECT_SETTINGS_UI_NAMESPACE::PROJECT_SETTINGS_UI, UIProjectSetting>
		{

			public:

				UIOnlySettings(Messager & messager, boost::filesystem::path const path_to_settings = boost::filesystem::path()) : UIOnlySettings_base(messager, path_to_settings)
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

				_impl(Messager & messager, Project & project, boost::filesystem::path const path_to_settings = boost::filesystem::path()) : _impl_base(messager)
				{
					CreateInternalImplementations(messager, project, path_to_settings);
				}

				class _UIRelatedImpl : public _UIProjectRelatedImpl_base
				{

					public:

						_UIRelatedImpl(Messager & messager, Project & project, boost::filesystem::path const path_to_settings = boost::filesystem::path())
							: _RelatedImpl_base<UIOnlySettings>(messager, path_to_settings)
							, _UIProjectRelatedImpl_base(messager, project, path_to_settings)
						{

						}

				};

				class _BackendRelatedImpl : public _BackendProjectRelatedImpl_base
				{

					public:

						_BackendRelatedImpl(Messager & messager, Project & project, boost::filesystem::path const path_to_settings = boost::filesystem::path())
							: _RelatedImpl_base<ProjectSettings>(messager, path_to_settings)
							, _BackendProjectRelatedImpl_base(messager, project, path_to_settings)
						{

						}

				};

			protected:

				void CreateInternalImplementations(Messager & messager, Project & project, boost::filesystem::path const path_to_settings = boost::filesystem::path());

				// throws catastrophic error - only here to support abstract base class, SFINAE cannot be used because it would require virtual template member functions
				void CreateInternalImplementations(Messager & messager, boost::filesystem::path const path_to_settings = boost::filesystem::path());

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

		void CreateImplementation(Messager & messager, Project & project, boost::filesystem::path const path_to_settings = boost::filesystem::path());

		// throws catastrophic error - only here to support abstract base class, SFINAE cannot be used because it would require virtual template member functions
		void CreateImplementation(Messager & messager, boost::filesystem::path const path_to_settings = boost::filesystem::path());

		std::unique_ptr<_impl> __impl;


	public:

		UIOnlySettings & getUISettings();
		ProjectSettings & getBackendSettings();

};

#endif // UIPROJECTSETTINGS_H
