#include "UIActionManager.h"

#include "../Project/InputProject.h"
#include "../Project/OutputProject.h"

/************************************************************************/
// ACTION_VARIABLE_GROUP_SET_MEMBER_SELECTION_CHANGED
/************************************************************************/
void UIActionManager::DoVariableGroupSetMemberSelectionChange(Messager & messager, WidgetActionItemRequest_ACTION_VARIABLE_GROUP_SET_MEMBER_SELECTION_CHANGED const & action_request, OutputProject & project)
{

	InputModel & input_model = project.model().getInputModel();
	
	//WidgetDataItem_VARIABLE_GROUPS_TOOLBOX variable_groups(widget_request);
	//variable_groups.identifiers = input_model.t_vgp_identifiers.getIdentifiers();

	//messager.EmitOutputWidgetDataRefresh(variable_groups);

	//messager.ShowMessageBox("Crazy crap");

}
