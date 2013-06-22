#include "UIDataManager.h"

#include "../Project/InputProject.h"
#include "../Project/OutputProject.h"

/************************************************************************/
// VARIABLE_GROUPS_SCROLL_AREA
/************************************************************************/
void UIDataManager::DoRefreshOutputWidget(Messager & messager, WidgetDataItemRequest_VARIABLE_GROUPS_SCROLL_AREA const & widget_request, OutputProject & project)
{

}

/************************************************************************/
// VARIABLE_GROUPS_TOOLBOX
/************************************************************************/
void UIDataManager::DoRefreshOutputWidget(Messager & messager, WidgetDataItemRequest_VARIABLE_GROUPS_TOOLBOX const & widget_request, OutputProject & project)
{
	InputModel & input_model = project.model().getInputModel();
	WidgetDataItem_VARIABLE_GROUPS_TOOLBOX variable_groups;
	variable_groups.variable_group_long_names = input_model.t_vgp_identifiers.identifiers;
	variable_groups.variable_group_long_names.push_back(widget_request.s);
	messager.EmitOutputWidgetDataRefresh(variable_groups);
}

