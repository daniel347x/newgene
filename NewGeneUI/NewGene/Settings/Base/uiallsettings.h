#ifndef UISETTINGS_H
#define UISETTINGS_H

#include <QObject>
#include "../../../NewGeneBackEnd/Settings/Settings.h"
#include "../../../NewGeneBackEnd/Settings/SettingsRepository.h"
#include "uisetting.h"

class UIAllSettings : public QObject
{

		Q_OBJECT


	public:

		explicit UIAllSettings(Messager & messager, QObject * parent = NULL);


	signals:

	public slots:


	protected:


	protected:

		template<typename SETTINGS_ENUM, typename SETTING_CLASS>
		class UIOnlySettings_base : public SettingsRepository<SETTINGS_ENUM, SETTING_CLASS>
		{

				// ***********************************************************************
				// Directly derive from SettingsRepository.
				// Therefore, we ourselves (through this base class)
				// maintain the UI-related settings.
				//
				// (Unlike the backend-related settings implementation, below,
				// which simply holds a pointer to a backend settings instance,
				// and it is the backend settings instance which derives from
				// SettingsRepository.)
				// ***********************************************************************

			protected:

				UIOnlySettings_base(Messager & messager) : SettingsRepository<SETTINGS_ENUM, SETTING_CLASS>(messager) {}
				UIOnlySettings_base(Messager & messager, boost::filesystem::path const path_to_settings) : SettingsRepository<SETTINGS_ENUM, SETTING_CLASS>(messager, path_to_settings) {}

		};

		template<typename BACKEND_SETTINGS_CLASS, typename UI_SETTINGS_CLASS>
		class _impl_base
		{

			public:

				_impl_base(Messager &)
				{}

			protected:

				template<typename SETTINGS_REPOSITORY_CLASS>
				class _RelatedImpl_base
				{

					public:



					protected:

						_RelatedImpl_base(Messager & messager)
							: _settings_repository(SettingsRepositoryFactory<SETTINGS_REPOSITORY_CLASS>()(messager))
						{

						}

						_RelatedImpl_base(Messager & messager, boost::filesystem::path const path_to_settings)
							: _settings_repository(SettingsRepositoryFactory<SETTINGS_REPOSITORY_CLASS>()(messager, path_to_settings))
						{

						}

						std::unique_ptr<SETTINGS_REPOSITORY_CLASS> _settings_repository;

				};

				class _UIRelatedImpl_base : public _RelatedImpl_base<UI_SETTINGS_CLASS>
				{

					protected:

						_UIRelatedImpl_base(Messager & messager)
							: _RelatedImpl_base<UI_SETTINGS_CLASS>(messager)
						{

						}

						_UIRelatedImpl_base(Messager & messager, boost::filesystem::path const path_to_settings)
							: _RelatedImpl_base<UI_SETTINGS_CLASS>(messager, path_to_settings)
						{

						}

				};

				class _BackendRelatedImpl_base : public _RelatedImpl_base<BACKEND_SETTINGS_CLASS>
				{

					protected:

						_BackendRelatedImpl_base(Messager & messager)
							: _RelatedImpl_base<BACKEND_SETTINGS_CLASS>(messager)
						{

						}

						_BackendRelatedImpl_base(Messager & messager, boost::filesystem::path const path_to_settings)
							: _RelatedImpl_base<BACKEND_SETTINGS_CLASS>(messager, path_to_settings)
						{

						}

				};

				virtual void CreateInternalImplementations(Messager & messager) = 0;
				virtual void CreateInternalImplementations(Messager & messager, boost::filesystem::path const path_to_settings) = 0;
				std::unique_ptr<_UIRelatedImpl_base> __ui_impl;
				std::unique_ptr<_BackendRelatedImpl_base> __backend_impl;

		};


	private:


	protected:

		virtual void CreateImplementation(Messager & messager) = 0;
		virtual void CreateImplementation(Messager & messager, boost::filesystem::path const path_to_settings) = 0;

};

#endif // UISETTINGS_H
