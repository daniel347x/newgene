#ifndef SEMAPHORE_DANIEL347X_H
#define SEMAPHORE_DANIEL347X_H

#include <mutex>
#include <condition_variable>

class semaphore
{

	private:

		std::mutex mtx;
		std::condition_variable cv;
		int count;

	public:

		semaphore(int count_ = 0) : count(count_) { ; }

		void notify()
		{

			std::unique_lock<std::mutex> lck(mtx);
			++count;
			cv.notify_one();

		}

		void wait()
		{

			std::unique_lock<std::mutex> lck(mtx);

			while (count == 0)
			{
				cv.wait(lck);
			}

			count--;

		}

};

#endif
