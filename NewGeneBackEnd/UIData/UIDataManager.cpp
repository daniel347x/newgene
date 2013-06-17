#include "UIDataManager.h"

#include "../Project/InputProject.h"
#include "../Project/OutputProject.h"

void UIDataManager::DoRefreshInputWidget(Messager & messager, DATA_WIDGETS widget, InputProject & project)
{

}

void UIDataManager::DoRefreshOutputWidget(Messager & messager, DATA_WIDGETS widget, OutputProject & project)
{
	// switch on widget type here

	InputModel & input_model = project.model().getInputModel();
	WidgetDataItem_VARIABLE_GROUPS_TOOLBOX variable_groups;
	variable_groups.variable_group_long_names = input_model.t_vgp_identifiers.identifiers;
	messager.EmitOutputWidgetDataRefresh(variable_groups);
}
