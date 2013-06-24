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
	WidgetDataItem_VARIABLE_GROUPS_TOOLBOX variable_groups(widget_request);
	variable_groups.identifiers = input_model.t_vgp_identifiers.getIdentifiers();
	messager.EmitOutputWidgetDataRefresh(variable_groups);
}

/************************************************************************/
// VARIABLE_GROUP_VARIABLE_GROUP_INSTANCE
/************************************************************************/
void UIDataManager::DoRefreshOutputWidget(Messager & messager, WidgetDataItemRequest_VARIABLE_GROUP_VARIABLE_GROUP_INSTANCE const & widget_request, OutputProject & project)
{
}

