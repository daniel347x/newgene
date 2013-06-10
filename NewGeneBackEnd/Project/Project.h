#ifndef PROJECT_H
#define PROJECT_H

#include <memory>
#include "../Settings/ProjectSettings.h"
#include "../Threads/ThreadPool.h"
#include "../Threads/WorkerThread.h"
#ifndef Q_MOC_RUN
#	include <boost/asio/io_service.hpp>
#endif

template<typename PROJECT_SETTINGS_ENUM, typename BACKEND_PROJECT_SETTING_CLASS, typename MODEL_CLASS>
class Project
{

	public:

		static int const number_worker_threads = 1; // For now, single thread only in pool

		Project(Messager & messager, std::shared_ptr<ProjectSettings<PROJECT_SETTINGS_ENUM, BACKEND_PROJECT_SETTING_CLASS> > const & settings, std::shared_ptr<MODEL_CLASS> const & model)
			: _settings(settings)
			, _model(model)
			, work(work_service)
			, worker_pool_backend(work_service, number_worker_threads)
		{

		}

		ProjectSettings<PROJECT_SETTINGS_ENUM, BACKEND_PROJECT_SETTING_CLASS> & settings()
		{
			return *_settings;
		}

		MODEL_CLASS & model()
		{
			return *_model;
		}

	protected:

		std::shared_ptr<ProjectSettings<PROJECT_SETTINGS_ENUM, BACKEND_PROJECT_SETTING_CLASS> > const _settings;
		std::shared_ptr<MODEL_CLASS> const _model;

		boost::asio::io_service work_service;
		boost::asio::io_service::work work;
		ThreadPool<WorkerThread> worker_pool_backend;

};

#endif
