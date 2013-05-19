#ifndef UIGLOBALSETTINGS_H
#define UIGLOBALSETTINGS_H

#include <QObject>
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

		UIGlobalSetting_MRUList(Messager & messager, std::string const & setting)
			: UIGlobalSetting()
			, StringSetting(messager, setting)
		{}

		virtual void DoSpecialParse(Messager &)
		{
//			boost::format msg("Here is a message!");
//			messager.AppendMessage(new UIMessagerErrorMessage(MESSAGER_MESSAGE__GENERAL_ERROR, msg.str()));
		}

};

template<>
SettingInfo GetSettingInfoFromEnum<GLOBAL_SETTINGS_UI_NAMESPACE::GLOBAL_SETTINGS_UI>(Messager & messager, GLOBAL_SETTINGS_UI_NAMESPACE::GLOBAL_SETTINGS_UI const value_);

class UIAllGlobalSettings : public UIAllSettings
{

		Q_OBJECT

	public:

		explicit UIAllGlobalSettings(Messager & messager, QObject * parent = NULL);
		UIAllGlobalSettings(Messager & messager, boost::filesystem::path const path_to_settings, QObject * parent = NULL);


	signals:

	public slots:

	protected:

		class UIOnlySettings : public UIOnlySettings_base<GLOBAL_SETTINGS_UI_NAMESPACE::GLOBAL_SETTINGS_UI, UIGlobalSetting>
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
				UIGlobalSetting * CloneSetting(Messager & messager, UIGlobalSetting * current_setting, SettingInfo & setting_info) const;
				UIGlobalSetting * NewSetting(Messager & messager, SettingInfo & setting_info, void const * setting_value_void);

		};

		class _impl : public _impl_base<GlobalSettings, UIOnlySettings>
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
				void CreateInternalImplementations(Messager & messager, boost::filesystem::path const path_to_settings)
;\
		};

		void CreateImplementation(Messager & messager);
		void CreateImplementation(Messager & messager, boost::filesystem::path const path_to_settings);
		std::unique_ptr<_impl> __impl;

};

#endif // UIGLOBALSETTINGS_H
