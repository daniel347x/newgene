#ifndef UIINPUTMODEL_H
#define UIINPUTMODEL_H

#include "../../../NewGeneBackEnd/Model/InputModel.h"
#include "uimodel.h"

class UIInputModel : public QObject, public UIModel<InputModelSettings>
{

	public:

		UIInputModel(UIMessager & messager, InputModelSettings * model_settings, QObject * parent = NULL)
			: QObject(parent)
			, UIModel(messager)
		{
			CreateImplementation(messager, model_settings);
		}


	protected:

		class _impl : public _impl_base<InputModel>
		{

			public:

				_impl(UIMessager & messager, InputModelSettings * model_settings) : _impl_base(messager)
				{
					CreateInternalImplementations(messager, model_settings);
				}

				class _RelatedImpl : public _RelatedImpl_base
				{

					public:

						_RelatedImpl(UIMessager & messager, InputModelSettings * model_settings)
							: _RelatedImpl_base(messager, model_settings)
						{

						}

				};

			protected:

				void CreateInternalImplementations(UIMessager & messager, InputModelSettings * model_settings)
				{
					__impl.reset(new _RelatedImpl(messager, model_settings));
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

		void CreateImplementation(UIMessager & messager, InputModelSettings * model_settings)
		{
			__impl.reset(new _impl(messager, model_settings));
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
