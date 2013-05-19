#include "Manager.h"

namespace MANAGER_DESCRIPTION_NAMESPACE
{

	std::string get_text_name_from_enum(MANAGER_DESCRIPTION_NAMESPACE::WHICH_MANAGER const which_manager)
	{
		if (which_manager == MANAGER_DESCRIPTION_NAMESPACE::MANAGER_DOCUMENTS)
		{
			return "UIDocumentManager";
		}
		else if (which_manager == MANAGER_DESCRIPTION_NAMESPACE::MANAGER_LOGGING)
		{
			return "UILoggingManager";
		}
		else if (which_manager == MANAGER_DESCRIPTION_NAMESPACE::MANAGER_MODEL)
		{
			return "UIModelManager";
		}
		else if (which_manager == MANAGER_DESCRIPTION_NAMESPACE::MANAGER_PROJECT)
		{
			return "UIProjectManager";
		}
		else if (which_manager == MANAGER_DESCRIPTION_NAMESPACE::MANAGER_SETTINGS)
		{
			return "UISettingsManager";
		}
		else if (which_manager == MANAGER_DESCRIPTION_NAMESPACE::MANAGER_STATUS)
		{
			return "UIStatusManager";
		}
		return std::string();
	}

}
