#ifndef UIINPUTMODELSETTINGS_H
#define UIINPUTMODELSETTINGS_H

#include "Base/uimodelsettings.h"
#include "inputmodelsettingsworkqueue.h"

class UIInputModelSettings : public QObject, public UIModelSettings<UI_INPUT_MODEL_SETTINGS>
{

		Q_OBJECT

	public:

		UIInputModelSettings(UIMessager & messager, boost::filesystem::path const path_to_settings = boost::filesystem::path(), QObject * parent = NULL)
			: QObject(parent)
			, UIModelSettings(messager, InputModelSettings::number_worker_threads)
			{
				CreateImplementation(messager, path_to_settings);
			}


	signals:

	public slots:

	protected:

		class _impl : public _impl_base<InputModelSettings>
		{

			public:

				_impl(UIMessager & messager, boost::filesystem::path const path_to_settings = boost::filesystem::path()) : _impl_base(messager)
				{
					CreateInternalImplementations(messager, path_to_settings);
				}

				class _ModelRelatedImpl : public _ModelRelatedImpl_base
				{

					public:

						_ModelRelatedImpl(UIMessager & messager, boost::filesystem::path const path_to_settings = boost::filesystem::path())
							: _ModelRelatedImpl_base(messager, path_to_settings)
						{

						}

				};

			protected:

				void CreateInternalImplementations(UIMessager & messager, boost::filesystem::path const path_to_settings = boost::filesystem::path())
				{
					__backend_impl.reset(new _ModelRelatedImpl(messager, path_to_settings));
				}

		};

		_impl_base<InputModelSettings> & getImplementation()
		{
			if (!__impl)
			{
				boost::format msg( "Model settings implementation not yet constructed." );
				throw NewGeneException() << newgene_error_description( msg.str() );
			}
			return *__impl;
		}

		void CreateImplementation(UIMessager & messager, boost::filesystem::path const path_to_settings = boost::filesystem::path())
		{
			__impl.reset(new _impl(messager, path_to_settings));
		}

		std::unique_ptr<_impl> __impl;

	public:

		InputModelSettings & getBackendSettings()
		{
			if (!__impl)
			{
				boost::format msg( "Internal settings implementation not yet constructed." );
				throw NewGeneException() << newgene_error_description( msg.str() );
			}
			return static_cast<InputModelSettings &>(getBackendSettings_base<InputModelSettings, INPUT_MODEL_SETTINGS_NAMESPACE::INPUT_MODEL_SETTINGS, InputModelSetting>(*__impl));
		}

		std::shared_ptr<InputModelSettings> & getBackendSettingsSharedPtr()
		{
			if (!__impl)
			{
				boost::format msg( "Internal settings implementation not yet constructed." );
				throw NewGeneException() << newgene_error_description( msg.str() );
			}
			return getBackendSettingsSharedPtr_base<InputModelSettings>(*__impl);
		}


		virtual void WriteSettingsToFile(Messager & messager)
		{
			boost::property_tree::ptree pt;
			InputModelSettings & backendsettings = getBackendSettings();
			backendsettings.WriteSettingsToPtree(messager, pt);
			backendsettings.WritePtreeToFile(messager, pt);
		}

	protected:

		WorkQueueManager<UI_INPUT_MODEL_SETTINGS> * InstantiateWorkQueue(void * ui_object)
		{
			InputModelSettingsWorkQueue * work_queue = new InputModelSettingsWorkQueue();
			work_queue->SetUIObject(reinterpret_cast<UIInputModelSettings*>(ui_object));
			work_queue->SetConnections();
			return work_queue;
		}

	public slots:
		void SignalMessageBox(QString msg);

};

#endif // UIINPUTMODELSETTINGS_H
