#ifndef WORKERTHREAD_H
#define WORKERTHREAD_H

#ifndef Q_MOC_RUN
	#include <boost/asio/io_service.hpp>
#endif

#include "../Messager/Messager.h"

class WorkerThread
{

	public:

		WorkerThread(Messager & messager_, boost::asio::io_service & work_service_)
			: work_service(work_service_)
			, messager(messager_)
		{

		}

		WorkerThread(WorkerThread const & rhs)
			: work_service(rhs.work_service)
			, messager(rhs.messager)
		{

		}

		void operator()()
		{

			// http://stackoverflow.com/a/13232440/368896
			// http://www.boost.org/doc/libs/1_46_0/doc/html/boost_asio/reference/io_service.html (section "Effect of exceptions thrown from handlers")
			for (;;)
			{
				try
				{
					// In our usage, because we are not calling "stop" until the project/model is unloaded,
					// this will block even when there are no tasks in the queue.
					// But if an EXCEPTION is thrown, we must call run() again.
					// See links just above.
					work_service.run();
					break;
				}
				catch (boost::exception & e)
				{
					if (std::string const * error_desc = boost::get_error_info<newgene_error_description>(e))
					{
						boost::format msg(error_desc->c_str());
						messager.ShowMessageBox(msg.str());
					}
					else
					{
						std::string the_error = boost::diagnostic_information(e);
						boost::format msg("Error: %1%");
						msg % the_error.c_str();
						messager.ShowMessageBox(msg.str());
					}

				}
				catch (std::exception & e)
				{
					std::string the_error = e.what();
					messager.ShowMessageBox(the_error);
				}
			}
		}

		void stop()
		{
			work_service.stop();
		}

	protected:

		boost::asio::io_service & work_service;

		Messager & messager;

};

#endif
