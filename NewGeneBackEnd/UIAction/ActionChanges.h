#ifndef ACTIONCHANGES_H
#define ACTIONCHANGES_H

#include "../UIData/DataChanges.h"
#include <thread>
#include <mutex>

class InputProject;
class OutputProject;

class DataChangeMessage
{

	public:

		DataChangeMessage(InputProject *);
		DataChangeMessage(OutputProject *);
		DataChangeMessage(DataChangeMessage const & rhs);

		std::shared_ptr<std::lock_guard<std::recursive_timed_mutex>> data_lock;
		DataChanges changes;

};



#endif
