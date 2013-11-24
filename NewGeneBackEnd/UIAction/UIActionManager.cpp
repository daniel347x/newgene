#include "UIActionManager.h"

#ifndef Q_MOC_RUN
#	include <boost/thread/thread.hpp>
#endif

std::recursive_mutex UIActionManager::is_busy_mutex;
std::atomic<bool> UIActionManager::is_busy(false);

bool UIActionManager::FailIfBusy(Messager & messager)
{

	for (int attempts = 0; attempts < 10; ++attempts)
	{
		{
			std::lock_guard<std::recursive_mutex> guard(is_busy_mutex);
			if (!is_busy)
			{
				is_busy = true;
				return false;
			}
		}
		boost::this_thread::sleep_for(boost::chrono::milliseconds(100));
	}

	messager.ShowMessageBox("Another operation is in progress.  Please wait for it to complete first.");
	return true;

}

void UIActionManager::EndFailIfBusy()
{
	is_busy = false;
}
