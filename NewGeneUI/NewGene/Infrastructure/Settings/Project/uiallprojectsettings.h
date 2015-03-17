#ifndef UIPROJECTSETTINGS_H
#define UIPROJECTSETTINGS_H

#include <QObject>
#include "../../../NewGeneBackEnd/Utilities/NewGeneException.h"
#include "uiallsettings.h"
#include "../../../NewGeneBackEnd/Settings/ProjectSettings.h"

template<typename BACKEND_PROJECT_SETTINGS_CLASS, typename PROJECT_SETTINGS_ENUM, typename UI_PROJECT_SETTINGS_ENUM, typename BACKEND_PROJECT_SETTING_CLASS, typename UI_PROJECT_SETTING_CLASS, WORK_QUEUE_THREAD_LOOP_CLASS_ENUM UI_THREAD_LOOP_CLASS_ENUM>
class UIAllProjectSettings : public UIAllSettings<UI_THREAD_LOOP_CLASS_ENUM>
{

	public:

		UIAllProjectSettings(UIMessager & messager, boost::filesystem::path const path_to_settings, int const number_worker_threads)
			: UIAllSettings<UI_THREAD_LOOP_CLASS_ENUM>(messager, number_worker_threads)
		{
			this->CreateImplementation(messager, path_to_settings);
		}



		// The following is the equivalent of the backend's ProjectSettings class
		#		ifdef _WIN32
		// Obnoxious Visual Studio bug: http://stackoverflow.com/questions/20847637/clang-os-x-requires-template-keyword-in-a-particular-nested-declaration-whi
		class UIOnlySettings : public UIAllSettings<UI_THREAD_LOOP_CLASS_ENUM>::UIOnlySettings_base<UI_PROJECT_SETTINGS_ENUM, UI_PROJECT_SETTING_CLASS>
		#		else
		class UIOnlySettings : public UIAllSettings<UI_THREAD_LOOP_CLASS_ENUM>::template UIOnlySettings_base<UI_PROJECT_SETTINGS_ENUM, UI_PROJECT_SETTING_CLASS>
		#		endif
		{

			public:

				UIOnlySettings(Messager & messager_, boost::filesystem::path const path_to_settings = boost::filesystem::path()) : UIAllSettings<UI_THREAD_LOOP_CLASS_ENUM>::template
					UIOnlySettings_base<UI_PROJECT_SETTINGS_ENUM, UI_PROJECT_SETTING_CLASS>(static_cast<UIMessager &>(messager_), path_to_settings)
				{

				}


			protected:

		};

	protected:

		#		ifdef _WIN32
		// Obnoxious Visual Studio bug: http://stackoverflow.com/questions/20847637/clang-os-x-requires-template-keyword-in-a-particular-nested-declaration-whi
		class _impl : public UIAllSettings<UI_THREAD_LOOP_CLASS_ENUM>::_impl_base<BACKEND_PROJECT_SETTINGS_CLASS, UIOnlySettings>
		#		else
		class _impl : public UIAllSettings<UI_THREAD_LOOP_CLASS_ENUM>::template _impl_base<BACKEND_PROJECT_SETTINGS_CLASS, UIOnlySettings>
		#		endif
		{

			public:

				_impl(UIMessager & messager, boost::filesystem::path const path_to_settings = boost::filesystem::path()) : UIAllSettings<UI_THREAD_LOOP_CLASS_ENUM>::template
					_impl_base<BACKEND_PROJECT_SETTINGS_CLASS, UIOnlySettings>(messager)
				{
					CreateInternalImplementations(messager, path_to_settings);
				}

				class _UIRelatedImpl : public UIAllSettings<UI_THREAD_LOOP_CLASS_ENUM>::template _impl_base<BACKEND_PROJECT_SETTINGS_CLASS, UIOnlySettings>::_UIProjectRelatedImpl_base
				{

					public:

					_UIRelatedImpl(UIMessager & messager, boost::filesystem::path const path_to_settings = boost::filesystem::path())
					: UIAllSettings<UI_THREAD_LOOP_CLASS_ENUM>::template _impl_base<BACKEND_PROJECT_SETTINGS_CLASS, UIOnlySettings>::template _RelatedImpl_base<UIOnlySettings>(messager,
							path_to_settings)
					, UIAllSettings<UI_THREAD_LOOP_CLASS_ENUM>::template _impl_base<BACKEND_PROJECT_SETTINGS_CLASS, UIOnlySettings>::_UIProjectRelatedImpl_base(messager, path_to_settings)
				{

				}

																				   };

				class _BackendRelatedImpl : public UIAllSettings<UI_THREAD_LOOP_CLASS_ENUM>::template _impl_base<BACKEND_PROJECT_SETTINGS_CLASS, UIOnlySettings>::_BackendProjectRelatedImpl_base
				{

					public:

					_BackendRelatedImpl(UIMessager & messager, boost::filesystem::path const path_to_settings = boost::filesystem::path())
					: UIAllSettings<UI_THREAD_LOOP_CLASS_ENUM>::template _impl_base<BACKEND_PROJECT_SETTINGS_CLASS, UIOnlySettings>::template _RelatedImpl_base<BACKEND_PROJECT_SETTINGS_CLASS>
					(messager, path_to_settings)
					, UIAllSettings<UI_THREAD_LOOP_CLASS_ENUM>::template _impl_base<BACKEND_PROJECT_SETTINGS_CLASS, UIOnlySettings>::_BackendProjectRelatedImpl_base(messager, path_to_settings)
				{

				}

																				   };

			protected:

				void CreateInternalImplementations(UIMessager & messager, boost::filesystem::path const path_to_settings = boost::filesystem::path())
				{
					this->__ui_impl.reset(new _UIRelatedImpl(messager, path_to_settings));
					this->__backend_impl.reset(new _BackendRelatedImpl(messager, path_to_settings));
				}

		};

		typename UIAllSettings<UI_THREAD_LOOP_CLASS_ENUM>::template _impl_base<BACKEND_PROJECT_SETTINGS_CLASS, UIOnlySettings> & getImplementation()
		{
			if (!__impl)
			{
				boost::format msg("UI Project settings implementation not yet constructed.");
				throw NewGeneException() << newgene_error_description(msg.str());
			}

			return *__impl;
		}

		void CreateImplementation(UIMessager & messager, boost::filesystem::path const path_to_settings = boost::filesystem::path())
		{
			__impl.reset(new _impl(messager, path_to_settings));
		}

		std::unique_ptr<_impl> __impl;


	public:

		UIOnlySettings & getUISettings()
		{
			if (!__impl)
			{
				boost::format msg("Internal settings implementation not yet constructed.");
				throw NewGeneException() << newgene_error_description(msg.str());
			}

			return static_cast<UIOnlySettings &>(this->template getUISettings_base<BACKEND_PROJECT_SETTINGS_CLASS, UIOnlySettings, UI_PROJECT_SETTINGS_ENUM, UI_PROJECT_SETTING_CLASS>
												 (*__impl));
		}

		BACKEND_PROJECT_SETTINGS_CLASS & getBackendSettings()
		{
			if (!__impl)
			{
				boost::format msg("Internal settings implementation not yet constructed.");
				throw NewGeneException() << newgene_error_description(msg.str());
			}

			return static_cast<BACKEND_PROJECT_SETTINGS_CLASS &>(this->template
					getBackendSettings_base<BACKEND_PROJECT_SETTINGS_CLASS, UIOnlySettings, PROJECT_SETTINGS_ENUM, BACKEND_PROJECT_SETTING_CLASS>(*__impl));
		}

		std::shared_ptr<BACKEND_PROJECT_SETTINGS_CLASS> getBackendSettingsSharedPtr()
		{
			if (!__impl)
			{
				boost::format msg("Internal settings implementation not yet constructed.");
				throw NewGeneException() << newgene_error_description(msg.str());
			}

			return this->template getBackendSettingsSharedPtr_base<BACKEND_PROJECT_SETTINGS_CLASS, UIOnlySettings>(*__impl);
		}

};

#endif // UIPROJECTSETTINGS_H
