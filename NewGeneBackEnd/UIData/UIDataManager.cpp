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
	if (sqlite3_threadsafe() == 0)
	{
		messager.ShowMessageBox("SQLite is not threadsafe!");
		return;
	}
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
	InputModel & input_model = project.model().getInputModel();
	WidgetDataItem_VARIABLE_GROUP_VARIABLE_GROUP_INSTANCE variable_group(widget_request);
	if (widget_request.identifier && widget_request.identifier->uuid)
	{
		variable_group.identifiers = input_model.t_vgp_setmembers.getIdentifiers(*widget_request.identifier->uuid);
	}
	messager.EmitOutputWidgetDataRefresh(variable_group);
}

/************************************************************************/
// VARIABLE_GROUPS_SUMMARY
/************************************************************************/
void UIDataManager::DoRefreshOutputWidget(Messager & messager, WidgetDataItemRequest_VARIABLE_GROUPS_SUMMARY_SCROLL_AREA const & widget_request, OutputProject & project)
{
	InputModel & input_model = project.model().getInputModel();
	WidgetDataItem_VARIABLE_GROUPS_SUMMARY_SCROLL_AREA variable_groups(widget_request);
	variable_groups.identifiers = input_model.t_vgp_identifiers.getIdentifiers();
	messager.EmitOutputWidgetDataRefresh(variable_groups);
}

/************************************************************************/
// VARIABLE_GROUPS_SUMMARY_VARIABLE_GROUP_INSTANCE
/************************************************************************/
void UIDataManager::DoRefreshOutputWidget(Messager & messager, WidgetDataItemRequest_VARIABLE_GROUPS_SUMMARY_VARIABLE_GROUP_INSTANCE const & widget_request, OutputProject & project)
{
	InputModel & input_model = project.model().getInputModel();
	WidgetDataItem_VARIABLE_GROUPS_SUMMARY_VARIABLE_GROUP_INSTANCE variable_group(widget_request);
	if (widget_request.identifier && widget_request.identifier->uuid)
	{
		variable_group.identifiers = input_model.t_vgp_setmembers.getIdentifiers(*widget_request.identifier->uuid);
	}
	messager.EmitOutputWidgetDataRefresh(variable_group);
}
