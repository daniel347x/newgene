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
	OutputModel & output_model = project.model();
	WidgetDataItem_VARIABLE_GROUP_VARIABLE_GROUP_INSTANCE variable_group(widget_request);
	if (widget_request.identifier && widget_request.identifier->uuid)
	{
		WidgetInstanceIdentifiers identifiers = input_model.t_vgp_setmembers.getIdentifiers(*widget_request.identifier->uuid);
		WidgetInstanceIdentifiers selectedIdentifiers = output_model.t_variables_selected_identifiers.getIdentifiers(*widget_request.identifier->uuid);
		std::for_each(identifiers.cbegin(), identifiers.cend(), [&selectedIdentifiers, &variable_group](WidgetInstanceIdentifier const & identifier)
		{
			bool found = false;
			std::for_each(selectedIdentifiers.cbegin(), selectedIdentifiers.cend(), [&identifier, &found, &variable_group](WidgetInstanceIdentifier const & selectedIdentifier)
			{
				if (identifier.IsEqual(WidgetInstanceIdentifier::EQUALITY_CHECK_TYPE__UUID_PLUS_STRING_CODE, selectedIdentifier))
				{
					found = true;
					return; // from lambda
				}
			});
			if (found)
			{
				variable_group.identifiers.push_back(std::make_pair(identifier, true));
			}
			else
			{
				variable_group.identifiers.push_back(std::make_pair(identifier, false));
			}
		});
	}
	std::sort(variable_group.identifiers.begin(), variable_group.identifiers.end());
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
	std::sort(variable_groups.identifiers.begin(), variable_groups.identifiers.end());
	messager.EmitOutputWidgetDataRefresh(variable_groups);
}

/************************************************************************/
// VARIABLE_GROUPS_SUMMARY_VARIABLE_GROUP_INSTANCE
/************************************************************************/
void UIDataManager::DoRefreshOutputWidget(Messager & messager, WidgetDataItemRequest_VARIABLE_GROUPS_SUMMARY_VARIABLE_GROUP_INSTANCE const & widget_request, OutputProject & project)
{
	OutputModel & output_model = project.model();
	WidgetDataItem_VARIABLE_GROUPS_SUMMARY_VARIABLE_GROUP_INSTANCE variable_group(widget_request);
	if (widget_request.identifier && widget_request.identifier->uuid)
	{
		variable_group.identifiers = output_model.t_variables_selected_identifiers.getIdentifiers(*widget_request.identifier->uuid);
	}
	messager.EmitOutputWidgetDataRefresh(variable_group);
}

/************************************************************************/
// KAD_SPIN_CONTROLS_AREA
/************************************************************************/
void UIDataManager::DoRefreshOutputWidget(Messager & messager, WidgetDataItemRequest_KAD_SPIN_CONTROLS_AREA const & widget_request, OutputProject & project)
{
	InputModel & input_model = project.model().getInputModel();
	WidgetDataItem_KAD_SPIN_CONTROLS_AREA variable_groups(widget_request);
	variable_groups.active_dmus = project.model().t_variables_selected_identifiers.GetActiveDMUs(&project.model(), &project.model().getInputModel());
	variable_groups.identifiers = input_model.t_dmu_category.getIdentifiers();
	messager.EmitOutputWidgetDataRefresh(variable_groups);
	std::set<WidgetInstanceIdentifier> active_dmus;
}

/************************************************************************/
// KAD_SPIN_CONTROL_WIDGET
/************************************************************************/
void UIDataManager::DoRefreshOutputWidget(Messager & messager, WidgetDataItemRequest_KAD_SPIN_CONTROL_WIDGET const & widget_request, OutputProject & project)
{
	OutputModel & output_model = project.model();
	WidgetDataItem_KAD_SPIN_CONTROL_WIDGET kad_spincontrol(widget_request);
	if (widget_request.identifier && widget_request.identifier->uuid)
	{
		WidgetInstanceIdentifier_Int_Pair spinControlData = output_model.t_kad_count.getIdentifier(*widget_request.identifier->uuid);
		kad_spincontrol.count = spinControlData.second;
	}
	messager.EmitOutputWidgetDataRefresh(kad_spincontrol);
}

/************************************************************************/
// TIMERANGE_REGION_WIDGET
/************************************************************************/
void UIDataManager::DoRefreshOutputWidget(Messager & messager, WidgetDataItemRequest_TIMERANGE_REGION_WIDGET const & widget_request, OutputProject & project)
{
	OutputModel & output_model = project.model();
	WidgetDataItem_TIMERANGE_REGION_WIDGET timerange_region(widget_request);
	WidgetInstanceIdentifier_Int64_Pair timerange_start_identifier;
	std::pair<bool, std::int64_t> info = output_model.t_general_options.getRandomSamplingInfo(project.model().getDb());
	timerange_region.do_random_sampling = info.first;
	timerange_region.random_sampling_count_per_stage = info.second;
	messager.EmitOutputWidgetDataRefresh(timerange_region);
}

/************************************************************************/
// DATETIME_WIDGET
/************************************************************************/
void UIDataManager::DoRefreshOutputWidget(Messager & messager, WidgetDataItemRequest_DATETIME_WIDGET const & widget_request, OutputProject & project)
{
	OutputModel & output_model = project.model();
	WidgetDataItem_DATETIME_WIDGET timerange_datetimecontrol(widget_request);
	if (widget_request.identifier && widget_request.identifier->code && *widget_request.identifier->code == "0")
	{
		if (widget_request.identifier->flags == "s")
		{
			WidgetInstanceIdentifier_Int64_Pair timerange_start_identifier;
			bool found = output_model.t_time_range.getIdentifierFromStringCodeAndFlags("0", "s", timerange_start_identifier);
			if (found)
			{
				timerange_datetimecontrol.the_date_time = timerange_start_identifier.second;
			}
		}
		else if (widget_request.identifier->flags == "e")
		{
			WidgetInstanceIdentifier_Int64_Pair timerange_end_identifier;
			bool found = output_model.t_time_range.getIdentifierFromStringCodeAndFlags("0", "e", timerange_end_identifier);
			if (found)
			{
				timerange_datetimecontrol.the_date_time = timerange_end_identifier.second;
			}
		}
	}
	messager.EmitOutputWidgetDataRefresh(timerange_datetimecontrol);
}

/************************************************************************/
// GENERATE_OUTPUT_TAB
/************************************************************************/
void UIDataManager::DoRefreshOutputWidget(Messager & messager, WidgetDataItemRequest_GENERATE_OUTPUT_TAB const & widget_request, OutputProject & project)
{
	OutputModel & output_model = project.model();
	WidgetDataItem_GENERATE_OUTPUT_TAB generate_output_tab_data(widget_request);
	WidgetInstanceIdentifier_Int64_Pair timerange_start_identifier;
	messager.EmitOutputWidgetDataRefresh(generate_output_tab_data);
}
