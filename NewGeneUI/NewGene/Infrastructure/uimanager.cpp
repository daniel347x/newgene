class NewGeneMainWindow;
extern NewGeneMainWindow * theMainWindow;

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
		else if (which_manager == MANAGER_DESCRIPTION_NAMESPACE::MANAGER_THREADS_UI)
		{
			return "UIThreadManager";
		}
		else if (which_manager == MANAGER_DESCRIPTION_NAMESPACE::MANAGER_TRIGGERS_UI)
		{
			return "UITriggerManager";
		}
		else if (which_manager == MANAGER_DESCRIPTION_NAMESPACE::MANAGER_UI_DATA_UI)
		{
			return "UITriggerManager";
		}
		else if (which_manager == MANAGER_DESCRIPTION_NAMESPACE::MANAGER_UI_ACTION_UI)
		{
			return "UITriggerManager";
		}
		else if (which_manager == MANAGER_DESCRIPTION_NAMESPACE::MANAGER_MODEL_ACTION_UI)
		{
			return "UITriggerManager";
		}

		return std::string();
	}

}

