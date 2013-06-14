#ifndef EVENTLOOPTHREADMANAGER_H
#define EVENTLOOPTHREADMANAGER_H

#include <QObject>
#include <QThread>
#include "../../../NewGeneBackEnd/Threads/ThreadPool.h"
#include "../../../NewGeneBackEnd/Threads/WorkerThread.h"
#include "workqueuemanager.h"

class EventLoopThreadManager : public QObject
{
		Q_OBJECT

	public:
		explicit EventLoopThreadManager(int const number_worker_threads, QObject *parent = 0);

	signals:

	public slots:

	public:

		WorkQueueManager & getQueueManager()
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

	protected:

		// Boost-layer thread pool, which is accessible in both the UI layer
		// and in the backend layer
		boost::asio::io_service work_service;
		boost::asio::io_service::work work;
		ThreadPool<WorkerThread> worker_pool_ui;

		// Qt-layer thread, which runs the event loop in a separate background thread
		QThread work_queue_manager_thread;
		WorkQueueManager work_queue_manager;

	private:

		void StopPoolAndWaitForTasksToComplete()
		{
			// blocks
			worker_pool_ui.StopPoolAndWaitForTasksToComplete();
		}

};

#endif // EVENTLOOPTHREADMANAGER_H
