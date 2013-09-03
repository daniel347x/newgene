#ifndef UISETTINGS_H
#define UISETTINGS_H

#include <QObject>
#include "../../../NewGeneBackEnd/Settings/Settings.h"
#include "../../../NewGeneBackEnd/Settings/SettingsManager.h"
#include "../../../NewGeneBackEnd/Settings/SettingsRepository.h"
#include "../../../NewGeneBackEnd/Settings/GlobalSettings.h"
#include "../../../NewGeneBackEnd/Settings/ProjectSettings.h"
#include "../../../NewGeneBackEnd/Project/Project.h"
#include "../../../NewGeneBackEnd/globals.h"
#include "uisetting.h"
#include "eventloopthreadmanager.h"

template<WORK_QUEUE_THREAD_LOOP_CLASS_ENUM UI_THREAD_LOOP_CLASS_ENUM>
class UIAllSettings : public EventLoopThreadManager<UI_THREAD_LOOP_CLASS_ENUM>
{

	public:

		UIAllSettings(UIMessager & messager, int const number_worker_threads)
			: EventLoopThreadManager<UI_THREAD_LOOP_CLASS_ENUM>(number_worker_threads)
		{

		}

		virtual void WriteSettingsToFile() {}

		virtual void UpdateConnections() {}

	signals:

	public slots:


	protected:


	protected:

		// The following is the equivalent of the backend's Settings class.
		// It is itself a SettingsRepository class, just like the backend
		// ProjectSettings class, but this SettingsRepository class
		// stores settings related to the user interface for the project.
		// A data member for this class is stored within an instance of
		// _UIRelatedImpl_base, defined below.
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

			public:

				template<typename T>
				void UpdateSetting(Messager & messager, SETTINGS_ENUM const which_setting, T const & setting_value)
				{
					SettingInfo setting_info = SettingInfoObject.GetSettingInfoFromEnum(messager, which_setting);
					_settings_map[which_setting] = std::unique_ptr<SETTING_CLASS>(NewSetting(messager, setting_info, (void const *)(&setting_value)));
					WriteSettingsToFile(messager);
				}

				void WriteSettingsToFile(Messager & messager)
				{
					boost::property_tree::ptree pt;
					WriteSettingsToPtree(messager, pt);
					WritePtreeToFile(messager, pt);
				}

			protected:

				UIOnlySettings_base(UIMessager & messager, boost::filesystem::path const path_to_settings) : SettingsRepository<SETTINGS_ENUM, SETTING_CLASS>(messager, path_to_settings) {}

		};

		template<typename BACKEND_SETTINGS_CLASS, typename UI_SETTINGS_CLASS>
		class _impl_base
		{

			public:

				_impl_base(UIMessager &)
				{}

				template<typename SETTINGS_REPOSITORY_CLASS>
				class _RelatedImpl_base
				{

					public:

						virtual SETTINGS_REPOSITORY_CLASS & getSettingsRepository() = 0;

					protected:

						_RelatedImpl_base(UIMessager & messager, boost::filesystem::path const)
						{

						}

				};

				template<typename SETTINGS_REPOSITORY_CLASS>
				class _GlobalRelatedImpl_base : virtual public _RelatedImpl_base<SETTINGS_REPOSITORY_CLASS>
				{

					protected:

						_GlobalRelatedImpl_base(UIMessager & messager, boost::filesystem::path const path_to_settings)
							: _RelatedImpl_base<SETTINGS_REPOSITORY_CLASS>(messager, path_to_settings)
						{

						}

				};

				template<typename SETTINGS_REPOSITORY_CLASS>
				class _ProjectRelatedImpl_base : virtual public _RelatedImpl_base<SETTINGS_REPOSITORY_CLASS>
				{

					protected:

						_ProjectRelatedImpl_base(UIMessager & messager, boost::filesystem::path const path_to_settings)
							: _RelatedImpl_base<SETTINGS_REPOSITORY_CLASS>(messager, path_to_settings)
						{

						}

				};

				class _UIRelatedImpl_base : virtual public _RelatedImpl_base<UI_SETTINGS_CLASS>
				{

					public:

						// Ignore VS warning C4250: "inherits via dominance".  The warning is because the virtual function in _RelatedImpl_base is being overridden,
						// and it is the overridden version that is being used.
						// But the virtual function being overridden is abstract, so the compiler warning should not be given in this scenario.
						UI_SETTINGS_CLASS & getSettingsRepository()
						{
							if (!_settings_repository)
							{
								boost::format msg( "Settings repository instance not yet constructed." );
								throw NewGeneException() << newgene_error_description( msg.str() );
							}
							return *(_settings_repository.get());
						}

					protected:

						_UIRelatedImpl_base(UIMessager & messager, boost::filesystem::path const path_to_settings)
							: _RelatedImpl_base<UI_SETTINGS_CLASS>(messager, path_to_settings)
							, _settings_repository(SettingsRepositoryFactory<UI_SETTINGS_CLASS>()(messager, path_to_settings))
						{

						}

						std::unique_ptr<UI_SETTINGS_CLASS> _settings_repository;

				};

				class _BackendRelatedImpl_base : virtual public _RelatedImpl_base<BACKEND_SETTINGS_CLASS>
				{

					protected:

						_BackendRelatedImpl_base(UIMessager & messager, boost::filesystem::path const path_to_settings)
							: _RelatedImpl_base<BACKEND_SETTINGS_CLASS>(messager, path_to_settings)
						{

						}

				};

				class _BackendGlobalRelatedImpl_base : public _BackendRelatedImpl_base, public _GlobalRelatedImpl_base<BACKEND_SETTINGS_CLASS>
				{

					public:

						// Ignore VS warning C4250: "inherits via dominance".  The warning is because the virtual function in _RelatedImpl_base is being overridden,
						// and it is the overridden version that is being used.
						// But the virtual function being overridden is abstract, so the compiler warning should not be given in this scenario.
						BACKEND_SETTINGS_CLASS & getSettingsRepository()
						{
							if (!settingsManager()._global_settings)
							{
								boost::format msg( "GlobalSettings instance not yet constructed." );
								throw NewGeneException() << newgene_error_description( msg.str() );
							}
							return *(settingsManager()._global_settings.get());
						}

					protected:

						_BackendGlobalRelatedImpl_base(UIMessager & messager, boost::filesystem::path const path_to_settings)
							: _RelatedImpl_base<BACKEND_SETTINGS_CLASS>(messager, path_to_settings)
							, _BackendRelatedImpl_base(messager, path_to_settings)
							, _GlobalRelatedImpl_base<BACKEND_SETTINGS_CLASS>(messager, path_to_settings)
						{
							settingsManager()._global_settings.reset(SettingsRepositoryFactory<BACKEND_SETTINGS_CLASS>()(messager, path_to_settings));
						}

				};

				class _BackendProjectRelatedImpl_base : public _BackendRelatedImpl_base, public _ProjectRelatedImpl_base<BACKEND_SETTINGS_CLASS>
				{

					public:

						BACKEND_SETTINGS_CLASS & getSettingsRepository()
						{
							if (!_settings_repository)
							{
								boost::format msg( "Settings repository instance not yet constructed." );
								throw NewGeneException() << newgene_error_description( msg.str() );
							}
							return *_settings_repository;
						}

						std::shared_ptr<BACKEND_SETTINGS_CLASS> & getSettingsRepositorySharedPtr()
						{
							return _settings_repository;
						}


					protected:

						_BackendProjectRelatedImpl_base(UIMessager & messager, boost::filesystem::path const path_to_settings)
							: _RelatedImpl_base<BACKEND_SETTINGS_CLASS>(messager, path_to_settings)
							, _BackendRelatedImpl_base(messager, path_to_settings)
							, _ProjectRelatedImpl_base<BACKEND_SETTINGS_CLASS>(messager, path_to_settings)
							, _settings_repository(SettingsRepositoryFactory<BACKEND_SETTINGS_CLASS>()(messager, path_to_settings))
						{
						}

						std::shared_ptr<BACKEND_SETTINGS_CLASS> _settings_repository;

				};

				class _UIGlobalRelatedImpl_base : public _UIRelatedImpl_base, public _GlobalRelatedImpl_base<UI_SETTINGS_CLASS>
				{

					protected:

						_UIGlobalRelatedImpl_base(UIMessager & messager, boost::filesystem::path const path_to_settings)
							: _RelatedImpl_base<UI_SETTINGS_CLASS>(messager, path_to_settings)
							, _UIRelatedImpl_base(messager, path_to_settings)
							, _GlobalRelatedImpl_base<UI_SETTINGS_CLASS>(messager, path_to_settings)
						{

						}

				};

				class _UIProjectRelatedImpl_base : public _UIRelatedImpl_base, public _ProjectRelatedImpl_base<UI_SETTINGS_CLASS>
				{

					protected:

						_UIProjectRelatedImpl_base(UIMessager & messager, boost::filesystem::path const path_to_settings)
							: _RelatedImpl_base<UI_SETTINGS_CLASS>(messager, path_to_settings)
							, _UIRelatedImpl_base(messager, path_to_settings)
							, _ProjectRelatedImpl_base<UI_SETTINGS_CLASS>(messager, path_to_settings)
						{

						}

				};

				std::unique_ptr<_UIRelatedImpl_base> __ui_impl;
				std::unique_ptr<_BackendRelatedImpl_base> __backend_impl;

				_UIRelatedImpl_base & getInternalUIImplementation()
				{
					if (!__ui_impl)
					{
						boost::format msg( "Internal UI settings implementation not yet constructed." );
						throw NewGeneException() << newgene_error_description( msg.str() );
					}
					return *(__ui_impl.get());
				}

				_BackendRelatedImpl_base & getInternalBackendImplementation()
				{
					if (!__backend_impl)
					{
						boost::format msg( "Internal backend settings implementation not yet constructed." );
						throw NewGeneException() << newgene_error_description( msg.str() );
					}
					return *(__backend_impl.get());
				}

		};

	public:


	private:


	protected:

		template<typename BACKEND_SETTINGS_CLASS, typename UI_SETTINGS_CLASS, typename SETTINGS_ENUM, typename SETTING_CLASS>
		UIOnlySettings_base<SETTINGS_ENUM, SETTING_CLASS> &
		getUISettings_base(_impl_base<BACKEND_SETTINGS_CLASS, UI_SETTINGS_CLASS> & impl)
		{
			_impl_base<BACKEND_SETTINGS_CLASS, UI_SETTINGS_CLASS>::_UIRelatedImpl_base & impl_ui = impl.getInternalUIImplementation();
			UI_SETTINGS_CLASS & settings_repository = impl_ui.getSettingsRepository();
			return static_cast<UIOnlySettings_base<SETTINGS_ENUM, SETTING_CLASS> &>(settings_repository);
		}

		template<typename BACKEND_SETTINGS_CLASS, typename UI_SETTINGS_CLASS, typename SETTINGS_ENUM, typename SETTING_CLASS>
		Settings<SETTINGS_ENUM, SETTING_CLASS> &
		getBackendSettings_base(_impl_base<BACKEND_SETTINGS_CLASS, UI_SETTINGS_CLASS> & impl)
		{
			_impl_base<BACKEND_SETTINGS_CLASS, UI_SETTINGS_CLASS>::_BackendRelatedImpl_base & impl_backend = impl.getInternalBackendImplementation();
			BACKEND_SETTINGS_CLASS & settings_repository = impl_backend.getSettingsRepository();
			return static_cast<Settings<SETTINGS_ENUM, SETTING_CLASS> &>(settings_repository);
		}

		template<typename BACKEND_SETTINGS_CLASS, typename UI_SETTINGS_CLASS>
		std::shared_ptr<BACKEND_SETTINGS_CLASS>
		getBackendSettingsSharedPtr_base(_impl_base<BACKEND_SETTINGS_CLASS, UI_SETTINGS_CLASS> & impl)
		{
			try
			{
				_impl_base<BACKEND_SETTINGS_CLASS, UI_SETTINGS_CLASS>::_BackendProjectRelatedImpl_base & impl_backend = dynamic_cast<_impl_base<BACKEND_SETTINGS_CLASS, UI_SETTINGS_CLASS>::_BackendProjectRelatedImpl_base &>(impl.getInternalBackendImplementation());
				return impl_backend.getSettingsRepositorySharedPtr();
			}
			catch (std::bad_cast & bc)
			{
				boost::format msg( "Cannot convert from _BackendRelatedImpl_base to _BackendProjectRelatedImpl_base in getBackendSettingsSharedPtr_base: %1%" );
				msg % bc.what();
				throw NewGeneException() << newgene_error_description( msg.str() );
			}
		}

};

#endif // UISETTINGS_H
