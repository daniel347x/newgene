#ifndef EVENTLOOPTHREADMANAGER_H
#define EVENTLOOPTHREADMANAGER_H

#include <QThread>
#include "../../../NewGeneBackEnd/Threads/ThreadPool.h"
#include "../../../NewGeneBackEnd/Threads/WorkerThread.h"
#include "workqueuemanager.h"

template<WORK_QUEUE_THREAD_LOOP_CLASS_ENUM UI_THREAD_LOOP_CLASS_ENUM>
class EventLoopThreadManager
{

	public:

		EventLoopThreadManager(int const number_worker_threads)
			: work(work_service)
			, worker_pool_ui(work_service, number_worker_threads)
		{
			work_queue_manager_thread.start();
			work_queue_manager.moveToThread(&work_queue_manager_thread);
		}

	public:

		WorkQueueManager<UI_THREAD_LOOP_CLASS_ENUM> & getQueueManager()
		{
			return work_queue_manager;
		}

		QThread & getQueueManagerThread()
		{
			return work_queue_manager_thread;
		}

		void EndLoopAndBackgroundPool()
		{
			// First, kill the Boost thread pool (but complete pending tasks first)
			StopPoolAndWaitForTasksToComplete(); // Do this here so that this complete object is valid, and Qt thread is still running, while background tasks run to completion

			// Only now kill the Qt-layer event loop
			getQueueManagerThread().quit();
		}

		QObject * getConnector()
		{
			return &work_queue_manager;
		}

	protected:

		// Boost-layer thread pool, which is accessible in both the UI layer
		// and in the backend layer
		boost::asio::io_service work_service;
		boost::asio::io_service::work work;
		ThreadPool<WorkerThread> worker_pool_ui;

		// Qt-layer thread, which runs the event loop in a separate background thread
		QThread work_queue_manager_thread;
		WorkQueueManager<UI_THREAD_LOOP_CLASS_ENUM> work_queue_manager;

	private:

		void StopPoolAndWaitForTasksToComplete()
		{
			// blocks
			worker_pool_ui.StopPoolAndWaitForTasksToComplete();
		}

};

#endif // EVENTLOOPTHREADMANAGER_H
