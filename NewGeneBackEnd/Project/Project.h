#ifndef PROJECT_H
#define PROJECT_H

#ifndef Q_MOC_RUN
	//#	include <boost/thread/recursive_timed_mutex.hpp>
#endif
#include <memory>
#include "../Settings/ProjectSettings.h"
#include "../Settings/ModelSettings.h"
#include "../Threads/ThreadPool.h"
#include "../Threads/WorkerThread.h"
#include <mutex>

template<typename PROJECT_SETTINGS_ENUM, typename BACKEND_PROJECT_SETTING_CLASS, typename MODEL_SETTINGS_ENUM, typename MODEL_SETTING_CLASS, typename MODEL_CLASS>
class Project
{

	public:

		static int const number_worker_threads = 1; // For now, single thread only in pool

		Project(std::shared_ptr<ProjectSettings<PROJECT_SETTINGS_ENUM, BACKEND_PROJECT_SETTING_CLASS> > const & project_settings,
				std::shared_ptr<ModelSettings<MODEL_SETTINGS_ENUM, MODEL_SETTING_CLASS>> const & model_settings,
				std::shared_ptr<MODEL_CLASS> const & model)
			: _project_settings(project_settings)
			, _model_settings(model_settings)
			, _model(model)
		{

		}

		ProjectSettings<PROJECT_SETTINGS_ENUM, BACKEND_PROJECT_SETTING_CLASS> & projectSettings()
		{
			return *_project_settings;
		}

		ModelSettings<MODEL_SETTINGS_ENUM, MODEL_SETTING_CLASS> & modelSettings()
		{
			return *_model_settings;
		}

		MODEL_CLASS & model()
		{
			return *_model;
		}

		std::shared_ptr<ModelSettings<MODEL_SETTINGS_ENUM, MODEL_SETTING_CLASS>> const modelSettingsSharedPtr()
		{
			return _model_settings;
		}

		std::shared_ptr<MODEL_CLASS> const modelSharedPtr()
		{
			return _model;
		}

	protected:

		std::shared_ptr<ProjectSettings<PROJECT_SETTINGS_ENUM, BACKEND_PROJECT_SETTING_CLASS>> const _project_settings;
		std::shared_ptr<ModelSettings<MODEL_SETTINGS_ENUM, MODEL_SETTING_CLASS>> const _model_settings;
		std::shared_ptr<MODEL_CLASS> const _model;

	public:

		std::recursive_timed_mutex data_mutex;

};

#endif
