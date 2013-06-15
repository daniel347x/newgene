#ifndef UIMODELSETTINGS_H
#define UIMODELSETTINGS_H

#include <QObject>
#include "../../Messager/uimessager.h"
#include "../../../NewGeneBackEnd/globals.h"
#include "eventloopthreadmanager.h"

template<WORK_QUEUE_THREAD_LOOP_CLASS_ENUM UI_THREAD_LOOP_CLASS_ENUM>
class UIModelSettings : public EventLoopThreadManager<UI_THREAD_LOOP_CLASS_ENUM>
{

	public:

		UIModelSettings(UIMessager & messager, int const number_worker_threads)
			: EventLoopThreadManager<UI_THREAD_LOOP_CLASS_ENUM>(number_worker_threads)
		{

		}


	signals:

	public slots:


	protected:


	protected:

		template<typename BACKEND_SETTINGS_CLASS>
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

				class _ModelRelatedImpl_base : public _RelatedImpl_base<BACKEND_SETTINGS_CLASS>
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

						_ModelRelatedImpl_base(UIMessager & messager, boost::filesystem::path const path_to_settings)
							: _RelatedImpl_base<BACKEND_SETTINGS_CLASS>(messager, path_to_settings)
							, _settings_repository(SettingsRepositoryFactory<BACKEND_SETTINGS_CLASS>()(messager, path_to_settings))
						{
						}

						std::shared_ptr<BACKEND_SETTINGS_CLASS> _settings_repository;

				};

				std::unique_ptr<_ModelRelatedImpl_base> __backend_impl;

				_ModelRelatedImpl_base & getInternalBackendImplementation()
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

		template<typename BACKEND_SETTINGS_CLASS, typename SETTINGS_ENUM, typename SETTING_CLASS>
		Settings<SETTINGS_ENUM, SETTING_CLASS> &
		getBackendSettings_base(_impl_base<BACKEND_SETTINGS_CLASS> & impl)
		{
			_impl_base<BACKEND_SETTINGS_CLASS>::_ModelRelatedImpl_base & impl_backend = impl.getInternalBackendImplementation();
			BACKEND_SETTINGS_CLASS & settings_repository = impl_backend.getSettingsRepository();
			return static_cast<Settings<SETTINGS_ENUM, SETTING_CLASS> &>(settings_repository);
		}

		template<typename BACKEND_SETTINGS_CLASS>
		std::shared_ptr<BACKEND_SETTINGS_CLASS>
		getBackendSettingsSharedPtr_base(_impl_base<BACKEND_SETTINGS_CLASS> & impl)
		{
			_impl_base<BACKEND_SETTINGS_CLASS>::_ModelRelatedImpl_base & impl_backend = impl.getInternalBackendImplementation();
			return impl_backend.getSettingsRepositorySharedPtr();
//			try
//			{
//				_impl_base<BACKEND_SETTINGS_CLASS>::_ModelRelatedImpl_base & impl_backend = dynamic_cast<_impl_base<BACKEND_SETTINGS_CLASS>::_ModelRelatedImpl_base &>(impl.getInternalBackendImplementation());
//				return impl_backend.getSettingsRepositorySharedPtr();
//			}
//			catch (std::bad_cast & bc)
//			{
//				boost::format msg( "Cannot convert from _BackendRelatedImpl_base to _BackendProjectRelatedImpl_base in getBackendSettingsSharedPtr_base: %1%" );
//				msg % bc.what();
//				throw NewGeneException() << newgene_error_description( msg.str() );
//			}
		}

};

#endif // UIMODELSETTINGS_H
