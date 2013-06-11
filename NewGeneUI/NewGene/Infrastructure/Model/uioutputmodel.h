#ifndef UIOUTPUTMODEL_H
#define UIOUTPUTMODEL_H

#include "../../../NewGeneBackEnd/Model/OutputModel.h"
#include "uimodel.h"

class UIOutputModel : public QObject, public UIModel<OutputModelSettings>
{

	public:

		UIOutputModel(UIMessager & messager, OutputModelSettings * model_settings, QObject * parent = NULL)
			: QObject(parent)
			, UIModel(messager)
		{
			CreateImplementation(messager, model_settings);
		}


	protected:

		class _impl : public _impl_base<OutputModel>
		{

			public:

				_impl(UIMessager & messager, OutputModelSettings * model_settings) : _impl_base(messager)
				{
					CreateInternalImplementations(messager, model_settings);
				}

				class _RelatedImpl : public _RelatedImpl_base
				{

					public:

						_RelatedImpl(UIMessager & messager, OutputModelSettings * model_settings)
							: _RelatedImpl_base(messager, model_settings)
						{

						}

				};

			protected:

				void CreateInternalImplementations(UIMessager & messager, OutputModelSettings * model_settings)
				{
					__impl.reset(new _RelatedImpl(messager, model_settings));
				}

		};

		_impl_base<OutputModel> & getImplementation()
		{
			if (!__impl)
			{
				boost::format msg( "Model implementation not yet constructed." );
				throw NewGeneException() << newgene_error_description( msg.str() );
			}
			return *__impl;
		}

		void CreateImplementation(UIMessager & messager, OutputModelSettings * model_settings)
		{
			__impl.reset(new _impl(messager, model_settings));
		}

		std::unique_ptr<_impl> __impl;


	public:

		OutputModel & getBackendModel()
		{
			if (!__impl)
			{
				boost::format msg( "Internal model implementation not yet constructed." );
				throw NewGeneException() << newgene_error_description( msg.str() );
			}
			return static_cast<OutputModel &>(getBackendModel_base<OutputModel>(*__impl));
		}

		std::shared_ptr<OutputModel> & getBackendModelSharedPtr()
		{
			if (!__impl)
			{
				boost::format msg( "Internal model implementation not yet constructed." );
				throw NewGeneException() << newgene_error_description( msg.str() );
			}
			return getBackendModelSharedPtr_base<OutputModel>(*__impl);
		}

};

#endif // UIOUTPUTMODEL_H
