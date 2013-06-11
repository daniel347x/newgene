#ifndef MODEL_H
#define MODEL_H

#ifndef Q_MOC_RUN
#	include <boost/filesystem.hpp>
#	include <boost/format.hpp>
#endif
#include "..\Messager\Messager.h"
#ifndef Q_MOC_RUN
#	include <boost/property_tree/ptree.hpp>
#	include <boost/property_tree/xml_parser.hpp>
#	include <boost/asio/io_service.hpp>
#endif
#include "../Threads/ThreadPool.h"
#include "../Threads/WorkerThread.h"
#include "../Settings/ModelSettings.h"

template<typename MODEL_SETTINGS_ENUM, typename MODEL_SETTING_CLASS>
class Model
{

	public:

		static int const number_worker_threads = 1; // For now, single thread only in pool

		Model(Messager & messager, ModelSettings<MODEL_SETTINGS_ENUM, MODEL_SETTING_CLASS> * model_settings)
			: _settings(model_settings)
			, work(work_service)
			, worker_pool_model(work_service, number_worker_threads)
		{

		}

	protected:

		std::unique_ptr<ModelSettings<MODEL_SETTINGS_ENUM, MODEL_SETTING_CLASS> > const _settings;

		boost::asio::io_service work_service;
		boost::asio::io_service::work work;
		ThreadPool<WorkerThread> worker_pool_model;

};

template<typename MODEL_CLASS, typename MODEL_SETTINGS_CLASS>
class ModelFactory
{

	public:

		MODEL_CLASS * operator()(Messager & messager, MODEL_SETTINGS_CLASS * model_settings)
		{
			MODEL_CLASS * new_model = new MODEL_CLASS(messager, model_settings);
			return new_model;
		}

};

#endif
