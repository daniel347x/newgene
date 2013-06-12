#ifndef UIINPUTMODEL_H
#define UIINPUTMODEL_H

#include "../../../NewGeneBackEnd/Model/InputModel.h"
#include "uimodel.h"

class UIInputModel : public UIModel
{

	Q_OBJECT

	public:

		UIInputModel(UIMessager & messager, std::shared_ptr<InputModel> const & backend_model_instance, QObject * parent = NULL)
			: UIModel(messager)
		{
			CreateImplementation(messager, backend_model_instance);
		}


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

		std::shared_ptr<InputModel> & getBackendModelSharedPtr()
		{
			if (!__impl)
			{
				boost::format msg( "Internal model implementation not yet constructed." );
				throw NewGeneException() << newgene_error_description( msg.str() );
			}
			return getBackendModelSharedPtr_base<InputModel>(*__impl);
		}

};

#endif // UIINPUTMODEL_H
