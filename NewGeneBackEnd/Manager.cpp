#include "Manager.h"

namespace MANAGER_DESCRIPTION_NAMESPACE
{

	std::string get_text_name_from_enum(MANAGER_DESCRIPTION_NAMESPACE::WHICH_MANAGER const which_manager)
	{
		if (which_manager == MANAGER_DESCRIPTION_NAMESPACE::MANAGER_DOCUMENTS)
		{
			return "DocumentManager";
		}
		else if (which_manager == MANAGER_DESCRIPTION_NAMESPACE::MANAGER_LOGGING)
		{
			return "LoggingManager";
		}
		else if (which_manager == MANAGER_DESCRIPTION_NAMESPACE::MANAGER_MODEL)
		{
			return "ModelManager";
		}
		else if (which_manager == MANAGER_DESCRIPTION_NAMESPACE::MANAGER_PROJECT)
		{
			return "ProjectManager";
		}
		else if (which_manager == MANAGER_DESCRIPTION_NAMESPACE::MANAGER_SETTINGS)
		{
			return "SettingsManager";
		}
		else if (which_manager == MANAGER_DESCRIPTION_NAMESPACE::MANAGER_STATUS)
		{
			return "StatusManager";
		}
		else if (which_manager == MANAGER_DESCRIPTION_NAMESPACE::MANAGER_THREADS)
		{
			return "ThreadManager";
		}
		else if (which_manager == MANAGER_DESCRIPTION_NAMESPACE::MANAGER_TRIGGERS)
		{
			return "TriggerManager";
		}
		else if (which_manager == MANAGER_DESCRIPTION_NAMESPACE::MANAGER_UI_DATA)
		{
			return "UIDataManager";
		}
		return std::string();
	}

}
