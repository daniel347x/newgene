#ifndef UIOUTPUTMODEL_H
#define UIOUTPUTMODEL_H

#include "../../../../NewGeneBackEnd/Model/OutputModel.h"
#include "uimodel.h"
#include "outputmodelworkqueue.h"

class UIOutputModel : public QObject, public UIModel<UI_OUTPUT_MODEL>
{

		Q_OBJECT

	public:

		UIOutputModel(UIMessager & messager, std::shared_ptr<OutputModel> const & backend_model_instance, QObject * parent = NULL)
			: QObject(parent)
			, UIModel<UI_OUTPUT_MODEL>(messager, OutputModel::number_worker_threads)
		{
			CreateImplementation(messager, backend_model_instance);
		}

		void UpdateConnections();

	protected:

		class _impl : public _impl_base<OutputModel>
		{

			public:

				_impl(UIMessager & messager, std::shared_ptr<OutputModel> const & backend_model_instance) : _impl_base(messager)
				{
					CreateInternalImplementations(messager, backend_model_instance);
				}

				class _RelatedImpl : public _RelatedImpl_base
				{

					public:

						_RelatedImpl(UIMessager & messager, std::shared_ptr<OutputModel> const & backend_model_instance)
							: _RelatedImpl_base(messager, backend_model_instance)
						{

						}

				};

			protected:

				void CreateInternalImplementations(UIMessager & messager, std::shared_ptr<OutputModel> const & backend_model_instance)
				{
					__impl.reset(new _RelatedImpl(messager, backend_model_instance));
				}

		};

		_impl_base<OutputModel> & getImplementation()
		{
			if (!__impl)
			{
				boost::format msg("Model implementation not yet constructed.");
				throw NewGeneException() << newgene_error_description(msg.str());
			}

			return *__impl;
		}

		void CreateImplementation(UIMessager & messager, std::shared_ptr<OutputModel> const & backend_model_instance)
		{
			__impl.reset(new _impl(messager, backend_model_instance));
		}

		std::unique_ptr<_impl> __impl;


	public:

		OutputModel & getBackendModel()
		{
			if (!__impl)
			{
				boost::format msg("Internal model implementation not yet constructed.");
				throw NewGeneException() << newgene_error_description(msg.str());
			}

			return static_cast<OutputModel &>(getBackendModel_base<OutputModel>(*__impl));
		}

		OutputModel & backend()
		{
			return getBackendModel();
		}

		std::shared_ptr<OutputModel> getBackendModelSharedPtr()
		{
			if (!__impl)
			{
				boost::format msg("Internal model implementation not yet constructed.");
				throw NewGeneException() << newgene_error_description(msg.str());
			}

			return getBackendModelSharedPtr_base<OutputModel>(*__impl);
		}

		bool is_model_equivalent(UIMessager & messager, UIOutputModel * model);

		QObject * getConnectorDatabase()
		{
			return getConnectorTwo();
		}

	protected:

		WorkQueueManager<UI_OUTPUT_MODEL> * InstantiateWorkQueue(void * ui_object, bool isPool2_ = false)
		{
			OutputModelWorkQueue * work_queue = new OutputModelWorkQueue(isPool2_);
			work_queue->SetUIObject(reinterpret_cast<UIOutputModel *>(ui_object));
			work_queue->SetConnections();
			return work_queue;
		}

	signals:

		void DoneLoadingFromDatabase(UI_OUTPUT_MODEL_PTR, QObject *);

	public slots:

		void SignalMessageBox(STD_STRING);

};

#endif // UIOUTPUTMODEL_H
