#ifndef UIMODEL_H
#define UIMODEL_H

#include <QObject>
#include "../../../NewGeneBackEnd/Model/Model.h"
#include "uimessager.h"
#include "../../../NewGeneBackEnd/globals.h"
#include "eventloopthreadmanager.h"
#ifndef Q_MOC_RUN
#	include <boost/atomic.hpp>
#endif


template<WORK_QUEUE_THREAD_LOOP_CLASS_ENUM UI_THREAD_LOOP_CLASS_ENUM>
class UIModel : public EventLoopThreadManager<UI_THREAD_LOOP_CLASS_ENUM>
{

	protected:
		static int const number_of_pools;

	public:

        UIModel(UIMessager &, int const number_worker_threads)
			: EventLoopThreadManager<UI_THREAD_LOOP_CLASS_ENUM>(number_worker_threads, number_of_pools)
			, loaded_(false)
		{

		}

		bool loaded()
		{
			return loaded_.load();
		}

		void loaded(bool const loaded__)
		{
			loaded_.store(loaded__);
		}

		virtual void UpdateConnections() {}

	protected:

		template<typename BACKEND_MODEL_CLASS>
		class _impl_base
		{

			public:

				_impl_base(UIMessager &)
				{}

				class _RelatedImpl_base
				{

					public:

						BACKEND_MODEL_CLASS & getModel()
						{
							if (!_model)
							{
								boost::format msg( "Model instance not yet constructed." );
								throw NewGeneException() << newgene_error_description( msg.str() );
							}
							return *_model;
						}

						std::shared_ptr<BACKEND_MODEL_CLASS> & getModelSharedPtr()
						{
							return _model;
						}


					protected:

                        _RelatedImpl_base(UIMessager &, std::shared_ptr<BACKEND_MODEL_CLASS> const & backend_model_instance)
							: _model(backend_model_instance)
						{
						}

						std::shared_ptr<BACKEND_MODEL_CLASS> _model;

				};

                std::unique_ptr<UIModel<UI_THREAD_LOOP_CLASS_ENUM>::_impl_base<BACKEND_MODEL_CLASS>::_RelatedImpl_base> __impl;

                UIModel<UI_THREAD_LOOP_CLASS_ENUM>::_impl_base<BACKEND_MODEL_CLASS>::_RelatedImpl_base & getInternalImplementation()
				{
                    if (!__impl)
					{
						boost::format msg( "Internal backend model implementation not yet constructed." );
						throw NewGeneException() << newgene_error_description( msg.str() );
					}
                    return *(__impl.get());
				}

		};


	protected:

		template<typename BACKEND_MODEL_CLASS>
		BACKEND_MODEL_CLASS &
        getBackendModel_base(UIModel<UI_THREAD_LOOP_CLASS_ENUM>::_impl_base<BACKEND_MODEL_CLASS> & impl)
		{
            return impl.getInternalImplementation().getModel();
		}

		template<typename BACKEND_MODEL_CLASS>
		std::shared_ptr<BACKEND_MODEL_CLASS>
		getBackendModelSharedPtr_base(_impl_base<BACKEND_MODEL_CLASS> & impl)
		{
            return impl.getInternalImplementation().getModelSharedPtr();
		}

		boost::atomic<bool> loaded_;

};

template<WORK_QUEUE_THREAD_LOOP_CLASS_ENUM UI_THREAD_LOOP_CLASS_ENUM>
int const UIModel<UI_THREAD_LOOP_CLASS_ENUM>::number_of_pools = 2;

#endif // UImodel_H
