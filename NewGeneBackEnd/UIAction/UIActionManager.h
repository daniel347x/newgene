#ifndef UIACTIONMANAGER_H
#define UIACTIONMANAGER_H

#include "../globals.h"
#include "..\Manager.h"
#include "../Messager/Messager.h"

class InputProject;
class OutputProject;

class UIActionManager : public Manager<UIActionManager, MANAGER_DESCRIPTION_NAMESPACE::MANAGER_UI_ACTION>
{

	public:

		void DoVariableGroupSetMemberSelectionChange(Messager & messager, WidgetActionItemRequest_ACTION_VARIABLE_GROUP_SET_MEMBER_SELECTION_CHANGED const & action_request, OutputProject & project);
		void DoKAdCountChange(Messager & messager, WidgetActionItemRequest_ACTION_KAD_COUNT_CHANGE const & action_request, OutputProject & project);
		void DoGenerateOutput(Messager & messager, WidgetActionItemRequest_ACTION_GENERATE_OUTPUT const & action_request, OutputProject & project);

};

#endif
