#ifndef UIINPUTMODEL_H
#define UIINPUTMODEL_H

#include "../../../../NewGeneBackEnd/Model/InputModel.h"
#include "uimodel.h"
#include "inputmodelworkqueue.h"

class UIInputModel : public QObject, public UIModel<UI_INPUT_MODEL>
{

	Q_OBJECT

	public:

		UIInputModel(UIMessager & messager, std::shared_ptr<InputModel> const & backend_model_instance, QObject * parent = NULL)
			: QObject(parent)
			, UIModel<UI_INPUT_MODEL>(messager, InputModel::number_worker_threads)
		{
			CreateImplementation(messager, backend_model_instance);
		}

		void UpdateConnections();

	protected:

		class _impl : public _impl_base<InputModel>
		{

			public:

				_impl(UIMessager & messager, std::shared_ptr<InputModel> const & backend_model_instance) : _impl_base(messager)
				{
					CreateInternalImplementations(messager, backend_model_instance);
				}

				class _RelatedImpl : public _RelatedImpl_base
				{

					public:

						_RelatedImpl(UIMessager & messager, std::shared_ptr<InputModel> const & backend_model_instance)
							: _RelatedImpl_base(messager, backend_model_instance)
						{

						}

				};

			protected:

				void CreateInternalImplementations(UIMessager & messager, std::shared_ptr<InputModel> const & backend_model_instance)
				{
					__impl.reset(new _RelatedImpl(messager, backend_model_instance));
				}

		};

		_impl_base<InputModel> & getImplementation()
		{
			if (!__impl)
			{
				boost::format msg( "Model implementation not yet constructed." );
				throw NewGeneException() << newgene_error_description( msg.str() );
			}
			return *__impl;
		}

		void CreateImplementation(UIMessager & messager, std::shared_ptr<InputModel> const & backend_model_instance)
		{
			__impl.reset(new _impl(messager, backend_model_instance));
		}

		std::unique_ptr<_impl> __impl;


	public:

		InputModel & getBackendModel()
		{
			if (!__impl)
			{
				boost::format msg( "Internal model implementation not yet constructed." );
				throw NewGeneException() << newgene_error_description( msg.str() );
			}
			return static_cast<InputModel &>(getBackendModel_base<InputModel>(*__impl));
		}

		InputModel & backend()
		{
			return getBackendModel();
		}

        std::shared_ptr<InputModel> const & getBackendModelSharedPtr()
		{
			if (!__impl)
			{
				boost::format msg( "Internal model implementation not yet constructed." );
				throw NewGeneException() << newgene_error_description( msg.str() );
			}
            return this->getBackendModelSharedPtr_base<InputModel>(*__impl);
		}

		bool is_model_equivalent(UIMessager & messager, UIInputModel * model);

		QObject * getConnectorDatabase()
		{
			return getConnectorTwo();
		}

	protected:

		WorkQueueManager<UI_INPUT_MODEL> * InstantiateWorkQueue(void * ui_object, bool isPool2_ = false)
		{
			InputModelWorkQueue * work_queue = new InputModelWorkQueue(isPool2_);
			work_queue->SetUIObject(reinterpret_cast<UIInputModel*>(ui_object));
			work_queue->SetConnections();
			return work_queue;
		}

	signals:

		void DoneLoadingFromDatabase(UI_INPUT_MODEL_PTR);

	public slots:

		void SignalMessageBox(STD_STRING);

};

#endif // UIINPUTMODEL_H
