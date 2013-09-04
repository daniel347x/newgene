#include "UIActionManager.h"

std::recursive_mutex UIActionManager::is_busy_mutex;
std::atomic<bool> UIActionManager::is_busy = false;

bool UIActionManager::FailIfBusy(Messager & messager)
{

	{
		std::lock_guard<std::recursive_mutex> guard(is_busy_mutex);
		if (is_busy)
		{
			messager.ShowMessageBox("Another operation is in progress.  Please wait for it to complete first.");
			return true;
		}
		is_busy = true;
	}

	return false;

}

void UIActionManager::EndFailIfBusy()
{
	is_busy = false;
}
