#include "uimanager.h"

namespace MANAGER_DESCRIPTION_NAMESPACE
{

	std::string get_text_name_from_enum_ui(MANAGER_DESCRIPTION_NAMESPACE::WHICH_MANAGER_UI const which_manager)
	{
		if (which_manager == MANAGER_DESCRIPTION_NAMESPACE::MANAGER_DOCUMENTS_UI)
		{
			return "UIDocumentManager";
		}
		else if (which_manager == MANAGER_DESCRIPTION_NAMESPACE::MANAGER_LOGGING_UI)
		{
			return "UILoggingManager";
		}
		else if (which_manager == MANAGER_DESCRIPTION_NAMESPACE::MANAGER_MODEL_UI)
		{
			return "UIModelManager";
		}
		else if (which_manager == MANAGER_DESCRIPTION_NAMESPACE::MANAGER_PROJECT_UI)
		{
			return "UIProjectManager";
		}
		else if (which_manager == MANAGER_DESCRIPTION_NAMESPACE::MANAGER_SETTINGS_UI)
		{
			return "UISettingsManager";
		}
		else if (which_manager == MANAGER_DESCRIPTION_NAMESPACE::MANAGER_STATUS_UI)
		{
			return "UIStatusManager";
		}
		return std::string();
	}

}

