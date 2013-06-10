#ifndef UIPROJECT_H
#define UIPROJECT_H

//#include "globals.h"
#include <QObject>
#include <memory>
#ifndef Q_MOC_RUN
#	include <boost/filesystem.hpp>
#	include <boost/asio/io_service.hpp>
#endif
#include "../../../NewGeneBackEnd/Project/Project.h"
#include "../Model/Base/uimodel.h"
#include "../../../NewGeneBackEnd/Threads/ThreadPool.h"
#include "../../../NewGeneBackEnd/Threads/WorkerThread.h"
#include "workqueuemanager.h"
#include <QThread>

class UIModelManager;
class UISettingsManager;
class UIDocumentManager;
class UIStatusManager;
class UIProjectManager;
class NewGeneMainWindow;
class UILoggingManager;
class UIProjectManager;

template<typename BACKEND_PROJECT_CLASS, typename UI_PROJECT_SETTINGS_CLASS, typename UI_MODEL_CLASS>
class UIProject
{
	public:

		static int const number_worker_threads = 1; // For now, single thread only in pool

		UIProject(UIMessager & messager, UI_PROJECT_SETTINGS_CLASS * ui_settings, UI_MODEL_CLASS * ui_model)
			: _project_settings(ui_settings)
			, _model(ui_model)
			, _backend_project( new BACKEND_PROJECT_CLASS(messager, _project_settings->getBackendSettingsSharedPtr(), _model->getBackendModelSharedPtr()) )
			, work(work_service)
			, worker_pool_ui(work_service, number_worker_threads)
		{
			work_queue_manager.moveToThread(&work_queue_manager_thread);
		}

		~UIProject()
		{
			work_queue_manager_thread.quit();
		}

		// TODO: Test for validity
		UI_PROJECT_SETTINGS_CLASS & settings()
		{
			return *_project_settings;
		}

		// TODO: Test for validity
		UI_MODEL_CLASS & model()
		{
			return *_model;
		}

		// TODO: Test for validity
		BACKEND_PROJECT_CLASS & backend()
		{
			return *_backend_project;
		}

	protected:

		// The order of initialization is important.
		// C++ data members are initialized in the order they appear
		// in the class declaration (this file).
		// Do not reorder the declarations of these member variables.

		// The settings are distinguished from the model only by the fact
		// that they are stored permanently in human-readable XML or text files
		// in order to be more accessible to end-users, and hence readable and
		// editable by knowledgeable users.
		std::shared_ptr<UI_PROJECT_SETTINGS_CLASS> const _project_settings;

		// The model represents the data and metadata itself.
		// The model and the settings (below) form an unseparable pair
		// that together define a single NewGene "project".
		// The model definition is contained within a single database file,
		// although the project may contain many database files.
		std::shared_ptr<UI_MODEL_CLASS> const _model;

		// The backend project is the backend equivalent of this UIProject,
		// with the exception that this UIProject carries a shared_ptr to the backend project,
		// but not vice-versa (the backend project should not be, and is not,
		// affected by the state of the user interface).
		std::unique_ptr<BACKEND_PROJECT_CLASS> const _backend_project;

		boost::asio::io_service work_service;
		boost::asio::io_service::work work;
		ThreadPool<WorkerThread> worker_pool_ui;

		QThread work_queue_manager_thread;
		WorkQueueManager work_queue_manager;

};

#endif // UIPROJECT_H
