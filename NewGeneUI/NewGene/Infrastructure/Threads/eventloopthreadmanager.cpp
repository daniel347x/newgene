#include "eventloopthreadmanager.h"

EventLoopThreadManager::EventLoopThreadManager(int const number_worker_threads, QObject *parent) :
      QObject(parent)
    , work(work_service)
    , worker_pool_ui(work_service, number_worker_threads)
{
	work_queue_manager_thread.start();
	work_queue_manager.moveToThread(&work_queue_manager_thread);
}
