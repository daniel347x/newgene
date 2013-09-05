#ifndef WORKERTHREAD_H
#define WORKERTHREAD_H

#ifndef Q_MOC_RUN
#	include <boost/asio/io_service.hpp>
#endif

class WorkerThread
{

public:

	WorkerThread(boost::asio::io_service & work_service_)
		: work_service(work_service_)
	{

	}

	WorkerThread(WorkerThread const & rhs)
		: work_service(rhs.work_service)
	{

	}

	void operator()()
	{
		try
		{
			work_service.run();
		}
		catch (std::exception & e)
		{
			std::string the_error = e.what();
			std::string copy_error = the_error; // to add a line for debugging purposes only
		}
		catch (boost::exception &)
		{
			//std::string err = e.
		}
	}

	void stop()
	{
		work_service.stop();
	}

protected:

	boost::asio::io_service & work_service;

};

#endif
