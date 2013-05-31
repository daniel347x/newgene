#ifndef UIOUTPUTMODEL_H
#define UIOUTPUTMODEL_H

#include "../../../NewGeneBackEnd/Model/OutputModel.h"
#include "uimodel.h"

class UIOutputModel : public UIModel
{

	public:

		UIOutputModel(UIMessager & messager, boost::filesystem::path const path_to_model = boost::filesystem::path(), QObject * parent = NULL)
			: UIModel(messager, parent)
		{
			CreateImplementation(messager, path_to_model);
		}


	protected:

		class _impl : public _impl_base<OutputModel>
		{

			public:

				_impl(UIMessager & messager, boost::filesystem::path const path_to_model = boost::filesystem::path()) : _impl_base(messager)
				{
					CreateInternalImplementations(messager, path_to_model);
				}

				class _RelatedImpl : public _RelatedImpl_base
				{

					public:

						_RelatedImpl(UIMessager & messager, boost::filesystem::path const path_to_model = boost::filesystem::path())
							: _RelatedImpl_base(messager, path_to_model)
						{

						}

				};

			protected:

				void CreateInternalImplementations(UIMessager & messager, boost::filesystem::path const path_to_model = boost::filesystem::path())
				{
					__impl.reset(new _RelatedImpl(messager, path_to_model));
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

		void CreateImplementation(UIMessager & messager, boost::filesystem::path const path_to_model = boost::filesystem::path())
		{
			__impl.reset(new _impl(messager, path_to_model));
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
