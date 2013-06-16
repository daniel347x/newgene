#ifndef UIOUTPUTMODELSETTINGS_H
#define UIOUTPUTMODELSETTINGS_H

#include "Base/uimodelsettings.h"
#include "outputmodelsettingsworkqueue.h"
#include "../../../../NewGeneBackEnd/Settings/outputmodelsettings.h"

class UIOutputModelSettings : public QObject, public UIModelSettings<UI_OUTPUT_MODEL_SETTINGS>
{

		Q_OBJECT

	public:

		UIOutputModelSettings(UIMessager & messager, boost::filesystem::path const path_to_settings = boost::filesystem::path(), QObject * parent = NULL)
			: QObject(parent)
			, UIModelSettings(messager, OutputModelSettings::number_worker_threads)
			{
				CreateImplementation(messager, path_to_settings);
			}


	signals:

	public slots:

	protected:

		class _impl : public _impl_base<OutputModelSettings>
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

		_impl_base<OutputModelSettings> & getImplementation()
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

		OutputModelSettings & getBackendSettings()
		{
			if (!__impl)
			{
				boost::format msg( "Internal settings implementation not yet constructed." );
				throw NewGeneException() << newgene_error_description( msg.str() );
			}
			return static_cast<OutputModelSettings &>(getBackendSettings_base<OutputModelSettings, OUTPUT_MODEL_SETTINGS_NAMESPACE::OUTPUT_MODEL_SETTINGS, OutputModelSetting>(*__impl));
		}

		std::shared_ptr<OutputModelSettings> & getBackendSettingsSharedPtr()
		{
			if (!__impl)
			{
				boost::format msg( "Internal settings implementation not yet constructed." );
				throw NewGeneException() << newgene_error_description( msg.str() );
			}
			return getBackendSettingsSharedPtr_base<OutputModelSettings>(*__impl);
		}

		virtual void WriteSettingsToFile(Messager & messager)
		{
			boost::property_tree::ptree pt;
			OutputModelSettings & backendsettings = getBackendSettings();
			backendsettings.WriteSettingsToPtree(messager, pt);
			backendsettings.WritePtreeToFile(messager, pt);
		}

	protected:

		WorkQueueManager<UI_OUTPUT_MODEL_SETTINGS> * InstantiateWorkQueue(void * ui_object, bool isPool2_ = false)
		{
			OutputModelSettingsWorkQueue * work_queue = new OutputModelSettingsWorkQueue();
			work_queue->SetUIObject(reinterpret_cast<UIOutputModelSettings*>(ui_object));
			work_queue->SetConnections();
			return work_queue;
		}

	public slots:
		void SignalMessageBox(STD_STRING);

};

#endif // UIOUTPUTMODELSETTINGS_H
