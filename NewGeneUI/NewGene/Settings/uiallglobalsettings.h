#ifndef UIGLOBALSETTINGS_H
#define UIGLOBALSETTINGS_H

#include <QObject>
#include "../../../NewGeneBackEnd/Utilities/NewGeneException.h"
#include "uiallsettings.h"
#include "../../../NewGeneBackEnd/Settings/GlobalSettings.h"

namespace GLOBAL_SETTINGS_UI_NAMESPACE
{

	enum GLOBAL_SETTINGS_UI
	{
		  SETTING_FIRST = 0
		, MRU_LIST
		, SETTING_LAST
	};

}

class UIGlobalSetting_MRUList : public UIGlobalSetting, public StringSetting
{

	public:

		UIGlobalSetting_MRUList(UIMessager & messager, std::string const & setting)
			: UIGlobalSetting()
			, StringSetting(messager, setting)
		{}

		virtual void DoSpecialParse(UIMessager &)
		{
//			boost::format msg("Here is a message!");
//			messager.AppendMessage(new UIMessagerErrorMessage(MESSAGER_MESSAGE__GENERAL_ERROR, msg.str()));
		}

};

class UIAllGlobalSettings : public UIAllSettings
{

		Q_OBJECT

	public:

		UIAllGlobalSettings(UIMessager & messager, boost::filesystem::path const path_to_settings = boost::filesystem::path(), QObject * parent = NULL);


	signals:

	public slots:

	protected:

		// The following is the equivalent of the backend's GlobalSettings class
		class UIOnlySettings : public UIOnlySettings_base<GLOBAL_SETTINGS_UI_NAMESPACE::GLOBAL_SETTINGS_UI, UIGlobalSetting>
		{

			public:

				UIOnlySettings(UIMessager & messager, boost::filesystem::path const path_to_settings = boost::filesystem::path()) : UIOnlySettings_base(messager, path_to_settings)
				{

				}


			protected:

				boost::filesystem::path GetSettingsPath(UIMessager & messager, SettingInfo & setting_info);
				void SetMapEntry(UIMessager & messager, SettingInfo & setting_info, boost::property_tree::ptree & pt);
				UIGlobalSetting * CloneSetting(UIMessager & messager, UIGlobalSetting * current_setting, SettingInfo & setting_info) const;
				UIGlobalSetting * NewSetting(UIMessager & messager, SettingInfo & setting_info, void const * setting_value_void);

		};

		class _impl : public _impl_base<GlobalSettings, UIOnlySettings>
		{

			public:

				_impl(UIMessager & messager, boost::filesystem::path const path_to_settings = boost::filesystem::path()) : _impl_base(messager)
				{
					CreateInternalImplementations(messager, path_to_settings);
				}

				class _UIRelatedImpl : public _UIGlobalRelatedImpl_base
				{

					public:

						_UIRelatedImpl(UIMessager & messager, boost::filesystem::path const path_to_settings = boost::filesystem::path())
							: _RelatedImpl_base<UIOnlySettings>(messager, path_to_settings)
							, _UIGlobalRelatedImpl_base(messager, path_to_settings)
						{

						}

				};

				class _BackendRelatedImpl : public _BackendGlobalRelatedImpl_base
				{

					public:

						_BackendRelatedImpl(UIMessager & messager, boost::filesystem::path const path_to_settings = boost::filesystem::path())
							: _RelatedImpl_base<GlobalSettings>(messager, path_to_settings)
							, _BackendGlobalRelatedImpl_base(messager, path_to_settings)
						{

						}

				};

			protected:

				void CreateInternalImplementations(UIMessager & messager, boost::filesystem::path const path_to_settings = boost::filesystem::path());

		};

		_impl_base<GlobalSettings, UIOnlySettings> & getImplementation()
		{
			if (!__impl)
			{
				boost::format msg( "UI Global settings implementation not yet constructed." );
				throw NewGeneException() << newgene_error_description( msg.str() );
			}
			return *__impl;
		}

		void CreateImplementation(UIMessager & messager, boost::filesystem::path const path_to_settings = boost::filesystem::path());

		std::unique_ptr<_impl> __impl;

	public:

		UIOnlySettings & getUISettings();
		GlobalSettings & getBackendSettings();

};

#endif // UIGLOBALSETTINGS_H
